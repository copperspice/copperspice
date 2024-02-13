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

#ifndef QDIR_P_H
#define QDIR_P_H

#include <qfilesystementry_p.h>
#include <qfilesystemmetadata_p.h>

class QDirPrivate : public QSharedData
{
 public:
   QDirPrivate(const QString &path, const QStringList &nameFilters_ = QStringList(),
         QDir::SortFlags sort_ = QDir::SortFlags(QDir::Name | QDir::IgnoreCase),
         QDir::Filters filters_ = QDir::AllEntries);

   QDirPrivate(const QDirPrivate &copy);

   bool exists() const;

   void initFileEngine();
   void initFileLists(const QDir &dir) const;

   static void sortFileList(QDir::SortFlags, QFileInfoList &, QStringList *, QFileInfoList *);

   static inline QChar getFilterSepChar(const QString &nameFilter);

   static inline QStringList splitFilters(const QString &nameFilter, QChar sep = QChar());

   void setPath(const QString &path);

   void clearFileLists();

   void resolveAbsoluteEntry() const;

   QStringList nameFilters;
   QDir::SortFlags sort;
   QDir::Filters filters;

   QScopedPointer<QAbstractFileEngine> fileEngine;

   mutable bool fileListsInitialized;
   mutable QStringList files;
   mutable QFileInfoList fileInfos;

   QFileSystemEntry dirEntry;
   mutable QFileSystemEntry absoluteDirEntry;
   mutable QFileSystemMetaData metaData;
};

#endif
