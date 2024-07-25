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

#include <qnetworkproxy.h>

#ifndef QT_NO_NETWORKPROXY

#include <qauthenticator.h>
#include <qdebug.h>
#include <qmutex.h>
#include <qstringlist.h>
#include <qurl.h>

#ifndef QT_NO_BEARERMANAGEMENT
#include <qnetworkconfiguration.h>
#endif

#include <qhttpsocketengine_p.h>
#include <qnetwork_request_p.h>
#include <qnetworkproxy_p.h>
#include <qsocks5socketengine_p.h>

class QHttpSocketEngineHandler;
class QSocks5SocketEngineHandler;

class QGlobalNetworkProxy
{
 public:
   QGlobalNetworkProxy()
      : applicationLevelProxy(nullptr), applicationLevelProxyFactory(nullptr),
        socks5SocketEngineHandler(nullptr), httpSocketEngineHandler(nullptr)
   {
#ifdef QT_USE_SYSTEM_PROXIES
      setApplicationProxyFactory(new QSystemConfigurationProxyFactory);
#endif

      socks5SocketEngineHandler = new QSocks5SocketEngineHandler();
      httpSocketEngineHandler   = new QHttpSocketEngineHandler();
   }

   ~QGlobalNetworkProxy() {
      delete applicationLevelProxy;
      delete applicationLevelProxyFactory;
      delete socks5SocketEngineHandler;
      delete httpSocketEngineHandler;
   }

   void setApplicationProxy(const QNetworkProxy &proxy) {
      QRecursiveMutexLocker lock(&mutex);

      if (! applicationLevelProxy) {
         applicationLevelProxy = new QNetworkProxy;
      }

      *applicationLevelProxy = proxy;
      delete applicationLevelProxyFactory;
      applicationLevelProxyFactory = nullptr;
   }

   void setApplicationProxyFactory(QNetworkProxyFactory *factory) {
      QRecursiveMutexLocker lock(&mutex);

      if (factory == applicationLevelProxyFactory) {
         return;
      }

      if (applicationLevelProxy) {
         *applicationLevelProxy = QNetworkProxy();
      }

      delete applicationLevelProxyFactory;
      applicationLevelProxyFactory = factory;
   }

   QNetworkProxy applicationProxy() {
      return proxyForQuery(QNetworkProxyQuery()).first();
   }

   QList<QNetworkProxy> proxyForQuery(const QNetworkProxyQuery &query);

 private:
   QRecursiveMutex mutex;
   QNetworkProxy *applicationLevelProxy;
   QNetworkProxyFactory *applicationLevelProxyFactory;
   QSocks5SocketEngineHandler *socks5SocketEngineHandler;
   QHttpSocketEngineHandler *httpSocketEngineHandler;
};

QGlobalNetworkProxy *cs_GlobalNetworkProxy()
{
   static QGlobalNetworkProxy retval;
   return &retval;
}

QList<QNetworkProxy> QGlobalNetworkProxy::proxyForQuery(const QNetworkProxyQuery &query)
{
   QRecursiveMutexLocker locker(&mutex);

   QList<QNetworkProxy> result;

   // don't look for proxies for a local connection
   QHostAddress parsed;
   QString hostname = query.url().host();

   if (hostname == "localhost" || hostname.startsWith("localhost.") || (parsed.setAddress(hostname) && (parsed.isLoopback()))) {
      result << QNetworkProxy(QNetworkProxy::NoProxy);
      return result;
   }

   if (!applicationLevelProxyFactory) {
      if (applicationLevelProxy && applicationLevelProxy->type() != QNetworkProxy::DefaultProxy) {
         result << *applicationLevelProxy;
      } else {
         result << QNetworkProxy(QNetworkProxy::NoProxy);
      }
      return result;
   }

   // we have a factory
   result = applicationLevelProxyFactory->queryProxy(query);

   if (result.isEmpty()) {
      qWarning("QGlobalNetworkProxy::proxyForQuery() Factory returned an empty result set");
      result << QNetworkProxy(QNetworkProxy::NoProxy);
   }

   return result;
}

