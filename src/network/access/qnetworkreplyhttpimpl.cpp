/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qnetworkreplyhttpimpl_p.h"
#include "qnetworkaccessmanager_p.h"
#include "qnetworkaccesscache_p.h"
#include "qabstractnetworkcache.h"
#include "qnetworkrequest.h"
#include "qnetworkreply.h"
#include "qnetworkrequest_p.h"
#include "qnetworkcookie.h"
#include "qnetworkcookie_p.h"
#include "qsslconfiguration.h"
#include "qhttpthreaddelegate_p.h"
#include "qnetworkcookiejar.h"

#include "qcoreapplication.h"
#include "qdatetime.h"
#include "qelapsedtimer.h"
#include "qthread.h"

#include <qthread_p.h>
#include <string.h>             // for strchr

class QNetworkProxy;

static inline bool isSeparator(char c)
{
   static const char separators[] = "()<>@,;:\\\"/[]?={}";
   return isLWS(c) || strchr(separators, c) != 0;
}

// ### merge with nextField in cookiejar.cpp
static QHash<QByteArray, QByteArray> parseHttpOptionHeader(const QByteArray &header)
{
   // The HTTP header is of the form:
   // header          = #1(directives)
   // directives      = token | value-directive
   // value-directive = token "=" (token | quoted-string)
   QHash<QByteArray, QByteArray> result;

   int pos = 0;
   while (true) {
      // skip spaces
      pos = nextNonWhitespace(header, pos);

      if (pos == header.length()) {
         return result;   // end of parsing
      }

      // pos points to a non-whitespace
      int comma = header.indexOf(',', pos);
      int equal = header.indexOf('=', pos);
      if (comma == pos || equal == pos)
         // huh? Broken header.
      {
         return result;
      }

      // The key name is delimited by either a comma, an equal sign or the end
      // of the header, whichever comes first
      int end = comma;
      if (end == -1) {
         end = header.length();
      }
      if (equal != -1 && end > equal) {
         end = equal;   // equal sign comes before comma/end
      }
      QByteArray key = QByteArray(header.constData() + pos, end - pos).trimmed().toLower();
      pos = end + 1;

      if (uint(equal) < uint(comma)) {
         // case: token "=" (token | quoted-string)
         // skip spaces
         pos = nextNonWhitespace(header, pos);
         if (pos == header.length())
            // huh? Broken header
         {
            return result;
         }

         QByteArray value;
         value.reserve(header.length() - pos);
         if (header.at(pos) == '"') {
            // case: quoted-string
            // quoted-string  = ( <"> *(qdtext | quoted-pair ) <"> )
            // qdtext         = <any TEXT except <">>
            // quoted-pair    = "\" CHAR
            ++pos;
            while (pos < header.length()) {
               char c = header.at(pos);
               if (c == '"') {
                  // end of quoted text
                  break;
               } else if (c == '\\') {
                  ++pos;
                  if (pos >= header.length())
                     // broken header
                  {
                     return result;
                  }
                  c = header.at(pos);
               }

               value += c;
               ++pos;
            }
         } else {
            // case: token
            while (pos < header.length()) {
               char c = header.at(pos);
               if (isSeparator(c)) {
                  break;
               }
               value += c;
               ++pos;
            }
         }

         result.insert(key, value);

         // find the comma now:
         comma = header.indexOf(',', pos);
         if (comma == -1) {
            return result;   // end of parsing
         }
         pos = comma + 1;
      } else {
         // case: token
         // key is already set
         result.insert(key, QByteArray());
      }
   }
}

QNetworkReplyHttpImpl::QNetworkReplyHttpImpl(QNetworkAccessManager *const manager,
      const QNetworkRequest &request,
      QNetworkAccessManager::Operation &operation,
      QIODevice *outgoingData)
   : QNetworkReply(*new QNetworkReplyHttpImplPrivate, manager)
{
   Q_D(QNetworkReplyHttpImpl);
   d->manager = manager;
   d->managerPrivate = manager->d_func();
   d->request = request;
   d->originalRequest = request;
   d->operation = operation;
   d->outgoingData = outgoingData;
   d->url = request.url();

#ifdef QT_SSL
   d->sslConfiguration = request.sslConfiguration();
#endif

   // FIXME Later maybe set to Unbuffered, especially if it is zerocopy or from cache?
   QIODevice::open(QIODevice::ReadOnly);


   // Internal code that does a HTTP reply for the synchronous Ajax in WebKit.
   QVariant synchronousHttpAttribute = request.attribute(
                                          static_cast<QNetworkRequest::Attribute>(QNetworkRequest::SynchronousRequestAttribute));
   if (synchronousHttpAttribute.isValid()) {
      d->synchronous = synchronousHttpAttribute.toBool();
      if (d->synchronous && outgoingData) {
         // The synchronous HTTP is a corner case, we will put all upload data in one big QByteArray in the outgoingDataBuffer.
         // Yes, this is not the most efficient thing to do, but on the other hand synchronous XHR needs to die anyway.
         d->outgoingDataBuffer = QSharedPointer<QRingBuffer>::create();
         qint64 previousDataSize = 0;
         do {
            previousDataSize = d->outgoingDataBuffer->size();
            d->outgoingDataBuffer->append(d->outgoingData->readAll());
         } while (d->outgoingDataBuffer->size() != previousDataSize);
         d->_q_startOperation();
         return;
      }
   }


   if (outgoingData) {
      // there is data to be uploaded, e.g. HTTP POST.

      if (!d->outgoingData->isSequential()) {
         // fixed size non-sequential (random-access)
         // just start the operation
         QMetaObject::invokeMethod(this, "_q_startOperation", Qt::QueuedConnection);
         // FIXME make direct call?
      } else {
         bool bufferingDisallowed =
            request.attribute(QNetworkRequest::DoNotBufferUploadDataAttribute,
                              false).toBool();

         if (bufferingDisallowed) {
            // if a valid content-length header for the request was supplied, we can disable buffering
            // if not, we will buffer anyway
            if (request.header(QNetworkRequest::ContentLengthHeader).isValid()) {
               QMetaObject::invokeMethod(this, "_q_startOperation", Qt::QueuedConnection);
               // FIXME make direct call?
            } else {
               d->state = d->Buffering;
               QMetaObject::invokeMethod(this, "_q_bufferOutgoingData", Qt::QueuedConnection);
            }
         } else {
            // _q_startOperation will be called when the buffering has finished.
            d->state = d->Buffering;
            QMetaObject::invokeMethod(this, "_q_bufferOutgoingData", Qt::QueuedConnection);
         }
      }
   } else {
      // No outgoing data (POST, ..)
      d->_q_startOperation();
   }
}

QNetworkReplyHttpImpl::~QNetworkReplyHttpImpl()
{
   // This will do nothing if the request was already finished or aborted
   emit abortHttpRequest();
}

void QNetworkReplyHttpImpl::close()
{
   Q_D(QNetworkReplyHttpImpl);

   if (d->state == QNetworkReplyPrivate::Aborted ||
         d->state == QNetworkReplyPrivate::Finished) {
      return;
   }

   // According to the documentation close only stops the download
   // by closing we can ignore the download part and continue uploading.
   QNetworkReply::close();

   // call finished which will emit signals
   // FIXME shouldn't this be emitted Queued?
   d->error(OperationCanceledError, tr("Operation canceled"));
   d->finished();
}

void QNetworkReplyHttpImpl::abort()
{
   Q_D(QNetworkReplyHttpImpl);
   // FIXME
   if (d->state == QNetworkReplyPrivate::Finished || d->state == QNetworkReplyPrivate::Aborted) {
      return;
   }

   QNetworkReply::close();

   if (d->state != QNetworkReplyPrivate::Finished) {
      // call finished which will emit signals
      // FIXME shouldn't this be emitted Queued?
      d->error(OperationCanceledError, tr("Operation canceled"));

      // If state is WaitingForSession, calling finished has no effect
      if (d->state == QNetworkReplyPrivate::WaitingForSession) {
         d->state = QNetworkReplyPrivate::Working;
      }
      d->finished();
   }

   d->state = QNetworkReplyPrivate::Aborted;

   emit abortHttpRequest();
}

qint64 QNetworkReplyHttpImpl::bytesAvailable() const
{
   Q_D(const QNetworkReplyHttpImpl);

   // if we load from cache device
   if (d->cacheLoadDevice) {
      return QNetworkReply::bytesAvailable() + d->cacheLoadDevice->bytesAvailable() + d->downloadMultiBuffer.byteAmount();
   }

   // zerocopy buffer
   if (d->downloadZerocopyBuffer) {
      return QNetworkReply::bytesAvailable() + d->downloadBufferCurrentSize - d->downloadBufferReadPosition;
   }

   // normal buffer
   return QNetworkReply::bytesAvailable() + d->downloadMultiBuffer.byteAmount();
}

bool QNetworkReplyHttpImpl::isSequential () const
{
   // FIXME In the cache of a cached load or the zero-copy buffer we could actually be non-sequential.
   // FIXME however this requires us to implement stuff like seek() too.
   return true;
}

qint64 QNetworkReplyHttpImpl::size() const
{
   // FIXME At some point, this could return a proper value, e.g. if we're non-sequential.
   return QNetworkReply::size();
}

qint64 QNetworkReplyHttpImpl::readData(char *data, qint64 maxlen)
{
   Q_D(QNetworkReplyHttpImpl);

   // cacheload device
   if (d->cacheLoadDevice) {
      // FIXME bytesdownloaded, position etc?

      // There is something already in the buffer we buffered before because the user did not read()
      // anything, so we read there first:
      if (!d->downloadMultiBuffer.isEmpty()) {
         return d->downloadMultiBuffer.read(data, maxlen);
      }

      qint64 ret = d->cacheLoadDevice->read(data, maxlen);
      return ret;
   }

   // zerocopy buffer
   if (d->downloadZerocopyBuffer) {
      // FIXME bytesdownloaded, position etc?

      qint64 howMuch = qMin(maxlen, (d->downloadBufferCurrentSize - d->downloadBufferReadPosition));
      memcpy(data, d->downloadZerocopyBuffer + d->downloadBufferReadPosition, howMuch);
      d->downloadBufferReadPosition += howMuch;
      return howMuch;

   }

   // normal buffer
   if (d->downloadMultiBuffer.isEmpty()) {
      if (d->state == d->Finished || d->state == d->Aborted) {
         return -1;
      }
      return 0;
   }

   if (maxlen == 1) {
      // optimization for getChar()
      *data = d->downloadMultiBuffer.getChar();
      if (readBufferSize()) {
         emit readBufferFreed(1);
      }
      return 1;
   }

   maxlen = qMin<qint64>(maxlen, d->downloadMultiBuffer.byteAmount());
   qint64 bytesRead = d->downloadMultiBuffer.read(data, maxlen);
   if (readBufferSize()) {
      emit readBufferFreed(bytesRead);
   }
   return bytesRead;
}

void QNetworkReplyHttpImpl::setReadBufferSize(qint64 size)
{
   QNetworkReply::setReadBufferSize(size);
   emit readBufferSizeChanged(size);
   return;
}

