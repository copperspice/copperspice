/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the ActiveQt framework of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qaxbindable.h"
#include "qaxfactory.h"

#ifndef QT_NO_WIN_ACTIVEQT

#include <qapplication.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qmetaobject.h>
#include <qsettings.h>
#include <qvariant.h>
#include <qtextstream.h>

#include <qt_windows.h>
#include <olectl.h>

QT_BEGIN_NAMESPACE

#define Q_REQUIRED_RPCNDR_H_VERSION 475

// Some global variables to store module information
bool qAxIsServer = false;
HANDLE qAxInstance = 0;
ITypeLib *qAxTypeLibrary = 0;
wchar_t qAxModuleFilename[MAX_PATH];
bool qAxOutProcServer = false;

// The QAxFactory instance
static QAxFactory* qax_factory = 0;
extern CLSID CLSID_QRect;
extern CLSID CLSID_QSize;
extern CLSID CLSID_QPoint;
extern void qax_shutDown();
extern bool qax_ownQApp;


extern QAxFactory *qax_instantiate();

QAxFactory *qAxFactory()
{
    if (!qax_factory) {
        bool hadQApp = qApp != 0;
        qax_factory = qax_instantiate();
        // QAxFactory created a QApplication
        if (!hadQApp && qApp)
            qax_ownQApp = true;

        // register all types with metatype system as pointers
        QStringList keys(qax_factory->featureList());
        for (int i = 0; i < keys.count(); ++i) {
            QString key(keys.at(i));
            qRegisterMetaType((key + QLatin1Char('*')).toLatin1(), (void**)(quintptr)-1);
        }
    }
    return qax_factory;
}

// Some local variables to handle module lifetime
static unsigned long qAxModuleRef = 0;
static CRITICAL_SECTION qAxModuleSection;


/////////////////////////////////////////////////////////////////////////////
// Server control
/////////////////////////////////////////////////////////////////////////////

static int initCount = 0;

QString qAxInit()
{
    static QString libFile;

    if (initCount++)
        return libFile;

    InitializeCriticalSection(&qAxModuleSection);

    libFile = QString::fromWCharArray(qAxModuleFilename);
    libFile = libFile.toLower();
    if (LoadTypeLibEx((wchar_t*)libFile.utf16(), REGKIND_NONE, &qAxTypeLibrary) == S_OK)
        return libFile;

    int lastDot = libFile.lastIndexOf(QLatin1Char('.'));
    libFile = libFile.left(lastDot) + QLatin1String(".tlb");
    if (LoadTypeLibEx((wchar_t*)libFile.utf16(), REGKIND_NONE, &qAxTypeLibrary) == S_OK)
        return libFile;

    lastDot = libFile.lastIndexOf(QLatin1Char('.'));
    libFile = libFile.left(lastDot) + QLatin1String(".olb");
    if (LoadTypeLibEx((wchar_t*)libFile.utf16(), REGKIND_NONE, &qAxTypeLibrary) == S_OK)
        return libFile;

    libFile = QString();
    return libFile;
}

void qAxCleanup()
{
    if (!initCount)
        qWarning("qAxInit/qAxCleanup mismatch");

    if (--initCount)
        return;

    delete qax_factory;
    qax_factory = 0;

    if (qAxTypeLibrary) {
        qAxTypeLibrary->Release();
        qAxTypeLibrary = 0;
    }

    DeleteCriticalSection(&qAxModuleSection);
}

unsigned long qAxLock()
{
    EnterCriticalSection(&qAxModuleSection);
    unsigned long ref = ++qAxModuleRef;
    LeaveCriticalSection(&qAxModuleSection);
    return ref;
}

unsigned long qAxUnlock()
{
    if (!initCount) // cleaned up already
        return 0;

    EnterCriticalSection(&qAxModuleSection);
    unsigned long ref = --qAxModuleRef;
    LeaveCriticalSection(&qAxModuleSection);

    if (!ref)
        qax_shutDown();
    return ref;
}

unsigned long qAxLockCount()
{
    return qAxModuleRef;
}

/////////////////////////////////////////////////////////////////////////////
// Registry
/////////////////////////////////////////////////////////////////////////////

extern bool qax_disable_inplaceframe;

QString qax_clean_type(const QString &type, const QMetaObject *mo)
{
    if (mo) {
        int classInfoIdx = mo->indexOfClassInfo("CoClassAlias");
        if (classInfoIdx != -1) {
            const QMetaClassInfo classInfo = mo->classInfo(classInfoIdx);
            return QLatin1String(classInfo.value());
        }
    }

    QString alias(type);
    alias.remove(QLatin1String("::"));
    return alias;
}

