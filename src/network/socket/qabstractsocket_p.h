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

#ifndef QABSTRACTSOCKET_P_H
#define QABSTRACTSOCKET_P_H

#include <qabstractsocket.h>

#include <qbytearray.h>
#include <qlist.h>
#include <qnetworkproxy.h>
#include <qtimer.h>

#include <qabstractsocketengine_p.h>
#include <qiodevice_p.h>
#include <qringbuffer_p.h>

class QHostInfo;

class QAbstractSocketPrivate : public QIODevicePrivate, public QAbstractSocketEngineReceiver
{
   Q_DECLARE_PUBLIC(QAbstractSocket)

 public:
   QAbstractSocketPrivate();
   virtual ~QAbstractSocketPrivate();

   // from QAbstractSocketEngineReceiver
   void readNotification() override {
      canReadNotification();
   }

   void writeNotification() override {
      canWriteNotification();
   }

   void exceptionNotification() override {
   }

   void closeNotification() override {
      canCloseNotification();
   }

   void connectionNotification() override;

#ifndef QT_NO_NETWORKPROXY
   void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator) override {
      Q_Q(QAbstractSocket);
      q->proxyAuthenticationRequired(proxy, authenticator);
   }
#endif

   virtual bool bind(const QHostAddress &address, quint16 port, QAbstractSocket::BindMode mode);

   bool canReadNotification();
   bool canWriteNotification();
   void canCloseNotification();

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
   bool pendingClose;

   QAbstractSocket::PauseModes pauseMode;

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
   qintptr cachedSocketDescriptor;

#ifndef QT_NO_NETWORKPROXY
   QNetworkProxy proxy;
   QNetworkProxy proxyInUse;
   void resolveProxy(const QString &hostName, quint16 port);
#else
   void resolveProxy(const QString &, quint16) {
   }

#endif

   void resolveProxy(quint16 port) {
      resolveProxy(QString(), port);
   }

   void resetSocketLayer();
   bool flush();

   bool initSocketLayer(QAbstractSocket::NetworkLayerProtocol protocol);
   virtual void configureCreatedSocket();
   void startConnectingByName(const QString &host);
   void fetchConnectionParameters();
   void setupSocketNotifiers();
   bool readFromSocket();

   void setError(QAbstractSocket::SocketError errorCode, const QString &errorString);
   void setErrorAndEmit(QAbstractSocket::SocketError errorCode, const QString &errorString);

   qint64 readBufferMaxSize;
   QRingBuffer writeBuffer;

   bool isBuffered;

   QTimer *connectTimer;
   QTimer *disconnectTimer;
   int connectTimeElapsed;

   int hostLookupId;

   QAbstractSocket::SocketType socketType;
   QAbstractSocket::SocketState state;

   QAbstractSocket::SocketError socketError;
   QAbstractSocket::NetworkLayerProtocol preferredNetworkLayerProtocol;

   bool prePauseReadSocketNotifierState;
   bool prePauseWriteSocketNotifierState;
   bool prePauseExceptionSocketNotifierState;
   static void pauseSocketNotifiers(QAbstractSocket *);
   static void resumeSocketNotifiers(QAbstractSocket *);
   static QAbstractSocketEngine *getSocketEngine(QAbstractSocket *);
};

#endif
