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

#ifndef IAPCONF_H
#define IAPCONF_H

#include <QString>
#include <QVariant>

namespace Maemo {

class IAPConfPrivate;
class IAPConf {
public:
    IAPConf(const QString &iap_id);
    virtual ~IAPConf();

    /**
        Get one IAP value.
    */
    QVariant value(const QString& key) const;

    /**
        Return all the IAPs found in the system. If return_path is true,
	then do not strip the IAP path away.
    */
    static void getAll(QList<QString> &all_iaps, bool return_path=false);

private:
    IAPConfPrivate *d_ptr;
};

} // namespace Maemo

#endif
