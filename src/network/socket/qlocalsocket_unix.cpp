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

#include <qlocalsocket.h>

#include <qlocalsocket_p.h>
#include <qnet_unix_p.h>

#ifndef QT_NO_LOCALSOCKET

#include <qdebug.h>
#include <qdir.h>
#include <qelapsedtimer.h>
#include <qstring.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define QT_CONNECT_TIMEOUT 30000

QLocalSocketPrivate::QLocalSocketPrivate()
   : QIODevicePrivate(), delayConnect(nullptr), connectTimer(nullptr), connectingSocket(-1),
      connectingOpenMode(Qt::EmptyFlag), state(QLocalSocket::UnconnectedState)
{
}

void QLocalSocketPrivate::init()
{
   Q_Q(QLocalSocket);

   q->connect(&unixSocket, &QLocalUnixSocket::aboutToClose, q, &QLocalSocket::aboutToClose);
   q->connect(&unixSocket, &QLocalUnixSocket::bytesWritten, q, &QLocalSocket::bytesWritten);
   q->connect(&unixSocket, &QLocalUnixSocket::readyRead,    q, &QLocalSocket::readyRead);

   q->connect(&unixSocket, &QLocalUnixSocket::connected,    q, &QLocalSocket::connected);
   q->connect(&unixSocket, &QLocalUnixSocket::disconnected, q, &QLocalSocket::disconnected);
   q->connect(&unixSocket, &QLocalUnixSocket::stateChanged, q, &QLocalSocket::_q_stateChanged);
   q->connect(&unixSocket, &QLocalUnixSocket::error,        q, &QLocalSocket::_q_error);

   q->connect(&unixSocket, &QLocalUnixSocket::readChannelFinished, q, &QLocalSocket::readChannelFinished);

   unixSocket.setParent(q);
}

void QLocalSocketPrivate::_q_error(QAbstractSocket::SocketError socketError)
{
   Q_Q(QLocalSocket);

   QString function = "QLocalSocket";
   QLocalSocket::LocalSocketError error = (QLocalSocket::LocalSocketError)socketError;

   QString errorString = generateErrorString(error, function);
   q->setErrorString(errorString);

   emit q->error(error);
}

void QLocalSocketPrivate::_q_stateChanged(QAbstractSocket::SocketState newState)
{
   Q_Q(QLocalSocket);
   QLocalSocket::LocalSocketState currentState = state;

   switch (newState) {
      case QAbstractSocket::UnconnectedState:
         state = QLocalSocket::UnconnectedState;
         serverName.clear();
         fullServerName.clear();
         break;

      case QAbstractSocket::ConnectingState:
         state = QLocalSocket::ConnectingState;
         break;

      case QAbstractSocket::ConnectedState:
         state = QLocalSocket::ConnectedState;
         break;

      case QAbstractSocket::ClosingState:
         state = QLocalSocket::ClosingState;
         break;

      default:
#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug() << "QLocalSocket::_q_stateChanged() New socket state = " << newState;
#endif
         return;
   }

   if (currentState != state) {
      emit q->stateChanged(state);
   }
}

QString QLocalSocketPrivate::generateErrorString(QLocalSocket::LocalSocketError error, const QString &function) const
{
   QString errorString;

   switch (error) {
      case QLocalSocket::ConnectionRefusedError:
         errorString = QLocalSocket::tr("%1: Connection refused").formatArg(function);
         break;

      case QLocalSocket::PeerClosedError:
         errorString = QLocalSocket::tr("%1: Remote closed").formatArg(function);
         break;

      case QLocalSocket::ServerNotFoundError:
         errorString = QLocalSocket::tr("%1: Invalid name").formatArg(function);
         break;

      case QLocalSocket::SocketAccessError:
         errorString = QLocalSocket::tr("%1: Socket access error").formatArg(function);
         break;

      case QLocalSocket::SocketResourceError:
         errorString = QLocalSocket::tr("%1: Socket resource error").formatArg(function);
         break;

      case QLocalSocket::SocketTimeoutError:
         errorString = QLocalSocket::tr("%1: Socket operation timed out").formatArg(function);
         break;

      case QLocalSocket::DatagramTooLargeError:
         errorString = QLocalSocket::tr("%1: Datagram too large").formatArg(function);
         break;

      case QLocalSocket::ConnectionError:
         errorString = QLocalSocket::tr("%1: Connection error").formatArg(function);
         break;

      case QLocalSocket::UnsupportedSocketOperationError:
         errorString = QLocalSocket::tr("%1: The socket operation is not supported").formatArg(function);
         break;

      case QLocalSocket::OperationError:
         errorString = QLocalSocket::tr("%1: Operation not permitted when socket is in this state").formatArg(function);
         break;

      case QLocalSocket::UnknownSocketError:
      default:
         errorString = QLocalSocket::tr("%1: Unknown error %2").formatArg(function).formatArg(errno);
   }

   return errorString;
}

