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

#include <qdesktopservices.h>

#ifndef QT_NO_DESKTOPSERVICES

#include <qprocess.h>
#include <qurl.h>
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qt_x11_p.h>
#include <qcoreapplication.h>
#include <stdlib.h>

QT_BEGIN_NAMESPACE

inline static bool launch(const QUrl &url, const QString &client)
{
   QString data = client + ' ' + QString::fromUtf8(url.toEncoded());

#if ! defined(QT_NO_PROCESS)
   return QProcess::startDetached(data);
#else
   return ::system(data.constData()) != -1;
#endif
}

static bool openDocument(const QUrl &url)
{
   if (! url.isValid()) {
      return false;
   }

   if (launch(url, "xdg-open")) {
      return true;
   }

   // Use the X11->desktopEnvironment value if X11 is non-NULL,
   //  otherwise just attempt to launch command regardless of the desktop environment
   if ((!X11 || (X11 && X11->desktopEnvironment == DE_GNOME)) && launch(url, QLatin1String("gnome-open"))) {
      return true;
   } else {
      if ((!X11 || (X11 && X11->desktopEnvironment == DE_KDE)) && launch(url, QLatin1String("kfmclient exec"))) {
         return true;
      }
   }

   if (launch(url, QLatin1String("firefox"))) {
      return true;
   }
   if (launch(url, QLatin1String("mozilla"))) {
      return true;
   }
   if (launch(url, QLatin1String("netscape"))) {
      return true;
   }
   if (launch(url, QLatin1String("opera"))) {
      return true;
   }

   return false;
}

static bool launchWebBrowser(const QUrl &url)
{
   if (!url.isValid()) {
      return false;
   }
   if (url.scheme() == "mailto") {
      return openDocument(url);
   }

   if (launch(url, "xdg-open")) {
      return true;
   }

   if (launch(url, QString::fromUtf8(getenv("DEFAULT_BROWSER")))) {
      return true;
   }

   if (launch(url, QString::fromUtf8(getenv("BROWSER")))) {
      return true;
   }

   // Use the X11->desktopEnvironment value if X11 is non-NULL,
   //  otherwise just attempt to launch command regardless of the desktop environment
   if ((!X11 || (X11 && X11->desktopEnvironment == DE_GNOME)) && launch(url, QLatin1String("gnome-open"))) {
      return true;

   } else {
      if ((!X11 || (X11 && X11->desktopEnvironment == DE_KDE)) && launch(url, QLatin1String("kfmclient openURL"))) {
         return true;
      }
   }

   if (launch(url, QLatin1String("firefox"))) {
      return true;
   }
   if (launch(url, QLatin1String("mozilla"))) {
      return true;
   }
   if (launch(url, QLatin1String("netscape"))) {
      return true;
   }
   if (launch(url, QLatin1String("opera"))) {
      return true;
   }
   return false;
}



QString QDesktopServices::storageLocation(StandardLocation type)
{
   if (type == QDesktopServices::HomeLocation) {
      return QDir::homePath();
   }
   if (type == QDesktopServices::TempLocation) {
      return QDir::tempPath();
   }

   // http://standards.freedesktop.org/basedir-spec/basedir-spec-0.6.html
   if (type == QDesktopServices::CacheLocation) {
      QString xdgCacheHome = QString::fromUtf8(qgetenv("XDG_CACHE_HOME"));

      if (xdgCacheHome.isEmpty()) {
         xdgCacheHome = QDir::homePath() + "/.cache";
      }

      xdgCacheHome += '/' + QCoreApplication::organizationName() + '/' + QCoreApplication::applicationName();
      return xdgCacheHome;
   }

   if (type == QDesktopServices::DataLocation) {
      QString xdgDataHome = QString::fromUtf8(qgetenv("XDG_DATA_HOME"));

      if (xdgDataHome.isEmpty()) {
         xdgDataHome = QDir::homePath() + QLatin1String("/.local/share");
      }

      xdgDataHome += "/data/" + QCoreApplication::organizationName() + '/' + QCoreApplication::applicationName();
      return xdgDataHome;
   }

   // http://www.freedesktop.org/wiki/Software/xdg-user-dirs
   QString xdgConfigHome = QString::fromUtf8(qgetenv("XDG_CONFIG_HOME"));

   if (xdgConfigHome.isEmpty()) {
      xdgConfigHome = QDir::homePath() + "/.config";
   }

   QFile file(xdgConfigHome + "/user-dirs.dirs");

   if (file.exists() && file.open(QIODevice::ReadOnly)) {
      QHash<QString, QString> lines;
      QTextStream stream(&file);

      // Only look for lines like: XDG_DESKTOP_DIR="$HOME/Desktop"
      QRegularExpression regExp("^XDG_(.*)_DIR=(.*)$");


      while (! stream.atEnd()) {
         QString line = stream.readLine();
         QRegularExpressionMatch match = regExp.match(line);

         if (match.hasMatch()) {
            QString key     = match.captured(1);
            QString value   = match.captured(2);

            if (value.length() > 2 && value.startsWith('\"') && value.endsWith('\"')) {
               value = value.mid(1, value.length() - 2);
            }

            // Store the key and value: "DESKTOP", "$HOME/Desktop"
            lines[key] = value;
         }
      }

      QString key;

      switch (type) {
         case DesktopLocation:
            key = QLatin1String("DESKTOP");
            break;
         case DocumentsLocation:
            key = QLatin1String("DOCUMENTS");
            break;
         case PicturesLocation:
            key = QLatin1String("PICTURES");
            break;
         case MusicLocation:
            key = QLatin1String("MUSIC");
            break;
         case MoviesLocation:
            key = QLatin1String("VIDEOS");
            break;
         default:
            break;
      }

      if (!key.isEmpty() && lines.contains(key)) {
         QString value = lines[key];
         // value can start with $HOME
         if (value.startsWith(QLatin1String("$HOME"))) {
            value = QDir::homePath() + value.mid(5);
         }
         return value;
      }
   }

   QDir emptyDir;
   QString path;
   switch (type) {
      case DesktopLocation:
         path = QDir::homePath() + QLatin1String("/Desktop");
         break;
      case DocumentsLocation:
         path = QDir::homePath() + QLatin1String("/Documents");
         break;
      case PicturesLocation:
         path = QDir::homePath() + QLatin1String("/Pictures");
         break;

      case FontsLocation:
         path = QDir::homePath() + QLatin1String("/.fonts");
         break;

      case MusicLocation:
         path = QDir::homePath() + QLatin1String("/Music");
         break;

      case MoviesLocation:
         path = QDir::homePath() + QLatin1String("/Videos");
         break;

      case ApplicationsLocation:
      default:
         break;
   }

   return path;
}

QString QDesktopServices::displayName(StandardLocation type)
{
   Q_UNUSED(type);
   return QString();
}

QT_END_NAMESPACE

#endif // QT_NO_DESKTOPSERVICES
