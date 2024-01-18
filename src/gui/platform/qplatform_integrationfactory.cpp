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

#include <qplatform_integrationfactory_p.h>
#include <qplatform_integrationplugin.h>
#include <qfactoryloader_p.h>
#include <qmutex.h>
#include <qdir.h>

#include <qguiapplication.h>
#include <qdebug.h>

static QFactoryLoader *loader()
{
   static QFactoryLoader retval(QPlatformIntegrationInterface_ID, "/platforms", Qt::CaseInsensitive);
   return &retval;
}

static QFactoryLoader *directLoader()
{
   static QFactoryLoader retval(QPlatformIntegrationInterface_ID, "", Qt::CaseInsensitive);
   return &retval;
}

static inline QPlatformIntegration *loadIntegration(QFactoryLoader *loader, const QString &key,
                  const QStringList &parameters, int &argc, char ** argv)
{
   if (loader->keySet().contains(key)) {

      if (QPlatformIntegrationPlugin *factory = dynamic_cast<QPlatformIntegrationPlugin *>(loader->instance(key))) {

         if (QPlatformIntegration *result = factory->create(key, parameters, argc, argv)) {
            return result;
         }
      }
   }

   return nullptr;
}

QPlatformIntegration *QPlatformIntegrationFactory::create(const QString &platform, const QStringList &paramList,
                  int &argc, char **argv, const QString &platformPluginPath)
{
    // try loading the plugin from the passed value of platformPluginPath
    if (! platformPluginPath.isEmpty()) {

        QCoreApplication::addLibraryPath(platformPluginPath);

        if (QPlatformIntegration *retval = loadIntegration(directLoader(), platform, paramList, argc, argv)) {
            return retval;
        }
    }

    // try loading using the default path or the path in cs.conf
    if (QPlatformIntegration *retval = loadIntegration(loader(), platform, paramList, argc, argv)) {
        return retval;
    }

    return nullptr;
}

QStringList QPlatformIntegrationFactory::keys(const QString &platformPluginPath)
{
    QStringList list;

    if (! platformPluginPath.isEmpty()) {
        QCoreApplication::addLibraryPath(platformPluginPath);

        auto keySet = directLoader()->keySet();
        list.append(keySet.toList());

        if (! list.isEmpty()) {
            const QString postFix = QString(" (from ") + QDir::toNativeSeparators(platformPluginPath) + ')';

            for (auto &tmp : list) {
               tmp.append(postFix);
            }
        }
    }

    auto keySet = loader()->keySet();
    list.append(keySet.toList());

    return list;
}

