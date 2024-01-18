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

#ifndef QCOREWLANENGINE_H
#define QCOREWLANENGINE_H

#include "../qbearerengine_impl.h"

#include <QMap>
#include <QTimer>
#include <SystemConfiguration/SystemConfiguration.h>
#include <QThread>

#ifndef QT_NO_BEARERMANAGEMENT

QT_BEGIN_NAMESPACE

class QNetworkConfigurationPrivate;
class QScanThread;

class QCoreWlanEngine : public QBearerEngineImpl
{
     friend class QScanThread;
    Q_OBJECT

public:
    QCoreWlanEngine(QObject *parent = nullptr);
    ~QCoreWlanEngine();

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

    bool requiresPolling() const;

private Q_SLOTS:
    void doRequestUpdate();
    void networksChanged();

private:
    bool isWifiReady(const QString &dev);
    QList<QNetworkConfigurationPrivate *> foundConfigurations;

    SCDynamicStoreRef storeSession;
    CFRunLoopSourceRef runloopSource;
    bool hasWifi;
    bool scanning;
    QScanThread *scanThread;

    quint64 getBytes(const QString &interfaceName,bool b);

protected:
    void startNetworkChangeLoop();

};

class QScanThread : public QThread
{
    Q_OBJECT

public:
    QScanThread(QObject *parent = nullptr);
    ~QScanThread();

    void quit();
    QList<QNetworkConfigurationPrivate *> getConfigurations();
    QString interfaceName;
    QMap<QString, QString> configurationInterface;
    void getUserConfigurations();
    QString getNetworkNameFromSsid(const QString &ssid);
    QString getSsidFromNetworkName(const QString &name);
    bool isKnownSsid(const QString &ssid);
    QMap<QString, QMap<QString,QString> > userProfiles;

signals:
    void networksChanged();

protected:
    void run();

private:
    QList<QNetworkConfigurationPrivate *> fetchedConfigurations;
    QMutex mutex;
    QStringList foundNetwork(const QString &id, const QString &ssid, const QNetworkConfiguration::StateFlags state, const QString &interfaceName, const QNetworkConfiguration::Purpose purpose);

};

QT_END_NAMESPACE

#endif // QT_NO_BEARERMANAGEMENT

#endif
