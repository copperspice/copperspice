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
#include <qtemporarydir.h>

#include <qlocalsocket_p.h>
#include <qnet_unix_p.h>

#ifndef QT_NO_LOCALSERVER

#include <qdebug.h>
#include <qdir.h>
#include <qdatetime.h>

#include <sys/socket.h>
#include <sys/un.h>

void QLocalServerPrivate::init()
{
}

bool QLocalServerPrivate::removeServer(const QString &name)
{
   QString fileName;

   if (name.startsWith('/')) {
      fileName = name;

   } else {
      fileName = QDir::cleanPath(QDir::tempPath());
      fileName += QChar('/') + name;
   }

   if (QFile::exists(fileName)) {
      return QFile::remove(fileName);
   } else {
      return true;
   }
}

bool QLocalServerPrivate::listen(const QString &requestedServerName)
{
   Q_Q(QLocalServer);

   // determine the full server path
   if (requestedServerName.startsWith('/')) {
      fullServerName = requestedServerName;
   } else {
      fullServerName = QDir::cleanPath(QDir::tempPath());
      fullServerName += QChar('/') + requestedServerName;
   }

   serverName = requestedServerName;

   QByteArray encodedTempPath;
   const QByteArray encodedFullServerName = QFile::encodeName(fullServerName);

   QScopedPointer<QTemporaryDir> tempDir;

   // Check any of the flags
   if (socketOptions & QLocalServer::WorldAccessOption) {
      QFileInfo serverNameFileInfo(fullServerName);
      tempDir.reset(new QTemporaryDir(serverNameFileInfo.absolutePath() + QChar('/')));

      if (!tempDir->isValid()) {
         setError("QLocalServer::listen");
         return false;
      }
      encodedTempPath = QFile::encodeName(tempDir->path() + QString("/s"));
   }

   // create the unix socket
   listenSocket = qt_safe_socket(PF_UNIX, SOCK_STREAM, 0);

   if (listenSocket == -1) {
      setError("QLocalServer::listen");
      closeServer();
      return false;
   }

   // Construct the unix address
   struct ::sockaddr_un addr;
   addr.sun_family = PF_UNIX;

   if (sizeof(addr.sun_path) < (uint)encodedFullServerName.size() + 1) {
      setError("QLocalServer::listen");
      closeServer();
      return false;
   }

   if (socketOptions & QLocalServer::WorldAccessOption) {
      if (sizeof(addr.sun_path) < (uint)encodedTempPath.size() + 1) {
         setError("QLocalServer::listen");
         closeServer();
         return false;
      }

      ::memcpy(addr.sun_path, encodedTempPath.constData(), encodedTempPath.size() + 1);

   } else {
      ::memcpy(addr.sun_path, encodedFullServerName.constData(), encodedFullServerName.size() + 1);
   }

   // bind
   if (QT_SOCKET_BIND(listenSocket, (sockaddr *)&addr, sizeof(sockaddr_un)) == -1) {
      setError("QLocalServer::listen");

      // if address is in use already, just close the socket, but do not delete the file
      if (errno == EADDRINUSE) {
         QT_CLOSE(listenSocket);

      }  else {
         // otherwise, close the socket and delete the file
         closeServer();
      }

      listenSocket = -1;

      return false;
   }

   // listen for connections
   if (qt_safe_listen(listenSocket, 50) == -1) {
      setError("QLocalServer::listen");
      closeServer();
      listenSocket = -1;

      if (error != QAbstractSocket::AddressInUseError) {
         QFile::remove(fullServerName);
      }

      return false;
   }

   if (socketOptions & QLocalServer::WorldAccessOption) {
      mode_t mode = 000;

      if (socketOptions & QLocalServer::UserAccessOption) {
         mode |= S_IRWXU;
      }

      if (socketOptions & QLocalServer::GroupAccessOption) {
         mode |= S_IRWXG;
      }

      if (socketOptions & QLocalServer::OtherAccessOption) {
         mode |= S_IRWXO;
      }

      if (::chmod(encodedTempPath.constData(), mode) == -1) {
         setError("QLocalServer::listen");
         closeServer();
         return false;
      }

      if (::rename(encodedTempPath.constData(), encodedFullServerName.constData()) == -1) {
         setError("QLocalServer::listen");
         closeServer();
         return false;
      }
   }

   Q_ASSERT(!socketNotifier);
   socketNotifier = new QSocketNotifier(listenSocket, QSocketNotifier::Read, q);

   q->connect(socketNotifier, &QSocketNotifier::activated, q, &QLocalServer::_q_onNewConnection);

   socketNotifier->setEnabled(maxPendingConnections > 0);

   return true;
}

