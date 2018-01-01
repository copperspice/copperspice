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
#include <qfileinfo.h>
#include <qglobal.h>
#include <qdir.h>
#include <qfileinfo_p.h>

QT_BEGIN_NAMESPACE

QString QFileInfoPrivate::getFileName(QAbstractFileEngine::FileName name) const
{
   if (cache_enabled && !fileNames[(int)name].isNull()) {
      return fileNames[(int)name];
   }

   QString ret;
   if (fileEngine == 0) { 
      // local file; use the QFileSystemEngine directly

      switch (name) {
         case QAbstractFileEngine::CanonicalName:
         case QAbstractFileEngine::CanonicalPathName: {
            QFileSystemEntry entry = QFileSystemEngine::canonicalName(fileEntry, metaData);
            if (cache_enabled) { // be smart and store both
               fileNames[QAbstractFileEngine::CanonicalName] = entry.filePath();
               fileNames[QAbstractFileEngine::CanonicalPathName] = entry.path();
            }
            if (name == QAbstractFileEngine::CanonicalName) {
               ret = entry.filePath();
            } else {
               ret = entry.path();
            }
            break;
         }

         case QAbstractFileEngine::LinkName:
            ret = QFileSystemEngine::getLinkTarget(fileEntry, metaData).filePath();
            break;

         case QAbstractFileEngine::BundleName:
            ret = QFileSystemEngine::bundleName(fileEntry);
            break;

         case QAbstractFileEngine::AbsoluteName:
         case QAbstractFileEngine::AbsolutePathName: {
            QFileSystemEntry entry = QFileSystemEngine::absoluteName(fileEntry);

            if (cache_enabled) { 
               // be smart and store both
               fileNames[QAbstractFileEngine::AbsoluteName] = entry.filePath();
               fileNames[QAbstractFileEngine::AbsolutePathName] = entry.path();
            }
            if (name == QAbstractFileEngine::AbsoluteName) {
               ret = entry.filePath();
            } else {
               ret = entry.path();
            }
            break;
         }

         default:
            break;
      }
   } else {
      ret = fileEngine->fileName(name);
   }

   if (ret.isNull()) {
      ret = QLatin1String("");
   }

   if (cache_enabled) {
      fileNames[(int)name] = ret;
   }

   return ret;
}

QString QFileInfoPrivate::getFileOwner(QAbstractFileEngine::FileOwner own) const
{
   if (cache_enabled && !fileOwners[(int)own].isNull()) {
      return fileOwners[(int)own];
   }

   QString ret;
   if (fileEngine == 0) {
      switch (own) {
         case QAbstractFileEngine::OwnerUser:
            ret = QFileSystemEngine::resolveUserName(fileEntry, metaData);
            break;

         case QAbstractFileEngine::OwnerGroup:
            ret = QFileSystemEngine::resolveGroupName(fileEntry, metaData);
            break;
      }
   } else {
      ret = fileEngine->owner(own);
   }

   if (ret.isNull()) {
      ret = QLatin1String("");
   }

   if (cache_enabled) {
      fileOwners[(int)own] = ret;
   }

   return ret;
}

