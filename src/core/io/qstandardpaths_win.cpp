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

#include <qstandardpaths.h>

#include <qdir.h>
#include <qsystemlibrary_p.h>
#include <qstringlist.h>
#include <qcoreapplication.h>
#include <qt_windows.h>
#include <shlobj.h>
#include <intshcut.h>

#ifndef CSIDL_MYMUSIC
#define CSIDL_MYMUSIC 13
#define CSIDL_MYVIDEO 14
#endif

#ifndef QT_NO_STANDARDPATHS

QT_BEGIN_NAMESPACE

typedef BOOL (WINAPI *GetSpecialFolderPath)(HWND, LPWSTR, int, BOOL);
static GetSpecialFolderPath resolveGetSpecialFolderPath()
{
   static GetSpecialFolderPath gsfp = 0;
   if (!gsfp) {
      QSystemLibrary library(QLatin1String("shell32"));
      gsfp = (GetSpecialFolderPath)library.resolve("SHGetSpecialFolderPathW");
   }
   return gsfp;
}

static QString convertCharArray(const wchar_t *path)
{
   return QDir::fromNativeSeparators(QString::fromWCharArray(path));
}

QString QStandardPaths::writableLocation(StandardLocation type)
{
   QString result;

   static GetSpecialFolderPath SHGetSpecialFolderPath = resolveGetSpecialFolderPath();
   if (!SHGetSpecialFolderPath) {
      return QString();
   }

   wchar_t path[MAX_PATH];

   switch (type) {
      case ConfigLocation: // same as DataLocation, on Windows (oversight, but too late to fix it)
      case GenericConfigLocation: // same as GenericDataLocation, on Windows
      case DataLocation:
      case GenericDataLocation:

         if (SHGetSpecialFolderPath(0, path, CSIDL_LOCAL_APPDATA, FALSE)) {
            result = convertCharArray(path);
         }

         if (isTestModeEnabled()) {
            result += QLatin1String("/qttest");
         }

         if (type != GenericDataLocation && type != GenericConfigLocation) {
            if (!QCoreApplication::organizationName().isEmpty()) {
               result += QLatin1Char('/') + QCoreApplication::organizationName();
            }
            if (!QCoreApplication::applicationName().isEmpty()) {
               result += QLatin1Char('/') + QCoreApplication::applicationName();
            }
         }

         break;

      case DesktopLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_DESKTOPDIRECTORY, FALSE)) {
            result = convertCharArray(path);
         }
         break;

      case DownloadLocation: // TODO implement with SHGetKnownFolderPath(FOLDERID_Downloads) (starting from Vista)
      case DocumentsLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_PERSONAL, FALSE)) {
            result = convertCharArray(path);
         }
         break;

      case FontsLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_FONTS, FALSE)) {
            result = convertCharArray(path);
         }
         break;

      case ApplicationsLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_PROGRAMS, FALSE)) {
            result = convertCharArray(path);
         }
         break;

      case MusicLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_MYMUSIC, FALSE)) {
            result = convertCharArray(path);
         }
         break;

      case MoviesLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_MYVIDEO, FALSE)) {
            result = convertCharArray(path);
         }
         break;

      case PicturesLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_MYPICTURES, FALSE)) {
            result = convertCharArray(path);
         }
         break;

      case CacheLocation:
         // Although Microsoft has a Cache key it is a pointer to IE's cache, not a cache
         // location for everyone.  Most applications seem to be using a
         // cache directory located in their AppData directory
         return writableLocation(DataLocation) + QLatin1String("/cache");

      case GenericCacheLocation:
         return writableLocation(GenericDataLocation) + QLatin1String("/cache");

      case RuntimeLocation:
      case HomeLocation:
         result = QDir::homePath();
         break;

      case TempLocation:
         result = QDir::tempPath();
         break;
   }
   return result;
}

QStringList QStandardPaths::standardLocations(StandardLocation type)
{
   QStringList dirs;

   // type-specific handling goes here
   static GetSpecialFolderPath SHGetSpecialFolderPath = resolveGetSpecialFolderPath();

   if (SHGetSpecialFolderPath) {
      wchar_t path[MAX_PATH];
      switch (type) {
         case ConfigLocation: // same as DataLocation, on Windows (oversight, but too late to fix it)
         case GenericConfigLocation: // same as GenericDataLocation, on Windows
         case DataLocation:
         case GenericDataLocation:
            if (SHGetSpecialFolderPath(0, path, CSIDL_COMMON_APPDATA, FALSE)) {
               QString result = convertCharArray(path);
               if (type != GenericDataLocation && type != GenericConfigLocation) {

                  if (!QCoreApplication::organizationName().isEmpty()) {
                     result += QLatin1Char('/') + QCoreApplication::organizationName();
                  }
                  if (!QCoreApplication::applicationName().isEmpty()) {
                     result += QLatin1Char('/') + QCoreApplication::applicationName();
                  }

               }
               dirs.append(result);

               if (type != GenericDataLocation) {
                  dirs.append(QCoreApplication::applicationDirPath());
                  dirs.append(QCoreApplication::applicationDirPath() + QLatin1String("/data"));
               }
            }

            break;
         default:
            break;
      }
   }

   const QString localDir = writableLocation(type);
   dirs.prepend(localDir);
   return dirs;
}

QT_END_NAMESPACE

#endif // QT_NO_STANDARDPATHS