void QLocalSocketPrivate::errorOccurred(QLocalSocket::LocalSocketError error, const QString &function)
{
   Q_Q(QLocalSocket);

   switch (error) {
      case QLocalSocket::ConnectionRefusedError:
         unixSocket.setSocketError(QAbstractSocket::ConnectionRefusedError);
         break;

      case QLocalSocket::PeerClosedError:
         unixSocket.setSocketError(QAbstractSocket::RemoteHostClosedError);
         break;

      case QLocalSocket::ServerNotFoundError:
         unixSocket.setSocketError(QAbstractSocket::HostNotFoundError);
         break;

      case QLocalSocket::SocketAccessError:
         unixSocket.setSocketError(QAbstractSocket::SocketAccessError);
         break;

      case QLocalSocket::SocketResourceError:
         unixSocket.setSocketError(QAbstractSocket::SocketResourceError);
         break;

      case QLocalSocket::SocketTimeoutError:
         unixSocket.setSocketError(QAbstractSocket::SocketTimeoutError);
         break;

      case QLocalSocket::DatagramTooLargeError:
         unixSocket.setSocketError(QAbstractSocket::DatagramTooLargeError);
         break;

      case QLocalSocket::ConnectionError:
         unixSocket.setSocketError(QAbstractSocket::NetworkError);
         break;

      case QLocalSocket::UnsupportedSocketOperationError:
         unixSocket.setSocketError(QAbstractSocket::UnsupportedSocketOperationError);
         break;

      case QLocalSocket::UnknownSocketError:
      default:
         unixSocket.setSocketError(QAbstractSocket::UnknownSocketError);
   }

   QString errorString = generateErrorString(error, function);
   q->setErrorString(errorString);
   emit q->error(error);

   // errors cause a disconnect
   unixSocket.setSocketState(QAbstractSocket::UnconnectedState);
   bool stateChanged = (state != QLocalSocket::UnconnectedState);
   state = QLocalSocket::UnconnectedState;
   q->close();

   if (stateChanged) {
      q->emit stateChanged(state);
   }
}

void QLocalSocket::connectToServer(OpenMode openMode)
{
   Q_D(QLocalSocket);

   if (state() == ConnectedState || state() == ConnectingState) {
      QString errorString = d->generateErrorString(QLocalSocket::OperationError, "QLocalSocket::connectToserver");
      setErrorString(errorString);
      emit error(QLocalSocket::OperationError);
      return;
   }

   d->errorString.clear();
   d->unixSocket.setSocketState(QAbstractSocket::ConnectingState);
   d->state = ConnectingState;
   emit stateChanged(d->state);

   if (d->serverName.isEmpty()) {
      d->errorOccurred(ServerNotFoundError, "QLocalSocket::connectToServer");
      return;
   }

   // create the socket
   if (-1 == (d->connectingSocket = qt_safe_socket(PF_UNIX, SOCK_STREAM, 0, O_NONBLOCK))) {
      d->errorOccurred(UnsupportedSocketOperationError, "QLocalSocket::connectToServer");
      return;
   }

   // _q_connectToSocket does the actual connecting
   d->connectingName = d->serverName;
   d->connectingOpenMode = openMode;
   d->_q_connectToSocket();

   return;
}

