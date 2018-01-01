/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QICDENGINE_H
#define QICDENGINE_H

#include <qbearerengine_p.h>
#include <QtCore/qtimer.h>
#include "maemo_icd.h"

QT_BEGIN_NAMESPACE

class QNetworkConfigurationPrivate;
class IapMonitor;
class QDBusInterface;
class QDBusServiceWatcher;

inline QNetworkConfiguration::BearerType bearerTypeFromIapType(const QString &iapType)
{
    if (iapType == QLatin1String("WLAN_INFRA") ||
        iapType == QLatin1String("WLAN_ADHOC")) {
        return QNetworkConfiguration::BearerWLAN;

    } else if (iapType == QLatin1String("GPRS")) {
        return QNetworkConfiguration::BearerHSPA;

    } else {
        return QNetworkConfiguration::BearerUnknown;
    }
}

class IcdNetworkConfigurationPrivate : public QNetworkConfigurationPrivate
{
public:
    IcdNetworkConfigurationPrivate();
    ~IcdNetworkConfigurationPrivate();

    virtual QString bearerTypeName() const;

    // In Maemo the id field (defined in QNetworkConfigurationPrivate)
    // is the IAP id (which typically is UUID)
    QByteArray network_id;  // typically WLAN ssid or similar
    QString iap_type;       // is this one WLAN or GPRS

    QString service_type;
    QString service_id;
    quint32 service_attrs;

    // Network attributes for this IAP, this is the value returned by icd and
    // passed to it when connecting.
    quint32 network_attrs;
};

inline IcdNetworkConfigurationPrivate *toIcdConfig(QNetworkConfigurationPrivatePointer ptr)
{
    return static_cast<IcdNetworkConfigurationPrivate *>(ptr.data());
}

class QIcdEngine : public QBearerEngine
{
    Q_OBJECT

public:
    QIcdEngine(QObject *parent = nullptr);
    ~QIcdEngine();

    bool hasIdentifier(const QString &id);

    Q_INVOKABLE void initialize();
    Q_INVOKABLE void requestUpdate();

    QNetworkConfigurationManager::Capabilities capabilities() const;

    QNetworkSessionPrivate *createSessionBackend();

    QNetworkConfigurationPrivatePointer defaultConfiguration();

    void deleteConfiguration(const QString &iap_id);

    inline QNetworkConfigurationPrivatePointer configuration(const QString &id)
    {
        QMutexLocker locker(&mutex);

        return accessPointConfigurations.value(id);
    }

    inline void addSessionConfiguration(QNetworkConfigurationPrivatePointer ptr)
    {
        QMutexLocker locker(&mutex);

        accessPointConfigurations.insert(ptr->id, ptr);

        locker.unlock();
        emit configurationAdded(ptr);
    }

    inline void changedSessionConfiguration(QNetworkConfigurationPrivatePointer ptr)
    {
        emit configurationChanged(ptr);
    }

    void cleanup();

    void addConfiguration(QString &iap_id);

Q_SIGNALS:
    void iapStateChanged(const QString& iapid, uint icd_connection_state);

private Q_SLOTS:
    void finishAsyncConfigurationUpdate();
    void asyncUpdateConfigurationsSlot(QDBusMessage msg);
    void connectionStateSignalsSlot(QDBusMessage msg);
    void icdServiceOwnerChanged(const QString &serviceName, const QString &oldOwner,
                                const QString &newOwner);

private:
    void startListeningStateSignalsForAllConnections();
    void doRequestUpdate(QList<Maemo::IcdScanResult> scanned = QList<Maemo::IcdScanResult>());
    void cancelAsyncConfigurationUpdate();
    void getIcdInitialState();
    bool ensureDBusConnection();

private:
    IapMonitor *iapMonitor;
    QDBusInterface *m_dbusInterface;
    QTimer m_scanTimer;
    QString m_onlineIapId;
    QStringList m_typesToBeScanned;
    QList<Maemo::IcdScanResult> m_scanResult;

    QDBusServiceWatcher *m_icdServiceWatcher;

    bool firstUpdate;
    bool m_scanGoingOn;
};

QT_END_NAMESPACE

#endif // QICDENGINE_H