uint QFileInfoPrivate::getFileFlags(QAbstractFileEngine::FileFlags request) const
{
   Q_ASSERT(fileEngine); 

   // this should never be called when using the native FS
   // We split the testing into tests for for LinkType, BundleType, PermsMask and the rest.

   // Tests for file permissions on Windows can be slow, expecially on network paths and NTFS drives.

   // In order to determine if a file is a symlink or not, we have to lstat().
   // If we're not interested in that information, we might as well avoid one extra syscall

   // Bundle detecton on Mac can be slow, expecially on network paths, so we separate out that as well.

   QAbstractFileEngine::FileFlags req = 0;
   uint cachedFlags = 0;

   if (request & (QAbstractFileEngine::FlagsMask | QAbstractFileEngine::TypesMask)) {
      if (!getCachedFlag(CachedFileFlags)) {
         req |= QAbstractFileEngine::FlagsMask;
         req |= QAbstractFileEngine::TypesMask;
         req &= (~QAbstractFileEngine::LinkType);
         req &= (~QAbstractFileEngine::BundleType);

         cachedFlags |= CachedFileFlags;
      }

      if (request & QAbstractFileEngine::LinkType) {
         if (!getCachedFlag(CachedLinkTypeFlag)) {
            req |= QAbstractFileEngine::LinkType;
            cachedFlags |= CachedLinkTypeFlag;
         }
      }

      if (request & QAbstractFileEngine::BundleType) {
         if (!getCachedFlag(CachedBundleTypeFlag)) {
            req |= QAbstractFileEngine::BundleType;
            cachedFlags |= CachedBundleTypeFlag;
         }
      }
   }

   if (request & QAbstractFileEngine::PermsMask) {
      if (!getCachedFlag(CachedPerms)) {
         req |= QAbstractFileEngine::PermsMask;
         cachedFlags |= CachedPerms;
      }
   }

   if (req) {
      if (cache_enabled) {
         req &= (~QAbstractFileEngine::Refresh);
      } else {
         req |= QAbstractFileEngine::Refresh;
      }

      QAbstractFileEngine::FileFlags flags = fileEngine->fileFlags(req);
      fileFlags |= uint(flags);
      setCachedFlag(cachedFlags);
   }

   return fileFlags & request;
}

QDateTime &QFileInfoPrivate::getFileTime(QAbstractFileEngine::FileTime request) const
{
   Q_ASSERT(fileEngine); 
   // should never be called when using the native FS

   if (!cache_enabled) {
      clearFlags();
   }

   uint cf;
   if (request == QAbstractFileEngine::CreationTime) {
      cf = CachedCTime;
   } else if (request == QAbstractFileEngine::ModificationTime) {
      cf = CachedMTime;
   } else {
      cf = CachedATime;
   }

   if (!getCachedFlag(cf)) {
      fileTimes[request] = fileEngine->fileTime(request);
      setCachedFlag(cf);
   }
   return fileTimes[request];
}

QFileInfo::QFileInfo(QFileInfoPrivate *p) : d_ptr(p)
{
}

QFileInfo::QFileInfo() : d_ptr(new QFileInfoPrivate())
{
}

QFileInfo::QFileInfo(const QString &file) : d_ptr(new QFileInfoPrivate(file))
{
}

QFileInfo::QFileInfo(const QFile &file) : d_ptr(new QFileInfoPrivate(file.fileName()))
{
}

QFileInfo::QFileInfo(const QDir &dir, const QString &file)
   : d_ptr(new QFileInfoPrivate(dir.filePath(file)))
{
}

QFileInfo::QFileInfo(const QFileInfo &fileinfo)
   : d_ptr(fileinfo.d_ptr)
{
}

QFileInfo::~QFileInfo()
{
}

bool QFileInfo::operator==(const QFileInfo &fileinfo) const
{
   Q_D(const QFileInfo);

   // ### Qt5 understand long and short file names on Windows
   // ### (GetFullPathName())

   if (fileinfo.d_ptr == d_ptr) {
      return true;
   }

   if (d->isDefaultConstructed || fileinfo.d_ptr->isDefaultConstructed) {
      return false;
   }

   // Assume files are the same if path is the same
   if (d->fileEntry.filePath() == fileinfo.d_ptr->fileEntry.filePath()) {
      return true;
   }

   Qt::CaseSensitivity sensitive;
   if (d->fileEngine == 0 || fileinfo.d_ptr->fileEngine == 0) {
      if (d->fileEngine != fileinfo.d_ptr->fileEngine) { // one is native, the other is a custom file-engine
         return false;
      }

      sensitive = QFileSystemEngine::isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive;

   } else {
      if (d->fileEngine->caseSensitive() != fileinfo.d_ptr->fileEngine->caseSensitive()) {
         return false;
      }
      sensitive = d->fileEngine->caseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive;
   }

   if (fileinfo.size() != size()) { //if the size isn't the same...
      return false;
   }

   // Fallback to expensive canonical path computation
   return canonicalFilePath().compare(fileinfo.canonicalFilePath(), sensitive) == 0;
}

