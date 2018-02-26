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

#include <qnetworkaccessmanager.h>
#include <qnetworkaccessmanager_p.h>

#include <qauthenticator_p.h>
#include <qsharednetworksession_p.h>
#include <qnetworkaccessftpbackend_p.h>
#include <qnetworkaccessfilebackend_p.h>
#include <qnetworkaccesscachebackend_p.h>
#include <qnetworkreplydataimpl_p.h>
#include <qnetworkreplyfileimpl_p.h>
#include <qnetworkreplyhttpimpl_p.h>
#include <qnetworkreply_p.h>
#include <qhttpmultipart_p.h>

#include <qnetworkrequest.h>
#include <qnetworkreply.h>
#include <qnetworkcookie.h>
#include <qnetworkcookiejar.h>
#include <qabstractnetworkcache.h>
#include <qnetworksession.h>
#include <qsslconfiguration.h>
#include <qnetworkconfigmanager.h>
#include <qhttpmultipart.h>

#include <qbuffer.h>
#include <qthread.h>
#include <qurl.h>
#include <qvector.h>

Q_GLOBAL_STATIC(QNetworkAccessFileBackendFactory, fileBackend)

#ifndef QT_NO_FTP
Q_GLOBAL_STATIC(QNetworkAccessFtpBackendFactory, ftpBackend)
#endif

#ifdef QT_BUILD_INTERNAL
Q_GLOBAL_STATIC(QNetworkAccessDebugPipeBackendFactory, debugpipeBackend)
#endif

#if defined(Q_OS_MAC)

#include <CoreServices/CoreServices.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <Security/SecKeychain.h>

