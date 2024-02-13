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

#include <qfilesystemengine_p.h>

#include <qdir.h>
#include <qset.h>

#include <qabstractfileengine_p.h>
#include <qresource_p.h>

QString QFileSystemEngine::slowCanonicalized(const QString &path)
{
   if (path.isEmpty()) {
      return path;
   }

   QFileInfo fi;

   const QChar slash('/');

   QString tmpPath = path;
   int separatorPos = 0;

   QSet<QString> nonSymlinks;
   QSet<QString> known;

   known.insert(path);

   do {

#ifdef Q_OS_WIN

      if (separatorPos == 0) {
         if (tmpPath.size() >= 2 && tmpPath.at(0) == slash && tmpPath.at(1) == slash) {
            // UNC, skip past the first two elements
            separatorPos = tmpPath.indexOf(slash, 2);

         } else if (tmpPath.size() >= 3 && tmpPath.at(1) == ':' && tmpPath.at(2) == slash) {
            // volume root, skip since it can not be a symlink
            separatorPos = 2;
         }
      }

      if (separatorPos != -1)
#endif
         separatorPos = tmpPath.indexOf(slash, separatorPos + 1);

      QString prefix = separatorPos == -1 ? tmpPath : tmpPath.left(separatorPos);

      if (! nonSymlinks.contains(prefix)) {
         fi.setFile(prefix);

         if (fi.isSymLink()) {
            QString target = fi.symLinkTarget();

            if (QFileInfo(target).isRelative()) {
               target = fi.absolutePath() + slash + target;
            }

            if (separatorPos != -1) {
               if (fi.isDir() && !target.endsWith(slash)) {
                  target.append(slash);
               }

               target.append(tmpPath.mid(separatorPos));
            }

            tmpPath = QDir::cleanPath(target);
            separatorPos = 0;

            if (known.contains(tmpPath)) {
               return QString();
            }

            known.insert(tmpPath);
         } else {
            nonSymlinks.insert(prefix);
         }
      }

   } while (separatorPos != -1);

   return QDir::cleanPath(tmpPath);
}

static inline bool _q_checkEntry(QFileSystemEntry &entry, QFileSystemMetaData &data, bool resolvingEntry)
{
   if (resolvingEntry) {
      if (! QFileSystemEngine::fillMetaData(entry, data, QFileSystemMetaData::ExistsAttribute) || ! data.exists()) {
         data.clear();
         return false;
      }
   }

   return true;
}

static inline bool _q_checkEntry(QAbstractFileEngine *&engine, bool resolvingEntry)
{
   if (resolvingEntry) {
      if (!(engine->fileFlags(QAbstractFileEngine::FlagsMask) & QAbstractFileEngine::ExistsFlag)) {
         delete engine;
         engine = nullptr;
         return false;
      }
   }

   return true;
}

static bool _q_resolveEntryAndCreateLegacyEngine_recursive(QFileSystemEntry &entry, QFileSystemMetaData &data,
      QAbstractFileEngine *&engine, bool resolvingEntry = false)
{
   QString const &filePath = entry.filePath();

   if ((engine = qt_custom_file_engine_handler_create(filePath))) {
      return _q_checkEntry(engine, resolvingEntry);
   }

   for (int prefixSeparator = 0; prefixSeparator < filePath.size(); ++prefixSeparator) {
      QChar const ch = filePath[prefixSeparator];

      if (ch == '/') {
         break;
      }

      if (ch == ':') {
         if (prefixSeparator == 0) {
            engine = new QResourceFileEngine(filePath);
            return _q_checkEntry(engine, resolvingEntry);
         }

         if (prefixSeparator == 1) {
            break;
         }

         const QStringList &paths = QDir::searchPaths(filePath.left(prefixSeparator));

         for (int i = 0; i < paths.count(); i++) {
            entry = QFileSystemEntry(QDir::cleanPath(paths.at(i) + '/' + filePath.mid(prefixSeparator + 1)));

            // recurse
            if (_q_resolveEntryAndCreateLegacyEngine_recursive(entry, data, engine, true)) {
               return true;
            }
         }

         // entry may have been clobbered at this point.
         return false;
      }

      //  There's no need to fully validate the prefix here. Consulting the
      //  unicode tables could be expensive and validation is already
      //  performed in QDir::setSearchPaths.
      //
      //  if (!ch.isLetterOrNumber())
      //      break;
   }

   return _q_checkEntry(entry, data, resolvingEntry);
}

QAbstractFileEngine *QFileSystemEngine::resolveEntryAndCreateLegacyEngine(
      QFileSystemEntry &entry, QFileSystemMetaData &data)
{
   QFileSystemEntry copy       = entry;
   QAbstractFileEngine *engine = nullptr;

   if (_q_resolveEntryAndCreateLegacyEngine_recursive(copy, data, engine)) {
      // Reset entry to resolved copy
      entry = copy;
   } else {
      data.clear();
   }

   return engine;
}

#ifdef Q_OS_UNIX

bool QFileSystemEngine::fillMetaData(int fd, QFileSystemMetaData &data)
{
   data.entryFlags &= ~QFileSystemMetaData::PosixStatFlags;
   data.knownFlagsMask |= QFileSystemMetaData::PosixStatFlags;

   QT_STATBUF statBuffer;

   if (QT_FSTAT(fd, &statBuffer) == 0) {
      data.fillFromStatBuf(statBuffer);
      return true;
   }

   return false;
}

