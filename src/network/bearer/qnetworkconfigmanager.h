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

#ifndef QNETWORKCONFIGMANAGER_H
#define QNETWORKCONFIGMANAGER_H

#include <qobject.h>
#include <qnetworkconfiguration.h>

#ifndef QT_NO_BEARERMANAGEMENT


class QNetworkConfigurationManagerPrivate;

class Q_NETWORK_EXPORT QNetworkConfigurationManager : public QObject
{
   NET_CS_OBJECT(QNetworkConfigurationManager)

 public:
   enum Capability {
      CanStartAndStopInterfaces  = 0x00000001,
      DirectConnectionRouting = 0x00000002,
      SystemSessionSupport = 0x00000004,
      ApplicationLevelRoaming = 0x00000008,
      ForcedRoaming = 0x00000010,
      DataStatistics = 0x00000020,
      NetworkSessionRequired = 0x00000040
   };
   using Capabilities = QFlags<Capability>;

   explicit QNetworkConfigurationManager(QObject *parent = nullptr);
   virtual ~QNetworkConfigurationManager();

   QNetworkConfigurationManager::Capabilities capabilities() const;

   QNetworkConfiguration defaultConfiguration() const;
   QList<QNetworkConfiguration> allConfigurations(QNetworkConfiguration::StateFlags flags = QNetworkConfiguration::StateFlags()) const;
   QNetworkConfiguration configurationFromIdentifier(const QString &identifier) const;

   bool isOnline() const;

   NET_CS_SLOT_1(Public, void updateConfigurations())
   NET_CS_SLOT_2(updateConfigurations)

   NET_CS_SIGNAL_1(Public, void configurationAdded(const QNetworkConfiguration &config))
   NET_CS_SIGNAL_2(configurationAdded, config)

   NET_CS_SIGNAL_1(Public, void configurationRemoved(const QNetworkConfiguration &config))
   NET_CS_SIGNAL_2(configurationRemoved, config)

   NET_CS_SIGNAL_1(Public, void configurationChanged(const QNetworkConfiguration &config))
   NET_CS_SIGNAL_2(configurationChanged, config)

   NET_CS_SIGNAL_1(Public, void onlineStateChanged(bool isOnline))
   NET_CS_SIGNAL_2(onlineStateChanged, isOnline)

   NET_CS_SIGNAL_1(Public, void updateCompleted())
   NET_CS_SIGNAL_2(updateCompleted)

  private:
    QNetworkConfigurationManager (const QNetworkConfigurationManager &) = delete;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QNetworkConfigurationManager::Capabilities)


#endif // QT_NO_BEARERMANAGEMENT

#endif // QNETWORKCONFIGURATIONMANAGER_H
