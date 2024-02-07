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

#ifndef QFILESYSTEMITERATOR_P_H
#define QFILESYSTEMITERATOR_P_H

#include <qglobal.h>

#ifndef QT_NO_FILESYSTEMITERATOR

#include <qdir.h>
#include <qdiriterator.h>
#include <qstringlist.h>

#include <qfilesystementry_p.h>
#include <qfilesystemmetadata_p.h>

#if ! defined(Q_OS_WIN)
#include <qscopedpointer.h>
#endif

class QFileSystemIterator
{
 public:
   QFileSystemIterator(const QFileSystemEntry &entry, QDir::Filters filters, const QStringList &nameFilters,
         QDirIterator::IteratorFlags flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);

   QFileSystemIterator(const QFileSystemIterator &) = delete;
   QFileSystemIterator &operator=(const QFileSystemIterator &) = delete;

   ~QFileSystemIterator();

   bool advance(QFileSystemEntry &fileEntry, QFileSystemMetaData &metaData);

 private:
   QString nativePath;

#if defined(Q_OS_WIN)
   QString dirPath;
   HANDLE findFileHandle;
   QStringList uncShares;
   bool uncFallback;
   int uncShareIndex;
   bool onlyDirs;
#else
   QT_DIR *dir;
   QT_DIRENT *dirEntry;

   int lastError;
#endif
};

#endif // QT_NO_FILESYSTEMITERATOR

#endif
