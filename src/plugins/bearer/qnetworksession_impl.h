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

#ifndef QNETWORKSESSION_IMPL_H
#define QNETWORKSESSION_IMPL_H

#include <qbearerengine_impl.h>

#include <qnetworkconfigmanager_p.h>
#include <qnetworksession_p.h>

#ifndef QT_NO_BEARERMANAGEMENT

class QBearerEngineImpl;

class QNetworkSessionPrivateImpl : public QNetworkSessionPrivate
{
   CS_OBJECT(QNetworkSessionPrivateImpl)

 public:
   QNetworkSessionPrivateImpl()
      : startTime(0), sessionTimeout(-1)
   {
   }

   ~QNetworkSessionPrivateImpl()
   {
   }

    //called by QNetworkSession constructor and ensures
    //that the state is immediately updated (w/o actually opening
    //a session). Also this function should take care of
    //notification hooks to discover future state changes.
    void syncStateWithInterface();

#ifndef QT_NO_NETWORKINTERFACE
    QNetworkInterface currentInterface() const;
#endif

    QVariant sessionProperty(const QString& key) const;
    void setSessionProperty(const QString& key, const QVariant& value);

    void open();
    void close();
    void stop();
    void migrate();
    void accept();
    void ignore();
    void reject();

    QString errorString() const; //must return translated string
    QNetworkSession::SessionError error() const;

    quint64 bytesWritten() const;
    quint64 bytesReceived() const;
    quint64 activeTime() const;

 private:
    CS_SLOT_1(Private, void networkConfigurationsChanged())
    CS_SLOT_2(networkConfigurationsChanged)

    CS_SLOT_1(Private, void configurationChanged(QNetworkConfigurationPrivatePointer config))
    CS_SLOT_2(configurationChanged)

    CS_SLOT_1(Private, void forcedSessionClose(const QNetworkConfiguration &config))
    CS_SLOT_2(forcedSessionClose)

    CS_SLOT_1(Private, void connectionError(const QString &id, QBearerEngineImpl::ConnectionError error))
    CS_SLOT_2(connectionError)

    CS_SLOT_1(Private, void decrementTimeout())
    CS_SLOT_2(decrementTimeout)

    void updateStateFromServiceNetwork();
    void updateStateFromActiveConfig();

    QBearerEngineImpl *engine;

    quint64 startTime;
    QNetworkSession::SessionError lastError;
    int sessionTimeout;
    bool opened;
};

#endif // QT_NO_BEARERMANAGEMENT

#endif
