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

#include <QtGui/QPlatformIntegrationPlugin>
#include "qwaylandintegration.h"

QT_BEGIN_NAMESPACE

class QWaylandIntegrationPlugin : public QPlatformIntegrationPlugin
{
public:
    QStringList keys() const;
    QPlatformIntegration *create(const QString&, const QStringList&);
};

QStringList QWaylandIntegrationPlugin::keys() const
{
    QStringList list;
    list << "Wayland";
    list << "WaylandGL";
    return list;
}

QPlatformIntegration *QWaylandIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    if (system.toLower() == "wayland")
        return new QWaylandIntegration;
    if (system.toLower() == "waylandgl")
        return new QWaylandIntegration(true);

    return 0;
}

Q_EXPORT_PLUGIN2(wayland, QWaylandIntegrationPlugin)

QT_END_NAMESPACE
