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

#include <algorithm>

#include <qnetworkaccessmanager.h>
#include <qnetworkaccessmanager_p.h>
#include <qnetworkaccessauthenticationmanager_p.h>
#include <qsslerror.h>

#include <qauthenticator.h>
#include <qbuffer.h>
#include <qmutex.h>
#include <qurl.h>
#include <qvector.h>

class QNetworkAuthenticationCache: private QVector<QNetworkAuthenticationCredential>, public QNetworkAccessCache::CacheableObject
{
 public:
   QNetworkAuthenticationCache() {
      setExpires(false);
      setShareable(true);
      reserve(1);
   }

   QNetworkAuthenticationCredential *findClosestMatch(const QString &domain) {
      iterator it = std::lower_bound(begin(), end(), domain);

      if (it == end() && !isEmpty()) {
         --it;
      }

      if (it == end() || ! domain.startsWith(it->domain)) {
         return 0;
      }
      return &*it;
   }

   void insert(const QString &domain, const QString &user, const QString &password) {
      QNetworkAuthenticationCredential *closestMatch = findClosestMatch(domain);

      if (closestMatch && closestMatch->domain == domain) {
         // overriding the current credentials
         closestMatch->user = user;
         closestMatch->password = password;

      } else {
         QNetworkAuthenticationCredential newCredential;
         newCredential.domain = domain;
         newCredential.user = user;
         newCredential.password = password;

         if (closestMatch) {
            auto tmp = begin() + (++closestMatch - data());
            QVector<QNetworkAuthenticationCredential>::insert(tmp, newCredential);

         } else {
            QVector<QNetworkAuthenticationCredential>::insert(end(), newCredential);
         }
      }
   }

   void dispose() override {
      delete this;
   }
};

#ifndef QT_NO_NETWORKPROXY
static QByteArray proxyAuthenticationKey(const QNetworkProxy &proxy, const QString &realm)
{
   QUrl key;

   switch (proxy.type()) {
      case QNetworkProxy::Socks5Proxy:
         key.setScheme(QLatin1String("proxy-socks5"));
         break;

      case QNetworkProxy::HttpProxy:
      case QNetworkProxy::HttpCachingProxy:
         key.setScheme(QLatin1String("proxy-http"));
         break;

      case QNetworkProxy::FtpCachingProxy:
         key.setScheme(QLatin1String("proxy-ftp"));
         break;

      case QNetworkProxy::DefaultProxy:
      case QNetworkProxy::NoProxy:
         // shouldn't happen
         return QByteArray();

         // no default:
         // let there be errors if a new proxy type is added in the future
   }

   if (key.scheme().isEmpty()) {
      // proxy type not handled
      return QByteArray();
   }

   key.setUserName(proxy.user());
   key.setHost(proxy.hostName());
   key.setPort(proxy.port());
   key.setFragment(realm);
   return "auth:" + key.toEncoded();
}
#endif

static inline QByteArray authenticationKey(const QUrl &url, const QString &realm)
{
   QUrl copy = url;
   copy.setFragment(realm);
   return "auth:" + copy.toEncoded(QUrl::RemovePassword | QUrl::RemovePath | QUrl::RemoveQuery);
}


#ifndef QT_NO_NETWORKPROXY
void QNetworkAccessAuthenticationManager::cacheProxyCredentials(const QNetworkProxy &p,
      const QAuthenticator *authenticator)
{
   Q_ASSERT(authenticator);
   Q_ASSERT(p.type() != QNetworkProxy::DefaultProxy);
   Q_ASSERT(p.type() != QNetworkProxy::NoProxy);

   QMutexLocker mutexLocker(&mutex);

   QString realm = authenticator->realm();
   QNetworkProxy proxy = p;
   proxy.setUser(authenticator->user());

   // don't cache null passwords, empty password may be valid though
   if (authenticator->password().isEmpty()) {
      return;
   }

   // Set two credentials: one with the username and one without
   do {
      // Set two credentials actually: one with and one without the realm
      do {
         QByteArray cacheKey = proxyAuthenticationKey(proxy, realm);
         if (cacheKey.isEmpty()) {
            return;   // should not happen
         }

         QNetworkAuthenticationCache *auth = new QNetworkAuthenticationCache;
         auth->insert(QString(), authenticator->user(), authenticator->password());
         authenticationCache.addEntry(cacheKey, auth); // replace the existing one, if there's any

         if (realm.isEmpty()) {
            break;
         } else {
            realm.clear();
         }
      } while (true);

      if (proxy.user().isEmpty()) {
         break;
      } else {
         proxy.setUser(QString());
      }
   } while (true);
}

