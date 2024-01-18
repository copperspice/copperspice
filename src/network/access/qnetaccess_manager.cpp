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

#include <qnetaccess_manager.h>
#include <qnetaccess_manager_p.h>

#include <qalgorithms.h>
#include <qabstract_networkcache.h>
#include <qbuffer.h>
#include <qhttp_multipart.h>
#include <qnetwork_request.h>
#include <qnetwork_reply.h>
#include <qnetwork_cookie.h>
#include <qnetwork_cookiejar.h>
#include <qnetworksession.h>
#include <qnetworkconfigmanager.h>
#include <qsslconfiguration.h>
#include <qthread.h>
#include <qurl.h>
#include <qvector.h>

#include <qauthenticator_p.h>
#include <qsharednetworksession_p.h>
#include <qnetaccess_ftpbackend_p.h>
#include <qnetaccess_filebackend_p.h>
#include <qnetaccess_cachebackend_p.h>
#include <qnetwork_replydata_p.h>
#include <qnetwork_replyfile_p.h>
#include <qnetwork_replyhttp_p.h>
#include <qnetwork_reply_p.h>
#include <qhttp_multipart_p.h>

QNetworkAccessFileBackendFactory *cs_FileBackend()
{
   static QNetworkAccessFileBackendFactory retval;
   return &retval;
}

#ifndef QT_NO_FTP
QNetworkAccessFtpBackendFactory *cs_FtpBackend()
{
   static QNetworkAccessFtpBackendFactory retval;
   return &retval;
}
#endif

#if defined(Q_OS_DARWIN)

#include <CoreServices/CoreServices.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <Security/SecKeychain.h>

bool getProxyAuth(const QString &proxyHostname, const QString &scheme, QString &username, QString &password)
{
   OSStatus err;
   SecKeychainItemRef itemRef;
   bool retValue = false;
   SecProtocolType protocolType = kSecProtocolTypeAny;

   if (scheme.compare("ftp", Qt::CaseInsensitive) == 0) {
      protocolType = kSecProtocolTypeFTPProxy;

   } else if (scheme.compare("http", Qt::CaseInsensitive) == 0
              || scheme.compare("preconnect-http", Qt::CaseInsensitive) == 0) {
      protocolType = kSecProtocolTypeHTTPProxy;

   } else if (scheme.compare("https", Qt::CaseInsensitive) == 0
              || scheme.compare("preconnect-https", Qt::CaseInsensitive) == 0) {
      protocolType = kSecProtocolTypeHTTPSProxy;
   }

   QByteArray proxyHostnameUtf8(proxyHostname.toUtf8());
   err = SecKeychainFindInternetPassword(nullptr, proxyHostnameUtf8.length(), proxyHostnameUtf8.constData(),
                 0, nullptr, 0, nullptr, 0, nullptr, 0, protocolType, kSecAuthenticationTypeAny, nullptr, nullptr, &itemRef);

   if (err == noErr) {

      SecKeychainAttribute attr;
      SecKeychainAttributeList attrList;
      UInt32 length;
      void *outData;

      attr.tag = kSecAccountItemAttr;
      attr.length = 0;
      attr.data = nullptr;

      attrList.count = 1;
      attrList.attr = &attr;

      if (SecKeychainItemCopyContent(itemRef, nullptr, &attrList, &length, &outData) == noErr) {
         username = QString::fromUtf8((const char *)attr.data, attr.length);
         password = QString::fromUtf8((const char *)outData, length);
         SecKeychainItemFreeContent(&attrList, outData);
         retValue = true;
      }

      CFRelease(itemRef);
   }

   return retValue;
}
#endif

static void ensureInitialized()
{

#ifndef QT_NO_FTP
   (void) cs_FtpBackend();
#endif

   // leave this one last since it will query the special QAbstractFileEngines
   (void) cs_FileBackend();
}

QNetworkAccessManager::QNetworkAccessManager(QObject *parent)
   : QObject(parent), d_ptr(new QNetworkAccessManagerPrivate)
{
   d_ptr->q_ptr = this;
   ensureInitialized();

#ifndef QT_NO_BEARERMANAGEMENT
   Q_D(QNetworkAccessManager);

   // if a session is required, we track online state through
   // the QNetworkSession's signals if a request is already made.
   // we need to track current accessibility state by default

   connect(&d->networkConfigurationManager, &QNetworkConfigurationManager::onlineStateChanged,   this,
         &QNetworkAccessManager::_q_onlineStateChanged);

   connect(&d->networkConfigurationManager, &QNetworkConfigurationManager::configurationChanged, this,
         &QNetworkAccessManager::_q_configurationChanged);

#endif
}