bool QFileInfo::operator==(const QFileInfo &fileinfo)
{
   return const_cast<const QFileInfo *>(this)->operator==(fileinfo);
}

QFileInfo &QFileInfo::operator=(const QFileInfo &fileinfo)
{
   d_ptr = fileinfo.d_ptr;
   return *this;
}

void QFileInfo::setFile(const QString &file)
{
   bool caching = d_ptr.constData()->cache_enabled;
   *this = QFileInfo(file);
   d_ptr->cache_enabled = caching;
}

void QFileInfo::setFile(const QFile &file)
{
   setFile(file.fileName());
}

void QFileInfo::setFile(const QDir &dir, const QString &file)
{
   setFile(dir.filePath(file));
}

QString QFileInfo::absoluteFilePath() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }
   return d->getFileName(QAbstractFileEngine::AbsoluteName);
}

QString QFileInfo::canonicalFilePath() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }
   return d->getFileName(QAbstractFileEngine::CanonicalName);
}


QString QFileInfo::absolutePath() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return QLatin1String("");

   } else if (d->fileEntry.isEmpty()) {
      qWarning("QFileInfo::absolutePath() Constructed with empty filename");
      return QLatin1String("");

   }
   return d->getFileName(QAbstractFileEngine::AbsolutePathName);
}

QString QFileInfo::canonicalPath() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }
   return d->getFileName(QAbstractFileEngine::CanonicalPathName);
}


QString QFileInfo::path() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }
   return d->fileEntry.path();
}

bool QFileInfo::isRelative() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return true;
   }

   if (d->fileEngine == 0) {
      return d->fileEntry.isRelative();
   }
   return d->fileEngine->isRelativePath();
}

bool QFileInfo::makeAbsolute()
{
   if (d_ptr.constData()->isDefaultConstructed || ! d_ptr.constData()->fileEntry.isRelative()) {
      return false;
   }

   setFile(absoluteFilePath());
   return true;
}

bool QFileInfo::exists() const
{
   Q_D(const QFileInfo);
    
   if (d->isDefaultConstructed) {  
      return false;
   }

   if (d->fileEngine == 0) {
      if (! d->cache_enabled || ! d->metaData.hasFlags(QFileSystemMetaData::ExistsAttribute)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::ExistsAttribute);
      }

      return d->metaData.exists();
   }    

   return d->getFileFlags(QAbstractFileEngine::ExistsFlag);
}

void QFileInfo::refresh()
{
   Q_D(QFileInfo);
   d->clear();
}

QString QFileInfo::filePath() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }
   return d->fileEntry.filePath();
}

QString QFileInfo::fileName() const
{
   Q_D(const QFileInfo);
 
   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }

   return d->fileEntry.fileName();
}

QString QFileInfo::bundleName() const
{
   Q_D(const QFileInfo);
   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }
   return d->getFileName(QAbstractFileEngine::BundleName);
}

QString QFileInfo::baseName() const
{
   Q_D(const QFileInfo);
   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }
   return d->fileEntry.baseName();
}

QString QFileInfo::completeBaseName() const
{
   Q_D(const QFileInfo);
   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }
   return d->fileEntry.completeBaseName();
}

QString QFileInfo::completeSuffix() const
{
   Q_D(const QFileInfo);
   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }
   return d->fileEntry.completeSuffix();
}

QString QFileInfo::suffix() const
{
   Q_D(const QFileInfo);
   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }
   return d->fileEntry.suffix();
}

QDir QFileInfo::dir() const
{
   Q_D(const QFileInfo);

   // ### Qt5 Maybe rename this to parentDirectory(), considering what it actually does?
   return QDir(d->fileEntry.path());
}

QDir QFileInfo::absoluteDir() const
{
   return QDir(absolutePath());
}

bool QFileInfo::isReadable() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return false;
   }

   if (d->fileEngine == 0) {
      if (!d->cache_enabled || !d->metaData.hasFlags(QFileSystemMetaData::UserReadPermission)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::UserReadPermission);
      }

      return (d->metaData.permissions() & QFile::ReadUser) != 0;
   }

   return d->getFileFlags(QAbstractFileEngine::ReadUserPerm);
}

