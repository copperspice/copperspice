/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <qopengl.h>
#include <qopengl_p.h>

#include <qopenglcontext.h>
#include <qopenglfunctions.h>
#include <qoffscreensurface.h>
#include <qdebug.h>
#include <qjsondocument.h>
#include <qjsonvalue.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qdir.h>

#include <set>

#if defined(QT_OPENGL_3)
typedef const GLubyte * (QOPENGLF_APIENTRYP qt_glGetStringi)(GLenum, GLuint);
#endif

QOpenGLExtensionMatcher::QOpenGLExtensionMatcher()
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    QOpenGLFunctions *funcs = ctx->functions();
    const char *extensionStr = nullptr;

    if (ctx->isOpenGLES() || ctx->format().majorVersion() < 3)
        extensionStr = reinterpret_cast<const char *>(funcs->glGetString(GL_EXTENSIONS));

    if (extensionStr) {
        QByteArray ba(extensionStr);
        QList<QByteArray> extensions = ba.split(' ');
        m_extensions = extensions.toSet();
    } else {
#ifdef QT_OPENGL_3
        // clear error state
        while (funcs->glGetError()) {}

        if (ctx) {
            qt_glGetStringi glGetStringi = (qt_glGetStringi)ctx->getProcAddress("glGetStringi");

            if (!glGetStringi)
                return;

            GLint numExtensions = 0;
            funcs->glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

            for (int i = 0; i < numExtensions; ++i) {
                const char *str = reinterpret_cast<const char *>(glGetStringi(GL_EXTENSIONS, i));
                m_extensions.insert(str);
            }
        }
#endif // QT_OPENGL_3
    }
}

/* Helpers to read out the list of features matching a device from
 * a Chromium driver bug list. Note that not all keys are supported and
 * some may behave differently: gl_vendor is a substring match instead of regex.
 {
  "entries": [
 {
      "id": 20,
      "description": "Disable EXT_draw_buffers on GeForce GT 650M on Linux due to driver bugs",
      "os": {
        "type": "linux"
      },
      // Optional: "exceptions" list
      "vendor_id": "0x10de",
      "device_id": ["0x0fd5"],
      "multi_gpu_category": "any",
      "features": [
        "disable_ext_draw_buffers"
      ]
    },
   ....
   }
*/

QDebug operator<<(QDebug d, const QOpenGLConfig::Gpu &g)
{
    QDebugStateSaver s(d);
    d.nospace();
    d << "Gpu(";
    if (g.isValid()) {
        d << "vendor=" << hex << showbase <<g.vendorId << ", device=" << g.deviceId
          << "version=" << g.driverVersion;
    } else {
        d << 0;
    }
    d << ')';
    return d;
}

static inline QString valueKey()             { return QString("value");      }
static inline QString opKey()                { return QString("op");         }
static inline QString versionKey()           { return QString("version");    }
static inline QString releaseKey()           { return QString("release");    }
static inline QString typeKey()              { return QString("type");       }
static inline QString osKey()                { return QString("os");         }
static inline QString vendorIdKey()          { return QString("vendor_id");  }
static inline QString glVendorKey()          { return QString("gl_vendor");  }
static inline QString deviceIdKey()          { return QString("device_id");  }
static inline QString driverVersionKey()     { return QString("driver_version");     }
static inline QString driverDescriptionKey() { return QString("driver_description"); }
static inline QString featuresKey()          { return QString("features");    }
static inline QString idKey()                { return QString("id");          }
static inline QString descriptionKey()       { return QString("description"); }
static inline QString exceptionsKey()        { return QString("exceptions");  }


static inline bool contains(const QJsonArray &haystack, unsigned needle)
{
    for (auto it = haystack.constBegin(), cend = haystack.constEnd(); it != cend; ++it) {
        if (needle == it->toString().toInteger<unsigned int>(nullptr, /* base */ 0))
            return true;
    }
    return false;
}

static inline bool contains(const QJsonArray &haystack, const QString &needle)
{
    for (auto it = haystack.constBegin(), cend = haystack.constEnd(); it != cend; ++it) {
        if (needle == it->toString())
            return true;
    }
    return false;
}