/*!
    Destroys the QNetworkAccessManager object and frees up any
    resources. Note that QNetworkReply objects that are returned from
    this class have this object set as their parents, which means that
    they will be deleted along with it if you don't call
    QObject::setParent() on them.
*/
QNetworkAccessManager::~QNetworkAccessManager()
{

#ifndef QT_NO_NETWORKPROXY
   delete d_func()->proxyFactory;
#endif

   // Delete the QNetworkReply children first.
   // Else a QAbstractNetworkCache might get deleted in ~QObject
   // before a QNetworkReply that accesses the QAbstractNetworkCache
   // object in its destructor.

   qDeleteAll(findChildren<QNetworkReply *>());

   // The other children will be deleted in this ~QObject
   // FIXME instead of using this code. make the QNetworkReplyImpl
   // properly watch the cache deletion, e.g. via a QWeakPointer.
}

#ifndef QT_NO_NETWORKPROXY

QNetworkProxy QNetworkAccessManager::proxy() const
{
   return d_func()->proxy;
}

void QNetworkAccessManager::setProxy(const QNetworkProxy &proxy)
{
   Q_D(QNetworkAccessManager);

   delete d->proxyFactory;
   d->proxy = proxy;
   d->proxyFactory = nullptr;
}

QNetworkProxyFactory *QNetworkAccessManager::proxyFactory() const
{
   return d_func()->proxyFactory;
}

void QNetworkAccessManager::setProxyFactory(QNetworkProxyFactory *factory)
{
   Q_D(QNetworkAccessManager);
   delete d->proxyFactory;
   d->proxyFactory = factory;
   d->proxy = QNetworkProxy();
}
#endif

QAbstractNetworkCache *QNetworkAccessManager::cache() const
{
   Q_D(const QNetworkAccessManager);
   return d->networkCache;
}

void QNetworkAccessManager::setCache(QAbstractNetworkCache *cache)
{
   Q_D(QNetworkAccessManager);
   if (d->networkCache != cache) {
      delete d->networkCache;
      d->networkCache = cache;
      if (d->networkCache) {
         d->networkCache->setParent(this);
      }
   }
}

QNetworkCookieJar *QNetworkAccessManager::cookieJar() const
{
   Q_D(const QNetworkAccessManager);
   if (!d->cookieJar) {
      d->createCookieJar();
   }
   return d->cookieJar;
}

void QNetworkAccessManager::setCookieJar(QNetworkCookieJar *cookieJar)
{
   Q_D(QNetworkAccessManager);
   d->cookieJarCreated = true;
   if (d->cookieJar != cookieJar) {
      if (d->cookieJar && d->cookieJar->parent() == this) {
         delete d->cookieJar;
      }
      d->cookieJar = cookieJar;
      if (thread() == cookieJar->thread()) {
         d->cookieJar->setParent(this);
      }
   }
}

QNetworkReply *QNetworkAccessManager::head(const QNetworkRequest &request)
{
   return d_func()->postProcess(createRequest(QNetworkAccessManager::HeadOperation, request));
}

QNetworkReply *QNetworkAccessManager::get(const QNetworkRequest &request)
{
   return d_func()->postProcess(createRequest(QNetworkAccessManager::GetOperation, request));
}

QNetworkReply *QNetworkAccessManager::post(const QNetworkRequest &request, QIODevice *data)
{
   return d_func()->postProcess(createRequest(QNetworkAccessManager::PostOperation, request, data));
}

QNetworkReply *QNetworkAccessManager::post(const QNetworkRequest &request, const QByteArray &data)
{
   QBuffer *buffer = new QBuffer;
   buffer->setData(data);
   buffer->open(QIODevice::ReadOnly);

   QNetworkReply *reply = post(request, buffer);
   buffer->setParent(reply);
   return reply;
}

QNetworkReply *QNetworkAccessManager::post(const QNetworkRequest &request, QHttpMultiPart *multiPart)
{
   QNetworkRequest newRequest = d_func()->prepareMultipart(request, multiPart);
   QIODevice *device = multiPart->d_func()->device;
   QNetworkReply *reply = post(newRequest, device);
   return reply;
}

QNetworkReply *QNetworkAccessManager::put(const QNetworkRequest &request, QHttpMultiPart *multiPart)
{
   QNetworkRequest newRequest = d_func()->prepareMultipart(request, multiPart);
   QIODevice *device = multiPart->d_func()->device;
   QNetworkReply *reply = put(newRequest, device);
   return reply;
}

QNetworkReply *QNetworkAccessManager::put(const QNetworkRequest &request, QIODevice *data)
{
   return d_func()->postProcess(createRequest(QNetworkAccessManager::PutOperation, request, data));
}

QNetworkReply *QNetworkAccessManager::put(const QNetworkRequest &request, const QByteArray &data)
{
   QBuffer *buffer = new QBuffer;
   buffer->setData(data);
   buffer->open(QIODevice::ReadOnly);

   QNetworkReply *reply = put(request, buffer);
   buffer->setParent(reply);
   return reply;
}

QNetworkReply *QNetworkAccessManager::deleteResource(const QNetworkRequest &request)
{
   return d_func()->postProcess(createRequest(QNetworkAccessManager::DeleteOperation, request));
}

#ifndef QT_NO_BEARERMANAGEMENT

