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

#ifndef QDBUSERROR_H
#define QDBUSERROR_H

#include <QtDBus/qdbusmacros.h>
#include <QtCore/qstring.h>

#ifndef QT_NO_DBUS

struct DBusError;

QT_BEGIN_NAMESPACE

class QDBusMessage;

class Q_DBUS_EXPORT QDBusError
{
public:
    enum ErrorType {
        NoError = 0,
        Other = 1,
        Failed,
        NoMemory,
        ServiceUnknown,
        NoReply,
        BadAddress,
        NotSupported,
        LimitsExceeded,
        AccessDenied,
        NoServer,
        Timeout,
        NoNetwork,
        AddressInUse,
        Disconnected,
        InvalidArgs,
        UnknownMethod,
        TimedOut,
        InvalidSignature,
        UnknownInterface,
        InternalError,
        UnknownObject,
        InvalidService,
        InvalidObjectPath,
        InvalidInterface,
        InvalidMember,

        // don't use this one!
        LastErrorType = InvalidMember
    };

    QDBusError(const DBusError *error = 0);
    QDBusError(const QDBusMessage& msg);
    QDBusError(ErrorType error, const QString &message);
    QDBusError(const QDBusError &other);
    QDBusError &operator=(const QDBusError &other);

    ErrorType type() const;
    QString name() const;
    QString message() const;
    bool isValid() const;

    static QString errorString(ErrorType error);

private:
    ErrorType code;
    QString msg;
    QString nm;
    void *unused;
};

Q_DBUS_EXPORT QDebug operator<<(QDebug, const QDBusError &);

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif
