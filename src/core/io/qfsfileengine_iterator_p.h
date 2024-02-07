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

#ifndef QFSFILEENGINE_ITERATOR_P_H
#define QFSFILEENGINE_ITERATOR_P_H

#include <qabstractfileengine.h>
#include <qdir.h>

#include <qfilesystemiterator_p.h>

#ifndef QT_NO_FSFILEENGINE
#ifndef QT_NO_FILESYSTEMITERATOR

class QFSFileEngineIteratorPrivate;
class QFSFileEngineIteratorPlatformSpecificData;

class QFSFileEngineIterator : public QAbstractFileEngineIterator
{
 public:
   QFSFileEngineIterator(QDir::Filters filters, const QStringList &filterNames);
   ~QFSFileEngineIterator();

   QString next() override;
   bool hasNext() const override;

   QString currentFileName() const override;
   QFileInfo currentFileInfo() const override;

 private:
   void advance() const;
   mutable QScopedPointer<QFileSystemIterator> nativeIterator;
   mutable QFileInfo currentInfo;
   mutable QFileInfo nextInfo;
   mutable bool done;
};

#endif // QT_NO_FILESYSTEMITERATOR
#endif // QT_NO_FSFILEENGINE

#endif