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

#ifndef QDBUSMESSAGE_P_H
#define QDBUSMESSAGE_P_H

#include <qatomic.h>
#include <qstring.h>
#include <qdbusmessage.h>
#include <qdbusconnection.h>

struct DBusMessage;

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QDBusConnectionPrivate;

class QDBusMessagePrivate
{
public:
    QDBusMessagePrivate();
    ~QDBusMessagePrivate();

    QList<QVariant> arguments;

    // the following parameters are "const": they are not changed after the constructors
    // the parametersValidated member below controls whether they've been validated already
    QString service, path, interface, name, message, signature;

    DBusMessage *msg;
    DBusMessage *reply;
    int type;
    int timeout;
    mutable QDBusMessage *localReply;
    QAtomicInt ref;

    mutable uint delayedReply : 1;
    uint localMessage : 1;
    mutable uint parametersValidated : 1;
    uint autoStartService : 1;

    static void setParametersValidated(QDBusMessage &msg, bool enable)
    { msg.d_ptr->parametersValidated = enable; }

    static DBusMessage *toDBusMessage(const QDBusMessage &message, QDBusConnection::ConnectionCapabilities capabilities,
                                      QDBusError *error);
    static QDBusMessage fromDBusMessage(DBusMessage *dmsg, QDBusConnection::ConnectionCapabilities capabilities);

    static bool isLocal(const QDBusMessage &msg);
    static QDBusMessage makeLocal(const QDBusConnectionPrivate &conn,
                                  const QDBusMessage &asSent);
    static QDBusMessage makeLocalReply(const QDBusConnectionPrivate &conn,
                                       const QDBusMessage &asSent);
};

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif
