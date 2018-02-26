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

#include <qnetworkproxy.h>

#ifndef QT_NO_NETWORKPROXY

#include <qnetworkproxy_p.h>
#include <qnetworkrequest_p.h>
#include <qsocks5socketengine_p.h>
#include <qhttpsocketengine_p.h>
#include <qauthenticator.h>
#include <qdebug.h>
#include <qmutex.h>
#include <qstringlist.h>
#include <qurl.h>

#ifndef QT_NO_BEARERMANAGEMENT
#include <QtNetwork/QNetworkConfiguration>
#endif

class QSocks5SocketEngineHandler;
class QHttpSocketEngineHandler;

class QGlobalNetworkProxy
{
 public:
   QGlobalNetworkProxy()
      : mutex(QMutex::Recursive)
      , applicationLevelProxy(0)
      , applicationLevelProxyFactory(0)
      , socks5SocketEngineHandler(0)
      , httpSocketEngineHandler(0) {
#ifdef QT_USE_SYSTEM_PROXIES
      setApplicationProxyFactory(new QSystemConfigurationProxyFactory);
#endif


      socks5SocketEngineHandler = new QSocks5SocketEngineHandler();



      httpSocketEngineHandler = new QHttpSocketEngineHandler();

   }

   ~QGlobalNetworkProxy() {
      delete applicationLevelProxy;
      delete applicationLevelProxyFactory;
      delete socks5SocketEngineHandler;
      delete httpSocketEngineHandler;
   }

   void setApplicationProxy(const QNetworkProxy &proxy) {
      QMutexLocker lock(&mutex);
      if (!applicationLevelProxy) {
         applicationLevelProxy = new QNetworkProxy;
      }
      *applicationLevelProxy = proxy;
      delete applicationLevelProxyFactory;
      applicationLevelProxyFactory = 0;
   }