static QNetworkProxy::Capabilities defaultCapabilitiesForType(QNetworkProxy::ProxyType type)
{
   static_assert(int(QNetworkProxy::DefaultProxy) == 0);
   static_assert(int(QNetworkProxy::FtpCachingProxy) == 5);

   static const int defaults[] = {
      /* [QNetworkProxy::DefaultProxy] = */
      (int(QNetworkProxy::ListeningCapability) |
       int(QNetworkProxy::TunnelingCapability) |
       int(QNetworkProxy::UdpTunnelingCapability)),
      /* [QNetworkProxy::Socks5Proxy] = */
      (int(QNetworkProxy::TunnelingCapability) |
       int(QNetworkProxy::ListeningCapability) |
       int(QNetworkProxy::UdpTunnelingCapability) |
       int(QNetworkProxy::HostNameLookupCapability)),
      // it's weird to talk about the proxy capabilities of a "not proxy"...
      /* [QNetworkProxy::NoProxy] = */
      (int(QNetworkProxy::ListeningCapability) |
       int(QNetworkProxy::TunnelingCapability) |
       int(QNetworkProxy::UdpTunnelingCapability)),
      /* [QNetworkProxy::HttpProxy] = */
      (int(QNetworkProxy::TunnelingCapability) |
       int(QNetworkProxy::CachingCapability) |
       int(QNetworkProxy::HostNameLookupCapability)),
      /* [QNetworkProxy::HttpCachingProxy] = */
      (int(QNetworkProxy::CachingCapability) |
       int(QNetworkProxy::HostNameLookupCapability)),
      /* [QNetworkProxy::FtpCachingProxy] = */
      (int(QNetworkProxy::CachingCapability) |
       int(QNetworkProxy::HostNameLookupCapability)),
   };

   if (int(type) < 0 || int(type) > int(QNetworkProxy::FtpCachingProxy)) {
      type = QNetworkProxy::DefaultProxy;
   }

   return QNetworkProxy::Capabilities(defaults[int(type)]);
}

class QNetworkProxyPrivate: public QSharedData
{
 public:
   QString hostName;
   QString user;
   QString password;
   QNetworkProxy::Capabilities capabilities;
   quint16 port;
   QNetworkProxy::ProxyType type;
   bool capabilitiesSet;
   QNetworkHeadersPrivate headers;

   QNetworkProxyPrivate(QNetworkProxy::ProxyType t = QNetworkProxy::DefaultProxy, const QString &h = QString(),
                               quint16 p = 0, const QString &u = QString(), const QString &pw = QString())

      : hostName(h), user(u), password(pw), capabilities(defaultCapabilitiesForType(t)),
        port(p), type(t), capabilitiesSet(false)
   {
   }

   bool operator==(const QNetworkProxyPrivate &other) const {
      return type == other.type && port == other.port &&
             hostName == other.hostName && user == other.user &&
             password == other.password && capabilities == other.capabilities;
   }
};

template <>
void QSharedDataPointer<QNetworkProxyPrivate>::detach()
{
   if (d && d->ref.load() == 1) {
      return;
   }

   QNetworkProxyPrivate *x = (d ? new QNetworkProxyPrivate(*d) : new QNetworkProxyPrivate);
   x->ref.ref();
   if (d && !d->ref.deref()) {
      delete d;
   }
   d = x;
}

QNetworkProxy::QNetworkProxy()
   : d(nullptr)
{
   // make sure we have QGlobalNetworkProxy singleton created, otherwise
   // you don't have any socket engine handler created when directly setting
   // a proxy to the socket

   cs_GlobalNetworkProxy();
}

QNetworkProxy::QNetworkProxy(ProxyType type, const QString &hostName, quint16 port,
                             const QString &user, const QString &password)
   : d(new QNetworkProxyPrivate(type, hostName, port, user, password))
{
   // make sure we have QGlobalNetworkProxy singleton created, otherwise you don't have
   // any socket engine handler created when directly setting a proxy to a socket

   cs_GlobalNetworkProxy();
}

