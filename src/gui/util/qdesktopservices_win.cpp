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

#include <qsettings.h>
#include <qdir.h>
#include <qsystemlibrary_p.h>
#include <qurl.h>
#include <qstringlist.h>
#include <qprocess.h>
#include <qtemporaryfile.h>
#include <qcoreapplication.h>
#include <qt_windows.h>
#include <shlobj.h>
#include <intshcut.h>

#ifndef CSIDL_MYMUSIC
#define CSIDL_MYMUSIC	13
#define CSIDL_MYVIDEO	14
#endif

#ifndef QT_NO_DESKTOPSERVICES

QT_BEGIN_NAMESPACE

static inline bool shellExecute(const QUrl &url)
{
    if (!url.isValid())
        return false;

    const QString nativeFilePath =
            url.isLocalFile() ? QDir::toNativeSeparators(url.toLocalFile()) : url.toString();
    const quintptr result = (quintptr)ShellExecute(0, 0, (wchar_t*)nativeFilePath.utf16(), 0, 0, SW_SHOWNORMAL);
    // ShellExecute returns a value greater than 32 if successful
    if (result <= 32) {
        qWarning("ShellExecute '%s' failed (error %s).", qPrintable(url.toString()), qPrintable(QString::number(result)));
        return false;
    }
    return true;
}

static bool openDocument(const QUrl &file)
{
    return shellExecute(file);
}

static QString expandEnvStrings(const QString &command)
{
   wchar_t buffer[MAX_PATH];

   if (ExpandEnvironmentStrings((wchar_t *)command.utf16(), buffer, MAX_PATH)) {
      return QString::fromWCharArray(buffer);
   } else {
      return command;
   }

}

static bool launchWebBrowser(const QUrl &url)
{
   if (url.scheme() == QLatin1String("mailto")) {
      //Retrieve the commandline for the default mail client
      //the default key used below is the command line for the mailto: shell command
      DWORD  bufferSize = sizeof(wchar_t) * MAX_PATH;
      long  returnValue =  -1;
      QString command;

      HKEY handle;
      LONG res;
      wchar_t keyValue[MAX_PATH] = {0};
      QString keyName(QLatin1String("mailto"));

      //Check if user has set preference, otherwise use default.
      res = RegOpenKeyEx(HKEY_CURRENT_USER,
                         L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\mailto\\UserChoice",
                         0, KEY_READ, &handle);
      if (res == ERROR_SUCCESS) {
         returnValue = RegQueryValueEx(handle, L"Progid", 0, 0, reinterpret_cast<unsigned char *>(keyValue), &bufferSize);
         if (!returnValue) {
            keyName = QString::fromUtf16((const ushort *)keyValue);
         }
         RegCloseKey(handle);
      }
      keyName += QLatin1String("\\Shell\\Open\\Command");
      res = RegOpenKeyExW(HKEY_CLASSES_ROOT, (const wchar_t *)keyName.utf16(), 0, KEY_READ, &handle);
      if (res != ERROR_SUCCESS) {
         return false;
      }

      bufferSize = sizeof(wchar_t) * MAX_PATH;
      returnValue = RegQueryValueEx(handle, L"", 0, 0, reinterpret_cast<unsigned char *>(keyValue), &bufferSize);
      if (!returnValue) {
         command = QString::fromRawData((QChar *)keyValue, bufferSize);
      }
      RegCloseKey(handle);

      if (returnValue) {
         return false;
      }

      command = expandEnvStrings(command);
      command = command.trimmed();
      //Make sure the path for the process is in quotes
      int index = -1 ;
      if (command[0] != QLatin1Char('\"')) {
         index = command.indexOf(QLatin1String(".exe "), 0, Qt::CaseInsensitive);
         command.insert(index + 4, QLatin1Char('\"'));
         command.insert(0, QLatin1Char('\"'));
      }
      //pass the url as the parameter
      index =  command.lastIndexOf(QLatin1String("%1"));
      if (index != -1) {
         command.replace(index, 2, url.toString());
      }
      //start the process
      PROCESS_INFORMATION pi;
      ZeroMemory(&pi, sizeof(pi));
      STARTUPINFO si;
      ZeroMemory(&si, sizeof(si));
      si.cb = sizeof(si);

      returnValue = CreateProcess(NULL, (wchar_t *)command.utf16(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

      if (!returnValue) {
         return false;
      }

      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      return true;
   }

   if (!url.isValid()) {
      return false;
   }

   if (url.scheme().isEmpty()) {
      return openDocument(url);
   }

   quintptr returnValue = (quintptr)ShellExecute(0, 0, (wchar_t *)QString::fromUtf8(url.toEncoded().constData()).utf16(),
                          0, 0, SW_SHOWNORMAL);
   return (returnValue > 32);
}

QString QDesktopServices::storageLocation(StandardLocation type)
{
   QString result;

   QSystemLibrary library(QLatin1String("shell32"));

   typedef BOOL (WINAPI * GetSpecialFolderPath)(HWND, LPWSTR, int, BOOL);
   static GetSpecialFolderPath SHGetSpecialFolderPath =
      (GetSpecialFolderPath)library.resolve("SHGetSpecialFolderPathW");
   if (!SHGetSpecialFolderPath) {
      return QString();
   }

   wchar_t path[MAX_PATH];

   switch (type) {
      case DataLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_LOCAL_APPDATA, FALSE)) {
            result = QString::fromWCharArray(path);
            if (!QCoreApplication::organizationName().isEmpty()) {
               result = result + QLatin1String("\\") + QCoreApplication::organizationName();
            }
            if (!QCoreApplication::applicationName().isEmpty()) {
               result = result + QLatin1String("\\") + QCoreApplication::applicationName();
            }
         }
         break;

      case DesktopLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_DESKTOPDIRECTORY, FALSE)) {
            result = QString::fromWCharArray(path);
         }
         break;

      case DocumentsLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_PERSONAL, FALSE)) {
            result = QString::fromWCharArray(path);
         }
         break;

      case FontsLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_FONTS, FALSE)) {
            result = QString::fromWCharArray(path);
         }
         break;

      case ApplicationsLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_PROGRAMS, FALSE)) {
            result = QString::fromWCharArray(path);
         }
         break;

      case MusicLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_MYMUSIC, FALSE)) {
            result = QString::fromWCharArray(path);
         }
         break;

      case MoviesLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_MYVIDEO, FALSE)) {
            result = QString::fromWCharArray(path);
         }
         break;

      case PicturesLocation:
         if (SHGetSpecialFolderPath(0, path, CSIDL_MYPICTURES, FALSE)) {
            result = QString::fromWCharArray(path);
         }
         break;

      case CacheLocation:
         // Although Microsoft has a Cache key it is a pointer to IE's cache, not a cache
         // location for everyone.  Most applications seem to be using a
         // cache directory located in their AppData directory
         return storageLocation(DataLocation) + QLatin1String("\\cache");

      case QDesktopServices::HomeLocation:
         return QDir::homePath();
         break;

      case QDesktopServices::TempLocation:
         return QDir::tempPath();
         break;

      default:
         break;
   }
   return result;
}

QString QDesktopServices::displayName(StandardLocation type)
{
   Q_UNUSED(type);
   return QString();
}

QT_END_NAMESPACE

#endif // QT_NO_DESKTOPSERVICES
