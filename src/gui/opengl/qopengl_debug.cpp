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

#include <qopengl_debug.h>

#include <qglobal.h>
#include <qvarlengtharray.h>
#include <qopengl.h>
#include <qopenglfunctions.h>
#include <qoffscreensurface.h>

// When using OpenGL ES 2.0, all the necessary GL_KHR_debug constants are
// provided in qopengles2ext.h. Unfortunately, newer versions of that file
// suffix everything with _KHR which causes extra headache when the goal is
// to have a single piece of code that builds in all our target
// environments. Therefore, try to detect this and use our custom defines
// instead, which we anyway need for OS X.

#if defined(GL_KHR_debug) && defined(GL_DEBUG_SOURCE_API_KHR)
#define USE_MANUAL_DEFS
#endif

// Under OSX (at least up to 10.8) we can not include our own copy of glext.h,
// using the system version unfortunately lacks the needed  defines/typedefs.
// Work around by adding GL_KHR_debug defines manually

#ifndef GL_KHR_debug
#define GL_KHR_debug 1
#define USE_MANUAL_DEFS
#endif

#ifdef USE_MANUAL_DEFS

#ifndef GL_DEBUG_OUTPUT_SYNCHRONOUS
#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#endif
#ifndef GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH 0x8243
#endif
#ifndef GL_DEBUG_CALLBACK_FUNCTION
#define GL_DEBUG_CALLBACK_FUNCTION        0x8244
#endif
#ifndef GL_DEBUG_CALLBACK_USER_PARAM
#define GL_DEBUG_CALLBACK_USER_PARAM      0x8245
#endif
#ifndef GL_DEBUG_SOURCE_API
#define GL_DEBUG_SOURCE_API               0x8246
#endif
#ifndef GL_DEBUG_SOURCE_WINDOW_SYSTEM
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM     0x8247
#endif
#ifndef GL_DEBUG_SOURCE_SHADER_COMPILER
#define GL_DEBUG_SOURCE_SHADER_COMPILER   0x8248
#endif
#ifndef GL_DEBUG_SOURCE_THIRD_PARTY
#define GL_DEBUG_SOURCE_THIRD_PARTY       0x8249
#endif
#ifndef GL_DEBUG_SOURCE_APPLICATION
#define GL_DEBUG_SOURCE_APPLICATION       0x824A
#endif
#ifndef GL_DEBUG_SOURCE_OTHER
#define GL_DEBUG_SOURCE_OTHER             0x824B
#endif
#ifndef GL_DEBUG_TYPE_ERROR
#define GL_DEBUG_TYPE_ERROR               0x824C
#endif
#ifndef GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#endif
#ifndef GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  0x824E
#endif
#ifndef GL_DEBUG_TYPE_PORTABILITY
#define GL_DEBUG_TYPE_PORTABILITY         0x824F
#endif
#ifndef GL_DEBUG_TYPE_PERFORMANCE
#define GL_DEBUG_TYPE_PERFORMANCE         0x8250
#endif
#ifndef GL_DEBUG_TYPE_OTHER
#define GL_DEBUG_TYPE_OTHER               0x8251
#endif
#ifndef GL_DEBUG_TYPE_MARKER
#define GL_DEBUG_TYPE_MARKER              0x8268
#endif
#ifndef GL_DEBUG_TYPE_PUSH_GROUP
#define GL_DEBUG_TYPE_PUSH_GROUP          0x8269
#endif
#ifndef GL_DEBUG_TYPE_POP_GROUP
#define GL_DEBUG_TYPE_POP_GROUP           0x826A
#endif
#ifndef GL_DEBUG_SEVERITY_NOTIFICATION
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#endif
#ifndef GL_MAX_DEBUG_GROUP_STACK_DEPTH
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH    0x826C
#endif
#ifndef GL_DEBUG_GROUP_STACK_DEPTH
#define GL_DEBUG_GROUP_STACK_DEPTH        0x826D
#endif
#ifndef GL_BUFFER
#define GL_BUFFER                         0x82E0
#endif
#ifndef GL_SHADER
#define GL_SHADER                         0x82E1
#endif
#ifndef GL_PROGRAM
#define GL_PROGRAM                        0x82E2
#endif
#ifndef GL_QUERY
#define GL_QUERY                          0x82E3
#endif
#ifndef GL_PROGRAM_PIPELINE
#define GL_PROGRAM_PIPELINE               0x82E4
#endif
#ifndef GL_SAMPLER
#define GL_SAMPLER                        0x82E6
#endif
#ifndef GL_DISPLAY_LIST
#define GL_DISPLAY_LIST                   0x82E7
#endif
#ifndef GL_MAX_LABEL_LENGTH
#define GL_MAX_LABEL_LENGTH               0x82E8
#endif
#ifndef GL_MAX_DEBUG_MESSAGE_LENGTH
#define GL_MAX_DEBUG_MESSAGE_LENGTH       0x9143
#endif
#ifndef GL_MAX_DEBUG_LOGGED_MESSAGES
#define GL_MAX_DEBUG_LOGGED_MESSAGES      0x9144
#endif
#ifndef GL_DEBUG_LOGGED_MESSAGES
#define GL_DEBUG_LOGGED_MESSAGES          0x9145
#endif
#ifndef GL_DEBUG_SEVERITY_HIGH
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#endif
#ifndef GL_DEBUG_SEVERITY_MEDIUM
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#endif
#ifndef GL_DEBUG_SEVERITY_LOW
#define GL_DEBUG_SEVERITY_LOW             0x9148
#endif
#ifndef GL_DEBUG_OUTPUT
#define GL_DEBUG_OUTPUT                   0x92E0
#endif
#ifndef GL_CONTEXT_FLAG_DEBUG_BIT
#define GL_CONTEXT_FLAG_DEBUG_BIT         0x00000002
#endif
#ifndef GL_STACK_OVERFLOW
#define GL_STACK_OVERFLOW                 0x0503
#endif
#ifndef GL_STACK_UNDERFLOW
#define GL_STACK_UNDERFLOW                0x0504
#endif

