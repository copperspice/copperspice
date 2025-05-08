/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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
#include <qurl.h>

#include <qcore_mac_p.h>

#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>


// Translates a QStandardPaths::StandardLocation into the mac equivalent
OSType translateLocation(QStandardPaths::StandardLocation type)
{
   switch (type) {
      case QStandardPaths::ConfigLocation:
      case QStandardPaths::GenericConfigLocation:
      case QStandardPaths::AppConfigLocation:
         return kPreferencesFolderType;

      case QStandardPaths::DesktopLocation:
         return kDesktopFolderType;

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
      case QStandardPaths::AppDataLocation:
      case QStandardPaths::AppLocalDataLocation:
         return kApplicationSupportFolderType;

      case QStandardPaths::GenericCacheLocation:
      case QStandardPaths::CacheLocation:
         return kCachedDataFolderType;

      default:
         return kDesktopFolderType;
   }
}

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

   if (! org.isEmpty()) {
      path += '/' + org;
   }

   const QString appName = QCoreApplication::applicationName();

   if (! appName.isEmpty()) {
      path += '/' + appName;
   }
}

static QString macLocation(QStandardPaths::StandardLocation type, short domain)
{
   // https://developer.apple.com/library/mac/documentation/Cocoa/Reference/Foundation/Classes/NSFileManager_Class/index.html
   if (type == QStandardPaths::DownloadLocation) {
      NSFileManager *fileManager = [NSFileManager defaultManager];
      NSURL *url = [fileManager URLForDirectory:NSDownloadsDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:nil];

      if (! url) {
         return QString();
      }

      return QString::fromNSString([url path]);
   }

   // http://developer.apple.com/documentation/Carbon/Reference/Folder_Manager/Reference/reference.html
   FSRef ref;

   OSErr err = FSFindFolder(domain, translateLocation(type), false, &ref);

   if (err) {
      return QString();
   }

   QString path = getFullPath(ref);

   if (type == QStandardPaths::AppDataLocation || type == QStandardPaths::AppLocalDataLocation ||
         type == QStandardPaths::CacheLocation || type == QStandardPaths::AppConfigLocation) {
      appendOrganizationAndApp(path);
   }

   return path;
}

QString QStandardPaths::writableLocation(StandardLocation type)
{
   if (isTestModeEnabled()) {
      const QString qttestDir = QDir::homePath() + "/.qttest";
      QString path;

      switch (type) {
         case GenericDataLocation:
         case AppDataLocation:
         case AppLocalDataLocation:
            path = qttestDir + "/Application Support";

            if (type != GenericDataLocation) {
               appendOrganizationAndApp(path);
            }

            return path;

         case GenericCacheLocation:
         case CacheLocation:
            path = qttestDir + "/Cache";

            if (type == CacheLocation) {
               appendOrganizationAndApp(path);
            }

            return path;

         case GenericConfigLocation:
         case ConfigLocation:
         case AppConfigLocation:
            path = qttestDir + "/Preferences";

            if (type == AppConfigLocation) {
               appendOrganizationAndApp(path);
            }

            return path;

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
      case AppDataLocation:
      case AppLocalDataLocation:
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

   if (type == GenericDataLocation || type == AppDataLocation || type == AppLocalDataLocation || type == GenericCacheLocation || type == CacheLocation) {
      const QString path = macLocation(type, kOnAppropriateDisk);

      if (! path.isEmpty()) {
         dirs.append(path);
      }
   }

   if (type == AppDataLocation || type == AppLocalDataLocation) {

      CFBundleRef mainBundle = CFBundleGetMainBundle();

      if (mainBundle) {
         CFURLRef bundleUrl = CFBundleCopyBundleURL(mainBundle);
         CFStringRef cfBundlePath = CFURLCopyPath(bundleUrl);
         QString bundlePath = QCFString::toQString(cfBundlePath);
         CFRelease(cfBundlePath);
         CFRelease(bundleUrl);

         CFURLRef resourcesUrl = CFBundleCopyResourcesDirectoryURL(mainBundle);
         CFStringRef cfResourcesPath = CFURLCopyPath(resourcesUrl);

         QString resourcesPath = QCFString::toQString(cfResourcesPath);
         CFRelease(cfResourcesPath);
         CFRelease(resourcesUrl);

         // Handle bundled vs unbundled executables. CFBundleGetMainBundle() returns
         // a valid bundle in both cases. CFBundleCopyResourcesDirectoryURL() returns
         // an absolute path for unbundled executables.

         if (resourcesPath.startsWith('/')) {
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

   if (QStandardPaths::DownloadLocation == type) {
      NSFileManager *fileManager = [NSFileManager defaultManager];
      NSURL *url = [fileManager URLForDirectory:NSDownloadsDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:nil];

      if (!url) {
         return QString();
      }

      return QString::fromNSString([fileManager displayNameAtPath: [url absoluteString]]);
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

   return displayName.toQString();
}