void QNetworkAccessManager::setConfiguration(const QNetworkConfiguration &config)
{
   Q_D(QNetworkAccessManager);
   d->networkConfiguration = config;
   d->customNetworkConfiguration = true;
   d->createSession(config);
}

QNetworkConfiguration QNetworkAccessManager::configuration() const
{
   Q_D(const QNetworkAccessManager);

   QSharedPointer<QNetworkSession> session(d->getNetworkSession());
   if (session) {
      return session->configuration();
   } else {
      QNetworkConfigurationManager manager;
      return manager.defaultConfiguration();
   }
}

QNetworkConfiguration QNetworkAccessManager::activeConfiguration() const
{
   Q_D(const QNetworkAccessManager);

   QSharedPointer<QNetworkSession> networkSession(d->getNetworkSession());
   QNetworkConfigurationManager manager;

   if (networkSession) {
      return manager.configurationFromIdentifier(
                networkSession->sessionProperty(QString("ActiveConfiguration")).toString());
   } else {
      return manager.defaultConfiguration();
   }
}

void QNetworkAccessManager::setNetworkAccessible(QNetworkAccessManager::NetworkAccessibility accessible)
{
   Q_D(QNetworkAccessManager);

   d->defaultAccessControl = accessible == NotAccessible ? false : true;

   if (d->networkAccessible != accessible) {
      NetworkAccessibility previous = networkAccessible();
      d->networkAccessible = accessible;
      NetworkAccessibility current = networkAccessible();
      if (previous != current) {
         emit networkAccessibleChanged(current);
      }
   }
}

QNetworkAccessManager::NetworkAccessibility QNetworkAccessManager::networkAccessible() const
{
   Q_D(const QNetworkAccessManager);

   if (d->networkConfiguration.state().testFlag(QNetworkConfiguration::Undefined)) {
      return UnknownAccessibility;
   }

   if (d->networkSessionRequired) {
      QSharedPointer<QNetworkSession> networkSession(d->getNetworkSession());
      if (networkSession) {
         // d->online holds online/offline state of this network session.
         if (d->online) {
            return d->networkAccessible;
         } else {
            return NotAccessible;
         }
      } else {
         if (d->defaultAccessControl) {
            if (d->online) {
               return d->networkAccessible;
            } else {
               return NotAccessible;
            }
         }
         return (d->networkAccessible);
      }
   } else {
      if (d->online) {
         return d->networkAccessible;
      } else {
         return NotAccessible;
      }
   }
}

// internal
const QWeakPointer<const QNetworkSession> QNetworkAccessManagerPrivate::getNetworkSession(const QNetworkAccessManager *q)
{
   return q->d_func()->networkSessionWeakRef;
}

QSharedPointer<QNetworkSession> QNetworkAccessManagerPrivate::getNetworkSession() const
{
   if (networkSessionStrongRef) {
      return networkSessionStrongRef;
   }
   return networkSessionWeakRef.toStrongRef();
}

#endif // QT_NO_BEARERMANAGEMENT


#ifdef QT_SSL

void QNetworkAccessManager::connectToHostEncrypted(const QString &hostName, quint16 port,
      const QSslConfiguration &sslConfiguration)
{
   QUrl url;
   url.setHost(hostName);
   url.setPort(port);
   url.setScheme("preconnect-https");

   QNetworkRequest request(url);

   if (sslConfiguration != QSslConfiguration::defaultConfiguration()) {
      request.setSslConfiguration(sslConfiguration);
   }

   // There is no way to enable SPDY via a request, so we need to check
   // the ssl configuration whether SPDY is allowed here.
   if (sslConfiguration.allowedNextProtocols().contains(QSslConfiguration::NextProtocolSpdy3_0)) {
      request.setAttribute(QNetworkRequest::SpdyAllowedAttribute, true);
   }

   get(request);
}

#endif

void QNetworkAccessManager::connectToHost(const QString &hostName, quint16 port)
{
   QUrl url;
   url.setHost(hostName);
   url.setPort(port);
   url.setScheme("preconnect-http");
   QNetworkRequest request(url);
   get(request);
}

QNetworkReply *QNetworkAccessManager::sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QIODevice *data)
{
   QNetworkRequest newRequest(request);
   newRequest.setAttribute(QNetworkRequest::CustomVerbAttribute, verb);
   return d_func()->postProcess(createRequest(QNetworkAccessManager::CustomOperation, newRequest, data));
}