bool QFileInfo::isWritable() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return false;
   }

   if (d->fileEngine == 0) {
      if (!d->cache_enabled || !d->metaData.hasFlags(QFileSystemMetaData::UserWritePermission)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::UserWritePermission);
      }

      return (d->metaData.permissions() & QFile::WriteUser) != 0;
   }

   return d->getFileFlags(QAbstractFileEngine::WriteUserPerm);
}

bool QFileInfo::isExecutable() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return false;
   }

   if (d->fileEngine == 0) {
      if (! d->cache_enabled || ! d->metaData.hasFlags(QFileSystemMetaData::UserExecutePermission)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::UserExecutePermission);
      }

      return (d->metaData.permissions() & QFile::ExeUser) != 0;
   }

   return d->getFileFlags(QAbstractFileEngine::ExeUserPerm);
}

bool QFileInfo::isHidden() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return false;
   }

   if (d->fileEngine == 0) {
      if (! d->cache_enabled || ! d->metaData.hasFlags(QFileSystemMetaData::HiddenAttribute)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::HiddenAttribute);
      }

      return d->metaData.isHidden();
   }

   return d->getFileFlags(QAbstractFileEngine::HiddenFlag);
}

bool QFileInfo::isFile() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return false;
   }

   if (d->fileEngine == 0) {
      if (! d->cache_enabled || ! d->metaData.hasFlags(QFileSystemMetaData::FileType)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::FileType);
      }

      return d->metaData.isFile();
   }

   return d->getFileFlags(QAbstractFileEngine::FileType);
}

bool QFileInfo::isDir() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return false;
   }

   if (d->fileEngine == 0) {
      if (!d->cache_enabled || !d->metaData.hasFlags(QFileSystemMetaData::DirectoryType)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::DirectoryType);
      }
      return d->metaData.isDirectory();
   }

   return d->getFileFlags(QAbstractFileEngine::DirectoryType);
}

bool QFileInfo::isBundle() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return false;
   }

   if (d->fileEngine == 0) {
      if (!d->cache_enabled || !d->metaData.hasFlags(QFileSystemMetaData::BundleType)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::BundleType);
      }
      return d->metaData.isBundle();
   }
   return d->getFileFlags(QAbstractFileEngine::BundleType);
}

bool QFileInfo::isSymLink() const
{
   Q_D(const QFileInfo);
   if (d->isDefaultConstructed) {
      return false;
   }
   if (d->fileEngine == 0) {
      if (!d->cache_enabled || !d->metaData.hasFlags(QFileSystemMetaData::LegacyLinkType)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::LegacyLinkType);
      }
      return d->metaData.isLegacyLink();
   }
   return d->getFileFlags(QAbstractFileEngine::LinkType);
}

bool QFileInfo::isRoot() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return true;
   }

   if (d->fileEngine == 0) {
      if (d->fileEntry.isRoot()) {

#if defined(Q_OS_WIN)
         // the path is a drive root, but the drive may not exist
         // for backward compatibility return true only if the drive exists

         if (! d->cache_enabled || ! d->metaData.hasFlags(QFileSystemMetaData::ExistsAttribute)) {
            QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::ExistsAttribute);
         }    

         return d->metaData.exists();         
#else
         return true;
#endif

      }

      return false;
   }

   return d->getFileFlags(QAbstractFileEngine::RootFlag);
}

QString QFileInfo::readLink() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }
   return d->getFileName(QAbstractFileEngine::LinkName);
}

QString QFileInfo::owner() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }
   return d->getFileOwner(QAbstractFileEngine::OwnerUser);
}

uint QFileInfo::ownerId() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return 0;
   }

   if (d->fileEngine == 0) {
      if (!d->cache_enabled || !d->metaData.hasFlags(QFileSystemMetaData::UserId)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::UserId);
      }
      return d->metaData.userId();
   }
   return d->fileEngine->ownerId(QAbstractFileEngine::OwnerUser);
}

