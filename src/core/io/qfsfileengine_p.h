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

#ifndef QFSFILEENGINE_P_H
#define QFSFILEENGINE_P_H

#include <qfsfileengine.h>

#include <qhash.h>
#include <qplatformdefs.h>

#include <qabstractfileengine_p.h>
#include <qfilesystementry_p.h>
#include <qfilesystemmetadata_p.h>

#ifndef QT_NO_FSFILEENGINE

class QFSFileEnginePrivate : public QAbstractFileEnginePrivate
{
   Q_DECLARE_PUBLIC(QFSFileEngine)

 public:

#ifdef Q_OS_WIN
   static QString longFileName(const QString &path);
#endif

   QFileSystemEntry fileEntry;
   QIODevice::OpenMode openMode;

   bool renameOverwrite(const QString &newName);

   bool nativeOpen(QIODevice::OpenMode openMode);
   bool openFh(QIODevice::OpenMode flags, FILE *fh);
   bool openFd(QIODevice::OpenMode flags, int fd);
   bool nativeClose();
   bool closeFdFh();
   bool syncToDisk();
   bool nativeFlush();
   bool nativeSyncToDisk();
   bool flushFh();
   qint64 nativeSize() const;

#ifndef Q_OS_WIN
   qint64 sizeFdFh() const;
#endif

   qint64 nativePos() const;
   qint64 posFdFh() const;
   bool nativeSeek(qint64);
   bool seekFdFh(qint64);
   qint64 nativeRead(char *data, qint64 maxlen);
   qint64 readFdFh(char *data, qint64 maxlen);
   qint64 nativeReadLine(char *data, qint64 maxlen);
   qint64 readLineFdFh(char *data, qint64 maxlen);
   qint64 nativeWrite(const char *data, qint64 len);
   qint64 writeFdFh(const char *data, qint64 len);
   int nativeHandle() const;
   bool nativeIsSequential() const;

#ifndef Q_OS_WIN
   bool isSequentialFdFh() const;
#endif

   uchar *map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags);
   bool unmap(uchar *ptr);

   mutable QFileSystemMetaData metaData;

   FILE *fh;

#ifdef Q_OS_WIN
   HANDLE fileHandle;
   HANDLE mapHandle;
   QHash<uchar *, DWORD> maps;

   mutable int cachedFd;
   mutable DWORD fileAttrib;

#else
   QHash<uchar *, QPair<int, size_t>> maps;

#endif

   int fd;

   enum LastIOCommand {
      IOFlushCommand,
      IOReadCommand,
      IOWriteCommand
   };

   LastIOCommand lastIOCommand;

   bool lastFlushFailed;
   bool closeFileHandle;

   mutable uint is_sequential : 2;
   mutable uint could_stat : 1;
   mutable uint tried_stat : 1;
   mutable uint need_lstat : 1;
   mutable uint is_link : 1;

#if defined(Q_OS_WIN)
   bool doStat(QFileSystemMetaData::MetaDataFlags flags) const;
#else
   bool doStat(QFileSystemMetaData::MetaDataFlags flags = QFileSystemMetaData::PosixStatFlags) const;
#endif

   bool isSymlink() const;

#if defined(Q_OS_WIN)
   int sysOpen(const QString &, int flags);
#endif

 protected:
   QFSFileEnginePrivate();

   void init();

   QAbstractFileEngine::FileFlags getPermissions(QAbstractFileEngine::FileFlags type) const;
};

#endif // QT_NO_FSFILEENGINE

#endif
