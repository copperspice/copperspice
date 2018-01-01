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

#include "pvreglscreen.h"

#include <QScreenDriverPlugin>
#include <QStringList>

class PvrEglScreenPlugin : public QScreenDriverPlugin
{
public:
    PvrEglScreenPlugin();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

PvrEglScreenPlugin::PvrEglScreenPlugin()
    : QScreenDriverPlugin()
{
}

QStringList PvrEglScreenPlugin::keys() const
{
    return (QStringList() << "powervr");
}

QScreen* PvrEglScreenPlugin::create(const QString& driver, int displayId)
{
    if (driver.toLower() != "powervr")
        return 0;

    return new PvrEglScreen(displayId);
}

Q_EXPORT_PLUGIN2(qgfxpvregl, PvrEglScreenPlugin)