typedef void (QOPENGLF_APIENTRY *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,
   GLsizei length,const GLchar *message,const GLvoid *userParam);

#endif

template <typename T, typename U>
std::enable_if_t<sizeof(T) == sizeof(U) &&
   std::is_trivially_copyable_v<T> && std::is_trivially_copyable_v<U>, T>
cs_bitCast(const U &input) noexcept
{
   static_assert(std::is_trivially_constructible_v<T>);

   T retval;
   std::memcpy(&retval, &input, sizeof(U));

   return retval;
}

static QOpenGLDebugMessage::Source qt_messageSourceFromGL(GLenum source)
{
   switch (source) {
      case GL_DEBUG_SOURCE_API:
         return QOpenGLDebugMessage::APISource;

      case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
         return QOpenGLDebugMessage::WindowSystemSource;

      case GL_DEBUG_SOURCE_SHADER_COMPILER:
         return QOpenGLDebugMessage::ShaderCompilerSource;

      case GL_DEBUG_SOURCE_THIRD_PARTY:
          return QOpenGLDebugMessage::ThirdPartySource;

      case GL_DEBUG_SOURCE_APPLICATION:
         return QOpenGLDebugMessage::ApplicationSource;

      case GL_DEBUG_SOURCE_OTHER:
         return QOpenGLDebugMessage::OtherSource;
    }

    Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown message source from GL");

    return QOpenGLDebugMessage::OtherSource;
}

static GLenum qt_messageSourceToGL(QOpenGLDebugMessage::Source source)
{
   switch (source) {
      case QOpenGLDebugMessage::InvalidSource:
         break;

      case QOpenGLDebugMessage::APISource:
         return GL_DEBUG_SOURCE_API;

      case QOpenGLDebugMessage::WindowSystemSource:
         return GL_DEBUG_SOURCE_WINDOW_SYSTEM;

      case QOpenGLDebugMessage::ShaderCompilerSource:
         return GL_DEBUG_SOURCE_SHADER_COMPILER;

      case QOpenGLDebugMessage::ThirdPartySource:
         return GL_DEBUG_SOURCE_THIRD_PARTY;

      case QOpenGLDebugMessage::ApplicationSource:
         return GL_DEBUG_SOURCE_APPLICATION;

      case QOpenGLDebugMessage::OtherSource:
         return GL_DEBUG_SOURCE_OTHER;

      case QOpenGLDebugMessage::AnySource:
         break;
   }

    Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid message source");
    return GL_DEBUG_SOURCE_OTHER;
}

static QString qt_messageSourceToString(QOpenGLDebugMessage::Source source)
{
   switch (source) {
      case QOpenGLDebugMessage::InvalidSource:
         return QString("InvalidSource");

      case QOpenGLDebugMessage::APISource:
         return QString("APISource");

      case QOpenGLDebugMessage::WindowSystemSource:
         return QString("WindowSystemSource");

      case QOpenGLDebugMessage::ShaderCompilerSource:
         return QString("ShaderCompilerSource");

      case QOpenGLDebugMessage::ThirdPartySource:
         return QString("ThirdPartySource");

      case QOpenGLDebugMessage::ApplicationSource:
         return QString("ApplicationSource");

      case QOpenGLDebugMessage::OtherSource:
         return QString("OtherSource");

      case QOpenGLDebugMessage::AnySource:
         return QString("AnySource");
    }

    Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown message source");
    return QString();
}

static QOpenGLDebugMessage::Type qt_messageTypeFromGL(GLenum type)
{
   switch (type) {
      case GL_DEBUG_TYPE_ERROR:
         return QOpenGLDebugMessage::ErrorType;

      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
         return QOpenGLDebugMessage::DeprecatedBehaviorType;

      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
         return QOpenGLDebugMessage::UndefinedBehaviorType;

      case GL_DEBUG_TYPE_PORTABILITY:
         return QOpenGLDebugMessage::PortabilityType;

      case GL_DEBUG_TYPE_PERFORMANCE:
         return QOpenGLDebugMessage::PerformanceType;

      case GL_DEBUG_TYPE_OTHER:
         return QOpenGLDebugMessage::OtherType;

      case GL_DEBUG_TYPE_MARKER:
         return QOpenGLDebugMessage::MarkerType;

      case GL_DEBUG_TYPE_PUSH_GROUP:
         return QOpenGLDebugMessage::GroupPushType;

      case GL_DEBUG_TYPE_POP_GROUP:
         return QOpenGLDebugMessage::GroupPopType;
   }

   Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown message type from GL");

   return QOpenGLDebugMessage::OtherType;
}

