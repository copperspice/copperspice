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

#ifndef QRESOURCE_H
#define QRESOURCE_H

#include <qstring.h>
#include <qlocale.h>
#include <qstringlist.h>
#include <qlist.h>

class QResourcePrivate;

class Q_CORE_EXPORT QResource
{
 public:
   QResource(const QString &file = QString(), const QLocale &locale = QLocale());
   ~QResource();

   void setFileName(const QString &file);
   QString fileName() const;
   QString absoluteFilePath() const;

   void setLocale(const QLocale &locale);
   QLocale locale() const;

   bool isValid() const;

   bool isCompressed() const;
   qint64 size() const;
   const uchar *data() const;

   static void addSearchPath(const QString &path);
   static QStringList searchPaths();

   static bool registerResource(const QString &fileName, const QString &resourceRoot = QString());
   static bool unregisterResource(const QString &fileName, const QString &resourceRoot = QString());

   static bool registerResource(const uchar *rccData, const QString &resourceRoot = QString());
   static bool unregisterResource(const uchar *rccData, const QString &resourceRoot = QString());

 protected:
   friend class QResourceFileEngine;
   friend class QResourceFileEngineIterator;
   bool isDir() const;

   bool isFile() const {
      return !isDir();
   }

   QStringList children() const;

   QScopedPointer<QResourcePrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QResource)
};

#endif
