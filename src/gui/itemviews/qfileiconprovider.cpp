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

#include <qfileiconprovider.h>
#include <qfileiconprovider_p.h>

#include <qdir.h>
#include <qglobal.h>
#include <qpixmapcache.h>
#include <qplatform_integration.h>
#include <qplatform_services.h>
#include <qplatform_theme.h>

#include <qguiapplication_p.h>
#include <qicon_p.h>

#if defined(Q_OS_WIN)
#  include <qt_windows.h>

#  include <commctrl.h>
#  include <objbase.h>
#endif

#if defined(Q_OS_UNIX) && ! defined(QT_NO_STYLE_GTK)
#  include <qgtkstyle_p.h>
#endif

static bool isCacheable(const QFileInfo &fi);

class QFileIconEngine : public QPixmapIconEngine
{
 public:
   QFileIconEngine(const QFileInfo &info, QFileIconProvider::Options opts)
      : QPixmapIconEngine(), m_fileInfo(info), m_fipOpts(opts)
   { }

   QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
      (void) mode;
      (void) state;

      QPixmap pixmap;

      if (! size.isValid()) {
         return pixmap;
      }

      const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme();
      if (! theme) {
         return pixmap;
      }

      const QString &keyBase = "qt_." + m_fileInfo.suffix().toUpper();

      bool cacheable = isCacheable(m_fileInfo);
      if (cacheable) {
         QPixmapCache::find(keyBase + QString::number(size.width()), pixmap);
         if (! pixmap.isNull()) {
            return pixmap;
         }
      }

      QPlatformTheme::IconOptions iconOptions;
      if (m_fipOpts & QFileIconProvider::DontUseCustomDirectoryIcons) {
         iconOptions |= QPlatformTheme::DontUseCustomDirectoryIcons;
      }

      pixmap = theme->fileIconPixmap(m_fileInfo, size, iconOptions);
      if (! pixmap.isNull()) {
         if (cacheable) {
            QPixmapCache::insert(keyBase + QString::number(size.width()), pixmap);
         }
      }

      return pixmap;
   }

   QList<QSize> availableSizes(QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off) const override {
      (void) mode;
      (void) state;

      static QList<QSize> sizes;
      static QPlatformTheme *theme = nullptr;

      if (! theme) {
         theme = QGuiApplicationPrivate::platformTheme();
         if (!theme) {
            return sizes;
         }

         QList<int> themeSizes = theme->themeHint(QPlatformTheme::IconPixmapSizes).value<QList<int>>();
         if (themeSizes.isEmpty()) {
            return sizes;
         }

         for (int size : themeSizes) {
            sizes << QSize(size, size);
         }
      }

      return sizes;
   }

   QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
      const QList<QSize> &sizes = availableSizes(mode, state);
      const int numberSizes     = sizes.length();

      if (numberSizes == 0) {
         return QSize();
      }

      // Find the smallest available size whose area is still larger than the input
      // size. Otherwise, use the largest area available size. Do not assume the
      // platform theme sizes are sorted, hence the extra logic.

      const int sizeArea = size.width() * size.height();
      QSize actualSize = sizes.first();
      int actualArea = actualSize.width() * actualSize.height();

      for (int i = 1; i < numberSizes; ++i) {
         const QSize &s = sizes.at(i);
         const int a = s.width() * s.height();

         if ((sizeArea <= a && a < actualArea) || (actualArea < sizeArea && actualArea < a)) {
            actualSize = s;
            actualArea = a;
         }
      }

      if (! actualSize.isNull() && (actualSize.width() > size.width() || actualSize.height() > size.height())) {
         actualSize.scale(size, Qt::KeepAspectRatio);
      }

      return actualSize;
   }

 private:
   QFileInfo m_fileInfo;
   QFileIconProvider::Options m_fipOpts;
};

QFileIconProviderPrivate::QFileIconProviderPrivate(QFileIconProvider *q)
   : q_ptr(q), homePath(QDir::home().absolutePath())
{
}

