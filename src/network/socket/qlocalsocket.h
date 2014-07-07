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

#ifndef QLOCALSOCKET_H
#define QLOCALSOCKET_H

#include <QtCore/qiodevice.h>
#include <QtNetwork/qabstractsocket.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_LOCALSOCKET

class QLocalSocketPrivate;

class Q_NETWORK_EXPORT QLocalSocket : public QIODevice
{
   CS_OBJECT(QLocalSocket)
   Q_DECLARE_PRIVATE(QLocalSocket)

 public:
   enum LocalSocketError {
      ConnectionRefusedError = QAbstractSocket::ConnectionRefusedError,
      PeerClosedError = QAbstractSocket::RemoteHostClosedError,
      ServerNotFoundError = QAbstractSocket::HostNotFoundError,
      SocketAccessError = QAbstractSocket::SocketAccessError,
      SocketResourceError = QAbstractSocket::SocketResourceError,
      SocketTimeoutError = QAbstractSocket::SocketTimeoutError,
      DatagramTooLargeError = QAbstractSocket::DatagramTooLargeError,
      ConnectionError = QAbstractSocket::NetworkError,
      UnsupportedSocketOperationError = QAbstractSocket::UnsupportedSocketOperationError,
      UnknownSocketError = QAbstractSocket::UnknownSocketError
   };

   enum LocalSocketState {
      UnconnectedState = QAbstractSocket::UnconnectedState,
      ConnectingState = QAbstractSocket::ConnectingState,
      ConnectedState = QAbstractSocket::ConnectedState,
      ClosingState = QAbstractSocket::ClosingState
   };

   QLocalSocket(QObject *parent = 0);
   ~QLocalSocket();

   void connectToServer(const QString &name, OpenMode openMode = ReadWrite);
   void disconnectFromServer();

   QString serverName() const;
   QString fullServerName() const;

   void abort();
   virtual bool isSequential() const;
   virtual qint64 bytesAvailable() const;
   virtual qint64 bytesToWrite() const;
   virtual bool canReadLine() const;
   virtual void close();
   LocalSocketError error() const;
   bool flush();
   bool isValid() const;
   qint64 readBufferSize() const;
   void setReadBufferSize(qint64 size);

   bool setSocketDescriptor(quintptr socketDescriptor,
                            LocalSocketState socketState = ConnectedState,
                            OpenMode openMode = ReadWrite);
   quintptr socketDescriptor() const;

   LocalSocketState state() const;
   bool waitForBytesWritten(int msecs = 30000);
   bool waitForConnected(int msecs = 30000);
   bool waitForDisconnected(int msecs = 30000);
   bool waitForReadyRead(int msecs = 30000);

   NET_CS_SIGNAL_1(Public, void connected())
   NET_CS_SIGNAL_2(connected)

   NET_CS_SIGNAL_1(Public, void disconnected())
   NET_CS_SIGNAL_2(disconnected)

   NET_CS_SIGNAL_1(Public, void error(QLocalSocket::LocalSocketError socketError))
   NET_CS_SIGNAL_OVERLOAD(error, (QLocalSocket::LocalSocketError), socketError)

   NET_CS_SIGNAL_1(Public, void stateChanged(QLocalSocket::LocalSocketState socketState))
   NET_CS_SIGNAL_2(stateChanged, socketState)

 protected:
   virtual qint64 readData(char *, qint64);
   virtual qint64 writeData(const char *, qint64);

 private:
   Q_DISABLE_COPY(QLocalSocket)

#if defined(QT_LOCALSOCKET_TCP)
   NET_CS_SLOT_1(Private, void _q_stateChanged(QAbstractSocket::SocketState un_named_arg1))
   NET_CS_SLOT_2(_q_stateChanged)

   NET_CS_SLOT_1(Private, void _q_error(QAbstractSocket::SocketError un_named_arg1))
   NET_CS_SLOT_2(_q_error)

#elif defined(Q_OS_WIN)
   NET_CS_SLOT_1(Private, void _q_notified())
   NET_CS_SLOT_2(_q_notified)

   NET_CS_SLOT_1(Private, void _q_canWrite())
   NET_CS_SLOT_2(_q_canWrite)

   NET_CS_SLOT_1(Private, void _q_pipeClosed())
   NET_CS_SLOT_2(_q_pipeClosed)

   NET_CS_SLOT_1(Private, void _q_emitReadyRead())
   NET_CS_SLOT_2(_q_emitReadyRead)

#else
   NET_CS_SLOT_1(Private, void _q_stateChanged(QAbstractSocket::SocketState un_named_arg1))
   NET_CS_SLOT_2(_q_stateChanged)

   NET_CS_SLOT_1(Private, void _q_error(QAbstractSocket::SocketError un_named_arg1))
   NET_CS_SLOT_2(_q_error)

   NET_CS_SLOT_1(Private, void _q_connectToSocket())
   NET_CS_SLOT_2(_q_connectToSocket)

   NET_CS_SLOT_1(Private, void _q_abortConnectionAttempt())
   NET_CS_SLOT_2(_q_abortConnectionAttempt)
#endif

};

#include <QtCore/qdebug.h>

Q_NETWORK_EXPORT QDebug operator<<(QDebug, QLocalSocket::LocalSocketError);
Q_NETWORK_EXPORT QDebug operator<<(QDebug, QLocalSocket::LocalSocketState);

#endif // QT_NO_LOCALSOCKET

QT_END_NAMESPACE

#endif // QLOCALSOCKET_H
