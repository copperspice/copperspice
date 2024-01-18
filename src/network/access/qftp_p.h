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

#ifndef QFTP_P_H
#define QFTP_P_H

#include <qstring.h>
#include <qobject.h>
#include <qscopedpointer.h>

#include <qurlinfo_p.h>

#ifndef QT_NO_FTP

class QFtpPrivate;

class Q_NETWORK_EXPORT QFtp : public QObject
{
   NET_CS_OBJECT(QFtp)

 public:
   enum State {
      Unconnected,
      HostLookup,
      Connecting,
      Connected,
      LoggedIn,
      Closing
   };
   enum Error {
      NoError,
      UnknownError,
      HostNotFound,
      ConnectionRefused,
      NotConnected
   };
   enum Command {
      None,
      SetTransferMode,
      SetProxy,
      ConnectToHost,
      Login,
      Close,
      List,
      Cd,
      Get,
      Put,
      Remove,
      Mkdir,
      Rmdir,
      Rename,
      RawCommand
   };

   enum TransferMode {
      Active,
      Passive
   };

   enum TransferType {
      Binary,
      Ascii
   };

   explicit QFtp(QObject *parent = nullptr);

   QFtp(const QFtp &) = delete;
   QFtp &operator=(const QFtp &) = delete;

   virtual ~QFtp();

   int setProxy(const QString &host, quint16 port);
   int connectToHost(const QString &host, quint16 port = 21);
   int login(const QString &user = QString(), const QString &password = QString());
   int close();
   int setTransferMode(TransferMode mode);
   int list(const QString &dir = QString());
   int cd(const QString &dir);
   int get(const QString &file, QIODevice *dev = nullptr, TransferType type = Binary);
   int put(const QByteArray &data, const QString &file, TransferType type = Binary);
   int put(QIODevice *dev, const QString &file, TransferType type = Binary);
   int remove(const QString &file);
   int mkdir(const QString &dir);
   int rmdir(const QString &dir);
   int rename(const QString &oldname, const QString &newname);

   int rawCommand(const QString &command);

   qint64 bytesAvailable() const;
   qint64 read(char *data, qint64 maxlen);
   QByteArray readAll();

   int currentId() const;
   QIODevice *currentDevice() const;
   Command currentCommand() const;
   bool hasPendingCommands() const;
   void clearPendingCommands();

   State state() const;

   Error error() const;
   QString errorString() const;

   NET_CS_SLOT_1(Public, void abort())
   NET_CS_SLOT_2(abort)

   NET_CS_SIGNAL_1(Public, void stateChanged(int state))
   NET_CS_SIGNAL_2(stateChanged, state)

   NET_CS_SIGNAL_1(Public, void listInfo(const QUrlInfo &urlInfo))
   NET_CS_SIGNAL_2(listInfo, urlInfo)

   NET_CS_SIGNAL_1(Public, void readyRead())
   NET_CS_SIGNAL_2(readyRead)

   NET_CS_SIGNAL_1(Public, void dataTransferProgress(qint64 done, qint64 total))
   NET_CS_SIGNAL_2(dataTransferProgress, done, total)

   NET_CS_SIGNAL_1(Public, void rawCommandReply(int replyCode, const QString &detail))
   NET_CS_SIGNAL_2(rawCommandReply, replyCode, detail )

   NET_CS_SIGNAL_1(Public, void commandStarted(int id))
   NET_CS_SIGNAL_2(commandStarted, id)

   NET_CS_SIGNAL_1(Public, void commandFinished(int id, bool error))
   NET_CS_SIGNAL_2(commandFinished, id, error)

   NET_CS_SIGNAL_1(Public, void done(bool error))
   NET_CS_SIGNAL_2(done, error)

 protected:
   QScopedPointer<QFtpPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QFtp)

   NET_CS_SLOT_1(Private, void _q_startNextCommand())
   NET_CS_SLOT_2(_q_startNextCommand)

   NET_CS_SLOT_1(Private, void _q_piFinished(const QString &textMsg))
   NET_CS_SLOT_2(_q_piFinished)

   NET_CS_SLOT_1(Private, void _q_piError(int errorCode, const QString &textMsg))
   NET_CS_SLOT_2(_q_piError)

   NET_CS_SLOT_1(Private, void _q_piConnectState(int connectState))
   NET_CS_SLOT_2(_q_piConnectState)

   NET_CS_SLOT_1(Private, void _q_piFtpReply(int code, const QString &textMsg))
   NET_CS_SLOT_2(_q_piFtpReply)
};

#endif // QT_NO_FTP

#endif
