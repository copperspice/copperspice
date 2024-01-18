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

#ifndef QDIRITERATOR_H
#define QDIRITERATOR_H

#include <qdir.h>

class QDirIteratorPrivate;

class Q_CORE_EXPORT QDirIterator
{
 public:
   enum IteratorFlag {
      NoIteratorFlags = 0x0,
      FollowSymlinks = 0x1,
      Subdirectories = 0x2
   };
   using IteratorFlags = QFlags<IteratorFlag>;

   QDirIterator(const QDir &dir, IteratorFlags flags = NoIteratorFlags);
   QDirIterator(const QString &path, IteratorFlags flags = NoIteratorFlags);

   QDirIterator(const QString &path, QDir::Filters filters, IteratorFlags flags = NoIteratorFlags);
   QDirIterator(const QString &path, const QStringList &nameFilters, QDir::Filters filters = QDir::NoFilter, IteratorFlags flags = NoIteratorFlags);

   QDirIterator(const QDirIterator &) = delete;
   QDirIterator &operator=(const QDirIterator &) = delete;

   ~QDirIterator();

   QString next();
   bool hasNext() const;

   QString fileName() const;
   QString filePath() const;
   QFileInfo fileInfo() const;
   QString path() const;

 private:
   QScopedPointer<QDirIteratorPrivate> d;
   friend class QDir;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDirIterator::IteratorFlags)

#endif
