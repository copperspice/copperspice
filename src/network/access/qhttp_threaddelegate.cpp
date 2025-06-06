/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qhttp_threaddelegate_p.h>

#include <qauthenticator.h>
#include <qeventloop.h>
#include <qthread.h>
#include <qtimer.h>

#include <qhttp_networkreply_p.h>
#include <qnetaccess_cache_p.h>
#include <qnoncontiguousbytedevice_p.h>

static QNetworkReply::NetworkError statusCodeFromHttp(int httpStatusCode, const QUrl &url)
{
   QNetworkReply::NetworkError code;
   // we have an error
   switch (httpStatusCode) {
      case 400:               // Bad Request
         code = QNetworkReply::ProtocolInvalidOperationError;
         break;

      case 401:               // Authorization required
         code = QNetworkReply::AuthenticationRequiredError;
         break;

      case 403:               // Access denied
         code = QNetworkReply::ContentOperationNotPermittedError;
         break;

      case 404:               // Not Found
         code = QNetworkReply::ContentNotFoundError;
         break;

      case 405:               // Method Not Allowed
         code = QNetworkReply::ContentOperationNotPermittedError;
         break;

      case 407:
         code = QNetworkReply::ProxyAuthenticationRequiredError;
         break;

      case 409:               // Resource Conflict
         code = QNetworkReply::ContentConflictError;
         break;

      case 410:               // Content no longer available
         code = QNetworkReply::ContentGoneError;
         break;

      case 418:               // I'm a teapot
         code = QNetworkReply::ProtocolInvalidOperationError;
         break;

      case 500:               // Internal Server Error
         code = QNetworkReply::InternalServerError;
         break;

      case 501:               // Server does not support this functionality
         code = QNetworkReply::OperationNotImplementedError;
         break;

      case 503:               // Service unavailable
         code = QNetworkReply::ServiceUnavailableError;
         break;

      default:
         if (httpStatusCode > 500) {
            // some kind of server error
            code = QNetworkReply::UnknownServerError;

         } else if (httpStatusCode >= 400) {
            // content error we did not handle above
            code = QNetworkReply::UnknownContentError;

         } else {
            qWarning("QNetworkReply::statusCodeFromHttp() HTTP status code %d was not expected from url %s",
                     httpStatusCode, csPrintable(url.toString()));

            code = QNetworkReply::ProtocolFailure;
         }
   }

   return code;
}

static QByteArray makeCacheKey(QUrl &url, QNetworkProxy *proxy)
{
   QString result;

   QUrl copy = url;
   QString scheme = copy.scheme();

   bool isEncrypted = (scheme == "https");

   copy.setPort(copy.port(isEncrypted ? 443 : 80));

   if (scheme == "preconnect-http") {
      copy.setScheme("http");

   } else if (scheme == "preconnect-https") {
      copy.setScheme("https");
   }

   result = copy.toString(QUrl::RemoveUserInfo | QUrl::RemovePath |
                          QUrl::RemoveQuery | QUrl::RemoveFragment | QUrl::FullyEncoded);

#ifndef QT_NO_NETWORKPROXY
   if (proxy && proxy->type() != QNetworkProxy::NoProxy) {
      QUrl key;

      switch (proxy->type()) {
         case QNetworkProxy::Socks5Proxy:
            key.setScheme("proxy-socks5");
            break;

         case QNetworkProxy::HttpProxy:
         case QNetworkProxy::HttpCachingProxy:
            key.setScheme("proxy-http");
            break;

         default:
            break;
      }

      if (!key.scheme().isEmpty()) {
         key.setUserName(proxy->user());
         key.setHost(proxy->hostName());
         key.setPort(proxy->port());

         key.setQuery(result);
         result = key.toString(QUrl::FullyEncoded);
      }
   }

#else
   (void) proxy;
#endif

   return "http-connection:" + result.toLatin1();
}

class QNetworkAccessCachedHttpConnection: public QHttpNetworkConnection, public QNetworkAccessCache::CacheableObject
{
 public:

#ifdef QT_NO_BEARERMANAGEMENT
   QNetworkAccessCachedHttpConnection(const QString &hostName, quint16 port, bool encrypt,
                  QHttpNetworkConnection::ConnectionType connectionType)
      : QHttpNetworkConnection(hostName, port, encrypt, connectionType)
#else
   QNetworkAccessCachedHttpConnection(const QString & hostName, quint16 port, bool encrypt,
                  QHttpNetworkConnection::ConnectionType connectionType,
                  QSharedPointer<QNetworkSession> networkSession)