// (Un)Register the ActiveX server in the registry.
// The QAxFactory implementation provides the information.
HRESULT UpdateRegistry(BOOL bRegister)
{
    qAxIsServer = false;
    QString file = QString::fromWCharArray(qAxModuleFilename);
    QString path = file.left(file.lastIndexOf(QLatin1Char('\\'))+1);
    QString module = file.right(file.length() - path.length());
    module = module.left(module.lastIndexOf(QLatin1Char('.')));

    const QString appId = qAxFactory()->appID().toString().toUpper();
    const QString libId = qAxFactory()->typeLibID().toString().toUpper();

    QString libFile = qAxInit();
    QString typeLibVersion;

    TLIBATTR *libAttr = 0;
    if (qAxTypeLibrary)
        qAxTypeLibrary->GetLibAttr(&libAttr);
    if (!libAttr)
        return SELFREG_E_TYPELIB;

    DWORD major = libAttr->wMajorVerNum;
    DWORD minor = libAttr->wMinorVerNum;
    typeLibVersion = QString::number((uint)major) + QLatin1Char('.') + QString::number((uint)minor);

    if (bRegister)
        RegisterTypeLib(qAxTypeLibrary, (wchar_t*)libFile.utf16(), 0);
    else
        UnRegisterTypeLib(libAttr->guid, libAttr->wMajorVerNum, libAttr->wMinorVerNum, libAttr->lcid, libAttr->syskind);

    qAxTypeLibrary->ReleaseTLibAttr(libAttr);

    if (typeLibVersion.isEmpty())
        typeLibVersion = QLatin1String("1.0");

    // check whether the user has permission to write to HKLM\Software\Classes
    // if not, use HKCU\Software\Classes
    QString keyPath(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes"));
    QSettings test(keyPath, QSettings::NativeFormat);
    if (!test.isWritable())
        keyPath = QLatin1String("HKEY_CURRENT_USER\\Software\\Classes");

    QSettings settings(keyPath, QSettings::NativeFormat);

    // we try to create the ActiveX widgets later on...
    bool delete_qApp = false;
    if (!qApp) {
        int argc = 0;
        (void)new QApplication(argc, 0);
        delete_qApp = true;
    }

    if (bRegister) {
        if (qAxOutProcServer) {
            settings.setValue(QLatin1String("/AppID/") + appId + QLatin1String("/."), module);
            settings.setValue(QLatin1String("/AppID/") + module + QLatin1String(".EXE/AppID"), appId);
        }

        QStringList keys = qAxFactory()->featureList();
        for (QStringList::Iterator key = keys.begin(); key != keys.end(); ++key) {
            QString className = *key;
            QObject *object = qAxFactory()->createObject(className);
            const QMetaObject *mo = qAxFactory()->metaObject(className);
            const QString classId = qAxFactory()->classID(className).toString().toUpper();

            className = qax_clean_type(className, mo);

            if (object) { // don't register subobject classes
                QString classVersion = mo ? QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("Version")).value()) : QString();
                if (classVersion.isNull())
                    classVersion = QLatin1String("1.0");
                bool insertable = mo && !qstricmp(mo->classInfo(mo->indexOfClassInfo("Insertable")).value(), "yes");
                bool control = object->isWidgetType();
                const QString classMajorVersion = classVersion.left(classVersion.indexOf(QLatin1Char('.')));
                uint olemisc = OLEMISC_SETCLIENTSITEFIRST
                    |OLEMISC_ACTIVATEWHENVISIBLE
                    |OLEMISC_INSIDEOUT
                    |OLEMISC_CANTLINKINSIDE
                    |OLEMISC_RECOMPOSEONRESIZE;
                if (!control)
                    olemisc |= OLEMISC_INVISIBLEATRUNTIME;
                else if (object->findChild<QMenuBar*>() && !qax_disable_inplaceframe)
                    olemisc |= OLEMISC_WANTSTOMENUMERGE;

                settings.setValue(QLatin1Char('/') + module + QLatin1Char('.') + className + QLatin1Char('.') + classMajorVersion + QLatin1String("/."), className + QLatin1String(" Class"));
                settings.setValue(QLatin1Char('/') + module + QLatin1Char('.') + className + QLatin1Char('.') + classMajorVersion + QLatin1String("/CLSID/."), classId);
                if (insertable)
                    settings.setValue(QLatin1Char('/') + module + QLatin1Char('.') + className + QLatin1Char('.') + classMajorVersion + QLatin1String("/Insertable/."), QVariant(QLatin1String("")));

                settings.setValue(QLatin1Char('/') + module + QLatin1Char('.') + className + QLatin1String("/."), className + QLatin1String(" Class"));
                settings.setValue(QLatin1Char('/') + module + QLatin1Char('.') + className + QLatin1String("/CLSID/."), classId);
                settings.setValue(QLatin1Char('/') + module + QLatin1Char('.') + className + QLatin1String("/CurVer/."), module + QLatin1Char('.') + className + QLatin1Char('.') + classMajorVersion);

                settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/."), className + QLatin1String(" Class"));
                if (file.endsWith(QLatin1String("exe"), Qt::CaseInsensitive))
                    settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/AppID"), appId);
                if (control)
                    settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/Control/."), QVariant(QLatin1String("")));
                if (insertable)
                    settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/Insertable/."), QVariant(QLatin1String("")));
                if (file.right(3).toLower() == QLatin1String("dll"))
                    settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/InProcServer32/."), file);
                else
                    settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/LocalServer32/."),
                                      QLatin1Char('\"') + file + QLatin1String("\" -activex"));
                settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/MiscStatus/."), control ? QLatin1String("1") : QLatin1String("0"));
                settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/MiscStatus/1/."), QString::number(olemisc));
                settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/Programmable/."), QVariant(QLatin1String("")));
                settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/ToolboxBitmap32/."), QLatin1Char('\"') +
                                  file + QLatin1String("\", 101"));
                settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/TypeLib/."), libId); settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/Version/."), classVersion);
                settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/VersionIndependentProgID/."), module + QLatin1Char('.') + className);
                settings.setValue(QLatin1String("/CLSID/") + classId + QLatin1String("/ProgID/."), module + QLatin1Char('.') + className + QLatin1Char('.') + classVersion.left(classVersion.indexOf(QLatin1Char('.'))));

                QString mime = QLatin1String(mo->classInfo(mo->indexOfClassInfo("MIME")).value());
                if (!mime.isEmpty()) {
                    QStringList mimeTypes = mime.split(QLatin1Char(';'));
                    for (int m = 0; m < mimeTypes.count(); ++m) {
                        mime = mimeTypes.at(m);
                        if (mime.isEmpty())
                            continue;
                        QString extension;
                        while (mime.contains(QLatin1Char(':'))) {
                            extension = mime.mid(mime.lastIndexOf(QLatin1Char(':')) + 1);
                            mime = mime.left(mime.length() - extension.length() - 1);
                            // Prepend '.' before extension, if required.
                            extension = extension.trimmed();
                            if (extension[0] != QLatin1Char('.'))
                                extension = QLatin1Char('.') + extension;
                        }

                        if (!extension.isEmpty()) {
                            settings.setValue(QLatin1Char('/') + extension + QLatin1String("/."), module + QLatin1Char('.') + className);
                            settings.setValue(QLatin1Char('/') + extension + QLatin1String("/Content Type"), mime);

                            mime = mime.replace(QLatin1Char('/'), QLatin1Char('\\'));
                            settings.setValue(QLatin1String("/MIME/Database/Content Type/") + mime + QLatin1String("/CLSID"), classId);
                            settings.setValue(QLatin1String("/MIME/Database/Content Type/") + mime + QLatin1String("/Extension"), extension);
                        }
                    }
                }

                delete object;
            }

            qAxFactory()->registerClass(*key, &settings);
        }
    } else {
        if (qAxOutProcServer) {
            settings.remove(QLatin1String("/AppID/") + appId + QLatin1String("/."));
            settings.remove(QLatin1String("/AppID/") + module + QLatin1String(".EXE"));
        }
        QStringList keys = qAxFactory()->featureList();
        for (QStringList::Iterator key = keys.begin(); key != keys.end(); ++key) {
            QString className = *key;
            const QMetaObject *mo = qAxFactory()->metaObject(className);
            const QString classId = qAxFactory()->classID(className).toString().toUpper();
            className = qax_clean_type(className, mo);

            QString classVersion = mo ? QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("Version")).value()) : QString();
            if (classVersion.isNull())
                classVersion = QLatin1String("1.0");
            const QString classMajorVersion = classVersion.left(classVersion.indexOf(QLatin1Char('.')));

            qAxFactory()->unregisterClass(*key, &settings);

            settings.remove(QLatin1Char('/') + module + QLatin1Char('.') + className + QLatin1Char('.') + classMajorVersion + QLatin1String("/CLSID/."));
            settings.remove(QLatin1Char('/') + module + QLatin1Char('.') + className + QLatin1Char('.') + classMajorVersion + QLatin1String("/Insertable/."));
            settings.remove(QLatin1Char('/') + module + QLatin1Char('.') + className + QLatin1Char('.') + classMajorVersion + QLatin1String("/."));
            settings.remove(QLatin1Char('/') + module + QLatin1Char('.') + className + QLatin1Char('.') + classMajorVersion);

            settings.remove(QLatin1Char('/') + module + QLatin1Char('.') + className + QLatin1String("/CLSID/."));
            settings.remove(QLatin1Char('/') + module + QLatin1Char('.') + className + QLatin1String("/CurVer/."));
            settings.remove(QLatin1Char('/') + module + QLatin1Char('.') + className + QLatin1String("/."));
            settings.remove(QLatin1Char('/') + module + QLatin1Char('.') + className);

            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/AppID"));
            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/Control/."));
            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/Insertable/."));
            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/InProcServer32/."));
            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/LocalServer32/."));
            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/MiscStatus/1/."));
            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/MiscStatus/."));	
            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/Programmable/."));
            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/ToolboxBitmap32/."));
            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/TypeLib/."));
            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/Version/."));
            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/VersionIndependentProgID/."));
            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/ProgID/."));
            settings.remove(QLatin1String("/CLSID/") + classId + QLatin1String("/."));
            settings.remove(QLatin1String("/CLSID/") + classId);

            QString mime = QLatin1String(mo->classInfo(mo->indexOfClassInfo("MIME")).value());
            if (!mime.isEmpty()) {
                QStringList mimeTypes = mime.split(QLatin1Char(';'));
                for (int m = 0; m < mimeTypes.count(); ++m) {
                    mime = mimeTypes.at(m);
                    if (mime.isEmpty())
                        continue;
                    QString extension;
                    while (mime.contains(QLatin1Char(':'))) {
                        extension = mime.mid(mime.lastIndexOf(QLatin1Char(':')) + 1);
                        mime = mime.left(mime.length() - extension.length() - 1);
                        // Prepend '.' before extension, if required.
                        extension = extension.trimmed();
                        if (extension[0] != QLatin1Char('.'))
                            extension.prepend(QLatin1Char('.'));
                    }
                    if (!extension.isEmpty()) {
                        settings.remove(QLatin1Char('/') + extension + QLatin1String("/Content Type"));
                        settings.remove(QLatin1Char('/') + extension + QLatin1String("/."));
                        settings.remove(QLatin1Char('/') + extension);
                        mime.replace(QLatin1Char('/'), QLatin1Char('\\'));
                        settings.remove(QLatin1String("/MIME/Database/Content Type/") + mime + QLatin1String("/Extension"));
                        settings.remove(QLatin1String("/MIME/Database/Content Type/") + mime + QLatin1String("/CLSID"));
                        settings.remove(QLatin1String("/MIME/Database/Content Type/") + mime + QLatin1String("/."));
                        settings.remove(QLatin1String("/MIME/Database/Content Type/") + mime);
                    }
                }
            }
        }
    }

    if (delete_qApp)
        delete qApp;

    qAxCleanup();
    if (settings.status() == QSettings::NoError)
        return S_OK;
    return SELFREG_E_CLASS;
}

