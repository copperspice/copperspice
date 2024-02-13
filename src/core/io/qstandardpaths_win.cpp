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

#include <qstandardpaths.h>

#include <qdir.h>
#include <qstringlist.h>
#include <qcoreapplication.h>
#include <qt_windows.h>

#include <qsystemlibrary_p.h>

#include <shlobj.h>
#include <intshcut.h>

const GUID qCLSID_FOLDERID_Downloads = { 0x374de290, 0x123f, 0x4565, { 0x91, 0x64,  0x39,  0xc4,  0x92,  0x5e,  0x46,  0x7b } };

#ifndef CSIDL_MYMUSIC
#define CSIDL_MYMUSIC 13
#define CSIDL_MYVIDEO 14
#endif

#ifndef QT_NO_STANDARDPATHS

static QString convertCharArray(const std::wstring &path)
{
   return QDir::fromNativeSeparators(QString::fromStdWString(path));
}

static inline bool isGenericConfigLocation(QStandardPaths::StandardLocation type)
{
   return type == QStandardPaths::GenericConfigLocation || type == QStandardPaths::GenericDataLocation;
}

static inline bool isConfigLocation(QStandardPaths::StandardLocation type)
{
   return type == QStandardPaths::ConfigLocation || type == QStandardPaths::AppConfigLocation
         || type == QStandardPaths::AppDataLocation || type == QStandardPaths::AppLocalDataLocation
         || isGenericConfigLocation(type);
}

static void appendOrganizationAndApp(QString &path)
{
   const QString &org = QCoreApplication::organizationName();

   if (! org.isEmpty()) {
      path += '/' + org;
   }

   const QString &appName = QCoreApplication::applicationName();

   if (! appName.isEmpty()) {
      path += '/' + appName;
   }
}

// Map QStandardPaths::StandardLocation to CLSID of SHGetSpecialFolderPath()
static int writableSpecialFolderClsid(QStandardPaths::StandardLocation type)
{
   static const int clsids[] = {
      CSIDL_DESKTOPDIRECTORY, // DesktopLocation
      CSIDL_PERSONAL,         // DocumentsLocation
      CSIDL_FONTS,            // FontsLocation
      CSIDL_PROGRAMS,         // ApplicationsLocation
      CSIDL_MYMUSIC,          // MusicLocation
      CSIDL_MYVIDEO,          // MoviesLocation
      CSIDL_MYPICTURES,       // PicturesLocation
      -1, -1,                 // TempLocation/HomeLocation
      CSIDL_LOCAL_APPDATA,    // AppLocalDataLocation ("Local" path), AppLocalDataLocation = DataLocation
      -1,                     // CacheLocation
      CSIDL_LOCAL_APPDATA,    // GenericDataLocation ("Local" path)
      -1,                     // RuntimeLocation
      CSIDL_LOCAL_APPDATA,    // ConfigLocation ("Local" path)
      -1, -1,                 // DownloadLocation/GenericCacheLocation
      CSIDL_LOCAL_APPDATA,    // GenericConfigLocation ("Local" path)
      CSIDL_APPDATA,          // AppDataLocation ("Roaming" path)
      CSIDL_LOCAL_APPDATA,    // AppConfigLocation ("Local" path)
   };

   static_assert(std::size(clsids) == size_t(QStandardPaths::AppConfigLocation + 1));

   return size_t(type) < sizeof(clsids) / sizeof(clsids[0]) ? clsids[type] : -1;
}

static QString sHGetSpecialFolderPath(int clsid, QStandardPaths::StandardLocation type, bool warn = false)
{
   QString result;
   std::wstring path(MAX_PATH, L'\0');

   if (clsid >= 0 && SHGetSpecialFolderPath(nullptr, path.data(), clsid, FALSE)) {
      result = convertCharArray(path);

   } else {
      if (warn) {
         qErrnoWarning("SHGetSpecialFolderPath() failed for standard location \"%s\", clsid=0x%x.",
               csPrintable(QStandardPaths::displayName(type)), clsid);
      }
   }

   return result;
}

static QString sHGetKnownFolderPath(const GUID &clsid, QStandardPaths::StandardLocation type, bool warn = false)
{
   QString result;

   using GetKnownFolderPath = HRESULT (WINAPI *)(const GUID &, DWORD, HANDLE, LPWSTR *);

   // vista and newer
   static const GetKnownFolderPath sHGetKnownFolderPath =
         reinterpret_cast<GetKnownFolderPath>(QSystemLibrary::resolve("shell32", "SHGetKnownFolderPath"));

   LPWSTR path;

   if (sHGetKnownFolderPath && SUCCEEDED(sHGetKnownFolderPath(clsid, 0, nullptr, &path))) {
      result = convertCharArray(path);
      CoTaskMemFree(path);

   } else {
      if (warn) {
         qErrnoWarning("SHGetKnownFolderPath() failed for standard location \"%s\".",
               csPrintable(QStandardPaths::displayName(type)));
      }
   }

   return result;
}

QString QStandardPaths::writableLocation(StandardLocation type)
{
   QString result;

   switch (type) {

      case DownloadLocation:
         result = sHGetKnownFolderPath(qCLSID_FOLDERID_Downloads, type);

         if (result.isEmpty()) {
            result = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
         }

         break;

      case CacheLocation:
         // Although Microsoft has a Cache key it is a pointer to IE's cache, not a cache
         // location for everyone.  Most applications seem to be using a
         // cache directory located in their AppData directory

         result = sHGetSpecialFolderPath(writableSpecialFolderClsid(AppLocalDataLocation), type, /* warn */ true);

         if (! result.isEmpty()) {
            appendOrganizationAndApp(result);
            result += "/cache";
         }

         break;

      case GenericCacheLocation:
         result = sHGetSpecialFolderPath(writableSpecialFolderClsid(GenericDataLocation), type, /* warn */ true);

         if (! result.isEmpty()) {
            result += "/cache";
         }

         break;

      case RuntimeLocation:
      case HomeLocation:
         result = QDir::homePath();
         break;

      case TempLocation:
         result = QDir::tempPath();
         break;

      default:
         result = sHGetSpecialFolderPath(writableSpecialFolderClsid(type), type, /* warn */ isConfigLocation(type));

         if (! result.isEmpty() && isConfigLocation(type)) {

            if (! isGenericConfigLocation(type)) {
               appendOrganizationAndApp(result);
            }
         }

         break;
   }

   return result;
}

QStringList QStandardPaths::standardLocations(StandardLocation type)
{
   QStringList dirs;
   const QString localDir = writableLocation(type);

   if (! localDir.isEmpty()) {
      dirs.append(localDir);
   }

   // type-specific handling goes here
   if (isConfigLocation(type)) {
      QString programData = sHGetSpecialFolderPath(CSIDL_COMMON_APPDATA, type);

      if (! programData.isEmpty()) {
         if (! isGenericConfigLocation(type)) {
            appendOrganizationAndApp(programData);
         }

         dirs.append(programData);
      }

      dirs.append(QCoreApplication::applicationDirPath());
      dirs.append(QCoreApplication::applicationDirPath() + "/data");
   }

   return dirs;
}

#endif