void QFileSystemMetaData::fillFromStatBuf(const QT_STATBUF &statBuffer)
{
   // Permissions
   if (statBuffer.st_mode & S_IRUSR) {
      entryFlags |= QFileSystemMetaData::OwnerReadPermission;
   }

   if (statBuffer.st_mode & S_IWUSR) {
      entryFlags |= QFileSystemMetaData::OwnerWritePermission;
   }

   if (statBuffer.st_mode & S_IXUSR) {
      entryFlags |= QFileSystemMetaData::OwnerExecutePermission;
   }

   if (statBuffer.st_mode & S_IRGRP) {
      entryFlags |= QFileSystemMetaData::GroupReadPermission;
   }

   if (statBuffer.st_mode & S_IWGRP) {
      entryFlags |= QFileSystemMetaData::GroupWritePermission;
   }

   if (statBuffer.st_mode & S_IXGRP) {
      entryFlags |= QFileSystemMetaData::GroupExecutePermission;
   }

   if (statBuffer.st_mode & S_IROTH) {
      entryFlags |= QFileSystemMetaData::OtherReadPermission;
   }

   if (statBuffer.st_mode & S_IWOTH) {
      entryFlags |= QFileSystemMetaData::OtherWritePermission;
   }

   if (statBuffer.st_mode & S_IXOTH) {
      entryFlags |= QFileSystemMetaData::OtherExecutePermission;
   }

   // Type
   if ((statBuffer.st_mode & S_IFMT) == S_IFREG) {
      entryFlags |= QFileSystemMetaData::FileType;
   } else if ((statBuffer.st_mode & S_IFMT) == S_IFDIR) {
      entryFlags |= QFileSystemMetaData::DirectoryType;
   } else {
      entryFlags |= QFileSystemMetaData::SequentialType;
   }

   // Attributes
   entryFlags |= QFileSystemMetaData::ExistsAttribute;
   size_ = statBuffer.st_size;

#if defined(Q_OS_DARWIN)

   if (statBuffer.st_flags & UF_HIDDEN) {
      entryFlags     |= QFileSystemMetaData::HiddenAttribute;
      knownFlagsMask |= QFileSystemMetaData::HiddenAttribute;
   }

#endif

   // Times
   creationTime_ = statBuffer.st_ctime ? statBuffer.st_ctime : statBuffer.st_mtime;
   modificationTime_ = statBuffer.st_mtime;
   accessTime_ = statBuffer.st_atime;
   userId_ = statBuffer.st_uid;
   groupId_ = statBuffer.st_gid;
}

void QFileSystemMetaData::fillFromDirEnt(const QT_DIRENT &entry)
{

#if defined(_DIRENT_HAVE_D_TYPE) || defined(Q_OS_BSD4)
   // BSD4 includes Mac OS X

   // ### This will clear all entry flags and knownFlagsMask
   switch (entry.d_type) {
      case DT_DIR:
         knownFlagsMask = QFileSystemMetaData::LinkType
               | QFileSystemMetaData::FileType
               | QFileSystemMetaData::DirectoryType
               | QFileSystemMetaData::SequentialType
               | QFileSystemMetaData::ExistsAttribute;

         entryFlags = QFileSystemMetaData::DirectoryType
               | QFileSystemMetaData::ExistsAttribute;

         break;

      case DT_BLK:
      case DT_CHR:
      case DT_FIFO:
      case DT_SOCK:
         // ### System attribute
         knownFlagsMask = QFileSystemMetaData::LinkType
               | QFileSystemMetaData::FileType
               | QFileSystemMetaData::DirectoryType
               | QFileSystemMetaData::BundleType
               | QFileSystemMetaData::AliasType
               | QFileSystemMetaData::SequentialType
               | QFileSystemMetaData::ExistsAttribute;

         entryFlags = QFileSystemMetaData::SequentialType
               | QFileSystemMetaData::ExistsAttribute;

         break;

      case DT_LNK:
         knownFlagsMask = QFileSystemMetaData::LinkType;
         entryFlags = QFileSystemMetaData::LinkType;
         break;

      case DT_REG:
         knownFlagsMask = QFileSystemMetaData::LinkType
               | QFileSystemMetaData::FileType
               | QFileSystemMetaData::DirectoryType
               | QFileSystemMetaData::BundleType
               | QFileSystemMetaData::SequentialType
               | QFileSystemMetaData::ExistsAttribute;

         entryFlags = QFileSystemMetaData::FileType
               | QFileSystemMetaData::ExistsAttribute;

         break;

      case DT_UNKNOWN:
      default:
         clear();
   }

#else
   (void) entry;

#endif

}

#endif

// static method
QString QFileSystemEngine::resolveUserName(const QFileSystemEntry &entry, QFileSystemMetaData &metaData)
{

#if defined(Q_OS_WIN)
   (void) metaData;
   return QFileSystemEngine::owner(entry, QAbstractFileEngine::OwnerUser);

#else

   if (! metaData.hasFlags(QFileSystemMetaData::UserId)) {
      QFileSystemEngine::fillMetaData(entry, metaData, QFileSystemMetaData::UserId);
   }

   return resolveUserName(metaData.userId());
#endif
}

// static method
QString QFileSystemEngine::resolveGroupName(const QFileSystemEntry &entry, QFileSystemMetaData &metaData)
{

#if defined(Q_OS_WIN)
   (void) metaData;
   return QFileSystemEngine::owner(entry, QAbstractFileEngine::OwnerGroup);

#else

   if (!metaData.hasFlags(QFileSystemMetaData::GroupId)) {
      QFileSystemEngine::fillMetaData(entry, metaData, QFileSystemMetaData::GroupId);
   }

   return resolveGroupName(metaData.groupId());

#endif
}
