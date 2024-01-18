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

#include <qlocalserver.h>
#include <qlocalserver_p.h>
#include <qlocalsocket.h>
#include <qalgorithms.h>

#ifndef QT_NO_LOCALSERVER

QLocalServer::QLocalServer(QObject *parent)
   : QObject(parent), d_ptr(new QLocalServerPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QLocalServer);

   d->init();
}

QLocalServer::~QLocalServer()
{
   if (isListening()) {
      close();
   }
}

void QLocalServer::setSocketOptions(SocketOptions options)
{
   Q_D(QLocalServer);
   d->socketOptions = options;
}

QLocalServer::SocketOptions QLocalServer::socketOptions() const
{
   Q_D(const QLocalServer);
   return d->socketOptions;
}

void QLocalServer::close()
{
   Q_D(QLocalServer);

   if (!isListening()) {
      return;
   }

   qDeleteAll(d->pendingConnections);
   d->pendingConnections.clear();
   d->closeServer();
   d->serverName.clear();
   d->fullServerName.clear();
   d->errorString.clear();
   d->error = QAbstractSocket::UnknownSocketError;
}

QString QLocalServer::errorString() const
{
   Q_D(const QLocalServer);
   return d->errorString;
}

bool QLocalServer::hasPendingConnections() const
{
   Q_D(const QLocalServer);
   return !(d->pendingConnections.isEmpty());
}

void QLocalServer::incomingConnection(qintptr socketDescriptor)
{
   Q_D(QLocalServer);
   QLocalSocket *socket = new QLocalSocket(this);
   socket->setSocketDescriptor(socketDescriptor);
   d->pendingConnections.enqueue(socket);
   emit newConnection();
}

bool QLocalServer::isListening() const
{
   Q_D(const QLocalServer);
   return !(d->serverName.isEmpty());
}

bool QLocalServer::listen(const QString &name)
{
   Q_D(QLocalServer);
   if (isListening()) {
      qWarning("QLocalServer::listen() Called when already listening");
      return false;
   }

   if (name.isEmpty()) {
      d->error = QAbstractSocket::HostNotFoundError;
      QString function = "QLocalServer::listen";
      d->errorString = tr("%1: Name error").formatArg(function);
      return false;
   }

   if (! d->listen(name)) {
      d->serverName.clear();
      d->fullServerName.clear();
      return false;
   }

   d->serverName = name;

   return true;
}

bool QLocalServer::listen(qintptr socketDescriptor)
{
   Q_D(QLocalServer);

   if (isListening()) {
     qWarning("QLocalServer::listen() Called when already listening");
     return false;
   }

   d->serverName.clear();
   d->fullServerName.clear();

   if (! d->listen(socketDescriptor)) {
     return false;
   }

   return true;
}

int QLocalServer::maxPendingConnections() const
{
   Q_D(const QLocalServer);
   return d->maxPendingConnections;
}

QLocalSocket *QLocalServer::nextPendingConnection()
{
   Q_D(QLocalServer);

   if (d->pendingConnections.isEmpty()) {
      return nullptr;
   }

   QLocalSocket *nextSocket = d->pendingConnections.dequeue();

#ifndef QT_LOCALSOCKET_TCP

   if (d->pendingConnections.size() <= d->maxPendingConnections)
#ifndef Q_OS_WIN
      d->socketNotifier->setEnabled(true);
#else
      d->connectionEventNotifier->setEnabled(true);
#endif

#endif

   return nextSocket;
}

bool QLocalServer::removeServer(const QString &name)
{
   return QLocalServerPrivate::removeServer(name);
}

QString QLocalServer::serverName() const
{
   Q_D(const QLocalServer);
   return d->serverName;
}

QString QLocalServer::fullServerName() const
{
   Q_D(const QLocalServer);
   return d->fullServerName;
}

QAbstractSocket::SocketError QLocalServer::serverError() const
{
   Q_D(const QLocalServer);
   return d->error;
}

void QLocalServer::setMaxPendingConnections(int numConnections)
{
   Q_D(QLocalServer);
   d->maxPendingConnections = numConnections;
}

bool QLocalServer::waitForNewConnection(int msec, bool *timedOut)
{
   Q_D(QLocalServer);
   if (timedOut) {
      *timedOut = false;
   }

   if (!isListening()) {
      return false;
   }

   d->waitForNewConnection(msec, timedOut);

   return !d->pendingConnections.isEmpty();
}

void QLocalServer::_q_onNewConnection()
{
   Q_D(QLocalServer);
   d->_q_onNewConnection();
}

#endif