bool QNetworkReplyHttpImpl::canReadLine () const
{
   Q_D(const QNetworkReplyHttpImpl);

   if (QNetworkReply::canReadLine()) {
      return true;
   }

   if (d->cacheLoadDevice) {
      return d->cacheLoadDevice->canReadLine() || d->downloadMultiBuffer.canReadLine();
   }

   if (d->downloadZerocopyBuffer) {
      return memchr(d->downloadZerocopyBuffer + d->downloadBufferReadPosition, '\n', d->downloadBufferCurrentSize - d->downloadBufferReadPosition);
   }

   return d->downloadMultiBuffer.canReadLine();
}

#ifdef QT_SSL
void QNetworkReplyHttpImpl::ignoreSslErrors()
{
   Q_D(QNetworkReplyHttpImpl);

   d->pendingIgnoreAllSslErrors = true;
}

void QNetworkReplyHttpImpl::ignoreSslErrorsImplementation(const QList<QSslError> &errors)
{
   Q_D(QNetworkReplyHttpImpl);

   // the pending list is set if QNetworkReply::ignoreSslErrors(const QList<QSslError> &errors)
   // is called before QNetworkAccessManager::get() (or post(), etc.)
   d->pendingIgnoreSslErrorsList = errors;
}

void QNetworkReplyHttpImpl::setSslConfigurationImplementation(const QSslConfiguration &newconfig)
{
   // Setting a SSL configuration on a reply is not supported. The user needs to set
   // her/his QSslConfiguration on the QNetworkRequest.
   Q_UNUSED(newconfig);
}

void QNetworkReplyHttpImpl::sslConfigurationImplementation(QSslConfiguration &configuration) const
{
   Q_D(const QNetworkReplyHttpImpl);
   configuration = d->sslConfiguration;
}
#endif

QNetworkReplyHttpImplPrivate::QNetworkReplyHttpImplPrivate()
   : QNetworkReplyPrivate()
   , manager(0)
   , managerPrivate(0)
   , synchronous(false)
   , state(Idle)
   , statusCode(0)
   , uploadByteDevicePosition(false)
   , uploadDeviceChoking(false)
   , outgoingData(0)
   , bytesUploaded(-1)
   , cacheLoadDevice(0)
   , loadingFromCache(false)
   , cacheSaveDevice(0)
   , cacheEnabled(false)
   , resumeOffset(0)
   , preMigrationDownloaded(-1)
   , bytesDownloaded(0)
   , downloadBufferReadPosition(0)
   , downloadBufferCurrentSize(0)
   , downloadZerocopyBuffer(0)
   , pendingDownloadDataEmissions(QSharedPointer<QAtomicInt>::create())
   , pendingDownloadProgressEmissions(QSharedPointer<QAtomicInt>::create())
#ifdef QT_SSL
   , pendingIgnoreAllSslErrors(false)
#endif

{
}

QNetworkReplyHttpImplPrivate::~QNetworkReplyHttpImplPrivate()
{
}

/*
    For a given httpRequest
    1) If AlwaysNetwork, return
    2) If we have a cache entry for this url populate headers so the server can return 304
    3) Calculate if response_is_fresh and if so send the cache and set loadedFromCache to true
 */
bool QNetworkReplyHttpImplPrivate::loadFromCacheIfAllowed(QHttpNetworkRequest &httpRequest)
{
   QNetworkRequest::CacheLoadControl CacheLoadControlAttribute =
      (QNetworkRequest::CacheLoadControl)request.attribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork).toInt();
   if (CacheLoadControlAttribute == QNetworkRequest::AlwaysNetwork) {
      // If the request does not already specify preferred cache-control
      // force reload from the network and tell any caching proxy servers to reload too
      if (!request.rawHeaderList().contains("Cache-Control")) {
         httpRequest.setHeaderField("Cache-Control", "no-cache");
         httpRequest.setHeaderField("Pragma", "no-cache");
      }
      return false;
   }

   // The disk cache API does not currently support partial content retrieval.
   // That is why we don't use the disk cache for any such requests.
   if (request.hasRawHeader("Range")) {
      return false;
   }

   QAbstractNetworkCache *nc = managerPrivate->networkCache;
   if (!nc) {
      return false;   // no local cache
   }

   QNetworkCacheMetaData metaData = nc->metaData(httpRequest.url());
   if (!metaData.isValid()) {
      return false;   // not in cache
   }

   if (!metaData.saveToDisk()) {
      return false;
   }

   QNetworkHeadersPrivate cacheHeaders;
   QNetworkHeadersPrivate::RawHeadersList::const_iterator it;
   cacheHeaders.setAllRawHeaders(metaData.rawHeaders());

   it = cacheHeaders.findRawHeader("etag");
   if (it != cacheHeaders.rawHeaders.constEnd()) {
      httpRequest.setHeaderField("If-None-Match", it->second);
   }

   QDateTime lastModified = metaData.lastModified();
   if (lastModified.isValid()) {
      httpRequest.setHeaderField("If-Modified-Since", QNetworkHeadersPrivate::toHttpDate(lastModified));
   }

   it = cacheHeaders.findRawHeader("Cache-Control");
   if (it != cacheHeaders.rawHeaders.constEnd()) {
      QHash<QByteArray, QByteArray> cacheControl = parseHttpOptionHeader(it->second);
      if (cacheControl.contains("must-revalidate")) {
         return false;
      }
   }

   QDateTime currentDateTime = QDateTime::currentDateTimeUtc();
   QDateTime expirationDate = metaData.expirationDate();

   bool response_is_fresh;
   if (!expirationDate.isValid()) {
      /*
       * age_value
       *      is the value of Age: header received by the cache with
       *              this response.
       * date_value
       *      is the value of the origin server's Date: header
       * request_time
       *      is the (local) time when the cache made the request
       *              that resulted in this cached response
       * response_time
       *      is the (local) time when the cache received the
       *              response
       * now
       *      is the current (local) time
       */
      int age_value = 0;
      it = cacheHeaders.findRawHeader("age");
      if (it != cacheHeaders.rawHeaders.constEnd()) {
         age_value = it->second.toInt();
      }

      QDateTime dateHeader;
      int date_value = 0;
      it = cacheHeaders.findRawHeader("date");
      if (it != cacheHeaders.rawHeaders.constEnd()) {
         dateHeader = QNetworkHeadersPrivate::fromHttpDate(it->second);
         date_value = dateHeader.toTime_t();
      }

      int now = currentDateTime.toTime_t();
      int request_time = now;
      int response_time = now;

      // Algorithm from RFC 2616 section 13.2.3
      int apparent_age = qMax(0, response_time - date_value);
      int corrected_received_age = qMax(apparent_age, age_value);
      int response_delay = response_time - request_time;
      int corrected_initial_age = corrected_received_age + response_delay;
      int resident_time = now - response_time;
      int current_age   = corrected_initial_age + resident_time;

      int freshness_lifetime = 0;

      // RFC 2616 13.2.4 Expiration Calculations
      if (lastModified.isValid() && dateHeader.isValid()) {
         int diff = lastModified.secsTo(dateHeader);
         freshness_lifetime = diff / 10;
         if (httpRequest.headerField("Warning").isEmpty()) {
            QDateTime dt = currentDateTime.addSecs(current_age);
            if (currentDateTime.daysTo(dt) > 1) {
               httpRequest.setHeaderField("Warning", "113");
            }
         }
      }

      // the cache-saving code below sets the freshness_lifetime with (dateHeader - last_modified) / 10
      // if "last-modified" is present, or to Expires otherwise
      response_is_fresh = (freshness_lifetime > current_age);
   } else {
      // expiration date was calculated earlier (e.g. when storing object to the cache)
      response_is_fresh = currentDateTime.secsTo(expirationDate) >= 0;
   }

   if (!response_is_fresh) {
      return false;
   }

#if defined(QNETWORKACCESSHTTPBACKEND_DEBUG)
   qDebug() << "response_is_fresh" << CacheLoadControlAttribute;
#endif
   return sendCacheContents(metaData);
}

QHttpNetworkRequest::Priority QNetworkReplyHttpImplPrivate::convert(const QNetworkRequest::Priority &prio)
{
   switch (prio) {
      case QNetworkRequest::LowPriority:
         return QHttpNetworkRequest::LowPriority;
      case QNetworkRequest::HighPriority:
         return QHttpNetworkRequest::HighPriority;
      case QNetworkRequest::NormalPriority:
      default:
         return QHttpNetworkRequest::NormalPriority;
   }
}

