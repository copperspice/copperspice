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

#include <qxcb_gl_integrationfactory.h>

#include <qxcb_gl_integrationplugin.h>
#include <qapplication.h>
#include <qdir.h>

#include <qfactoryloader_p.h>

static QFactoryLoader *loader()
{
   static QFactoryLoader retval(QXcbGlIntegrationInterface_ID, "/xcbglintegrations", Qt::CaseInsensitive);
   return &retval;
}

static QFactoryLoader *directLoader()
{
   static QFactoryLoader retval(QXcbGlIntegrationInterface_ID, "", Qt::CaseInsensitive);
   return &retval;
}

static inline QXcbGlIntegration *loadIntegration(QFactoryLoader *loader, const QString &key)
{
   if (loader->keySet().contains(key)) {
      QXcbGlIntegrationPlugin *factory = dynamic_cast<QXcbGlIntegrationPlugin *>(loader->instance(key));

      if (factory != nullptr) {
         if (QXcbGlIntegration *result = factory->create()) {
            return result;
         }
      }
   }

   return nullptr;
}

QXcbGlIntegration *QXcbGlIntegrationFactory::create(const QString &platform, const QString &pluginPath)
{
   // try loading the xcb-glx plugin from the path first
   if (! pluginPath.isEmpty()) {
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


