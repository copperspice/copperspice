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

#include <qplatformdefs.h>

#include <qfilesystemiterator_p.h>

#ifndef QT_NO_FILESYSTEMITERATOR

#include <stdlib.h>
#include <errno.h>

QFileSystemIterator::QFileSystemIterator(const QFileSystemEntry &entry, QDir::Filters filters,
      const QStringList &nameFilters, QDirIterator::IteratorFlags flags)
   : nativePath(entry.nativeFilePath()), dir(nullptr), dirEntry(nullptr), lastError(0)
{
   (void) filters;
   (void) nameFilters;
   (void) flags;

   if ((dir = ::opendir(nativePath.constData())) == nullptr) {
      lastError = errno;
   } else {

      if (! nativePath.endsWith('/')) {
         nativePath.append('/');
      }
   }
}

QFileSystemIterator::~QFileSystemIterator()
{
   if (dir) {
      ::closedir(dir);
   }
}

bool QFileSystemIterator::advance(QFileSystemEntry &fileEntry, QFileSystemMetaData &metaData)
{
   if (! dir) {
      return false;
   }

   dirEntry = QT_READDIR(dir);

   if (dirEntry) {
      fileEntry = QFileSystemEntry(nativePath + QByteArray(dirEntry->d_name), QFileSystemEntry::FromNativePath());
      metaData.fillFromDirEnt(*dirEntry);
      return true;
   }

   lastError = errno;
   return false;
}

#endif // QT_NO_FILESYSTEMITERATOR