QNetworkReply *QNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op,
      const QNetworkRequest &req, QIODevice *outgoingData)
{
   Q_D(QNetworkAccessManager);

   bool isLocalFile = req.url().isLocalFile();
   QString scheme = req.url().scheme();

   // fast path for GET on file:// URLs
   // The QNetworkAccessFileBackend will right now only be used for PUT

   if (op == QNetworkAccessManager::GetOperation || op == QNetworkAccessManager::HeadOperation) {

      if (isLocalFile || scheme == "qrc") {
         return new QNetworkReplyFileImpl(this, req, op);
      }

      if (scheme == "data") {
         return new QNetworkReplyDataImpl(this, req, op);
      }

      // A request with QNetworkRequest::AlwaysCache does not need any bearer management
      QNetworkRequest::CacheLoadControl mode = static_cast<QNetworkRequest::CacheLoadControl>(
               req.attribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork).toInt());

      if (mode == QNetworkRequest::AlwaysCache) {
         // FIXME Implement a QNetworkReplyCacheImpl instead, see QTBUG-15106
         QNetworkReplyImpl *reply = new QNetworkReplyImpl(this);
         QNetworkReplyImplPrivate *priv = reply->d_func();

         priv->manager = this;
         priv->backend = new QNetworkAccessCacheBackend();
         priv->backend->manager = this->d_func();
         priv->backend->setParent(reply);
         priv->backend->reply = priv;
         priv->setup(op, req, outgoingData);
         return reply;
      }
   }

#ifndef QT_NO_BEARERMANAGEMENT
   // Return a disabled network reply if network access is disabled.
   // Except if the scheme is empty or file://.
   if (d->networkAccessible == NotAccessible && !isLocalFile) {
      return new QDisabledNetworkReply(this, req, op);
   }

   if (!d->networkSessionStrongRef && (d->initializeSession || !d->networkConfiguration.identifier().isEmpty())) {
      QNetworkConfigurationManager manager;
      if (! d->networkConfiguration.identifier().isEmpty()) {
         if ((d->networkConfiguration.state() & QNetworkConfiguration::Defined)
               && d->networkConfiguration != manager.defaultConfiguration()) {
            d->createSession(manager.defaultConfiguration());
         } else {
            d->createSession(d->networkConfiguration);
         }

      } else {
         if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
            d->createSession(manager.defaultConfiguration());
         } else {
            d->initializeSession = false;
         }
      }
   }
#endif

   QNetworkRequest request = req;
   if (! request.header(QNetworkRequest::ContentLengthHeader).isValid() && outgoingData && !outgoingData->isSequential()) {
      // request has no Content-Length
      // but the data that is outgoing is random-access
      request.setHeader(QNetworkRequest::ContentLengthHeader, outgoingData->size());
   }

   if (static_cast<QNetworkRequest::LoadControl>
         (request.attribute(QNetworkRequest::CookieLoadControlAttribute,
                            QNetworkRequest::Automatic).toInt()) == QNetworkRequest::Automatic) {
      if (d->cookieJar) {
         QList<QNetworkCookie> cookies = d->cookieJar->cookiesForUrl(request.url());

         if (! cookies.isEmpty()) {
            request.setHeader(QNetworkRequest::CookieHeader, QVariant::fromValue(cookies));
         }
      }
   }

   // use the new QNetworkReplyHttpImpl
   if (scheme == "http" || scheme == "preconnect-http"

#ifdef QT_SSL
         || scheme == "https" || scheme == "preconnect-https"
#endif

      ) {

      QNetworkReplyHttpImpl *reply = new QNetworkReplyHttpImpl(this, request, op, outgoingData);

#ifndef QT_NO_BEARERMANAGEMENT
      connect(this, &QNetworkAccessManager::networkSessionConnected, reply, &QNetworkReplyHttpImpl::_q_networkSessionConnected);
#endif

      return reply;
   }

   // first step: create the reply
   QNetworkReplyImpl *reply = new QNetworkReplyImpl(this);

#ifndef QT_NO_BEARERMANAGEMENT
   if (! isLocalFile) {
      connect(this, &QNetworkAccessManager::networkSessionConnected, reply, &QNetworkReplyImpl::_q_networkSessionConnected);
   }
#endif

   QNetworkReplyImplPrivate *priv = reply->d_func();
   priv->manager = this;

   // second step: fetch cached credentials
   // This is not done for the time being, we should use signal emissions to request
   // the credentials from cache.

   // third step: find a backend
   priv->backend = d->findBackend(op, request);

   if (priv->backend) {
      priv->backend->setParent(reply);
      priv->backend->reply = priv;
   }

#ifdef QT_SSL
   reply->setSslConfiguration(request.sslConfiguration());
#endif

   // fourth step: setup the reply
   priv->setup(op, request, outgoingData);

   return reply;
}

QStringList QNetworkAccessManager::supportedSchemes() const
{
   QStringList schemes;
   QNetworkAccessManager *self = const_cast<QNetworkAccessManager *>(this); // We know we call a const slot

   QMetaObject::invokeMethod(self, "supportedSchemesImplementation", Qt::DirectConnection,
                             Q_RETURN_ARG(QStringList, schemes));

   schemes.removeDuplicates();
   return schemes;
}

QStringList QNetworkAccessManager::supportedSchemesImplementation() const
{
   Q_D(const QNetworkAccessManager);

   QStringList schemes = d->backendSupportedSchemes();

   // Those ones don't exist in backends
   schemes << "http";

#ifdef QT_SSL
   if (QSslSocket::supportsSsl()) {
      schemes << "https";
   }
#endif

   schemes << "data";
   return schemes;
}

