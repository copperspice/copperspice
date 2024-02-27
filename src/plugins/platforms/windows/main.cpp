/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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
#include <qstringlist.h>
#include <qwin_gdi_integration.h>

class QWindowsIntegrationPlugin : public QPlatformIntegrationPlugin
{
   CS_OBJECT(QWindowsIntegrationPlugin)

   CS_PLUGIN_IID(QPlatformIntegrationInterface_ID)
   CS_PLUGIN_KEY("windows")

 public:
   QPlatformIntegration *create(const QString &, const QStringList &, int &, char **) override;
};

CS_PLUGIN_REGISTER(QWindowsIntegrationPlugin)

QPlatformIntegration *QWindowsIntegrationPlugin::create(const QString &system, const QStringList &paramList, int &, char **)
{
   if (system.compare(system, "windows", Qt::CaseInsensitive) == 0) {
      return new QWindowsGdiIntegration(paramList);
   }

   return nullptr;
}

