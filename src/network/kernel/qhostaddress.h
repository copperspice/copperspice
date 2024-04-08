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

#ifndef QHOSTADDRESS_H
#define QHOSTADDRESS_H

#include <qpair.h>
#include <qstring.h>
#include <qscopedpointer.h>
#include <qabstractsocket.h>

struct sockaddr;

class QHostAddressPrivate;
class QHostAddress;

Q_NETWORK_EXPORT uint qHash(const QHostAddress &key, uint seed = 0);

class Q_NETWORK_EXPORT QIPv6Address
{
 public:
   quint8 &operator [](int index) {
      return c[index];
   }

   quint8 operator [](int index) const {
      return c[index];
   }

   quint8 c[16];
};

using Q_IPV6ADDR = QIPv6Address;

class Q_NETWORK_EXPORT QHostAddress
{
 public:
   enum SpecialAddress {
      Null,
      Broadcast,
      LocalHost,
      LocalHostIPv6,
      Any,
      AnyIPv6,
      AnyIPv4
   };

   QHostAddress();

   explicit QHostAddress(quint32 ip4Addr);
   explicit QHostAddress(const quint8 *ip6Addr);
   explicit QHostAddress(const Q_IPV6ADDR &ip6Addr);

   explicit QHostAddress(const sockaddr *address);
   explicit QHostAddress(const QString &address);

   QHostAddress(SpecialAddress address);

   QHostAddress(const QHostAddress &other);

   ~QHostAddress();

   void setAddress(quint32 ip4Addr);
   void setAddress(const quint8 *ip6Addr);
   void setAddress(const Q_IPV6ADDR &ip6Addr);
   void setAddress(const sockaddr *sockAddr);
   bool setAddress(const QString &address);

   QAbstractSocket::NetworkLayerProtocol protocol() const;
   quint32 toIPv4Address(bool *ok = nullptr) const;
   Q_IPV6ADDR toIPv6Address() const;

   QString toString() const;

   QString scopeId() const;
   void setScopeId(const QString &id);

   QHostAddress &operator=(const QHostAddress &other);

   QHostAddress &operator=(QHostAddress &&other) {
      swap(other);
      return *this;
   }

   QHostAddress &operator=(const QString &address);

   bool operator ==(const QHostAddress &other) const;

   bool operator !=(const QHostAddress &other) const {
      return !operator==(other);
   }

   bool operator ==(SpecialAddress address) const;

   bool operator !=(SpecialAddress address) const {
      return !operator==(address);
   }

   bool isNull() const;
   void clear();

   bool isInSubnet(const QHostAddress &subnet, int netmask) const;
   bool isInSubnet(const QPair<QHostAddress, int> &subnet) const;

   bool isLoopback() const;
   bool isMulticast() const;

   static QPair<QHostAddress, int> parseSubnet(const QString &subnet);

   void swap(QHostAddress &other) {
      d.swap(other.d);
   }

   friend Q_NETWORK_EXPORT uint qHash(const QHostAddress &key, uint seed);

 protected:
   QScopedPointer<QHostAddressPrivate> d;
};

inline bool operator ==(QHostAddress::SpecialAddress address1, const QHostAddress &address2)
{
   return address2 == address1;
}

Q_NETWORK_EXPORT QDebug operator<<(QDebug, const QHostAddress &);

Q_NETWORK_EXPORT QDataStream &operator<<(QDataStream &, const QHostAddress &);
Q_NETWORK_EXPORT QDataStream &operator>>(QDataStream &, QHostAddress &);

CS_DECLARE_METATYPE(QHostAddress)

#endif
