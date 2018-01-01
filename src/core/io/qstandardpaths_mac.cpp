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
#include <qcore_mac_p.h>
#include <qcoreapplication.h>

#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>

QT_BEGIN_NAMESPACE

/*
    Translates a QStandardPaths::StandardLocation into the mac equivalent.
*/
OSType translateLocation(QStandardPaths::StandardLocation type)
{
   switch (type) {
      case QStandardPaths::ConfigLocation:
      case QStandardPaths::GenericConfigLocation:
         return kPreferencesFolderType;
      case QStandardPaths::DesktopLocation:
         return kDesktopFolderType;
      case QStandardPaths::DownloadLocation: // needs NSSearchPathForDirectoriesInDomains with NSDownloadsDirectory
      // which needs an objective-C *.mm file...
      case QStandardPaths::DocumentsLocation:
         return kDocumentsFolderType;
      case QStandardPaths::FontsLocation:
         // There are at least two different font directories on the mac: /Library/Fonts and ~/Library/Fonts.
         // To select a specific one we have to specify a different first parameter when calling FSFindFolder.
         return kFontsFolderType;
      case QStandardPaths::ApplicationsLocation:
         return kApplicationsFolderType;
      case QStandardPaths::MusicLocation:
         return kMusicDocumentsFolderType;
      case QStandardPaths::MoviesLocation:
         return kMovieDocumentsFolderType;
      case QStandardPaths::PicturesLocation:
         return kPictureDocumentsFolderType;
      case QStandardPaths::TempLocation:
         return kTemporaryFolderType;
      case QStandardPaths::GenericDataLocation:
      case QStandardPaths::RuntimeLocation:
      case QStandardPaths::DataLocation:
         return kApplicationSupportFolderType;
      case QStandardPaths::GenericCacheLocation:
      case QStandardPaths::CacheLocation:
         return kCachedDataFolderType;
      default:
         return kDesktopFolderType;
   }
}

/*
    Constructs a full unicode path from a FSRef.
*/
static QString getFullPath(const FSRef &ref)
{
   QByteArray ba(2048, 0);
   if (FSRefMakePath(&ref, reinterpret_cast<UInt8 *>(ba.data()), ba.size()) == noErr) {
      return QString::fromUtf8(ba.constData()).normalized(QString::NormalizationForm_C);
   }
   return QString();
}

static void appendOrganizationAndApp(QString &path)
{
   const QString org = QCoreApplication::organizationName();

   if (!org.isEmpty()) {
      path += QLatin1Char('/') + org;
   }

   const QString appName = QCoreApplication::applicationName();

   if (!appName.isEmpty()) {
      path += QLatin1Char('/') + appName;
   }
}

static QString macLocation(QStandardPaths::StandardLocation type, short domain)
{
   // http://developer.apple.com/documentation/Carbon/Reference/Folder_Manager/Reference/reference.html
   FSRef ref;
   OSErr err = FSFindFolder(domain, translateLocation(type), false, &ref);
   if (err) {
      return QString();
   }

   QString path = getFullPath(ref);

   if (type == QStandardPaths::DataLocation || type == QStandardPaths::CacheLocation) {
      appendOrganizationAndApp(path);
   }
   return path;
}

QString QStandardPaths::writableLocation(StandardLocation type)
{
   if (isTestModeEnabled()) {
      const QString qttestDir = QDir::homePath() + QLatin1String("/.qttest");
      QString path;
      switch (type) {
         case GenericDataLocation:
         case DataLocation:
            path = qttestDir + QLatin1String("/Application Support");
            if (type == DataLocation) {
               appendOrganizationAndApp(path);
            }
            return path;
         case GenericCacheLocation:
         case CacheLocation:
            path = qttestDir + QLatin1String("/Cache");
            if (type == CacheLocation) {
               appendOrganizationAndApp(path);
            }
            return path;
         case GenericConfigLocation:
         case ConfigLocation:
            return qttestDir + QLatin1String("/Preferences");
         default:
            break;
      }
   }

   switch (type) {
      case HomeLocation:
         return QDir::homePath();
      case TempLocation:
         return QDir::tempPath();
      case GenericDataLocation:
      case DataLocation:
      case GenericCacheLocation:
      case CacheLocation:
      case RuntimeLocation:
         return macLocation(type, kUserDomain);
      default:
         return macLocation(type, kOnAppropriateDisk);
   }
}

QStringList QStandardPaths::standardLocations(StandardLocation type)
{
   QStringList dirs;

   if (type == GenericDataLocation || type == DataLocation || type == GenericCacheLocation || type == CacheLocation) {
      const QString path = macLocation(type, kOnAppropriateDisk);
      if (!path.isEmpty()) {
         dirs.append(path);
      }
   }

   if (type == DataLocation) {
      CFBundleRef mainBundle = CFBundleGetMainBundle();
      if (mainBundle) {
         CFURLRef bundleUrl = CFBundleCopyBundleURL(mainBundle);
         CFStringRef cfBundlePath = CFURLCopyPath(bundleUrl);
         QString bundlePath = QCFString::toQString(cfBundlePath);
         CFRelease(cfBundlePath);
         CFRelease(bundleUrl);

         CFURLRef resourcesUrl = CFBundleCopyResourcesDirectoryURL(mainBundle);
         CFStringRef cfResourcesPath = CFURLCopyPath(bundleUrl);
         QString resourcesPath = QCFString::toQString(cfResourcesPath);
         CFRelease(cfResourcesPath);
         CFRelease(resourcesUrl);

         // Handle bundled vs unbundled executables. CFBundleGetMainBundle() returns
         // a valid bundle in both cases. CFBundleCopyResourcesDirectoryURL() returns
         // an absolute path for unbundled executables.
         if (resourcesPath.startsWith(QLatin1Char('/'))) {
            dirs.append(resourcesPath);
         } else {
            dirs.append(bundlePath + resourcesPath);
         }
      }
   }
   const QString localDir = writableLocation(type);
   dirs.prepend(localDir);
   return dirs;
}

QString QStandardPaths::displayName(StandardLocation type)
{
   if (QStandardPaths::HomeLocation == type) {
      return QCoreApplication::translate("QStandardPaths", "Home");
   }

   FSRef ref;
   OSErr err = FSFindFolder(kOnAppropriateDisk, translateLocation(type), false, &ref);
   if (err) {
      return QString();
   }

   QCFString displayName;
   err = LSCopyDisplayNameForRef(&ref, &displayName);
   if (err) {
      return QString();
   }

   return static_cast<QString>(displayName);
}


QT_END_NAMESPACE
