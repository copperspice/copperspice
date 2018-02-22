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
   explicit QUdpSocket(QObject *parent = nullptr);
   virtual ~QUdpSocket();


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
   qint64 readDatagram(char *data, qint64 maxlen, QHostAddress *host = nullptr, quint16 *port = nullptr);
   qint64 writeDatagram(const char *data, qint64 len, const QHostAddress &host, quint16 port);

   inline qint64 writeDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port) {
      return writeDatagram(datagram.constData(), datagram.size(), host, port);
   }

 private:
   Q_DISABLE_COPY(QUdpSocket)
   Q_DECLARE_PRIVATE(QUdpSocket)
};

#endif // QT_NO_UDPSOCKET

QT_END_NAMESPACE

#endif // QUDPSOCKET_H