static GLenum qt_messageTypeToGL(QOpenGLDebugMessage::Type type)
{
   switch (type) {
      case QOpenGLDebugMessage::InvalidType:
         break;

      case QOpenGLDebugMessage::ErrorType:
         return GL_DEBUG_TYPE_ERROR;

      case QOpenGLDebugMessage::DeprecatedBehaviorType:
         return GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR;

      case QOpenGLDebugMessage::UndefinedBehaviorType:
         return GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR;

      case QOpenGLDebugMessage::PortabilityType:
         return GL_DEBUG_TYPE_PORTABILITY;

      case QOpenGLDebugMessage::PerformanceType:
         return GL_DEBUG_TYPE_PERFORMANCE;

      case QOpenGLDebugMessage::OtherType:
         return GL_DEBUG_TYPE_OTHER;

      case QOpenGLDebugMessage::MarkerType:
         return GL_DEBUG_TYPE_MARKER;

      case QOpenGLDebugMessage::GroupPushType:
         return GL_DEBUG_TYPE_PUSH_GROUP;

      case QOpenGLDebugMessage::GroupPopType:
         return GL_DEBUG_TYPE_POP_GROUP;

      case QOpenGLDebugMessage::AnyType:
         break;
   }

   Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid message type");
   return GL_DEBUG_TYPE_OTHER;
}

static QString qt_messageTypeToString(QOpenGLDebugMessage::Type type)
{
   switch (type) {
      case QOpenGLDebugMessage::InvalidType:
         return QString("InvalidType");

      case QOpenGLDebugMessage::ErrorType:
         return QString("ErrorType");

      case QOpenGLDebugMessage::DeprecatedBehaviorType:
         return QString("DeprecatedBehaviorType");

      case QOpenGLDebugMessage::UndefinedBehaviorType:
         return QString("UndefinedBehaviorType");

      case QOpenGLDebugMessage::PortabilityType:
         return QString("PortabilityType");

      case QOpenGLDebugMessage::PerformanceType:
         return QString("PerformanceType");

      case QOpenGLDebugMessage::OtherType:
         return QString("OtherType");

      case QOpenGLDebugMessage::MarkerType:
         return QString("MarkerType");

      case QOpenGLDebugMessage::GroupPushType:
         return QString("GroupPushType");

      case QOpenGLDebugMessage::GroupPopType:
         return QString("GroupPopType");

      case QOpenGLDebugMessage::AnyType:
         return QString("AnyType");
      }

   Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown message type");

   return QString();
}

static QOpenGLDebugMessage::Severity qt_messageSeverityFromGL(GLenum severity)
{
   switch (severity) {
      case GL_DEBUG_SEVERITY_HIGH:
         return QOpenGLDebugMessage::HighSeverity;

      case GL_DEBUG_SEVERITY_MEDIUM:
         return QOpenGLDebugMessage::MediumSeverity;

      case GL_DEBUG_SEVERITY_LOW:
         return QOpenGLDebugMessage::LowSeverity;

      case GL_DEBUG_SEVERITY_NOTIFICATION:
         return QOpenGLDebugMessage::NotificationSeverity;
   }

   Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown message severity from GL");

   return QOpenGLDebugMessage::NotificationSeverity;
}

static GLenum qt_messageSeverityToGL(QOpenGLDebugMessage::Severity severity)
{
   switch (severity) {
      case QOpenGLDebugMessage::InvalidSeverity:
         break;

      case QOpenGLDebugMessage::HighSeverity:
         return GL_DEBUG_SEVERITY_HIGH;

      case QOpenGLDebugMessage::MediumSeverity:
         return GL_DEBUG_SEVERITY_MEDIUM;

      case QOpenGLDebugMessage::LowSeverity:
         return GL_DEBUG_SEVERITY_LOW;

      case QOpenGLDebugMessage::NotificationSeverity:
         return GL_DEBUG_SEVERITY_NOTIFICATION;

      case QOpenGLDebugMessage::AnySeverity:
         break;
   }

   Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid message severity");

   return GL_DEBUG_SEVERITY_NOTIFICATION;
}

static QString qt_messageSeverityToString(QOpenGLDebugMessage::Severity severity)
{
   switch (severity) {
      case QOpenGLDebugMessage::InvalidSeverity:
         return QString("InvalidSeverity");

      case QOpenGLDebugMessage::HighSeverity:
         return QString("HighSeverity");

      case QOpenGLDebugMessage::MediumSeverity:
         return QString("MediumSeverity");

      case QOpenGLDebugMessage::LowSeverity:
         return QString("LowSeverity");

      case QOpenGLDebugMessage::NotificationSeverity:
         return QString("NotificationSeverity");

      case QOpenGLDebugMessage::AnySeverity:
         return QString("AnySeverity");
   }

   Q_ASSERT_X(false, Q_FUNC_INFO, "Unknown message severity");

   return QString();
}

class QOpenGLDebugMessagePrivate : public QSharedData
{
 public:
    QOpenGLDebugMessagePrivate();

    QString message;
    GLuint id;
    QOpenGLDebugMessage::Source source;
    QOpenGLDebugMessage::Type type;
    QOpenGLDebugMessage::Severity severity;
};

// internal
QOpenGLDebugMessagePrivate::QOpenGLDebugMessagePrivate()
    : message(), id(0), source(QOpenGLDebugMessage::InvalidSource),
      type(QOpenGLDebugMessage::InvalidType), severity(QOpenGLDebugMessage::InvalidSeverity)
{
}