/////////////////////////////////////////////////////////////////////////////
// IDL generator
/////////////////////////////////////////////////////////////////////////////

static QList<QByteArray> enums;
static QList<QByteArray> enumValues;
static QList<QByteArray> subtypes;

static const char* const type_map[][2] =
{
    // QVariant/Qt Value data types
    { "QString",	   "BSTR" },
    { "QCString",	   "BSTR" },
    { "bool",	   	"VARIANT_BOOL" },
    { "int",	   	"int" },
    { "uint",	   	"unsigned int" },
    { "double",	   "double" },
    { "QColor",	   "OLE_COLOR" },
    { "QDate",	   	"DATE" },
    { "QTime",		   "DATE" },
    { "QDateTime",	"DATE" },
    { "QFont",	   	"IFontDisp*" },
    { "QPixmap",	   "IPictureDisp*" },
    { "QVariant",	   "VARIANT" },
    { "QVariantList",	 "SAFEARRAY(VARIANT)" },
    { "QList<QVariant>", "SAFEARRAY(VARIANT)" },
    { "quint64",	   "CY" },
    { "qint64",	   "CY" },
    { "QByteArray",	"SAFEARRAY(BYTE)" },
    { "QStringList",	"SAFEARRAY(BSTR)" },

    // Userdefined datatypes - some not on Borland though
    { "QCursor",         "enum MousePointer" },
    { "Qt::FocusPolicy", "enum FocusPolicy" },

# if __REQUIRED_RPCNDR_H_VERSION__ >= Q_REQUIRED_RPCNDR_H_VERSION
    { "QRect",		"struct QRect" },
    { "QSize",		"struct QSize" },
    { "QPoint",	"struct QPoint" },
# endif

    // And we support COM data types
    { "BOOL",		   "BOOL" },
    { "BSTR",		   "BSTR" },
    { "OLE_COLOR",	"OLE_COLOR" },
    { "DATE",		   "DATE" },
    { "VARIANT",	   "VARIANT" },
    { "IDispatch",	"IDispatch*" },
    { "IUnknown",	   "IUnknown*" },
    { "IDispatch*",	"IDispatch*" },
    { "IUnknown*",	"IUnknown*" },
    { 0,		0 }
};

