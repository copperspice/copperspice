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

#include <qguiplatformplugin_p.h>
#include <qdebug.h>
#include <qfile.h>
#include <qdir.h>
#include <qsettings.h>
#include <qfactoryloader_p.h>
#include <qstylefactory.h>
#include <qapplication.h>
#include <qplatformdefs.h>
#include <qicon.h>

#if defined(Q_WS_X11)
#include <qkde_p.h>
#include <qgtkstyle_p.h>
#include <qt_x11_p.h>
#endif

#include <stdlib.h>

QT_BEGIN_NAMESPACE


/*! \internal
    Return (an construct if necesseray) the Gui Platform plugin.

    The plugin key to be loaded is inside the QT_PLATFORM_PLUGIN environment variable.
    If it is not set, it will be the DESKTOP_SESSION on X11.

    If no plugin can be loaded, the default one is returned.
 */
QGuiPlatformPlugin *qt_guiPlatformPlugin()
{
   static QGuiPlatformPlugin *plugin;
   if (!plugin) {


      QString key = QString::fromLocal8Bit(qgetenv("QT_PLATFORM_PLUGIN"));
#ifdef Q_WS_X11
      if (key.isEmpty()) {
         switch (X11->desktopEnvironment) {
            case DE_KDE:
               key = QString::fromLatin1("kde");
               break;
            default:
               key = QString::fromLocal8Bit(qgetenv("DESKTOP_SESSION"));
               break;
         }
      }
#endif

      if (!key.isEmpty() && QApplication::desktopSettingsAware()) {
         QFactoryLoader loader(QGuiPlatformPluginInterface_iid, QLatin1String("/gui_platform"));
         plugin = qobject_cast<QGuiPlatformPlugin *>(loader.instance(key));
      }

      if (!plugin) {
         static QGuiPlatformPlugin def;
         plugin = &def;
      }
   }
   return plugin;
}


/* \class QPlatformPlugin
    QGuiPlatformPlugin can be used to integrate Qt applications in a platform built on top of Qt.
    The application developer should not know or use the plugin, it is only used by Qt internaly.

    But full platform that are built on top of Qt may provide a plugin so 3rd party Qt application
    running in the platform are integrated.
 */

/*
    The constructor can be used to install hooks in Qt
 */
QGuiPlatformPlugin::QGuiPlatformPlugin(QObject *parent) : QObject(parent) {}
QGuiPlatformPlugin::~QGuiPlatformPlugin() {}


/* return the string key to be used by default the application */
QString QGuiPlatformPlugin::styleName()
{

#if defined(Q_OS_WIN)
   if ((QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA
         && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based))) {
      return QLatin1String("WindowsVista");
   }

   else if ((QSysInfo::WindowsVersion >= QSysInfo::WV_XP
             && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based))) {
      return QLatin1String("WindowsXP");
   }

   else {
      return QLatin1String("Windows");   // default styles for Windows
   }

#elif defined(Q_WS_X11) && defined(Q_OS_SOLARIS)
   return QLatin1String("CDE");                        // default style for X11 on Solaris

#elif defined(Q_WS_QWS) || defined(Q_WS_QPA)
   return QLatin1String("Plastique");                  // default style for X11 and small devices

#elif defined(Q_OS_MAC)
   return QLatin1String("Macintosh");                  // default style for all Mac's

#elif defined(Q_WS_X11)
   QString stylename;
   switch (X11->desktopEnvironment) {
      case DE_KDE:
         stylename = QKde::kdeStyle();
         break;

      case DE_GNOME: {
         QStringList availableStyles = QStyleFactory::keys();
         // Set QGtkStyle for GNOME if available
         QString gtkStyleKey = QString::fromLatin1("GTK+");
         if (availableStyles.contains(gtkStyleKey)) {
            stylename = gtkStyleKey;
            break;
         }
         if (X11->use_xrender) {
            stylename = QLatin1String("cleanlooks");
         } else {
            stylename = QLatin1String("windows");
         }
         break;
      }
      case DE_CDE:
         stylename = QLatin1String("cde");
         break;

      default:
         // Don't do anything
         break;
   }
   return stylename;
