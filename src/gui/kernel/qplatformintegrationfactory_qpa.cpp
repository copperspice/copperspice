/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qplatformintegrationfactory_qpa_p.h>
#include <QPlatformIntegrationPlugin>
#include <qfactoryloader_p.h>
#include <qmutex.h>
#include <qapplication.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_SETTINGS)
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
                          (QPlatformIntegrationFactoryInterface_iid, QLatin1String("/platforms"), Qt::CaseInsensitive))
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, directLoader,
                          (QPlatformIntegrationFactoryInterface_iid, QLatin1String(""), Qt::CaseInsensitive))
#endif

QPlatformIntegration *QPlatformIntegrationFactory::create(const QString &key, const QString &platformPluginPath)
{
   QPlatformIntegration *ret = 0;
   QStringList paramList = key.split(QLatin1Char(':'));
   QString platform = paramList.takeFirst().toLower();

#if !defined(QT_NO_SETTINGS)
   // Try loading the plugin from platformPluginPath first:
   if (!platformPluginPath.isEmpty()) {
      QCoreApplication::addLibraryPath(platformPluginPath);
      if (QPlatformIntegrationFactoryInterface *factory =
               qobject_cast<QPlatformIntegrationFactoryInterface *>(directLoader()->instance(platform))) {
         ret = factory->create(key, paramList);
      }

      if (ret) {
         return ret;
      }
   }
   if (QPlatformIntegrationFactoryInterface *factory = qobject_cast<QPlatformIntegrationFactoryInterface *>
         (loader()->instance(platform))) {
      ret = factory->create(platform, paramList);
   }
#endif

   return ret;
}

/*!
    Returns the list of valid keys, i.e. the keys this factory can
    create styles for.

    \sa create()
*/
QStringList QPlatformIntegrationFactory::keys(const QString &platformPluginPath)
{
#if !defined(QT_NO_SETTINGS)
   QStringList list;

   if (!platformPluginPath.isEmpty()) {
      QCoreApplication::addLibraryPath(platformPluginPath);
      for (const QString & key : directLoader()->keys()) {
         list += key + QString(QLatin1String(" (from %1)")).arg(platformPluginPath);
      }
   }

   list += loader()->keys();
#else
   QStringList list;
#endif
   return list;
}

QT_END_NAMESPACE

