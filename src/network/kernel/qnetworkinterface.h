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

#ifndef QNETWORKINTERFACE_H
#define QNETWORKINTERFACE_H

#include <QtCore/qshareddata.h>
#include <QtCore/qscopedpointer.h>
#include <QtNetwork/qhostaddress.h>

#ifndef QT_NO_NETWORKINTERFACE

QT_BEGIN_NAMESPACE

template<typename T> class QList;

class QNetworkInterfacePrivate;
class QNetworkAddressEntryPrivate;

class Q_NETWORK_EXPORT QNetworkAddressEntry
{

 public:
   QNetworkAddressEntry();
   QNetworkAddressEntry(const QNetworkAddressEntry &other);
   QNetworkAddressEntry &operator=(const QNetworkAddressEntry &other);
   ~QNetworkAddressEntry();
   bool operator==(const QNetworkAddressEntry &other) const;
   inline bool operator!=(const QNetworkAddressEntry &other) const {
      return !(*this == other);
   }

   QHostAddress ip() const;
   void setIp(const QHostAddress &newIp);

   QHostAddress netmask() const;
   void setNetmask(const QHostAddress &newNetmask);
   int prefixLength() const;
   void setPrefixLength(int length);

   QHostAddress broadcast() const;
   void setBroadcast(const QHostAddress &newBroadcast);

 private:
   QScopedPointer<QNetworkAddressEntryPrivate> d;
};


class Q_NETWORK_EXPORT QNetworkInterface
{
 public:
   enum InterfaceFlag {
      IsUp = 0x1,
      IsRunning = 0x2,
      CanBroadcast = 0x4,
      IsLoopBack = 0x8,
      IsPointToPoint = 0x10,
      CanMulticast = 0x20
   };
   using InterfaceFlags = QFlags<InterfaceFlag>;

   QNetworkInterface();
   QNetworkInterface(const QNetworkInterface &other);
   QNetworkInterface &operator=(const QNetworkInterface &other);
   ~QNetworkInterface();

   bool isValid() const;

   int index() const;
   QString name() const;
   QString humanReadableName() const;
   InterfaceFlags flags() const;
   QString hardwareAddress() const;
   QList<QNetworkAddressEntry> addressEntries() const;

   static QNetworkInterface interfaceFromName(const QString &name);
   static QNetworkInterface interfaceFromIndex(int index);
   static QList<QNetworkInterface> allInterfaces();
   static QList<QHostAddress> allAddresses();

 private:
   friend class QNetworkInterfacePrivate;
   QSharedDataPointer<QNetworkInterfacePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QNetworkInterface::InterfaceFlags)

Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, const QNetworkInterface &networkInterface);

QT_END_NAMESPACE

#endif // QT_NO_NETWORKINTERFACE

#endif