   void setApplicationProxyFactory(QNetworkProxyFactory *factory) {
      QMutexLocker lock(&mutex);

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
   QMutex mutex;
   QNetworkProxy *applicationLevelProxy;
   QNetworkProxyFactory *applicationLevelProxyFactory;
   QSocks5SocketEngineHandler *socks5SocketEngineHandler;
   QHttpSocketEngineHandler *httpSocketEngineHandler;
};

QList<QNetworkProxy> QGlobalNetworkProxy::proxyForQuery(const QNetworkProxyQuery &query)
{
   QMutexLocker locker(&mutex);

   QList<QNetworkProxy> result;

   // don't look for proxies for a local connection
   QHostAddress parsed;
   QString hostname = query.url().host();
   if (hostname == QLatin1String("localhost")
         || hostname.startsWith(QLatin1String("localhost."))
         || (parsed.setAddress(hostname)
             && (parsed.isLoopback()))) {

      result << QNetworkProxy(QNetworkProxy::NoProxy);
      return result;
   }

   if (!applicationLevelProxyFactory) {
      if (applicationLevelProxy
            && applicationLevelProxy->type() != QNetworkProxy::DefaultProxy) {
         result << *applicationLevelProxy;
      } else {
         result << QNetworkProxy(QNetworkProxy::NoProxy);
      }
      return result;
   }

   // we have a factory
   result = applicationLevelProxyFactory->queryProxy(query);
   if (result.isEmpty()) {
      qWarning("QNetworkProxyFactory: factory %p has returned an empty result set",
               applicationLevelProxyFactory);
      result << QNetworkProxy(QNetworkProxy::NoProxy);
   }
   return result;
}

Q_GLOBAL_STATIC(QGlobalNetworkProxy, globalNetworkProxy)

namespace {
template<bool> struct StaticAssertTest;
template<> struct StaticAssertTest<true> {
   enum { Value = 1 };
};
}

static inline void qt_noop_with_arg(int) {}
#define q_static_assert(expr)   qt_noop_with_arg(sizeof(StaticAssertTest< expr >::Value))

static QNetworkProxy::Capabilities defaultCapabilitiesForType(QNetworkProxy::ProxyType type)
{
   q_static_assert(int(QNetworkProxy::DefaultProxy) == 0);
   q_static_assert(int(QNetworkProxy::FtpCachingProxy) == 5);

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

   inline QNetworkProxyPrivate(QNetworkProxy::ProxyType t = QNetworkProxy::DefaultProxy, const QString &h = QString(),
                               quint16 p = 0, const QString &u = QString(), const QString &pw = QString())

      : hostName(h),
        user(u),
        password(pw),
        capabilities(defaultCapabilitiesForType(t)),
        port(p),
        type(t),
        capabilitiesSet(false) {
   }

   inline bool operator==(const QNetworkProxyPrivate &other) const {
      return type == other.type && port == other.port &&
             hostName == other.hostName && user == other.user &&
             password == other.password && capabilities == other.capabilities;
   }
};

template<> void QSharedDataPointer<QNetworkProxyPrivate>::detach()
{
   if (d && d->ref.load() == 1) {
      return;
   }
   QNetworkProxyPrivate *x = (d ? new QNetworkProxyPrivate(*d)
                              : new QNetworkProxyPrivate);
   x->ref.ref();
   if (d && !d->ref.deref()) {
      delete d;
   }
   d = x;
}

QNetworkProxy::QNetworkProxy()
   : d(0)
{
   // make sure we have QGlobalNetworkProxy singleton created, otherwise
   // you don't have any socket engine handler created when directly setting
   // a proxy to the socket
   globalNetworkProxy();
}

/*!
    Constructs a QNetworkProxy with \a type, \a hostName, \a port,
    \a user and \a password.

    The default capabilities for proxy type \a type are set automatically.

    \sa capabilities()
*/
QNetworkProxy::QNetworkProxy(ProxyType type, const QString &hostName, quint16 port,
                             const QString &user, const QString &password)
   : d(new QNetworkProxyPrivate(type, hostName, port, user, password))
{
   // make sure we have QGlobalNetworkProxy singleton created, otherwise you don't have
   // any socket engine handler created when directly setting a proxy to a socket
   globalNetworkProxy();
}

/*!
    Constructs a copy of \a other.
*/
QNetworkProxy::QNetworkProxy(const QNetworkProxy &other)
   : d(other.d)
{
}

QNetworkProxy::~QNetworkProxy()
{
   // QSharedDataPointer takes care of deleting for us
}

/*!
    \since 4.4

    Compares the value of this network proxy to \a other and returns true
    if they are equal (same proxy type, server as well as username and password)
*/
bool QNetworkProxy::operator==(const QNetworkProxy &other) const
{
   return d == other.d || (d && other.d && *d == *other.d);
}

/*!
    \fn bool QNetworkProxy::operator!=(const QNetworkProxy &other) const
    \since 4.4

    Compares the value of this network proxy to \a other and returns true
    if they differ.
\*/

/*!
    \since 4.2

    Assigns the value of the network proxy \a other to this network proxy.
*/
QNetworkProxy &QNetworkProxy::operator=(const QNetworkProxy &other)
{
   d = other.d;
   return *this;
}

/*!
    Sets the proxy type for this instance to be \a type.

    Note that changing the type of a proxy does not change
    the set of capabilities this QNetworkProxy object holds if any
    capabilities have been set with setCapabilities().

    \sa type(), setCapabilities()
*/
void QNetworkProxy::setType(QNetworkProxy::ProxyType type)
{
   d->type = type;
   if (!d->capabilitiesSet) {
      d->capabilities = defaultCapabilitiesForType(type);
   }
}

/*!
    Returns the proxy type for this instance.

    \sa setType()
*/
QNetworkProxy::ProxyType QNetworkProxy::type() const
{
   return d ? d->type : DefaultProxy;
}

/*!
    \since 4.5

    Sets the capabilities of this proxy to \a capabilities.

    \sa setType(), capabilities()
*/
void QNetworkProxy::setCapabilities(Capabilities capabilities)
{
   d->capabilities = capabilities;
   d->capabilitiesSet = true;
}

/*!
    \since 4.5

    Returns the capabilities of this proxy server.

    \sa setCapabilities(), type()
*/
QNetworkProxy::Capabilities QNetworkProxy::capabilities() const
{
   return d ? d->capabilities : defaultCapabilitiesForType(DefaultProxy);
}

/*!
    \since 4.4

    Returns true if this proxy supports the
    QNetworkProxy::CachingCapability capability.

    In Qt 4.4, the capability was tied to the proxy type, but since Qt
    4.5 it is possible to remove the capability of caching from a
    proxy by calling setCapabilities().

    \sa capabilities(), type(), isTransparentProxy()
*/
bool QNetworkProxy::isCachingProxy() const
{
   return capabilities() & CachingCapability;
}

/*!
    \since 4.4

    Returns true if this proxy supports transparent tunneling of TCP
    connections. This matches the QNetworkProxy::TunnelingCapability
    capability.

    In Qt 4.4, the capability was tied to the proxy type, but since Qt
    4.5 it is possible to remove the capability of caching from a
    proxy by calling setCapabilities().

    \sa capabilities(), type(), isCachingProxy()
*/
bool QNetworkProxy::isTransparentProxy() const
{
   return capabilities() & TunnelingCapability;
}

/*!
    Sets the user name for proxy authentication to be \a user.

    \sa user(), setPassword(), password()
*/
void QNetworkProxy::setUser(const QString &user)
{
   d->user = user;
}

/*!
    Returns the user name used for authentication.

    \sa setUser(), setPassword(), password()
*/
QString QNetworkProxy::user() const
{
   return d ? d->user : QString();
}

/*!
    Sets the password for proxy authentication to be \a password.

    \sa user(), setUser(), password()
*/
void QNetworkProxy::setPassword(const QString &password)
{
   d->password = password;
}

/*!
    Returns the password used for authentication.

    \sa user(), setPassword(), setUser()
*/
QString QNetworkProxy::password() const
{
   return d ? d->password : QString();
}

/*!
    Sets the host name of the proxy host to be \a hostName.

    \sa hostName(), setPort(), port()
*/
void QNetworkProxy::setHostName(const QString &hostName)
{
   d->hostName = hostName;
}

/*!
    Returns the host name of the proxy host.

    \sa setHostName(), setPort(), port()
*/
QString QNetworkProxy::hostName() const
{
   return d ? d->hostName : QString();
}

/*!
    Sets the port of the proxy host to be \a port.

    \sa hostName(), setHostName(), port()
*/
void QNetworkProxy::setPort(quint16 port)
{
   d->port = port;
}

/*!
    Returns the port of the proxy host.

    \sa setHostName(), setPort(), hostName()
*/
quint16 QNetworkProxy::port() const
{
   return d ? d->port : 0;
}

void QNetworkProxy::setApplicationProxy(const QNetworkProxy &networkProxy)
{
   if (globalNetworkProxy()) {
      // don't accept setting the proxy to DefaultProxy
      if (networkProxy.type() == DefaultProxy) {
         globalNetworkProxy()->setApplicationProxy(QNetworkProxy::NoProxy);
      } else {
         globalNetworkProxy()->setApplicationProxy(networkProxy);
      }
   }
}

QNetworkProxy QNetworkProxy::applicationProxy()
{
   if (globalNetworkProxy()) {
      return globalNetworkProxy()->applicationProxy();
   }
   return QNetworkProxy();
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
   QNetworkHeadersPrivate::RawHeadersList::ConstIterator it =
      d->headers.findRawHeader(headerName);
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

template<> void QSharedDataPointer<QNetworkProxyQueryPrivate>::detach()
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

/*!
    Constructs a QNetworkProxyQuery with the URL \a requestUrl and
    sets the query type to \a queryType.

    \sa protocolTag(), peerHostName(), peerPort()
*/
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
      setApplicationProxyFactory(0);
   }
}

void QNetworkProxyFactory::setApplicationProxyFactory(QNetworkProxyFactory *factory)
{
   if (globalNetworkProxy()) {
      globalNetworkProxy()->setApplicationProxyFactory(factory);
   }
}

QList<QNetworkProxy> QNetworkProxyFactory::proxyForQuery(const QNetworkProxyQuery &query)
{
   if (!globalNetworkProxy()) {
      return QList<QNetworkProxy>() << QNetworkProxy(QNetworkProxy::NoProxy);
   }
   return globalNetworkProxy()->proxyForQuery(query);
}

QDebug operator<<(QDebug debug, const QNetworkProxy &proxy)
{
   //   QDebugStateSaver saver(debug);
   //   debug.resetFormat().nospace();

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