void QNetworkAccessManager::clearAccessCache()
{
   QNetworkAccessManagerPrivate::clearCache(this);
}

void QNetworkAccessManagerPrivate::_q_replyFinished()
{
   Q_Q(QNetworkAccessManager);

   QNetworkReply *reply = dynamic_cast<QNetworkReply *>(q->sender());
   if (reply) {
      emit q->finished(reply);
   }

#ifndef QT_NO_BEARERMANAGEMENT
   // If there are no active requests, release our reference to the network session.
   // It will not be destroyed immediately, but rather when the connection cache is flushed
   // after 2 minutes.
   activeReplyCount--;

   if (networkSessionStrongRef && activeReplyCount == 0) {
      networkSessionStrongRef.clear();
   }
#endif
}

void QNetworkAccessManagerPrivate::_q_replyEncrypted()
{
#ifdef QT_SSL
   Q_Q(QNetworkAccessManager);

   QNetworkReply *reply = dynamic_cast<QNetworkReply *>(q->sender());
   if (reply) {
      emit q->encrypted(reply);
   }
#endif
}

void QNetworkAccessManagerPrivate::_q_replySslErrors(const QList<QSslError> &errorList)
{
#ifdef QT_SSL
   Q_Q(QNetworkAccessManager);

   QNetworkReply *reply = dynamic_cast<QNetworkReply *>(q->sender());
   if (reply) {
      emit q->sslErrors(reply, errorList);
   }

#else
   (void) errorList;

#endif
}

void QNetworkAccessManagerPrivate::_q_replyPreSharedKeyAuthenticationRequired(
      QSslPreSharedKeyAuthenticator *authenticator)
{
#ifdef QT_SSL
   Q_Q(QNetworkAccessManager);

   QNetworkReply *reply = dynamic_cast<QNetworkReply *>(q->sender());

   if (reply) {
      emit q->preSharedKeyAuthenticationRequired(reply, authenticator);
   }

#else
   (void) authenticator;

#endif
}

QNetworkReply *QNetworkAccessManagerPrivate::postProcess(QNetworkReply *reply)
{
   Q_Q(QNetworkAccessManager);

   QNetworkReplyPrivate::setManager(reply, q);
   q->connect(reply, &QNetworkReply::finished,   q, &QNetworkAccessManager::_q_replyFinished);

#ifdef QT_SSL
   // In case we are compiled without SSL support, we do not have this signal and we need to
   // avoid getting a connection error.

   q->connect(reply, &QNetworkReply::encrypted, q, &QNetworkAccessManager::_q_replyEncrypted);
   q->connect(reply, &QNetworkReply::sslErrors, q, &QNetworkAccessManager::_q_replySslErrors);

   q->connect(reply, &QNetworkReply::preSharedKeyAuthenticationRequired, q,
      &QNetworkAccessManager::_q_replyPreSharedKeyAuthenticationRequired);
#endif

#ifndef QT_NO_BEARERMANAGEMENT
   activeReplyCount++;
#endif

   return reply;
}

void QNetworkAccessManagerPrivate::createCookieJar() const
{
   if (! cookieJarCreated) {
      // not great code, use anyway
      QNetworkAccessManagerPrivate *that = const_cast<QNetworkAccessManagerPrivate *>(this);
      that->cookieJar = new QNetworkCookieJar(that->q_func());
      that->cookieJarCreated = true;
   }
}

void QNetworkAccessManagerPrivate::authenticationRequired(QAuthenticator *authenticator,
      QNetworkReply *reply, bool synchronous, QUrl &url, QUrl *urlForLastAuthentication,
      bool allowAuthenticationReuse)
{
   Q_Q(QNetworkAccessManager);

   // don't try the cache for the same URL twice in a row
   // being called twice for the same URL means the authentication failed
   // also called when last URL is empty, e.g. on first call

   if (allowAuthenticationReuse && (urlForLastAuthentication->isEmpty()
            || url != *urlForLastAuthentication)) {

      // if credentials are included in the url, then use them
      if (! url.userName().isEmpty() && !url.password().isEmpty()) {
         authenticator->setUser(url.userName(QUrl::FullyDecoded));
         authenticator->setPassword(url.password(QUrl::FullyDecoded));
         *urlForLastAuthentication = url;
         authenticationManager->cacheCredentials(url, authenticator);

         return;
      }

      QNetworkAuthenticationCredential cred = authenticationManager->fetchCachedCredentials(url, authenticator);
      if (! cred.isNull()) {
         authenticator->setUser(cred.user);
         authenticator->setPassword(cred.password);
         *urlForLastAuthentication = url;

         return;
      }
   }

   // if we emit a signal here in synchronous mode, the user might spin
   // an event loop, which might recurse and lead to problems
   if (synchronous) {
      return;
   }

   *urlForLastAuthentication = url;
   emit q->authenticationRequired(reply, authenticator);

   if (allowAuthenticationReuse) {
      authenticationManager->cacheCredentials(url, authenticator);
   }
}