bool getProxyAuth(const QString &proxyHostname, const QString &scheme, QString &username, QString &password)
{
   OSStatus err;
   SecKeychainItemRef itemRef;
   bool retValue = false;
   SecProtocolType protocolType = kSecProtocolTypeAny;

   if (scheme.compare(QLatin1String("ftp"), Qt::CaseInsensitive) == 0) {
      protocolType = kSecProtocolTypeFTPProxy;

   } else if (scheme.compare(QLatin1String("http"), Qt::CaseInsensitive) == 0
              || scheme.compare(QLatin1String("preconnect-http"), Qt::CaseInsensitive) == 0) {
      protocolType = kSecProtocolTypeHTTPProxy;

   } else if (scheme.compare(QLatin1String("https"), Qt::CaseInsensitive) == 0
              || scheme.compare(QLatin1String("preconnect-https"), Qt::CaseInsensitive) == 0) {
      protocolType = kSecProtocolTypeHTTPSProxy;
   }

   QByteArray proxyHostnameUtf8(proxyHostname.toUtf8());
   err = SecKeychainFindInternetPassword(NULL, proxyHostnameUtf8.length(), proxyHostnameUtf8.constData(),
                 0, NULL, 0, NULL, 0, NULL, 0, protocolType, kSecAuthenticationTypeAny, 0, NULL, &itemRef);

   if (err == noErr) {

      SecKeychainAttribute attr;
      SecKeychainAttributeList attrList;
      UInt32 length;
      void *outData;

      attr.tag = kSecAccountItemAttr;
      attr.length = 0;
      attr.data = NULL;

      attrList.count = 1;
      attrList.attr = &attr;

      if (SecKeychainItemCopyContent(itemRef, NULL, &attrList, &length, &outData) == noErr) {
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
   (void) ftpBackend();
#endif

   // leave this one last since it will query the special QAbstractFileEngines
   (void) fileBackend();
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

   connect(&d->networkConfigurationManager, SIGNAL(onlineStateChanged(bool)), this,
           SLOT(_q_onlineStateChanged(bool)));

   connect(&d->networkConfigurationManager, SIGNAL(configurationChanged(const QNetworkConfiguration &)), this,
           SLOT(_q_configurationChanged(const QNetworkConfiguration &)));

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
   // FIXME instead of this "hack" make the QNetworkReplyImpl
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
   d->proxyFactory = 0;
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

/*!
    \overload

    Sends the contents of the \a data byte array to the destination
    specified by \a request.
*/
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

/*!
    \overload

    Sends the contents of the \a data byte array to the destination
    specified by \a request.
*/
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
                networkSession->sessionProperty(QLatin1String("ActiveConfiguration")).toString());
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

/*!
    \internal

    Returns the network session currently in use.
    This can be changed at any time, ownership remains with the QNetworkAccessManager
*/
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
   url.setScheme(QLatin1String("preconnect-https"));

   QNetworkRequest request(url);

   if (sslConfiguration != QSslConfiguration::defaultConfiguration()) {
      request.setSslConfiguration(sslConfiguration);
   }

   // There is no way to enable SPDY via a request, so we need to check
   // the ssl configuration whether SPDY is allowed here.
   if (sslConfiguration.allowedNextProtocols().contains(
            QSslConfiguration::NextProtocolSpdy3_0)) {
      request.setAttribute(QNetworkRequest::SpdyAllowedAttribute, true);
   }

   get(request);
}

#endif

/*!
    \since 5.2

    Initiates a connection to the host given by \a hostName at port \a port.
    This function is useful to complete the TCP handshake
    to a host before the HTTP request is made, resulting in a lower network latency.

    \note This function has no possibility to report errors.

    \sa connectToHostEncrypted(), get(), post(), put(), deleteResource()
*/
void QNetworkAccessManager::connectToHost(const QString &hostName, quint16 port)
{
   QUrl url;
   url.setHost(hostName);
   url.setPort(port);
   url.setScheme(QLatin1String("preconnect-http"));
   QNetworkRequest request(url);
   get(request);
}

/*!
    \since 4.7

    Sends a custom request to the server identified by the URL of \a request.

    It is the user's responsibility to send a \a verb to the server that is valid
    according to the HTTP specification.

    This method provides means to send verbs other than the common ones provided
    via get() or post() etc., for instance sending an HTTP OPTIONS command.

    If \a data is not empty, the contents of the \a data
    device will be uploaded to the server; in that case, data must be open for
    reading and must remain valid until the finished() signal is emitted for this reply.

    \note This feature is currently available for HTTP(S) only.

    \sa get(), post(), put(), deleteResource()
*/
QNetworkReply *QNetworkAccessManager::sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QIODevice *data)
{
   QNetworkRequest newRequest(request);
   newRequest.setAttribute(QNetworkRequest::CustomVerbAttribute, verb);
   return d_func()->postProcess(createRequest(QNetworkAccessManager::CustomOperation, newRequest, data));
}

