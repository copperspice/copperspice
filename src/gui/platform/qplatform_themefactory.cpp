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

#include <qplatform_themefactory_p.h>

#include <qplatform_themeplugin.h>
#include <qdir.h>
#include <qmutex.h>
#include <qguiapplication.h>
#include <qdebug.h>

#include <qfactoryloader_p.h>

static QFactoryLoader *loader()
{
   static QFactoryLoader retval(QPlatformThemeInterface_ID, "/platformthemes", Qt::CaseInsensitive);
   return &retval;
}

static QFactoryLoader *directLoader()
{
   static QFactoryLoader retval(QPlatformThemeInterface_ID, "", Qt::CaseInsensitive);
   return &retval;
}

QPlatformTheme *QPlatformThemeFactory::create(const QString &key, const QString &platformPluginPath)
{
   QStringList paramList = key.split(':');
   const QString platform = paramList.takeFirst().toLower();

   // Try loading the plugin from platformPluginPath first:
   if (! platformPluginPath.isEmpty()) {
      QCoreApplication::addLibraryPath(platformPluginPath);

      if (QPlatformTheme *ret = cs_load_plugin<QPlatformTheme, QPlatformThemePlugin>(directLoader(), platform, paramList)) {
         return ret;
      }
   }

   if (QPlatformTheme *ret = cs_load_plugin<QPlatformTheme, QPlatformThemePlugin>(loader(), platform, paramList)) {
      return ret;
   }

   return nullptr;
}

/*!
    Returns the list of valid keys, i.e. the keys this factory can
    create styles for.

    \sa create()
*/
QStringList QPlatformThemeFactory::keys(const QString &platformPluginPath)
{
   QStringList list;

   if (! platformPluginPath.isEmpty()) {
      QCoreApplication::addLibraryPath(platformPluginPath);

      auto keySet = directLoader()->keySet();
      list.append(keySet.toList());

      if (! list.isEmpty()) {
         const QString postFix = " (from " + QDir::toNativeSeparators(platformPluginPath) + ')';

         for (auto &tmp : list) {
            tmp.append(postFix);
         }
      }
   }

   auto keySet = loader()->keySet();
   list.append(keySet.toList());

   return list;

}