#ifndef QT_NO_NETWORKPROXY
void QNetworkAccessManagerPrivate::proxyAuthenticationRequired(const QUrl &url, const QNetworkProxy &proxy,
      bool synchronous, QAuthenticator *authenticator, QNetworkProxy *lastProxyAuthentication)
{
   Q_Q(QNetworkAccessManager);
   QAuthenticatorPrivate *priv = QAuthenticatorPrivate::getPrivate(*authenticator);

   if (proxy != *lastProxyAuthentication && (!priv || !priv->hasFailed)) {
      QNetworkAuthenticationCredential cred = authenticationManager->fetchCachedProxyCredentials(proxy);

      if (!cred.isNull()) {
         authenticator->setUser(cred.user);
         authenticator->setPassword(cred.password);
         return;
      }
   }

#if defined(Q_OS_DARWIN)
   //now we try to get the username and password from keychain if not successful signal will be emitted
   QString username;
   QString password;

   if (getProxyAuth(proxy.hostName(), url.scheme(), username, password)) {
      // only cache the system credentials if they are correct (or if they have changed)
      // to not run into an endless loop in case they are wrong
      QNetworkAuthenticationCredential cred = authenticationManager->fetchCachedProxyCredentials(proxy);

      if (!priv->hasFailed || cred.user != username || cred.password != password) {
         authenticator->setUser(username);
         authenticator->setPassword(password);
         authenticationManager->cacheProxyCredentials(proxy, authenticator);
         return;
      }
   }

#else
   (void) url;
#endif

   // if we emit a signal here in synchronous mode, the user might spin
   // an event loop, which might recurse and lead to problems
   if (synchronous) {
      return;
   }

   *lastProxyAuthentication = proxy;
   emit q->proxyAuthenticationRequired(proxy, authenticator);
   authenticationManager->cacheProxyCredentials(proxy, authenticator);
}

QList<QNetworkProxy> QNetworkAccessManagerPrivate::queryProxy(const QNetworkProxyQuery &query)
{
   QList<QNetworkProxy> proxies;
   if (proxyFactory) {
      proxies = proxyFactory->queryProxy(query);

      if (proxies.isEmpty()) {
         qWarning("QNetworkAccessManager::queryProxy() Factory returned an empty result set");
         proxies << QNetworkProxy::NoProxy;
      }

   } else if (proxy.type() == QNetworkProxy::DefaultProxy) {
      // no proxy set, query the application
      return QNetworkProxyFactory::proxyForQuery(query);

   } else {
      proxies << proxy;
   }

   return proxies;
}
#endif

void QNetworkAccessManagerPrivate::clearCache(QNetworkAccessManager *manager)
{
   manager->d_func()->objectCache.clear();
   manager->d_func()->authenticationManager->clearCache();

   if (manager->d_func()->httpThread) {
      manager->d_func()->httpThread->quit();
      manager->d_func()->httpThread->wait(5000);

      if (manager->d_func()->httpThread->isFinished()) {
         delete manager->d_func()->httpThread;
      } else {
         QObject::connect(manager->d_func()->httpThread, &QThread::finished,
               manager->d_func()->httpThread, &QThread::deleteLater);
      }

      manager->d_func()->httpThread = nullptr;
   }
}

QNetworkAccessManagerPrivate::~QNetworkAccessManagerPrivate()
{
   if (httpThread) {
      httpThread->quit();
      httpThread->wait(5000);

      if (httpThread->isFinished()) {
         delete httpThread;
      } else {
         QObject::connect(httpThread, &QThread::finished, httpThread, &QThread::deleteLater);
      }

      httpThread = nullptr;
   }
}

#ifndef QT_NO_BEARERMANAGEMENT
void QNetworkAccessManagerPrivate::createSession(const QNetworkConfiguration &config)
{
   Q_Q(QNetworkAccessManager);

   initializeSession = false;

   //resurrect weak ref if possible
   networkSessionStrongRef = networkSessionWeakRef.toStrongRef();

   QSharedPointer<QNetworkSession> newSession;
   if (config.isValid()) {
      newSession = QSharedNetworkSessionManager::getSession(config);
   }

   if (networkSessionStrongRef) {
      //do nothing if new and old session are the same
      if (networkSessionStrongRef == newSession) {
         return;
      }

      //disconnect from old session
      QObject::disconnect(networkSessionStrongRef.data(), &QNetworkSession::opened, q,
            &QNetworkAccessManager::networkSessionConnected);

      QObject::disconnect(networkSessionStrongRef.data(), &QNetworkSession::closed, q,
            &QNetworkAccessManager::_q_networkSessionClosed);

      QObject::disconnect(networkSessionStrongRef.data(), &QNetworkSession::stateChanged, q,
            &QNetworkAccessManager::_q_networkSessionStateChanged);

      QObject::disconnect(networkSessionStrongRef.data(), &QNetworkSession::error, q,
            &QNetworkAccessManager::_q_networkSessionFailed);
   }

   // switch to new session (null if config was invalid)
   networkSessionStrongRef = newSession;
   networkSessionWeakRef = networkSessionStrongRef.toWeakRef();

   if (!networkSessionStrongRef) {

      if (networkAccessible == QNetworkAccessManager::NotAccessible || !online) {
         emit q->networkAccessibleChanged(QNetworkAccessManager::NotAccessible);
      } else {
         emit q->networkAccessibleChanged(QNetworkAccessManager::UnknownAccessibility);
      }

      return;
   }

   // onnect to new session
   QObject::connect(networkSessionStrongRef.data(), &QNetworkSession::opened, q,
            &QNetworkAccessManager::networkSessionConnected, Qt::QueuedConnection);

   //QueuedConnection is used to avoid deleting the networkSession inside its closed signal
   QObject::connect(networkSessionStrongRef.data(), &QNetworkSession::closed, q,
            &QNetworkAccessManager::_q_networkSessionClosed, Qt::QueuedConnection);

   QObject::connect(networkSessionStrongRef.data(), &QNetworkSession::stateChanged, q,
            &QNetworkAccessManager::_q_networkSessionStateChanged, Qt::QueuedConnection);

   QObject::connect(networkSessionStrongRef.data(), &QNetworkSession::error, q,
            &QNetworkAccessManager::_q_networkSessionFailed);

   _q_networkSessionStateChanged(networkSessionStrongRef->state());
}

