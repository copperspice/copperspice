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

#ifndef QNLAENGINE_P_H
#define QNLAENGINE_P_H

#include "../qbearerengine_impl.h"
#include <qnativesocketengine_p.h>
#include <QMap>

QT_BEGIN_NAMESPACE

class QNetworkConfigurationPrivate;
class QNlaThread;

class QWindowsSockInit2
{
public:
    QWindowsSockInit2();
    ~QWindowsSockInit2();
    int version;
};

class QNlaEngine : public QBearerEngineImpl
{
    Q_OBJECT

    friend class QNlaThread;

public:
    QNlaEngine(QObject *parent = nullptr);
    ~QNlaEngine();

    QString getInterfaceFromId(const QString &id);
    bool hasIdentifier(const QString &id);

    void connectToId(const QString &id);
    void disconnectFromId(const QString &id);

    Q_INVOKABLE void requestUpdate();

    QNetworkSession::State sessionStateForId(const QString &id);

    QNetworkConfigurationManager::Capabilities capabilities() const;

    QNetworkSessionPrivate *createSessionBackend();

    QNetworkConfigurationPrivatePointer defaultConfiguration();

private Q_SLOTS:
    void networksChanged();

private:
    QWindowsSockInit2 winSock;
    QNlaThread *nlaThread;
    QMap<uint, QString> configurationInterface;
};

QT_END_NAMESPACE

#endif
