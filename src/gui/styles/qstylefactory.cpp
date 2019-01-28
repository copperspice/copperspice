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

#include <qstylefactory.h>
#include <qstyleplugin.h>
#include <qfactoryloader_p.h>
#include <qmutex.h>
#include <qapplication.h>
#include <qwindows_style_p.h>

#ifndef QT_NO_STYLE_FUSION
#include <qfusionstyle_p.h>
#endif

#ifndef QT_NO_STYLE_GTK
#include <qgtkstyle_p.h>
#endif

#ifndef QT_NO_STYLE_WINDOWSXP
#include <qwindows_xpstyle_p.h>
#endif

#ifndef QT_NO_STYLE_WINDOWSVISTA
#include <qwindows_vistastyle_p.h>
#endif

#if !defined(QT_NO_STYLE_MAC) && defined(Q_OS_MAC)
#include <qmacstyle_mac.h>
#endif

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QStyleFactoryInterface_iid, QLatin1String("/styles"), Qt::CaseInsensitive))

QStyle *QStyleFactory::create(const QString &key)
{
   QStyle *ret = 0;
   QString style = key.toLower();

#ifndef QT_NO_STYLE_WINDOWS
   if (style == QLatin1String("windows")) {
      ret = new QWindowsStyle;
   } else
#endif

#ifndef QT_NO_STYLE_WINDOWSXP
      if (style == QLatin1String("windowsxp")) {
         ret = new QWindowsXPStyle;
      } else
#endif

#ifndef QT_NO_STYLE_WINDOWSVISTA
         if (style == QLatin1String("windowsvista")) {
            ret = new QWindowsVistaStyle;
         } else
#endif

#ifndef QT_NO_STYLE_FUSION
            if (style == QLatin1String("fusion")) {
               ret = new QFusionStyle;
            } else
#endif

#ifndef QT_NO_STYLE_GTK
               if (style == QLatin1String("gtk") || style == QLatin1String("gtk+")) {
                  ret = new QGtkStyle;
               } else
#endif

#ifndef QT_NO_STYLE_MAC
                  if (style.startsWith(QLatin1String("macintosh"))) {
                     ret = new QMacStyle;
                  } else
#endif

                  { } // Keep these here

   if (! ret) {
      ret = cs_load_plugin<QStyle, QStylePlugin>(loader(), style);
   }

   if (ret) {
      ret->setObjectName(style);
   }

   return ret;
}

QStringList QStyleFactory::keys()
{
   QStringList list;

   auto keySet = loader()->keySet();
   list.append(keySet.toList());

#ifndef QT_NO_STYLE_WINDOWS
   if (! list.contains("Windows")) {
      list << "Windows";
   }
#endif

#ifndef QT_NO_STYLE_WINDOWSXP
   if (! list.contains("WindowsXP") &&
            (QSysInfo::WindowsVersion >= QSysInfo::WV_XP && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based))) {
      list << "WindowsXP";
   }
#endif

#ifndef QT_NO_STYLE_WINDOWSVISTA
   if (! list.contains("WindowsVista") &&
            (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based))) {
      list << "WindowsVista";
   }
#endif

#ifndef QT_NO_STYLE_GTK
   if (! list.contains("GTK+")) {
      list << "GTK+";
   }
#endif

#ifndef QT_NO_STYLE_FUSION
   if (! list.contains("Fusion")) {
      list << "Fusion";
   }
#endif

#ifndef QT_NO_STYLE_MAC
   QString mstyle = "Macintosh";

   if (!list.contains(mstyle)) {
      list << mstyle;
   }
#endif

   return list;
}