/*!
    Returns a new QNetworkReply object to handle the operation \a op
    and request \a req. The device \a outgoingData is always 0 for Get and
    Head requests, but is the value passed to post() and put() in
    those operations (the QByteArray variants will pass a QBuffer
    object).

    The default implementation calls QNetworkCookieJar::cookiesForUrl()
    on the cookie jar set with setCookieJar() to obtain the cookies to
    be sent to the remote server.

    The returned object must be in an open state.
*/
QNetworkReply *QNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op,
      const QNetworkRequest &req,
      QIODevice *outgoingData)
{
   Q_D(QNetworkAccessManager);

   bool isLocalFile = req.url().isLocalFile();
   QString scheme = req.url().scheme();

   // fast path for GET on file:// URLs
   // The QNetworkAccessFileBackend will right now only be used for PUT

   if (op == QNetworkAccessManager::GetOperation || op == QNetworkAccessManager::HeadOperation) {

      if (isLocalFile || scheme == QLatin1String("qrc")) {
         return new QNetworkReplyFileImpl(this, req, op);
      }

      if (scheme == QLatin1String("data")) {
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
      if (!d->networkConfiguration.identifier().isEmpty()) {
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
   if (!request.header(QNetworkRequest::ContentLengthHeader).isValid() &&
         outgoingData && !outgoingData->isSequential()) {
      // request has no Content-Length
      // but the data that is outgoing is random-access
      request.setHeader(QNetworkRequest::ContentLengthHeader, outgoingData->size());
   }

   if (static_cast<QNetworkRequest::LoadControl>
         (request.attribute(QNetworkRequest::CookieLoadControlAttribute,
                            QNetworkRequest::Automatic).toInt()) == QNetworkRequest::Automatic) {
      if (d->cookieJar) {
         QList<QNetworkCookie> cookies = d->cookieJar->cookiesForUrl(request.url());
         if (!cookies.isEmpty()) {
            request.setHeader(QNetworkRequest::CookieHeader, QVariant::fromValue(cookies));
         }
      }
   }

   // use the new QNetworkReplyHttpImpl
   if (scheme == QLatin1String("http") || scheme == QLatin1String("preconnect-http")

#ifdef QT_SSL
         || scheme == QLatin1String("https") || scheme == QLatin1String("preconnect-https")
#endif

      ) {

      QNetworkReplyHttpImpl *reply = new QNetworkReplyHttpImpl(this, request, op, outgoingData);

#ifndef QT_NO_BEARERMANAGEMENT
      connect(this, SIGNAL(networkSessionConnected()), reply, SLOT(_q_networkSessionConnected()));
#endif

      return reply;
   }

   // first step: create the reply
   QNetworkReplyImpl *reply = new QNetworkReplyImpl(this);

#ifndef QT_NO_BEARERMANAGEMENT
   if (! isLocalFile) {
      connect(this, SIGNAL(networkSessionConnected()), reply, SLOT(_q_networkSessionConnected()));
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

   QNetworkReply *reply = qobject_cast<QNetworkReply *>(q->sender());
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

   QNetworkReply *reply = qobject_cast<QNetworkReply *>(q->sender());
   if (reply) {
      emit q->encrypted(reply);
   }
#endif
}

void QNetworkAccessManagerPrivate::_q_replySslErrors(const QList<QSslError> &errors)
{
#ifdef QT_SSL
   Q_Q(QNetworkAccessManager);

   QNetworkReply *reply = qobject_cast<QNetworkReply *>(q->sender());
   if (reply) {
      emit q->sslErrors(reply, errors);
   }
#else
   Q_UNUSED(errors);
#endif
}

void QNetworkAccessManagerPrivate::_q_replyPreSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *authenticator)
{
#ifdef QT_SSL
   Q_Q(QNetworkAccessManager);

   QNetworkReply *reply = qobject_cast<QNetworkReply *>(q->sender());

   if (reply) {
      emit q->preSharedKeyAuthenticationRequired(reply, authenticator);
   }
#endif
}

QNetworkReply *QNetworkAccessManagerPrivate::postProcess(QNetworkReply *reply)
{
   Q_Q(QNetworkAccessManager);

   QNetworkReplyPrivate::setManager(reply, q);
   q->connect(reply, SIGNAL(finished()),   q, SLOT(_q_replyFinished()));

#ifdef QT_SSL
   // In case we are compiled without SSL support, we do not have this signal and we need to
   // avoid getting a connection error.

   q->connect(reply, SIGNAL(encrypted()),                         q, SLOT(_q_replyEncrypted()));
   q->connect(reply, SIGNAL(sslErrors(const QList<QSslError> &)), q, SLOT(_q_replySslErrors(const QList<QSslError> &)));

   q->connect(reply, SIGNAL(preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *)),
              q, SLOT(_q_replyPreSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *)));
#endif

#ifndef QT_NO_BEARERMANAGEMENT
   activeReplyCount++;
#endif

   return reply;
}

void QNetworkAccessManagerPrivate::createCookieJar() const
{
   if (!cookieJarCreated) {
      // keep the ugly hack in here
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
      if (!url.userName().isEmpty()
            && !url.password().isEmpty()) {
         authenticator->setUser(url.userName(QUrl::FullyDecoded));
         authenticator->setPassword(url.password(QUrl::FullyDecoded));
         *urlForLastAuthentication = url;
         authenticationManager->cacheCredentials(url, authenticator);
         return;
      }

      QNetworkAuthenticationCredential cred = authenticationManager->fetchCachedCredentials(url, authenticator);
      if (!cred.isNull()) {
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

#if defined(Q_OS_MAC)
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
   Q_UNUSED(url);
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
         qWarning("QNetworkAccessManager: factory %p has returned an empty result set",
                  proxyFactory);
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
         QObject::connect(manager->d_func()->httpThread, SIGNAL(finished()), manager->d_func()->httpThread, SLOT(deleteLater()));
      }
      manager->d_func()->httpThread = 0;
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
         QObject::connect(httpThread, SIGNAL(finished()), httpThread, SLOT(deleteLater()));
      }
      httpThread = 0;
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
      QObject::disconnect(networkSessionStrongRef.data(), SIGNAL(opened()), q, SLOT(networkSessionConnected()));
      QObject::disconnect(networkSessionStrongRef.data(), SIGNAL(closed()), q, SLOT(_q_networkSessionClosed()));

      QObject::disconnect(networkSessionStrongRef.data(), SIGNAL(stateChanged(QNetworkSession::State)),
                          q, SLOT(_q_networkSessionStateChanged(QNetworkSession::State)));

      QObject::disconnect(networkSessionStrongRef.data(), SIGNAL(error(QNetworkSession::SessionError)),
                          q, SLOT(_q_networkSessionFailed(QNetworkSession::SessionError)));
   }

   //switch to new session (null if config was invalid)
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

   //connect to new session
   QObject::connect(networkSessionStrongRef.data(), SIGNAL(opened()), q, SLOT(networkSessionConnected()), Qt::QueuedConnection);
   //QueuedConnection is used to avoid deleting the networkSession inside its closed signal
   QObject::connect(networkSessionStrongRef.data(), SIGNAL(closed()), q, SLOT(_q_networkSessionClosed()), Qt::QueuedConnection);

   QObject::connect(networkSessionStrongRef.data(), SIGNAL(stateChanged(QNetworkSession::State)),
                    q, SLOT(_q_networkSessionStateChanged(QNetworkSession::State)), Qt::QueuedConnection);

   QObject::connect(networkSessionStrongRef.data(), SIGNAL(error(QNetworkSession::SessionError)),
                    q, SLOT(_q_networkSessionFailed(QNetworkSession::SessionError)));

   _q_networkSessionStateChanged(networkSessionStrongRef->state());
}

