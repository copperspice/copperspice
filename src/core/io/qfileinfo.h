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

#ifndef QFILEINFO_H
#define QFILEINFO_H

#include <QtCore/qfile.h>
#include <QtCore/qlist.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QDir;
class QDirIteratorPrivate;
class QDateTime;
class QFileInfoPrivate;

class Q_CORE_EXPORT QFileInfo
{
   friend class QDirIteratorPrivate;

 public:
   explicit QFileInfo(QFileInfoPrivate *d);

   QFileInfo();
   QFileInfo(const QString &file);
   QFileInfo(const QFile &file);
   QFileInfo(const QDir &dir, const QString &file);
   QFileInfo(const QFileInfo &fileinfo);
   ~QFileInfo();

   QFileInfo &operator=(const QFileInfo &fileinfo);

   inline QFileInfo &operator=(QFileInfo && other) {
      qSwap(d_ptr, other.d_ptr);
      return *this;
   }

   bool operator==(const QFileInfo &fileinfo); // 5.0 - remove me
   bool operator==(const QFileInfo &fileinfo) const;

   inline bool operator!=(const QFileInfo &fileinfo) {
      return !(operator==(fileinfo));   // 5.0 - remove me
   }

   inline bool operator!=(const QFileInfo &fileinfo) const {
      return !(operator==(fileinfo));
   }

   void setFile(const QString &file);
   void setFile(const QFile &file);
   void setFile(const QDir &dir, const QString &file);
   bool exists() const;
   void refresh();

   QString filePath() const;
   QString absoluteFilePath() const;
   QString canonicalFilePath() const;
   QString fileName() const;
   QString baseName() const;
   QString completeBaseName() const;
   QString suffix() const;
   QString bundleName() const;
   QString completeSuffix() const;

   QString path() const;
   QString absolutePath() const;
   QString canonicalPath() const;
   QDir dir() const;
   QDir absoluteDir() const;

   bool isReadable() const;
   bool isWritable() const;
   bool isExecutable() const;
   bool isHidden() const;

   bool isRelative() const;
   inline bool isAbsolute() const {
      return !isRelative();
   }
   bool makeAbsolute();

   bool isFile() const;
   bool isDir() const;
   bool isSymLink() const;
   bool isRoot() const;
   bool isBundle() const;

   QString readLink() const;
   inline QString symLinkTarget() const {
      return readLink();
   }

   QString owner() const;
   uint ownerId() const;
   QString group() const;
   uint groupId() const;

   bool permission(QFile::Permissions permissions) const;
   QFile::Permissions permissions() const;

   qint64 size() const;

   QDateTime created() const;
   QDateTime lastModified() const;
   QDateTime lastRead() const;

   void detach();

   bool caching() const;
   void setCaching(bool on);

 protected:
   QSharedDataPointer<QFileInfoPrivate> d_ptr;

 private:
   inline QFileInfoPrivate *d_func() {
      detach();
      return const_cast<QFileInfoPrivate *>(d_ptr.constData());
   }

   inline const QFileInfoPrivate *d_func() const {
      return d_ptr.constData();
   }
};

Q_DECLARE_TYPEINFO(QFileInfo, Q_MOVABLE_TYPE);

typedef QList<QFileInfo> QFileInfoList;

QT_END_NAMESPACE

#endif // QFILEINFO_H
