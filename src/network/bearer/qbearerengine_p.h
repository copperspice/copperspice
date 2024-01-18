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

#ifndef QBEARERENGINE_P_H
#define QBEARERENGINE_P_H

#include <qobject.h>
#include <qglobal.h>
#include <qlist.h>
#include <qstring.h>
#include <qhash.h>
#include <qsharedpointer.h>
#include <qmutex.h>
#include <qnetworksession.h>
#include <qnetworkconfigmanager.h>

#include <qnetworkconfiguration_p.h>

#ifndef QT_NO_BEARERMANAGEMENT

class QNetworkConfiguration;

class Q_NETWORK_EXPORT QBearerEngine : public QObject
{
   NET_CS_OBJECT(QBearerEngine)

   friend class QNetworkConfigurationManagerPrivate;

 public:
   explicit QBearerEngine(QObject *parent = nullptr);
   virtual ~QBearerEngine();

   virtual bool hasIdentifier(const QString &id) = 0;

   virtual QNetworkConfigurationManager::Capabilities capabilities() const = 0;

   virtual QNetworkSessionPrivate *createSessionBackend() = 0;

   virtual QNetworkConfigurationPrivatePointer defaultConfiguration() = 0;

   virtual bool requiresPolling() const;
   bool configurationsInUse() const;

   NET_CS_SIGNAL_1(Public, void configurationAdded(QNetworkConfigurationPrivatePointer config))
   NET_CS_SIGNAL_2(configurationAdded, config)

   NET_CS_SIGNAL_1(Public, void configurationRemoved(QNetworkConfigurationPrivatePointer config))
   NET_CS_SIGNAL_2(configurationRemoved, config)

   NET_CS_SIGNAL_1(Public, void configurationChanged(QNetworkConfigurationPrivatePointer config))
   NET_CS_SIGNAL_2(configurationChanged, config)

   NET_CS_SIGNAL_1(Public, void updateCompleted())
   NET_CS_SIGNAL_2(updateCompleted)

 protected:
   //this table contains an up to date list of all configs at any time.
   //it must be updated if configurations change, are added/removed or
   //the members of ServiceNetworks change
   QHash<QString, QNetworkConfigurationPrivatePointer> accessPointConfigurations;
   QHash<QString, QNetworkConfigurationPrivatePointer> snapConfigurations;
   QHash<QString, QNetworkConfigurationPrivatePointer> userChoiceConfigurations;

   mutable QRecursiveMutex mutex;
};

#endif // QT_NO_BEARERMANAGEMENT

#endif // QBEARERENGINE_P_H
