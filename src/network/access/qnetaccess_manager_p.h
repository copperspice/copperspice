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

#ifndef QNETWORK_ACCESS_MANAGER_P_H
#define QNETWORK_ACCESS_MANAGER_P_H

#include <qnetaccess_manager.h>
#include <qnetworkproxy.h>
#include <qnetworksession.h>

#ifndef QT_NO_BEARERMANAGEMENT
#include <qnetworkconfigmanager.h>
#endif

#include <qnetaccess_cache_p.h>
#include <qnetaccess_backend_p.h>
#include <qnetaccess_authenticationmanager_p.h>

class QAuthenticator;
class QAbstractNetworkCache;
class QNetworkAuthenticationCredential;
class QNetworkCookieJar;
class QSslPreSharedKeyAuthenticator;

class QNetworkAccessManagerPrivate
{
 public:
   QNetworkAccessManagerPrivate()
      : networkCache(nullptr), cookieJar(nullptr), httpThread(nullptr),

#ifndef QT_NO_NETWORKPROXY
        proxyFactory(nullptr),
#endif

#ifndef QT_NO_BEARERMANAGEMENT
        lastSessionState(QNetworkSession::Invalid),
        networkConfiguration(networkConfigurationManager.defaultConfiguration()),
        customNetworkConfiguration(false),
        networkSessionRequired(networkConfigurationManager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired),
        activeReplyCount(0), online(false), initializeSession(true),
#endif
        cookieJarCreated(false), defaultAccessControl(true),
        authenticationManager(QSharedPointer<QNetworkAccessAuthenticationManager>::create())
    {

#ifndef QT_NO_BEARERMANAGEMENT
        online = (networkConfiguration.state().testFlag(QNetworkConfiguration::Active));

        if (online) {
            networkAccessible = QNetworkAccessManager::Accessible;
        } else if (networkConfiguration.state().testFlag(QNetworkConfiguration::Undefined)) {
            networkAccessible = QNetworkAccessManager::UnknownAccessibility;
        } else {
            networkAccessible = QNetworkAccessManager::NotAccessible;
        }
#endif
   }

   virtual ~QNetworkAccessManagerPrivate();

   void _q_replyFinished();
   void _q_replyEncrypted();
   void _q_replySslErrors(const QList<QSslError> &errorList);
   void _q_replyPreSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *authenticator);

   QNetworkReply *postProcess(QNetworkReply *reply);
   void createCookieJar() const;

   void authenticationRequired(QAuthenticator *authenticator, QNetworkReply *reply,
                  bool synchronous, QUrl &url, QUrl *urlForLastAuthentication, bool allowAuthenticationReuse = true);

   void cacheCredentials(const QUrl &url, const QAuthenticator *auth);
   QNetworkAuthenticationCredential *fetchCachedCredentials(const QUrl &url, const QAuthenticator *auth = nullptr);

#ifndef QT_NO_NETWORKPROXY
    void proxyAuthenticationRequired(const QUrl &url, const QNetworkProxy &proxy, bool synchronous,
                  QAuthenticator *authenticator, QNetworkProxy *lastProxyAuthentication);

    void cacheProxyCredentials(const QNetworkProxy &proxy, const QAuthenticator *auth);
    QNetworkAuthenticationCredential *fetchCachedProxyCredentials(const QNetworkProxy &proxy, const QAuthenticator *auth = nullptr);
    QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery &query);
#endif

   QNetworkAccessBackend *findBackend(QNetworkAccessManager::Operation op, const QNetworkRequest &request);
   QStringList backendSupportedSchemes() const;

#ifndef QT_NO_BEARERMANAGEMENT
   void createSession(const QNetworkConfiguration &config);
   QSharedPointer<QNetworkSession> getNetworkSession() const;

   void _q_networkSessionClosed();
   void _q_networkSessionNewConfigurationActivated();
   void _q_networkSessionPreferredConfigurationChanged(const QNetworkConfiguration &config, bool isSeamless);
   void _q_networkSessionStateChanged(QNetworkSession::State state);
   void _q_onlineStateChanged(bool isOnline);
   void _q_configurationChanged(const QNetworkConfiguration &configuration);
   void _q_networkSessionFailed(QNetworkSession::SessionError error);

    QSet<QString> onlineConfigurations;
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
   QNetworkConfigurationManager networkConfigurationManager;
   QNetworkConfiguration networkConfiguration;

   bool customNetworkConfiguration;
   bool networkSessionRequired;
   QNetworkAccessManager::NetworkAccessibility networkAccessible;
   int activeReplyCount;
   bool online;
   bool initializeSession;
#endif

   bool cookieJarCreated;
   bool defaultAccessControl;

   // The cache with authorization data:
   QSharedPointer<QNetworkAccessAuthenticationManager> authenticationManager;

   // this cache can be used by individual backends to cache e.g. their TCP connections to a server
   // and use the connections for multiple requests
   QNetworkAccessCache objectCache;

   static QNetworkAccessCache *getObjectCache(QNetworkAccessBackend *backend) {
      return &backend->manager->objectCache;
   }

   static void clearCache(QNetworkAccessManager *manager);

#ifndef QT_NO_BEARERMANAGEMENT
   static const QWeakPointer<const QNetworkSession> getNetworkSession(const QNetworkAccessManager *manager);
#endif

   Q_DECLARE_PUBLIC(QNetworkAccessManager)

 protected:
   QNetworkAccessManager *q_ptr;
};

#endif
