/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QHOSTADDRESS_H
#define QHOSTADDRESS_H

#include <QtCore/qpair.h>
#include <QtCore/qstring.h>
#include <QtCore/qscopedpointer.h>
#include <QtNetwork/qabstractsocket.h>

struct sockaddr;

QT_BEGIN_NAMESPACE

class QHostAddressPrivate;

class Q_NETWORK_EXPORT QIPv6Address
{
 public:
   inline quint8 &operator [](int index) {
      return c[index];
   }
   inline quint8 operator [](int index) const {
      return c[index];
   }
   quint8 c[16];
};

typedef QIPv6Address Q_IPV6ADDR;

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
   explicit QHostAddress(quint8 *ip6Addr);
   explicit QHostAddress(const Q_IPV6ADDR &ip6Addr);
   explicit QHostAddress(const sockaddr *sockaddr);
   explicit QHostAddress(const QString &address);
   QHostAddress(const QHostAddress &copy);
   QHostAddress(SpecialAddress address);
   ~QHostAddress();

   QHostAddress &operator=(const QHostAddress &other);
   QHostAddress &operator=(const QString &address);

   void setAddress(quint32 ip4Addr);
   void setAddress(quint8 *ip6Addr);
   void setAddress(const Q_IPV6ADDR &ip6Addr);
   void setAddress(const sockaddr *sockaddr);
   bool setAddress(const QString &address);

   QAbstractSocket::NetworkLayerProtocol protocol() const;
   quint32 toIPv4Address() const;
   Q_IPV6ADDR toIPv6Address() const;

   QString toString() const;

   QString scopeId() const;
   void setScopeId(const QString &id);

   bool operator ==(const QHostAddress &address) const;
   bool operator ==(SpecialAddress address) const;
   inline bool operator !=(const QHostAddress &address) const {
      return !operator==(address);
   }
   inline bool operator !=(SpecialAddress address) const {
      return !operator==(address);
   }
   bool isNull() const;
   void clear();

   bool isInSubnet(const QHostAddress &subnet, int netmask) const;
   bool isInSubnet(const QPair<QHostAddress, int> &subnet) const;

   static QPair<QHostAddress, int> parseSubnet(const QString &subnet);

 protected:
   QScopedPointer<QHostAddressPrivate> d;
};

inline bool operator ==(QHostAddress::SpecialAddress address1, const QHostAddress &address2)
{
   return address2 == address1;
}

Q_NETWORK_EXPORT QDebug operator<<(QDebug, const QHostAddress &);
Q_NETWORK_EXPORT uint qHash(const QHostAddress &key);

#ifndef QT_NO_DATASTREAM
Q_NETWORK_EXPORT QDataStream &operator<<(QDataStream &, const QHostAddress &);
Q_NETWORK_EXPORT QDataStream &operator>>(QDataStream &, QHostAddress &);
#endif

QT_END_NAMESPACE

#endif // QHOSTADDRESS_H
