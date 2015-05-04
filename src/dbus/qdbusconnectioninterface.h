/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDBUSBUS_H
#define QDBUSBUS_H

#include <QtCore/qstringlist.h>
#include <QtDBus/qdbusabstractinterface.h>
#include <QtDBus/qdbusreply.h>

#ifndef QT_NO_DBUS


QT_BEGIN_NAMESPACE

class QDBusConnection;
class QString;
class QByteArray;

/*
 * Proxy class for interface org.freedesktop.DBus
 */
class Q_DBUS_EXPORT QDBusConnectionInterface: public QDBusAbstractInterface
{
    Q_OBJECT
    Q_ENUMS(ServiceQueueOptions ServiceReplacementOptions RegisterServiceReply)
    friend class QDBusConnectionPrivate;
    static inline const char *staticInterfaceName();

    explicit QDBusConnectionInterface(const QDBusConnection &connection, QObject *parent);
    ~QDBusConnectionInterface();

    Q_PROPERTY(QDBusReply<QStringList> registeredServiceNames READ registeredServiceNames)

public:
    enum ServiceQueueOptions {
        DontQueueService,
        QueueService,
        ReplaceExistingService
    };
    enum ServiceReplacementOptions {
        DontAllowReplacement,
        AllowReplacement
    };
    enum RegisterServiceReply {
        ServiceNotRegistered = 0,
        ServiceRegistered,
        ServiceQueued
    };

public Q_SLOTS:
    QDBusReply<QStringList> registeredServiceNames() const;
    QDBusReply<bool> isServiceRegistered(const QString &serviceName) const;
    QDBusReply<QString> serviceOwner(const QString &name) const;
    QDBusReply<bool> unregisterService(const QString &serviceName);
    QDBusReply<QDBusConnectionInterface::RegisterServiceReply> registerService(const QString &serviceName,
                                                     ServiceQueueOptions qoption = DontQueueService,
                                                     ServiceReplacementOptions roption = DontAllowReplacement);

    QDBusReply<uint> servicePid(const QString &serviceName) const;
    QDBusReply<uint> serviceUid(const QString &serviceName) const;

    QDBusReply<void> startService(const QString &name);

Q_SIGNALS:
    void serviceRegistered(const QString &service);
    void serviceUnregistered(const QString &service);
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
    void callWithCallbackFailed(const QDBusError &error, const QDBusMessage &call);

    // internal signals
    // do not use
    void NameAcquired(const QString &);
    void NameLost(const QString &);
    void NameOwnerChanged(const QString &, const QString &, const QString &);

protected:
    void connectNotify(const char *);
    void disconnectNotify(const char *);

};

QT_END_NAMESPACE

Q_DECLARE_BUILTIN_METATYPE(QDBusConnectionInterface::RegisterServiceReply, UInt)

#endif // QT_NO_DBUS
#endif
