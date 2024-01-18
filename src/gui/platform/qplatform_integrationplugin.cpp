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

QPlatformIntegrationPlugin::QPlatformIntegrationPlugin(QObject *parent)
    : QObject(parent)
{
}

QPlatformIntegrationPlugin::~QPlatformIntegrationPlugin()
{
}

QPlatformIntegration *QPlatformIntegrationPlugin::create(const QString &key, const QStringList &paramList)
{
   (void) key;
   (void) paramList;

   return nullptr;
}

QPlatformIntegration *QPlatformIntegrationPlugin::create(const QString &key, const QStringList &paramList, int &argc, char **argv)
{
   (void) argc;
   (void) argv;

   return create(key, paramList); // Fallback for platform plugins that do not implement the argc/argv version.
}