      : QHttpNetworkConnection(hostName, port, encrypt, connectionType, nullptr, std::move(networkSession))
#endif

   {
      setExpires(true);
      setShareable(true);
   }

   void dispose() override {
      delete this;
   }
};

QThreadStorage<QNetworkAccessCache *> QHttpThreadDelegate::connections;

QHttpThreadDelegate::QHttpThreadDelegate(QObject *parent)
   : QObject(parent), ssl(false), downloadBufferMaximumSize(0), readBufferMaxSize(0),
     bytesEmitted(0), pendingDownloadData(nullptr), pendingDownloadProgress(nullptr), synchronous(false),
     incomingStatusCode(0), isPipeliningUsed(false), isSpdyUsed(false), incomingContentLength(-1),
     incomingErrorCode(QNetworkReply::NoError), downloadBuffer(nullptr), httpConnection(nullptr),
     httpReply(nullptr), synchronousRequestLoop(nullptr)
{
}

QHttpThreadDelegate::~QHttpThreadDelegate()
{
   // It could be that the main thread has asked us to shut down, so we need to delete the HTTP reply
   if (httpReply) {
      delete httpReply;
   }

   // Get the object cache that stores our QHttpNetworkConnection objects
   // and release the entry for this QHttpNetworkConnection
   if (connections.hasLocalData() && !cacheKey.isEmpty()) {
      connections.localData()->releaseEntry(cacheKey);
   }
}

// This is invoked as BlockingQueuedConnection from QNetworkAccessHttpBackend in the user thread
void QHttpThreadDelegate::startRequestSynchronously()
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QHttpThreadDelegate::startRequestSynchronously() thread=" << QThread::currentThreadId();
#endif

   synchronous = true;

   QEventLoop eventLoop;
   this->synchronousRequestLoop = &eventLoop;

   // Worst case timeout
   QTimer::singleShot(30 * 1000, this, SLOT(abortRequest()));

   QMetaObject::invokeMethod(this, "startRequest", Qt::QueuedConnection);
   eventLoop.exec();

   connections.localData()->releaseEntry(cacheKey);
   connections.setLocalData(nullptr);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QHttpThreadDelegate::startRequestSynchronously() thread=" << QThread::currentThreadId() << "finished";
#endif
}

// invoked as QueuedConnection from QNetworkAccessHttpBackend in the user thread
void QHttpThreadDelegate::startRequest()
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QHttpThreadDelegate::startRequest() thread=" << QThread::currentThreadId();
#endif

   // Check QThreadStorage for the QNetworkAccessCache
   // If not there, create this connection cache
   if (! connections.hasLocalData()) {
      connections.setLocalData(new QNetworkAccessCache());
   }

   // check if we have an open connection to this host
   QUrl urlCopy = httpRequest.url();
   urlCopy.setPort(urlCopy.port(ssl ? 443 : 80));

   QHttpNetworkConnection::ConnectionType connectionType = QHttpNetworkConnection::ConnectionTypeHTTP;

#ifdef QT_SSL
   if (httpRequest.isSPDYAllowed() && ssl) {
      connectionType = QHttpNetworkConnection::ConnectionTypeSPDY;
      urlCopy.setScheme("spdy");             // to differentiate SPDY requests from HTTPS requests
      QList<QByteArray> nextProtocols;

      nextProtocols << QSslConfiguration::NextProtocolSpdy3_0
                    << QSslConfiguration::NextProtocolHttp1_1;

      incomingSslConfiguration.setAllowedNextProtocols(nextProtocols);
   }
#endif

#ifndef QT_NO_NETWORKPROXY
   if (transparentProxy.type() != QNetworkProxy::NoProxy) {
      cacheKey = makeCacheKey(urlCopy, &transparentProxy);

   } else if (cacheProxy.type() != QNetworkProxy::NoProxy) {
      cacheKey = makeCacheKey(urlCopy, &cacheProxy);

   } else
