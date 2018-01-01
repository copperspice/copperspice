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

#ifndef PROXYCONF_H
#define PROXYCONF_H

#include <QString>
#include <QNetworkProxy>

namespace Maemo {

class ProxyConfPrivate;
class ProxyConf {
private:
    ProxyConfPrivate *d_ptr;

public:
    ProxyConf();
    virtual ~ProxyConf();

    QList<QNetworkProxy> flush(const QNetworkProxyQuery &query = QNetworkProxyQuery());  // read the proxies from db
    void readProxyData();

    /* Note that for each update() call there should be corresponding
     * clear() call because the ProxyConf class implements a reference
     * counting mechanism. The factory is removed only when there is
     * no one using the factory any more.
     */
    static void update(void);          // this builds QNetworkProxy factory
    static void clear(void);           // this removes QNetworkProxy factory
};

} // namespace Maemo

#endif
