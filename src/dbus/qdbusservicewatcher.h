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

#ifndef QDBUSSERVICEWATCHER_H
#define QDBUSSERVICEWATCHER_H

#include <QtCore/qobject.h>
#include <QtDBus/qdbusmacros.h>

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QDBusConnection;
class QDBusServiceWatcherPrivate;

class Q_DBUS_EXPORT QDBusServiceWatcher: public QObject
{
    CS_OBJECT(QDBusServiceWatcher)
    CS_PROPERTY_READ(watchedServices, watchedServices)
    CS_PROPERTY_WRITE(watchedServices, setWatchedServices)
    CS_PROPERTY_READ(watchMode, watchMode)
    CS_PROPERTY_WRITE(watchMode, setWatchMode)
public:
    enum WatchModeFlag {
        WatchForRegistration = 0x01,
        WatchForUnregistration = 0x02,
        WatchForOwnerChange = 0x03
    };
    using WatchMode = QFlags<WatchModeFlag>;

    explicit QDBusServiceWatcher(QObject *parent = nullptr);
    QDBusServiceWatcher(const QString &service, const QDBusConnection &connection,
                        WatchMode watchMode = WatchForOwnerChange, QObject *parent = nullptr);
    ~QDBusServiceWatcher();

    QStringList watchedServices() const;
    void setWatchedServices(const QStringList &services);
    void addWatchedService(const QString &newService);
    bool removeWatchedService(const QString &service);

    WatchMode watchMode() const;
    void setWatchMode(WatchMode mode);

    QDBusConnection connection() const;
    void setConnection(const QDBusConnection &connection);

public:
    CS_SIGNAL_1(Public, void serviceRegistered(const QString & service))
    CS_SIGNAL_2(serviceRegistered,service) 
    CS_SIGNAL_1(Public, void serviceUnregistered(const QString & service))
    CS_SIGNAL_2(serviceUnregistered,service) 
    CS_SIGNAL_1(Public, void serviceOwnerChanged(const QString & service,const QString & oldOwner,const QString & newOwner))
    CS_SIGNAL_2(serviceOwnerChanged,service,oldOwner,newOwner) 

private:
    CS_SLOT_1(Private, void _q_serviceOwnerChanged(QString un_named_arg1,QString un_named_arg2,QString un_named_arg3))
    CS_SLOT_2(_q_serviceOwnerChanged)

/*  PRIVATE_SLOT
void _q_serviceOwnerChanged(QString un_named_arg1,QString un_named_arg2,QString un_named_arg3)
{
	Q_D(QDBusServiceWatcher);
	d->_q_serviceOwnerChanged();
}
*/
    Q_DISABLE_COPY(QDBusServiceWatcher)
    Q_DECLARE_PRIVATE(QDBusServiceWatcher)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDBusServiceWatcher::WatchMode)

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif // QDBUSSERVICEWATCHER_H