static QByteArray convertTypes(const QByteArray &qtype, bool *ok)
{
    qRegisterMetaType("IDispatch*", (void**)0);
    qRegisterMetaType("IUnknown*", (void**)0);

    *ok = false;

    int i = 0;
    while (type_map[i][0]) {
        if (qtype == type_map[i][0] && type_map[i][1]) {
            *ok = true;
            return type_map[i][1];	
        }
        ++i;
    }
    if (enums.contains(qtype)) {
        *ok = true;
        return "enum " + qtype;
    }
    if (subtypes.contains(qtype)) {
        *ok = true;
    } else if (qtype.endsWith('*')) {
        QByteArray cleanType = qtype.left(qtype.length() - 1);
        const QMetaObject *mo = qAxFactory()->metaObject(QString::fromLatin1(cleanType.constData()));
        if (mo) {
            cleanType = qax_clean_type(QString::fromLatin1(cleanType), mo).toLatin1();
            if (subtypes.contains(cleanType)) {
                *ok = true;
                return cleanType + '*';
            }
        }
    }
    return qtype;
}

static const char* const keyword_map[][2] =
{
    { "aggregatable",	"aggregating"	    },
    { "allocate",	"alloc"		    },
    { "appobject",	"appObject"	    },
    { "arrays",		"array"		    },
    { "async",		"asynchronous"	    },
    { "bindable",	"binding"	    },
    { "Boolean",	"boolval"	    },
    { "boolean",	"boolval"	    },
    { "broadcast",	"broadCast"	    },
    { "callback",	"callBack"	    },
    { "decode",		"deCode"	    },
    { "default",	"defaulted"	    },
    { "defaultbind",	"defaultBind"	    },
    { "defaultvalue",	"defaultValue"	    },
    { "encode"		"enCode"	    },
    { "endpoint",	"endPoint"	    },
    { "hidden",		"isHidden"	    },
    { "ignore",		"ignore_"	    },
    { "local",		"local_"	    },
    { "notify",		"notify_"	    },
    { "object",		"object_"	    },
    { "optimize",	"optimize_"	    },
    { "optional",	"optional_"	    },
    { "out",		"out_"		    },
    { "pipe",		"pipe_"		    },
    { "proxy",		"proxy_"	    },
    { "ptr",		"pointer"	    },
    { "readonly",	"readOnly"	    },
    { "small",		"small_"	    },
    { "source",		"source_"	    },
    { "string",		"string_"	    },
    { "uuid",		"uuid_"		    },
    { 0,		0		    }
};

static QByteArray replaceKeyword(const QByteArray &name)
{
    int i = 0;
    while (keyword_map[i][0]) {
        if (name == keyword_map[i][0] && keyword_map[i][1])
            return keyword_map[i][1];
        ++i;
    }
    return name;
}

static QMap<QByteArray, int> mapping;

static QByteArray renameOverloads(const QByteArray &name)
{
    QByteArray newName = name;

    int n = mapping.value(name);
    if (mapping.contains(name)) {
        int n = mapping.value(name);
        newName = name + '_' + QByteArray::number(n);
        mapping.insert(name, n+1);
    } else {
        mapping.insert(name, 1);
    }

    return newName;
}

// filter out some properties
static const char* const ignore_props[] =
{
    "name",
    "objectName",
    "isTopLevel",
    "isDialog",
    "isModal",
    "isPopup",
    "isDesktop",
    "geometry",
    "pos",
    "frameSize",
    "frameGeometry",
    "size",
    "sizeHint",
    "minimumSizeHint",
    "microFocusHint",
    "rect",
    "childrenRect",
    "childrenRegion",
    "minimumSize",
    "maximumSize",
    "sizeIncrement",
    "baseSize",
    "ownPalette",
    "ownFont",
    "ownCursor",
    "visibleRect",
    "isActiveWindow",
    "underMouse",
    "visible",
    "hidden",
    "minimized",
    "focus",
    "focusEnabled",
    "customWhatsThis",
    "shown",
    "windowOpacity",
    0
};

