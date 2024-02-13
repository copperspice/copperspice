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
#include <qfile.h>
#include <qhash.h>
#include <qtextstream.h>
#include <qregularexpression.h>

#include <qfilesystemengine_p.h>

#include <errno.h>
#include <stdlib.h>

#ifndef QT_NO_STANDARDPATHS

static void appendOrganizationAndApp(QString &path)
{
   const QString org = QCoreApplication::organizationName();

   if (! org.isEmpty()) {
      path += '/' + org;
   }

   const QString appName = QCoreApplication::applicationName();

   if (! appName.isEmpty()) {
      path += '/' + appName;
   }
}

QString QStandardPaths::writableLocation(StandardLocation type)
{
   switch (type) {
      case HomeLocation:
         return QDir::homePath();

      case TempLocation:
         return QDir::tempPath();

      case CacheLocation:
      case GenericCacheLocation: {
         // http://standards.freedesktop.org/basedir-spec/basedir-spec-0.6.html
         QString xdgCacheHome = QFile::decodeName(qgetenv("XDG_CACHE_HOME"));

         if (xdgCacheHome.isEmpty()) {
            xdgCacheHome = QDir::homePath() + "/.cache";
         }

         if (type == QStandardPaths::CacheLocation) {
            appendOrganizationAndApp(xdgCacheHome);
         }

         return xdgCacheHome;
      }

      case AppDataLocation:
      case AppLocalDataLocation:
      case GenericDataLocation: {
         QString xdgDataHome = QFile::decodeName(qgetenv("XDG_DATA_HOME"));

         if (xdgDataHome.isEmpty()) {
            xdgDataHome = QDir::homePath() + "/.local/share";
         }

         if (type == AppDataLocation || type == AppLocalDataLocation) {
            appendOrganizationAndApp(xdgDataHome);
         }

         return xdgDataHome;
      }

      case AppConfigLocation:
      case ConfigLocation:
      case GenericConfigLocation:  {
         // http://standards.freedesktop.org/basedir-spec/latest/
         QString xdgConfigHome = QFile::decodeName(qgetenv("XDG_CONFIG_HOME"));

         if (xdgConfigHome.isEmpty()) {
            xdgConfigHome = QDir::homePath() + "/.config";
         }

         if (type == AppConfigLocation) {
            appendOrganizationAndApp(xdgConfigHome);
         }

         return xdgConfigHome;
      }

      case RuntimeLocation: {
         const uid_t myUid = geteuid();
         // http://standards.freedesktop.org/basedir-spec/latest/

         QFileInfo fileInfo;

         QString xdgRuntimeDir = QFile::decodeName(qgetenv("XDG_RUNTIME_DIR"));

         if (xdgRuntimeDir.isEmpty()) {
            const QString userName = QFileSystemEngine::resolveUserName(myUid);
            xdgRuntimeDir = QDir::tempPath() + "/runtime-" + userName;

            fileInfo.setFile(xdgRuntimeDir);

            if (! fileInfo.isDir()) {
               if (! QDir().mkdir(xdgRuntimeDir)) {
                  qWarning("QStandardPaths::writableLocation() Error creating runtime directory %s: %s",
                        csPrintable(xdgRuntimeDir), csPrintable(qt_error_string(errno)));
                  return QString();
               }
            }

            qWarning("QStandardPaths::writableLocation() XDG_RUNTIME_DIR not set, defaulting to %s",
                  csPrintable(xdgRuntimeDir));

         } else {
            fileInfo.setFile(xdgRuntimeDir);

            if (! fileInfo.exists()) {
               qWarning("QStandardPaths::writableLocation() XDG_RUNTIME_DIR points to an invalid path of %s, "
                     "create using permissions of 0700", csPrintable(xdgRuntimeDir));

               return QString();
            }

            if (! fileInfo.isDir()) {
               qWarning("QStandardPaths::writableLocation() XDG_RUNTIME_DIR points to %s which is not a directory",
                     csPrintable(xdgRuntimeDir));

               return QString();
            }
         }

         // the directory MUST be owned by the user

         if (fileInfo.ownerId() != myUid) {
            qWarning("QStandardPaths::writableLocation() Incorrect ownership on runtime directory %s, %d instead of %d",
                  csPrintable(xdgRuntimeDir), fileInfo.ownerId(), myUid);

            return QString();
         }

         // Unix access mode MUST be 0700
         const QFile::Permissions wantedPerms = QFile::ReadUser | QFile::WriteUser | QFile::ExeUser
               | QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner;

         if (fileInfo.permissions() != wantedPerms) {
            QFile file(xdgRuntimeDir);

            if (! file.setPermissions(wantedPerms)) {
               qWarning("QStandardPaths::writableLocation() Unable to set permissions on runtime directory %s: %s",
                     csPrintable(xdgRuntimeDir), csPrintable(file.errorString()));

               return QString();
            }
         }

         return xdgRuntimeDir;

      }

      default:
         break;
   }

   // http://www.freedesktop.org/wiki/Software/xdg-user-dirs
   QString xdgConfigHome = QFile::decodeName(qgetenv("XDG_CONFIG_HOME"));

   if (xdgConfigHome.isEmpty()) {
      xdgConfigHome = QDir::homePath() + "/.config";
   }

   QFile file(xdgConfigHome + "/user-dirs.dirs");

   if (! isTestModeEnabled() && file.open(QIODevice::ReadOnly)) {
      QHash<QString, QString> lines;
      QTextStream stream(&file);

      // Only look for lines like: XDG_DESKTOP_DIR="$HOME/Desktop"
      QRegularExpression exp("^XDG_(.*)_DIR=(.*)$");
      QRegularExpressionMatch match;

      while (! stream.atEnd()) {
         const QString &line = stream.readLine();
         match = exp.match(line);

         if (match.hasMatch()) {
            const QString key = match.captured(1);
            QString value     = match.captured(2);

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
            key = "DESKTOP";
            break;

         case DocumentsLocation:
            key = "DOCUMENTS";
            break;

         case PicturesLocation:
            key = "PICTURES";
            break;

         case MusicLocation:
            key = "MUSIC";
            break;

         case MoviesLocation:
            key = "VIDEOS";
            break;

         case DownloadLocation:
            key = "DOWNLOAD";
            break;

         default:
            break;
      }

      if (! key.isEmpty()) {
         QString value = lines.value(key);

         if (!value.isEmpty()) {
            // value can start with $HOME

            if (value.startsWith("$HOME")) {
               value = QDir::homePath() + value.mid(5);
            }

            if (value.length() > 1 && value.endsWith('/')) {
               value.chop(1);
            }

            return value;
         }
      }
   }

   QString path;

   switch (type) {
      case DesktopLocation:
         path = QDir::homePath() + "/Desktop";
         break;

      case DocumentsLocation:
         path = QDir::homePath() + "/Documents";
         break;

      case PicturesLocation:
         path = QDir::homePath() + "/Pictures";
         break;

      case FontsLocation:
         path = QDir::homePath() + "/.fonts";
         break;

      case MusicLocation:
         path = QDir::homePath() + "/Music";
         break;

      case MoviesLocation:
         path = QDir::homePath() + "/Videos";
         break;

      case DownloadLocation:
         path = QDir::homePath() + "/Downloads";
         break;

      case ApplicationsLocation:
         path = writableLocation(GenericDataLocation) + "/applications";
         break;

      default:
         break;
   }

   return path;
}

static QStringList xdgDataDirs()
{
   QStringList dirs;

   // http://standards.freedesktop.org/basedir-spec/latest/
   QString xdgDataDirsEnv = QFile::decodeName(qgetenv("XDG_DATA_DIRS"));

   if (xdgDataDirsEnv.isEmpty()) {
      dirs.append(QString::fromLatin1("/usr/local/share"));
      dirs.append(QString::fromLatin1("/usr/share"));

   } else {
      dirs = xdgDataDirsEnv.split(':', QStringParser::SkipEmptyParts);

      // Normalize paths, skip relative paths
      QMutableListIterator<QString> it(dirs);

      while (it.hasNext()) {
         const QString dir = it.next();

         if (! dir.startsWith('/')) {
            it.remove();
         } else {
            it.setValue(QDir::cleanPath(dir));
         }
      }

      dirs.removeDuplicates();
   }

   return dirs;
}

static QStringList xdgConfigDirs()
{
   QStringList dirs;

   // http://standards.freedesktop.org/basedir-spec/latest/
   const QString xdgConfigDirs = QFile::decodeName(qgetenv("XDG_CONFIG_DIRS"));

   if (xdgConfigDirs.isEmpty()) {
      dirs.append(QString::fromLatin1("/etc/xdg"));
   } else {
      dirs = xdgConfigDirs.split(':');
   }

   return dirs;
}

QStringList QStandardPaths::standardLocations(StandardLocation type)
{
   QStringList dirs;

   switch (type) {
      case ConfigLocation:
      case GenericConfigLocation:
         dirs = xdgConfigDirs();
         break;

      case AppConfigLocation:
         dirs = xdgConfigDirs();

         for (int i = 0; i < dirs.count(); ++i) {
            appendOrganizationAndApp(dirs[i]);
         }

         break;

      case GenericDataLocation:
         dirs = xdgDataDirs();
         break;

      case ApplicationsLocation:
         dirs = xdgDataDirs();

         for (int i = 0; i < dirs.count(); ++i) {
            dirs[i].append("/applications");
         }

         break;

      case AppDataLocation:
      case AppLocalDataLocation:
         dirs = xdgDataDirs();

         for (int i = 0; i < dirs.count(); ++i) {
            appendOrganizationAndApp(dirs[i]);
         }

         break;

      default:
         break;
   }

   const QString localDir = writableLocation(type);
   dirs.prepend(localDir);

   return dirs;
}

#endif
