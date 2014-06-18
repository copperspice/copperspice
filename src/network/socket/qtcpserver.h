/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef QTCPSERVER_H
#define QTCPSERVER_H

#include <QtCore/qobject.h>
#include <QtNetwork/qabstractsocket.h>
#include <QtNetwork/qhostaddress.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QTcpServerPrivate;
class QTcpSocket;

#ifndef QT_NO_NETWORKPROXY
class QNetworkProxy;
#endif

class Q_NETWORK_EXPORT QTcpServer : public QObject
{
   CS_OBJECT(QTcpServer)

 public:
   explicit QTcpServer(QObject *parent = 0);
   virtual ~QTcpServer();

   bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
   void close();

   bool isListening() const;

   void setMaxPendingConnections(int numConnections);
   int maxPendingConnections() const;

   quint16 serverPort() const;
   QHostAddress serverAddress() const;

   int socketDescriptor() const;
   bool setSocketDescriptor(int socketDescriptor);

   bool waitForNewConnection(int msec = 0, bool *timedOut = 0);
   virtual bool hasPendingConnections() const;
   virtual QTcpSocket *nextPendingConnection();

   QAbstractSocket::SocketError serverError() const;
   QString errorString() const;

   NET_CS_SIGNAL_1(Public, void newConnection())
   NET_CS_SIGNAL_2(newConnection)

#ifndef QT_NO_NETWORKPROXY
   void setProxy(const QNetworkProxy &networkProxy);
   QNetworkProxy proxy() const;
#endif

 protected:
   virtual void incomingConnection(int handle);
   void addPendingConnection(QTcpSocket *socket);

   QScopedPointer<QTcpServerPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QTcpServer)
   Q_DECLARE_PRIVATE(QTcpServer)

};

QT_END_NAMESPACE

#endif // QTCPSERVER_H