QOpenGLDebugMessage::QOpenGLDebugMessage()
    : d(new QOpenGLDebugMessagePrivate)
{
}

QOpenGLDebugMessage::QOpenGLDebugMessage(const QOpenGLDebugMessage &debugMessage)
    : d(debugMessage.d)
{
}

QOpenGLDebugMessage::~QOpenGLDebugMessage()
{
}

QOpenGLDebugMessage &QOpenGLDebugMessage::operator=(const QOpenGLDebugMessage &debugMessage)
{
    d = debugMessage.d;
    return *this;
}

QOpenGLDebugMessage::Source QOpenGLDebugMessage::source() const
{
    return d->source;
}

QOpenGLDebugMessage::Type QOpenGLDebugMessage::type() const
{
    return d->type;
}

QOpenGLDebugMessage::Severity QOpenGLDebugMessage::severity() const
{
    return d->severity;
}

GLuint QOpenGLDebugMessage::id() const
{
    return d->id;
}

QString QOpenGLDebugMessage::message() const
{
    return d->message;
}

QOpenGLDebugMessage QOpenGLDebugMessage::createApplicationMessage(const QString &text,
      GLuint id, QOpenGLDebugMessage::Severity severity, QOpenGLDebugMessage::Type type)
{
    QOpenGLDebugMessage m;
    m.d->message = text;
    m.d->id = id;
    m.d->severity = severity;
    m.d->type = type;
    m.d->source = ApplicationSource;

    return m;
}

QOpenGLDebugMessage QOpenGLDebugMessage::createThirdPartyMessage(const QString &text,
      GLuint id, QOpenGLDebugMessage::Severity severity, QOpenGLDebugMessage::Type type)
{
    QOpenGLDebugMessage m;
    m.d->message = text;
    m.d->id = id;
    m.d->severity = severity;
    m.d->type = type;
    m.d->source = ThirdPartySource;

    return m;
}

bool QOpenGLDebugMessage::operator==(const QOpenGLDebugMessage &debugMessage) const
{
   return (d == debugMessage.d)
            || (d->id == debugMessage.d->id
                && d->source == debugMessage.d->source
                && d->type == debugMessage.d->type
                && d->severity == debugMessage.d->severity
                && d->message == debugMessage.d->message);
}

QDebug operator<<(QDebug debug, QOpenGLDebugMessage::Source source)
{
   QDebugStateSaver saver(debug);
   debug.nospace() << "QOpenGLDebugMessage::Source("
                   << qt_messageSourceToString(source)
                   << ')';

    return debug;
}

QDebug operator<<(QDebug debug, QOpenGLDebugMessage::Type type)
{
   QDebugStateSaver saver(debug);
   debug.nospace() << "QOpenGLDebugMessage::Type("
                   << qt_messageTypeToString(type)
                   << ')';

    return debug;
}

QDebug operator<<(QDebug debug, QOpenGLDebugMessage::Severity severity)
{
   QDebugStateSaver saver(debug);
   debug.nospace() << "QOpenGLDebugMessage::Severity("
                   << qt_messageSeverityToString(severity)
                   << ')';

   return debug;
}

QDebug operator<<(QDebug debug, const QOpenGLDebugMessage &message)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "QOpenGLDebugMessage("
                    << qt_messageSourceToString(message.source()) << ", "
                    << message.id() << ", "
                    << message.message() << ", "
                    << qt_messageSeverityToString(message.severity()) << ", "
                    << qt_messageTypeToString(message.type()) << ')';
    return debug;

}

typedef void (QOPENGLF_APIENTRYP qt_glDebugMessageControl_t)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);

typedef void (QOPENGLF_APIENTRYP qt_glDebugMessageInsert_t)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);

typedef void (QOPENGLF_APIENTRYP qt_glDebugMessageCallback_t)(GLDEBUGPROC callback, const void *userParam);

typedef GLuint (QOPENGLF_APIENTRYP qt_glGetDebugMessageLog_t)(GLuint count, GLsizei bufsize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);

typedef void (QOPENGLF_APIENTRYP qt_glPushDebugGroup_t)(GLenum source, GLuint id, GLsizei length, const GLchar *message);
typedef void (QOPENGLF_APIENTRYP qt_glPopDebugGroup_t)();
typedef void (QOPENGLF_APIENTRYP qt_glGetPointerv_t)(GLenum pname, GLvoid **params);

class QOpenGLDebugLoggerPrivate
{
    Q_DECLARE_PUBLIC(QOpenGLDebugLogger)

 public:
    QOpenGLDebugLoggerPrivate();

    void handleMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *rawMessage);

    void controlDebugMessages(QOpenGLDebugMessage::Sources sources, QOpenGLDebugMessage::Types types,
          QOpenGLDebugMessage::Severities severities, const QVector<GLuint> &ids,
          const QByteArray &callerName, bool enable);

    void _q_contextAboutToBeDestroyed();

    qt_glDebugMessageControl_t glDebugMessageControl;
    qt_glDebugMessageInsert_t glDebugMessageInsert;
    qt_glDebugMessageCallback_t glDebugMessageCallback;
    qt_glGetDebugMessageLog_t glGetDebugMessageLog;
    qt_glPushDebugGroup_t glPushDebugGroup;
    qt_glPopDebugGroup_t glPopDebugGroup;
    qt_glGetPointerv_t glGetPointerv;

    GLDEBUGPROC oldDebugCallbackFunction;
    void *oldDebugCallbackParameter;
    QOpenGLContext *context;
    GLint maxMessageLength;
    QOpenGLDebugLogger::LoggingMode loggingMode;
    bool initialized : 1;
    bool isLogging : 1;
    bool debugWasEnabled : 1;
    bool syncDebugWasEnabled : 1;

 protected:
   QOpenGLDebugLogger *q_ptr;
};