// filter out some slots
static const char* const ignore_slots[] =
{
    "deleteLater",
    "setMouseTracking",
    "update",
    "repaint",
    "iconify",
    "showMinimized",
    "showMaximized",
    "showFullScreen",
    "showNormal",
    "polish",
    "constPolish",
    "stackUnder",
    "setShown",
    "setHidden",
    "move_1",
    "resize_1",
    "setGeometry_1",
    0
};

static bool ignore(const char *test, const char *const *table)
{
    if (!test)
        return true;
    int i = 0;
    while (table[i]) {
        if (!strcmp(test, table[i]))
            return true;
        ++i;
    }
    return false;
}

bool ignoreSlots(const char *test)
{
    return ignore(test, ignore_slots);
}

bool ignoreProps(const char *test)
{
    return ignore(test, ignore_props);
}

#define STRIPCB(x) x = x.mid(1, x.length()-2)

static QByteArray prototype(const QList<QByteArray> &parameterTypes, const QList<QByteArray> &parameterNames, bool *ok)
{
    QByteArray prototype;

    for (int p = 0; p < parameterTypes.count() && *ok; ++p) {
        bool out = false;
        QByteArray type(parameterTypes.at(p));
        QByteArray name(parameterNames.at(p));

        if (type.endsWith('&')) {
            out = true;
            type.truncate(type.length() - 1);
        } else if (type.endsWith("**")) {
            out = true;
            type.truncate(type.length() - 1);
        } else if (type.endsWith('*') && !subtypes.contains(type)) {
            type.truncate(type.length() - 1);
        }
        if (type.isEmpty()) {
            *ok = false;
            break;
        }
        type = convertTypes(type, ok);
        if (!out)
            prototype += "[in] " + type + ' ';
        else
            prototype += "[in,out] " + type + ' ';

        if (out)
            prototype += '*';
        if (name.isEmpty())
            prototype += 'p' + QByteArray::number(p);
        else
            prototype += "p_" + replaceKeyword(name);

        if (p < parameterTypes.count() - 1)
            prototype += ", ";
    }

    return prototype;
}

static QByteArray addDefaultArguments(const QByteArray &prototype, int numDefArgs)
{
    // nothing to do, or unsupported anyway
    if (!numDefArgs || prototype.contains("/**"))
        return prototype;

    QByteArray ptype(prototype);
    int in = -1;
    while (numDefArgs) {
        in = ptype.lastIndexOf(']', in);
        ptype.replace(in, 1, ",optional]");
        in = ptype.indexOf(' ', in) + 1;
        QByteArray type = ptype.mid(in, ptype.indexOf(' ', in) - in);
        if (type == "enum")
            type += ' ' + ptype.mid(in + 5, ptype.indexOf(' ', in + 5) - in - 5);
        ptype.replace(in, type.length(), QByteArray("VARIANT /*was: ") + type + "*/");
        --numDefArgs;
    }

    return ptype;
}

