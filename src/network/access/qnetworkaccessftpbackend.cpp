/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qnetworkaccessftpbackend_p.h>
#include <qnetworkaccessmanager_p.h>
#include <qauthenticator.h>
#include <qnoncontiguousbytedevice_p.h>
#include <qstring.h>
#include <qstringlist.h>

#ifndef QT_NO_FTP

enum {
   DefaultFtpPort = 21
};

static QByteArray makeCacheKey(const QUrl &url)
{
   QUrl copy = url;
   copy.setPort(url.port(DefaultFtpPort));

   return "ftp-connection:" +
          copy.toEncoded(QUrl::RemovePassword | QUrl::RemovePath | QUrl::RemoveQuery | QUrl::RemoveFragment);
}

QStringList QNetworkAccessFtpBackendFactory::supportedSchemes() const
{
   return QStringList("ftp");
}
QNetworkAccessBackend *
QNetworkAccessFtpBackendFactory::create(QNetworkAccessManager::Operation op, const QNetworkRequest &request) const
{
   // is it an operation we know of?
   switch (op) {
      case QNetworkAccessManager::GetOperation:
      case QNetworkAccessManager::PutOperation:
         break;

      default:
         // no, we can't handle this operation
         return 0;
   }

   QUrl url = request.url();
   if (url.scheme().compare("ftp", Qt::CaseInsensitive) == 0) {
      return new QNetworkAccessFtpBackend;
   }

   return 0;
}

class QNetworkAccessCachedFtpConnection : public QFtp, public QNetworkAccessCache::CacheableObject
{
 public:
   QNetworkAccessCachedFtpConnection() {
      setExpires(true);
      setShareable(false);
   }

   void dispose() override {
      connect(this, SIGNAL(done(bool)), this, SLOT(deleteLater()));
      close();
   }
};

QNetworkAccessFtpBackend::QNetworkAccessFtpBackend()
   : ftp(0), uploadDevice(0), totalBytes(0), helpId(-1), sizeId(-1), mdtmId(-1),
     supportsSize(false), supportsMdtm(false), state(Idle)
{
}

QNetworkAccessFtpBackend::~QNetworkAccessFtpBackend()
{
   //if backend destroyed while in use, then abort (this is the code path from QNetworkReply::abort)
   if (ftp && state != Disconnecting) {
      ftp->abort();
   }

   disconnectFromFtp();
}

void QNetworkAccessFtpBackend::open()
{
#ifndef QT_NO_NETWORKPROXY
   QNetworkProxy proxy;

   for (const QNetworkProxy &p : proxyList()) {
      // use the first FTP proxy
      // or no proxy at all
      if (p.type() == QNetworkProxy::FtpCachingProxy || p.type() == QNetworkProxy::NoProxy) {
         proxy = p;
         break;
      }
   }

   // did we find an FTP proxy or a NoProxy?
   if (proxy.type() == QNetworkProxy::DefaultProxy) {
      // unsuitable proxies
      error(QNetworkReply::ProxyNotFoundError, tr("No suitable proxy found"));
      finished();
      return;
   }

#endif

   QUrl url = this->url();

   if (url.path().isEmpty()) {
      url.setPath(QLatin1String("/"));
      setUrl(url);
   }

   if (url.path().endsWith('/')) {
      error(QNetworkReply::ContentOperationNotPermittedError, tr("Can not open %1: is a directory").formatArg(url.toString()));
      finished();
      return;
   }

   state = LoggingIn;

   QNetworkAccessCache *objectCache = QNetworkAccessManagerPrivate::getObjectCache(this);
   QByteArray cacheKey = makeCacheKey(url);

   if (! objectCache->requestEntry(cacheKey, this, SLOT(ftpConnectionReady(QNetworkAccessCache::CacheableObject *)))) {
      ftp = new QNetworkAccessCachedFtpConnection;

#ifndef QT_NO_BEARERMANAGEMENT
      // copy network session down to the QFtp
      ftp->setProperty("_q_networksession", property("_q_networksession"));
#endif

#ifndef QT_NO_NETWORKPROXY
      if (proxy.type() == QNetworkProxy::FtpCachingProxy) {
         ftp->setProxy(proxy.hostName(), proxy.port());
      }
#endif

      ftp->connectToHost(url.host(), url.port(DefaultFtpPort));
      ftp->login(url.userName(), url.password());

      objectCache->addEntry(cacheKey, ftp);
      ftpConnectionReady(ftp);
   }

   // Put operation
   if (operation() == QNetworkAccessManager::PutOperation) {
      uploadDevice = QNonContiguousByteDeviceFactory::wrap(createUploadByteDevice());
      uploadDevice->setParent(this);
   }
}

void QNetworkAccessFtpBackend::closeDownstreamChannel()
{
   state = Disconnecting;
   if (operation() == QNetworkAccessManager::GetOperation) {
      ftp->abort();
   }
}

void QNetworkAccessFtpBackend::downstreamReadyWrite()
{
   if (state == Transferring && ftp && ftp->bytesAvailable()) {
      ftpReadyRead();
   }
}

void QNetworkAccessFtpBackend::ftpConnectionReady(QNetworkAccessCache::CacheableObject *o)
{
   ftp = static_cast<QNetworkAccessCachedFtpConnection *>(o);
   connect(ftp, SIGNAL(done(bool)),  this, SLOT(ftpDone()));
   connect(ftp, SIGNAL(rawCommandReply(int, QString )), this, SLOT(ftpRawCommandReply(int, QString )));
   connect(ftp, SIGNAL(readyRead()), this, SLOT(ftpReadyRead()));

   // is the login process done already?
   if (ftp->state() == QFtp::LoggedIn) {
      ftpDone();
   }

   // no, defer the actual operation until after we've logged in
}

