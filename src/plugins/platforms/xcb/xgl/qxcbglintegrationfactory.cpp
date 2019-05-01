/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qxcbglintegrationfactory.h"
#include "qxcbglintegrationplugin.h"

#include "qxcbglintegrationplugin.h"
#include "qfactoryloader_p.h"
#include "qapplication.h"
#include "qdir.h"

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QXcbGlIntegrationInterface_ID, "/xcbglintegrations", Qt::CaseInsensitive))
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, directLoader, (QXcbGlIntegrationInterface_ID, "", Qt::CaseInsensitive))

static inline QXcbGlIntegration *loadIntegration(QFactoryLoader *loader, const QString &key)
{
   QXcbGlIntegrationPlugin *factory = qobject_cast<QXcbGlIntegrationPlugin *>(loader->instance(key));

   if (factory != nullptr) {
      if (QXcbGlIntegration *result = factory->create()) {
         return result;
      }
   }

   return nullptr;
}

QStringList QXcbGlIntegrationFactory::keys(const QString &pluginPath)
{
   QStringList list;

   if (! pluginPath.isEmpty()) {
      QCoreApplication::addLibraryPath(pluginPath);
      list = directLoader()->keySet().toList();

      if (! list.isEmpty()) {
         const QString postFix = QString(" (from ") + QDir::toNativeSeparators(pluginPath) + ')';

         for (auto &item : list) {
            item.append(postFix);
         }
      }
   }

   list.append(loader()->keySet().toList());

   return list;
}

QXcbGlIntegration *QXcbGlIntegrationFactory::create(const QString &platform, const QString &pluginPath)
{
   // Try loading the plugin from platformPluginPath first:
   if (!pluginPath.isEmpty()) {
      QCoreApplication::addLibraryPath(pluginPath);

      if (QXcbGlIntegration *ret = loadIntegration(directLoader(), platform)) {
         return ret;
      }
   }

   if (QXcbGlIntegration *ret = loadIntegration(loader(), platform)) {
      return ret;
   }

   return nullptr;
}

