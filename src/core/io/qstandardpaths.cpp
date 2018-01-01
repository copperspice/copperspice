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
#include <qfileinfo.h>
#include <qhash.h>
#include <qobject.h>
#include <qcoreapplication.h>

#ifndef QT_NO_STANDARDPATHS

QT_BEGIN_NAMESPACE

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
      const QString path = *dir + QLatin1Char('/') + fileName;
      if (existsAsSpecified(path, options)) {
         return path;
      }
   }
   return QString();
}

/*!
   Tries to find all files or directories called \a fileName in the standard locations
   for \a type.

   The \a options flag allows to specify whether to look for files or directories.

   Returns the list of all the files that were found.
 */
QStringList QStandardPaths::locateAll(StandardLocation type, const QString &fileName, LocateOptions options)
{
   const QStringList &dirs = standardLocations(type);
   QStringList result;
   for (QStringList::const_iterator dir = dirs.constBegin(); dir != dirs.constEnd(); ++dir) {
      const QString path = *dir + QLatin1Char('/') + fileName;
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
   const QStringList pathExt = QString::fromLocal8Bit(qgetenv("PATHEXT")).toLower().split(QLatin1Char(';'));
   return pathExt.contains(QLatin1String(".exe"), Qt::CaseInsensitive) ?
          pathExt :
          QStringList() << QLatin1String(".exe") << QLatin1String(".com")
          << QLatin1String(".bat") << QLatin1String(".cmd");
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

static inline QString searchExecutable(const QStringList &searchPaths,
                                       const QString &executableName)
{
   const QDir currentDir = QDir::current();

   for (const QString & searchPath : searchPaths) {
      const QString candidate = currentDir.absoluteFilePath(searchPath + QLatin1Char('/') + executableName);
      const QString absPath = checkExecutable(candidate);

      if (! absPath.isEmpty()) {
         return absPath;
      }
   }
   return QString();
}

#ifdef Q_OS_WIN

// Find executable appending candidate suffixes, used for suffix-less executables
// on Windows.
static inline QString
searchExecutableAppendSuffix(const QStringList &searchPaths,
                             const QString &executableName,
                             const QStringList &suffixes)
{
   const QDir currentDir = QDir::current();

   for (const QString & searchPath : searchPaths) {
      const QString candidateRoot = currentDir.absoluteFilePath(searchPath + QLatin1Char('/') + executableName);

      for (const QString & suffix : suffixes) {
         const QString absPath = checkExecutable(candidateRoot + suffix);

         if (! absPath.isEmpty()) {
            return absPath;
         }
      }
   }
   return QString();
}

#endif // Q_OS_WIN

/*!
  Finds the executable named \a executableName in the paths specified by \a paths,
  or the system paths if \a paths is empty.

  On most operating systems the system path is determined by the PATH environment variable.

  The directories where to search for the executable can be set in the \a paths argument.
  To search in both your own paths and the system paths, call findExecutable twice, once with
  \a paths set and once with \a paths empty.

  Symlinks are not resolved, in order to preserve behavior for the case of executables
  whose behavior depends on the name they are invoked with.

  \note On Windows, the usual executable extensions (from the PATHEXT environment variable)
  are automatically appended, so that for instance findExecutable("foo") will find foo.exe
  or foo.bat if present.

  Returns the absolute file path to the executable, or an empty string if not found.
 */
QString QStandardPaths::findExecutable(const QString &executableName, const QStringList &paths)
{
   if (QFileInfo(executableName).isAbsolute()) {
      return checkExecutable(executableName);
   }

   QStringList searchPaths = paths;
   if (paths.isEmpty()) {
      QByteArray pEnv = qgetenv("PATH");
#if defined(Q_OS_WIN)
      const QLatin1Char pathSep(';');
#else
      const QLatin1Char pathSep(':');
#endif
      // Remove trailing slashes, which occur on Windows.
      const QStringList rawPaths = QString::fromLocal8Bit(pEnv.constData()).split(pathSep, QString::SkipEmptyParts);
      searchPaths.reserve(rawPaths.size());

      for (const QString & rawPath : rawPaths) {
         QString cleanPath = QDir::cleanPath(rawPath);

         if (cleanPath.size() > 1 && cleanPath.endsWith(QLatin1Char('/'))) {
            cleanPath.truncate(cleanPath.size() - 1);
         }

         searchPaths.push_back(cleanPath);
      }
   }

#ifdef Q_OS_WIN
   // On Windows, if the name does not have a suffix or a suffix not
   // in PATHEXT ("xx.foo"), append suffixes from PATHEXT.
   static const QStringList executable_extensions = executableExtensions();
   if (executableName.contains(QLatin1Char('.'))) {
      const QString suffix = QFileInfo(executableName).suffix();
      if (suffix.isEmpty() || !executable_extensions.contains(QLatin1Char('.') + suffix, Qt::CaseInsensitive)) {
         return searchExecutableAppendSuffix(searchPaths, executableName, executable_extensions);
      }
   } else {
      return searchExecutableAppendSuffix(searchPaths, executableName, executable_extensions);
   }
#endif
   return searchExecutable(searchPaths, executableName);
}

/*!
    \fn QString QStandardPaths::displayName(StandardLocation type)

    Returns a localized display name for the given location \a type or
    an empty QString if no relevant location can be found.
*/

#if !defined(Q_OS_MAC)
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
      case DataLocation:
         return QCoreApplication::translate("QStandardPaths", "Application Data");
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
   }
   // not reached
   return QString();
}
#endif

/*!
  \fn void QStandardPaths::enableTestMode(bool testMode)
  \obsolete Use QStandardPaths::setTestModeEnabled
 */
/*!
  \fn void QStandardPaths::setTestModeEnabled(bool testMode)

  If \a testMode is true, this enables a special "test mode" in
  QStandardPaths, which changes writable locations
  to point to test directories, in order to prevent auto tests from reading from
  or writing to the current user's configuration.

  This affects the locations into which test programs might write files:
  GenericDataLocation, DataLocation, ConfigLocation, GenericConfigLocation,
  GenericCacheLocation, CacheLocation.
  Other locations are not affected.

  On Unix, XDG_DATA_HOME is set to ~/.qttest/share, XDG_CONFIG_HOME is
  set to ~/.qttest/config, and XDG_CACHE_HOME is set to ~/.qttest/cache.

  On Mac, data goes to "~/.qttest/Application Support", cache goes to
  ~/.qttest/Cache, and config goes to ~/.qttest/Preferences.

  On Windows, everything goes to a "qttest" directory under Application Data.
*/

static bool qsp_testMode = false;

void QStandardPaths::enableTestMode(bool testMode)
{
   qsp_testMode = testMode;
}

void QStandardPaths::setTestModeEnabled(bool testMode)
{
   qsp_testMode = testMode;
}

/*!
  \fn void QStandardPaths::isTestModeEnabled()

  \internal

  Returns true if test mode is enabled in QStandardPaths; otherwise returns false.
*/

bool QStandardPaths::isTestModeEnabled()
{
   return qsp_testMode;
}


QT_END_NAMESPACE

#endif // QT_NO_STANDARDPATHS