void QNetworkReplyHttpImplPrivate::postRequest(const QNetworkRequest &newHttpRequest)
{
   Q_Q(QNetworkReplyHttpImpl);

   QThread *thread = 0;
   if (synchronous) {
      // A synchronous HTTP request uses its own thread
      thread = new QThread();
      thread->setObjectName("HTTP Synchronous Thread");
      QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
      thread->start();

   } else if (!managerPrivate->httpThread) {
      // We use the manager-global thread.
      // At some point we could switch to having multiple threads if it makes sense.
      managerPrivate->httpThread = new QThread();
      managerPrivate->httpThread->setObjectName("HTTP Thread");
      managerPrivate->httpThread->start();

      thread = managerPrivate->httpThread;
   } else {
      // Asynchronous request, thread already exists
      thread = managerPrivate->httpThread;
   }

   QUrl url = newHttpRequest.url();
   httpRequest.setUrl(url);
   httpRequest.setRedirectCount(newHttpRequest.maximumRedirectsAllowed());

   QString scheme = url.scheme();
   bool ssl = (scheme == QLatin1String("https") || scheme == QLatin1String("preconnect-https"));
   q->setAttribute(QNetworkRequest::ConnectionEncryptedAttribute, ssl);
   httpRequest.setSsl(ssl);

   bool preConnect = (scheme == QLatin1String("preconnect-http")
                      || scheme == QLatin1String("preconnect-https"));
   httpRequest.setPreConnect(preConnect);

#ifndef QT_NO_NETWORKPROXY
   QNetworkProxy transparentProxy, cacheProxy;

   // FIXME the proxy stuff should be done in the HTTP thread
   for (const QNetworkProxy &p : managerPrivate->queryProxy(QNetworkProxyQuery(newHttpRequest.url()))) {
      // use the first proxy that works
      // for non-encrypted connections, any transparent or HTTP proxy
      // for encrypted, only transparent proxies
      if (!ssl
            && (p.capabilities() & QNetworkProxy::CachingCapability)
            && (p.type() == QNetworkProxy::HttpProxy ||
                p.type() == QNetworkProxy::HttpCachingProxy)) {
         cacheProxy = p;
         transparentProxy = QNetworkProxy::NoProxy;
         break;
      }
      if (p.isTransparentProxy()) {
         transparentProxy = p;
         cacheProxy = QNetworkProxy::NoProxy;
         break;
      }
   }

   // check if at least one of the proxies
   if (transparentProxy.type() == QNetworkProxy::DefaultProxy &&
         cacheProxy.type() == QNetworkProxy::DefaultProxy) {
      // unsuitable proxies
      QMetaObject::invokeMethod(q, "_q_error", synchronous ? Qt::DirectConnection : Qt::QueuedConnection,
                                Q_ARG(QNetworkReply::NetworkError, QNetworkReply::ProxyNotFoundError),
                                Q_ARG(QString, QNetworkReplyHttpImpl::tr("No suitable proxy found")));

      QMetaObject::invokeMethod(q, "_q_finished", synchronous ? Qt::DirectConnection : Qt::QueuedConnection);
      return;
   }
#endif

   if (newHttpRequest.attribute(QNetworkRequest::FollowRedirectsAttribute).toBool()) {
      httpRequest.setFollowRedirects(true);
   }

   httpRequest.setPriority(convert(newHttpRequest.priority()));

   switch (operation) {
      case QNetworkAccessManager::GetOperation:
         httpRequest.setOperation(QHttpNetworkRequest::Get);
         if (loadFromCacheIfAllowed(httpRequest)) {
            return;   // no need to send the request! :)
         }
         break;

      case QNetworkAccessManager::HeadOperation:
         httpRequest.setOperation(QHttpNetworkRequest::Head);
         if (loadFromCacheIfAllowed(httpRequest)) {
            return;   // no need to send the request! :)
         }
         break;

      case QNetworkAccessManager::PostOperation:
         invalidateCache();
         httpRequest.setOperation(QHttpNetworkRequest::Post);
         createUploadByteDevice();
         break;

      case QNetworkAccessManager::PutOperation:
         invalidateCache();
         httpRequest.setOperation(QHttpNetworkRequest::Put);
         createUploadByteDevice();
         break;

      case QNetworkAccessManager::DeleteOperation:
         invalidateCache();
         httpRequest.setOperation(QHttpNetworkRequest::Delete);
         break;

      case QNetworkAccessManager::CustomOperation:
         invalidateCache(); // for safety reasons, we don't know what the operation does
         httpRequest.setOperation(QHttpNetworkRequest::Custom);
         createUploadByteDevice();
         httpRequest.setCustomVerb(newHttpRequest.attribute(
                                      QNetworkRequest::CustomVerbAttribute).toByteArray());
         break;

      default:
         break;                  // can't happen
   }

   QList<QByteArray> headers = newHttpRequest.rawHeaderList();
   if (resumeOffset != 0) {
      if (headers.contains("Range")) {
         // Need to adjust resume offset for user specified range

         headers.removeOne("Range");

         // We've already verified that requestRange starts with "bytes=", see canResume.
         QByteArray requestRange = newHttpRequest.rawHeader("Range").mid(6);

         int index = requestRange.indexOf('-');

         quint64 requestStartOffset = requestRange.left(index).toULongLong();
         quint64 requestEndOffset = requestRange.mid(index + 1).toULongLong();

         requestRange = "bytes=" + QByteArray::number(resumeOffset + requestStartOffset) +
                        '-' + QByteArray::number(requestEndOffset);

         httpRequest.setHeaderField("Range", requestRange);
      } else {
         httpRequest.setHeaderField("Range", "bytes=" + QByteArray::number(resumeOffset) + '-');
      }
   }

   for (const QByteArray &header : headers) {
      httpRequest.setHeaderField(header, newHttpRequest.rawHeader(header));
   }

   if (newHttpRequest.attribute(QNetworkRequest::HttpPipeliningAllowedAttribute).toBool()) {
      httpRequest.setPipeliningAllowed(true);
   }

   if (request.attribute(QNetworkRequest::SpdyAllowedAttribute).toBool()) {
      httpRequest.setSPDYAllowed(true);
   }

   if (static_cast<QNetworkRequest::LoadControl>
         (newHttpRequest.attribute(QNetworkRequest::AuthenticationReuseAttribute,
                                   QNetworkRequest::Automatic).toInt()) == QNetworkRequest::Manual) {
      httpRequest.setWithCredentials(false);
   }

   if (request.attribute(QNetworkRequest::EmitAllUploadProgressSignalsAttribute).toBool()) {
      emitAllUploadProgressSignals = true;
   }


   // Create the HTTP thread delegate
   QHttpThreadDelegate *delegate = new QHttpThreadDelegate;

#ifndef QT_NO_BEARERMANAGEMENT
   delegate->networkSession = managerPrivate->getNetworkSession();
#endif

   // For the synchronous HTTP, this is the normal way the delegate gets deleted
   // For the asynchronous HTTP this is a safety measure, the delegate deletes itself when HTTP is finished
   QObject::connect(thread, SIGNAL(finished()), delegate, SLOT(deleteLater()));

   // Set the properties it needs
   delegate->httpRequest = httpRequest;

#ifndef QT_NO_NETWORKPROXY
   delegate->cacheProxy = cacheProxy;
   delegate->transparentProxy = transparentProxy;
#endif

   delegate->ssl = ssl;

#ifdef QT_SSL
   if (ssl) {
      delegate->incomingSslConfiguration = newHttpRequest.sslConfiguration();
   }
#endif

   // Do we use synchronous HTTP?
   delegate->synchronous = synchronous;

   // The authentication manager is used to avoid the BlockingQueuedConnection communication
   // from HTTP thread to user thread in some cases.
   delegate->authenticationManager = managerPrivate->authenticationManager;

   if (!synchronous) {
      // Tell our zerocopy policy to the delegate
      QVariant downloadBufferMaximumSizeAttribute = newHttpRequest.attribute(QNetworkRequest::MaximumDownloadBufferSizeAttribute);
      if (downloadBufferMaximumSizeAttribute.isValid()) {
         delegate->downloadBufferMaximumSize = downloadBufferMaximumSizeAttribute.toLongLong();
      } else {
         // If there is no MaximumDownloadBufferSizeAttribute set (which is for the majority
         // of QNetworkRequest) then we can assume we'll do it anyway for small HTTP replies.
         // This helps with performance and memory fragmentation.
         delegate->downloadBufferMaximumSize = 128 * 1024;
      }

      // These atomic integers are used for signal compression
      delegate->pendingDownloadData = pendingDownloadDataEmissions;
      delegate->pendingDownloadProgress = pendingDownloadProgressEmissions;

      // Connect the signals of the delegate to us
      QObject::connect(delegate, SIGNAL(downloadData(QByteArray)),
                  q, SLOT(replyDownloadData(QByteArray)), Qt::QueuedConnection);

      QObject::connect(delegate, SIGNAL(downloadFinished()),
                  q, SLOT(replyFinished()), Qt::QueuedConnection);

      QObject::connect(delegate, SIGNAL(downloadMetaData(QList<QPair<QByteArray, QByteArray> >,
                  int, QString, bool, QSharedPointer<char>, qint64, bool)),
                  q, SLOT(replyDownloadMetaData(QList<QPair<QByteArray, QByteArray> >, int, QString, bool,
                  QSharedPointer<char>, qint64, bool)), Qt::QueuedConnection);

      QObject::connect(delegate, SIGNAL(downloadProgress(qint64, qint64)),
                  q, SLOT(replyDownloadProgressSlot(qint64, qint64)), Qt::QueuedConnection);

      QObject::connect(delegate, SIGNAL(error(QNetworkReply::NetworkError, QString)),
                  q, SLOT(httpError(QNetworkReply::NetworkError, QString)), Qt::QueuedConnection);

      QObject::connect(delegate, SIGNAL(redirected(QUrl, int, int)),
                  q, SLOT(onRedirected(QUrl, int, int)), Qt::QueuedConnection);


#ifdef QT_SSL
      QObject::connect(delegate, SIGNAL(sslConfigurationChanged(QSslConfiguration)),
                  q, SLOT(replySslConfigurationChanged(QSslConfiguration)), Qt::QueuedConnection);
#endif

      // Those need to report back, therefire BlockingQueuedConnection
      QObject::connect(delegate, SIGNAL(authenticationRequired(QHttpNetworkRequest, QAuthenticator *)),
                  q, SLOT(httpAuthenticationRequired(QHttpNetworkRequest, QAuthenticator *)), Qt::BlockingQueuedConnection);

#ifndef QT_NO_NETWORKPROXY
      QObject::connect(delegate, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)),
                       q, SLOT(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), Qt::BlockingQueuedConnection);
#endif

#ifdef QT_SSL
      QObject::connect(delegate, SIGNAL(encrypted()), q, SLOT(replyEncrypted()), Qt::BlockingQueuedConnection);

      QObject::connect(delegate, SIGNAL(sslErrors(const QList<QSslError> &, bool *, QList<QSslError> *)),
                       q, SLOT(replySslErrors(const QList<QSslError> &, bool *, QList<QSslError> *)), Qt::BlockingQueuedConnection);

      QObject::connect(delegate, SIGNAL(preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *)),
                       q, SLOT(replyPreSharedKeyAuthenticationRequiredSlot(QSslPreSharedKeyAuthenticator *)), Qt::BlockingQueuedConnection);
#endif

      // this signal we will use to start the request
      QObject::connect(q, SIGNAL(startHttpRequest()), delegate, SLOT(startRequest()));
      QObject::connect(q, SIGNAL(abortHttpRequest()), delegate, SLOT(abortRequest()));

      // to throttle the connection
      QObject::connect(q, SIGNAL(readBufferSizeChanged(qint64)), delegate, SLOT(readBufferSizeChanged(qint64)));
      QObject::connect(q, SIGNAL(readBufferFreed(qint64)),       delegate, SLOT(readBufferFreed(qint64)));

      if (uploadByteDevice) {
         QNonContiguousByteDeviceThreadForwardImpl *forwardUploadDevice =
            new QNonContiguousByteDeviceThreadForwardImpl(uploadByteDevice->atEnd(), uploadByteDevice->size());

         forwardUploadDevice->setParent(delegate); // needed to make sure it is moved on moveToThread()
         delegate->httpRequest.setUploadByteDevice(forwardUploadDevice);

         // If the device in the user thread claims it has more data, keep the flow to HTTP thread going
         QObject::connect(uploadByteDevice.data(), SIGNAL(readyRead()),
                          q, SLOT(uploadByteDeviceReadyReadSlot()), Qt::QueuedConnection);

         // From user thread to http thread:
         QObject::connect(q, SIGNAL(haveUploadData(qint64, QByteArray, bool, qint64)),
                          forwardUploadDevice, SLOT(haveDataSlot(qint64, QByteArray, bool, qint64)), Qt::QueuedConnection);

         QObject::connect(uploadByteDevice.data(), SIGNAL(readyRead()),
                          forwardUploadDevice, SLOT(readyRead()), Qt::QueuedConnection);

         // From http thread to user thread:
         QObject::connect(forwardUploadDevice, SIGNAL(wantData(qint64)),
                          q, SLOT(wantUploadDataSlot(qint64)));

         QObject::connect(forwardUploadDevice, SIGNAL(processedData(qint64, qint64)),
                          q, SLOT(sentUploadDataSlot(qint64, qint64)));

         QObject::connect(forwardUploadDevice, SIGNAL(resetData(bool *)), q, SLOT(resetUploadDataSlot(bool *)),
                          Qt::BlockingQueuedConnection); // this is the only one with BlockingQueued!
      }
   } else if (synchronous) {
      QObject::connect(q, SIGNAL(startHttpRequestSynchronously()), delegate, SLOT(startRequestSynchronously()),
                       Qt::BlockingQueuedConnection);

      if (uploadByteDevice) {
         // For the synchronous HTTP use case the use thread (this one here) is blocked
         // so we cannot use the asynchronous upload architecture.
         // We therefore won't use the QNonContiguousByteDeviceThreadForwardImpl but directly
         // use the uploadByteDevice provided to us by the QNetworkReplyImpl.
         // The code that is in start() makes sure it is safe to use from a thread
         // since it only wraps a QRingBuffer

         delegate->httpRequest.setUploadByteDevice(uploadByteDevice.data());
      }
   }


   // Move the delegate to the http thread
   delegate->moveToThread(thread);
   // This call automatically moves the uploadDevice too for the asynchronous case.

   // Prepare timers for progress notifications
   downloadProgressSignalChoke.start();
   uploadProgressSignalChoke.invalidate();

   // Send an signal to the delegate so it starts working in the other thread
   if (synchronous) {
      emit q->startHttpRequestSynchronously(); // This one is BlockingQueuedConnection, so it will return when all work is done

      if (delegate->incomingErrorCode != QNetworkReply::NoError) {
         replyDownloadMetaData(delegate->incomingHeaders, delegate->incomingStatusCode, delegate->incomingReasonPhrase,
                  delegate->isPipeliningUsed, QSharedPointer<char>(), delegate->incomingContentLength, delegate->isSpdyUsed);

         replyDownloadData(delegate->synchronousDownloadData);
         httpError(delegate->incomingErrorCode, delegate->incomingErrorDetail);

      } else {
         replyDownloadMetaData(delegate->incomingHeaders, delegate->incomingStatusCode, delegate->incomingReasonPhrase,
                  delegate->isPipeliningUsed, QSharedPointer<char>(), delegate->incomingContentLength, delegate->isSpdyUsed);
         replyDownloadData(delegate->synchronousDownloadData);
      }

      thread->quit();
      thread->wait(5000);

      if (thread->isFinished()) {
         delete thread;
      } else {
         QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
      }

      finished();

   } else {
      emit q->startHttpRequest(); // Signal to the HTTP thread and go back to user.
   }
}

