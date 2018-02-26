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

#ifndef QLOCALSOCKET_H
#define QLOCALSOCKET_H

#include <qiodevice.h>
#include <qabstractsocket.h>

#ifndef QT_NO_LOCALSOCKET

#include <qdebug.h>

class QLocalSocketPrivate;

class Q_NETWORK_EXPORT QLocalSocket : public QIODevice
{
   NET_CS_OBJECT(QLocalSocket)
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
      UnknownSocketError = QAbstractSocket::UnknownSocketError,
      OperationError = QAbstractSocket::OperationError
   };

   enum LocalSocketState {
      UnconnectedState = QAbstractSocket::UnconnectedState,
      ConnectingState = QAbstractSocket::ConnectingState,
      ConnectedState = QAbstractSocket::ConnectedState,
      ClosingState = QAbstractSocket::ClosingState
   };

   QLocalSocket(QObject *parent = nullptr);
   ~QLocalSocket();

   void connectToServer(OpenMode openMode = ReadWrite);
   void connectToServer(const QString &name, OpenMode openMode = ReadWrite);
   void disconnectFromServer();

   void setServerName(const QString &name);
   QString serverName() const;
   QString fullServerName() const;

   void abort();
   bool isSequential() const override;
   qint64 bytesAvailable() const override;
   qint64 bytesToWrite() const override;
   bool canReadLine() const override;
   bool open(OpenMode openMode = ReadWrite) override;
   void close() override;

   LocalSocketError error() const;
   bool flush();
   bool isValid() const;
   qint64 readBufferSize() const;
   void setReadBufferSize(qint64 size);

   bool setSocketDescriptor(qintptr socketDescriptor, LocalSocketState socketState = ConnectedState,
                  OpenMode openMode = ReadWrite);

   qintptr socketDescriptor() const;

   LocalSocketState state() const;
   bool waitForBytesWritten(int msecs = 30000) override;
   bool waitForConnected(int msecs = 30000) ;
   bool waitForDisconnected(int msecs = 30000);
   bool waitForReadyRead(int msecs = 30000) override;

   NET_CS_SIGNAL_1(Public, void connected())
   NET_CS_SIGNAL_2(connected)

   NET_CS_SIGNAL_1(Public, void disconnected())
   NET_CS_SIGNAL_2(disconnected)

   NET_CS_SIGNAL_1(Public, void error(QLocalSocket::LocalSocketError socketError))
   NET_CS_SIGNAL_OVERLOAD(error, (QLocalSocket::LocalSocketError), socketError)

   NET_CS_SIGNAL_1(Public, void stateChanged(QLocalSocket::LocalSocketState socketState))
   NET_CS_SIGNAL_2(stateChanged, socketState)

 protected:
   qint64 readData(char *, qint64) override;
   qint64 writeData(const char *, qint64) override;

 private:
   Q_DISABLE_COPY(QLocalSocket)

#if defined(QT_LOCALSOCKET_TCP)
   NET_CS_SLOT_1(Private, void _q_stateChanged(QAbstractSocket::SocketState un_named_arg1))
   NET_CS_SLOT_2(_q_stateChanged)

   NET_CS_SLOT_1(Private, void _q_error(QAbstractSocket::SocketError un_named_arg1))
   NET_CS_SLOT_2(_q_error)

#elif defined(Q_OS_WIN)
   // GONE NET_CS_SLOT_1(Private, void _q_notified())
   // GONE NET_CS_SLOT_2(_q_notified)

   NET_CS_SLOT_1(Private, void _q_canWrite())
   NET_CS_SLOT_2(_q_canWrite)

   NET_CS_SLOT_1(Private, void _q_pipeClosed())
   NET_CS_SLOT_2(_q_pipeClosed)

   NET_CS_SLOT_1(Private, void _q_winError(ulong, const QString &))
   NET_CS_SLOT_2(_q_winError)

   // GONE NET_CS_SLOT_1(Private, void _q_emitReadyRead())
   // GONE NET_CS_SLOT_2(_q_emitReadyRead)

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

Q_NETWORK_EXPORT QDebug operator<<(QDebug, QLocalSocket::LocalSocketError);
Q_NETWORK_EXPORT QDebug operator<<(QDebug, QLocalSocket::LocalSocketState);

#endif // QT_NO_LOCALSOCKET

#endif // QLOCALSOCKET_H
