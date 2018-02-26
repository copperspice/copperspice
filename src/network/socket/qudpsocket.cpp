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

//#define QUDPSOCKET_DEBUG

#include <qudpsocket.h>
#include <qhostaddress.h>
#include <qnetworkinterface.h>
#include <qabstractsocket_p.h>



#ifndef QT_NO_UDPSOCKET

#define QT_CHECK_BOUND(function, a) do { \
    if (!isValid()) { \
        qWarning(function" called on a QUdpSocket when not in QUdpSocket::BoundState"); \
        return (a); \
    } } while (0)

class QUdpSocketPrivate : public QAbstractSocketPrivate
{
   Q_DECLARE_PUBLIC(QUdpSocket)

   bool doEnsureInitialized(const QHostAddress &bindAddress, quint16 bindPort,
                            const QHostAddress &remoteAddress);
 public:
   inline bool ensureInitialized(const QHostAddress &bindAddress, quint16 bindPort) {
      return doEnsureInitialized(bindAddress, bindPort, QHostAddress());
   }

   inline bool ensureInitialized(const QHostAddress &remoteAddress) {
      return doEnsureInitialized(QHostAddress(), 0, remoteAddress);
   }
};

bool QUdpSocketPrivate::doEnsureInitialized(const QHostAddress &bindAddress, quint16 bindPort,
      const QHostAddress &remoteAddress)
{
   const QHostAddress *address = &bindAddress;
   QAbstractSocket::NetworkLayerProtocol proto = address->protocol();
   if (proto == QUdpSocket::UnknownNetworkLayerProtocol) {
      address = &remoteAddress;
      proto = address->protocol();
   }

   // now check if the socket engine is initialized and to the right type
   if (!socketEngine || !socketEngine->isValid()) {
      resolveProxy(remoteAddress.toString(), bindPort);
      if (! initSocketLayer(address->protocol())) {
         return false;
      }
   }

   return true;
}


QUdpSocket::QUdpSocket(QObject *parent)
   : QAbstractSocket(UdpSocket, *new QUdpSocketPrivate, parent)
{
   d_func()->isBuffered = false;
}


QUdpSocket::~QUdpSocket()
{
}





#ifndef QT_NO_NETWORKINTERFACE


bool QUdpSocket::joinMulticastGroup(const QHostAddress &groupAddress)
{
   return joinMulticastGroup(groupAddress, QNetworkInterface());
}


bool QUdpSocket::joinMulticastGroup(const QHostAddress &groupAddress,
                                    const QNetworkInterface &iface)
{
   Q_D(QUdpSocket);
   QT_CHECK_BOUND("QUdpSocket::joinMulticastGroup()", false);
   return d->socketEngine->joinMulticastGroup(groupAddress, iface);
}


bool QUdpSocket::leaveMulticastGroup(const QHostAddress &groupAddress)
{
   return leaveMulticastGroup(groupAddress, QNetworkInterface());
}


bool QUdpSocket::leaveMulticastGroup(const QHostAddress &groupAddress,
                                     const QNetworkInterface &iface)
{
   QT_CHECK_BOUND("QUdpSocket::leaveMulticastGroup()", false);
   return d_func()->socketEngine->leaveMulticastGroup(groupAddress, iface);
}


QNetworkInterface QUdpSocket::multicastInterface() const
{
   Q_D(const QUdpSocket);
   QT_CHECK_BOUND("QUdpSocket::multicastInterface()", QNetworkInterface());
   return d->socketEngine->multicastInterface();
}


void QUdpSocket::setMulticastInterface(const QNetworkInterface &iface)
{
   Q_D(QUdpSocket);
   if (!isValid()) {
      qWarning("QUdpSocket::setMulticastInterface() called on a QUdpSocket when not in QUdpSocket::BoundState");
      return;
   }
   d->socketEngine->setMulticastInterface(iface);
}

#endif // QT_NO_NETWORKINTERFACE


bool QUdpSocket::hasPendingDatagrams() const
{
   QT_CHECK_BOUND("QUdpSocket::hasPendingDatagrams()", false);
   return d_func()->socketEngine->hasPendingDatagrams();
}


qint64 QUdpSocket::pendingDatagramSize() const
{
   QT_CHECK_BOUND("QUdpSocket::pendingDatagramSize()", -1);
   return d_func()->socketEngine->pendingDatagramSize();
}


qint64 QUdpSocket::writeDatagram(const char *data, qint64 size, const QHostAddress &address,
                                 quint16 port)
{
   Q_D(QUdpSocket);
#if defined QUDPSOCKET_DEBUG
   qDebug("QUdpSocket::writeDatagram(%p, %llu, \"%s\", %i)", data, size,
          address.toString().toLatin1().constData(), port);
#endif
   if (! d->doEnsureInitialized(QHostAddress::Any, 0, address)) {
      return -1;
   }

   if (state() == UnconnectedState) {
      bind();
   }

   qint64 sent = d->socketEngine->writeDatagram(data, size, QIpPacketHeader(address, port));
   d->cachedSocketDescriptor = d->socketEngine->socketDescriptor();

   if (sent >= 0) {
      emit bytesWritten(sent);

   } else {
      d->setErrorAndEmit(d->socketEngine->error(), d->socketEngine->errorString());
   }
   return sent;
}




qint64 QUdpSocket::readDatagram(char *data, qint64 maxSize, QHostAddress *address,
                                quint16 *port)
{
   Q_D(QUdpSocket);

#if defined QUDPSOCKET_DEBUG
   qDebug("QUdpSocket::readDatagram(%p, %llu, %p, %p)", data, maxSize, address, port);
#endif

   QT_CHECK_BOUND("QUdpSocket::readDatagram()", -1);

   qint64 readBytes;
   if (address || port) {
      QIpPacketHeader header;
      readBytes = d->socketEngine->readDatagram(data, maxSize, &header,
                  QAbstractSocketEngine::WantDatagramSender);
      if (address) {
         *address = header.senderAddress;
      }
      if (port) {
         *port = header.senderPort;
      }
   } else {
      readBytes = d->socketEngine->readDatagram(data, maxSize);
   }
   d_func()->socketEngine->setReadNotificationEnabled(true);

   if (readBytes < 0) {
      d->setErrorAndEmit(d->socketEngine->error(), d->socketEngine->errorString());
   }

   return readBytes;
}
#endif // QT_NO_UDPSOCKET

QT_END_NAMESPACE