void QNetworkReplyHttpImplPrivate::invalidateCache()
{
   QAbstractNetworkCache *nc = managerPrivate->networkCache;
   if (nc) {
      nc->remove(httpRequest.url());
   }
}

void QNetworkReplyHttpImplPrivate::initCacheSaveDevice()
{
   Q_Q(QNetworkReplyHttpImpl);

   // The disk cache does not support partial content, so don't even try to
   // save any such content into the cache.
   if (q->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 206) {
      cacheEnabled = false;
      return;
   }

   // save the meta data
   QNetworkCacheMetaData metaData;
   metaData.setUrl(url);
   metaData = fetchCacheMetaData(metaData);

   // save the redirect request also in the cache
   QVariant redirectionTarget = q->attribute(QNetworkRequest::RedirectionTargetAttribute);
   if (redirectionTarget.isValid()) {
      QNetworkCacheMetaData::AttributesMap attributes = metaData.attributes();
      attributes.insert(QNetworkRequest::RedirectionTargetAttribute, redirectionTarget);
      metaData.setAttributes(attributes);
   }

   cacheSaveDevice = managerPrivate->networkCache->prepare(metaData);

   if (cacheSaveDevice) {
      q->connect(cacheSaveDevice, SIGNAL(aboutToClose()), SLOT(_q_cacheSaveDeviceAboutToClose()));
   }

   if (!cacheSaveDevice || (cacheSaveDevice && !cacheSaveDevice->isOpen())) {
      if (cacheSaveDevice && !cacheSaveDevice->isOpen())
         qCritical("QNetworkReplyImpl: network cache returned a device that is not open -- "
                   "class %s probably needs to be fixed",
                   csPrintable(managerPrivate->networkCache->metaObject()->className()));

      managerPrivate->networkCache->remove(url);
      cacheSaveDevice = 0;
      cacheEnabled = false;
   }
}

void QNetworkReplyHttpImplPrivate::replyDownloadData(QByteArray d)
{
   Q_Q(QNetworkReplyHttpImpl);

   // If we're closed just ignore this data
   if (!q->isOpen()) {
      return;
   }

   int pendingSignals = (int)pendingDownloadDataEmissions->fetchAndAddAcquire(-1) - 1;

   if (pendingSignals > 0) {
      // Some more signal emissions to this slot are pending.
      // Instead of writing the downstream data, we wait
      // and do it in the next call we get
      // (signal comppression)
      pendingDownloadData.append(d);
      return;
   }

   pendingDownloadData.append(d);
   d.clear();
   // We need to usa a copy for calling writeDownstreamData as we could
   // possibly recurse into this this function when we call
   // appendDownstreamDataSignalEmissions because the user might call
   // processEvents() or spin an event loop when this occur.
   QByteDataBuffer pendingDownloadDataCopy = pendingDownloadData;
   pendingDownloadData.clear();

   if (cacheEnabled && isCachingAllowed() && !cacheSaveDevice) {
      initCacheSaveDevice();
   }

   qint64 bytesWritten = 0;
   for (int i = 0; i < pendingDownloadDataCopy.bufferCount(); i++) {
      QByteArray const &item = pendingDownloadDataCopy[i];

      // This is going to look a little strange. When downloading data while a
      // HTTP redirect is happening (and enabled), we write the redirect
      // response to the cache. However, we do not append it to our internal
      // buffer as that will contain the response data only for the final
      // response
      if (cacheSaveDevice) {
         cacheSaveDevice->write(item.constData(), item.size());
      }

      if (!isHttpRedirectResponse()) {
         downloadMultiBuffer.append(item);
      }

      bytesWritten += item.size();
   }
   pendingDownloadDataCopy.clear();

   QVariant totalSize = cookedHeaders.value(QNetworkRequest::ContentLengthHeader);
   if (preMigrationDownloaded != Q_INT64_C(-1)) {
      totalSize = totalSize.toLongLong() + preMigrationDownloaded;
   }

   if (isHttpRedirectResponse()) {
      return;
   }

   bytesDownloaded += bytesWritten;

   emit q->readyRead();
   // emit readyRead before downloadProgress incase this will cause events to be
   // processed and we get into a recursive call (as in QProgressDialog).
   if (downloadProgressSignalChoke.elapsed() >= progressSignalInterval) {
      downloadProgressSignalChoke.restart();
      emit q->downloadProgress(bytesDownloaded,
                               totalSize.isNull() ? Q_INT64_C(-1) : totalSize.toLongLong());
   }

}

void QNetworkReplyHttpImplPrivate::replyFinished()
{
   // We are already loading from cache, we still however
   // got this signal because it was posted already
   if (loadingFromCache) {
      return;
   }

   finished();
}

QNetworkAccessManager::Operation QNetworkReplyHttpImplPrivate::getRedirectOperation(QNetworkAccessManager::Operation currentOp, int httpStatus)
{
   // HTTP status code can be used to decide if we can redirect with a GET
   // operation or not. See http://www.ietf.org/rfc/rfc2616.txt [Sec 10.3] for
   // more details
   Q_UNUSED(httpStatus);

   switch (currentOp) {
      case QNetworkAccessManager::HeadOperation:
         return QNetworkAccessManager::HeadOperation;
      default:
         break;
   }
   // For now, we're always returning GET for anything other than HEAD
   return QNetworkAccessManager::GetOperation;
}

bool QNetworkReplyHttpImplPrivate::isHttpRedirectResponse() const
{
   return httpRequest.isFollowRedirects() && QHttpNetworkReply::isHttpRedirect(statusCode);
}

QNetworkRequest QNetworkReplyHttpImplPrivate::createRedirectRequest(const QNetworkRequest &originalRequest,
      const QUrl &url,
      int maxRedirectsRemaining)
{
   QNetworkRequest newRequest(originalRequest);
   newRequest.setUrl(url);
   newRequest.setMaximumRedirectsAllowed(maxRedirectsRemaining);

   return newRequest;
}

void QNetworkReplyHttpImplPrivate::onRedirected(QUrl redirectUrl, int httpStatus, int maxRedirectsRemaining)
{
   Q_Q(QNetworkReplyHttpImpl);

   if (isFinished) {
      return;
   }

   if (httpRequest.isFollowRedirects()) { // update the reply's url as it could've changed
      url = redirectUrl;
   }

   QNetworkRequest redirectRequest = createRedirectRequest(originalRequest, redirectUrl, maxRedirectsRemaining);
   operation = getRedirectOperation(operation, httpStatus);

   cookedHeaders.clear();

   if (managerPrivate->httpThread) {
      managerPrivate->httpThread->disconnect();
   }

   // Recurse
   QMetaObject::invokeMethod(q, "start", Qt::QueuedConnection,
                             Q_ARG(QNetworkRequest, redirectRequest));

   emit q->redirected(redirectUrl);
}

void QNetworkReplyHttpImplPrivate::checkForRedirect(const int statusCode)
{
   Q_Q(QNetworkReplyHttpImpl);
   switch (statusCode) {
      case 301:                   // Moved Permanently
      case 302:                   // Found
      case 303:                   // See Other
      case 307:                   // Temporary Redirect
         // What do we do about the caching of the HTML note?
         // The response to a 303 MUST NOT be cached, while the response to
         // all of the others is cacheable if the headers indicate it to be
         QByteArray header = q->rawHeader("location");
         QUrl url = QUrl(QString::fromUtf8(header));
         if (!url.isValid()) {
            url = QUrl(QLatin1String(header));
         }
         q->setAttribute(QNetworkRequest::RedirectionTargetAttribute, url);
   }
}

void QNetworkReplyHttpImplPrivate::replyDownloadMetaData
(QList<QPair<QByteArray, QByteArray> > hm,
 int sc, QString rp, bool pu,
 QSharedPointer<char> db,
 qint64 contentLength, bool spdyWasUsed)
{
   Q_Q(QNetworkReplyHttpImpl);
   Q_UNUSED(contentLength);

   statusCode = sc;
   reasonPhrase = rp;

   // Download buffer
   if (!db.isNull()) {
      downloadBufferPointer = db;
      downloadZerocopyBuffer = downloadBufferPointer.data();
      downloadBufferCurrentSize = 0;

      q->setAttribute(QNetworkRequest::DownloadBufferAttribute,
                      QVariant::fromValue<QSharedPointer<char> > (downloadBufferPointer));
   }

   q->setAttribute(QNetworkRequest::HttpPipeliningWasUsedAttribute, pu);
   q->setAttribute(QNetworkRequest::SpdyWasUsedAttribute, spdyWasUsed);

   // reconstruct the HTTP header
   QList<QPair<QByteArray, QByteArray> > headerMap = hm;
   QList<QPair<QByteArray, QByteArray> >::const_iterator it = headerMap.constBegin(), end = headerMap.constEnd();

   for (; it != end; ++it) {
      QByteArray value = q->rawHeader(it->first);

      // Reset any previous "location" header set in the reply. In case of
      // redirects, we don't want to 'append' multiple location header values,
      // rather we keep only the latest one
      if (it->first.toLower() == "location") {
         value.clear();
      }

      if (!value.isEmpty()) {
         // Why are we appending values for headers which are already
         // present?
         if (qstricmp(it->first.constData(), "set-cookie") == 0) {
            value += '\n';
         } else {
            value += ", ";
         }
      }
      value += it->second;
      q->setRawHeader(it->first, value);
   }

   q->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, statusCode);
   q->setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, reasonPhrase);

   // is it a redirection?
   if (!isHttpRedirectResponse()) {
      checkForRedirect(statusCode);
   }

   if (statusCode >= 500 && statusCode < 600) {
      QAbstractNetworkCache *nc = managerPrivate->networkCache;
      if (nc) {
         QNetworkCacheMetaData metaData = nc->metaData(httpRequest.url());
         QNetworkHeadersPrivate cacheHeaders;
         cacheHeaders.setAllRawHeaders(metaData.rawHeaders());
         QNetworkHeadersPrivate::RawHeadersList::const_iterator it;
         it = cacheHeaders.findRawHeader("Cache-Control");
         bool mustReValidate = false;
         if (it != cacheHeaders.rawHeaders.constEnd()) {
            QHash<QByteArray, QByteArray> cacheControl = parseHttpOptionHeader(it->second);
            if (cacheControl.contains("must-revalidate")) {
               mustReValidate = true;
            }
         }
         if (!mustReValidate && sendCacheContents(metaData)) {
            return;
         }
      }
   }

   if (statusCode == 304) {
#if defined(QNETWORKACCESSHTTPBACKEND_DEBUG)
      qDebug() << "Received a 304 from" << request.url();
#endif
      QAbstractNetworkCache *nc = managerPrivate->networkCache;
      if (nc) {
         QNetworkCacheMetaData oldMetaData = nc->metaData(httpRequest.url());
         QNetworkCacheMetaData metaData = fetchCacheMetaData(oldMetaData);
         if (oldMetaData != metaData) {
            nc->updateMetaData(metaData);
         }
         if (sendCacheContents(metaData)) {
            return;
         }
      }
   }


   if (statusCode != 304 && statusCode != 303) {
      if (!isCachingEnabled()) {
         setCachingEnabled(true);
      }
   }

   _q_metaDataChanged();
}