// internal
QOpenGLDebugLoggerPrivate::QOpenGLDebugLoggerPrivate()
    : glDebugMessageControl(nullptr), glDebugMessageInsert(nullptr), glDebugMessageCallback(nullptr),
      glGetDebugMessageLog(nullptr), glPushDebugGroup(nullptr), glPopDebugGroup(nullptr),
      oldDebugCallbackFunction(nullptr), context(nullptr), maxMessageLength(0),
      loggingMode(QOpenGLDebugLogger::AsynchronousLogging),
      initialized(false), isLogging(false), debugWasEnabled(false), syncDebugWasEnabled(false)
{
}

// internal
void QOpenGLDebugLoggerPrivate::handleMessage(GLenum source, GLenum type, GLuint id,
      GLenum severity, GLsizei length, const GLchar *rawMessage)
{
    if (oldDebugCallbackFunction) {
       oldDebugCallbackFunction(source, type, id, severity, length, rawMessage, oldDebugCallbackParameter);
    }

    QOpenGLDebugMessage message;

    QOpenGLDebugMessagePrivate *messagePrivate = message.d.data();
    messagePrivate->source = qt_messageSourceFromGL(source);
    messagePrivate->type = qt_messageTypeFromGL(type);
    messagePrivate->id = id;
    messagePrivate->severity = qt_messageSeverityFromGL(severity);

    // not passing the length to fromUtf8, as some bugged OpenGL drivers
    // do not handle the length correctly. Just rely on the message to be NUL terminated.
    messagePrivate->message = QString::fromUtf8(rawMessage);

    Q_Q(QOpenGLDebugLogger);
    emit q->messageLogged(message);
}

