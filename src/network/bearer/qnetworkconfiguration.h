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

#ifndef QNETWORKCONFIGURATION_H
#define QNETWORKCONFIGURATION_H

#include <qglobal.h>
#include <qmetatype.h>
#include <qshareddata.h>
#include <qstring.h>
#include <qlist.h>

class QNetworkConfigurationPrivate;

class Q_NETWORK_EXPORT QNetworkConfiguration
{

 public:
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

   enum BearerType {
      BearerUnknown,
      BearerEthernet,
      BearerWLAN,
      Bearer2G,
      BearerCDMA2000,
      BearerWCDMA,
      BearerHSPA,
      BearerBluetooth,
      BearerWiMAX,
      BearerEVDO,
      BearerLTE,
      Bearer3G,
      Bearer4G
   };

   QNetworkConfiguration();
   QNetworkConfiguration(const QNetworkConfiguration &other);
   ~QNetworkConfiguration();

   QNetworkConfiguration &operator=(QNetworkConfiguration &&other)  {
      swap(other);
      return *this;
   }

   QNetworkConfiguration &operator=(const QNetworkConfiguration &other);

   bool operator==(const QNetworkConfiguration &other) const;

   inline bool operator!=(const QNetworkConfiguration &other) const {
      return !operator==(other);
   }

   void swap(QNetworkConfiguration &other) {
      qSwap(d, other.d);
   }

   StateFlags state() const;
   Type type() const;
   Purpose purpose() const;

   BearerType bearerType() const;
   BearerType bearerTypeFamily() const;
   QString bearerTypeName() const;

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

Q_DECLARE_METATYPE(QNetworkConfiguration)

#endif // QNETWORKCONFIGURATION_H