static HRESULT classIDL(QObject *o, const QMetaObject *mo, const QString &className, bool isBindable, QTextStream &out)
{
    int id = 1;
    int i = 0;
    if (!mo)
        return 3;

    QString topclass = qAxFactory()->exposeToSuperClass(className);
    if (topclass.isEmpty())
        topclass = QLatin1String("QObject");
    bool hasStockEvents = qAxFactory()->hasStockEvents(className);

    const QMetaObject *pmo = mo;
    do {
        pmo = pmo->superClass();
    } while (pmo && topclass != QString::fromLatin1(pmo->className()));

    int enumoff = pmo ? pmo->enumeratorOffset() : mo->enumeratorOffset();
    int methodoff = pmo ? pmo->methodOffset() : mo->methodOffset();
    int propoff = pmo ? pmo->propertyOffset() : mo->propertyOffset();

    int qtProps = 0;
    int qtSlots = 0;

    bool control = false;

    if (o && o->isWidgetType()) {
        qtProps = QWidget::staticMetaObject.propertyCount();
        qtSlots = QWidget::staticMetaObject.methodCount();
        control = true;
    }

    QString classID = qAxFactory()->classID(className).toString().toUpper();
    if (QUuid(classID).isNull())
        return 4;
    STRIPCB(classID);
    QString interfaceID = qAxFactory()->interfaceID(className).toString().toUpper();
    if (QUuid(interfaceID).isNull())
        return 5;
    STRIPCB(interfaceID);
    QString eventsID = qAxFactory()->eventsID(className).toString().toUpper();
    bool hasEvents = !QUuid(eventsID).isNull();
    STRIPCB(eventsID);

    QString cleanClassName = qax_clean_type(className, mo);
    QString defProp(QLatin1String(mo->classInfo(mo->indexOfClassInfo("DefaultProperty")).value()));
    QString defSignal(QLatin1String(mo->classInfo(mo->indexOfClassInfo("DefaultSignal")).value()));

    for (i = enumoff; i < mo->enumeratorCount(); ++i) {
        const QMetaEnum enumerator = mo->enumerator(i);
        if (enums.contains(enumerator.name()))
            continue;

        enums.append(enumerator.name());

        out << "\tenum " << enumerator.name() << " {" << endl;

        for (int j = 0; j < enumerator.keyCount(); ++j) {
            QByteArray key(enumerator.key(j));
            while (enumValues.contains(key)) {
                key += '_';
            }
            enumValues.append(key);
            uint value = (uint)enumerator.value(j);
            key = key.leftJustified(20);
            out << "\t\t" << key << "\t= ";
            if (enumerator.isFlag())
                out << "0x" << QByteArray::number(value, 16).rightJustified(8, '0');
            else
                out << value;
            if (j < enumerator.keyCount()-1)
                out << ", ";
            out << endl;
        }
        out << "\t};" << endl << endl;
    }

    // mouse cursor enum for QCursor support
    if (!enums.contains("MousePointer")) {
        enums.append("MousePointer");
        out << "\tenum MousePointer {" << endl;
        out << "\t\tArrowCursor             = " << Qt::ArrowCursor << ',' << endl;
        out << "\t\tUpArrowCursor           = " << Qt::UpArrowCursor << ',' << endl;
        out << "\t\tCrossCursor             = " << Qt::CrossCursor << ',' << endl;
        out << "\t\tWaitCursor              = " << Qt::WaitCursor << ',' << endl;
        out << "\t\tIBeamCursor             = " << Qt::IBeamCursor << ',' << endl;
        out << "\t\tSizeVerCursor           = " << Qt::SizeVerCursor << ',' << endl;
        out << "\t\tSizeHorCursor           = " << Qt::SizeHorCursor << ',' << endl;
        out << "\t\tSizeBDiagCursor         = " << Qt::SizeBDiagCursor << ',' << endl;
        out << "\t\tSizeFDiagCursor         = " << Qt::SizeFDiagCursor << ',' << endl;
        out << "\t\tSizeAllCursor           = " << Qt::SizeAllCursor << ',' << endl;
        out << "\t\tBlankCursor             = " << Qt::BlankCursor << ',' << endl;
        out << "\t\tSplitVCursor            = " << Qt::SplitVCursor << ',' << endl;
        out << "\t\tSplitHCursor            = " << Qt::SplitHCursor << ',' << endl;
        out << "\t\tPointingHandCursor      = " << Qt::PointingHandCursor << ',' << endl;
        out << "\t\tForbiddenCursor         = " << Qt::ForbiddenCursor << ',' << endl;
        out << "\t\tWhatsThisCursor         = " << Qt::WhatsThisCursor << ',' << endl;
        out << "\t\tBusyCursor\t= " << Qt::BusyCursor << endl;
        out << "\t};" << endl << endl;
    }
    if (!enums.contains("FocusPolicy")) {
        enums.append("FocusPolicy");
        out << "\tenum FocusPolicy {" << endl;
        out << "\t\tNoFocus             = " << Qt::NoFocus << ',' << endl;
        out << "\t\tTabFocus            = " << Qt::TabFocus << ',' << endl;
        out << "\t\tClickFocus          = " << Qt::ClickFocus << ',' << endl;
        out << "\t\tStrongFocus         = " << Qt::StrongFocus << ',' << endl;
        out << "\t\tWheelFocus          = " << Qt::WheelFocus << endl;
        out << "\t};" << endl << endl;
    }

    out << endl;
    out << "\t[" << endl;
    out << "\t\tuuid(" << interfaceID << ")," << endl;
    out << "\t\thelpstring(\"" << cleanClassName << " Interface\")" << endl;
    out << "\t]" << endl;
    out << "\tdispinterface I" << cleanClassName  << endl;
    out << "\t{" << endl;

    out << "\tproperties:" << endl;
    for (i = propoff; i < mo->propertyCount(); ++i) {
        const QMetaProperty property = mo->property(i);
        /*	if (property.testFlags(QMetaProperty::Override))
        continue;*/
        if (i <= qtProps && ignoreProps(property.name()))
            continue;
        if (!property.name() || mo->indexOfProperty(property.name()) > i)
            continue;

        bool ok = true;
        QByteArray type(convertTypes(property.typeName(), &ok));
        QByteArray name(replaceKeyword(property.name()));

        if (!ok)
            out << "\t/****** Property is of unsupported datatype" << endl;

        out << "\t\t[id(" << id << ')';
        if (!property.isWritable())
            out << ", readonly";
        if (isBindable && property.isScriptable(o))
            out << ", bindable";
        if (!property.isDesignable(o))
            out << ", nonbrowsable";
        if (isBindable)
            out << ", requestedit";
        if (defProp == QLatin1String(name))
            out << ", uidefault";
        out << "] " << type << ' ' << name << ';' << endl;

        if (!ok)
            out << "\t******/" << endl;
        ++id;
    }
    out << endl;
    out << "\tmethods:" << endl;
    int numDefArgs = 0;
    QByteArray outBuffer;
    for (i = methodoff; i < mo->methodCount(); ++i) {
        const QMetaMethod slot = mo->method(i);
        if (slot.access() != QMetaMethod::Public || slot.methodType() == QMetaMethod::Signal)
            continue;

        if (slot.attributes() & QMetaMethod::Cloned) {
            ++numDefArgs;
            continue;
        }
        if (!outBuffer.isEmpty()) {
            outBuffer = addDefaultArguments(outBuffer, numDefArgs);
            numDefArgs = 0;
            out << outBuffer;
            outBuffer = QByteArray();
        }

        QByteArray signature(slot.signature());
        QByteArray name(signature.left(signature.indexOf('(')));
        if (i <= qtSlots && ignoreSlots(name))
            continue;

        signature = signature.mid(name.length() + 1);
        signature.truncate(signature.length() - 1);
        name = renameOverloads(replaceKeyword(name));
        if (ignoreSlots(name))
            continue;

        QList<QByteArray> parameterTypes(slot.parameterTypes());
        QList<QByteArray> parameterNames(slot.parameterNames());

        bool ok = true;
        QByteArray type = slot.typeName();
        if (type.isEmpty())
            type = "void";
        else
            type = convertTypes(type, &ok);

        QByteArray ptype(prototype(parameterTypes, parameterNames, &ok));
        if (!ok)
            outBuffer += "\t/****** Slot parameter uses unsupported datatype\n";

        outBuffer += "\t\t[id(" + QString::number(id).toLatin1() + ")] " + type + ' ' + name + '(' + ptype + ");\n";

        if (!ok)
            outBuffer += "\t******/\n";
        ++id;
    }
    if (!outBuffer.isEmpty()) {
        outBuffer = addDefaultArguments(outBuffer, numDefArgs);
        numDefArgs = 0;
        out << outBuffer;
        outBuffer = QByteArray();
    }
    out << "\t};" << endl << endl;

    mapping.clear();
    id = 1;

    if (hasEvents) {
        out << "\t[" << endl;
        out << "\t\tuuid(" << eventsID << ")," << endl;
        out << "\t\thelpstring(\"" << cleanClassName << " Events Interface\")" << endl;
        out << "\t]" << endl;
        out << "\tdispinterface I" << cleanClassName << "Events" << endl;
        out << "\t{" << endl;
        out << "\tproperties:" << endl;
        out << "\tmethods:" << endl;

        if (hasStockEvents) {
            out << "\t/****** Stock events ******/" << endl;
            out << "\t\t[id(DISPID_CLICK)] void Click();" << endl;
            out << "\t\t[id(DISPID_DBLCLICK)] void DblClick();" << endl;
            out << "\t\t[id(DISPID_KEYDOWN)] void KeyDown(short* KeyCode, short Shift);" << endl;
            out << "\t\t[id(DISPID_KEYPRESS)] void KeyPress(short* KeyAscii);" << endl;
            out << "\t\t[id(DISPID_KEYUP)] void KeyUp(short* KeyCode, short Shift);" << endl;
            out << "\t\t[id(DISPID_MOUSEDOWN)] void MouseDown(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);" << endl;
            out << "\t\t[id(DISPID_MOUSEMOVE)] void MouseMove(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);" << endl;
            out << "\t\t[id(DISPID_MOUSEUP)] void MouseUp(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);" << endl << endl;
        }

        for (i = methodoff; i < mo->methodCount(); ++i) {
            const QMetaMethod signal = mo->method(i);
            if (signal.methodType() != QMetaMethod::Signal)
                continue;

            QByteArray signature(signal.signature());
            QByteArray name(signature.left(signature.indexOf('(')));
            signature = signature.mid(name.length() + 1);
            signature.truncate(signature.length() - 1);

            QList<QByteArray> parameterTypes(signal.parameterTypes());
            QList<QByteArray> parameterNames(signal.parameterNames());

            bool isDefault = defSignal == QLatin1String(name);
            name = renameOverloads(replaceKeyword(name));
            bool ok = true;

            QByteArray type = signal.typeName();
            if (!type.isEmpty()) // signals with return value not supported
                continue;

            QByteArray ptype(prototype(parameterTypes, parameterNames, &ok));
            if (!ok)
                out << "\t/****** Signal parameter uses unsupported datatype" << endl;

            out << "\t\t[id(" << id << ')';
            if (isDefault)
                out << ", uidefault";
            out << "] void " << name << '(' << ptype << ");" << endl;

            if (!ok)
                out << "\t******/" << endl;
            ++id;
        }
        out << "\t};" << endl << endl;
    }

    out << "\t[" << endl;

    if (qstricmp(mo->classInfo(mo->indexOfClassInfo("Aggregatable")).value(), "no"))
        out << "\t\taggregatable," << endl;
    if (!qstricmp(mo->classInfo(mo->indexOfClassInfo("RegisterObject")).value(), "yes"))
        out << "\t\tappobject," << endl;
    if (mo->classInfo(mo->indexOfClassInfo("LicenseKey")).value())
        out << "\t\tlicensed," << endl;
    const char *helpString = mo->classInfo(mo->indexOfClassInfo("Description")).value();
    if (helpString)
        out << "\t\thelpstring(\"" << helpString << "\")," << endl;
    else
        out << "\t\thelpstring(\"" << cleanClassName << " Class\")," << endl;
    const char *classVersion = mo->classInfo(mo->indexOfClassInfo("Version")).value();
    if (classVersion)
        out << "\t\tversion(" << classVersion << ")," << endl;
    out << "\t\tuuid(" << classID << ')';
    if (control) {
        out << ", " << endl;
        out << "\t\tcontrol";
    } else if (!o) {
        out << ", " << endl;
        out << "\t\tnoncreatable";
    }
    out << endl;
    out << "\t]" << endl;
    out << "\tcoclass " << cleanClassName << endl;
    out << "\t{" << endl;
    out << "\t\t[default] dispinterface I" << cleanClassName << ';' << endl;
    if (hasEvents)
        out << "\t\t[default, source] dispinterface I" << cleanClassName << "Events;" << endl;
    out << "\t};" << endl;

    return S_OK;
}


