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

#include <QtCore/QByteArray>
#include <QtCore/QUrl>

#ifndef QT_NO_NETWORKPROXY

/*
 * Construct a proxy from the environment variable http_proxy.
 * Or no system proxy. Just return a list with NoProxy.
 */

QT_BEGIN_NAMESPACE

static bool ignoreProxyFor(const QNetworkProxyQuery &query)
{
   const QByteArray noProxy = qgetenv("no_proxy").trimmed();

   if (noProxy.isEmpty()) {
      return false;
   }

   const QList<QByteArray> noProxyTokens = noProxy.split(',');
   for (const QByteArray &rawToken : noProxyTokens) {
      QByteArray token = rawToken.trimmed();
      QString peerHostName = query.peerHostName();
      if (token.startsWith('*')) {
         token = token.mid(1);
      }

      // Harmonize trailing dot notation
      if (token.endsWith('.') && !peerHostName.endsWith('.')) {
         token = token.left(token.length() - 1);
      }

      // We prepend a dot to both values, so that when we do a suffix match,
      // we don't match "donotmatch.com" with "match.com"
      if (!token.startsWith('.')) {
         token.prepend('.');
      }

      if (!peerHostName.startsWith('.')) {
         peerHostName.prepend('.');
      }

      if (peerHostName.endsWith(QString::fromLatin1(token))) {
         return true;
      }
   }

   return false;
}
QList<QNetworkProxy> QNetworkProxyFactory::systemProxyForQuery(const QNetworkProxyQuery &query)
{
   QList<QNetworkProxy> proxyList;

   if (ignoreProxyFor(query)) {
      return proxyList << QNetworkProxy::NoProxy;
   }
   const QString queryProtocol = query.protocolTag().toLower();
   QByteArray proxy_env;

   if (queryProtocol == QLatin1String("http")) {
      proxy_env = qgetenv("http_proxy");
   } else if (queryProtocol == QLatin1String("https")) {
      proxy_env = qgetenv("https_proxy");
   } else if (queryProtocol == QLatin1String("ftp")) {
      proxy_env = qgetenv("ftp_proxy");
   } else {
      proxy_env = qgetenv("all_proxy");
   }

   // Fallback to http_proxy is no protocol specific proxy was found
   if (proxy_env.isEmpty()) {
      proxy_env = qgetenv("http_proxy");
   }

   if (!proxy_env.isEmpty()) {
      QUrl url = QUrl(QString::fromLocal8Bit(proxy_env));
      if (url.scheme() == QLatin1String("socks5")) {
         QNetworkProxy proxy(QNetworkProxy::Socks5Proxy, url.host(),
                             url.port() ? url.port() : 1080, url.userName(), url.password());
         proxyList << proxy;

      } else if (url.scheme() == QLatin1String("socks5h")) {
         QNetworkProxy proxy(QNetworkProxy::Socks5Proxy, url.host(),
                             url.port() ? url.port() : 1080, url.userName(), url.password());
         proxy.setCapabilities(QNetworkProxy::HostNameLookupCapability);
         proxyList << proxy;

      } else if ((url.scheme() == QLatin1String("http") || url.scheme().isEmpty())
                 && query.queryType() != QNetworkProxyQuery::UdpSocket
                 && query.queryType() != QNetworkProxyQuery::TcpServer) {

         QNetworkProxy proxy(QNetworkProxy::HttpProxy, url.host(),
                             url.port() ? url.port() : 8080, url.userName(), url.password());
         proxyList << proxy;
      }
   }

   if (proxyList.isEmpty()) {
      proxyList << QNetworkProxy::NoProxy;
   }

   return proxyList;
}

QT_END_NAMESPACE

#endif
