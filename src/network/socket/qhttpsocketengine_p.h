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

#ifndef QHTTPSOCKETENGINE_P_H
#define QHTTPSOCKETENGINE_P_H

#include <qabstractsocketengine_p.h>
#include <qabstractsocket.h>
#include <qnetworkproxy.h>
#include <qauthenticator_p.h>

#if ! defined(QT_NO_NETWORKPROXY)

class QTcpSocket;
class QHttpNetworkReply;
class QHttpSocketEnginePrivate;

class QHttpSocketEngine : public QAbstractSocketEngine
{
   NET_CS_OBJECT(QHttpSocketEngine)

 public:
   enum HttpState {
      None,
      ConnectSent,
      Connected,
      SendAuthentication,
      ReadResponseContent,
      ReadResponseHeader
   };

   QHttpSocketEngine(QObject *parent = nullptr);
   ~QHttpSocketEngine();

   bool initialize(QAbstractSocket::SocketType type, QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol) override;
   bool initialize(qintptr socketDescriptor, QAbstractSocket::SocketState socketState = QAbstractSocket::ConnectedState) override;
   void setProxy(const QNetworkProxy &networkProxy);

   qintptr socketDescriptor() const override;

   bool isValid() const override;

   bool connectInternal();
   bool connectToHost(const QHostAddress &address, quint16 port) override;
   bool connectToHostByName(const QString &name, quint16 port) override;
   bool bind(const QHostAddress &address, quint16 port) override;
   bool listen() override;
   int accept() override;
   void close() override;

   qint64 bytesAvailable() const override;
   qint64 read(char *data, qint64 maxlen) override;
   qint64 write(const char *data, qint64 len) override;

#ifndef QT_NO_UDPSOCKET

#ifndef QT_NO_NETWORKINTERFACE
   bool joinMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &interface) override;
   bool leaveMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &interface) override;
   QNetworkInterface multicastInterface() const override;
   bool setMulticastInterface(const QNetworkInterface &iface) override;
#endif

   qint64 readDatagram(char *data, qint64 maxlen, QIpPacketHeader *, PacketHeaderOptions) override;
   qint64 writeDatagram(const char *data, qint64 len, const QIpPacketHeader &) override;
   bool hasPendingDatagrams() const override;
   qint64 pendingDatagramSize() const override;
#endif

   qint64 bytesToWrite() const override;

   int option(SocketOption option) const override;
   bool setOption(SocketOption option, int value) override;

   bool waitForRead(int msecs = 30000, bool *timedOut = 0) override;
   bool waitForWrite(int msecs = 30000, bool *timedOut = 0) override;

   bool waitForReadOrWrite(bool *readyToRead, bool *readyToWrite, bool checkRead, bool checkWrite,
                  int msecs = 30000, bool *timedOut = 0) override;

   bool isReadNotificationEnabled() const override;
   void setReadNotificationEnabled(bool enable) override;
   bool isWriteNotificationEnabled() const override;
   void setWriteNotificationEnabled(bool enable) override;
   bool isExceptionNotificationEnabled() const override;
   void setExceptionNotificationEnabled(bool enable) override;

   NET_CS_SLOT_1(Public, void slotSocketConnected())
   NET_CS_SLOT_2(slotSocketConnected)

   NET_CS_SLOT_1(Public, void slotSocketDisconnected())
   NET_CS_SLOT_2(slotSocketDisconnected)

   NET_CS_SLOT_1(Public, void slotSocketReadNotification())
   NET_CS_SLOT_2(slotSocketReadNotification)

   NET_CS_SLOT_1(Public, void slotSocketBytesWritten())
   NET_CS_SLOT_2(slotSocketBytesWritten)

   NET_CS_SLOT_1(Public, void slotSocketError(QAbstractSocket::SocketError error))
   NET_CS_SLOT_2(slotSocketError)

   NET_CS_SLOT_1(Public, void slotSocketStateChanged(QAbstractSocket::SocketState state))
   NET_CS_SLOT_2(slotSocketStateChanged)

 private:
   NET_CS_SLOT_1(Private, void emitPendingReadNotification())
   NET_CS_SLOT_2(emitPendingReadNotification)

   NET_CS_SLOT_1(Private, void emitPendingWriteNotification())
   NET_CS_SLOT_2(emitPendingWriteNotification)

   NET_CS_SLOT_1(Private, void emitPendingConnectionNotification())
   NET_CS_SLOT_2(emitPendingConnectionNotification)

   void emitReadNotification();
   void emitWriteNotification();
   void emitConnectionNotification();

   bool readHttpHeader();
   Q_DECLARE_PRIVATE(QHttpSocketEngine)
   Q_DISABLE_COPY(QHttpSocketEngine)
};

class QHttpSocketEnginePrivate : public QAbstractSocketEnginePrivate
{
   Q_DECLARE_PUBLIC(QHttpSocketEngine)

 public:
   QHttpSocketEnginePrivate();
   ~QHttpSocketEnginePrivate();

   QNetworkProxy proxy;
   QString peerName;
   QTcpSocket *socket;
   QHttpNetworkReply *reply; // only used for parsing the proxy response
   QHttpSocketEngine::HttpState state;
   QAuthenticator authenticator;

   bool readNotificationEnabled;
   bool writeNotificationEnabled;
   bool exceptNotificationEnabled;
   bool readNotificationActivated;
   bool writeNotificationActivated;
   bool readNotificationPending;
   bool writeNotificationPending;
   bool connectionNotificationPending;
   bool credentialsSent;
   uint pendingResponseData;
};

class QHttpSocketEngineHandler : public QSocketEngineHandler
{
 public:
   virtual QAbstractSocketEngine *createSocketEngine(QAbstractSocket::SocketType socketType,
                  const QNetworkProxy &, QObject *parent) override;

   virtual QAbstractSocketEngine *createSocketEngine(qintptr socketDescriptor, QObject *parent) override;
};
#endif


#endif // QHTTPSOCKETENGINE_H