void QNetworkAccessManagerPrivate::_q_networkSessionClosed()
{
   Q_Q(QNetworkAccessManager);
   QSharedPointer<QNetworkSession> networkSession(getNetworkSession());

   if (networkSession) {
      networkConfiguration = networkSession->configuration();

      //disconnect from old session
      QObject::disconnect(networkSession.data(), &QNetworkSession::opened, q,
            &QNetworkAccessManager::networkSessionConnected);

      QObject::disconnect(networkSession.data(), &QNetworkSession::closed, q,
            &QNetworkAccessManager::_q_networkSessionClosed);

      QObject::disconnect(networkSession.data(), &QNetworkSession::stateChanged, q,
            &QNetworkAccessManager::_q_networkSessionStateChanged);

      QObject::disconnect(networkSession.data(), &QNetworkSession::error, q,
            &QNetworkAccessManager::_q_networkSessionFailed);

      networkSessionStrongRef.clear();
      networkSessionWeakRef.clear();
   }
}

void QNetworkAccessManagerPrivate::_q_networkSessionStateChanged(QNetworkSession::State state)
{
   Q_Q(QNetworkAccessManager);
   bool reallyOnline = false;

   //Do not emit the networkSessionConnected signal here, except for roaming -> connected
   //transition, otherwise it is emitted twice in a row when opening a connection.
   if (state == QNetworkSession::Connected && lastSessionState != QNetworkSession::Roaming) {
      emit q->networkSessionConnected();
   }

   lastSessionState = state;

   if (online && state == QNetworkSession::Disconnected) {
      for (const QNetworkConfiguration &cfg : networkConfigurationManager.allConfigurations()) {
         if (cfg.state().testFlag(QNetworkConfiguration::Active)) {
            reallyOnline = true;
         }
      }

   } else if (state == QNetworkSession::Connected || state == QNetworkSession::Roaming) {
      reallyOnline = true;
   }

   if (! reallyOnline) {
      if (state != QNetworkSession::Connected && state != QNetworkSession::Roaming) {
         if (networkAccessible != QNetworkAccessManager::NotAccessible) {
            networkAccessible = QNetworkAccessManager::NotAccessible;
            emit q->networkAccessibleChanged(networkAccessible);
         }
      }

   } else {
      if (defaultAccessControl)
         if (networkAccessible != QNetworkAccessManager::Accessible) {
            networkAccessible = QNetworkAccessManager::Accessible;
            emit q->networkAccessibleChanged(networkAccessible);
         }
   }

   online = reallyOnline;

   if (online && (state != QNetworkSession::Connected && state != QNetworkSession::Roaming)) {
      _q_networkSessionClosed();
      createSession(q->configuration());
   }
}

void QNetworkAccessManagerPrivate::_q_onlineStateChanged(bool isOnline)
{
   Q_Q(QNetworkAccessManager);

   // if the user set a config, we only care whether this one is active.
   // Otherwise, this QNAM is online if there is an online config.
   if (customNetworkConfiguration) {
      online = (networkConfiguration.state() & QNetworkConfiguration::Active);

   } else {
      if (online != isOnline) {
         _q_networkSessionClosed();
         createSession(q->configuration());
         online = isOnline;
      }
   }

   if (online) {
      if (defaultAccessControl) {
         if (networkAccessible != QNetworkAccessManager::Accessible) {
            networkAccessible = QNetworkAccessManager::Accessible;
            emit q->networkAccessibleChanged(networkAccessible);
         }
      }

   } else {
      if (networkAccessible != QNetworkAccessManager::NotAccessible) {
         networkAccessible = QNetworkAccessManager::NotAccessible;
         emit q->networkAccessibleChanged(networkAccessible);
      }
   }
}

