/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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
    Q_OBJECT
    Q_PROPERTY(QStringList watchedServices READ watchedServices WRITE setWatchedServices)
    Q_PROPERTY(WatchMode watchMode READ watchMode WRITE setWatchMode)
public:
    enum WatchModeFlag {
        WatchForRegistration = 0x01,
        WatchForUnregistration = 0x02,
        WatchForOwnerChange = 0x03
    };
    Q_DECLARE_FLAGS(WatchMode, WatchModeFlag)

    explicit QDBusServiceWatcher(QObject *parent = 0);
    QDBusServiceWatcher(const QString &service, const QDBusConnection &connection,
                        WatchMode watchMode = WatchForOwnerChange, QObject *parent = 0);
    ~QDBusServiceWatcher();

    QStringList watchedServices() const;
    void setWatchedServices(const QStringList &services);
    void addWatchedService(const QString &newService);
    bool removeWatchedService(const QString &service);

    WatchMode watchMode() const;
    void setWatchMode(WatchMode mode);

    QDBusConnection connection() const;
    void setConnection(const QDBusConnection &connection);

Q_SIGNALS:
    void serviceRegistered(const QString &service);
    void serviceUnregistered(const QString &service);
    void serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_serviceOwnerChanged(QString,QString,QString))
    Q_DISABLE_COPY(QDBusServiceWatcher)
    Q_DECLARE_PRIVATE(QDBusServiceWatcher)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDBusServiceWatcher::WatchMode)

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif // QDBUSSERVICEWATCHER_H