QNetworkAuthenticationCredential
QNetworkAccessAuthenticationManager::fetchCachedProxyCredentials(const QNetworkProxy &p,
      const QAuthenticator *authenticator)
{
   QNetworkProxy proxy = p;
   if (proxy.type() == QNetworkProxy::DefaultProxy) {
      proxy = QNetworkProxy::applicationProxy();
   }
   if (!proxy.password().isEmpty()) {
      return QNetworkAuthenticationCredential();   // no need to set credentials if it already has them
   }

   QString realm;
   if (authenticator) {
      realm = authenticator->realm();
   }

   QMutexLocker mutexLocker(&mutex);
   QByteArray cacheKey = proxyAuthenticationKey(proxy, realm);
   if (cacheKey.isEmpty()) {
      return QNetworkAuthenticationCredential();
   }
   if (!authenticationCache.hasEntry(cacheKey)) {
      return QNetworkAuthenticationCredential();
   }

   QNetworkAuthenticationCache *auth     = static_cast<QNetworkAuthenticationCache *>(authenticationCache.requestEntryNow(cacheKey));
   QNetworkAuthenticationCredential cred = *auth->findClosestMatch(QString());
   authenticationCache.releaseEntry(cacheKey);

   // proxy cache credentials always have exactly one item
   Q_ASSERT_X(!cred.isNull(), "QNetworkAccessManager",
              "Internal inconsistency: found a cache key for a proxy, but it is empty");
   return cred;
}

#endif

void QNetworkAccessAuthenticationManager::cacheCredentials(const QUrl &url,
      const QAuthenticator *authenticator)
{
   Q_ASSERT(authenticator);

   if (authenticator->isNull()) {
      return;
   }

   QString domain = QString::fromLatin1("/"); // FIXME: make QAuthenticator return the domain
   QString realm = authenticator->realm();

   QMutexLocker mutexLocker(&mutex);

   // Set two credentials actually: one with and one without the username in the URL
   QUrl copy = url;
   copy.setUserName(authenticator->user());
   do {
      QByteArray cacheKey = authenticationKey(copy, realm);
      if (authenticationCache.hasEntry(cacheKey)) {
         QNetworkAuthenticationCache *auth =
            static_cast<QNetworkAuthenticationCache *>(authenticationCache.requestEntryNow(cacheKey));
         auth->insert(domain, authenticator->user(), authenticator->password());
         authenticationCache.releaseEntry(cacheKey);
      } else {
         QNetworkAuthenticationCache *auth = new QNetworkAuthenticationCache;
         auth->insert(domain, authenticator->user(), authenticator->password());
         authenticationCache.addEntry(cacheKey, auth);
      }

      if (copy.userName().isEmpty()) {
         break;
      } else {
         copy.setUserName(QString());
      }
   } while (true);
}

/*!
    Fetch the credential data from the credential cache.

    If auth is 0 (as it is when called from createRequest()), this will try to
    look up with an empty realm. That fails in most cases for HTTP (because the
    realm is seldom empty for HTTP challenges). In any case, QHttpNetworkConnection
    never sends the credentials on the first attempt: it needs to find out what
    authentication methods the server supports.

    For FTP, realm is always empty.
*/
QNetworkAuthenticationCredential
QNetworkAccessAuthenticationManager::fetchCachedCredentials(const QUrl &url,
      const QAuthenticator *authentication)
{
   if (!url.password().isEmpty()) {
      return QNetworkAuthenticationCredential();   // no need to set credentials if it already has them
   }

   QString realm;
   if (authentication) {
      realm = authentication->realm();
   }

   QByteArray cacheKey = authenticationKey(url, realm);

   QMutexLocker mutexLocker(&mutex);
   if (!authenticationCache.hasEntry(cacheKey)) {
      return QNetworkAuthenticationCredential();
   }

   QNetworkAuthenticationCache *auth      = static_cast<QNetworkAuthenticationCache *>(authenticationCache.requestEntryNow(cacheKey));
   QNetworkAuthenticationCredential *cred = auth->findClosestMatch(url.path());
   QNetworkAuthenticationCredential ret;

   if (cred) {
      ret = *cred;
   }

   authenticationCache.releaseEntry(cacheKey);
   return ret;
}

void QNetworkAccessAuthenticationManager::clearCache()
{
   authenticationCache.clear();
}