// internal
void QOpenGLDebugLoggerPrivate::controlDebugMessages(QOpenGLDebugMessage::Sources sources,
      QOpenGLDebugMessage::Types types, QOpenGLDebugMessage::Severities severities,
      const QVector<GLuint> &ids, const QByteArray &callerName, bool enable)
{
    if (! initialized) {
        qWarning("QOpenGLDebugLogger::%s(): Object must be initialized before enabling/disabling messages", callerName.constData());
        return;
    }

    if (sources == QOpenGLDebugMessage::InvalidSource) {
        qWarning("QOpenGLDebugLogger::%s() Invalid source specified", callerName.constData());
        return;
    }

    if (types == QOpenGLDebugMessage::InvalidType) {
        qWarning("QOpenGLDebugLogger::%s(): invalid type specified", callerName.constData());
        return;
    }

    if (severities == QOpenGLDebugMessage::InvalidSeverity) {
        qWarning("QOpenGLDebugLogger::%s(): invalid severity specified", callerName.constData());
        return;
    }

    QVarLengthArray<GLenum, 8> glSources;
    QVarLengthArray<GLenum, 8> glTypes;
    QVarLengthArray<GLenum, 8> glSeverities;

    if (ids.count() > 0) {
        Q_ASSERT(severities == QOpenGLDebugMessage::AnySeverity);

        // The GL_KHR_debug extension says:
        //
        //        - If <count> is greater than zero, then <ids> is an array of <count>
        //          message IDs for the specified combination of <source> and <type>. In
        //          this case, if <source> or <type> is DONT_CARE, or <severity> is not
        //          DONT_CARE, the error INVALID_OPERATION is generated. If <count> is
        //          zero, the value if <ids> is ignored.
        //
        // This means we can't convert AnySource or AnyType into DONT_CARE, but we have to roll
        // them into individual sources/types.

        if (sources == QOpenGLDebugMessage::AnySource) {
            sources = QOpenGLDebugMessage::InvalidSource;
            for (uint i = 1; i <= QOpenGLDebugMessage::LastSource; i = i << 1)
                sources |= QOpenGLDebugMessage::Source(i);
        }

        if (types == QOpenGLDebugMessage::AnyType) {
            types = QOpenGLDebugMessage::InvalidType;
            for (uint i = 1; i <= QOpenGLDebugMessage::LastType; i = i << 1)
                types |= QOpenGLDebugMessage::Type(i);
        }
    }

#define CONVERT_TO_GL_DEBUG_MESSAGE_CONTROL_PARAMETERS(type, source, target) \
    if (source == QOpenGLDebugMessage::Any ## type) { \
        target << GL_DONT_CARE; \
    } else { \
        for (uint i = 1; i <= QOpenGLDebugMessage::Last ## type; i = i << 1) \
            if (source.testFlag(QOpenGLDebugMessage:: type (i))) \
                target << qt_message ## type ## ToGL (QOpenGLDebugMessage:: type (i)); \
    }

    CONVERT_TO_GL_DEBUG_MESSAGE_CONTROL_PARAMETERS(Source, sources, glSources)
    CONVERT_TO_GL_DEBUG_MESSAGE_CONTROL_PARAMETERS(Type, types, glTypes)
    CONVERT_TO_GL_DEBUG_MESSAGE_CONTROL_PARAMETERS(Severity, severities, glSeverities)
#undef CONVERT_TO_GL_DEBUG_MESSAGE_CONTROL_PARAMETERS

    const GLsizei idCount = ids.count();
    // The GL_KHR_debug extension says that if idCount is 0, idPtr must be ignored.
    // Unfortunately, some bugged drivers do NOT ignore it, so pass NULL in case.
    const GLuint * const idPtr = idCount ? ids.constData() : nullptr;

    for (GLenum source : glSources) {
        for (GLenum type : glTypes) {
            for (GLenum severity : glSeverities) {
                glDebugMessageControl(source, type, severity, idCount, idPtr, GLboolean(enable));
            }
        }
    }
}

// internal
void QOpenGLDebugLoggerPrivate::_q_contextAboutToBeDestroyed()
{
    Q_ASSERT(context);

    // Re-make our context current somehow, otherwise stopLogging will fail.

    // Save the current context and its surface in case we need to set them back
    QOpenGLContext *currentContext = QOpenGLContext::currentContext();
    QSurface *currentSurface = nullptr;

    QScopedPointer<QOffscreenSurface> offscreenSurface;

    if (context != currentContext) {
        // Make our old context current on a temporary surface
        if (currentContext)
            currentSurface = currentContext->surface();

        offscreenSurface.reset(new QOffscreenSurface);
        offscreenSurface->setFormat(context->format());
        offscreenSurface->create();

        if (! context->makeCurrent(offscreenSurface.data())) {
            qWarning("QOpenGLDebugLogger::_q_contextAboutToBeDestroyed(): "
               "Unable to make the owning GL context current for cleanup");
        }
    }

    Q_Q(QOpenGLDebugLogger);
    q->stopLogging();

    if (offscreenSurface) {
        // we did change the current context: set it back
        if (currentContext)
            currentContext->makeCurrent(currentSurface);
        else
            context->doneCurrent();
    }

    QObject::disconnect(context, &QOpenGLContext::aboutToBeDestroyed, q, &QOpenGLDebugLogger::_q_contextAboutToBeDestroyed);
    context = nullptr;
    initialized = false;
}

extern "C" {
static void QOPENGLF_APIENTRY qt_opengl_debug_callback(GLenum source, GLenum type, GLuint id,
      GLenum severity, GLsizei length, const GLchar *rawMessage, const GLvoid *userParam)
{
    QOpenGLDebugLoggerPrivate *loggerPrivate = static_cast<QOpenGLDebugLoggerPrivate *>(const_cast<GLvoid *>(userParam));
    loggerPrivate->handleMessage(source, type, id, severity, length, rawMessage);
}
}

QOpenGLDebugLogger::QOpenGLDebugLogger(QObject *parent)
    : QObject(parent), d_ptr(new QOpenGLDebugLoggerPrivate)
{
   d_ptr->q_ptr = this;
}

QOpenGLDebugLogger::~QOpenGLDebugLogger()
{
    stopLogging();
}

bool QOpenGLDebugLogger::initialize()
{
    QOpenGLContext *context = QOpenGLContext::currentContext();
    if (!context) {
        qWarning("QOpenGLDebugLogger::initialize(): no current OpenGL context found.");
        return false;
    }

    Q_D(QOpenGLDebugLogger);
    if (d->context == context) {
        // context is non-NULL, d->context is non NULL only on successful initialization.
        Q_ASSERT(d->initialized);
        return true;
    }

    if (d->isLogging) {
        qWarning("QOpenGLDebugLogger::initialize(): cannot initialize the object while logging. Please stop the logging first.");
        return false;
    }

    if (d->context)
        disconnect(d->context, &QOpenGLContext::aboutToBeDestroyed, this, &QOpenGLDebugLogger::_q_contextAboutToBeDestroyed);

    d->initialized = false;
    d->context = nullptr;

    if (! context->hasExtension("GL_KHR_debug"))
        return false;

    d->context = context;
    connect(d->context, &QOpenGLContext::aboutToBeDestroyed, this, &QOpenGLDebugLogger::_q_contextAboutToBeDestroyed);

#define GET_DEBUG_PROC_ADDRESS(procName) \
    d->procName = reinterpret_cast< qt_ ## procName ## _t >( \
        d->context->getProcAddress(#procName) \
    );

    GET_DEBUG_PROC_ADDRESS(glDebugMessageControl);
    GET_DEBUG_PROC_ADDRESS(glDebugMessageInsert);
    GET_DEBUG_PROC_ADDRESS(glDebugMessageCallback);
    GET_DEBUG_PROC_ADDRESS(glGetDebugMessageLog);
    GET_DEBUG_PROC_ADDRESS(glPushDebugGroup);
    GET_DEBUG_PROC_ADDRESS(glPopDebugGroup);

    // Windows Desktop GL does not allow resolution of "basic GL entry points"
    // through wglGetProcAddress

#if defined(Q_OS_WIN) && ! defined(QT_OPENGL_ES_2)
    {
        HMODULE handle = static_cast<HMODULE>(QOpenGLContext::openGLModuleHandle());

        if (! handle) {
           handle = GetModuleHandleA("opengl32.dll");
        }

        d->glGetPointerv = cs_bitCast<qt_glGetPointerv_t>(GetProcAddress(handle, "glGetPointerv"));
    }

#else
    GET_DEBUG_PROC_ADDRESS(glGetPointerv)
#endif

#undef GET_DEBUG_PROC_ADDRESS

    QOpenGLContext::currentContext()->functions()->glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH, &d->maxMessageLength);

#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
    if (! d->context->format().testOption(QSurfaceFormat::DebugContext)) {
        qWarning("QOpenGLDebugLogger::initialize(): the current context is not a debug context:\n"
                 "    which means the GL may not generate any debug output.\n"
                 "    To avoid this warning try creating the context with the\n"
                 "    QSurfaceFormat::DebugContext surface format option.");
    }
#endif

    d->initialized = true;
    return true;
}