namespace {

enum Operator {
   NotEqual,
   LessThan,
   LessEqualThan,
   Equals,
   GreaterThan,
   GreaterEqualThan
};

static const char operators[][3] = {"!=", "<", "<=", "=", ">", ">="};

// VersionTerm describing a version term consisting of number and operator
// found in os.version and driver_version.

struct VersionTerm {
    VersionTerm() : op(NotEqual) {}
    static VersionTerm fromJson(const QJsonValue &v);
    bool isNull() const { return number.isNull(); }
    bool matches(const QVersionNumber &other) const;

    QVersionNumber number;
    Operator op;
};

bool VersionTerm::matches(const QVersionNumber &other) const
{
    if (isNull() || other.isNull()) {
        qWarning("called with invalid parameters");
        return false;
    }
    switch (op) {
    case NotEqual:
        return other != number;
    case LessThan:
        return other < number;
    case LessEqualThan:
        return other <= number;
    case Equals:
        return other == number;
    case GreaterThan:
        return other > number;
    case GreaterEqualThan:
        return other >= number;
    }
    return false;
}

VersionTerm VersionTerm::fromJson(const QJsonValue &v)
{
    VersionTerm result;
    if (!v.isObject())
        return result;
    const QJsonObject o = v.toObject();
    result.number = QVersionNumber::fromString(o.value(valueKey()).toString());
    const QString opS = o.value(opKey()).toString();
    for (size_t i = 0; i < sizeof(operators) / sizeof(operators[0]); ++i) {
        if (opS == QLatin1String(operators[i])) {
            result.op = static_cast<Operator>(i);
            break;
        }
    }
    return result;
}

// OS term consisting of name and optional version found in
// under "os" in main array and in "exceptions" lists.
struct OsTypeTerm
{
    static OsTypeTerm fromJson(const QJsonValue &v);
    static QString hostOs();

    static QString hostOsRelease() {
        QString ver;

#ifdef Q_OS_WIN
        switch (QSysInfo::windowsVersion()) {
        case QSysInfo::WV_XP:
        case QSysInfo::WV_2003:
            ver = "xp";
            break;

        case QSysInfo::WV_VISTA:
            ver = "vista";
            break;

        case QSysInfo::WV_WINDOWS7:
            ver = "7";
            break;

        case QSysInfo::WV_WINDOWS8:
            ver = "8";
            break;

        case QSysInfo::WV_WINDOWS8_1:
            ver = "8.1";
            break;

        case QSysInfo::WV_WINDOWS10:
            ver = "10";
            break;

        default:
            break;
        }
#endif
        return ver;
    }

    bool isNull() const {
      return type.isEmpty();
    }

    bool matches(const QString &osName, const QString &osRelease) const
    {
        if (isNull() || osName.isEmpty()) {
            qWarning("called with invalid parameters");
            return false;
        }

        if (type != osName)
            return false;


        // release is a list of Windows versions where the rule should match
        if (! release.isEmpty() && !contains(release, osRelease))
            return false;
        return true;
    }

    QString type;
    VersionTerm versionTerm;
    QJsonArray release;
};

OsTypeTerm OsTypeTerm::fromJson(const QJsonValue &v)
{
    OsTypeTerm result;

    if (!v.isObject())
        return result;

    const QJsonObject o = v.toObject();
    result.type = o.value(typeKey()).toString();
    result.versionTerm = VersionTerm::fromJson(o.value(versionKey()));
    result.release = o.value(releaseKey()).toArray();

    return result;
}

QString OsTypeTerm::hostOs()
{
    // Determine Host OS
#if defined(Q_OS_WIN)
    return  QString("win");

#elif defined(Q_OS_LINUX)
    return QString("linux");

#elif defined(Q_OS_DARWIN)
    return  QString("macosx");

#elif defined(Q_OS_ANDROID)
    return  QString("android");

#else
    return QString();

#endif
}
} // anonymous namespace

