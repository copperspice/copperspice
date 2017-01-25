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

#ifndef QABSTRACTSOCKET_P_H
#define QABSTRACTSOCKET_P_H

#include <QtNetwork/qabstractsocket.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qtimer.h>
#include <qringbuffer_p.h>
#include <qiodevice_p.h>
#include <qabstractsocketengine_p.h>
#include <qnetworkproxy.h>

QT_BEGIN_NAMESPACE

class QHostInfo;

class QAbstractSocketPrivate : public QIODevicePrivate, public QAbstractSocketEngineReceiver
{
   Q_DECLARE_PUBLIC(QAbstractSocket)

 public:
   QAbstractSocketPrivate();
   virtual ~QAbstractSocketPrivate();

   // from QAbstractSocketEngineReceiver
   inline void readNotification() override {
      canReadNotification();
   }

   inline void writeNotification() override {
      canWriteNotification();
   }

   inline void exceptionNotification() override {}
   void connectionNotification() override;

#ifndef QT_NO_NETWORKPROXY
   inline void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator) override {
      Q_Q(QAbstractSocket);
      q->proxyAuthenticationRequired(proxy, authenticator);
   }
#endif

   bool canReadNotification();
   bool canWriteNotification();

   // slots
   void _q_connectToNextAddress();
   void _q_startConnecting(const QHostInfo &hostInfo);
   void _q_testConnection();
   void _q_abortConnectionAttempt();
   void _q_forceDisconnect();

   bool readSocketNotifierCalled;
   bool readSocketNotifierState;
   bool readSocketNotifierStateSet;

   bool emittedReadyRead;
   bool emittedBytesWritten;

   bool abortCalled;
   bool closeCalled;
   bool pendingClose;

   QString hostName;
   quint16 port;
   QHostAddress host;
   QList<QHostAddress> addresses;

   quint16 localPort;
   quint16 peerPort;
   QHostAddress localAddress;
   QHostAddress peerAddress;
   QString peerName;

   QAbstractSocketEngine *socketEngine;
   int cachedSocketDescriptor;

#ifndef QT_NO_NETWORKPROXY
   QNetworkProxy proxy;
   QNetworkProxy proxyInUse;
   void resolveProxy(const QString &hostName, quint16 port);
#else
   inline void resolveProxy(const QString &, quint16) { }
#endif

   inline void resolveProxy(quint16 port) {
      resolveProxy(QString(), port);
   }

   void resetSocketLayer();
   bool flush();

   bool initSocketLayer(QAbstractSocket::NetworkLayerProtocol protocol);
   void startConnectingByName(const QString &host);
   void fetchConnectionParameters();
   void setupSocketNotifiers();
   bool readFromSocket();

   qint64 readBufferMaxSize;
   QRingBuffer readBuffer;
   QRingBuffer writeBuffer;

   bool isBuffered;
   int blockingTimeout;

   QTimer *connectTimer;
   QTimer *disconnectTimer;
   int connectTimeElapsed;

   int hostLookupId;

   QAbstractSocket::SocketType socketType;
   QAbstractSocket::SocketState state;

   QAbstractSocket::SocketError socketError;

   bool prePauseReadSocketNotifierState;
   bool prePauseWriteSocketNotifierState;
   bool prePauseExceptionSocketNotifierState;
   static void pauseSocketNotifiers(QAbstractSocket *);
   static void resumeSocketNotifiers(QAbstractSocket *);
   static QAbstractSocketEngine *getSocketEngine(QAbstractSocket *);
};

QT_END_NAMESPACE

#endif // QABSTRACTSOCKET_P_H
