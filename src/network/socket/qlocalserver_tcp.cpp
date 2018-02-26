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

#include <qlocalserver.h>
#include <qlocalserver_p.h>
#include <qlocalsocket.h>
#include <qlocalsocket_p.h>

#include <qhostaddress.h>
#include <qsettings.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

void QLocalServerPrivate::init()
{
   Q_Q(QLocalServer);
   q->connect(&tcpServer, SIGNAL(newConnection()), SLOT(_q_onNewConnection()));
}

bool QLocalServerPrivate::listen(const QString &requestedServerName)
{
   if (!tcpServer.listen(QHostAddress::LocalHost)) {
      return false;
   }

   const QLatin1String prefix("QLocalServer/");
   if (requestedServerName.startsWith(prefix)) {
      fullServerName = requestedServerName;
   } else {
      fullServerName = prefix + requestedServerName;
   }

   QSettings settings(QLatin1String("CopperSpice"), QLatin1String("CS"));
   if (settings.contains(fullServerName)) {
      qWarning("QLocalServer::listen: server name is already in use.");
      tcpServer.close();
      return false;
   }

   settings.setValue(fullServerName, tcpServer.serverPort());
   return true;
}

bool QLocalServerPrivate::listen(qintptr socketDescriptor)
{
   return tcpServer.setSocketDescriptor(socketDescriptor);
}
void QLocalServerPrivate::closeServer()
{
   QSettings settings(QLatin1String("CopperSpice"), QLatin1String("CS"));
   if (fullServerName == QLatin1String("QLocalServer")) {
      settings.setValue(fullServerName, QVariant());
   } else {
      settings.remove(fullServerName);
   }
   tcpServer.close();
}

void QLocalServerPrivate::waitForNewConnection(int msec, bool *timedOut)
{
   if (pendingConnections.isEmpty()) {
      tcpServer.waitForNewConnection(msec, timedOut);
   } else if (timedOut) {
      *timedOut = false;
   }
}

void QLocalServerPrivate::_q_onNewConnection()
{
   Q_Q(QLocalServer);
   QTcpSocket *tcpSocket = tcpServer.nextPendingConnection();
   if (!tcpSocket) {
      qWarning("QLocalServer: no pending connection");
      return;
   }

   tcpSocket->setParent(q);
   const qintptr socketDescriptor = tcpSocket->socketDescriptor();
   q->incomingConnection(socketDescriptor);
}

bool QLocalServerPrivate::removeServer(const QString &name)
{
   const QLatin1String prefix("QLocalServer/");
   QString serverName;
   if (name.startsWith(prefix)) {
      serverName = name;
   } else {
      serverName = prefix + name;
   }

   QSettings settings(QLatin1String("CopperSpice"), QLatin1String("CS"));
   if (settings.contains(serverName)) {
      settings.remove(serverName);
   }

   return true;
}

QT_END_NAMESPACE
