/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#include "eglnullwsscreenplugin.h"
#include "eglnullwsscreen.h"

#include <QScreenDriverPlugin>
#include <QStringList>

class EGLNullWSScreenPlugin : public QScreenDriverPlugin
{
public:
    virtual QStringList keys() const;
    virtual QScreen *create(const QString& driver, int displayId);
};

QStringList EGLNullWSScreenPlugin::keys() const
{
    return QStringList() << QLatin1String(PluginName);
}

QScreen *EGLNullWSScreenPlugin::create(const QString& driver, int displayId)
{
    return (driver.toLower() == QLatin1String(PluginName) ?
        new EGLNullWSScreen(displayId) : 0);
}

Q_EXPORT_PLUGIN2(qeglnullws, EGLNullWSScreenPlugin)