void QNetworkReplyHttpImplPrivate::replyDownloadProgressSlot(qint64 bytesReceived,  qint64 bytesTotal)
{
   Q_Q(QNetworkReplyHttpImpl);

   // If we're closed just ignore this data
   if (!q->isOpen()) {
      return;
   }

   // we can be sure here that there is a download buffer

   int pendingSignals = (int)pendingDownloadProgressEmissions->fetchAndAddAcquire(-1) - 1;
   if (pendingSignals > 0) {
      // Let's ignore this signal and look at the next one coming in
      // (signal comppression)
      return;
   }

   if (!q->isOpen()) {
      return;
   }

   if (cacheEnabled && isCachingAllowed() && bytesReceived == bytesTotal) {
      // Write everything in one go if we use a download buffer. might be more performant.
      initCacheSaveDevice();
      // need to check again if cache enabled and device exists
      if (cacheSaveDevice && cacheEnabled) {
         cacheSaveDevice->write(downloadZerocopyBuffer, bytesTotal);
      }
      // FIXME where is it closed?
   }

   if (isHttpRedirectResponse()) {
      return;
   }

   bytesDownloaded = bytesReceived;

   downloadBufferCurrentSize = bytesReceived;

   // Only emit readyRead when actual data is there
   // emit readyRead before downloadProgress incase this will cause events to be
   // processed and we get into a recursive call (as in QProgressDialog).
   if (bytesDownloaded > 0) {
      emit q->readyRead();
   }
   if (downloadProgressSignalChoke.elapsed() >= progressSignalInterval) {
      downloadProgressSignalChoke.restart();
      emit q->downloadProgress(bytesDownloaded, bytesTotal);
   }
}

void QNetworkReplyHttpImplPrivate::httpAuthenticationRequired(const QHttpNetworkRequest &request, QAuthenticator *auth)
{
   managerPrivate->authenticationRequired(auth, q_func(), synchronous, url, &urlForLastAuthentication, request.withCredentials());
}

#ifndef QT_NO_NETWORKPROXY
void QNetworkReplyHttpImplPrivate::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)
{
   managerPrivate->proxyAuthenticationRequired(request.url(), proxy, synchronous, authenticator, &lastProxyAuthentication);
}
#endif

void QNetworkReplyHttpImplPrivate::httpError(QNetworkReply::NetworkError errorCode, QString errorString)
{

#if defined(QNETWORKACCESSHTTPBACKEND_DEBUG)
   qDebug() << "http error!" << errorCode << errorString;
#endif

   // FIXME?
   error(errorCode, errorString);
}

#ifdef QT_SSL
void QNetworkReplyHttpImplPrivate::replyEncrypted()
{
   Q_Q(QNetworkReplyHttpImpl);
   emit q->encrypted();
}

void QNetworkReplyHttpImplPrivate::replySslErrors(const QList<QSslError> &list, bool *ignoreAll, QList<QSslError> *toBeIgnored)
{
   Q_Q(QNetworkReplyHttpImpl);
   emit q->sslErrors(list);

   // Check if the callback set any ignore and return this here to http thread
   if (pendingIgnoreAllSslErrors) {
      *ignoreAll = true;
   }
   if (!pendingIgnoreSslErrorsList.isEmpty()) {
      *toBeIgnored = pendingIgnoreSslErrorsList;
   }
}

void QNetworkReplyHttpImplPrivate::replySslConfigurationChanged(QSslConfiguration sslConfiguration)
{
   // Receiving the used SSL configuration from the HTTP thread
   this->sslConfiguration = sslConfiguration;
}

void QNetworkReplyHttpImplPrivate::replyPreSharedKeyAuthenticationRequiredSlot(QSslPreSharedKeyAuthenticator *authenticator)
{
   Q_Q(QNetworkReplyHttpImpl);
   emit q->preSharedKeyAuthenticationRequired(authenticator);
}
#endif

// Coming from QNonContiguousByteDeviceThreadForwardImpl in HTTP thread
void QNetworkReplyHttpImplPrivate::resetUploadDataSlot(bool *r)
{
   *r = uploadByteDevice->reset();
   if (*r) {
      // reset our own position which is used for the inter-thread communication
      uploadByteDevicePosition = 0;
   }
}

// Coming from QNonContiguousByteDeviceThreadForwardImpl in HTTP thread
void QNetworkReplyHttpImplPrivate::sentUploadDataSlot(qint64 pos, qint64 amount)
{
   if (uploadByteDevicePosition + amount != pos) {
      // Sanity check, should not happen.
      error(QNetworkReply::UnknownNetworkError, QString());
      return;
   }
   uploadByteDevice->advanceReadPointer(amount);
   uploadByteDevicePosition += amount;
}

// Coming from QNonContiguousByteDeviceThreadForwardImpl in HTTP thread
void QNetworkReplyHttpImplPrivate::wantUploadDataSlot(qint64 maxSize)
{
   Q_Q(QNetworkReplyHttpImpl);

   // call readPointer
   qint64 currentUploadDataLength = 0;
   char *data = const_cast<char *>(uploadByteDevice->readPointer(maxSize, currentUploadDataLength));

   if (currentUploadDataLength == 0) {
      uploadDeviceChoking = true;
      // No bytes from upload byte device. There will be bytes later, it will emit readyRead()
      // and our uploadByteDeviceReadyReadSlot() is called.
      return;
   } else {
      uploadDeviceChoking = false;
   }

   // Let's make a copy of this data
   QByteArray dataArray(data, currentUploadDataLength);

   // Communicate back to HTTP thread
   emit q->haveUploadData(uploadByteDevicePosition, dataArray, uploadByteDevice->atEnd(), uploadByteDevice->size());
}

void QNetworkReplyHttpImplPrivate::uploadByteDeviceReadyReadSlot()
{
   // Start the flow between this thread and the HTTP thread again by triggering a upload.
   // However only do this when we were choking before, else the state in
   // QNonContiguousByteDeviceThreadForwardImpl gets messed up.
   if (uploadDeviceChoking) {
      uploadDeviceChoking = false;
      wantUploadDataSlot(1024);
   }
}


/*
    A simple web page that can be used to test us: http://www.procata.com/cachetest/
 */
bool QNetworkReplyHttpImplPrivate::sendCacheContents(const QNetworkCacheMetaData &metaData)
{
   Q_Q(QNetworkReplyHttpImpl);

   setCachingEnabled(false);
   if (!metaData.isValid()) {
      return false;
   }

   QAbstractNetworkCache *nc = managerPrivate->networkCache;
   Q_ASSERT(nc);
   QIODevice *contents = nc->data(url);
   if (!contents) {
#if defined(QNETWORKACCESSHTTPBACKEND_DEBUG)
      qDebug() << "Can not send cache, the contents are 0" << url;
#endif
      return false;
   }
   contents->setParent(q);

   QNetworkCacheMetaData::AttributesMap attributes = metaData.attributes();
   int status = attributes.value(QNetworkRequest::HttpStatusCodeAttribute).toInt();
   if (status < 100) {
      status = 200;   // fake it
   }

   statusCode = status;

   q->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, status);
   q->setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, attributes.value(QNetworkRequest::HttpReasonPhraseAttribute));
   q->setAttribute(QNetworkRequest::SourceIsFromCacheAttribute, true);

   QNetworkCacheMetaData::RawHeaderList rawHeaders = metaData.rawHeaders();
   QNetworkCacheMetaData::RawHeaderList::const_iterator it = rawHeaders.constBegin(),
                                                       end = rawHeaders.constEnd();
   QUrl redirectUrl;
   for ( ; it != end; ++it) {
      if (httpRequest.isFollowRedirects() &&
            !qstricmp(it->first.toLower().constData(), "location")) {
         redirectUrl = QUrl::fromEncoded(it->second);
      }
      setRawHeader(it->first, it->second);
   }

   if (!isHttpRedirectResponse()) {
      checkForRedirect(status);
   }

   cacheLoadDevice = contents;
   q->connect(cacheLoadDevice, SIGNAL(readyRead()), SLOT(_q_cacheLoadReadyRead()));
   q->connect(cacheLoadDevice, SIGNAL(readChannelFinished()), SLOT(_q_cacheLoadReadyRead()));

   // This needs to be emitted in the event loop because it can be reached at
   // the direct code path of qnam.get(...) before the user has a chance
   // to connect any signals.
   QMetaObject::invokeMethod(q, "_q_metaDataChanged", Qt::QueuedConnection);
   QMetaObject::invokeMethod(q, "_q_cacheLoadReadyRead", Qt::QueuedConnection);


#if defined(QNETWORKACCESSHTTPBACKEND_DEBUG)
   qDebug() << "Successfully sent cache:" << url << contents->size() << "bytes";
#endif

   // Do redirect processing
   if (httpRequest.isFollowRedirects() && QHttpNetworkReply::isHttpRedirect(status)) {
      QMetaObject::invokeMethod(q, "onRedirected", Qt::QueuedConnection,
                                Q_ARG(QUrl, redirectUrl),
                                Q_ARG(int, status),
                                Q_ARG(int, httpRequest.redirectCount() - 1));
   }

   // Set the following flag so we can ignore some signals from HTTP thread
   // that would still come
   loadingFromCache = true;
   return true;
}

