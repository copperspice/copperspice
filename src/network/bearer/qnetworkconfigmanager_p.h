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

#ifndef QNETWORKCONFIGMANAGER_P_H
#define QNETWORKCONFIGMANAGER_P_H

#include <qnetworkconfigmanager.h>
#include <qnetworkconfiguration_p.h>
#include <qmutex.h>
#include <qset.h>

#ifndef QT_NO_BEARERMANAGEMENT

class QBearerEngine;
class QTimer;

class Q_NETWORK_EXPORT QNetworkConfigurationManagerPrivate : public QObject
{
   NET_CS_OBJECT(QNetworkConfigurationManagerPrivate)

 public:
   QNetworkConfigurationManagerPrivate();
   virtual ~QNetworkConfigurationManagerPrivate();

   QNetworkConfiguration defaultConfiguration() const;
   QList<QNetworkConfiguration> allConfigurations(QNetworkConfiguration::StateFlags filter) const;
   QNetworkConfiguration configurationFromIdentifier(const QString &identifier) const;

   bool isOnline() const;

   QNetworkConfigurationManager::Capabilities capabilities() const;

   void performAsyncConfigurationUpdate();

   QList<QBearerEngine *> engines() const;

   void enablePolling();
   void disablePolling();

   void initialize();
   void cleanup();

   NET_CS_SLOT_1(Public, void updateConfigurations())
   NET_CS_SLOT_2(updateConfigurations)

   NET_CS_SLOT_1(Public, static void addPreAndPostRoutine())
   NET_CS_SLOT_2(addPreAndPostRoutine)

   NET_CS_SIGNAL_1(Public, void configurationAdded(const QNetworkConfiguration &config))
   NET_CS_SIGNAL_OVERLOAD(configurationAdded, (const QNetworkConfiguration &), config)

   NET_CS_SIGNAL_1(Public, void configurationRemoved(const QNetworkConfiguration &config))
   NET_CS_SIGNAL_OVERLOAD(configurationRemoved, (const QNetworkConfiguration &), config)

   NET_CS_SIGNAL_1(Public, void configurationChanged(const QNetworkConfiguration &config))
   NET_CS_SIGNAL_OVERLOAD(configurationChanged, (const QNetworkConfiguration &), config)

   NET_CS_SIGNAL_1(Public, void configurationUpdateComplete())
   NET_CS_SIGNAL_2(configurationUpdateComplete)


   NET_CS_SIGNAL_1(Public, void onlineStateChanged(bool isOnline))
   NET_CS_SIGNAL_2(onlineStateChanged, isOnline)

 private :
   NET_CS_SLOT_1(Private, void configurationAdded(QNetworkConfigurationPrivatePointer ptr))
   NET_CS_SLOT_OVERLOAD(configurationAdded, (QNetworkConfigurationPrivatePointer))

   NET_CS_SLOT_1(Private, void configurationRemoved(QNetworkConfigurationPrivatePointer ptr))
   NET_CS_SLOT_OVERLOAD(configurationRemoved, (QNetworkConfigurationPrivatePointer))

   NET_CS_SLOT_1(Private, void configurationChanged(QNetworkConfigurationPrivatePointer ptr))
   NET_CS_SLOT_OVERLOAD(configurationChanged, (QNetworkConfigurationPrivatePointer))

   NET_CS_SLOT_1(Private, void pollEngines())
   NET_CS_SLOT_2(pollEngines)

   NET_CS_INVOKABLE_METHOD_1(Private, void startPolling())
   NET_CS_INVOKABLE_METHOD_2(startPolling)

   QTimer *pollTimer;
   QThread *bearerThread;

   mutable QMutex mutex;

   QList<QBearerEngine *> sessionEngines;

   QSet<QString> onlineConfigurations;

   QSet<QBearerEngine *> pollingEngines;
   QSet<QBearerEngine *> updatingEngines;
   int forcedPolling;
   bool updating;

   bool firstUpdate;
};

Q_NETWORK_EXPORT QNetworkConfigurationManagerPrivate *qNetworkConfigurationManagerPrivate();


#endif // QT_NO_BEARERMANAGEMENT

#endif // QNETWORKCONFIGURATIONMANAGERPRIVATE_H