void QLocalSocketPrivate::_q_connectToSocket()
{
   Q_Q(QLocalSocket);

   QString connectingPathName;

   // determine the full server path
   if (connectingName.startsWith(QChar('/'))) {
      connectingPathName = connectingName;

   } else {
      connectingPathName = QDir::tempPath();
      connectingPathName += QChar('/') + connectingName;
   }

   const QByteArray encodedConnectingPathName = QFile::encodeName(connectingPathName);
   struct sockaddr_un name;
   name.sun_family = PF_UNIX;

   if (sizeof(name.sun_path) < (uint)encodedConnectingPathName.size() + 1) {
      QString function = "QLocalSocket::connectToServer";
      errorOccurred(QLocalSocket::ServerNotFoundError, function);
      return;
   }

   ::memcpy(name.sun_path, encodedConnectingPathName.constData(),
            encodedConnectingPathName.size() + 1);

   if (-1 == qt_safe_connect(connectingSocket, (struct sockaddr *)&name, sizeof(name))) {
      QString function = "QLocalSocket::connectToServer";

      switch (errno) {
         case EINVAL:
         case ECONNREFUSED:
            errorOccurred(QLocalSocket::ConnectionRefusedError, function);
            break;

         case ENOENT:
            errorOccurred(QLocalSocket::ServerNotFoundError, function);
            break;

         case EACCES:
         case EPERM:
            errorOccurred(QLocalSocket::SocketAccessError, function);
            break;

         case ETIMEDOUT:
            errorOccurred(QLocalSocket::SocketTimeoutError, function);
            break;

         case EAGAIN:
            // Try again later, all of the sockets listening are full
            if (! delayConnect) {
               delayConnect = new QSocketNotifier(connectingSocket, QSocketNotifier::Write, q);
               q->connect(delayConnect, &QSocketNotifier::activated, q, &QLocalSocket::_q_connectToSocket);
            }

            if (! connectTimer) {
               connectTimer = new QTimer(q);

               q->connect(connectTimer, &QTimer::timeout, q, &QLocalSocket::_q_abortConnectionAttempt, Qt::DirectConnection);
               connectTimer->start(QT_CONNECT_TIMEOUT);
            }

            delayConnect->setEnabled(true);
            break;

         default:
            errorOccurred(QLocalSocket::UnknownSocketError, function);
      }

      return;
   }

   cancelDelayedConnect();

   serverName = connectingName;
   fullServerName = connectingPathName;

   if (unixSocket.setSocketDescriptor(connectingSocket, QAbstractSocket::ConnectedState, connectingOpenMode)) {
      q->QIODevice::open(connectingOpenMode | QIODevice::Unbuffered);
      q->emit connected();

   } else {
      QString function = "QLocalSocket::connectToServer";
      errorOccurred(QLocalSocket::UnknownSocketError, function);
   }

   connectingSocket = -1;
   connectingName.clear();
   connectingOpenMode = Qt::EmptyFlag;
}

bool QLocalSocket::setSocketDescriptor(qintptr socketDescriptor, LocalSocketState socketState, OpenMode openMode)
{
   Q_D(QLocalSocket);
   QAbstractSocket::SocketState newSocketState = QAbstractSocket::UnconnectedState;

   switch (socketState) {
      case ConnectingState:
         newSocketState = QAbstractSocket::ConnectingState;
         break;

      case ConnectedState:
         newSocketState = QAbstractSocket::ConnectedState;
         break;

      case ClosingState:
         newSocketState = QAbstractSocket::ClosingState;
         break;

      case UnconnectedState:
         newSocketState = QAbstractSocket::UnconnectedState;
         break;
   }
   QIODevice::open(openMode);
   d->state = socketState;
   return d->unixSocket.setSocketDescriptor(socketDescriptor,
          newSocketState, openMode);
}

void QLocalSocketPrivate::_q_abortConnectionAttempt()
{
   Q_Q(QLocalSocket);
   q->close();
}

void QLocalSocketPrivate::cancelDelayedConnect()
{
   if (delayConnect) {
      delayConnect->setEnabled(false);
      delete delayConnect;

      delayConnect = nullptr;
      connectTimer->stop();
      delete connectTimer;

      connectTimer = nullptr;
   }
}

qintptr QLocalSocket::socketDescriptor() const
{
   Q_D(const QLocalSocket);
   return d->unixSocket.socketDescriptor();
}

qint64 QLocalSocket::readData(char *data, qint64 c)
{
   Q_D(QLocalSocket);
   return d->unixSocket.read(data, c);
}

qint64 QLocalSocket::writeData(const char *data, qint64 c)
{
   Q_D(QLocalSocket);
   return d->unixSocket.writeData(data, c);
}

void QLocalSocket::abort()
{
   Q_D(QLocalSocket);
   d->unixSocket.abort();
}

qint64 QLocalSocket::bytesAvailable() const
{
   Q_D(const QLocalSocket);
   return QIODevice::bytesAvailable() + d->unixSocket.bytesAvailable();
}

qint64 QLocalSocket::bytesToWrite() const
{
   Q_D(const QLocalSocket);
   return d->unixSocket.bytesToWrite();
}

bool QLocalSocket::canReadLine() const
{
   Q_D(const QLocalSocket);
   return QIODevice::canReadLine() || d->unixSocket.canReadLine();
}

