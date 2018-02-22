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

#ifndef QLOCALSERVER_H
#define QLOCALSERVER_H

#include <qabstractsocket.h>
#include <QScopedPointer>

#ifndef QT_NO_LOCALSERVER

class QLocalSocket;
class QLocalServerPrivate;

class Q_NETWORK_EXPORT QLocalServer : public QObject
{
   NET_CS_OBJECT(QLocalServer)
   Q_DECLARE_PRIVATE(QLocalServer)

   NET_CS_PROPERTY_READ(socketOptions,  socketOptions)
   NET_CS_PROPERTY_WRITE(socketOptions, setSocketOptions)

 public:
   enum SocketOption {
      NoOptions = 0x00,
      UserAccessOption  = 0x01,
      GroupAccessOption = 0x02,
      OtherAccessOption = 0x04,
      WorldAccessOption = 0x07
   };
   using SocketOptions = QFlags<SocketOption>;

   explicit QLocalServer(QObject *parent = nullptr);
   ~QLocalServer();

   void close();
   QString errorString() const;
   virtual bool hasPendingConnections() const;
   bool isListening() const;
   bool listen(const QString &name);
   bool listen(qintptr socketDescriptor);
   int maxPendingConnections() const;
   virtual QLocalSocket *nextPendingConnection();
   QString serverName() const;
   QString fullServerName() const;
   static bool removeServer(const QString &name);
   QAbstractSocket::SocketError serverError() const;
   void setMaxPendingConnections(int numConnections);
   bool waitForNewConnection(int msec = 0, bool *timedOut = nullptr);

   void setSocketOptions(SocketOptions options);
   SocketOptions socketOptions() const;

   NET_CS_SIGNAL_1(Public, void newConnection())
   NET_CS_SIGNAL_2(newConnection)

 protected:
   virtual void incomingConnection(qintptr socketDescriptor);
   QScopedPointer<QLocalServerPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QLocalServer)

   NET_CS_SLOT_1(Private, void _q_onNewConnection())
   NET_CS_SLOT_2(_q_onNewConnection)
};

#endif // QT_NO_LOCALSERVER


#endif // QLOCALSERVER_H