bool QOpenGLDebugLogger::isLogging() const
{
    Q_D(const QOpenGLDebugLogger);
    return d->isLogging;
}

void QOpenGLDebugLogger::startLogging(QOpenGLDebugLogger::LoggingMode loggingMode)
{
    Q_D(QOpenGLDebugLogger);
    if (! d->initialized) {
        qWarning("QOpenGLDebugLogger::startLogging(): object must be initialized before logging can start");
        return;
    }

    if (d->isLogging) {
        qWarning("QOpenGLDebugLogger::startLogging(): this object is already logging");
        return;
    }

    d->isLogging = true;
    d->loggingMode = loggingMode;

    d->glGetPointerv(GL_DEBUG_CALLBACK_FUNCTION, reinterpret_cast<void **>(&d->oldDebugCallbackFunction));
    d->glGetPointerv(GL_DEBUG_CALLBACK_USER_PARAM, &d->oldDebugCallbackParameter);

    d->glDebugMessageCallback(&qt_opengl_debug_callback, d);

    QOpenGLFunctions *funcs = QOpenGLContext::currentContext()->functions();
    d->debugWasEnabled = funcs->glIsEnabled(GL_DEBUG_OUTPUT);
    d->syncDebugWasEnabled = funcs->glIsEnabled(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    if (d->loggingMode == SynchronousLogging)
        funcs->glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    else
        funcs->glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    funcs->glEnable(GL_DEBUG_OUTPUT);
}

QOpenGLDebugLogger::LoggingMode QOpenGLDebugLogger::loggingMode() const
{
    Q_D(const QOpenGLDebugLogger);
    return d->loggingMode;
}

void QOpenGLDebugLogger::stopLogging()
{
    Q_D(QOpenGLDebugLogger);
    if (! d->isLogging)
        return;

    QOpenGLContext *currentContext = QOpenGLContext::currentContext();
    if (! currentContext || currentContext != d->context) {
        qWarning("QOpenGLDebugLogger::stopLogging(): attempting to stop logging with the wrong OpenGL context current");
        return;
    }

    d->isLogging = false;

    d->glDebugMessageCallback(d->oldDebugCallbackFunction, d->oldDebugCallbackParameter);

    QOpenGLFunctions *funcs = QOpenGLContext::currentContext()->functions();
    if (!d->debugWasEnabled)
        funcs->glDisable(GL_DEBUG_OUTPUT);

    if (d->syncDebugWasEnabled)
        funcs->glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    else
        funcs->glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
}

void QOpenGLDebugLogger::logMessage(const QOpenGLDebugMessage &debugMessage)
{
    Q_D(QOpenGLDebugLogger);
    if (!d->initialized) {
        qWarning("QOpenGLDebugLogger::logMessage(): object must be initialized before logging messages");
        return;
    }

    if (debugMessage.source() != QOpenGLDebugMessage::ApplicationSource
            && debugMessage.source() != QOpenGLDebugMessage::ThirdPartySource) {
        qWarning("QOpenGLDebugLogger::logMessage(): using a message source different from ApplicationSource\n"
                 "    or ThirdPartySource is not supported by GL_KHR_debug. The message will not be logged.");
        return;
    }

    if (debugMessage.type() == QOpenGLDebugMessage::InvalidType
            || debugMessage.type() == QOpenGLDebugMessage::AnyType
            || debugMessage.severity() == QOpenGLDebugMessage::InvalidSeverity
            || debugMessage.severity() == QOpenGLDebugMessage::AnySeverity) {
        qWarning("QOpenGLDebugLogger::logMessage(): the message has a non-valid type and/or severity. The message will not be logged.");
        return;
    }

    const GLenum source = qt_messageSourceToGL(debugMessage.source());
    const GLenum type = qt_messageTypeToGL(debugMessage.type());
    const GLenum severity = qt_messageSeverityToGL(debugMessage.severity());
    QByteArray rawMessage = debugMessage.message().toUtf8();
    rawMessage.append('\0');

    if (rawMessage.length() > d->maxMessageLength) {
        qWarning("QOpenGLDebugLogger::logMessage(): message too long, truncating it\n"
                 "    (%d bytes long, but the GL accepts up to %d bytes)", rawMessage.length(), d->maxMessageLength);
        rawMessage.resize(d->maxMessageLength - 1);
        rawMessage.append('\0');
    }

    // Don't pass rawMessage.length(), as unfortunately bugged
    // OpenGL drivers will eat the trailing NUL in the message. Just rely
    // on the message being NUL terminated.
    d->glDebugMessageInsert(source, type, debugMessage.id(), severity, -1, rawMessage.constData());
}

void QOpenGLDebugLogger::pushGroup(const QString &name, GLuint id, QOpenGLDebugMessage::Source source)
{
    Q_D(QOpenGLDebugLogger);

    if (! d->initialized) {
        qWarning("QOpenGLDebugLogger::pushGroup(): object must be initialized before pushing a debug group");
        return;
    }

    if (source != QOpenGLDebugMessage::ApplicationSource
            && source != QOpenGLDebugMessage::ThirdPartySource) {
        qWarning("QOpenGLDebugLogger::pushGroup(): using a source different from ApplicationSource\n"
                 "    or ThirdPartySource is not supported by GL_KHR_debug. The group will not be pushed.");
        return;
    }

    QByteArray rawName = name.toUtf8();
    rawName.append('\0');
    if (rawName.length() > d->maxMessageLength) {
        qWarning("QOpenGLDebugLogger::pushGroup(): group name too long, truncating it\n"
                 "    (%d bytes long, but the GL accepts up to %d bytes)", rawName.length(), d->maxMessageLength);
        rawName.resize(d->maxMessageLength - 1);
        rawName.append('\0');
    }

    // Don't pass rawMessage.length(), as unfortunately bugged
    // OpenGL drivers will eat the trailing NUL in the name. Just rely
    // on the name being NUL terminated.
    d->glPushDebugGroup(qt_messageSourceToGL(source), id, -1, rawName.constData());
}

void QOpenGLDebugLogger::popGroup()
{
    Q_D(QOpenGLDebugLogger);
    if (! d->initialized) {
        qWarning("QOpenGLDebugLogger::pushGroup(): object must be initialized before popping a debug group");
        return;
    }

    d->glPopDebugGroup();
}

void QOpenGLDebugLogger::enableMessages(QOpenGLDebugMessage::Sources sources,
      QOpenGLDebugMessage::Types types, QOpenGLDebugMessage::Severities severities)
{
    Q_D(QOpenGLDebugLogger);
    d->controlDebugMessages(sources, types, severities, QVector<GLuint>(), "enableMessages", true);
}

void QOpenGLDebugLogger::enableMessages(const QVector<GLuint> &ids,
      QOpenGLDebugMessage::Sources sources, QOpenGLDebugMessage::Types types)
{
    Q_D(QOpenGLDebugLogger);
    d->controlDebugMessages(sources, types, QOpenGLDebugMessage::AnySeverity, ids, "enableMessages", true);
}

void QOpenGLDebugLogger::disableMessages(QOpenGLDebugMessage::Sources sources,
      QOpenGLDebugMessage::Types types, QOpenGLDebugMessage::Severities severities)
{
    Q_D(QOpenGLDebugLogger);
    d->controlDebugMessages(sources, types, severities, QVector<GLuint>(), "disableMessages", false);
}

void QOpenGLDebugLogger::disableMessages(const QVector<GLuint> &ids,
      QOpenGLDebugMessage::Sources sources, QOpenGLDebugMessage::Types types)
{
    Q_D(QOpenGLDebugLogger);
    d->controlDebugMessages(sources, types, QOpenGLDebugMessage::AnySeverity, ids, "disableMessages", false);
}

QList<QOpenGLDebugMessage> QOpenGLDebugLogger::loggedMessages() const
{
    Q_D(const QOpenGLDebugLogger);

    if (! d->initialized) {
        qWarning("QOpenGLDebugLogger::loggedMessages(): object must be initialized before reading logged messages");
        return QList<QOpenGLDebugMessage>();
    }

    static constexpr const GLuint maxMessageCount = 128;
    GLuint messagesRead;

    GLenum messageSources[maxMessageCount];
    GLenum messageTypes[maxMessageCount];
    GLuint messageIds[maxMessageCount];
    GLenum messageSeverities[maxMessageCount];
    GLsizei messageLengths[maxMessageCount];

    QByteArray messagesBuffer;
    messagesBuffer.resize(maxMessageCount * d->maxMessageLength);

    QList<QOpenGLDebugMessage> messages;
    do {
        messagesRead = d->glGetDebugMessageLog(maxMessageCount, GLsizei(messagesBuffer.size()),
              messageSources, messageTypes, messageIds, messageSeverities,
              messageLengths, messagesBuffer.data());

        const char *messagesBufferPtr = messagesBuffer.constData();

        for (GLuint i = 0; i < messagesRead; ++i) {
            QOpenGLDebugMessage message;

            QOpenGLDebugMessagePrivate *messagePrivate = message.d.data();
            messagePrivate->source = qt_messageSourceFromGL(messageSources[i]);
            messagePrivate->type = qt_messageTypeFromGL(messageTypes[i]);
            messagePrivate->id = messageIds[i];
            messagePrivate->severity = qt_messageSeverityFromGL(messageSeverities[i]);
            messagePrivate->message = QString::fromUtf8(messagesBufferPtr, messageLengths[i] - 1);

            messagesBufferPtr += messageLengths[i];
            messages << message;
        }

    } while (messagesRead == maxMessageCount);

    return messages;
}

qint64 QOpenGLDebugLogger::maximumMessageLength() const
{
    Q_D(const QOpenGLDebugLogger);

    if (! d->initialized) {
        qWarning("QOpenGLDebugLogger::maximumMessageLength(): "
              "Object must be initialized before reading the maximum message length");
        return -1;
    }

    return d->maxMessageLength;
}

void QOpenGLDebugLogger::_q_contextAboutToBeDestroyed()
{
   Q_D(QOpenGLDebugLogger);
   d->_q_contextAboutToBeDestroyed();
}
