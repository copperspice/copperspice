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

#ifndef QCONNMANENGINE_P_H
#define QCONNMANENGINE_P_H

#include "../qbearerengine_impl.h"
#include "qconnmanservice_linux_p.h"

#include <QMap>
#include <QVariant>

#ifndef QT_NO_BEARERMANAGEMENT
#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QConnmanEngine : public QBearerEngineImpl
{
    Q_OBJECT

public:
    QConnmanEngine(QObject *parent = nullptr);
    ~QConnmanEngine();

    bool connmanAvailable() const;

    virtual QString getInterfaceFromId(const QString &id);
    bool hasIdentifier(const QString &id);

    virtual void connectToId(const QString &id);
    virtual void disconnectFromId(const QString &id);

    Q_INVOKABLE void initialize();
    Q_INVOKABLE void requestUpdate();

    QNetworkSession::State sessionStateForId(const QString &id);
    QNetworkSessionPrivate *createSessionBackend();

    virtual quint64 bytesWritten(const QString &id);
    virtual quint64 bytesReceived(const QString &id);
    virtual quint64 startTime(const QString &id);


    virtual QNetworkConfigurationManager::Capabilities capabilities() const;
    virtual QNetworkConfigurationPrivatePointer defaultConfiguration();

    void configurationChange(const QString &id);
    QList<QNetworkConfigurationPrivate *> getConfigurations();


private Q_SLOTS:

    void doRequestUpdate();
    void servicePropertyChangedContext(const QString &,const QString &,const QDBusVariant &);
    void propertyChangedContext(const QString &,const QString &,const QDBusVariant &);
    void technologyPropertyChangedContext(const QString &,const QString &, const QDBusVariant &);

private:
    QConnmanManagerInterface *connmanManager;

    QList<QNetworkConfigurationPrivate *> foundConfigurations;

    QString serviceFromId(const QString &id);
    QString networkFromId(const QString &id);

    QNetworkConfiguration::StateFlags getStateForService(const QString &service);
    QNetworkConfiguration::BearerType typeToBearer(const QString &type);

    void removeConfiguration(const QString &servicePath);
    void addServiceConfiguration(const QString &servicePath);
    QDateTime activeTime;


    QMap<QString,QConnmanTechnologyInterface *> technologies; // techpath, tech interface
    QMap<QString,QString> configInterfaces; // id, interface name
    QList<QString> serviceNetworks; //servpath

    QNetworkConfiguration::BearerType ofonoTechToBearerType(const QString &type);
    bool isRoamingAllowed(const QString &context);
protected:
    bool requiresPolling() const;
};


QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif // QT_NO_BEARERMANAGEMENT

#endif

