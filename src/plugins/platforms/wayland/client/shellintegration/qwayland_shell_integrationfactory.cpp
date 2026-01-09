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

// Copyright (C) 2014 Jolla Ltd

#include <qwayland_shell_integrationfactory_p.h>

#include <qcoreapplication.h>
#include <qdir.h>

#include <qfactoryloader_p.h>
#include <qwayland_shell_integration_p.h>
#include <qwayland_shell_integrationplugin_p.h>

namespace QtWaylandClient {

static QFactoryLoader *loader()
{
   static QFactoryLoader retval(QWaylandShellIntegrationFactoryInterface_ID, "/platforms", Qt::CaseInsensitive);
   return &retval;
}

static QFactoryLoader *directLoader()
{
   static QFactoryLoader retval(QWaylandShellIntegrationFactoryInterface_ID, QString(), Qt::CaseInsensitive);
   return &retval;
}

QStringList QWaylandShellIntegrationFactory::keys(const QString &pluginPath)
{
   QStringList list;

   if (! pluginPath.isEmpty()) {
      QCoreApplication::addLibraryPath(pluginPath);
      list = directLoader()->keySet().toList();

      if (! list.isEmpty()) {
         const QString postFix = " (from " + QDir::toNativeSeparators(pluginPath) + ')';

         for (auto &item : list) {
            item.append(postFix);
         }
      }
   }

   list.append(loader()->keySet().toList());

   return list;
}

QWaylandShellIntegration *QWaylandShellIntegrationFactory::create(const QString &platform, const QStringList &args, const QString &pluginPath)
{
   // Try loading the plugin from platformPluginPath first

   if (! pluginPath.isEmpty()) {
      QCoreApplication::addLibraryPath(pluginPath);

      QWaylandShellIntegration *retval = cs_load_plugin < QWaylandShellIntegration,
            QWaylandShellIntegrationPlugin > (directLoader(), platform, args);

      if (retval != nullptr) {
         return retval;
      }
   }

   QWaylandShellIntegration *retval = cs_load_plugin < QWaylandShellIntegration,
         QWaylandShellIntegrationPlugin > (loader(), platform, args);

   if (retval != nullptr) {
      return retval;
   }

   return nullptr;
}

}