QNetworkProxy::QNetworkProxy(const QNetworkProxy &other)
   : d(other.d)
{
}

QNetworkProxy::~QNetworkProxy()
{
   // QSharedDataPointer takes care of deleting for us
}

bool QNetworkProxy::operator==(const QNetworkProxy &other) const
{
   return d == other.d || (d && other.d && *d == *other.d);
}

QNetworkProxy &QNetworkProxy::operator=(const QNetworkProxy &other)
{
   d = other.d;
   return *this;
}

void QNetworkProxy::setType(QNetworkProxy::ProxyType type)
{
   d->type = type;
   if (!d->capabilitiesSet) {
      d->capabilities = defaultCapabilitiesForType(type);
   }
}

QNetworkProxy::ProxyType QNetworkProxy::type() const
{
   return d ? d->type : DefaultProxy;
}

void QNetworkProxy::setCapabilities(Capabilities capabilities)
{
   d->capabilities = capabilities;
   d->capabilitiesSet = true;
}

QNetworkProxy::Capabilities QNetworkProxy::capabilities() const
{
   return d ? d->capabilities : defaultCapabilitiesForType(DefaultProxy);
}

bool QNetworkProxy::isCachingProxy() const
{
   return capabilities() & CachingCapability;
}

bool QNetworkProxy::isTransparentProxy() const
{
   return capabilities() & TunnelingCapability;
}

void QNetworkProxy::setUser(const QString &user)
{
   d->user = user;
}

QString QNetworkProxy::user() const
{
   return d ? d->user : QString();
}

void QNetworkProxy::setPassword(const QString &password)
{
   d->password = password;
}

QString QNetworkProxy::password() const
{
   return d ? d->password : QString();
}

void QNetworkProxy::setHostName(const QString &hostName)
{
   d->hostName = hostName;
}

QString QNetworkProxy::hostName() const
{
   return d ? d->hostName : QString();
}

void QNetworkProxy::setPort(quint16 port)
{
   d->port = port;
}

quint16 QNetworkProxy::port() const
{
   return d ? d->port : 0;
}

void QNetworkProxy::setApplicationProxy(const QNetworkProxy &networkProxy)
{
   QGlobalNetworkProxy *obj = cs_GlobalNetworkProxy();

   // do not allow setting the proxy to DefaultProxy

   if (networkProxy.type() == DefaultProxy) {
      obj->setApplicationProxy(QNetworkProxy::NoProxy);
   } else {
      obj->setApplicationProxy(networkProxy);
   }
}

QNetworkProxy QNetworkProxy::applicationProxy()
{
   QGlobalNetworkProxy *obj = cs_GlobalNetworkProxy();

   return obj->applicationProxy();
}

QVariant QNetworkProxy::header(QNetworkRequest::KnownHeaders header) const
{
   if (d->type != HttpProxy && d->type != HttpCachingProxy) {
      return QVariant();
   }

   return d->headers.cookedHeaders.value(header);
}

void QNetworkProxy::setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
   if (d->type == HttpProxy || d->type == HttpCachingProxy) {
      d->headers.setCookedHeader(header, value);
   }
}

bool QNetworkProxy::hasRawHeader(const QByteArray &headerName) const
{
   if (d->type != HttpProxy && d->type != HttpCachingProxy) {
      return false;
   }

   return d->headers.findRawHeader(headerName) != d->headers.rawHeaders.constEnd();
}

QByteArray QNetworkProxy::rawHeader(const QByteArray &headerName) const
{
   if (d->type != HttpProxy && d->type != HttpCachingProxy) {
      return QByteArray();
   }
   QNetworkHeadersPrivate::RawHeadersList::const_iterator it = d->headers.findRawHeader(headerName);
   if (it != d->headers.rawHeaders.constEnd()) {
      return it->second;
   }
   return QByteArray();
}

QList<QByteArray> QNetworkProxy::rawHeaderList() const
{
   if (d->type != HttpProxy && d->type != HttpCachingProxy) {
      return QList<QByteArray>();
   }
   return d->headers.rawHeadersKeys();
}

