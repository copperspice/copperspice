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

#ifndef QLOCALSOCKET_P_H
#define QLOCALSOCKET_P_H

#ifndef QT_NO_LOCALSOCKET

#include <qlocalsocket.h>
#include <qiodevice_p.h>
#include <qtimer.h>

#if defined(QT_LOCALSOCKET_TCP)
#   include <qtcpsocket.h>

#elif defined(Q_OS_WIN)
#   include <qwindowspipewriter_p.h>
#   include <qringbuffer_p.h>
#   include <qwineventnotifier_p.h>

#else
#   include <qabstractsocketengine_p.h>
#   include <qtcpsocket.h>
#   include <qsocketnotifier.h>
#   include <errno.h>

#endif

QT_BEGIN_NAMESPACE

#if !defined(Q_OS_WIN) || defined(QT_LOCALSOCKET_TCP)
class QLocalUnixSocket : public QTcpSocket
{

 public:
   QLocalUnixSocket() : QTcpSocket() {
   };

   inline void setSocketState(QAbstractSocket::SocketState state) {
      QTcpSocket::setSocketState(state);
   };

   inline void setErrorString(const QString &string) {
      QTcpSocket::setErrorString(string);
   }

   inline void setSocketError(QAbstractSocket::SocketError error) {
      QTcpSocket::setSocketError(error);
   }

   inline qint64 readData(char *data, qint64 maxSize) {
      return QTcpSocket::readData(data, maxSize);
   }

   inline qint64 writeData(const char *data, qint64 maxSize) {
      return QTcpSocket::writeData(data, maxSize);
   }
};
#endif //#if !defined(Q_OS_WIN) || defined(QT_LOCALSOCKET_TCP)

class QLocalSocketPrivate : public QIODevicePrivate
{
   Q_DECLARE_PUBLIC(QLocalSocket)

 public:
   QLocalSocketPrivate();
   void init();

#if defined(QT_LOCALSOCKET_TCP)
   QLocalUnixSocket *tcpSocket;
   bool ownsTcpSocket;
   void setSocket(QLocalUnixSocket *);
   QString generateErrorString(QLocalSocket::LocalSocketError, const QString &function) const;
   void errorOccurred(QLocalSocket::LocalSocketError, const QString &function);
   void _q_stateChanged(QAbstractSocket::SocketState newState);
   void _q_error(QAbstractSocket::SocketError newError);

#elif defined(Q_OS_WIN)
   ~QLocalSocketPrivate();
   void destroyPipeHandles();
   void setErrorString(const QString &function);
   void _q_notified();
   void _q_canWrite();
   void _q_pipeClosed();
   void _q_emitReadyRead();
   DWORD checkPipeState();
   void startAsyncRead();
   bool completeAsyncRead();
   void checkReadyRead();
   HANDLE handle;
   OVERLAPPED overlapped;
   QWindowsPipeWriter *pipeWriter;
   qint64 readBufferMaxSize;
   QRingBuffer readBuffer;
   int actualReadBufferSize;
   QWinEventNotifier *dataReadNotifier;
   QLocalSocket::LocalSocketError error;
   bool readSequenceStarted;
   bool pendingReadyRead;
   bool pipeClosed;
   static const qint64 initialReadBufferSize = 4096;

#else
   QLocalUnixSocket unixSocket;
   QString generateErrorString(QLocalSocket::LocalSocketError, const QString &function) const;
   void errorOccurred(QLocalSocket::LocalSocketError, const QString &function);
   void _q_stateChanged(QAbstractSocket::SocketState newState);
   void _q_error(QAbstractSocket::SocketError newError);
   void _q_connectToSocket();
   void _q_abortConnectionAttempt();
   void cancelDelayedConnect();
   QSocketNotifier *delayConnect;
   QTimer *connectTimer;
   int connectingSocket;
   QString connectingName;
   QIODevice::OpenMode connectingOpenMode;
#endif

   QString serverName;
   QString fullServerName;
   QLocalSocket::LocalSocketState state;
};

QT_END_NAMESPACE

#endif // QT_NO_LOCALSOCKET

#endif // QLOCALSOCKET_P_H