void QNetworkAccessFtpBackend::disconnectFromFtp(CacheCleanupMode mode)
{
   state = Disconnecting;

   if (ftp) {
      disconnect(ftp, 0, this, 0);

      QByteArray key = makeCacheKey(url());

      if (mode == RemoveCachedConnection) {
         QNetworkAccessManagerPrivate::getObjectCache(this)->removeEntry(key);
         ftp->dispose();
      } else {
         QNetworkAccessManagerPrivate::getObjectCache(this)->releaseEntry(key);
      }

      ftp = 0;
   }
}

void QNetworkAccessFtpBackend::ftpDone()
{
   // the last command we sent is done
   if (state == LoggingIn && ftp->state() != QFtp::LoggedIn) {
      if (ftp->state() == QFtp::Connected) {
         // the login did not succeed
         QUrl newUrl = url();

         QString userInfo = newUrl.userInfo();
         newUrl.setUserInfo(QString());
         setUrl(newUrl);

         QAuthenticator auth;
         authenticationRequired(&auth);

         if (!auth.isNull()) {
            // try again:
            newUrl.setUserName(auth.user());
            ftp->login(auth.user(), auth.password());
            return;
         }

         newUrl.setUserInfo(userInfo);
         setUrl(newUrl);
         error(QNetworkReply::AuthenticationRequiredError,
               tr("Logging in to %1 failed: authentication required").formatArg(url().host()));

      } else {
         // we did not connect
         QNetworkReply::NetworkError code;
         switch (ftp->error()) {
            case QFtp::HostNotFound:
               code = QNetworkReply::HostNotFoundError;
               break;

            case QFtp::ConnectionRefused:
               code = QNetworkReply::ConnectionRefusedError;
               break;

            default:
               code = QNetworkReply::ProtocolFailure;
               break;
         }

         error(code, ftp->errorString());
      }

      // we're not connected, so remove the cache entry:
      disconnectFromFtp(RemoveCachedConnection);
      finished();
      return;
   }

   // check for errors:
   if (ftp->error() != QFtp::NoError) {
      QString msg;

      if (operation() == QNetworkAccessManager::GetOperation) {
         msg = tr("Error while downloading %1: %2");
      } else {
         msg = tr("Error while uploading %1: %2");
      }

      msg = msg.formatArgs(url().toString(), ftp->errorString());

      if (state == Statting) {
         // file probably doesn't exist
         error(QNetworkReply::ContentNotFoundError,  msg);

      } else {
         error(QNetworkReply::ContentAccessDenied, msg);
      }

      disconnectFromFtp(RemoveCachedConnection);
      finished();
   }

   if (state == LoggingIn) {
      state = CheckingFeatures;
      if (operation() == QNetworkAccessManager::GetOperation) {
         // send help command to find out if server supports "SIZE" and "MDTM"
         QString command = url().path();
         command.prepend("%1 ");
         helpId = ftp->rawCommand(QLatin1String("HELP")); // get supported commands

      } else {
         ftpDone();
      }
   } else if (state == CheckingFeatures) {
      state = Statting;
      if (operation() == QNetworkAccessManager::GetOperation) {
         // logged in successfully, send the stat requests (if supported)
         QString command = url().path();
         command.prepend(QLatin1String("%1 "));
         if (supportsSize) {
            ftp->rawCommand(QLatin1String("TYPE I"));
            sizeId = ftp->rawCommand(command.formatArg(QLatin1String("SIZE"))); // get size
         }

         if (supportsMdtm) {
            mdtmId = ftp->rawCommand(command.formatArg(QLatin1String("MDTM")));   // get modified time
         }
         if (!supportsSize && !supportsMdtm) {
            ftpDone();   // no commands sent, move to the next state
         }

      } else {
         ftpDone();
      }
   } else if (state == Statting) {
      // statted successfully, send the actual request
      emit metaDataChanged();
      state = Transferring;

      QFtp::TransferType type = QFtp::Binary;
      if (operation() == QNetworkAccessManager::GetOperation) {
         setCachingEnabled(true);
         ftp->get(url().path(), 0, type);
      } else {
         ftp->put(uploadDevice, url().path(), type);
      }

   } else if (state == Transferring) {
      // upload or download finished
      disconnectFromFtp();
      finished();
   }
}

void QNetworkAccessFtpBackend::ftpReadyRead()
{
   QByteArray data = ftp->readAll();
   QByteDataBuffer list;
   list.append(data);
   data.clear(); // important because of implicit sharing!
   writeDownstreamData(list);
}

void QNetworkAccessFtpBackend::ftpRawCommandReply(int code, const QString &text)
{
   //qDebug() << "FTP reply:" << code << text;
   int id = ftp->currentId();

   if ((id == helpId) && ((code == 200) || (code == 214))) {     // supported commands
      // the "FEAT" ftp command would be nice here, but it is not part of the
      // initial FTP RFC 959, neither ar "SIZE" nor "MDTM" (they are all specified
      // in RFC 3659)
      if (text.contains(QLatin1String("SIZE"), Qt::CaseSensitive)) {
         supportsSize = true;
      }

      if (text.contains(QLatin1String("MDTM"), Qt::CaseSensitive)) {
         supportsMdtm = true;
      }

   } else if (code == 213) {          // file status
      if (id == sizeId) {
         // reply to the size command
         setHeader(QNetworkRequest::ContentLengthHeader, text.toInteger<qint64>());

#ifndef QT_NO_DATESTRING
      } else if (id == mdtmId) {
         QDateTime dt = QDateTime::fromString(text, QLatin1String("yyyyMMddHHmmss"));
         setHeader(QNetworkRequest::LastModifiedHeader, dt);
#endif
      }
   }
}

#endif // QT_NO_FTP