void QNetworkProxy::setRawHeader(const QByteArray &headerName, const QByteArray &headerValue)
{
   if (d->type == HttpProxy || d->type == HttpCachingProxy) {
      d->headers.setRawHeader(headerName, headerValue);
   }
}

class QNetworkProxyQueryPrivate: public QSharedData
{
 public:
   inline QNetworkProxyQueryPrivate()
      : localPort(-1), type(QNetworkProxyQuery::TcpSocket) {
   }

   bool operator==(const QNetworkProxyQueryPrivate &other) const {
      return type == other.type &&
             localPort == other.localPort &&
             remote == other.remote;
   }

   QUrl remote;
   int localPort;
   QNetworkProxyQuery::QueryType type;

#ifndef QT_NO_BEARERMANAGEMENT
   QNetworkConfiguration config;
#endif
};

template <>
void QSharedDataPointer<QNetworkProxyQueryPrivate>::detach()
{
   if (d && d->ref.load() == 1) {
      return;
   }
   QNetworkProxyQueryPrivate *x = (d ? new QNetworkProxyQueryPrivate(*d)
                                   : new QNetworkProxyQueryPrivate);
   x->ref.ref();
   if (d && !d->ref.deref()) {
      delete d;
   }
   d = x;
}

QNetworkProxyQuery::QNetworkProxyQuery()
{
}

QNetworkProxyQuery::QNetworkProxyQuery(const QUrl &requestUrl, QueryType queryType)
{
   d->remote = requestUrl;
   d->type = queryType;
}

QNetworkProxyQuery::QNetworkProxyQuery(const QString &hostname, int port,
                                       const QString &protocolTag, QueryType queryType)
{
   d->remote.setScheme(protocolTag);
   d->remote.setHost(hostname);
   d->remote.setPort(port);
   d->type = queryType;
}

QNetworkProxyQuery::QNetworkProxyQuery(quint16 bindPort, const QString &protocolTag, QueryType queryType)
{
   d->remote.setScheme(protocolTag);
   d->localPort = bindPort;
   d->type = queryType;
}

#ifndef QT_NO_BEARERMANAGEMENT

QNetworkProxyQuery::QNetworkProxyQuery(const QNetworkConfiguration &networkConfiguration,
                                       const QUrl &requestUrl, QueryType queryType)
{
   d->config = networkConfiguration;
   d->remote = requestUrl;
   d->type = queryType;
}

QNetworkProxyQuery::QNetworkProxyQuery(const QNetworkConfiguration &networkConfiguration,
                                       const QString &hostname, int port, const QString &protocolTag, QueryType queryType)
{
   d->config = networkConfiguration;
   d->remote.setScheme(protocolTag);
   d->remote.setHost(hostname);
   d->remote.setPort(port);
   d->type = queryType;
}

QNetworkProxyQuery::QNetworkProxyQuery(const QNetworkConfiguration &networkConfiguration,
                                       quint16 bindPort, const QString &protocolTag, QueryType queryType)
{
   d->config = networkConfiguration;
   d->remote.setScheme(protocolTag);
   d->localPort = bindPort;
   d->type = queryType;
}
#endif

QNetworkProxyQuery::QNetworkProxyQuery(const QNetworkProxyQuery &other)
   : d(other.d)
{
}

QNetworkProxyQuery::~QNetworkProxyQuery()
{
   // QSharedDataPointer automatically deletes
}

QNetworkProxyQuery &QNetworkProxyQuery::operator=(const QNetworkProxyQuery &other)
{
   d = other.d;
   return *this;
}

bool QNetworkProxyQuery::operator==(const QNetworkProxyQuery &other) const
{
   return d == other.d || (d && other.d && *d == *other.d);
}

QNetworkProxyQuery::QueryType QNetworkProxyQuery::queryType() const
{
   return d ? d->type : TcpSocket;
}

void QNetworkProxyQuery::setQueryType(QueryType type)
{
   d->type = type;
}

int QNetworkProxyQuery::peerPort() const
{
   return d ? d->remote.port() : -1;
}