QNetworkCacheMetaData QNetworkReplyHttpImplPrivate::fetchCacheMetaData(const QNetworkCacheMetaData &oldMetaData) const
{
   Q_Q(const QNetworkReplyHttpImpl);

   QNetworkCacheMetaData metaData = oldMetaData;

   QNetworkHeadersPrivate cacheHeaders;
   cacheHeaders.setAllRawHeaders(metaData.rawHeaders());
   QNetworkHeadersPrivate::RawHeadersList::const_iterator it;

   QList<QByteArray> newHeaders = q->rawHeaderList();

   for (QByteArray header : newHeaders) {
      QByteArray originalHeader = header;
      header = header.toLower();

      bool hop_by_hop = (header == "connection" || header == "keep-alive" || header == "proxy-authenticate"
                         || header == "proxy-authorization"   || header == "te" || header == "trailers"
                         || header == "transfer-encoding"     || header ==  "upgrade");

      if (hop_by_hop) {
         continue;
      }

      if (header == "set-cookie") {
         continue;
      }

      // we were planning to not store the date header in the
      // cached resource; through that we planned to reduce the number
      // of writes to disk when using a QNetworkDiskCache (i.e. don't
      // write to disk when only the date changes).
      // However, without the date we cannot calculate the age of the page anymore.

      //if (header == "date")
      //continue;

      // Don't store Warning 1xx headers
      if (header == "warning") {
         QByteArray v = q->rawHeader(header);

         if (v.length() == 3
               && v[0] == '1'
               && v[1] >= '0' && v[1] <= '9'
               && v[2] >= '0' && v[2] <= '9') {
            continue;
         }
      }

      it = cacheHeaders.findRawHeader(header);
      if (it != cacheHeaders.rawHeaders.constEnd()) {
         // Match the behavior of Firefox and assume Cache-Control: "no-transform"
         if (header == "content-encoding"
               || header == "content-range"
               || header == "content-type") {
            continue;
         }

         // For MS servers that send "Content-Length: 0" on 304 responses
         // ignore this too
         if (header == "content-length") {
            continue;
         }
      }

#if defined(QNETWORKACCESSHTTPBACKEND_DEBUG)
      QByteArray n = q->rawHeader(header);
      QByteArray o;
      if (it != cacheHeaders.rawHeaders.constEnd()) {
         o = (*it).second;
      }
      if (n != o && header != "date") {
         qDebug() << "replacing" << header;
         qDebug() << "new" << n;
         qDebug() << "old" << o;
      }
#endif
      cacheHeaders.setRawHeader(originalHeader, q->rawHeader(header));
   }
   metaData.setRawHeaders(cacheHeaders.rawHeaders);

   bool checkExpired = true;

   QHash<QByteArray, QByteArray> cacheControl;
   it = cacheHeaders.findRawHeader("Cache-Control");
   if (it != cacheHeaders.rawHeaders.constEnd()) {
      cacheControl = parseHttpOptionHeader(it->second);
      QByteArray maxAge = cacheControl.value("max-age");
      if (!maxAge.isEmpty()) {
         checkExpired = false;
         QDateTime dt = QDateTime::currentDateTimeUtc();
         dt = dt.addSecs(maxAge.toInt());
         metaData.setExpirationDate(dt);
      }
   }
   if (checkExpired) {
      it = cacheHeaders.findRawHeader("expires");
      if (it != cacheHeaders.rawHeaders.constEnd()) {
         QDateTime expiredDateTime = QNetworkHeadersPrivate::fromHttpDate(it->second);
         metaData.setExpirationDate(expiredDateTime);
      }
   }

   it = cacheHeaders.findRawHeader("last-modified");
   if (it != cacheHeaders.rawHeaders.constEnd()) {
      metaData.setLastModified(QNetworkHeadersPrivate::fromHttpDate(it->second));
   }

   bool canDiskCache;
   // only cache GET replies by default, all other replies (POST, PUT, DELETE)
   //  are not cacheable by default (according to RFC 2616 section 9)
   if (httpRequest.operation() == QHttpNetworkRequest::Get) {

      canDiskCache = true;
      // 14.32
      // HTTP/1.1 caches SHOULD treat "Pragma: no-cache" as if the client
      // had sent "Cache-Control: no-cache".
      it = cacheHeaders.findRawHeader("pragma");
      if (it != cacheHeaders.rawHeaders.constEnd()
            && it->second == "no-cache") {
         canDiskCache = false;
      }

      // HTTP/1.1. Check the Cache-Control header
      if (cacheControl.contains("no-cache")) {
         canDiskCache = false;
      } else if (cacheControl.contains("no-store")) {
         canDiskCache = false;
      }

      // responses to POST might be cacheable
   } else if (httpRequest.operation() == QHttpNetworkRequest::Post) {

      canDiskCache = false;
      // some pages contain "expires:" and "cache-control: no-cache" field,
      // so we only might cache POST requests if we get "cache-control: max-age ..."
      if (cacheControl.contains("max-age")) {
         canDiskCache = true;
      }

      // responses to PUT and DELETE are not cacheable
   } else {
      canDiskCache = false;
   }

   metaData.setSaveToDisk(canDiskCache);
   QNetworkCacheMetaData::AttributesMap attributes;
   if (statusCode != 304) {
      // update the status code
      attributes.insert(QNetworkRequest::HttpStatusCodeAttribute, statusCode);
      attributes.insert(QNetworkRequest::HttpReasonPhraseAttribute, reasonPhrase);
   } else {
      // this is a redirection, keep the attributes intact
      attributes = oldMetaData.attributes();
   }
   metaData.setAttributes(attributes);
   return metaData;
}

bool QNetworkReplyHttpImplPrivate::canResume() const
{
   Q_Q(const QNetworkReplyHttpImpl);

   // Only GET operation supports resuming.
   if (operation != QNetworkAccessManager::GetOperation) {
      return false;
   }

   // Can only resume if server/resource supports Range header.
   QByteArray acceptRangesheaderName("Accept-Ranges");
   if (!q->hasRawHeader(acceptRangesheaderName) || q->rawHeader(acceptRangesheaderName) == "none") {
      return false;
   }

   // We only support resuming for byte ranges.
   if (request.hasRawHeader("Range")) {
      QByteArray range = request.rawHeader("Range");
      if (!range.startsWith("bytes=")) {
         return false;
      }
   }

   // If we're using a download buffer then we don't support resuming/migration
   // right now. Too much trouble.
   if (downloadZerocopyBuffer) {
      return false;
   }

   return true;
}

void QNetworkReplyHttpImplPrivate::setResumeOffset(quint64 offset)
{
   resumeOffset = offset;
}

/*!
    Starts the backend.  Returns \c true if the backend is started.  Returns \c false if the backend
    could not be started due to an unopened or roaming session.  The caller should recall this
    function once the session has been opened or the roaming process has finished.
*/
bool QNetworkReplyHttpImplPrivate::start(const QNetworkRequest &newHttpRequest)
{
#ifndef QT_NO_BEARERMANAGEMENT
   QSharedPointer<QNetworkSession> networkSession(managerPrivate->getNetworkSession());
   if (!networkSession) {
#endif
      postRequest(newHttpRequest);
      return true;
#ifndef QT_NO_BEARERMANAGEMENT
   }

   // This is not ideal.
   const QString host = url.host();
   if (host == QLatin1String("localhost") ||
         QHostAddress(host).isLoopback()) {
      // Don't need an open session for localhost access.
      postRequest(newHttpRequest);
      return true;
   }

   if (networkSession->isOpen() &&
         networkSession->state() == QNetworkSession::Connected) {
      Q_Q(QNetworkReplyHttpImpl);
      QObject::connect(networkSession.data(), SIGNAL(usagePoliciesChanged(QNetworkSession::UsagePolicies)),
                       q, SLOT(_q_networkSessionUsagePoliciesChanged(QNetworkSession::UsagePolicies)));
      postRequest(newHttpRequest);
      return true;
   } else if (synchronous) {
      // Command line applications using the synchronous path such as xmlpatterns may need an extra push.
      networkSession->open();
      if (networkSession->waitForOpened()) {
         postRequest(newHttpRequest);
         return true;
      }
   }
   return false;
#endif
}

void QNetworkReplyHttpImplPrivate::_q_startOperation()
{
   Q_Q(QNetworkReplyHttpImpl);

   // ensure this function is only being called once
   if (state == Working) {
      qDebug("QNetworkReplyImpl::_q_startOperation was called more than once");
      return;
   }
   state = Working;

#ifndef QT_NO_BEARERMANAGEMENT
   // Do not start background requests if they are not allowed by session policy
   QSharedPointer<QNetworkSession> session(manager->d_func()->getNetworkSession());
   QVariant isBackground = request.attribute(QNetworkRequest::BackgroundRequestAttribute, QVariant::fromValue(false));
   if (isBackground.toBool() && session && session->usagePolicies().testFlag(QNetworkSession::NoBackgroundTrafficPolicy)) {
      QMetaObject::invokeMethod(q, "_q_error", synchronous ? Qt::DirectConnection : Qt::QueuedConnection,
                                Q_ARG(QNetworkReply::NetworkError, QNetworkReply::BackgroundRequestNotAllowedError),
                                Q_ARG(QString, QCoreApplication::translate("QNetworkReply", "Background request not allowed.")));
      QMetaObject::invokeMethod(q, "_q_finished", synchronous ? Qt::DirectConnection : Qt::QueuedConnection);
      return;
   }

   if (!start(request)) {
      // backend failed to start because the session state is not Connected.
      // QNetworkAccessManager will call reply->backend->start() again for us when the session
      // state changes.
      state = WaitingForSession;

      if (session) {
         QObject::connect(session.data(), SIGNAL(error(QNetworkSession::SessionError)),
                          q, SLOT(_q_networkSessionFailed()), Qt::QueuedConnection);

         if (!session->isOpen()) {
            session->setSessionProperty("ConnectInBackground", isBackground);
            session->open();
         }
      } else {
         qWarning("Backend is waiting for QNetworkSession to connect, but there is none!");
         QMetaObject::invokeMethod(q, "_q_error", synchronous ? Qt::DirectConnection : Qt::QueuedConnection,
                                   Q_ARG(QNetworkReply::NetworkError, QNetworkReply::NetworkSessionFailedError),
                                   Q_ARG(QString, QCoreApplication::translate("QNetworkReply", "Network session error.")));
         QMetaObject::invokeMethod(q, "_q_finished", synchronous ? Qt::DirectConnection : Qt::QueuedConnection);
         return;
      }
   } else if (session) {
      QObject::connect(session.data(), SIGNAL(stateChanged(QNetworkSession::State)),
                       q, SLOT(_q_networkSessionStateChanged(QNetworkSession::State)),
                       Qt::QueuedConnection);
   }
#else
   if (!start(request)) {
      qWarning("Backend start failed");
      QMetaObject::invokeMethod(q, "_q_error", synchronous ? Qt::DirectConnection : Qt::QueuedConnection,
                                Q_ARG(QNetworkReply::NetworkError, QNetworkReply::UnknownNetworkError),
                                Q_ARG(QString, QCoreApplication::translate("QNetworkReply", "backend start error.")));
      QMetaObject::invokeMethod(q, "_q_finished", synchronous ? Qt::DirectConnection : Qt::QueuedConnection);
      return;
   }
#endif // QT_NO_BEARERMANAGEMENT

   if (synchronous) {
      state = Finished;
      q_func()->setFinished(true);
   }
}

