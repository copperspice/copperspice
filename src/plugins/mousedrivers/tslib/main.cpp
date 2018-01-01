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

#include <qmousedriverplugin_qws.h>
#include <qmousetslib_qws.h>

QT_BEGIN_NAMESPACE

class TslibMouseDriver : public QMouseDriverPlugin
{
public:
    TslibMouseDriver();

    QStringList keys() const;
    QWSMouseHandler* create(const QString &driver, const QString &device);
};

TslibMouseDriver::TslibMouseDriver()
    : QMouseDriverPlugin()
{
}

QStringList TslibMouseDriver::keys() const
{
    return (QStringList() << "tslib");
}

QWSMouseHandler* TslibMouseDriver::create(const QString &driver,
                                          const QString &device)
{
    if (driver.toLower() != "tslib")
        return 0;
    return new QWSTslibMouseHandler(driver, device);
}

Q_EXPORT_STATIC_PLUGIN(TslibMouseDriver)
Q_EXPORT_PLUGIN2(qwstslibmousehandler, TslibMouseDriver)

QT_END_NAMESPACE
