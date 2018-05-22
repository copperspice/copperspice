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

#include <qkde_p.h>
#include <QtCore/QLibrary>
#include <QtCore/QDir>
#include <QtCore/qdebug.h>
#include <QtCore/QSettings>
#include <QtGui/qstylefactory.h>
#include <qt_x11_p.h>

#if defined(Q_WS_X11)

QT_BEGIN_NAMESPACE

/*! \internal
Gets the current KDE home path like "/home/cs/.kde"
*/
QString QKde::kdeHome()
{
   static QString kdeHomePath;

   if (kdeHomePath.isEmpty()) {
      kdeHomePath = QString::fromUtf8(qgetenv("KDEHOME"));

      if (kdeHomePath.isEmpty()) {
         QDir homeDir(QDir::homePath());
         QString kdeConfDir("/.kde");

         if (4 == X11->desktopVersion && homeDir.exists(".kde4")) {
            kdeConfDir = "/.kde4";
         }

         kdeHomePath = QDir::homePath() + kdeConfDir;
      }
   }

   return kdeHomePath;
}

/*!\internal
  Reads the color from the config, and store it in the palette with the given color role if found
  */
static bool kdeColor(QPalette *pal, QPalette::ColorRole role, const QSettings &kdeSettings, const QString &kde4Key,
                     const QString &kde3Key = QString())
{
   QVariant variant = kdeSettings.value(kde4Key);

   if (!variant.isValid()) {
      QVariant variant = kdeSettings.value(kde3Key);
   }

   if (variant.isValid()) {
      QStringList values = variant.toStringList();

      if (values.size() == 3) {
         int r = values[0].toInteger<int>();
         int g = values[1].toInteger<int>();
         int b = values[2].toInteger<int>();
         pal->setBrush(role, QColor(r, g, b));

         return true;
      }
   }
   return false;
}


/*!\internal
    Returns the KDE palette
*/
QPalette QKde::kdePalette()
{
   const QSettings theKdeSettings(QKde::kdeHome() + "/share/config/kdeglobals", QSettings::IniFormat);
   QPalette pal;

   // Setup KDE palette
   kdeColor(&pal, QPalette::Button,          theKdeSettings, "Colors:Button/BackgroundNormal", "buttonBackground");
   kdeColor(&pal, QPalette::Window,          theKdeSettings, "Colors:Window/BackgroundNormal", "background");
   kdeColor(&pal, QPalette::Text,            theKdeSettings, "Colors:View/ForegroundNormal", "foreground");
   kdeColor(&pal, QPalette::WindowText,      theKdeSettings, "Colors:Window/ForegroundNormal", "windowForeground");
   kdeColor(&pal, QPalette::Base,            theKdeSettings, "Colors:View/BackgroundNormal", "windowBackground");
   kdeColor(&pal, QPalette::Highlight,       theKdeSettings, "Colors:Selection/BackgroundNormal", "selectBackground");
   kdeColor(&pal, QPalette::HighlightedText, theKdeSettings, "Colors:Selection/ForegroundNormal", "selectForeground");

   kdeColor(&pal, QPalette::AlternateBase,   theKdeSettings, "Colors:View/BackgroundAlternate", "alternateBackground");
   kdeColor(&pal, QPalette::ButtonText,      theKdeSettings, "Colors:Button/ForegroundNormal", "buttonForeground");
   kdeColor(&pal, QPalette::Link,            theKdeSettings, "Colors:View/ForegroundLink", "linkColor");
   kdeColor(&pal, QPalette::LinkVisited,     theKdeSettings, "Colors:View/ForegroundVisited", "visitedLinkColor");
   //## TODO tooltip color

   return pal;
}

/*!\internal
    Returns the name of the QStyle to use.
    (read from the kde config if needed)
*/
QString QKde::kdeStyle()
{
   if (X11->desktopVersion >= 4) {
      QSettings kdeSettings(QKde::kdeHome() + "/share/config/kdeglobals", QSettings::IniFormat);

      QVariant tmp  = kdeSettings.value("widgetStyle", QString("Oxygen"));
      QString style = tmp.toString();

      QStringList availableStyles = QStyleFactory::keys();
      if (availableStyles.contains(style, Qt::CaseInsensitive)) {
         return style;
      }
   }

   if (X11->use_xrender) {
      return QString("plastique");
   } else {
      return QString("windows");
   }
}


int QKde::kdeToolButtonStyle()
{
   QSettings settings(QKde::kdeHome() + QLatin1String("/share/config/kdeglobals"), QSettings::IniFormat);
   settings.beginGroup(QLatin1String("Toolbar style"));

   QString toolbarStyle = settings.value(QLatin1String("ToolButtonStyle"), QLatin1String("TextBesideIcon")).toString();

   if (toolbarStyle == QLatin1String("TextBesideIcon")) {
      return Qt::ToolButtonTextBesideIcon;

   } else if (toolbarStyle == QLatin1String("TextOnly")) {
      return Qt::ToolButtonTextOnly;

   } else if (toolbarStyle == QLatin1String("TextUnderIcon")) {
      return Qt::ToolButtonTextUnderIcon;

   }

   return Qt::ToolButtonTextBesideIcon;
}

int QKde::kdeToolBarIconSize()
{
   static int iconSize = -1;
   if (iconSize == -1) {
      QSettings settings(QKde::kdeHome() + QLatin1String("/share/config/kdeglobals"),
                         QSettings::IniFormat);
      settings.beginGroup(QLatin1String("ToolbarIcons"));
      iconSize = settings.value(QLatin1String("Size")).toInt();
   }
   return iconSize;
}

QT_END_NAMESPACE

#endif //Q_WS_X11