void QNetworkAccessManagerPrivate::_q_configurationChanged(const QNetworkConfiguration &configuration)
{
   const QString id = configuration.identifier();
   if (configuration.state().testFlag(QNetworkConfiguration::Active)) {
      if (!onlineConfigurations.contains(id)) {

         QSharedPointer<QNetworkSession> session(getNetworkSession());
         if (session) {
            if (online && session->configuration().identifier()
                  != networkConfigurationManager.defaultConfiguration().identifier()) {

               onlineConfigurations.insert(id);
               //this one disconnected but another one is online,
               // close and create new session
               _q_networkSessionClosed();
               createSession(networkConfigurationManager.defaultConfiguration());
            }
         }
      }

   } else if (onlineConfigurations.contains(id)) {
      //this one is disconnecting
      onlineConfigurations.remove(id);

      if (!onlineConfigurations.isEmpty()) {
         _q_networkSessionClosed();
         createSession(configuration);
      }
   }
}

void QNetworkAccessManagerPrivate::_q_networkSessionFailed(QNetworkSession::SessionError)
{
   for (const QNetworkConfiguration &cfg : networkConfigurationManager.allConfigurations()) {
      if (cfg.state().testFlag(QNetworkConfiguration::Active)) {
         online = true;
         _q_networkSessionClosed();
         createSession(networkConfigurationManager.defaultConfiguration());
         return;
      }
   }
}

#endif // QT_NO_BEARERMANAGEMENT

QNetworkRequest QNetworkAccessManagerPrivate::prepareMultipart(const QNetworkRequest &request, QHttpMultiPart *multiPart)
{
   // copy the request, we probably need to add some headers
   QNetworkRequest newRequest(request);

   // add Content-Type header if not there already
   if (! request.header(QNetworkRequest::ContentTypeHeader).isValid()) {
      QByteArray contentType;
      contentType.reserve(34 + multiPart->d_func()->boundary.count());
      contentType += "multipart/";

      switch (multiPart->d_func()->contentType) {
         case QHttpMultiPart::RelatedType:
            contentType += "related";
            break;

         case QHttpMultiPart::FormDataType:
            contentType += "form-data";
            break;

         case QHttpMultiPart::AlternativeType:
            contentType += "alternative";
            break;

         default:
            contentType += "mixed";
            break;
      }

      // putting the boundary into quotes, recommended in RFC 2046 section 5.1.1
      contentType += "; boundary=\"" + multiPart->d_func()->boundary + '"';
      newRequest.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(contentType));
   }

   // add MIME-Version header if not there already (we must include the header
   // if the message conforms to RFC 2045, see section 4 of that RFC)
   QByteArray mimeHeader("MIME-Version");
   if (!request.hasRawHeader(mimeHeader)) {
      newRequest.setRawHeader(mimeHeader, QByteArray("1.0"));
   }

   QIODevice *device = multiPart->d_func()->device;
   if (! device->isReadable()) {
      if (! device->isOpen()) {
         if (! device->open(QIODevice::ReadOnly)) {
            qWarning("QNetworkAccessManager::prepareMultipart() Unable to open device for reading");
         }

      } else {
         qWarning("QNetworkAccessManager::prepareMultipart() Device is not readable");
      }
   }

   return newRequest;
}

void QNetworkAccessManager::_q_replyFinished()
{
   Q_D(QNetworkAccessManager);
   d->_q_replyFinished();
}

void QNetworkAccessManager::_q_replyEncrypted()
{
   Q_D(QNetworkAccessManager);
   d->_q_replyEncrypted();
}

#ifdef QT_SSL

void QNetworkAccessManager::_q_replySslErrors(const QList<QSslError> &errorList)
{
   Q_D(QNetworkAccessManager);
   d->_q_replySslErrors(errorList);
}

void QNetworkAccessManager::_q_replyPreSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *data)
{
   Q_D(QNetworkAccessManager);
   d->_q_replyPreSharedKeyAuthenticationRequired(data);
}

#endif

#ifndef QT_NO_BEARERMANAGEMENT

void QNetworkAccessManager::_q_networkSessionClosed()
{
   Q_D(QNetworkAccessManager);
   d->_q_networkSessionClosed();
}

void QNetworkAccessManager::_q_networkSessionStateChanged(QNetworkSession::State data)
{
   Q_D(QNetworkAccessManager);
   d->_q_networkSessionStateChanged(data);
}

void QNetworkAccessManager::_q_onlineStateChanged(bool data)
{
   Q_D(QNetworkAccessManager);
   d->_q_onlineStateChanged(data);
}

void QNetworkAccessManager::_q_configurationChanged(const QNetworkConfiguration &data)
{
   Q_D(QNetworkAccessManager);
   d->_q_configurationChanged(data);
}

void QNetworkAccessManager::_q_networkSessionFailed(QNetworkSession::SessionError data)
{
   Q_D(QNetworkAccessManager);
   d->_q_networkSessionFailed(data);
}

#endif
