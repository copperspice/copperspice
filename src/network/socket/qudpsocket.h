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

#ifndef QUDPSOCKET_H
#define QUDPSOCKET_H

#include <qabstractsocket.h>
#include <qhostaddress.h>

#ifndef QT_NO_UDPSOCKET

class QNetworkInterface;
class QUdpSocketPrivate;

class Q_NETWORK_EXPORT QUdpSocket : public QAbstractSocket
{
   NET_CS_OBJECT(QUdpSocket)

 public:
   explicit QUdpSocket(QObject *parent = nullptr);

   QUdpSocket(const QUdpSocket &) = delete;
   QUdpSocket &operator=(const QUdpSocket &) = delete;

   virtual ~QUdpSocket();

#ifndef QT_NO_NETWORKINTERFACE
   bool joinMulticastGroup(const QHostAddress &groupAddress);
   bool joinMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &interfaceId);
   bool leaveMulticastGroup(const QHostAddress &groupAddress);
   bool leaveMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &interfaceId);

   QNetworkInterface multicastInterface() const;
   void setMulticastInterface(const QNetworkInterface &interfaceId);
#endif

   bool hasPendingDatagrams() const;
   qint64 pendingDatagramSize() const;
   qint64 readDatagram(char *data, qint64 maxlen, QHostAddress *address = nullptr, quint16 *port = nullptr);
   qint64 writeDatagram(const char *data, qint64 len, const QHostAddress &address, quint16 port);

   qint64 writeDatagram(const QByteArray &datagram, const QHostAddress &address, quint16 port) {
      return writeDatagram(datagram.constData(), datagram.size(), address, port);
   }

 private:
   Q_DECLARE_PRIVATE(QUdpSocket)
};

#endif // QT_NO_UDPSOCKET

#endif
