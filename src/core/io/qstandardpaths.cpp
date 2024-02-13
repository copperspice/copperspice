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

#include <qcoreapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qhash.h>
#include <qobject.h>
#include <qstringparser.h>

#ifndef QT_NO_STANDARDPATHS

static bool existsAsSpecified(const QString &path, QStandardPaths::LocateOptions options)
{
   if (options & QStandardPaths::LocateDirectory) {
      return QDir(path).exists();
   }

   return QFileInfo(path).isFile();
}

QString QStandardPaths::locate(StandardLocation type, const QString &fileName, LocateOptions options)
{
   const QStringList &dirs = standardLocations(type);

   for (QStringList::const_iterator dir = dirs.constBegin(); dir != dirs.constEnd(); ++dir) {
      const QString path = *dir + '/' + fileName;

      if (existsAsSpecified(path, options)) {
         return path;
      }
   }

   return QString();
}

QStringList QStandardPaths::locateAll(StandardLocation type, const QString &fileName, LocateOptions options)
{
   const QStringList &dirs = standardLocations(type);
   QStringList result;

   for (QStringList::const_iterator dir = dirs.constBegin(); dir != dirs.constEnd(); ++dir) {
      const QString path = *dir + '/' + fileName;

      if (existsAsSpecified(path, options)) {
         result.append(path);
      }
   }

   return result;
}

#ifdef Q_OS_WIN
static QStringList executableExtensions()
{
   // If %PATHEXT% does not contain .exe, it is either empty, malformed, or distorted in ways that we cannot support, anyway.
   const QStringList pathExt = QString::fromUtf8(qgetenv("PATHEXT")).toLower().split(';');

   return pathExt.contains(".exe", Qt::CaseInsensitive) ? pathExt : QStringList() << ".exe" << ".com" << ".bat" << ".cmd";
}
#endif

static QString checkExecutable(const QString &path)
{
   const QFileInfo info(path);

   if (info.isBundle()) {
      return info.bundleName();
   }

   if (info.isFile() && info.isExecutable()) {
      return QDir::cleanPath(path);
   }

   return QString();
}

static inline QString searchExecutable(const QStringList &searchPaths, const QString &executableName)
{
   const QDir currentDir = QDir::current();

   for (const QString &searchPath : searchPaths) {
      const QString candidate = currentDir.absoluteFilePath(searchPath + '/' + executableName);
      const QString absPath = checkExecutable(candidate);

      if (! absPath.isEmpty()) {
         return absPath;
      }
   }

   return QString();
}

#ifdef Q_OS_WIN
// Find executable appending candidate suffixes, used for suffix-less executables on Windows.

static inline QString searchExecutableAppendSuffix(const QStringList &searchPaths,
      const QString &executableName, const QStringList &suffixes)
{
   const QDir currentDir = QDir::current();

   for (const QString &searchPath : searchPaths) {
      const QString candidateRoot = currentDir.absoluteFilePath(searchPath + '/' + executableName);

      for (const QString &suffix : suffixes) {
         const QString absPath = checkExecutable(candidateRoot + suffix);

         if (! absPath.isEmpty()) {
            return absPath;
         }
      }
   }

   return QString();
}
#endif

QString QStandardPaths::findExecutable(const QString &executableName, const QStringList &paths)
{
   if (QFileInfo(executableName).isAbsolute()) {
      return checkExecutable(executableName);
   }

   QStringList searchPaths = paths;

   if (paths.isEmpty()) {
      QByteArray pEnv = qgetenv("PATH");

#if defined(Q_OS_WIN)
      const QChar pathSep(';');
#else
      const QChar pathSep(':');
#endif

      // Remove trailing slashes, which occur on Windows
      const QStringList rawPaths = QString::fromUtf8(pEnv.constData()).split(pathSep, QStringParser::SkipEmptyParts);

      for (const QString &rawPath : rawPaths) {
         QString cleanPath = QDir::cleanPath(rawPath);

         if (cleanPath.size() > 1 && cleanPath.endsWith('/')) {
            cleanPath.truncate(cleanPath.size() - 1);
         }

         searchPaths.push_back(cleanPath);
      }
   }

#ifdef Q_OS_WIN
   // On Windows, if the name does not have a suffix or a suffix not
   // in PATHEXT ("xx.foo"), append suffixes from PATHEXT.

   static const QStringList executable_extensions = executableExtensions();

   if (executableName.contains('.')) {
      const QString suffix = QFileInfo(executableName).suffix();

      if (suffix.isEmpty() || ! executable_extensions.contains('.' + suffix, Qt::CaseInsensitive)) {
         return searchExecutableAppendSuffix(searchPaths, executableName, executable_extensions);
      }

   } else {
      return searchExecutableAppendSuffix(searchPaths, executableName, executable_extensions);
   }

#endif

   return searchExecutable(searchPaths, executableName);
}

#if ! defined(Q_OS_DARWIN)
QString QStandardPaths::displayName(StandardLocation type)
{
   switch (type) {
      case DesktopLocation:
         return QCoreApplication::translate("QStandardPaths", "Desktop");

      case DocumentsLocation:
         return QCoreApplication::translate("QStandardPaths", "Documents");

      case FontsLocation:
         return QCoreApplication::translate("QStandardPaths", "Fonts");

      case ApplicationsLocation:
         return QCoreApplication::translate("QStandardPaths", "Applications");

      case MusicLocation:
         return QCoreApplication::translate("QStandardPaths", "Music");

      case MoviesLocation:
         return QCoreApplication::translate("QStandardPaths", "Movies");

      case PicturesLocation:
         return QCoreApplication::translate("QStandardPaths", "Pictures");

      case TempLocation:
         return QCoreApplication::translate("QStandardPaths", "Temporary Directory");

      case HomeLocation:
         return QCoreApplication::translate("QStandardPaths", "Home");

      case CacheLocation:
         return QCoreApplication::translate("QStandardPaths", "Cache");

      case GenericDataLocation:
         return QCoreApplication::translate("QStandardPaths", "Shared Data");

      case RuntimeLocation:
         return QCoreApplication::translate("QStandardPaths", "Runtime");

      case ConfigLocation:
         return QCoreApplication::translate("QStandardPaths", "Configuration");

      case GenericConfigLocation:
         return QCoreApplication::translate("QStandardPaths", "Shared Configuration");

      case GenericCacheLocation:
         return QCoreApplication::translate("QStandardPaths", "Shared Cache");

      case DownloadLocation:
         return QCoreApplication::translate("QStandardPaths", "Download");

      case AppDataLocation:
      case AppLocalDataLocation:
         return QCoreApplication::translate("QStandardPaths", "Application Data");

      case AppConfigLocation:
         return QCoreApplication::translate("QStandardPaths", "Application Configuration");
   }

   // not reached

   return QString();
}
#endif

static bool qsp_testMode = false;

void QStandardPaths::setTestModeEnabled(bool testMode)
{
   qsp_testMode = testMode;
}

bool QStandardPaths::isTestModeEnabled()
{
   return qsp_testMode;
}

#endif // QT_NO_STANDARDPATHS
