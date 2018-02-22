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

#ifndef QTCPSERVER_H
#define QTCPSERVER_H

#include <qobject.h>
#include <qabstractsocket.h>
#include <qhostaddress.h>
#include <QScopedPointer>



class QTcpServerPrivate;
class QTcpSocket;

#ifndef QT_NO_NETWORKPROXY
class QNetworkProxy;
#endif

class Q_NETWORK_EXPORT QTcpServer : public QObject
{
   NET_CS_OBJECT(QTcpServer)

 public:
   explicit QTcpServer(QObject *parent = nullptr);
   virtual ~QTcpServer();

   bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
   void close();

   bool isListening() const;

   void setMaxPendingConnections(int numConnections);
   int maxPendingConnections() const;

   quint16 serverPort() const;
   QHostAddress serverAddress() const;

   qintptr socketDescriptor() const;
   bool setSocketDescriptor(qintptr socketDescriptor);

   bool waitForNewConnection(int msec = 0, bool *timedOut = nullptr);
   virtual bool hasPendingConnections() const;
   virtual QTcpSocket *nextPendingConnection();

   QAbstractSocket::SocketError serverError() const;
   QString errorString() const;

   void pauseAccepting();
   void resumeAccepting();

   NET_CS_SIGNAL_1(Public, void newConnection())
   NET_CS_SIGNAL_2(newConnection)

   NET_CS_SIGNAL_1(Public, void acceptError(QAbstractSocket::SocketError socketError))
   NET_CS_SIGNAL_2(acceptError, socketError)

#ifndef QT_NO_NETWORKPROXY
   void setProxy(const QNetworkProxy &networkProxy);
   QNetworkProxy proxy() const;
#endif

 protected:
   QTcpServer(QTcpServerPrivate &dd, QObject *parent = nullptr);

   virtual void incomingConnection(qintptr handle);
   void addPendingConnection(QTcpSocket *socket);

   QScopedPointer<QTcpServerPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QTcpServer)
   Q_DECLARE_PRIVATE(QTcpServer)
};



#endif // QTCPSERVER_H