bool QLocalServerPrivate::listen(qintptr socketDescriptor)
{
   Q_Q(QLocalServer);

   // Attach to the localsocket
   listenSocket = socketDescriptor;

   ::fcntl(listenSocket, F_SETFD, FD_CLOEXEC);
   ::fcntl(listenSocket, F_SETFL, ::fcntl(listenSocket, F_GETFL) | O_NONBLOCK);

#ifdef Q_OS_LINUX
   struct ::sockaddr_un addr;

   QT_SOCKLEN_T len = sizeof(addr);
   memset(&addr, 0, sizeof(addr));

   if (::getsockname(listenSocket, (sockaddr *)&addr, &len) == 0) {
      // check for abstract sockets
      if (addr.sun_family == PF_UNIX && addr.sun_path[0] == 0) {
         addr.sun_path[0] = '@';
      }

      QString name = QString::fromLatin1(addr.sun_path);

      if (! name.isEmpty()) {
         fullServerName = name;
         serverName = fullServerName.mid(fullServerName.lastIndexOf('/') + 1);

         if (serverName.isEmpty()) {
            serverName = fullServerName;
         }
      }
   }

#else
   serverName.clear();
   fullServerName.clear();
#endif

   Q_ASSERT(! socketNotifier);
   socketNotifier = new QSocketNotifier(listenSocket, QSocketNotifier::Read, q);

   q->connect(socketNotifier, &QSocketNotifier::activated, q, &QLocalServer::_q_onNewConnection);

   socketNotifier->setEnabled(maxPendingConnections > 0);

   return true;
}

void QLocalServerPrivate::closeServer()
{
   if (socketNotifier) {
      socketNotifier->setEnabled(false); // Otherwise, closed socket is checked before deleter runs
      socketNotifier->deleteLater();
      socketNotifier = nullptr;
   }

   if (listenSocket != -1) {
      QT_CLOSE(listenSocket);
   }

   listenSocket = -1;

   if (! fullServerName.isEmpty()) {
      QFile::remove(fullServerName);
   }
}

void QLocalServerPrivate::_q_onNewConnection()
{
   Q_Q(QLocalServer);

   if (listenSocket == -1) {
      return;
   }

   ::sockaddr_un addr;

   QT_SOCKLEN_T length = sizeof(sockaddr_un);
   int connectedSocket = qt_safe_accept(listenSocket, (sockaddr *)&addr, &length);

   if (connectedSocket == -1) {
      setError("QLocalSocket::activated");
      closeServer();

   } else {
      socketNotifier->setEnabled(pendingConnections.size() <= maxPendingConnections);
      q->incomingConnection(connectedSocket);
   }
}

void QLocalServerPrivate::waitForNewConnection(int msec, bool *timedOut)
{
   fd_set readfds;
   FD_ZERO(&readfds);
   FD_SET(listenSocket, &readfds);

   struct timespec timeout;
   timeout.tv_sec = msec / 1000;
   timeout.tv_nsec = (msec % 1000) * 1000 * 1000;

   int result = -1;
   result = qt_safe_select(listenSocket + 1, &readfds, nullptr, nullptr, (msec == -1) ? nullptr : &timeout);

   if (result == -1) {
      setError("QLocalServer::waitForNewConnection");
      closeServer();
   }

   if (result > 0) {
      _q_onNewConnection();
   }

   if (timedOut) {
      *timedOut = (result == 0);
   }
}

void QLocalServerPrivate::setError(const QString &function)
{
   if (EAGAIN == errno) {
      return;
   }

   switch (errno) {
      case EACCES:
         errorString = QLocalServer::tr("%1: Permission denied").formatArg(function);
         error = QAbstractSocket::SocketAccessError;
         break;

      case ELOOP:
      case ENOENT:
      case ENAMETOOLONG:
      case EROFS:
      case ENOTDIR:
         errorString = QLocalServer::tr("%1: Name error").formatArg(function);
         error = QAbstractSocket::HostNotFoundError;
         break;

      case EADDRINUSE:
         errorString = QLocalServer::tr("%1: Address in use").formatArg(function);
         error = QAbstractSocket::AddressInUseError;
         break;

      default:
         errorString = QLocalServer::tr("%1: Unknown error %2").formatArg(function).formatArg(errno);
         error = QAbstractSocket::UnknownSocketError;

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug() << "QLocalServer::setError() Server Name = " << fullServerName << "Error = " << errorString;
#endif
   }
}

#endif // QT_NO_LOCALSERVER
