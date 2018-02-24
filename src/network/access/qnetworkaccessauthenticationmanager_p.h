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

#ifndef QNETWORKACCESSAUTHENTICATIONMANAGER_P_H
#define QNETWORKACCESSAUTHENTICATIONMANAGER_P_H

#include <qnetworkaccessmanager.h>
#include <qnetworkaccesscache_p.h>
#include <qnetworkaccessbackend_p.h>
#include <qnetworkproxy.h>
#include <QMutex>

class QAuthenticator;
class QAbstractNetworkCache;
class QNetworkAuthenticationCredential;
class QNetworkCookieJar;

class QNetworkAuthenticationCredential
{
 public:
   QString domain;
   QString user;
   QString password;
   bool isNull() const {
      return domain.isNull() && user.isNull() && password.isNull();
   }
};
Q_DECLARE_TYPEINFO(QNetworkAuthenticationCredential, Q_MOVABLE_TYPE);
inline bool operator<(const QNetworkAuthenticationCredential &t1, const QString &t2)
{
   return t1.domain < t2;
}
inline bool operator<(const QString &t1, const QNetworkAuthenticationCredential &t2)
{
   return t1 < t2.domain;
}

inline bool operator<(const QNetworkAuthenticationCredential &t1, const QNetworkAuthenticationCredential &t2)
{
   return t1.domain < t2.domain;
}

class QNetworkAccessAuthenticationManager
{
 public:
   QNetworkAccessAuthenticationManager() { };

   void cacheCredentials(const QUrl &url, const QAuthenticator *auth);
   QNetworkAuthenticationCredential fetchCachedCredentials(const QUrl &url,
         const QAuthenticator *auth = 0);

#ifndef QT_NO_NETWORKPROXY
   void cacheProxyCredentials(const QNetworkProxy &proxy, const QAuthenticator *auth);
   QNetworkAuthenticationCredential fetchCachedProxyCredentials(const QNetworkProxy &proxy,
         const QAuthenticator *auth = 0);
#endif

   void clearCache();

 protected:
   QNetworkAccessCache authenticationCache;
   QMutex mutex;
};

#endif