QIcon QFileIconProviderPrivate::getIcon(QStyle::StandardPixmap name) const
{
   switch (name) {
      case QStyle::SP_FileIcon:
         if (file.isNull()) {
            file = QApplication::style()->standardIcon(name);
         }

         return file;

      case QStyle::SP_FileLinkIcon:
         if (fileLink.isNull()) {
            fileLink = QApplication::style()->standardIcon(name);
         }

         return fileLink;

      case QStyle::SP_DirIcon:
         if (directory.isNull()) {
            directory = QApplication::style()->standardIcon(name);
         }

         return directory;

      case QStyle::SP_DirLinkIcon:
         if (directoryLink.isNull()) {
            directoryLink = QApplication::style()->standardIcon(name);
         }

         return directoryLink;

      case QStyle::SP_DriveHDIcon:
         if (harddisk.isNull()) {
            harddisk = QApplication::style()->standardIcon(name);
         }

         return harddisk;

      case QStyle::SP_DriveFDIcon:
         if (floppy.isNull()) {
            floppy = QApplication::style()->standardIcon(name);
         }

         return floppy;

      case QStyle::SP_DriveCDIcon:
         if (cdrom.isNull()) {
            cdrom = QApplication::style()->standardIcon(name);
         }

         return cdrom;

      case QStyle::SP_DriveNetIcon:
         if (network.isNull()) {
            network = QApplication::style()->standardIcon(name);
         }

         return network;

      case QStyle::SP_ComputerIcon:
         if (computer.isNull()) {
            computer = QApplication::style()->standardIcon(name);
         }

         return computer;

      case QStyle::SP_DesktopIcon:
         if (desktop.isNull()) {
            desktop = QApplication::style()->standardIcon(name);
         }

         return desktop;

      case QStyle::SP_TrashIcon:
         if (trashcan.isNull()) {
            trashcan = QApplication::style()->standardIcon(name);
         }

         return trashcan;

      case QStyle::SP_DirHomeIcon:
         if (home.isNull()) {
            home = QApplication::style()->standardIcon(name);
         }

         return home;

      default:
         return QIcon();
   }

   return QIcon();
}

QFileIconProvider::QFileIconProvider()
   : d_ptr(new QFileIconProviderPrivate(this))
{
}

QFileIconProvider::~QFileIconProvider()
{
}
void QFileIconProvider::setOptions(QFileIconProvider::Options options)
{
   Q_D(QFileIconProvider);
   d->options = options;
}

QFileIconProvider::Options QFileIconProvider::options() const
{
   Q_D(const QFileIconProvider);
   return d->options;
}

QIcon QFileIconProvider::icon(IconType type) const
{
   Q_D(const QFileIconProvider);

   switch (type) {
      case Computer:
         return d->getIcon(QStyle::SP_ComputerIcon);
      case Desktop:
         return d->getIcon(QStyle::SP_DesktopIcon);
      case Trashcan:
         return d->getIcon(QStyle::SP_TrashIcon);
      case Network:
         return d->getIcon(QStyle::SP_DriveNetIcon);
      case Drive:
         return d->getIcon(QStyle::SP_DriveHDIcon);
      case Folder:
         return d->getIcon(QStyle::SP_DirIcon);
      case File:
         return d->getIcon(QStyle::SP_FileIcon);
      default:
         break;
   };

   return QIcon();
}

static bool isCacheable(const QFileInfo &fi)
{
   if (!fi.isFile()) {
      return false;
   }

#ifdef Q_OS_WIN
   // On windows it's faster to just look at the file extensions. QTBUG-13182
   const QString fileExtension = fi.suffix();
   return fileExtension.compare("exe", Qt::CaseInsensitive) &&
         fileExtension.compare("lnk", Qt::CaseInsensitive)  &&
         fileExtension.compare("ico", Qt::CaseInsensitive);

#else
   return !fi.isExecutable() && !fi.isSymLink();

#endif
}