QString QFileInfo::group() const
{
   Q_D(const QFileInfo);
   if (d->isDefaultConstructed) {
      return QLatin1String("");
   }
   return d->getFileOwner(QAbstractFileEngine::OwnerGroup);
}

uint QFileInfo::groupId() const
{
   Q_D(const QFileInfo);   if (d->isDefaultConstructed) {
      return 0;
   }

   if (d->fileEngine == 0) {
      if (!d->cache_enabled || !d->metaData.hasFlags(QFileSystemMetaData::GroupId)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::GroupId);
      }
      return d->metaData.groupId();
   }

   return d->fileEngine->ownerId(QAbstractFileEngine::OwnerGroup);
}

bool QFileInfo::permission(QFile::Permissions permissions) const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return false;
   }

   if (d->fileEngine == 0) {
      // the QFileSystemMetaData::MetaDataFlag and QFile::Permissions overlap, so just static cast.
      QFileSystemMetaData::MetaDataFlag permissionFlags = static_cast<QFileSystemMetaData::MetaDataFlag>((int)permissions);
      if (!d->cache_enabled || !d->metaData.hasFlags(permissionFlags)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, permissionFlags);
      }
      return (d->metaData.permissions() & permissions) == permissions;
   }
   return d->getFileFlags(QAbstractFileEngine::FileFlags((int)permissions)) == (uint)permissions;
}

QFile::Permissions QFileInfo::permissions() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return 0;
   }

   if (d->fileEngine == 0) {
      if (!d->cache_enabled || !d->metaData.hasFlags(QFileSystemMetaData::Permissions)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::Permissions);
      }
      return d->metaData.permissions();
   }

   return QFile::Permissions(d->getFileFlags(QAbstractFileEngine::PermsMask) & QAbstractFileEngine::PermsMask);
}

qint64 QFileInfo::size() const
{
   Q_D(const QFileInfo);
  
   if (d->isDefaultConstructed) {
      return 0;
   }
  
   if (d->fileEngine == 0) {
      if (!d->cache_enabled || !d->metaData.hasFlags(QFileSystemMetaData::SizeAttribute)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::SizeAttribute);
      }
      return d->metaData.size();
   }

   if (!d->getCachedFlag(QFileInfoPrivate::CachedSize)) {
      d->setCachedFlag(QFileInfoPrivate::CachedSize);
      d->fileSize = d->fileEngine->size();
   }
   return d->fileSize;
}

QDateTime QFileInfo::created() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return QDateTime();
   }

   if (d->fileEngine == 0) {
      if (!d->cache_enabled || !d->metaData.hasFlags(QFileSystemMetaData::CreationTime)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::CreationTime);
      }

      return d->metaData.creationTime();
   }

   return d->getFileTime(QAbstractFileEngine::CreationTime);
}

QDateTime QFileInfo::lastModified() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return QDateTime();
   }

   if (d->fileEngine == 0) {
      if (!d->cache_enabled || !d->metaData.hasFlags(QFileSystemMetaData::ModificationTime)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::ModificationTime);
      }
      return d->metaData.modificationTime();
   }
   return d->getFileTime(QAbstractFileEngine::ModificationTime);
}

QDateTime QFileInfo::lastRead() const
{
   Q_D(const QFileInfo);

   if (d->isDefaultConstructed) {
      return QDateTime();
   }

   if (d->fileEngine == 0) {
      if (!d->cache_enabled || !d->metaData.hasFlags(QFileSystemMetaData::AccessTime)) {
         QFileSystemEngine::fillMetaData(d->fileEntry, d->metaData, QFileSystemMetaData::AccessTime);
      }
      return d->metaData.accessTime();
   }

   return d->getFileTime(QAbstractFileEngine::AccessTime);
}

void QFileInfo::detach()
{
   d_ptr.detach();
}

bool QFileInfo::caching() const
{
   Q_D(const QFileInfo);

   return d->cache_enabled;
}

void QFileInfo::setCaching(bool enable)
{
   Q_D(QFileInfo);
   d->cache_enabled = enable;
}

QT_END_NAMESPACE