static QString msgSyntaxWarning(const QJsonObject &object, const QString &what)
{
    QString result;

    QTextStream(&result) << "Id " << object.value(idKey()).toInt()
        << " (\"" << object.value(descriptionKey()).toString()
        << "\"): " << what;
    return result;
}

// Check whether an entry matches. Called recursively for
// "exceptions" list.

static bool matches(const QJsonObject &object,
                    const QString &osName,
                    const QString &osRelease,
                    const QOpenGLConfig::Gpu &gpu)
{
    const OsTypeTerm os = OsTypeTerm::fromJson(object.value(osKey()));
    if (!os.isNull() && ! os.matches(osName, osRelease))
        return false;

    const QJsonValue exceptionsV = object.value(exceptionsKey());
    if (exceptionsV.isArray()) {
        const QJsonArray exceptionsA = exceptionsV.toArray();
        for (auto it = exceptionsA.constBegin(), cend = exceptionsA.constEnd(); it != cend; ++it) {
            if (matches(it->toObject(), osName, osRelease, gpu))
                return false;
        }
    }

    const QJsonValue vendorV = object.value(vendorIdKey());
    if (vendorV.isString()) {
        if (gpu.vendorId != vendorV.toString().toInteger<unsigned int>(nullptr, /* base */ 0))
            return false;
    } else {
        if (object.contains(glVendorKey())) {
            const QByteArray glVendorV = object.value(glVendorKey()).toString().toUtf8();
            if (!gpu.glVendor.contains(glVendorV))
                return false;
        }
    }

    if (gpu.deviceId) {
        const QJsonValue deviceIdV = object.value(deviceIdKey());
        switch (deviceIdV.type()) {
        case QJsonValue::Array:
            if (!contains(deviceIdV.toArray(), gpu.deviceId))
                return false;
            break;
        case QJsonValue::Undefined:
        case QJsonValue::Null:
            break;
        default:
            qWarning().noquote()
                << msgSyntaxWarning(object,
                                    QLatin1String("Device ID must be of type array."));
        }
    }
    if (!gpu.driverVersion.isNull()) {
        const QJsonValue driverVersionV = object.value(driverVersionKey());
        switch (driverVersionV.type()) {
        case QJsonValue::Object:
            if (!VersionTerm::fromJson(driverVersionV).matches(gpu.driverVersion))
                return false;
            break;
        case QJsonValue::Undefined:
        case QJsonValue::Null:
            break;
        default:
            qWarning().noquote()
                << msgSyntaxWarning(object,
                                    QLatin1String("Driver version must be of type object."));
        }
    }

    if (!gpu.driverDescription.isEmpty()) {
        const QJsonValue driverDescriptionV = object.value(driverDescriptionKey());
        if (driverDescriptionV.isString()) {
            if (!gpu.driverDescription.contains(driverDescriptionV.toString().toUtf8()))
                return false;
        }
    }

    return true;
}

static bool readGpuFeatures(const QOpenGLConfig::Gpu &gpu, const QString &osName,
                  const QString &osRelease, const QJsonDocument &doc,
                  QSet<QString> *result, QString *errorMessage)
{
    result->clear();
    errorMessage->clear();
    const QJsonValue entriesV = doc.object().value(QString("entries"));

    if (!entriesV.isArray()) {
        *errorMessage = QLatin1String("No entries read.");
        return false;
    }

    const QJsonArray entriesA = entriesV.toArray();
    for (auto eit = entriesA.constBegin(), ecend = entriesA.constEnd(); eit != ecend; ++eit) {
        if (eit->isObject()) {
            const QJsonObject object = eit->toObject();
            if (matches(object, osName, osRelease, gpu)) {
                const QJsonValue featuresListV = object.value(featuresKey());
                if (featuresListV.isArray()) {
                    const QJsonArray featuresListA = featuresListV.toArray();
                    for (auto fit = featuresListA.constBegin(), fcend = featuresListA.constEnd(); fit != fcend; ++fit)
                        result->insert(fit->toString());
                }
            }
        }
    }
    return true;
}