QIcon QFileIconProviderPrivate::getIcon(const QFileInfo &fi) const
{
   const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme();
   if (!theme) {
      return QIcon();
   }

   QList<int> sizes = theme->themeHint(QPlatformTheme::IconPixmapSizes).value<QList<int>>();
   if (sizes.isEmpty()) {
      return QIcon();
   }

   return QIcon(new QFileIconEngine(fi, options));
}

QIcon QFileIconProvider::icon(const QFileInfo &info) const
{
   Q_D(const QFileIconProvider);


#if defined(Q_OS_UNIX) && !defined(QT_NO_STYLE_GTK)
   const QByteArray desktopEnvironment = QGuiApplicationPrivate::platformIntegration()->services()->desktopEnvironment();

   if (desktopEnvironment != "KDE") {
      QIcon gtkIcon = QGtkStylePrivate::getFilesystemIcon(info);

      if (! gtkIcon.isNull()) {
         return gtkIcon;
      }
   }
#endif

   QIcon retIcon = d->getIcon(info);
   if (!retIcon.isNull()) {
      return retIcon;
   }


   if (info.isRoot())

#if defined (Q_OS_WIN)
   {
      std::wstring tmp(info.absoluteFilePath().toStdWString());
      UINT type = GetDriveType(&tmp[0]);

      switch (type) {
         case DRIVE_REMOVABLE:
            return d->getIcon(QStyle::SP_DriveFDIcon);

         case DRIVE_FIXED:
            return d->getIcon(QStyle::SP_DriveHDIcon);

         case DRIVE_REMOTE:
            return d->getIcon(QStyle::SP_DriveNetIcon);

         case DRIVE_CDROM:
            return d->getIcon(QStyle::SP_DriveCDIcon);

         case DRIVE_RAMDISK:
         case DRIVE_UNKNOWN:
         case DRIVE_NO_ROOT_DIR:
         default:
            return d->getIcon(QStyle::SP_DriveHDIcon);
      }
   }

#else
      return d->getIcon(QStyle::SP_DriveHDIcon);
#endif

   if (info.isFile()) {
      if (info.isSymLink()) {
         return d->getIcon(QStyle::SP_FileLinkIcon);
      } else {
         return d->getIcon(QStyle::SP_FileIcon);
      }
   }

   if (info.isDir()) {
      if (info.isSymLink()) {
         return d->getIcon(QStyle::SP_DirLinkIcon);

      } else {
         if (info.absoluteFilePath() == d->homePath) {
            return d->getIcon(QStyle::SP_DirHomeIcon);
         } else {
            return d->getIcon(QStyle::SP_DirIcon);
         }
      }
   }

   return QIcon();
}

QString QFileIconProvider::type(const QFileInfo &info) const
{
   if (info.isRoot()) {
      return QApplication::translate("QFileDialog", "Drive");
   }

   if (info.isFile()) {
      if (!info.suffix().isEmpty()) {
         //: %1 is a file name suffix, for example txt
         return QApplication::translate("QFileDialog", "%1 File").formatArg(info.suffix());
      }
      return QApplication::translate("QFileDialog", "File");
   }

   if (info.isDir())
#ifdef Q_OS_WIN
      return QApplication::translate("QFileDialog", "File Folder", "Match Windows Explorer");
#else
      return QApplication::translate("QFileDialog", "Folder", "All other platforms");
#endif
   // Windows   - "File Folder"
   // OS X      - "Folder"
   // Konqueror - "Folder"
   // Nautilus  - "folder"

   if (info.isSymLink())

#ifdef Q_OS_DARWIN
      return QApplication::translate("QFileDialog", "Alias", "OS X Finder");
#else
      return QApplication::translate("QFileDialog", "Shortcut", "All other platforms");
#endif

   // OS X      - "Alias"
   // Windows   - "Shortcut"
   // Konqueror - "Folder" or "TXT File" i.e. what it is pointing to
   // Nautilus  - "link to folder" or "link to object file", same as Konqueror

   return QApplication::translate("QFileDialog", "Unknown");
}



