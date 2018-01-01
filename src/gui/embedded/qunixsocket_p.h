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

#ifndef QUNIXSOCKET_P_H
#define QUNIXSOCKET_P_H

#include <QtNetwork/qabstractsocket.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qlist.h>
#include <QtCore/qshareddata.h>

extern "C" {
#include <sys/types.h>
};

QT_BEGIN_NAMESPACE

class QUnixSocketRights;
class QUnixSocketRightsPrivate;
class QUnixSocketPrivate;
class QUnixSocketMessagePrivate;
struct iovec;

class Q_GUI_EXPORT QUnixSocketRights
{
 public:
   QUnixSocketRights(int);
   ~QUnixSocketRights();

   QUnixSocketRights(const QUnixSocketRights &);
   QUnixSocketRights &operator=(const QUnixSocketRights &);

   bool isValid() const;

   int dupFd() const;
   int peekFd() const;

 private:
   friend class QUnixSocket;
   QUnixSocketRights(int, int);
   QSharedDataPointer<QUnixSocketRightsPrivate> d;
};

class Q_GUI_EXPORT QUnixSocketMessage
{
 public:
   QUnixSocketMessage();
   QUnixSocketMessage(const QByteArray &);
   QUnixSocketMessage(const QByteArray &, const QList<QUnixSocketRights> &);
   QUnixSocketMessage(const QUnixSocketMessage &);
   QUnixSocketMessage(const iovec *, int);
   QUnixSocketMessage &operator=(const QUnixSocketMessage &);
   ~QUnixSocketMessage();

   void setBytes(const QByteArray &);
   void setRights(const QList<QUnixSocketRights> &);

   const QList<QUnixSocketRights> &rights() const;
   bool rightsWereTruncated() const;

   const QByteArray &bytes() const;

   pid_t processId() const;
   uid_t userId() const;
   gid_t groupId() const;

   void setProcessId(pid_t);
   void setUserId(uid_t);
   void setGroupId(gid_t);

   bool isValid() const;

 private:
   friend class QUnixSocket;
   friend class QUnixSocketPrivate;
   QSharedDataPointer<QUnixSocketMessagePrivate> d;
};

class Q_GUI_EXPORT QUnixSocket : public QIODevice
{
   GUI_CS_OBJECT(QUnixSocket)

 public:
   QUnixSocket(QObject * = 0);
   QUnixSocket(qint64, qint64, QObject * = 0);
   virtual ~QUnixSocket();

   enum SocketState {
      UnconnectedState = QAbstractSocket::UnconnectedState,
      HostLookupState = QAbstractSocket::HostLookupState,
      ConnectingState = QAbstractSocket::ConnectingState,
      ConnectedState = QAbstractSocket::ConnectedState,
      BoundState = QAbstractSocket::BoundState,
      ClosingState = QAbstractSocket::ClosingState,
      ListeningState = QAbstractSocket::ListeningState,
   };

   enum SocketError { NoError, InvalidPath, ResourceError,
                      NonexistentPath, ConnectionRefused, UnknownError,
                      ReadFailure, WriteFailure
                    };

   bool connect(const QByteArray &path);
   bool setSocketDescriptor(int socketDescriptor);
   int socketDescriptor() const;
   void abort();
   void close();

   bool flush();

   SocketError error() const;

   SocketState state() const;
   QByteArray address() const;

   qint64 bytesAvailable() const;
   qint64 bytesToWrite() const;

   qint64 readBufferSize() const;
   void setReadBufferSize(qint64 size);
   qint64 rightsBufferSize() const;
   void setRightsBufferSize(qint64 size);

   bool canReadLine() const;

   qint64 write(const char *data, qint64 maxSize) {
      return QIODevice::write(data, maxSize);
   }
   qint64 write(const QByteArray &byteArray) {
      return QIODevice::write(byteArray);
   }
   qint64 read(char *data, qint64 maxSize) {
      return QIODevice::read(data, maxSize);
   }
   QByteArray read(qint64 maxSize) {
      return QIODevice::read(maxSize);
   }

   qint64 write(const QUnixSocketMessage &);
   QUnixSocketMessage read();

   virtual bool isSequential() const;
   virtual bool waitForReadyRead(int msec = 300);
   virtual bool waitForBytesWritten(int msec = 300);

   GUI_CS_SIGNAL_1(Public, void stateChanged(SocketState socketState))
   GUI_CS_SIGNAL_2(stateChanged, socketState)

 protected:
   virtual qint64 readData(char *data, qint64 maxSize);
   virtual qint64 writeData (const char *data, qint64 maxSize);

 private:
   QUnixSocket(const QUnixSocket &);
   QUnixSocket &operator=(const QUnixSocket &);

   QUnixSocketPrivate *d;
};

QT_END_NAMESPACE

#endif // QUNIXSOCKET_P_H
