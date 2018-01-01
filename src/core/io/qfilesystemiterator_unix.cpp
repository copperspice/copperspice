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
#include <qfilesystemiterator_p.h>

#ifndef QT_NO_FILESYSTEMITERATOR

#include <stdlib.h>
#include <errno.h>

QT_BEGIN_NAMESPACE

QFileSystemIterator::QFileSystemIterator(const QFileSystemEntry &entry, QDir::Filters filters,
      const QStringList &nameFilters, QDirIterator::IteratorFlags flags)
   : nativePath(entry.nativeFilePath())
   , dir(0)
   , dirEntry(0)
   , lastError(0)
{
   Q_UNUSED(filters)
   Q_UNUSED(nameFilters)
   Q_UNUSED(flags)

   if ((dir = QT_OPENDIR(nativePath.constData())) == 0) {
      lastError = errno;
   } else {

      if (!nativePath.endsWith('/')) {
         nativePath.append('/');
      }

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
      // ### Race condition; we should use fpathconf and dirfd().
      size_t maxPathName = ::pathconf(nativePath.constData(), _PC_NAME_MAX);
      if (maxPathName == size_t(-1)) {
         maxPathName = FILENAME_MAX;
      }
      maxPathName += sizeof(QT_DIRENT) + 1;

      QT_DIRENT *p = reinterpret_cast<QT_DIRENT *>(::malloc(maxPathName));
      Q_CHECK_PTR(p);

      mt_file.reset(p);
#endif

   }
}

QFileSystemIterator::~QFileSystemIterator()
{
   if (dir) {
      QT_CLOSEDIR(dir);
   }
}

bool QFileSystemIterator::advance(QFileSystemEntry &fileEntry, QFileSystemMetaData &metaData)
{
   if (!dir) {
      return false;
   }

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
   lastError = QT_READDIR_R(dir, mt_file.data(), &dirEntry);
   if (lastError) {
      return false;
   }
#else
   // ### add local lock to prevent breaking reentrancy
   dirEntry = QT_READDIR(dir);
#endif

   if (dirEntry) {
      fileEntry = QFileSystemEntry(nativePath + QByteArray(dirEntry->d_name), QFileSystemEntry::FromNativePath());
      metaData.fillFromDirEnt(*dirEntry);
      return true;
   }

   lastError = errno;
   return false;
}

QT_END_NAMESPACE

#endif // QT_NO_FILESYSTEMITERATOR
