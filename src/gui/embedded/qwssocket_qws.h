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

#ifndef QWSSOCKET_QWS_H
#define QWSSOCKET_QWS_H

#include <qwsutils_qws.h>

#ifndef QT_NO_QWS_MULTIPROCESS

#ifndef QT_NO_SXE
#include <QtCore/qmutex.h>
#include <qunixsocketserver_p.h>
#include <qunixsocket_p.h>
#else
#include <QtNetwork/qtcpsocket.h>
#include <QtNetwork/qtcpserver.h>
#endif

QT_BEGIN_NAMESPACE

class QWSSocket : public QWS_SOCK_BASE
{
   GUI_CS_OBJECT(QWSSocket)

 public:
   explicit QWSSocket(QObject *parent = nullptr);
   ~QWSSocket();

   bool connectToLocalFile(const QString &file);

#ifndef QT_NO_SXE
   QString errorString();

   GUI_CS_SIGNAL_1(Public, void connected())
   GUI_CS_SIGNAL_2(connected)
   GUI_CS_SIGNAL_1(Public, void disconnected())
   GUI_CS_SIGNAL_2(disconnected)
   GUI_CS_SIGNAL_1(Public, void error(QAbstractSocket::SocketError un_named_arg1))
   GUI_CS_SIGNAL_2(error, un_named_arg1)

 private :
   GUI_CS_SLOT_1(Private, void forwardStateChange(SocketState un_named_arg1))
   GUI_CS_SLOT_2(forwardStateChange)

#endif

 private:
   Q_DISABLE_COPY(QWSSocket)
};


class QWSServerSocket : public QWS_SOCK_SERVER_BASE
{
   GUI_CS_OBJECT(QWSServerSocket)

 public:
   QWSServerSocket(const QString &file, QObject *parent = nullptr);
   ~QWSServerSocket();

#ifndef QT_NO_SXE
   QWSSocket *nextPendingConnection();

 public:
   GUI_CS_SIGNAL_1(Public, void newConnection())
   GUI_CS_SIGNAL_2(newConnection)

 protected:
   void incomingConnection(int socketDescriptor);

 private:
   QMutex ssmx;
   QList<int> inboundConnections;
#endif

 private:
   Q_DISABLE_COPY(QWSServerSocket)

   void init(const QString &file);
};

QT_END_NAMESPACE

#endif // QT_NO_QWS_MULTIPROCESS

#endif // QWSSOCKET_QWS_H