void QNetworkReplyHttpImplPrivate::_q_cacheLoadReadyRead()
{
   Q_Q(QNetworkReplyHttpImpl);

   if (state != Working) {
      return;
   }
   if (!cacheLoadDevice || !q->isOpen() || !cacheLoadDevice->bytesAvailable()) {
      return;
   }

   // FIXME Optimize to use zerocopy download buffer if it is a QBuffer.
   // Needs to be done where sendCacheContents() (?) of HTTP is emitting
   // metaDataChanged ?


   QVariant totalSize = cookedHeaders.value(QNetworkRequest::ContentLengthHeader);

   // emit readyRead before downloadProgress incase this will cause events to be
   // processed and we get into a recursive call (as in QProgressDialog).

   if (!(isHttpRedirectResponse())) {
      // This readyRead() goes to the user. The user then may or may not read() anything.
      emit q->readyRead();

      if (downloadProgressSignalChoke.elapsed() >= progressSignalInterval) {
         downloadProgressSignalChoke.restart();
         emit q->downloadProgress(bytesDownloaded,
                                  totalSize.isNull() ? Q_INT64_C(-1) : totalSize.toLongLong());
      }
   }
   // If there are still bytes available in the cacheLoadDevice then the user did not read
   // in response to the readyRead() signal. This means we have to load from the cacheLoadDevice
   // and buffer that stuff. This is needed to be able to properly emit finished() later.
   while (cacheLoadDevice->bytesAvailable() && !isHttpRedirectResponse()) {
      downloadMultiBuffer.append(cacheLoadDevice->readAll());
   }

   if (cacheLoadDevice->isSequential()) {
      // check if end and we can read the EOF -1
      char c;
      qint64 actualCount = cacheLoadDevice->read(&c, 1);
      if (actualCount < 0) {
         cacheLoadDevice->deleteLater();
         cacheLoadDevice = 0;
         QMetaObject::invokeMethod(q, "_q_finished", Qt::QueuedConnection);
      } else if (actualCount == 1) {
         // This is most probably not happening since most QIODevice returned something proper for bytesAvailable()
         // and had already been "emptied".
         cacheLoadDevice->ungetChar(c);
      }
   } else if ((!cacheLoadDevice->isSequential() && cacheLoadDevice->atEnd())) {
      // This codepath is in case the cache device is a QBuffer, e.g. from QNetworkDiskCache.
      cacheLoadDevice->deleteLater();
      cacheLoadDevice = 0;
      QMetaObject::invokeMethod(q, "_q_finished", Qt::QueuedConnection);
   }
}


void QNetworkReplyHttpImplPrivate::_q_bufferOutgoingDataFinished()
{
   Q_Q(QNetworkReplyHttpImpl);

   // make sure this is only called once, ever.
   //_q_bufferOutgoingData may call it or the readChannelFinished emission
   if (state != Buffering) {
      return;
   }

   // disconnect signals
   QObject::disconnect(outgoingData, SIGNAL(readyRead()), q, SLOT(_q_bufferOutgoingData()));
   QObject::disconnect(outgoingData, SIGNAL(readChannelFinished()), q, SLOT(_q_bufferOutgoingDataFinished()));

   // finally, start the request
   QMetaObject::invokeMethod(q, "_q_startOperation", Qt::QueuedConnection);
}

void QNetworkReplyHttpImplPrivate::_q_cacheSaveDeviceAboutToClose()
{
   // do not keep a dangling pointer to the device around (device
   // is closing because e.g. QAbstractNetworkCache::remove() was called).
   cacheSaveDevice = 0;
}

void QNetworkReplyHttpImplPrivate::_q_bufferOutgoingData()
{
   Q_Q(QNetworkReplyHttpImpl);

   if (!outgoingDataBuffer) {
      // first call, create our buffer
      outgoingDataBuffer = QSharedPointer<QRingBuffer>::create();

      QObject::connect(outgoingData, SIGNAL(readyRead()), q, SLOT(_q_bufferOutgoingData()));
      QObject::connect(outgoingData, SIGNAL(readChannelFinished()), q, SLOT(_q_bufferOutgoingDataFinished()));
   }

   qint64 bytesBuffered = 0;
   qint64 bytesToBuffer = 0;

   // read data into our buffer
   forever {
      bytesToBuffer = outgoingData->bytesAvailable();
      // unknown? just try 2 kB, this also ensures we always try to read the EOF
      if (bytesToBuffer <= 0) {
         bytesToBuffer = 2 * 1024;
      }

      char *dst = outgoingDataBuffer->reserve(bytesToBuffer);
      bytesBuffered = outgoingData->read(dst, bytesToBuffer);

      if (bytesBuffered == -1) {
         // EOF has been reached.
         outgoingDataBuffer->chop(bytesToBuffer);

         _q_bufferOutgoingDataFinished();
         break;
      } else if (bytesBuffered == 0) {
         // nothing read right now, just wait until we get called again
         outgoingDataBuffer->chop(bytesToBuffer);

         break;
      } else {
         // don't break, try to read() again
         outgoingDataBuffer->chop(bytesToBuffer - bytesBuffered);
      }
   }
}

#ifndef QT_NO_BEARERMANAGEMENT
void QNetworkReplyHttpImplPrivate::_q_networkSessionConnected()
{
   Q_Q(QNetworkReplyHttpImpl);

   if (!manager) {
      return;
   }

   QSharedPointer<QNetworkSession> session = managerPrivate->getNetworkSession();
   if (!session) {
      return;
   }

   if (session->state() != QNetworkSession::Connected) {
      return;
   }

   switch (state) {
      case QNetworkReplyPrivate::Buffering:
      case QNetworkReplyPrivate::Working:
      case QNetworkReplyPrivate::Reconnecting:
         // Migrate existing downloads to new network connection.
         migrateBackend();
         break;
      case QNetworkReplyPrivate::WaitingForSession:
         // Start waiting requests.
         QMetaObject::invokeMethod(q, "_q_startOperation", Qt::QueuedConnection);
         break;
      default:
         ;
   }
}

void QNetworkReplyHttpImplPrivate::_q_networkSessionStateChanged(QNetworkSession::State sessionState)
{
   if (sessionState == QNetworkSession::Disconnected
         && state != Idle && state != Reconnecting) {
      error(QNetworkReplyImpl::NetworkSessionFailedError,
            QCoreApplication::translate("QNetworkReply", "Network session error."));
      finished();
   }
}

void QNetworkReplyHttpImplPrivate::_q_networkSessionFailed()
{
   // Abort waiting and working replies.
   if (state == WaitingForSession || state == Working) {
      state = Working;
      QSharedPointer<QNetworkSession> session(manager->d_func()->getNetworkSession());
      QString errorStr;
      if (session) {
         errorStr = session->errorString();
      } else {
         errorStr = QCoreApplication::translate("QNetworkReply", "Network session error.");
      }
      error(QNetworkReplyImpl::NetworkSessionFailedError, errorStr);
      finished();
   }
}

void QNetworkReplyHttpImplPrivate::_q_networkSessionUsagePoliciesChanged(QNetworkSession::UsagePolicies newPolicies)
{
   if (request.attribute(QNetworkRequest::BackgroundRequestAttribute).toBool()) {
      if (newPolicies & QNetworkSession::NoBackgroundTrafficPolicy) {
         // Abort waiting and working replies.
         if (state == WaitingForSession || state == Working) {
            state = Working;
            error(QNetworkReply::BackgroundRequestNotAllowedError,
                  QCoreApplication::translate("QNetworkReply", "Background request not allowed."));
            finished();
         }
         // ### if canResume(), then we could resume automatically
      }
   }

}
#endif


// need to have this function since the reply is a private member variable
// and the special backends need to access this.
void QNetworkReplyHttpImplPrivate::emitReplyUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
   Q_Q(QNetworkReplyHttpImpl);
   if (isFinished) {
      return;
   }

   if (!emitAllUploadProgressSignals) {
      //choke signal emissions, except the first and last signals which are unconditional
      if (uploadProgressSignalChoke.isValid()) {
         if (bytesSent != bytesTotal && uploadProgressSignalChoke.elapsed() < progressSignalInterval) {
            return;
         }
         uploadProgressSignalChoke.restart();
      } else {
         uploadProgressSignalChoke.start();
      }
   }

   emit q->uploadProgress(bytesSent, bytesTotal);
}

QNonContiguousByteDevice *QNetworkReplyHttpImplPrivate::createUploadByteDevice()
{
   Q_Q(QNetworkReplyHttpImpl);

   if (outgoingDataBuffer) {
      uploadByteDevice = QNonContiguousByteDeviceFactory::createShared(outgoingDataBuffer);
   } else if (outgoingData) {
      uploadByteDevice = QNonContiguousByteDeviceFactory::createShared(outgoingData);
   } else {
      return 0;
   }

   // We want signal emissions only for normal asynchronous uploads
   if (!synchronous)
      QObject::connect(uploadByteDevice.data(), SIGNAL(readProgress(qint64, qint64)),
                       q, SLOT(emitReplyUploadProgress(qint64, qint64)));

   return uploadByteDevice.data();
}

void QNetworkReplyHttpImplPrivate::_q_finished()
{
   // This gets called queued, just forward to real call then
   finished();
}

void QNetworkReplyHttpImplPrivate::finished()
{
   Q_Q(QNetworkReplyHttpImpl);

   if (state == Finished || state == Aborted || state == WaitingForSession) {
      return;
   }

   QVariant totalSize = cookedHeaders.value(QNetworkRequest::ContentLengthHeader);
   if (preMigrationDownloaded != Q_INT64_C(-1)) {
      totalSize = totalSize.toLongLong() + preMigrationDownloaded;
   }

   if (manager) {
#ifndef QT_NO_BEARERMANAGEMENT
      QSharedPointer<QNetworkSession> session = managerPrivate->getNetworkSession();
      if (session && session->state() == QNetworkSession::Roaming &&
            state == Working && errorCode != QNetworkReply::OperationCanceledError) {
         // only content with a known size will fail with a temporary network failure error
         if (!totalSize.isNull()) {
            if (bytesDownloaded != totalSize) {
               if (migrateBackend()) {
                  // either we are migrating or the request is finished/aborted
                  if (state == Reconnecting || state == WaitingForSession) {
                     return; // exit early if we are migrating.
                  }
               } else {
                  error(QNetworkReply::TemporaryNetworkFailureError,
                        QNetworkReply::tr("Temporary network failure."));
               }
            }
         }
      }
#endif
   }

   // if we don't know the total size of or we received everything save the cache
   if (totalSize.isNull() || totalSize == -1 || bytesDownloaded == totalSize) {
      completeCacheSave();
   }

   // We check for errorCode too as in case of SSL handshake failure, we still
   // get the HTTP redirect status code (301, 303 etc)
   if (isHttpRedirectResponse() && errorCode == QNetworkReply::NoError) {
      return;
   }

   state = Finished;
   q->setFinished(true);

   if (totalSize.isNull() || totalSize == -1) {
      emit q->downloadProgress(bytesDownloaded, bytesDownloaded);
   } else {
      emit q->downloadProgress(bytesDownloaded, totalSize.toLongLong());
   }

   if (bytesUploaded == -1 && (outgoingData || outgoingDataBuffer)) {
      emit q->uploadProgress(0, 0);
   }

   emit q->readChannelFinished();
   emit q->finished();
}

void QNetworkReplyHttpImplPrivate::_q_error(QNetworkReplyImpl::NetworkError code, const QString &errorMessage)
{
   this->error(code, errorMessage);
}


void QNetworkReplyHttpImplPrivate::error(QNetworkReplyImpl::NetworkError code, const QString &errorMessage)
{
   Q_Q(QNetworkReplyHttpImpl);
   // Can't set and emit multiple errors.
   if (errorCode != QNetworkReply::NoError) {
      qWarning("QNetworkReplyImplPrivate::error: Internal problem, this method must only be called once.");
      return;
   }

   errorCode = code;
   q->setErrorString(errorMessage);

   // note: might not be a good idea, since users could decide to delete us
   // which would delete the backend too...
   // maybe we should protect the backend
   emit q->error(code);
}