void QLocalSocket::close()
{
   Q_D(QLocalSocket);
   d->unixSocket.close();
   d->cancelDelayedConnect();

   if (d->connectingSocket != -1) {
      ::close(d->connectingSocket);
   }

   d->connectingSocket = -1;
   d->connectingName.clear();
   d->connectingOpenMode = Qt::EmptyFlag;
   d->serverName.clear();
   d->fullServerName.clear();

   QIODevice::close();
}

bool QLocalSocket::waitForBytesWritten(int msecs)
{
   Q_D(QLocalSocket);
   return d->unixSocket.waitForBytesWritten(msecs);
}

bool QLocalSocket::flush()
{
   Q_D(QLocalSocket);
   return d->unixSocket.flush();
}

void QLocalSocket::disconnectFromServer()
{
   Q_D(QLocalSocket);
   d->unixSocket.disconnectFromHost();
}

QLocalSocket::LocalSocketError QLocalSocket::error() const
{
   Q_D(const QLocalSocket);

   switch (d->unixSocket.error()) {
      case QAbstractSocket::ConnectionRefusedError:
         return QLocalSocket::ConnectionRefusedError;

      case QAbstractSocket::RemoteHostClosedError:
         return QLocalSocket::PeerClosedError;

      case QAbstractSocket::HostNotFoundError:
         return QLocalSocket::ServerNotFoundError;

      case QAbstractSocket::SocketAccessError:
         return QLocalSocket::SocketAccessError;

      case QAbstractSocket::SocketResourceError:
         return QLocalSocket::SocketResourceError;

      case QAbstractSocket::SocketTimeoutError:
         return QLocalSocket::SocketTimeoutError;

      case QAbstractSocket::DatagramTooLargeError:
         return QLocalSocket::DatagramTooLargeError;

      case QAbstractSocket::NetworkError:
         return QLocalSocket::ConnectionError;

      case QAbstractSocket::UnsupportedSocketOperationError:
         return QLocalSocket::UnsupportedSocketOperationError;

      case QAbstractSocket::UnknownSocketError:
         return QLocalSocket::UnknownSocketError;

      default:
#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug() << "QLocalSocket::error() Error not handled, " << d->unixSocket.error();
#endif
         break;
   }
   return UnknownSocketError;
}

bool QLocalSocket::isValid() const
{
   Q_D(const QLocalSocket);
   return d->unixSocket.isValid();
}

qint64 QLocalSocket::readBufferSize() const
{
   Q_D(const QLocalSocket);
   return d->unixSocket.readBufferSize();
}

void QLocalSocket::setReadBufferSize(qint64 size)
{
   Q_D(QLocalSocket);
   d->unixSocket.setReadBufferSize(size);
}

bool QLocalSocket::waitForConnected(int msec)
{
   Q_D(QLocalSocket);
   if (state() != ConnectingState) {
      return (state() == ConnectedState);
   }

   fd_set fds;
   FD_ZERO(&fds);
   FD_SET(d->connectingSocket, &fds);

   timeval timeout;
   timeout.tv_sec = msec / 1000;
   timeout.tv_usec = (msec % 1000) * 1000;

   // timeout can not be 0 or else select will return an error.
   if (0 == msec) {
      timeout.tv_usec = 1000;
   }

   int result = -1;
   // on Linux timeout will be updated by select, but _not_ on other systems.
   QElapsedTimer timer;
   timer.start();

   while (state() == ConnectingState && (-1 == msec || timer.elapsed() < msec)) {

      result = ::select(d->connectingSocket + 1, &fds, nullptr, nullptr, &timeout);

      if (-1 == result && errno != EINTR) {
         d->errorOccurred( QLocalSocket::UnknownSocketError, "QLocalSocket::waitForConnected");
         break;
      }

      if (result > 0) {
         d->_q_connectToSocket();
      }
   }

   return (state() == ConnectedState);
}

bool QLocalSocket::waitForDisconnected(int msecs)
{
   Q_D(QLocalSocket);
   if (state() == UnconnectedState) {
      qWarning("QLocalSocket::waitForDisconnected() Not allowed in UnconnectedState");
      return false;
   }
   return (d->unixSocket.waitForDisconnected(msecs));
}

bool QLocalSocket::waitForReadyRead(int msecs)
{
   Q_D(QLocalSocket);
   if (state() == QLocalSocket::UnconnectedState) {
      return false;
   }
   return (d->unixSocket.waitForReadyRead(msecs));
}

#endif
