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

#include <qgraphicssystemfactory_p.h>
#include <qgraphicssystemplugin_p.h>
#include <qfactoryloader_p.h>
#include <qmutex.h>
#include <qapplication.h>
#include <qgraphicssystem_raster_p.h>
#include <qgraphicssystem_runtime_p.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
                          (QGraphicsSystemFactoryInterface_iid, QLatin1String("/graphicssystems"), Qt::CaseInsensitive))

QGraphicsSystem *QGraphicsSystemFactory::create(const QString &key)
{
   QGraphicsSystem *ret = 0;
   QString system = key.toLower();

#if defined (QT_GRAPHICSSYSTEM_OPENGL)
   if (system.isEmpty()) {
      system = QLatin1String("opengl");
   }
#elif defined (QT_GRAPHICSSYSTEM_OPENVG)
   if (system.isEmpty()) {
      system = QLatin1String("openvg");
   }
#elif defined (QT_GRAPHICSSYSTEM_RUNTIME)
   if (system.isEmpty()) {
      system = QLatin1String("runtime");
   }
#elif defined (QT_GRAPHICSSYSTEM_RASTER) && !defined(Q_OS_WIN) || defined(Q_WS_X11)
   if (system.isEmpty()) {
      system = QLatin1String("raster");
   }
#endif

   if (system == QLatin1String("raster")) {
      return new QRasterGraphicsSystem;
   } else if (system == QLatin1String("runtime")) {
      return new QRuntimeGraphicsSystem;
   } else if (system.isEmpty() || system == QLatin1String("native")) {
      return 0;
   }

   if (!ret) {
      if (QGraphicsSystemFactoryInterface *factory = qobject_cast<QGraphicsSystemFactoryInterface *>(loader()->instance(
               system))) {
         ret = factory->create(system);
      }
   }

   if (!ret) {
      qWarning() << "Unable to load graphicssystem" << system;
   }

   return ret;
}

/*!
    Returns the list of valid keys, i.e. the keys this factory can
    create styles for.

    \sa create()
*/
QStringList QGraphicsSystemFactory::keys()
{
   QStringList list = loader()->keys();

   if (!list.contains(QLatin1String("Raster"))) {
      list << QLatin1String("raster");
   }
   return list;
}

QT_END_NAMESPACE