static bool readGpuFeatures(const QOpenGLConfig::Gpu &gpu,
                            const QString &osName,
                            const QString &osRelease,
                            const QByteArray &jsonAsciiData,
                            QSet<QString> *result, QString *errorMessage)
{
    result->clear();
    errorMessage->clear();
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(jsonAsciiData, &error);
    if (document.isNull()) {
        const int lineNumber = 1 + jsonAsciiData.left(error.offset).count('\n');
        QTextStream str(errorMessage);
        str << "Failed to parse data: \"" << error.errorString()
            << "\" at line " << lineNumber << " (offset: "
            << error.offset << ").";
        return false;
    }
    return readGpuFeatures(gpu, osName, osRelease, document, result, errorMessage);
}

static bool readGpuFeatures(const QOpenGLConfig::Gpu &gpu,
                            const QString &osName,
                            const QString &osRelease,
                            const QString &fileName,
                            QSet<QString> *result, QString *errorMessage)
{
    result->clear();
    errorMessage->clear();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QTextStream str(errorMessage);
        str << "Cannot open \"" << QDir::toNativeSeparators(fileName) << "\": "
            << file.errorString();
        return false;
    }
    const bool success = readGpuFeatures(gpu, osName, osRelease, file.readAll(), result, errorMessage);
    if (!success) {
        errorMessage->prepend(QLatin1String("Error reading \"")
                              + QDir::toNativeSeparators(fileName)
                              + QLatin1String("\": "));
    }
    return success;
}

QSet<QString> QOpenGLConfig::gpuFeatures(const QOpenGLConfig::Gpu &gpu,
                                         const QString &osName,
                                         const QString &osRelease,
                                         const QJsonDocument &doc)
{
    QSet<QString> result;
    QString errorMessage;
    if (!readGpuFeatures(gpu, osName, osRelease, doc, &result, &errorMessage))
        qWarning().noquote() << errorMessage;
    return result;
}

QSet<QString> QOpenGLConfig::gpuFeatures(const QOpenGLConfig::Gpu &gpu,
                                         const QString &osName,
                                         const QString &osRelease,
                                         const QString &fileName)
{
    QSet<QString> result;
    QString errorMessage;
    if (!readGpuFeatures(gpu, osName, osRelease, fileName, &result, &errorMessage))
        qWarning().noquote() << errorMessage;
    return result;
}

QSet<QString> QOpenGLConfig::gpuFeatures(const Gpu &gpu, const QJsonDocument &doc)
{
    return gpuFeatures(gpu, OsTypeTerm::hostOs(), OsTypeTerm::hostOsRelease(), doc);
}

QSet<QString> QOpenGLConfig::gpuFeatures(const Gpu &gpu, const QString &fileName)
{
    return gpuFeatures(gpu, OsTypeTerm::hostOs(), OsTypeTerm::hostOsRelease(), fileName);
}

QOpenGLConfig::Gpu QOpenGLConfig::Gpu::fromContext()
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    QScopedPointer<QOpenGLContext> tmpContext;
    QScopedPointer<QOffscreenSurface> tmpSurface;
    if (!ctx) {
        tmpContext.reset(new QOpenGLContext);
        if (!tmpContext->create()) {
            qWarning("QOpenGLConfig::Gpu::fromContext: Failed to create temporary context");
            return QOpenGLConfig::Gpu();
        }
        tmpSurface.reset(new QOffscreenSurface);
        tmpSurface->setFormat(tmpContext->format());
        tmpSurface->create();
        tmpContext->makeCurrent(tmpSurface.data());
    }

    QOpenGLConfig::Gpu gpu;
    ctx = QOpenGLContext::currentContext();
    const GLubyte *p = ctx->functions()->glGetString(GL_VENDOR);
    if (p)
        gpu.glVendor = QByteArray(reinterpret_cast<const char *>(p));

    return gpu;
}

Q_GUI_EXPORT std::set<QByteArray> *qgpu_features(const QString &filename)
{
    const QSet<QString> features = QOpenGLConfig::gpuFeatures(QOpenGLConfig::Gpu::fromContext(), filename);
    std::set<QByteArray> *result = new std::set<QByteArray>;

    for (const QString &feature : features) {
        result->insert(feature.toUtf8());
    }

    return result;
}


