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

#include <qnetworkproxy.h>

#include <QtCore/QByteArray>
#include <QtCore/QUrl>

#ifndef QT_NO_NETWORKPROXY

/*
 * Construct a proxy from the environment variable http_proxy.
 * Or no system proxy. Just return a list with NoProxy.
 */

QT_BEGIN_NAMESPACE

QList<QNetworkProxy> QNetworkProxyFactory::systemProxyForQuery(const QNetworkProxyQuery &query)
{
   QList<QNetworkProxy> proxyList;

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
      } else if (url.scheme() == QLatin1String("http") || url.scheme().isEmpty()) {
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
