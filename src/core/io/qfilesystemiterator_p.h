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

#ifndef QFILESYSTEMITERATOR_P_H
#define QFILESYSTEMITERATOR_P_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_FILESYSTEMITERATOR

#include <qdir.h>
#include <qdiriterator.h>
#include <qstringlist.h>
#include <qfilesystementry_p.h>
#include <qfilesystemmetadata_p.h>

// Platform-specific headers
#if defined(Q_OS_WIN)
#else
#include <QtCore/qscopedpointer.h>
#endif

QT_BEGIN_NAMESPACE

class QFileSystemIterator
{
 public:
   QFileSystemIterator(const QFileSystemEntry &entry, QDir::Filters filters, const QStringList &nameFilters, 
         QDirIterator::IteratorFlags flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);

   ~QFileSystemIterator();

   bool advance(QFileSystemEntry &fileEntry, QFileSystemMetaData &metaData);

 private:
   QFileSystemEntry::NativePath nativePath;

#if defined(Q_OS_WIN)
   QFileSystemEntry::NativePath dirPath;
   HANDLE findFileHandle;
   QStringList uncShares;
   bool uncFallback;
   int uncShareIndex;
   bool onlyDirs;
#else
   QT_DIR *dir;
   QT_DIRENT *dirEntry;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
   // for readdir_r
   QScopedPointer<QT_DIRENT, QScopedPointerPodDeleter> mt_file;

#endif
   int lastError;

#endif

   Q_DISABLE_COPY(QFileSystemIterator)
};

QT_END_NAMESPACE

#endif // QT_NO_FILESYSTEMITERATOR

#endif // include guard