#endif

      cacheKey = makeCacheKey(urlCopy, nullptr);

   // the http object is actually a QHttpNetworkConnection
   httpConnection = static_cast<QNetworkAccessCachedHttpConnection *>(connections.localData()->requestEntryNow(cacheKey));

   if (httpConnection == nullptr) {
      // no entry in cache; create an object
      // the http object is actually a QHttpNetworkConnection

#ifdef QT_NO_BEARERMANAGEMENT
      httpConnection = new QNetworkAccessCachedHttpConnection(urlCopy.host(), urlCopy.port(), ssl, connectionType);
#else
      httpConnection = new QNetworkAccessCachedHttpConnection(urlCopy.host(),
            urlCopy.port(), ssl, connectionType, networkSession);
#endif

#ifdef QT_SSL
      // Set the QSslConfiguration from this QNetworkRequest.
      if (ssl && incomingSslConfiguration != QSslConfiguration::defaultConfiguration()) {
         httpConnection->setSslConfiguration(incomingSslConfiguration);
      }
#endif

#ifndef QT_NO_NETWORKPROXY
      httpConnection->setTransparentProxy(transparentProxy);
      httpConnection->setCacheProxy(cacheProxy);
#endif

      // cache the QHttpNetworkConnection corresponding to this cache key
      connections.localData()->addEntry(cacheKey, httpConnection);

   } else {
      if (httpRequest.withCredentials()) {
         QNetworkAuthenticationCredential credential = authenticationManager->fetchCachedCredentials(httpRequest.url(), nullptr);

         if (!credential.user.isEmpty() && !credential.password.isEmpty()) {
            QAuthenticator auth;
            auth.setUser(credential.user);
            auth.setPassword(credential.password);
            httpConnection->d_func()->copyCredentials(-1, &auth, false);
         }
      }
   }

   // Send the request to the connection
   httpReply = httpConnection->sendRequest(httpRequest);
   httpReply->setParent(this);

   // Connect the reply signals that we need to handle and then forward
   if (synchronous) {
      connect(httpReply, &QHttpNetworkReply::headerChanged,          this, &QHttpThreadDelegate::synchronousHeaderChangedSlot);
      connect(httpReply, &QHttpNetworkReply::finished,               this, &QHttpThreadDelegate::synchronousFinishedSlot);
      connect(httpReply, &QHttpNetworkReply::finishedWithError,      this, &QHttpThreadDelegate::synchronousFinishedWithErrorSlot);
      connect(httpReply, &QHttpNetworkReply::authenticationRequired, this, &QHttpThreadDelegate::synchronousAuthenticationRequiredSlot);

#ifndef QT_NO_NETWORKPROXY
      connect(httpReply, &QHttpNetworkReply::proxyAuthenticationRequired, this,
            &QHttpThreadDelegate::synchronousProxyAuthenticationRequiredSlot);
#endif

      // ignore SSL errors for now in the synchronous HTTP case.

   } else if (! synchronous) {
      connect(httpReply, &QHttpNetworkReply::headerChanged,     this, &QHttpThreadDelegate::headerChangedSlot);
      connect(httpReply, &QHttpNetworkReply::finished,          this, &QHttpThreadDelegate::finishedSlot);
      connect(httpReply, &QHttpNetworkReply::finishedWithError, this, &QHttpThreadDelegate::finishedWithErrorSlot);

      // some signals are only interesting when normal asynchronous style is used
      connect(httpReply, &QHttpNetworkReply::readyRead,         this, &QHttpThreadDelegate::readyReadSlot);
      connect(httpReply, &QHttpNetworkReply::dataReadProgress,  this, &QHttpThreadDelegate::dataReadProgressSlot);

#ifdef QT_SSL
      connect(httpReply, &QHttpNetworkReply::encrypted, this, &QHttpThreadDelegate::encryptedSlot);
      connect(httpReply, &QHttpNetworkReply::sslErrors, this, &QHttpThreadDelegate::sslErrorsSlot);

      connect(httpReply, &QHttpNetworkReply::preSharedKeyAuthenticationRequired,
            this, &QHttpThreadDelegate::preSharedKeyAuthenticationRequiredSlot);

#endif
      // In the asynchronous HTTP case we can just forward those signals
      // Connect the reply signals that we can directly forward
      connect(httpReply, &QHttpNetworkReply::authenticationRequired, this, &QHttpThreadDelegate::authenticationRequired);

#ifndef QT_NO_NETWORKPROXY
      connect(httpReply, &QHttpNetworkReply::proxyAuthenticationRequired, this, &QHttpThreadDelegate::proxyAuthenticationRequired);
#endif
   }

   connect(httpReply, &QHttpNetworkReply::cacheCredentials, this, &QHttpThreadDelegate::cacheCredentialsSlot);
}

