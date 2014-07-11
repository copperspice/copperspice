/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QNETWORKACCESSMANAGER_P_H
#define QNETWORKACCESSMANAGER_P_H

#include <qnetworkaccessmanager.h>
#include <qnetworkaccesscache_p.h>
#include <qnetworkaccessbackend_p.h>
#include <QtNetwork/qnetworkproxy.h>
#include <QtNetwork/qnetworksession.h>
#include <qnetworkaccessauthenticationmanager_p.h>

QT_BEGIN_NAMESPACE

class QAuthenticator;
class QAbstractNetworkCache;
class QNetworkAuthenticationCredential;
class QNetworkCookieJar;

class QNetworkAccessManagerPrivate
{

 public:
   QNetworkAccessManagerPrivate()
      : networkCache(0), cookieJar(0),
        httpThread(0),
#ifndef QT_NO_NETWORKPROXY
        proxyFactory(0),
#endif
#ifndef QT_NO_BEARERMANAGEMENT
        lastSessionState(QNetworkSession::Invalid),
        networkAccessible(QNetworkAccessManager::Accessible),
        activeReplyCount(0),
        online(false),
        initializeSession(true),
#endif
        cookieJarCreated(false),
        authenticationManager(new QNetworkAccessAuthenticationManager) {
   }

   virtual ~QNetworkAccessManagerPrivate();

   void _q_replyFinished();
   void _q_replySslErrors(const QList<QSslError> &errors);
   QNetworkReply *postProcess(QNetworkReply *reply);
   void createCookieJar() const;

   void authenticationRequired(QNetworkAccessBackend *backend, QAuthenticator *authenticator);
   void cacheCredentials(const QUrl &url, const QAuthenticator *auth);
   QNetworkAuthenticationCredential *fetchCachedCredentials(const QUrl &url,
         const QAuthenticator *auth = 0);

#ifndef QT_NO_NETWORKPROXY
   void proxyAuthenticationRequired(QNetworkAccessBackend *backend, const QNetworkProxy &proxy,
                                    QAuthenticator *authenticator);
   void cacheProxyCredentials(const QNetworkProxy &proxy, const QAuthenticator *auth);
   QNetworkAuthenticationCredential *fetchCachedProxyCredentials(const QNetworkProxy &proxy,
         const QAuthenticator *auth = 0);
   QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery &query);
#endif

   QNetworkAccessBackend *findBackend(QNetworkAccessManager::Operation op, const QNetworkRequest &request);

#ifndef QT_NO_BEARERMANAGEMENT
   void createSession(const QNetworkConfiguration &config);
   QSharedPointer<QNetworkSession> getNetworkSession() const;

   void _q_networkSessionClosed();
   void _q_networkSessionNewConfigurationActivated();
   void _q_networkSessionPreferredConfigurationChanged(const QNetworkConfiguration &config,
         bool isSeamless);
   void _q_networkSessionStateChanged(QNetworkSession::State state);
#endif

   QNetworkRequest prepareMultipart(const QNetworkRequest &request, QHttpMultiPart *multiPart);

   // this is the cache for storing downloaded files
   QAbstractNetworkCache *networkCache;

   QNetworkCookieJar *cookieJar;

   QThread *httpThread;

#ifndef QT_NO_NETWORKPROXY
   QNetworkProxy proxy;
   QNetworkProxyFactory *proxyFactory;
#endif

#ifndef QT_NO_BEARERMANAGEMENT
   QSharedPointer<QNetworkSession> networkSessionStrongRef;
   QWeakPointer<QNetworkSession> networkSessionWeakRef;
   QNetworkSession::State lastSessionState;
   QString networkConfiguration;
   QNetworkAccessManager::NetworkAccessibility networkAccessible;
   int activeReplyCount;
   bool online;
   bool initializeSession;
#endif

   bool cookieJarCreated;

   // The cache with authorization data:
   QSharedPointer<QNetworkAccessAuthenticationManager> authenticationManager;

   // this cache can be used by individual backends to cache e.g. their TCP connections to a server
   // and use the connections for multiple requests.
   QNetworkAccessCache objectCache;
   static inline QNetworkAccessCache *getObjectCache(QNetworkAccessBackend *backend) {
      return &backend->manager->objectCache;
   }
   static void clearCache(QNetworkAccessManager *manager);

   Q_DECLARE_PUBLIC(QNetworkAccessManager)

 protected:
   QNetworkAccessManager *q_ptr;

};

QT_END_NAMESPACE

#endif
