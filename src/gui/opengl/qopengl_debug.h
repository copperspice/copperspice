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

#ifndef QOPENGLDEBUG_H
#define QOPENGLDEBUG_H

#include <qglobal.h>

#ifndef QT_NO_OPENGL

#include <qshareddata.h>
#include <qflags.h>
#include <qlist.h>
#include <qvector.h>
#include <qdebug.h>
#include <qopenglcontext.h>

class QOpenGLDebugLogger;
class QOpenGLDebugLoggerPrivate;
class QOpenGLDebugMessagePrivate;

class Q_GUI_EXPORT QOpenGLDebugMessage
{
public:
    enum Source {
        InvalidSource        = 0x00000000,
        APISource            = 0x00000001,
        WindowSystemSource   = 0x00000002,
        ShaderCompilerSource = 0x00000004,
        ThirdPartySource     = 0x00000008,
        ApplicationSource    = 0x00000010,
        OtherSource          = 0x00000020,
        LastSource           = OtherSource, // private API
        AnySource            = 0xffffffff
    };
    using Sources = QFlags<Source>;

    enum Type {
        InvalidType            = 0x00000000,
        ErrorType              = 0x00000001,
        DeprecatedBehaviorType = 0x00000002,
        UndefinedBehaviorType  = 0x00000004,
        PortabilityType        = 0x00000008,
        PerformanceType        = 0x00000010,
        OtherType              = 0x00000020,
        MarkerType             = 0x00000040,
        GroupPushType          = 0x00000080,
        GroupPopType           = 0x00000100,
        LastType               = GroupPopType, // private API
        AnyType                = 0xffffffff
    };
    using Types = QFlags<Type>;

    enum Severity {
        InvalidSeverity      = 0x00000000,
        HighSeverity         = 0x00000001,
        MediumSeverity       = 0x00000002,
        LowSeverity          = 0x00000004,
        NotificationSeverity = 0x00000008,
        LastSeverity         = NotificationSeverity, // private API
        AnySeverity          = 0xffffffff
    };
    using Severities = QFlags<Severity>;

    QOpenGLDebugMessage();
    QOpenGLDebugMessage(const QOpenGLDebugMessage &debugMessage);

    QOpenGLDebugMessage &operator=(const QOpenGLDebugMessage &debugMessage);

    QOpenGLDebugMessage &operator=(QOpenGLDebugMessage &&other)  { swap(other); return *this; }

    ~QOpenGLDebugMessage();

    void swap(QOpenGLDebugMessage &other)  { qSwap(d, other.d); }

    Source source() const;
    Type type() const;
    Severity severity() const;
    GLuint id() const;
    QString message() const;

    static QOpenGLDebugMessage createApplicationMessage(const QString &text,
                                                        GLuint id = 0,
                                                        Severity severity = NotificationSeverity,
                                                        Type type = OtherType);
    static QOpenGLDebugMessage createThirdPartyMessage(const QString &text,
                                                       GLuint id = 0,
                                                       Severity severity = NotificationSeverity,
                                                       Type type = OtherType);

    bool operator==(const QOpenGLDebugMessage &debugMessage) const;
    inline bool operator!=(const QOpenGLDebugMessage &debugMessage) const { return !operator==(debugMessage); }

private:
    friend class QOpenGLDebugLogger;
    friend class QOpenGLDebugLoggerPrivate;
    QSharedDataPointer<QOpenGLDebugMessagePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QOpenGLDebugMessage::Sources)
Q_DECLARE_OPERATORS_FOR_FLAGS(QOpenGLDebugMessage::Types)
Q_DECLARE_OPERATORS_FOR_FLAGS(QOpenGLDebugMessage::Severities)

Q_GUI_EXPORT QDebug operator<<(QDebug debug, const QOpenGLDebugMessage &message);
Q_GUI_EXPORT QDebug operator<<(QDebug debug, QOpenGLDebugMessage::Source source);
Q_GUI_EXPORT QDebug operator<<(QDebug debug, QOpenGLDebugMessage::Type type);
Q_GUI_EXPORT QDebug operator<<(QDebug debug, QOpenGLDebugMessage::Severity severity);

class QOpenGLDebugLoggerPrivate;

class Q_GUI_EXPORT QOpenGLDebugLogger : public QObject
{
    GUI_CS_OBJECT(QOpenGLDebugLogger)

    GUI_CS_ENUM(LoggingMode)
    GUI_CS_PROPERTY_READ(loggingMode, loggingMode)

 public:
    enum LoggingMode {
        AsynchronousLogging,
        SynchronousLogging
    };

    explicit QOpenGLDebugLogger(QObject *parent = nullptr);

    QOpenGLDebugLogger(const QOpenGLDebugLogger &) = delete;
    QOpenGLDebugLogger &operator=(const QOpenGLDebugLogger &) = delete;

    ~QOpenGLDebugLogger();

    bool initialize();

    bool isLogging() const;
    LoggingMode loggingMode() const;

    qint64 maximumMessageLength() const;

    void pushGroup(const QString &name, GLuint id = 0,
                        QOpenGLDebugMessage::Source source = QOpenGLDebugMessage::ApplicationSource);

    void popGroup();

    void enableMessages(QOpenGLDebugMessage::Sources sources = QOpenGLDebugMessage::AnySource,
                        QOpenGLDebugMessage::Types types = QOpenGLDebugMessage::AnyType,
                        QOpenGLDebugMessage::Severities severities = QOpenGLDebugMessage::AnySeverity);

    void enableMessages(const QVector<GLuint> &ids,
                        QOpenGLDebugMessage::Sources sources = QOpenGLDebugMessage::AnySource,
                        QOpenGLDebugMessage::Types types = QOpenGLDebugMessage::AnyType);

    void disableMessages(QOpenGLDebugMessage::Sources sources = QOpenGLDebugMessage::AnySource,
                        QOpenGLDebugMessage::Types types = QOpenGLDebugMessage::AnyType,
                        QOpenGLDebugMessage::Severities severities = QOpenGLDebugMessage::AnySeverity);

    void disableMessages(const QVector<GLuint> &ids,
                        QOpenGLDebugMessage::Sources sources = QOpenGLDebugMessage::AnySource,
                        QOpenGLDebugMessage::Types types = QOpenGLDebugMessage::AnyType);

    QList<QOpenGLDebugMessage> loggedMessages() const;

    GUI_CS_SLOT_1(Public, void logMessage(const QOpenGLDebugMessage & debugMessage))
    GUI_CS_SLOT_2(logMessage)

    GUI_CS_SLOT_1(Public, void startLogging(LoggingMode loggingMode = AsynchronousLogging))
    GUI_CS_SLOT_2(startLogging)

    GUI_CS_SLOT_1(Public, void stopLogging())
    GUI_CS_SLOT_2(stopLogging)

    GUI_CS_SIGNAL_1(Public, void messageLogged(const QOpenGLDebugMessage & debugMessage))
    GUI_CS_SIGNAL_2(messageLogged,debugMessage)

 protected:
   QScopedPointer<QOpenGLDebugLoggerPrivate> d_ptr;

 private:
    Q_DECLARE_PRIVATE(QOpenGLDebugLogger)

    GUI_CS_SLOT_1(Private, void _q_contextAboutToBeDestroyed())
    GUI_CS_SLOT_2(_q_contextAboutToBeDestroyed)
};

#endif // QT_NO_OPENGL

#endif
