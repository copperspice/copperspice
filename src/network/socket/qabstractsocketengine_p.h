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

#ifndef QABSTRACTSOCKETENGINE_P_H
#define QABSTRACTSOCKETENGINE_P_H

#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qabstractsocket.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QAuthenticator;
class QAbstractSocketEnginePrivate;

#ifndef QT_NO_NETWORKINTERFACE
class QNetworkInterface;
#endif

class QNetworkProxy;

class QAbstractSocketEngineReceiver
{

 public:
   virtual ~QAbstractSocketEngineReceiver() {}

   virtual void readNotification() = 0;
   virtual void writeNotification() = 0;
   virtual void exceptionNotification() = 0;
   virtual void connectionNotification() = 0;

#ifndef QT_NO_NETWORKPROXY
   virtual void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator) = 0;
#endif

};

class QAbstractSocketEngine : public QObject
{
   CS_OBJECT(QAbstractSocketEngine)

 public:
   static QAbstractSocketEngine *createSocketEngine(QAbstractSocket::SocketType socketType,
         const QNetworkProxy &, QObject *parent);

   static QAbstractSocketEngine *createSocketEngine(int socketDescripter, QObject *parent);

   QAbstractSocketEngine(QObject *parent = 0);
   ~QAbstractSocketEngine();

   enum SocketOption {
      NonBlockingSocketOption,
      BroadcastSocketOption,
      ReceiveBufferSocketOption,
      SendBufferSocketOption,
      AddressReusable,
      BindExclusively,
      ReceiveOutOfBandData,
      LowDelayOption,
      KeepAliveOption,
      MulticastTtlOption,
      MulticastLoopbackOption
   };

   virtual bool initialize(QAbstractSocket::SocketType type,
                           QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol) = 0;

   virtual bool initialize(int socketDescriptor,
                           QAbstractSocket::SocketState socketState = QAbstractSocket::ConnectedState) = 0;

   virtual int socketDescriptor() const = 0;

   virtual bool isValid() const = 0;

   virtual bool connectToHost(const QHostAddress &address, quint16 port) = 0;
   virtual bool connectToHostByName(const QString &name, quint16 port) = 0;
   virtual bool bind(const QHostAddress &address, quint16 port) = 0;
   virtual bool listen() = 0;
   virtual int accept() = 0;
   virtual void close() = 0;

   virtual qint64 bytesAvailable() const = 0;

   virtual qint64 read(char *data, qint64 maxlen) = 0;
   virtual qint64 write(const char *data, qint64 len) = 0;

#ifndef QT_NO_UDPSOCKET
#ifndef QT_NO_NETWORKINTERFACE
   virtual bool joinMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &iface) = 0;
   virtual bool leaveMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &iface) = 0;

   virtual QNetworkInterface multicastInterface() const = 0;
   virtual bool setMulticastInterface(const QNetworkInterface &iface) = 0;
#endif

   virtual qint64 readDatagram(char *data, qint64 maxlen, QHostAddress *addr = 0, quint16 *port = 0) = 0;

   virtual qint64 writeDatagram(const char *data, qint64 len, const QHostAddress &addr, quint16 port) = 0;

   virtual bool hasPendingDatagrams() const = 0;
   virtual qint64 pendingDatagramSize() const = 0;
#endif

   virtual qint64 bytesToWrite() const = 0;

   virtual int option(SocketOption option) const = 0;
   virtual bool setOption(SocketOption option, int value) = 0;

   virtual bool waitForRead(int msecs = 30000, bool *timedOut = 0) = 0;
   virtual bool waitForWrite(int msecs = 30000, bool *timedOut = 0) = 0;
   virtual bool waitForReadOrWrite(bool *readyToRead, bool *readyToWrite, bool checkRead, bool checkWrite,
                                   int msecs = 30000, bool *timedOut = 0) = 0;

   QAbstractSocket::SocketError error() const;
   QString errorString() const;
   QAbstractSocket::SocketState state() const;
   QAbstractSocket::SocketType socketType() const;
   QAbstractSocket::NetworkLayerProtocol protocol() const;

   QHostAddress localAddress() const;
   quint16 localPort() const;
   QHostAddress peerAddress() const;
   quint16 peerPort() const;

   virtual bool isReadNotificationEnabled() const = 0;
   virtual void setReadNotificationEnabled(bool enable) = 0;
   virtual bool isWriteNotificationEnabled() const = 0;
   virtual void setWriteNotificationEnabled(bool enable) = 0;
   virtual bool isExceptionNotificationEnabled() const = 0;
   virtual void setExceptionNotificationEnabled(bool enable) = 0;

   NET_CS_SLOT_1(Public, void readNotification())
   NET_CS_SLOT_2(readNotification)
   NET_CS_SLOT_1(Public, void writeNotification())
   NET_CS_SLOT_2(writeNotification)
   NET_CS_SLOT_1(Public, void exceptionNotification())
   NET_CS_SLOT_2(exceptionNotification)
   NET_CS_SLOT_1(Public, void connectionNotification())
   NET_CS_SLOT_2(connectionNotification)

#ifndef QT_NO_NETWORKPROXY
   NET_CS_SLOT_1(Public, void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator))
   NET_CS_SLOT_2(proxyAuthenticationRequired)
#endif

   void setReceiver(QAbstractSocketEngineReceiver *receiver);

 protected:
   QAbstractSocketEngine(QAbstractSocketEnginePrivate &dd, QObject *parent = 0);

   void setError(QAbstractSocket::SocketError error, const QString &errorString) const;
   void setState(QAbstractSocket::SocketState state);
   void setSocketType(QAbstractSocket::SocketType socketType);
   void setProtocol(QAbstractSocket::NetworkLayerProtocol protocol);
   void setLocalAddress(const QHostAddress &address);
   void setLocalPort(quint16 port);
   void setPeerAddress(const QHostAddress &address);
   void setPeerPort(quint16 port);

   QScopedPointer<QAbstractSocketEnginePrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QAbstractSocketEngine)
   Q_DISABLE_COPY(QAbstractSocketEngine)  
};

class QAbstractSocketEnginePrivate
{
   Q_DECLARE_PUBLIC(QAbstractSocketEngine)

 public:
   QAbstractSocketEnginePrivate();
   virtual ~QAbstractSocketEnginePrivate() {}

   mutable QAbstractSocket::SocketError socketError;
   mutable bool hasSetSocketError;
   mutable QString socketErrorString;
   QAbstractSocket::SocketState socketState;
   QAbstractSocket::SocketType socketType;
   QAbstractSocket::NetworkLayerProtocol socketProtocol;
   QHostAddress localAddress;
   quint16 localPort;
   QHostAddress peerAddress;
   quint16 peerPort;
   QAbstractSocketEngineReceiver *receiver;

 protected:
   QAbstractSocketEngine *q_ptr;
};


class QSocketEngineHandler
{
 protected:
   QSocketEngineHandler();
   virtual ~QSocketEngineHandler();
   virtual QAbstractSocketEngine *createSocketEngine(QAbstractSocket::SocketType socketType,
         const QNetworkProxy &, QObject *parent) = 0;
   virtual QAbstractSocketEngine *createSocketEngine(int socketDescripter, QObject *parent) = 0;

 private:
   friend class QAbstractSocketEngine;
};

QT_END_NAMESPACE

#endif // QABSTRACTSOCKETENGINE_P_H
