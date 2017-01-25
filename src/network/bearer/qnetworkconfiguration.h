/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QNETWORKCONFIGURATION_H
#define QNETWORKCONFIGURATION_H

#ifndef QT_MOBILITY_BEARER
# include <QtCore/qglobal.h>
#else
# include <qmobilityglobal.h>
#endif

#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>

#if defined(Q_OS_WIN) && defined(interface)
#undef interface
#endif

#ifndef QT_MOBILITY_BEARER
QT_BEGIN_NAMESPACE
#define QNetworkConfigurationExport Q_NETWORK_EXPORT

#else
QTM_BEGIN_NAMESPACE
#define QNetworkConfigurationExport Q_BEARER_EXPORT

#endif

class QNetworkConfigurationPrivate;
class QNetworkConfigurationExport QNetworkConfiguration
{

 public:
   QNetworkConfiguration();
   QNetworkConfiguration(const QNetworkConfiguration &other);
   QNetworkConfiguration &operator=(const QNetworkConfiguration &other);
   ~QNetworkConfiguration();

   bool operator==(const QNetworkConfiguration &other) const;
   inline bool operator!=(const QNetworkConfiguration &other) const {
      return !operator==(other);
   }

   enum Type {
      InternetAccessPoint = 0,
      ServiceNetwork,
      UserChoice,
      Invalid
   };

   enum Purpose {
      UnknownPurpose = 0,
      PublicPurpose,
      PrivatePurpose,
      ServiceSpecificPurpose
   };

   enum StateFlag {
      Undefined        = 0x0000001,
      Defined          = 0x0000002,
      Discovered       = 0x0000006,
      Active           = 0x000000e
   };
   using StateFlags = QFlags<StateFlag>;

#ifndef QT_MOBILITY_BEARER
   enum BearerType {
      BearerUnknown,
      BearerEthernet,
      BearerWLAN,
      Bearer2G,
      BearerCDMA2000,
      BearerWCDMA,
      BearerHSPA,
      BearerBluetooth,
      BearerWiMAX
   };
#endif

   StateFlags state() const;
   Type type() const;
   Purpose purpose() const;

#ifndef QT_MOBILITY_BEARER
#ifdef QT_DEPRECATED
   // Required to maintain source compatibility with Qt Mobility.
   QT_DEPRECATED inline QString bearerName() const {
      return bearerTypeName();
   }
#endif
   BearerType bearerType() const;
   QString bearerTypeName() const;
#else
   QString bearerName() const;
#endif

   QString identifier() const;
   bool isRoamingAvailable() const;
   QList<QNetworkConfiguration> children() const;

   QString name() const;
   bool isValid() const;

 private:
   friend class QNetworkConfigurationPrivate;
   friend class QNetworkConfigurationManager;
   friend class QNetworkConfigurationManagerPrivate;
   friend class QNetworkSessionPrivate;
   QExplicitlySharedDataPointer<QNetworkConfigurationPrivate> d;
};

#ifndef QT_MOBILITY_BEARER
QT_END_NAMESPACE
#else
QTM_END_NAMESPACE
#endif

#endif // QNETWORKCONFIGURATION_H
