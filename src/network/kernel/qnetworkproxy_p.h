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

#ifndef QNETWORKPROXY_P_H
#define QNETWORKPROXY_P_H

#ifndef QT_NO_NETWORKPROXY



class QSystemConfigurationProxyFactory : public QNetworkProxyFactory
{
 public:
   QSystemConfigurationProxyFactory() : QNetworkProxyFactory() {}

   virtual QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery &query)  {
      QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(query);

      // Make sure NoProxy is in the list, so that QTcpServer can work:
      // it searches for the first proxy that can has the ListeningCapability capability
      // if none have (as is the case with HTTP proxies), it fails to bind.
      // NoProxy allows it to fallback to the 'no proxy' case and bind.
      proxies.append(QNetworkProxy::NoProxy);

      return proxies;
   }
};



#endif // QT_NO_NETWORKINTERFACE

#endif

