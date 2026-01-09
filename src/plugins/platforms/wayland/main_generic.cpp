/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <qplatform_integrationplugin.h>

#include <qwayland_integration_p.h>

namespace QtWaylandClient {

class QWaylandIntegrationPlugin : public QPlatformIntegrationPlugin
{
   CS_OBJECT(QWaylandIntegrationPlugin)

   CS_PLUGIN_IID(QPlatformIntegrationInterface_ID)
   CS_PLUGIN_KEY("wayland")

 public:
   QPlatformIntegration *create(const QString &system, const QStringList &, int &, char **) override;
};

CS_PLUGIN_REGISTER(QWaylandIntegrationPlugin)

QPlatformIntegration *QWaylandIntegrationPlugin::create(const QString &system, const QStringList &, int &, char **)
{
   if (! system.compare("wayland", Qt::CaseInsensitive)) {
      return new QWaylandIntegration();
   }

   return nullptr;
}

}