// This gets called from the user thread or by the synchronous HTTP timeout timer
void QHttpThreadDelegate::abortRequest()
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QHttpThreadDelegate::abortRequest() thread=" << QThread::currentThreadId() << "sync=" << synchronous;
#endif

   if (httpReply) {
      httpReply->abort();
      delete httpReply;
      httpReply = nullptr;
   }

   // Got aborted by the timeout timer
   if (synchronous) {
      incomingErrorCode = QNetworkReply::TimeoutError;
      QMetaObject::invokeMethod(synchronousRequestLoop, "quit", Qt::QueuedConnection);
   } else {
      //only delete this for asynchronous mode or QNetworkAccessHttpBackend will crash - see QNetworkAccessHttpBackend::postRequest()
      this->deleteLater();
   }
}

void QHttpThreadDelegate::readBufferSizeChanged(qint64 size)
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QHttpThreadDelegate::readBufferSizeChanged() size " << size;
#endif
   if (httpReply) {
      httpReply->setDownstreamLimited(size > 0);
      httpReply->setReadBufferSize(size);
      readBufferMaxSize = size;
   }
}

void QHttpThreadDelegate::readBufferFreed(qint64 size)
{
   if (readBufferMaxSize) {
      bytesEmitted -= size;

      QMetaObject::invokeMethod(this, "readyReadSlot", Qt::QueuedConnection);
   }
}

void QHttpThreadDelegate::readyReadSlot()
{
   if (! httpReply) {
      return;
   }

   // Don't do in zerocopy case
   if (! downloadBuffer.isNull()) {
      return;
   }

   if (readBufferMaxSize) {
      if (bytesEmitted < readBufferMaxSize) {
         qint64 sizeEmitted = 0;

         while (httpReply->readAnyAvailable() && (sizeEmitted < (readBufferMaxSize - bytesEmitted))) {
            if (httpReply->sizeNextBlock() > (readBufferMaxSize - bytesEmitted)) {
               sizeEmitted = readBufferMaxSize - bytesEmitted;
               bytesEmitted += sizeEmitted;
               pendingDownloadData->fetchAndAddRelease(1);
               emit downloadData(httpReply->read(sizeEmitted));
            } else {
               sizeEmitted = httpReply->sizeNextBlock();
               bytesEmitted += sizeEmitted;
               pendingDownloadData->fetchAndAddRelease(1);
               emit downloadData(httpReply->readAny());
            }
         }
      } else {
         // We need to wait until we empty data from the read buffer in the reply.
      }

   } else {
      while (httpReply->readAnyAvailable()) {
         pendingDownloadData->fetchAndAddRelease(1);
         emit downloadData(httpReply->readAny());
      }
   }
}

