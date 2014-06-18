/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QFSFILEENGINE_H
#define QFSFILEENGINE_H

#include <QtCore/qabstractfileengine.h>

#ifndef QT_NO_FSFILEENGINE

QT_BEGIN_NAMESPACE

class QFSFileEnginePrivate;

class Q_CORE_EXPORT QFSFileEngine : public QAbstractFileEngine
{
   Q_DECLARE_PRIVATE(QFSFileEngine)

 public:
   QFSFileEngine();
   explicit QFSFileEngine(const QString &file);
   ~QFSFileEngine();

   bool open(QIODevice::OpenMode openMode);
   bool open(QIODevice::OpenMode flags, FILE *fh);
   bool close();
   bool flush();
   bool syncToDisk();
   qint64 size() const;
   qint64 pos() const;
   bool seek(qint64);
   bool isSequential() const;
   bool remove();
   bool copy(const QString &newName);
   bool rename(const QString &newName);
   bool renameOverwrite(const QString &newName);
   bool link(const QString &newName);
   bool mkdir(const QString &dirName, bool createParentDirectories) const;
   bool rmdir(const QString &dirName, bool recurseParentDirectories) const;
   bool setSize(qint64 size);
   bool caseSensitive() const;
   bool isRelativePath() const;
   QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const;
   FileFlags fileFlags(FileFlags type) const;
   bool setPermissions(uint perms);
   QString fileName(FileName file) const;
   uint ownerId(FileOwner) const;
   QString owner(FileOwner) const;
   QDateTime fileTime(FileTime time) const;
   void setFileName(const QString &file);
   int handle() const;

#ifndef QT_NO_FILESYSTEMITERATOR
   Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames);
   Iterator *endEntryList();
#endif

   qint64 read(char *data, qint64 maxlen);
   qint64 readLine(char *data, qint64 maxlen);
   qint64 write(const char *data, qint64 len);

   bool extension(Extension extension, const ExtensionOption *option = 0, ExtensionReturn *output = 0);
   bool supportsExtension(Extension extension) const;

   //FS only!!
   bool open(QIODevice::OpenMode flags, int fd);
   bool open(QIODevice::OpenMode flags, int fd, QFile::FileHandleFlags handleFlags);
   bool open(QIODevice::OpenMode flags, FILE *fh, QFile::FileHandleFlags handleFlags);

   static bool setCurrentPath(const QString &path);
   static QString currentPath(const QString &path = QString());
   static QString homePath();
   static QString rootPath();
   static QString tempPath();
   static QFileInfoList drives();

 protected:
   QFSFileEngine(QFSFileEnginePrivate &dd);
};

QT_END_NAMESPACE

#endif // QT_NO_FSFILEENGINE

#endif // QFSFILEENGINE_H