#endif
}

/* return an additional default palette  (only work on X11) */
QPalette QGuiPlatformPlugin::palette()
{
#ifdef Q_WS_X11
   if (QApplication::desktopSettingsAware() && X11->desktopEnvironment == DE_KDE) {
      return QKde::kdePalette();
   }
#endif

   return QPalette();
}

/* the default icon theme name for QIcon::fromTheme. */
QString QGuiPlatformPlugin::systemIconThemeName()
{
   QString result;
#ifdef Q_WS_X11
   if (X11->desktopEnvironment == DE_GNOME) {
#ifndef QT_NO_STYLE_GTK
      result = QGtkStylePrivate::getIconThemeName();
#endif
      if (result.isEmpty()) {
         result = QString::fromLatin1("gnome");
      }
   } else if (X11->desktopEnvironment == DE_KDE) {
      result =  X11->desktopVersion >= 4 ? QString::fromLatin1("oxygen") : QString::fromLatin1("crystalsvg");
      QSettings settings(QKde::kdeHome() + QLatin1String("/share/config/kdeglobals"), QSettings::IniFormat);
      settings.beginGroup(QLatin1String("Icons"));
      result = settings.value(QLatin1String("Theme"), result).toString();
   }
#endif
   return result;
}


QStringList QGuiPlatformPlugin::iconThemeSearchPaths()
{
   QStringList paths;
#if defined(Q_WS_X11)
   QString xdgDirString = QFile::decodeName(getenv("XDG_DATA_DIRS"));
   if (xdgDirString.isEmpty()) {
      xdgDirString = QLatin1String("/usr/local/share/:/usr/share/");
   }

   QStringList xdgDirs = xdgDirString.split(QLatin1Char(':'));

   for (int i = 0 ; i < xdgDirs.size() ; ++i) {
      QDir dir(xdgDirs[i]);
      if (dir.exists()) {
         paths.append(dir.path() + QLatin1String("/icons"));
      }
   }
   if (X11->desktopEnvironment == DE_KDE) {
      paths << QLatin1Char(':') + QKde::kdeHome() + QLatin1String("/share/icons");
      QStringList kdeDirs = QFile::decodeName(getenv("KDEDIRS")).split(QLatin1Char(':'));
      for (int i = 0 ; i < kdeDirs.count() ; ++i) {
         QDir dir(QLatin1Char(':') + kdeDirs.at(i) + QLatin1String("/share/icons"));
         if (dir.exists()) {
            paths.append(dir.path());
         }
      }
   }

   // Add home directory first in search path
   QDir homeDir(QDir::homePath() + QLatin1String("/.icons"));
   if (homeDir.exists()) {
      paths.prepend(homeDir.path());
   }
#endif

#if defined(Q_OS_WIN)
   paths.append(qApp->applicationDirPath() + QLatin1String("/icons"));
#elif defined(Q_OS_MAC)
   paths.append(qApp->applicationDirPath() + QLatin1String("/../Resources/icons"));
#endif
   return paths;
}

/* backend for QFileIconProvider,  null icon means default */
QIcon QGuiPlatformPlugin::fileSystemIcon(const QFileInfo &)
{
   return QIcon();
}

/* Like QStyle::styleHint */
int QGuiPlatformPlugin::platformHint(PlatformHint hint)
{
   int ret = 0;
   switch (hint) {
      case PH_ToolButtonStyle:
         ret = Qt::ToolButtonIconOnly;
#ifdef Q_WS_X11
         if (X11->desktopEnvironment == DE_KDE && X11->desktopVersion >= 4
               && QApplication::desktopSettingsAware()) {
            ret = QKde::kdeToolButtonStyle();
         }
#endif
         break;
      case PH_ToolBarIconSize:
#ifdef Q_WS_X11
         if (X11->desktopEnvironment == DE_KDE && X11->desktopVersion >= 4
               && QApplication::desktopSettingsAware()) {
            ret = QKde::kdeToolBarIconSize();
         }
#endif
         //by default keep ret = 0 so QCommonStyle will use the style default
         break;
      default:
         break;
   }
   return ret;
}


QT_END_NAMESPACE
