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

#include <qplatformdefs.h>
#include <qlibrary_p.h>
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qfilesystementry_p.h>
#include <qstringparser.h>

#if defined(QT_NO_LIBRARY) && defined(Q_OS_WIN)
#undef QT_NO_LIBRARY
#pragma message("QT_NO_LIBRARY is not supported on Windows")
#endif

#include <qt_windows.h>

extern QString qt_error_string(int code);

bool QLibraryPrivate::load_sys()
{
   // avoid 'Bad Image' message box
   UINT oldmode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

   // make the following attempts at locating the library:
   //
   // Windows
   // if (loadHints & QLibrary::ImprovedSearchHeuristics)
   //   if (absolute)
   //     fileName
   //     fileName + ".dll"
   //   else
   //     fileName + ".dll"
   //     fileName
   // else
   //   fileName
   //   fileName + ".dll"
   //
   // NB If it's a plugin we do not ever try the ".dll" extension

   QStringList attempts;
   QFileSystemEntry fsEntry(fileName);

   if (loadHints & QLibrary::ImprovedSearchHeuristics) {
      if (pluginState != IsAPlugin) {
         attempts.append(fileName + ".dll");
      }

      // If the fileName is an absolute path we try that first, otherwise we
      // use the system-specific suffix first
      if (fsEntry.isAbsolute()) {
         attempts.prepend(fileName);
      } else {
         attempts.append(fileName);
      }

   } else {
      attempts.append(fileName);

      if (pluginState != IsAPlugin) {
         attempts.append(fileName + QLatin1String(".dll"));
      }
   }

   for (const QString & attempt : attempts) {
      pHnd = LoadLibrary(&QDir::toNativeSeparators(attempt).toStdWString()[0]);

      // If we have a handle or the last error is something other than "unable
      // to find the module", then bail out
      if (pHnd || ::GetLastError() != ERROR_MOD_NOT_FOUND) {
         break;
      }
   }

   SetErrorMode(oldmode);
   if (! pHnd) {
      errorString = QLibrary::tr("Can not load library %1: %2").formatArgs(fileName, qt_error_string());

   } else {
      // Query the actual name of the library that was loaded
      errorString.clear();

      std::wstring buffer(MAX_PATH, L'\0');
      ::GetModuleFileName(pHnd, &buffer[0], MAX_PATH);

      QString moduleFileName = QString::fromStdWString(buffer);
      moduleFileName.remove(0, 1 + moduleFileName.lastIndexOf('\\'));

      const QDir dir(fsEntry.path());

      if (dir.path() == ".") {
         qualifiedFileName = moduleFileName;
      } else {
         qualifiedFileName = dir.filePath(moduleFileName);
      }
   }

   return (pHnd != 0);
}

bool QLibraryPrivate::unload_sys()
{
   if (!FreeLibrary(pHnd)) {
      errorString = QLibrary::tr("Can not unload library %1: %2").formatArg(fileName).formatArg(qt_error_string());
      return false;
   }

   errorString.clear();
   return true;
}

void *QLibraryPrivate::resolve_sys(const QString &symbol)
{
   void *address = (void *)GetProcAddress(pHnd, symbol.constData());

   if (! address) {
      errorString = QLibrary::tr("Can not resolve symbol \"%1\" in %2: %3").formatArg(symbol).formatArg(fileName).formatArg(qt_error_string());

   } else {
      errorString.clear();
   }

   return address;
}