void QHttpThreadDelegate::finishedSlot()
{
   if (! httpReply) {
      return;
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QHttpThreadDelegate::finishedSlot() thread=" << QThread::currentThreadId() << "result=" <<
            httpReply->statusCode();
#endif

   // If there is still some data left emit that now
   while (httpReply->readAnyAvailable()) {
      pendingDownloadData->fetchAndAddRelease(1);
      emit downloadData(httpReply->readAny());
   }

#ifdef QT_SSL
   if (ssl) {
      emit sslConfigurationChanged(httpReply->sslConfiguration());
   }
#endif

   if (httpReply->statusCode() >= 400) {
      // an error reply
      QString msg = QString::fromLatin1(cs_mark_tr("QNetworkReply", "Error transferring %1 - server replied: %2"))
                  .formatArgs(httpRequest.url().toString(), httpReply->reasonPhrase());

      emit error(statusCodeFromHttp(httpReply->statusCode(), httpRequest.url()), msg);
   }

   if (httpRequest.isFollowRedirects() && httpReply->isRedirecting()) {
      emit redirected(httpReply->redirectUrl(), httpReply->statusCode(), httpReply->request().redirectCount() - 1);
   }

   emit downloadFinished();

   QMetaObject::invokeMethod(httpReply, "deleteLater", Qt::QueuedConnection);
   QMetaObject::invokeMethod(this, "deleteLater", Qt::QueuedConnection);
   httpReply = nullptr;
}

void QHttpThreadDelegate::synchronousFinishedSlot()
{
   if (!httpReply) {
      return;
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QHttpThreadDelegate::synchronousFinishedSlot() thread=" << QThread::currentThreadId() << "result=" << httpReply->statusCode();
#endif

   if (httpReply->statusCode() >= 400) {
      // error reply
      QString msg = QString::fromLatin1(cs_mark_tr("QNetworkReply", "Error transferring %1 - server replied: %2"));

      incomingErrorDetail = msg.formatArgs(httpRequest.url().toString(), httpReply->reasonPhrase());
      incomingErrorCode = statusCodeFromHttp(httpReply->statusCode(), httpRequest.url());
   }

   synchronousDownloadData = httpReply->readAll();

   QMetaObject::invokeMethod(httpReply, "deleteLater", Qt::QueuedConnection);
   QMetaObject::invokeMethod(synchronousRequestLoop, "quit", Qt::QueuedConnection);
   httpReply = nullptr;
}

void QHttpThreadDelegate::finishedWithErrorSlot(QNetworkReply::NetworkError errorCode, const QString &detail)
{
   if (!httpReply) {
      return;
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QHttpThreadDelegate::finishedWithErrorSlot() thread=" << QThread::currentThreadId() << "error="
            << errorCode << detail;
#endif

#ifdef QT_SSL
   if (ssl) {
      emit sslConfigurationChanged(httpReply->sslConfiguration());
   }
#endif

   emit error(errorCode, detail);
   emit downloadFinished();

   QMetaObject::invokeMethod(httpReply, "deleteLater", Qt::QueuedConnection);
   QMetaObject::invokeMethod(this, "deleteLater", Qt::QueuedConnection);
   httpReply = nullptr;
}


void QHttpThreadDelegate::synchronousFinishedWithErrorSlot(QNetworkReply::NetworkError errorCode, const QString &detail)
{
   if (!httpReply) {
      return;
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QHttpThreadDelegate::synchronousFinishedWithErrorSlot() thread=" << QThread::currentThreadId() << "error="
            << errorCode << detail;
#endif

   incomingErrorCode = errorCode;
   incomingErrorDetail = detail;
   synchronousDownloadData = httpReply->readAll();

   QMetaObject::invokeMethod(httpReply, "deleteLater", Qt::QueuedConnection);
   QMetaObject::invokeMethod(synchronousRequestLoop, "quit", Qt::QueuedConnection);
   httpReply = nullptr;
}

static void downloadBufferDeleter(char *ptr)
{
   delete[] ptr;
}

void QHttpThreadDelegate::headerChangedSlot()
{
   if (! httpReply) {
      return;
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QHttpThreadDelegate::headerChangedSlot() thread=" << QThread::currentThreadId();
#endif

#ifdef QT_SSL
   if (ssl) {
      emit sslConfigurationChanged(httpReply->sslConfiguration());
   }
#endif

   // Is using a zerocopy buffer allowed by user and possible with this reply?
   if (httpReply->supportsUserProvidedDownloadBuffer()
         && (downloadBufferMaximumSize > 0) && (httpReply->contentLength() <= downloadBufferMaximumSize)) {

      try {
         char *buf = new char[httpReply->contentLength()]; // throws if allocation fails

         if (buf) {
            downloadBuffer = QSharedPointer<char>(buf, downloadBufferDeleter);
            httpReply->setUserProvidedDownloadBuffer(buf);
         }

      } catch (const std::bad_alloc &) {
         // out of memory
      }
   }

   // We fetch this into our own
   incomingHeaders = httpReply->header();
   incomingStatusCode = httpReply->statusCode();
   incomingReasonPhrase = httpReply->reasonPhrase();
   isPipeliningUsed = httpReply->isPipeliningUsed();
   incomingContentLength = httpReply->contentLength();
   isSpdyUsed = httpReply->isSpdyUsed();

   emit downloadMetaData(incomingHeaders, incomingStatusCode, incomingReasonPhrase, isPipeliningUsed,
               downloadBuffer, incomingContentLength, isSpdyUsed);
}

void QHttpThreadDelegate::synchronousHeaderChangedSlot()
{
   if (!httpReply) {
      return;
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug() << "QHttpThreadDelegate::synchronousHeaderChangedSlot() thread=" << QThread::currentThreadId();
#endif

   // Store the information we need in this object, the QNetworkAccessHttpBackend will later read it
   incomingHeaders = httpReply->header();
   incomingStatusCode = httpReply->statusCode();
   incomingReasonPhrase = httpReply->reasonPhrase();
   isPipeliningUsed = httpReply->isPipeliningUsed();
   isSpdyUsed = httpReply->isSpdyUsed();
   incomingContentLength = httpReply->contentLength();
}

void QHttpThreadDelegate::dataReadProgressSlot(qint64 done, qint64 total)
{
   // If we don't have a download buffer don't attempt to go this codepath
   // It is not used by QNetworkAccessHttpBackend
   if (downloadBuffer.isNull()) {
      return;
   }

   pendingDownloadProgress->fetchAndAddRelease(1);
   emit downloadProgress(done, total);
}

void QHttpThreadDelegate::cacheCredentialsSlot(const QHttpNetworkRequest &request, QAuthenticator *authenticator)
{
   authenticationManager->cacheCredentials(request.url(), authenticator);
}

#ifdef QT_SSL
void QHttpThreadDelegate::encryptedSlot()
{
   if (! httpReply) {
      return;
   }

   emit sslConfigurationChanged(httpReply->sslConfiguration());
   emit encrypted();
}

void QHttpThreadDelegate::sslErrorsSlot(const QList<QSslError> &errorList)
{
   if (! httpReply) {
      return;
   }
   emit sslConfigurationChanged(httpReply->sslConfiguration());

   bool ignoreAll = false;
   QList<QSslError> toBeIgnored;
   emit sslErrors(errorList, &ignoreAll, &toBeIgnored);

   if (ignoreAll) {
      httpReply->ignoreSslErrors();
   }

   if (! toBeIgnored.isEmpty()) {
      httpReply->ignoreSslErrors(toBeIgnored);
   }
}

void QHttpThreadDelegate::preSharedKeyAuthenticationRequiredSlot(QSslPreSharedKeyAuthenticator *authenticator)
{
   if (! httpReply) {
      return;
   }
   emit preSharedKeyAuthenticationRequired(authenticator);
}
#endif

void QHttpThreadDelegate::synchronousAuthenticationRequiredSlot(const QHttpNetworkRequest &request, QAuthenticator *a)
{
   (void) request;

   if (! httpReply) {
      return;
   }

   // Ask the credential cache
   QNetworkAuthenticationCredential credential = authenticationManager->fetchCachedCredentials(httpRequest.url(), a);

   if (! credential.isNull()) {
      a->setUser(credential.user);
      a->setPassword(credential.password);
   }

   // Disconnect this connection now since we only want to ask the authentication cache once.
   QObject::disconnect(httpReply, &QHttpNetworkReply::authenticationRequired, this,
            &QHttpThreadDelegate::synchronousAuthenticationRequiredSlot);
}

#ifndef QT_NO_NETWORKPROXY
void QHttpThreadDelegate::synchronousProxyAuthenticationRequiredSlot(const QNetworkProxy &proxy, QAuthenticator *authenticator)
{
   if (! httpReply) {
      return;
   }

   // Ask the credential cache
   QNetworkAuthenticationCredential credential = authenticationManager->fetchCachedProxyCredentials(proxy, authenticator);

   if (! credential.isNull()) {
      authenticator->setUser(credential.user);
      authenticator->setPassword(credential.password);
   }

   // Disconnect this connection now since we only want to ask the authentication cache once.
   QObject::disconnect(httpReply, &QHttpNetworkReply::proxyAuthenticationRequired, this,
         &QHttpThreadDelegate::synchronousProxyAuthenticationRequiredSlot);
}
#endif

void QNonContiguousByteDeviceThreadForwardImpl::haveDataSlot(qint64 pos, const QByteArray &dataArray, bool dataAtEnd, qint64 dataSize)
{
   if (pos != m_pos) {
      // Sometimes when re-sending a request in the qhttpnetwork* layer there is a pending haveData from the
      // user thread on the way to us. We need to ignore it since it is the data for the wrong(later) chunk.
      return;
   }

   wantDataPending = false;

   m_dataArray = dataArray;
   m_data = const_cast<char *>(m_dataArray.constData());
   m_amount = dataArray.size();

   m_atEnd = dataAtEnd;
   m_size = dataSize;

   // This will tell the HTTP code (QHttpNetworkConnectionChannel) that we have data available now
   emit readyRead();
}
