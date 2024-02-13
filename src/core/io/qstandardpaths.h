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

#ifndef QSTANDARDPATHS_H
#define QSTANDARDPATHS_H

#include <qstringlist.h>

#ifndef QT_NO_STANDARDPATHS

class Q_CORE_EXPORT QStandardPaths
{
 public:
   // Do not re-order, this must match QDesktopServices
   enum StandardLocation {
      DesktopLocation,
      DocumentsLocation,
      FontsLocation,
      ApplicationsLocation,
      MusicLocation,
      MoviesLocation,
      PicturesLocation,
      TempLocation,
      HomeLocation,
      DataLocation,
      CacheLocation,
      GenericDataLocation,
      RuntimeLocation,
      ConfigLocation,
      DownloadLocation,
      GenericCacheLocation,
      GenericConfigLocation,
      AppDataLocation,
      AppConfigLocation,
      AppLocalDataLocation = DataLocation
   };

   static QString writableLocation(StandardLocation type);
   static QStringList standardLocations(StandardLocation type);

   enum LocateOption {
      LocateFile = 0x0,
      LocateDirectory = 0x1
   };
   using LocateOptions = QFlags<LocateOption>;

   static QString locate(StandardLocation type, const QString &fileName, LocateOptions options = LocateFile);
   static QStringList locateAll(StandardLocation type, const QString &fileName, LocateOptions options = LocateFile);
   static QString displayName(StandardLocation type);

   static QString findExecutable(const QString &executableName, const QStringList &paths = QStringList());

   static void setTestModeEnabled(bool testMode);
   static bool isTestModeEnabled();

 private:
   // prevent construction
   QStandardPaths();
   ~QStandardPaths();
};

#endif // QT_NO_STANDARDPATHS

#endif // QSTANDARDPATHS_H
