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

#ifndef QNETWORKACCESSMANAGER_H
#define QNETWORKACCESSMANAGER_H

#include <QObject>
#include <QNetworkSession>
#include <QStringList>

#ifdef QT_SSL
#include <QSslConfiguration>
#include <QSslPreSharedKeyAuthenticator>
#endif

class QIODevice;
class QAbstractNetworkCache;
class QAuthenticator;
class QByteArray;
class QNetworkCookieJar;
class QNetworkRequest;
class QNetworkReply;
class QNetworkProxy;
class QNetworkProxyFactory;
class QSslError;

#ifndef QT_NO_BEARERMANAGEMENT
class QNetworkConfiguration;
#endif

class QHttpMultiPart;
class QNetworkReplyImplPrivate;
class QNetworkAccessManagerPrivate;

template<typename T> class QList;
class QNetworkCookie;

class Q_NETWORK_EXPORT QNetworkAccessManager: public QObject
{
   NET_CS_OBJECT(QNetworkAccessManager)

#ifndef QT_NO_BEARERMANAGEMENT
   NET_CS_PROPERTY_READ(networkAccessible, networkAccessible)
   NET_CS_PROPERTY_WRITE(networkAccessible, setNetworkAccessible)
   NET_CS_PROPERTY_NOTIFY(networkAccessible, networkAccessibleChanged)
#endif

 public:
   enum Operation {
      HeadOperation = 1,
      GetOperation,
      PutOperation,
      PostOperation,
      DeleteOperation,
      CustomOperation,

      UnknownOperation = 0
   };

#ifndef QT_NO_BEARERMANAGEMENT
   enum NetworkAccessibility {
      UnknownAccessibility = -1,
      NotAccessible = 0,
      Accessible = 1
   };
#endif

   explicit QNetworkAccessManager(QObject *parent = nullptr);
   ~QNetworkAccessManager();
    virtual QStringList supportedSchemes() const;
    void clearAccessCache();

#ifndef QT_NO_NETWORKPROXY
   QNetworkProxy proxy() const;
   void setProxy(const QNetworkProxy &proxy);
   QNetworkProxyFactory *proxyFactory() const;
   void setProxyFactory(QNetworkProxyFactory *factory);
#endif

   QAbstractNetworkCache *cache() const;
   void setCache(QAbstractNetworkCache *cache);

   QNetworkCookieJar *cookieJar() const;
   void setCookieJar(QNetworkCookieJar *cookieJar);

   QNetworkReply *head(const QNetworkRequest &request);
   QNetworkReply *get(const QNetworkRequest &request);
   QNetworkReply *post(const QNetworkRequest &request, QIODevice *data);
   QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data);
   QNetworkReply *post(const QNetworkRequest &request, QHttpMultiPart *multiPart);
   QNetworkReply *put(const QNetworkRequest &request, QIODevice *data);
   QNetworkReply *put(const QNetworkRequest &request, const QByteArray &data);
   QNetworkReply *put(const QNetworkRequest &request, QHttpMultiPart *multiPart);
   QNetworkReply *deleteResource(const QNetworkRequest &request);
   QNetworkReply *sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QIODevice *data = nullptr);

#if ! defined(QT_NO_BEARERMANAGEMENT)
   void setConfiguration(const QNetworkConfiguration &config);
   QNetworkConfiguration configuration() const;
   QNetworkConfiguration activeConfiguration() const;

   void setNetworkAccessible(NetworkAccessibility accessible);
   NetworkAccessibility networkAccessible() const;
#endif

#ifdef QT_SSL
   void connectToHostEncrypted(const QString &hostName, quint16 port = 443,
                  const QSslConfiguration &sslConfiguration = QSslConfiguration::defaultConfiguration());
#endif

   void connectToHost(const QString &hostName, quint16 port = 80);

#ifndef QT_NO_NETWORKPROXY
   NET_CS_SIGNAL_1(Public, void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator))
   NET_CS_SIGNAL_2(proxyAuthenticationRequired, proxy, authenticator)
#endif

   NET_CS_SIGNAL_1(Public, void authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator))
   NET_CS_SIGNAL_2(authenticationRequired, reply, authenticator)

   NET_CS_SIGNAL_1(Public, void finished(QNetworkReply *reply))
   NET_CS_SIGNAL_2(finished, reply)


#ifdef QT_SSL
   NET_CS_SIGNAL_1(Public,  void encrypted(QNetworkReply *reply))
   NET_CS_SIGNAL_2(encrypted, reply)

   NET_CS_SIGNAL_1(Public, void sslErrors(QNetworkReply *reply, const QList <QSslError> &errors))
   NET_CS_SIGNAL_2(sslErrors, reply, errors)

   NET_CS_SIGNAL_1(Public, void preSharedKeyAuthenticationRequired(QNetworkReply *reply, QSslPreSharedKeyAuthenticator *authenticator))
   NET_CS_SIGNAL_2(preSharedKeyAuthenticationRequired, reply, authenticator)
#endif

#ifndef QT_NO_BEARERMANAGEMENT
   NET_CS_SIGNAL_1(Public, void networkSessionConnected())
   NET_CS_SIGNAL_2(networkSessionConnected)

   NET_CS_SIGNAL_1(Public, void networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible))
   NET_CS_SIGNAL_2(networkAccessibleChanged, accessible)
#endif

 protected:
   QScopedPointer<QNetworkAccessManagerPrivate> d_ptr;

   virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData = nullptr);

   NET_CS_SLOT_1(Protected, QStringList supportedSchemesImplementation() const)
   NET_CS_SLOT_2(supportedSchemesImplementation)

 private:
   friend class QNetworkReplyImplPrivate;
   friend class QNetworkReplyHttpImpl;
   friend class QNetworkReplyHttpImplPrivate;

   Q_DECLARE_PRIVATE(QNetworkAccessManager)

   NET_CS_SLOT_1(Private, void _q_replyFinished())
   NET_CS_SLOT_2(_q_replyFinished)

   NET_CS_SLOT_1(Private, void _q_replyEncrypted())
   NET_CS_SLOT_2(_q_replyEncrypted)

#ifdef QT_SSL
   NET_CS_SLOT_1(Private, void _q_replySslErrors(const QList<QSslError> &errList))
   NET_CS_SLOT_2(_q_replySslErrors)

   NET_CS_SLOT_1(Private, void _q_replyPreSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *data))
   NET_CS_SLOT_2(_q_replyPreSharedKeyAuthenticationRequired)
#endif

#ifndef QT_NO_BEARERMANAGEMENT
   NET_CS_SLOT_1(Private, void _q_networkSessionClosed())
   NET_CS_SLOT_2(_q_networkSessionClosed)

   NET_CS_SLOT_1(Private, void _q_networkSessionStateChanged(QNetworkSession::State data))
   NET_CS_SLOT_2(_q_networkSessionStateChanged)

   NET_CS_SLOT_1(Private, void _q_onlineStateChanged(bool))
   NET_CS_SLOT_2(_q_onlineStateChanged)

   NET_CS_SLOT_1(Private, void _q_configurationChanged(const QNetworkConfiguration &data))
   NET_CS_SLOT_2(_q_configurationChanged)

   NET_CS_SLOT_1(Private, void _q_networkSessionFailed(QNetworkSession::SessionError data))
   NET_CS_SLOT_2(_q_networkSessionFailed)
#endif

};

#endif
