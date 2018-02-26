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

#ifndef QNETWORKACCESSFTPBACKEND_P_H
#define QNETWORKACCESSFTPBACKEND_P_H

#include <qnetworkaccessbackend_p.h>
#include <qnetworkaccesscache_p.h>
#include <qnetworkrequest.h>
#include <qnetworkreply.h>
#include <qftp_p.h>
#include <qpointer.h>

#ifndef QT_NO_FTP

class QNetworkAccessFtpIODevice;
class QNetworkAccessCachedFtpConnection;

class QNetworkAccessFtpBackend: public QNetworkAccessBackend
{
   NET_CS_OBJECT(QNetworkAccessFtpBackend)

 public:
   enum State {
      Idle,
      //Connecting,
      LoggingIn,
      CheckingFeatures,
      Statting,
      Transferring,
      Disconnecting
   };

   QNetworkAccessFtpBackend();
   virtual ~QNetworkAccessFtpBackend();

   void open() override;
   void closeDownstreamChannel() override;

   void downstreamReadyWrite() override;

   enum CacheCleanupMode {
       ReleaseCachedConnection,
       RemoveCachedConnection
   };

   void disconnectFromFtp(CacheCleanupMode mode = ReleaseCachedConnection);

   NET_CS_SLOT_1(Public, void ftpConnectionReady(QNetworkAccessCache::CacheableObject *object))
   NET_CS_SLOT_2(ftpConnectionReady)

   NET_CS_SLOT_1(Public, void ftpDone())
   NET_CS_SLOT_2(ftpDone)

   NET_CS_SLOT_1(Public, void ftpReadyRead())
   NET_CS_SLOT_2(ftpReadyRead)

   NET_CS_SLOT_1(Public, void ftpRawCommandReply(int code, const QString &text))
   NET_CS_SLOT_2(ftpRawCommandReply)

 private:
   QPointer<QNetworkAccessCachedFtpConnection> ftp;
   QIODevice *uploadDevice;
   qint64 totalBytes;
   int helpId, sizeId, mdtmId;
   bool supportsSize, supportsMdtm;
   State state;

   friend class QNetworkAccessFtpIODevice;
};

class QNetworkAccessFtpBackendFactory: public QNetworkAccessBackendFactory
{
 public:
   QStringList supportedSchemes() const override;
   QNetworkAccessBackend *create(QNetworkAccessManager::Operation op, const QNetworkRequest &request) const override;
};

#endif // QT_NO_FTP

#endif