extern "C" HRESULT __stdcall DumpIDL(const QString &outfile, const QString &ver)
{
    qAxIsServer = false;
    QTextStream out;
    if (outfile.contains(QLatin1String("\\"))) {
        QString outpath = outfile.left(outfile.lastIndexOf(QLatin1String("\\")));
        QDir dir;
        dir.mkpath(outpath);
    }
    QFile file(outfile);
    file.remove();

    QString filebase = QString::fromWCharArray(qAxModuleFilename);
    filebase = filebase.left(filebase.lastIndexOf(QLatin1Char('.')));

    QString appID = qAxFactory()->appID().toString().toUpper();
    if (QUuid(appID).isNull())
        return 1;
    STRIPCB(appID);
    QString typeLibID = qAxFactory()->typeLibID().toString().toUpper();
    if (QUuid(typeLibID).isNull())
        return 2;
    STRIPCB(typeLibID);
    QString typelib = filebase.right(filebase.length() - filebase.lastIndexOf(QLatin1String("\\"))-1);

    if (!file.open(QIODevice::WriteOnly))
        return -1;

    out.setDevice(&file);

    QString version(ver.unicode(), ver.length());
    while (version.count(QLatin1Char('.')) > 1) {
        int lastdot = version.lastIndexOf(QLatin1Char('.'));
        version = version.left(lastdot) + version.right(version.length() - lastdot - 1);
    }
    if (version.isEmpty())
        version = QLatin1String("1.0");

    QString idQRect(QUuid(CLSID_QRect).toString());
    STRIPCB(idQRect);
    QString idQSize(QUuid(CLSID_QSize).toString());
    STRIPCB(idQSize);
    QString idQPoint(QUuid(CLSID_QPoint).toString());
    STRIPCB(idQPoint);

    out << "/****************************************************************************" << endl;
    out << "** Interface definition generated for ActiveQt project" << endl;
    out << "**" << endl;
    out << "**     '" << QString::fromWCharArray(qAxModuleFilename) << '\'' << endl;
    out << "**" << endl;
    out << "** Created:  " << QDateTime::currentDateTime().toString() << endl;
    out << "**" << endl;
    out << "** WARNING! All changes made in this file will be lost!" << endl;
    out << "****************************************************************************/" << endl << endl;

    out << "import \"ocidl.idl\";" << endl;
    out << "#include <olectl.h>" << endl << endl;

    // dummy application to create widgets
    bool delete_qApp = false;
    if (!qApp) {
        int argc;
        (void)new QApplication(argc, 0);
        delete_qApp = true;
    }

    out << '[' << endl;
    out << "\tuuid(" << typeLibID << ")," << endl;
    out << "\tversion(" << version << ")," << endl;
    out << "\thelpstring(\"" << typelib << ' ' << version << " Type Library\")" << endl;
    out << ']' << endl;
    out << "library " << typelib << "Lib" << endl;
    out << '{' << endl;
    out << "\timportlib(\"stdole32.tlb\");" << endl;
    out << "\timportlib(\"stdole2.tlb\");" << endl << endl;

    QStringList keys = qAxFactory()->featureList();
    QStringList::ConstIterator key;

    out << "\t/************************************************************************" << endl;
    out << "\t** If this causes a compile error in MIDL you need to upgrade the" << endl;
    out << "\t** Platform SDK you are using. Download the SDK from msdn.microsoft.com" << endl;
    out << "\t** and make sure that both the system and the Visual Studio environment" << endl;
    out << "\t** use the correct files." << endl;
    out << "\t**" << endl;

#if __REQUIRED_RPCNDR_H_VERSION__ < Q_REQUIRED_RPCNDR_H_VERSION
    out << "\t** Required version of MIDL could not be verified. QRect, QSize and QPoint" << endl;
    out << "\t** support needs an updated Platform SDK to be installed." << endl;
    out << "\t*************************************************************************" << endl;
#else
    out << "\t************************************************************************/" << endl;
#endif

    out << endl;
    out << "\t[uuid(" << idQRect << ")]" << endl;
    out << "\tstruct QRect {" << endl;
    out << "\t\tint left;" << endl;
    out << "\t\tint top;" << endl;
    out << "\t\tint right;" << endl;
    out << "\t\tint bottom;" << endl;
    out << "\t};" << endl << endl;

    out << "\t[uuid(" << idQSize << ")]" << endl;
    out << "\tstruct QSize {" << endl;
    out << "\t\tint width;" << endl;
    out << "\t\tint height;" << endl;
    out << "\t};" << endl << endl;

    out << "\t[uuid(" << idQPoint << ")]" << endl;
    out << "\tstruct QPoint {" << endl;
    out << "\t\tint x;" << endl;
    out << "\t\tint y;" << endl;
    out << "\t};" << endl;
#if __REQUIRED_RPCNDR_H_VERSION__ < Q_REQUIRED_RPCNDR_H_VERSION
    out << "\t*/" << endl;
#endif

    out << endl;

    out << "\t/* Forward declaration of classes that might be used as parameters */" << endl << endl;

    int res = S_OK;
    for (key = keys.begin(); key != keys.end(); ++key) {
        QByteArray className = (*key).toLatin1();
        const QMetaObject *mo = qAxFactory()->metaObject(QString::fromLatin1(className.constData()));
        // We have meta object information for this type. Forward declare it.
        if (mo) {
            QByteArray cleanType = qax_clean_type(*key, mo).toLatin1();
            out << "\tcoclass " << cleanType << ';' << endl;
            subtypes.append(cleanType);
            qRegisterMetaType(cleanType, (void**)0);
            cleanType += '*';
            subtypes.append(cleanType);
            qRegisterMetaType(cleanType, (void**)0);
        }
    }
    out << endl;

    for (key = keys.begin(); key != keys.end(); ++key) {
        QByteArray className = (*key).toLatin1();
        const QMetaObject *mo = qAxFactory()->metaObject(QString::fromLatin1(className.constData()));
        // We have meta object information for this type. Define it.
        if (mo) {
            QObject *o = qAxFactory()->createObject(QString::fromLatin1(className.constData()));
            // It's not a control class, so it is actually a subtype. Define it.
            if (!o)
                res = classIDL(0, mo, QString::fromLatin1(className), false, out);
            delete o;
        }
    }

    out << endl;
    if (res != S_OK)
        goto ErrorInClass;

    for (key = keys.begin(); key != keys.end(); ++key) {
        QByteArray className = (*key).toLatin1();
        QObject *o = qAxFactory()->createObject(QString::fromLatin1(className.constData()));
        if (!o)
            continue;
        const QMetaObject *mo = o->metaObject();
        QAxBindable *bind = (QAxBindable*)o->qt_metacast("QAxBindable");
        bool isBindable =  bind != 0;

        QByteArray cleanType = qax_clean_type(*key, mo).toLatin1();
        subtypes.append(cleanType);
        subtypes.append(cleanType + '*');
        res = classIDL(o, mo, QString::fromLatin1(className.constData()), isBindable, out);
        delete o;
        if (res != S_OK)
            break;
    }

    out << "};" << endl;
    out.flush();

ErrorInClass:
    if (delete_qApp)
        delete qApp;

    if (res != S_OK) {
        file.close();
        file.remove();
    }

    return res;
}

QT_END_NAMESPACE
#endif // QT_NO_WIN_ACTIVEQT
