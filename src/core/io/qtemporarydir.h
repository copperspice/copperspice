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

#ifndef QTEMPORARYDIR_H
#define QTEMPORARYDIR_H

#include <qdir.h>
#include <qscopedpointer.h>

#ifndef QT_NO_TEMPORARYFILE

class QTemporaryDirPrivate;

class Q_CORE_EXPORT QTemporaryDir
{
 public:
   QTemporaryDir();
   explicit QTemporaryDir(const QString &tempPath);

   QTemporaryDir(const QTemporaryDir &) = delete;
   QTemporaryDir &operator=(const QTemporaryDir &) = delete;

   ~QTemporaryDir();

   bool isValid() const;

   bool autoRemove() const;
   QString errorString() const;
   void setAutoRemove(bool b);
   bool remove();

   QString path() const;

 private:
   QScopedPointer<QTemporaryDirPrivate> d_ptr;
};

#endif // QT_NO_TEMPORARYFILE

#endif
