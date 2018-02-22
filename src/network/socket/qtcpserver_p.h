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

#ifndef QTCPSERVER_P_H
#define QTCPSERVER_P_H

#include <qtcpserver.h>
#include <qabstractsocketengine_p.h>
#include <qabstractsocket.h>
#include <qnetworkproxy.h>
#include <qlist.h>
#include <qhostaddress.h>

class QTcpServerPrivate : public QAbstractSocketEngineReceiver
{
   Q_DECLARE_PUBLIC(QTcpServer)

 public:
   QTcpServerPrivate();
   ~QTcpServerPrivate();

   QList<QTcpSocket *> pendingConnections;

   quint16 port;
   QHostAddress address;

   QAbstractSocket::SocketState state;
   QAbstractSocketEngine *socketEngine;

   QAbstractSocket::SocketError serverSocketError;
   QString serverSocketErrorString;

   int maxConnections;

#ifndef QT_NO_NETWORKPROXY
   QNetworkProxy proxy;
   QNetworkProxy resolveProxy(const QHostAddress &address, quint16 port);
#endif

   virtual void configureCreatedSocket();

   // from QAbstractSocketEngineReceiver
   void readNotification() override;
   void closeNotification() override {
      readNotification();
   }

   void writeNotification() override {}
   void exceptionNotification() override {}
   void connectionNotification() override {}

#ifndef QT_NO_NETWORKPROXY
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *) override {}
#endif

 protected:
   QTcpServer *q_ptr;

};

#endif