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

#include <qcoreapplication.h>
#include <qdir.h>

QT_BEGIN_NAMESPACE

static bool launchWebBrowser(const QUrl &url)
{
   Q_UNUSED(url);
   qWarning("QDesktopServices::launchWebBrowser not implemented");
   return false;
}

static bool openDocument(const QUrl &file)
{
   Q_UNUSED(file);
   qWarning("QDesktopServices::openDocument not implemented");
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

   if (type == DataLocation) {
      QString qwsDataHome = QLatin1String(qgetenv("QWS_DATA_HOME"));
      if (qwsDataHome.isEmpty()) {
         qwsDataHome = QDir::homePath() + QLatin1String("/.qws/share");
      }
      qwsDataHome += QLatin1String("/data/")
                     + QCoreApplication::organizationName() + QLatin1Char('/')
                     + QCoreApplication::applicationName();
      return qwsDataHome;
   }
   if (type == QDesktopServices::CacheLocation) {
      QString qwsCacheHome = QLatin1String(qgetenv("QWS_CACHE_HOME"));
      if (qwsCacheHome.isEmpty()) {
         qwsCacheHome = QDir::homePath() + QLatin1String("/.qws/cache/");
      }
      qwsCacheHome += QCoreApplication::organizationName() +  QLatin1Char('/')
                      + QCoreApplication::applicationName();
      return qwsCacheHome;
   }

   qWarning("QDesktopServices::storageLocation %d not implemented", type);
   return QString();
}

QString QDesktopServices::displayName(StandardLocation type)
{
   Q_UNUSED(type);
   qWarning("QDesktopServices::displayName not implemented");
   return QString();
}

QT_END_NAMESPACE