void QNetworkAccessManagerPrivate::_q_networkSessionClosed()
{
   Q_Q(QNetworkAccessManager);
   QSharedPointer<QNetworkSession> networkSession(getNetworkSession());

   if (networkSession) {
      networkConfiguration = networkSession->configuration();

      //disconnect from old session
      QObject::disconnect(networkSession.data(), SIGNAL(opened()), q, SLOT(networkSessionConnected()));
      QObject::disconnect(networkSession.data(), SIGNAL(closed()), q, SLOT(_q_networkSessionClosed()));

      QObject::disconnect(networkSession.data(), SIGNAL(stateChanged(QNetworkSession::State)),
                          q, SLOT(_q_networkSessionStateChanged(QNetworkSession::State)));

      QObject::disconnect(networkSession.data(), SIGNAL(error(QNetworkSession::SessionError)),
                          q, SLOT(_q_networkSessionFailed(QNetworkSession::SessionError)));

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

   if (!reallyOnline) {
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
   if (!request.header(QNetworkRequest::ContentTypeHeader).isValid()) {
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
   if (!device->isReadable()) {
      if (!device->isOpen()) {
         if (!device->open(QIODevice::ReadOnly)) {
            qWarning("could not open device for reading");
         }
      } else {
         qWarning("device is not readable");
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

void QNetworkAccessManager::_q_replySslErrors(const QList<QSslError> &errList)
{
   Q_D(QNetworkAccessManager);
   d->_q_replySslErrors(errList);
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


