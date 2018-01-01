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

#ifndef QT_NO_DESKTOPSERVICES

#include <qprocess.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qurl.h>
#include <qstringlist.h>
#include <qcore_mac_p.h>
#include <qcoreapplication.h>

// OS X framework
#include <ApplicationServices/ApplicationServices.h>

QT_BEGIN_NAMESPACE

/*
    Translates a QDesktopServices::StandardLocation into the mac equivalent.
*/
OSType translateLocation(QDesktopServices::StandardLocation type)
{
   switch (type) {
      case QDesktopServices::DesktopLocation:
         return kDesktopFolderType;
         break;

      case QDesktopServices::DocumentsLocation:
         return kDocumentsFolderType;
         break;

      case QDesktopServices::FontsLocation:
         // There are at least two different font directories on the mac: /Library/Fonts and ~/Library/Fonts.
         // To select a specific one we have to specify a different first parameter when calling FSFindFolder.
         return kFontsFolderType;
         break;

      case QDesktopServices::ApplicationsLocation:
         return kApplicationsFolderType;
         break;

      case QDesktopServices::MusicLocation:
         return kMusicDocumentsFolderType;
         break;

      case QDesktopServices::MoviesLocation:
         return kMovieDocumentsFolderType;
         break;

      case QDesktopServices::PicturesLocation:
         return kPictureDocumentsFolderType;
         break;

      case QDesktopServices::TempLocation:
         return kTemporaryFolderType;
         break;

      case QDesktopServices::DataLocation:
         return kApplicationSupportFolderType;
         break;

      case QDesktopServices::CacheLocation:
         return kCachedDataFolderType;
         break;

      default:
         return kDesktopFolderType;
         break;
   }
}

static bool lsOpen(const QUrl &url)
{
   if (!url.isValid() || url.scheme().isEmpty()) {
      return false;
   }

   QCFType<CFURLRef> cfUrl = CFURLCreateWithString(0, QCFString(QString::fromLatin1(url.toEncoded())), 0);
   if (cfUrl == 0) {
      return false;
   }

   const OSStatus err = LSOpenCFURLRef(cfUrl, 0);
   return (err == noErr);
}

static bool launchWebBrowser(const QUrl &url)
{
   return lsOpen(url);
}

static bool openDocument(const QUrl &file)
{
   if (!file.isValid()) {
      return false;
   }

   // LSOpen does not work in this case, use QProcess open instead.
   return QProcess::startDetached(QLatin1String("open"), QStringList() << file.toLocalFile());
}

/*
    Constructs a full unicode path from a FSRef.
*/
static QString getFullPath(const FSRef &ref)
{
   QByteArray ba(2048, 0);
   if (FSRefMakePath(&ref, reinterpret_cast<UInt8 *>(ba.data()), ba.size()) == noErr) {
      return QString::fromUtf8(ba).normalized(QString::NormalizationForm_C);
   }
   return QString();
}

QString QDesktopServices::storageLocation(StandardLocation type)
{
   if (type == HomeLocation) {
      return QDir::homePath();
   }

   if (type == TempLocation) {
      return QDir::tempPath();
   }

   short domain = kOnAppropriateDisk;

   if (type == DataLocation || type == CacheLocation) {
      domain = kUserDomain;
   }

   // http://developer.apple.com/documentation/Carbon/Reference/Folder_Manager/Reference/reference.html
   FSRef ref;
   OSErr err = FSFindFolder(domain, translateLocation(type), false, &ref);
   if (err) {
      return QString();
   }

   QString path = getFullPath(ref);

   if (type == DataLocation || type == CacheLocation) {
      if (QCoreApplication::organizationName().isEmpty() == false) {
         path += QLatin1Char('/') + QCoreApplication::organizationName();
      }
      if (QCoreApplication::applicationName().isEmpty() == false) {
         path += QLatin1Char('/') + QCoreApplication::applicationName();
      }
   }

   return path;
}

QString QDesktopServices::displayName(StandardLocation type)
{
   if (QDesktopServices::HomeLocation == type) {
      return QObject::tr("Home");
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

#endif // QT_NO_DESKTOPSERVICES
