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

#include <qlocalsocket.h>
#include <qlocalsocket_p.h>

#ifndef QT_NO_LOCALSOCKET

QLocalSocket::QLocalSocket(QObject *parent)
   : QIODevice(*new QLocalSocketPrivate, parent)
{
   Q_D(QLocalSocket);
   d->init();
}

/*!
    Destroys the socket, closing the connection if necessary.
 */
QLocalSocket::~QLocalSocket()
{
   close();

#if !defined(Q_OS_WIN) && ! defined(QT_LOCALSOCKET_TCP)
   Q_D(QLocalSocket);
   d->unixSocket.setParent(0);
#endif
}

bool QLocalSocket::open(OpenMode openMode)
{
   connectToServer(openMode);
   return isOpen();
}
void QLocalSocket::connectToServer(const QString &name, OpenMode openMode)
{
   setServerName(name);
   connectToServer(openMode);
}

void QLocalSocket::setServerName(const QString &name)
{
   Q_D(QLocalSocket);
   if (d->state != UnconnectedState) {
      qWarning("QLocalSocket::setServerName() called while not in unconnected state");
      return;
   }
   d->serverName = name;
}

QString QLocalSocket::serverName() const
{
   Q_D(const QLocalSocket);
   return d->serverName;
}

/*!
    Returns the server path that the socket is connected to.

    \note The return value of this function is platform specific.

    \sa connectToServer(), serverName()
 */
QString QLocalSocket::fullServerName() const
{
   Q_D(const QLocalSocket);
   return d->fullServerName;
}

QLocalSocket::LocalSocketState QLocalSocket::state() const
{
   Q_D(const QLocalSocket);
   return d->state;
}

bool QLocalSocket::isSequential() const
{
   return true;
}

QDebug operator<<(QDebug debug, QLocalSocket::LocalSocketError error)
{
   // QDebugStateSaver saver(debug);
   // debug.resetFormat().nospace();

   switch (error) {
      case QLocalSocket::ConnectionRefusedError:
         debug << "QLocalSocket::ConnectionRefusedError";
         break;
      case QLocalSocket::PeerClosedError:
         debug << "QLocalSocket::PeerClosedError";
         break;
      case QLocalSocket::ServerNotFoundError:
         debug << "QLocalSocket::ServerNotFoundError";
         break;
      case QLocalSocket::SocketAccessError:
         debug << "QLocalSocket::SocketAccessError";
         break;
      case QLocalSocket::SocketResourceError:
         debug << "QLocalSocket::SocketResourceError";
         break;
      case QLocalSocket::SocketTimeoutError:
         debug << "QLocalSocket::SocketTimeoutError";
         break;
      case QLocalSocket::DatagramTooLargeError:
         debug << "QLocalSocket::DatagramTooLargeError";
         break;
      case QLocalSocket::ConnectionError:
         debug << "QLocalSocket::ConnectionError";
         break;
      case QLocalSocket::UnsupportedSocketOperationError:
         debug << "QLocalSocket::UnsupportedSocketOperationError";
         break;
      case QLocalSocket::UnknownSocketError:
         debug << "QLocalSocket::UnknownSocketError";
         break;
      default:
         debug << "QLocalSocket::SocketError(" << int(error) << ')';
         break;
   }
   return debug;
}

QDebug operator<<(QDebug debug, QLocalSocket::LocalSocketState state)
{
   // QDebugStateSaver saver(debug);
   // debug.resetFormat().nospace();

   switch (state) {
      case QLocalSocket::UnconnectedState:
         debug << "QLocalSocket::UnconnectedState";
         break;
      case QLocalSocket::ConnectingState:
         debug << "QLocalSocket::ConnectingState";
         break;
      case QLocalSocket::ConnectedState:
         debug << "QLocalSocket::ConnectedState";
         break;
      case QLocalSocket::ClosingState:
         debug << "QLocalSocket::ClosingState";
         break;
      default:
         debug << "QLocalSocket::SocketState(" << int(state) << ')';
         break;
   }
   return debug;
}

#if defined(QT_LOCALSOCKET_TCP)

void QLocalSocket::_q_stateChanged(QAbstractSocket::SocketState un_named_arg1)
{
   Q_D(QLocalSocket);
   d->_q_stateChanged(un_named_arg1);
}

void QLocalSocket::_q_error(QAbstractSocket::SocketError un_named_arg1)
{
   Q_D(QLocalSocket);
   d->_q_error(un_named_arg1);
}

#elif defined(Q_OS_WIN)

/* GONE
void QLocalSocket::_q_notified()
{
   Q_D(QLocalSocket);
   d->_q_notified();
}
*/

void QLocalSocket::_q_canWrite()
{
   Q_D(QLocalSocket);
   d->_q_canWrite();
}

void QLocalSocket::_q_pipeClosed()
{
   Q_D(QLocalSocket);
   d->_q_pipeClosed();
}

void QLocalSocket::_q_winError(ulong data1, const QString &data2)
{
   Q_D(QLocalSocket);
   d->_q_winError(data1, data2);
}

/* GONE
void QLocalSocket::_q_emitReadyRead()
{
   Q_D(QLocalSocket);
   d->_q_emitReadyRead();
}
*/

#else

void QLocalSocket::_q_stateChanged(QAbstractSocket::SocketState un_named_arg1)
{
   Q_D(QLocalSocket);
   d->_q_stateChanged(un_named_arg1);
}

void QLocalSocket::_q_error(QAbstractSocket::SocketError un_named_arg1)
{
   Q_D(QLocalSocket);
   d->_q_error(un_named_arg1);
}

void QLocalSocket::_q_connectToSocket()
{
   Q_D(QLocalSocket);
   d->_q_connectToSocket();
}

void QLocalSocket::_q_abortConnectionAttempt()
{
   Q_D(QLocalSocket);
   d->_q_abortConnectionAttempt();
}

#endif

#endif
