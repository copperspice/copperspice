/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
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

#ifndef QDBUSMESSAGE_H
#define QDBUSMESSAGE_H

#include <QtDBus/qdbusmacros.h>
#include <QtDBus/qdbuserror.h>
#include <QtCore/qlist.h>
#include <QtCore/qvariant.h>

#ifndef QT_NO_DBUS

#if defined(Q_OS_WIN) && defined(interface)
#  undef interface
#endif

QT_BEGIN_NAMESPACE

class QDBusMessagePrivate;
class Q_DBUS_EXPORT QDBusMessage
{
public:
    enum MessageType {
        InvalidMessage,
        MethodCallMessage,
        ReplyMessage,
        ErrorMessage,
        SignalMessage
    };

    QDBusMessage();
    QDBusMessage(const QDBusMessage &other);
    QDBusMessage &operator=(const QDBusMessage &other);
    ~QDBusMessage();

    static QDBusMessage createSignal(const QString &path, const QString &interface,
                                     const QString &name);
    static QDBusMessage createMethodCall(const QString &destination, const QString &path,
                                         const QString &interface, const QString &method);
    static QDBusMessage createError(const QString &name, const QString &msg);
    static inline QDBusMessage createError(const QDBusError &err)
    { return createError(err.name(), err.message()); }
    static inline QDBusMessage createError(QDBusError::ErrorType type, const QString &msg)
    { return createError(QDBusError::errorString(type), msg); }

    QDBusMessage createReply(const QList<QVariant> &arguments = QList<QVariant>()) const;
    inline QDBusMessage createReply(const QVariant &argument) const
    { return createReply(QList<QVariant>() << argument); }

    QDBusMessage createErrorReply(const QString name, const QString &msg) const;
    inline QDBusMessage createErrorReply(const QDBusError &err) const
    { return createErrorReply(err.name(), err.message()); }
    QDBusMessage createErrorReply(QDBusError::ErrorType type, const QString &msg) const;

    // there are no setters; if this changes, see qdbusmessage_p.h
    QString service() const;
    QString path() const;
    QString interface() const;
    QString member() const;
    QString errorName() const;
    QString errorMessage() const;
    MessageType type() const;
    QString signature() const;

    bool isReplyRequired() const;

    void setDelayedReply(bool enable) const;
    bool isDelayedReply() const;

    void setAutoStartService(bool enable);
    bool autoStartService() const;

    void setArguments(const QList<QVariant> &arguments);
    QList<QVariant> arguments() const;

    QDBusMessage &operator<<(const QVariant &arg);

private:
    friend class QDBusMessagePrivate;
    QDBusMessagePrivate *d_ptr;
};

Q_DBUS_EXPORT QDebug operator<<(QDebug, const QDBusMessage &);


QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif

