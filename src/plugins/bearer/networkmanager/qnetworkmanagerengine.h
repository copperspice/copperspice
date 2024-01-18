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

#ifndef QNETWORKMANAGERENGINE_P_H
#define QNETWORKMANAGERENGINE_P_H

#include "../qbearerengine_impl.h"
#include "qnetworkmanagerservice.h"

#include <QMap>
#include <QVariant>

#ifndef QT_NO_BEARERMANAGEMENT
#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QNetworkManagerEngine : public QBearerEngineImpl
{
    Q_OBJECT

public:
    QNetworkManagerEngine(QObject *parent = nullptr);
    ~QNetworkManagerEngine();

    bool networkManagerAvailable() const;

    QString getInterfaceFromId(const QString &id);
    bool hasIdentifier(const QString &id);

    void connectToId(const QString &id);
    void disconnectFromId(const QString &id);

    Q_INVOKABLE void initialize();
    Q_INVOKABLE void requestUpdate();

    QNetworkSession::State sessionStateForId(const QString &id);

    quint64 bytesWritten(const QString &id);
    quint64 bytesReceived(const QString &id);
    quint64 startTime(const QString &id);

    QNetworkConfigurationManager::Capabilities capabilities() const;

    QNetworkSessionPrivate *createSessionBackend();

    QNetworkConfigurationPrivatePointer defaultConfiguration();

private Q_SLOTS:
    void interfacePropertiesChanged(const QString &path,
                                    const QMap<QString, QVariant> &properties);
    void activeConnectionPropertiesChanged(const QString &path,
                                           const QMap<QString, QVariant> &properties);
    void devicePropertiesChanged(const QString &path,
                                 const QMap<QString, QVariant> &properties);

    void deviceAdded(const QDBusObjectPath &path);
    void deviceRemoved(const QDBusObjectPath &path);

    void newConnection(const QDBusObjectPath &path, QNetworkManagerSettings *settings = 0);
    void removeConnection(const QString &path);
    void updateConnection(const QNmSettingsMap &settings);
    void activationFinished(QDBusPendingCallWatcher *watcher);

    void newAccessPoint(const QString &path, const QDBusObjectPath &objectPath);
    void removeAccessPoint(const QString &path, const QDBusObjectPath &objectPath);
    void updateAccessPoint(const QMap<QString, QVariant> &map);

private:
    QNetworkConfigurationPrivate *parseConnection(const QString &service,
                                                  const QString &settingsPath,
                                                  const QNmSettingsMap &map);
    QNetworkManagerSettingsConnection *connectionFromId(const QString &id) const;

private:
    QNetworkManagerInterface *interface;
    QNetworkManagerSettings *systemSettings;
    QNetworkManagerSettings *userSettings;
    QHash<QString, QNetworkManagerInterfaceDeviceWireless *> wirelessDevices;
    QHash<QString, QNetworkManagerConnectionActive *> activeConnections;
    QList<QNetworkManagerSettingsConnection *> connections;
    QList<QNetworkManagerInterfaceAccessPoint *> accessPoints;
    QList<QNetworkManagerInterfaceAccessPoint *> configuredAccessPoints;
};

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif // QT_NO_BEARERMANAGEMENT

#endif

