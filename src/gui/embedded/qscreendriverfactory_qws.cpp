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

#include <qscreendriverfactory_qws.h>
#include <qscreen_qws.h>
#include <qapplication.h>
#include <qscreenlinuxfb_qws.h>
#include <qscreentransformed_qws.h>
#include <qscreenvfb_qws.h>
#include <qscreenmulti_qws_p.h>
#include <qscreenqnx_qws.h>
#include <qscreenintegrityfb_qws.h>
#include <stdlib.h>
#include <qfactoryloader_p.h>
#include <qscreendriverplugin_qws.h>

#ifndef QT_NO_QWS_DIRECTFB
#include <qdirectfbscreen.h>
#endif

#ifndef QT_NO_QWS_VNC
#include <qscreenvnc_qws.h>
#endif

QT_BEGIN_NAMESPACE

#if ! defined(Q_OS_WIN32) || ! defined(QT_STATIC)
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
                  (QScreenDriverFactoryInterface_iid, QLatin1String("/gfxdrivers"), Qt::CaseInsensitive))
#endif

QScreen *QScreenDriverFactory::create(const QString &key, int displayId)
{
   QString driver = key.toLower();

#ifndef QT_NO_QWS_QVFB
   if (driver == QLatin1String("qvfb") || driver.isEmpty()) {
      return new QVFbScreen(displayId);
   }
#endif
#ifndef QT_NO_QWS_LINUXFB
   if (driver == QLatin1String("linuxfb") || driver.isEmpty()) {
      return new QLinuxFbScreen(displayId);
   }
#endif
#ifndef QT_NO_QWS_DIRECTFB
   if (driver == QLatin1String("directfb") || driver.isEmpty()) {
      return new QDirectFBScreen(displayId);
   }
#endif
#ifndef QT_NO_QWS_TRANSFORMED
   if (driver == QLatin1String("transformed")) {
      return new QTransformedScreen(displayId);
   }
#endif
#ifndef QT_NO_QWS_VNC
   if (driver == QLatin1String("vnc")) {
      return new QVNCScreen(displayId);
   }
#endif
#ifndef QT_NO_QWS_MULTISCREEN
   if (driver == QLatin1String("multi")) {
      return new QMultiScreen(displayId);
   }
#endif

#if ! defined(Q_OS_WIN32) || ! defined(QT_STATIC)
   if (QScreenDriverFactoryInterface *factory = qobject_cast<QScreenDriverFactoryInterface *>(loader()->instance(key))) {
      return factory->create(driver, displayId);
   }
#endif

   return 0;
}

/*!
    Returns the list of valid keys, i.e. the available screen drivers.

    \sa create()
*/
QStringList QScreenDriverFactory::keys()
{
   QStringList list;

#ifndef QT_NO_QWS_QVFB
   list << QLatin1String("QVFb");
#endif
#ifndef QT_NO_QWS_LINUXFB
   list << QLatin1String("LinuxFb");
#endif
#ifndef QT_NO_QWS_TRANSFORMED
   list << QLatin1String("Transformed");
#endif
#ifndef QT_NO_QWS_VNC
   list << QLatin1String("VNC");
#endif
#ifndef QT_NO_QWS_MULTISCREEN
   list << QLatin1String("Multi");
#endif

#if ! defined(Q_OS_WIN32) || ! defined(QT_STATIC)
   QStringList plugins = loader()->keys();

   for (int i = 0; i < plugins.size(); ++i) {

# ifdef QT_NO_QWS_QVFB
      // give QVFb top priority for autodetection
      if (plugins.at(i) == QLatin1String("QVFb")) {
         list.prepend(plugins.at(i));
      } else
# endif
         if (!list.contains(plugins.at(i))) {
            list += plugins.at(i);
         }
   }
#endif

   return list;
}

QT_END_NAMESPACE
