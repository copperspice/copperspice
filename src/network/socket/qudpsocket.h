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

#ifndef QUDPSOCKET_H
#define QUDPSOCKET_H

#include <QtNetwork/qabstractsocket.h>
#include <QtNetwork/qhostaddress.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_UDPSOCKET

class QNetworkInterface;
class QUdpSocketPrivate;

class Q_NETWORK_EXPORT QUdpSocket : public QAbstractSocket
{
   NET_CS_OBJECT(QUdpSocket)

 public:
   enum BindFlag {
      DefaultForPlatform = 0x0,
      ShareAddress = 0x1,
      DontShareAddress = 0x2,
      ReuseAddressHint = 0x4
   };
   using BindMode = QFlags<BindFlag>;

   explicit QUdpSocket(QObject *parent = 0);
   virtual ~QUdpSocket();

   bool bind(const QHostAddress &address, quint16 port);
   bool bind(quint16 port = 0);
   bool bind(const QHostAddress &address, quint16 port, BindMode mode);
   bool bind(quint16 port, BindMode mode);

   // ### Qt5/Merge the bind functions

#ifndef QT_NO_NETWORKINTERFACE
   bool joinMulticastGroup(const QHostAddress &groupAddress);
   bool joinMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &iface);
   bool leaveMulticastGroup(const QHostAddress &groupAddress);
   bool leaveMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &iface);

   QNetworkInterface multicastInterface() const;
   void setMulticastInterface(const QNetworkInterface &iface);
#endif

   bool hasPendingDatagrams() const;
   qint64 pendingDatagramSize() const;
   qint64 readDatagram(char *data, qint64 maxlen, QHostAddress *host = 0, quint16 *port = 0);
   qint64 writeDatagram(const char *data, qint64 len, const QHostAddress &host, quint16 port);

   inline qint64 writeDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port) {
      return writeDatagram(datagram.constData(), datagram.size(), host, port);
   }

 private:
   Q_DISABLE_COPY(QUdpSocket)
   Q_DECLARE_PRIVATE(QUdpSocket)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QUdpSocket::BindMode)

#endif // QT_NO_UDPSOCKET

QT_END_NAMESPACE

#endif // QUDPSOCKET_H