void QNetworkProxyQuery::setPeerPort(int port)
{
   d->remote.setPort(port);
}

QString QNetworkProxyQuery::peerHostName() const
{
   return d ? d->remote.host() : QString();
}

void QNetworkProxyQuery::setPeerHostName(const QString &hostname)
{
   d->remote.setHost(hostname);
}

int QNetworkProxyQuery::localPort() const
{
   return d ? d->localPort : -1;
}

void QNetworkProxyQuery::setLocalPort(int port)
{
   d->localPort = port;
}

QString QNetworkProxyQuery::protocolTag() const
{
   return d ? d->remote.scheme() : QString();
}

void QNetworkProxyQuery::setProtocolTag(const QString &protocolTag)
{
   d->remote.setScheme(protocolTag);
}

QUrl QNetworkProxyQuery::url() const
{
   return d ? d->remote : QUrl();
}

void QNetworkProxyQuery::setUrl(const QUrl &url)
{
   d->remote = url;
}

#ifndef QT_NO_BEARERMANAGEMENT

QNetworkConfiguration QNetworkProxyQuery::networkConfiguration() const
{
   return d ? d->config : QNetworkConfiguration();
}

void QNetworkProxyQuery::setNetworkConfiguration(const QNetworkConfiguration &networkConfiguration)
{
   d->config = networkConfiguration;
}
#endif

QNetworkProxyFactory::QNetworkProxyFactory()
{
}

QNetworkProxyFactory::~QNetworkProxyFactory()
{
}

void QNetworkProxyFactory::setUseSystemConfiguration(bool enable)
{
   if (enable) {
      setApplicationProxyFactory(new QSystemConfigurationProxyFactory);
   } else {
      setApplicationProxyFactory(nullptr);
   }
}

void QNetworkProxyFactory::setApplicationProxyFactory(QNetworkProxyFactory *factory)
{
   QGlobalNetworkProxy *obj = cs_GlobalNetworkProxy();
   obj->setApplicationProxyFactory(factory);
}

QList<QNetworkProxy> QNetworkProxyFactory::proxyForQuery(const QNetworkProxyQuery &query)
{
   QGlobalNetworkProxy *obj = cs_GlobalNetworkProxy();
   return obj->proxyForQuery(query);
}

QDebug operator<<(QDebug debug, const QNetworkProxy &proxy)
{
   // QDebugStateSaver saver(debug);
   // debug.resetFormat().nospace();

   QNetworkProxy::ProxyType type = proxy.type();

   switch (type) {
      case QNetworkProxy::NoProxy:
         debug << "NoProxy ";
         break;

      case QNetworkProxy::DefaultProxy:
         debug << "DefaultProxy ";
         break;

      case QNetworkProxy::Socks5Proxy:
         debug << "Socks5Proxy ";
         break;

      case QNetworkProxy::HttpProxy:
         debug << "HttpProxy ";
         break;

      case QNetworkProxy::HttpCachingProxy:
         debug << "HttpCachingProxy ";
         break;

      case QNetworkProxy::FtpCachingProxy:
         debug << "FtpCachingProxy ";
         break;

      default:
         debug << "Unknown proxy " << int(type);
         break;
   }

   debug << '"' << proxy.hostName() << ':' << proxy.port() << "\" ";
   QNetworkProxy::Capabilities caps = proxy.capabilities();
   QStringList scaps;

   if (caps & QNetworkProxy::TunnelingCapability) {
      scaps << "Tunnel";
   }

   if (caps & QNetworkProxy::ListeningCapability) {
      scaps << "Listen";
   }

   if (caps & QNetworkProxy::UdpTunnelingCapability) {
      scaps << "UDP";
   }

   if (caps & QNetworkProxy::CachingCapability) {
      scaps << "Caching";
   }

   if (caps & QNetworkProxy::HostNameLookupCapability) {
      scaps << "NameLookup";
   }

   debug << '[' << scaps.join(" ") << ']';
   return debug;
}

#endif // QT_NO_NETWORKPROXY
