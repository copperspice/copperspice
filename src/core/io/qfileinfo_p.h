/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QFILEINFO_P_H
#define QFILEINFO_P_H

#include <qfileinfo.h>
#include <qabstractfileengine.h>
#include <qdatetime.h>
#include <qatomic.h>
#include <qshareddata.h>
#include <qfilesystemengine_p.h>
#include <qfilesystementry_p.h>
#include <qfilesystemmetadata_p.h>

QT_BEGIN_NAMESPACE

class QFileInfoPrivate : public QSharedData
{
 public:
   enum { CachedFileFlags = 0x01, CachedLinkTypeFlag = 0x02, CachedBundleTypeFlag = 0x04,
          CachedMTime = 0x10, CachedCTime = 0x20, CachedATime = 0x40,
          CachedSize = 0x08, CachedPerms = 0x80
        };

   inline QFileInfoPrivate()
      : QSharedData(), fileEngine(0),
        cachedFlags(0),
        isDefaultConstructed(true),
        cache_enabled(true), fileFlags(0), fileSize(0) {
   }

   inline QFileInfoPrivate(const QFileInfoPrivate &copy)
      : QSharedData(copy),
        fileEntry(copy.fileEntry),
        metaData(copy.metaData),
        fileEngine(QFileSystemEngine::resolveEntryAndCreateLegacyEngine(fileEntry, metaData)),
        cachedFlags(0),
#ifndef QT_NO_FSFILEENGINE
        isDefaultConstructed(false),
#else
        isDefaultConstructed(!fileEngine),
#endif
        cache_enabled(copy.cache_enabled), fileFlags(0), fileSize(0) {
   }
   inline QFileInfoPrivate(const QString &file)
      : fileEntry(QDir::fromNativeSeparators(file)),
        fileEngine(QFileSystemEngine::resolveEntryAndCreateLegacyEngine(fileEntry, metaData)),
        cachedFlags(0),
#ifndef QT_NO_FSFILEENGINE
        isDefaultConstructed(false),
#else
        isDefaultConstructed(!fileEngine),
#endif
        cache_enabled(true), fileFlags(0), fileSize(0) {
   }

   inline QFileInfoPrivate(const QFileSystemEntry &file, const QFileSystemMetaData &data)
      : QSharedData(),
        fileEntry(file),
        metaData(data),
        fileEngine(QFileSystemEngine::resolveEntryAndCreateLegacyEngine(fileEntry, metaData)),
        cachedFlags(0),
        isDefaultConstructed(false),
        cache_enabled(true), fileFlags(0), fileSize(0) {
     
      // If the file engine is not null, this maybe a "mount point" for a custom file engine
      // in which case we ca not trust the metadata
      if (fileEngine) {
         metaData = QFileSystemMetaData();
      }
   }

   inline void clearFlags() const {
      fileFlags = 0;
      cachedFlags = 0;
      if (fileEngine) {
         (void)fileEngine->fileFlags(QAbstractFileEngine::Refresh);
      }
   }

   inline void clear() {
      metaData.clear();
      clearFlags();
      for (int i = QAbstractFileEngine::NFileNames - 1 ; i >= 0 ; --i) {
         fileNames[i].clear();
      }
      fileOwners[1].clear();
      fileOwners[0].clear();
   }

   uint getFileFlags(QAbstractFileEngine::FileFlags) const;
   QDateTime &getFileTime(QAbstractFileEngine::FileTime) const;
   QString getFileName(QAbstractFileEngine::FileName) const;
   QString getFileOwner(QAbstractFileEngine::FileOwner own) const;

   QFileSystemEntry fileEntry;
   mutable QFileSystemMetaData metaData;

   QScopedPointer<QAbstractFileEngine> const fileEngine;

   mutable QString fileNames[QAbstractFileEngine::NFileNames];
   mutable QString fileOwners[2];

   mutable uint cachedFlags : 30;
   bool const isDefaultConstructed : 1; // QFileInfo is a default constructed instance
   bool cache_enabled : 1;
   mutable uint fileFlags;
   mutable qint64 fileSize;
   mutable QDateTime fileTimes[3];

   inline bool getCachedFlag(uint c) const {
      return cache_enabled ? (cachedFlags & c) : 0;
   }

   inline void setCachedFlag(uint c) const {
      if (cache_enabled) {
         cachedFlags |= c;
      }
   }

};

QT_END_NAMESPACE

#endif // QFILEINFO_P_H