void QNetworkReplyHttpImplPrivate::_q_metaDataChanged()
{
   // FIXME merge this with replyDownloadMetaData(); ?

   Q_Q(QNetworkReplyHttpImpl);
   // 1. do we have cookies?
   // 2. are we allowed to set them?
   if (cookedHeaders.contains(QNetworkRequest::SetCookieHeader) && manager
         && (static_cast<QNetworkRequest::LoadControl>
             (request.attribute(QNetworkRequest::CookieSaveControlAttribute,
                                QNetworkRequest::Automatic).toInt()) == QNetworkRequest::Automatic)) {
      QList<QNetworkCookie> cookies =
         qvariant_cast<QList<QNetworkCookie> >(cookedHeaders.value(QNetworkRequest::SetCookieHeader));
      QNetworkCookieJar *jar = manager->cookieJar();
      if (jar) {
         jar->setCookiesFromUrl(cookies, url);
      }
   }
   emit q->metaDataChanged();
}

/*
    Migrates the backend of the QNetworkReply to a new network connection if required.  Returns
    true if the reply is migrated or it is not required; otherwise returns \c false.
*/
bool QNetworkReplyHttpImplPrivate::migrateBackend()
{
   Q_Q(QNetworkReplyHttpImpl);

   // Network reply is already finished or aborted, don't need to migrate.
   if (state == Finished || state == Aborted) {
      return true;
   }

   // Backend does not support resuming download.
   if (!canResume()) {
      return false;
   }

   // Request has outgoing data, not migrating.
   if (outgoingData) {
      return false;
   }

   // Request is serviced from the cache, don't need to migrate.
   if (cacheLoadDevice) {
      return true;
   }

   state = Reconnecting;

   cookedHeaders.clear();
   rawHeaders.clear();

   preMigrationDownloaded = bytesDownloaded;

   setResumeOffset(bytesDownloaded);

   emit q->abortHttpRequest();

   QMetaObject::invokeMethod(q, "_q_startOperation", Qt::QueuedConnection);

   return true;
}


void QNetworkReplyHttpImplPrivate::createCache()
{
   // check if we can save and if we're allowed to
   if (!managerPrivate->networkCache
         || !request.attribute(QNetworkRequest::CacheSaveControlAttribute, true).toBool()) {
      return;
   }
   cacheEnabled = true;
}

bool QNetworkReplyHttpImplPrivate::isCachingEnabled() const
{
   return (cacheEnabled && managerPrivate->networkCache != 0);
}

void QNetworkReplyHttpImplPrivate::setCachingEnabled(bool enable)
{
   if (!enable && !cacheEnabled) {
      return;   // nothing to do
   }
   if (enable && cacheEnabled) {
      return;   // nothing to do either!
   }

   if (enable) {
      if (bytesDownloaded) {
         qDebug() << "setCachingEnabled: " << bytesDownloaded << " bytesDownloaded";
         // refuse to enable in this case
         qCritical("QNetworkReplyImpl: backend error: caching was enabled after some bytes had been written");
         return;
      }

      createCache();
   } else {
      // someone told us to turn on, then back off?
      // ok... but you should make up your mind
      qDebug("QNetworkReplyImpl: setCachingEnabled(true) called after setCachingEnabled(false)");
      managerPrivate->networkCache->remove(url);
      cacheSaveDevice = 0;
      cacheEnabled = false;
   }
}

bool QNetworkReplyHttpImplPrivate::isCachingAllowed() const
{
   return operation == QNetworkAccessManager::GetOperation || operation == QNetworkAccessManager::HeadOperation;
}

void QNetworkReplyHttpImplPrivate::completeCacheSave()
{
   if (cacheEnabled && errorCode != QNetworkReplyImpl::NoError) {
      managerPrivate->networkCache->remove(url);
   } else if (cacheEnabled && cacheSaveDevice) {
      managerPrivate->networkCache->insert(cacheSaveDevice);
   }
   cacheSaveDevice = 0;
   cacheEnabled = false;
}


void QNetworkReplyHttpImpl::_q_startOperation()
{
   Q_D(QNetworkReplyHttpImpl);

   d->_q_startOperation();
}

bool QNetworkReplyHttpImpl::start(const QNetworkRequest &un_named_arg1)
{
   Q_D(QNetworkReplyHttpImpl);
   return d->start(un_named_arg1);
}

void QNetworkReplyHttpImpl::_q_cacheLoadReadyRead()
{
   Q_D(QNetworkReplyHttpImpl);
   d->_q_cacheLoadReadyRead();
}

void QNetworkReplyHttpImpl::_q_bufferOutgoingData()
{
   Q_D(QNetworkReplyHttpImpl);
   d->_q_bufferOutgoingData();
}

void QNetworkReplyHttpImpl::_q_bufferOutgoingDataFinished()
{
   Q_D(QNetworkReplyHttpImpl);
   d->_q_bufferOutgoingDataFinished();
}

#ifndef QT_NO_BEARERMANAGEMENT

void QNetworkReplyHttpImpl::_q_networkSessionConnected()
{
   Q_D(QNetworkReplyHttpImpl);
   d->_q_networkSessionConnected();
}

void QNetworkReplyHttpImpl::_q_networkSessionFailed()
{
   Q_D(QNetworkReplyHttpImpl);
   d->_q_networkSessionFailed();
}

void QNetworkReplyHttpImpl::_q_networkSessionStateChanged(QNetworkSession::State un_named_arg1)
{
   Q_D(QNetworkReplyHttpImpl);
   d->_q_networkSessionStateChanged(un_named_arg1);
}

void QNetworkReplyHttpImpl::_q_networkSessionUsagePoliciesChanged(QNetworkSession::UsagePolicies un_named_arg1)
{
   Q_D(QNetworkReplyHttpImpl);
   d->_q_networkSessionUsagePoliciesChanged(un_named_arg1);
}

#endif


void QNetworkReplyHttpImpl::_q_finished()
{
   Q_D(QNetworkReplyHttpImpl);
   d->_q_finished();
}

void QNetworkReplyHttpImpl::_q_error(QNetworkReply::NetworkError un_named_arg1, const QString &un_named_arg2)
{
   Q_D(QNetworkReplyHttpImpl);
   d->_q_error(un_named_arg1, un_named_arg2);
}

void QNetworkReplyHttpImpl::replyDownloadData(QByteArray un_named_arg1)
{
   Q_D(QNetworkReplyHttpImpl);
   d->replyDownloadData(un_named_arg1);
}

void QNetworkReplyHttpImpl::replyFinished()
{
   Q_D(QNetworkReplyHttpImpl);
   d->replyFinished();
}

void QNetworkReplyHttpImpl::replyDownloadMetaData(QList <QPair <QByteArray, QByteArray>> un_named_arg1,
      int un_named_arg2, QString un_named_arg3, bool un_named_arg4, QSharedPointer <char> un_named_arg5,
      qint64 un_named_arg6, bool un_named_arg7)
{
   Q_D(QNetworkReplyHttpImpl);
   d->replyDownloadMetaData(un_named_arg1, un_named_arg2, un_named_arg3, un_named_arg4, un_named_arg5, un_named_arg6,
                            un_named_arg7);
}

void QNetworkReplyHttpImpl::replyDownloadProgressSlot(qint64 un_named_arg1, qint64 un_named_arg2)
{
   Q_D(QNetworkReplyHttpImpl);
   d->replyDownloadProgressSlot(un_named_arg1, un_named_arg2);
}

void QNetworkReplyHttpImpl::httpAuthenticationRequired(QHttpNetworkRequest un_named_arg1, QAuthenticator *un_named_arg2)
{
   Q_D(QNetworkReplyHttpImpl);
   d->httpAuthenticationRequired(un_named_arg1, un_named_arg2);
}

void QNetworkReplyHttpImpl::httpError(QNetworkReply::NetworkError un_named_arg1, QString un_named_arg2)
{
   Q_D(QNetworkReplyHttpImpl);
   d->httpError(un_named_arg1, un_named_arg2);
}

#ifdef QT_SSL

void QNetworkReplyHttpImpl::replyEncrypted()
{
   Q_D(QNetworkReplyHttpImpl);
   d->replyEncrypted();
}

void QNetworkReplyHttpImpl::replySslErrors(const QList <QSslError> &un_named_arg1, bool *un_named_arg2, QList <QSslError> *un_named_arg3)
{
   Q_D(QNetworkReplyHttpImpl);
   d->replySslErrors(un_named_arg1, un_named_arg2, un_named_arg3);
}

void QNetworkReplyHttpImpl::replySslConfigurationChanged(QSslConfiguration un_named_arg1)
{
   Q_D(QNetworkReplyHttpImpl);
   d->replySslConfigurationChanged(un_named_arg1);
}

void QNetworkReplyHttpImpl::replyPreSharedKeyAuthenticationRequiredSlot(QSslPreSharedKeyAuthenticator *un_named_arg1)
{
   Q_D(QNetworkReplyHttpImpl);
   d->replyPreSharedKeyAuthenticationRequiredSlot(un_named_arg1);
}

#endif


#ifndef QT_NO_NETWORKPROXY
void QNetworkReplyHttpImpl::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth)
{
   Q_D(QNetworkReplyHttpImpl);
   d->proxyAuthenticationRequired(proxy, auth);
}
#endif

void QNetworkReplyHttpImpl::resetUploadDataSlot(bool *r)
{
   Q_D(QNetworkReplyHttpImpl);
   d->resetUploadDataSlot(r);
}

void QNetworkReplyHttpImpl::wantUploadDataSlot(qint64 un_named_arg1)
{
   Q_D(QNetworkReplyHttpImpl);
   d->wantUploadDataSlot(un_named_arg1);
}

void QNetworkReplyHttpImpl::sentUploadDataSlot(qint64 un_named_arg1, qint64 un_named_arg2)
{
   Q_D(QNetworkReplyHttpImpl);
   d->sentUploadDataSlot(un_named_arg1, un_named_arg2);
}

void QNetworkReplyHttpImpl::uploadByteDeviceReadyReadSlot()
{
   Q_D(QNetworkReplyHttpImpl);
   d->uploadByteDeviceReadyReadSlot();
}

void QNetworkReplyHttpImpl::emitReplyUploadProgress(qint64 un_named_arg1, qint64 un_named_arg2)
{
   Q_D(QNetworkReplyHttpImpl);
   d->emitReplyUploadProgress(un_named_arg1, un_named_arg2);
}

void QNetworkReplyHttpImpl::_q_cacheSaveDeviceAboutToClose()
{
   Q_D(QNetworkReplyHttpImpl);
   d->_q_cacheSaveDeviceAboutToClose();
}

void QNetworkReplyHttpImpl::_q_metaDataChanged()
{
   Q_D(QNetworkReplyHttpImpl);
   d->_q_metaDataChanged();
}

void QNetworkReplyHttpImpl::onRedirected(QUrl un_named_arg1, int un_named_arg2, int un_named_arg3)
{
   Q_D(QNetworkReplyHttpImpl);
   d->onRedirected(un_named_arg1, un_named_arg2, un_named_arg3);
}



