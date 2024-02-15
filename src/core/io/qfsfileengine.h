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

#ifndef QFSFILEENGINE_H
#define QFSFILEENGINE_H

#include <qabstractfileengine.h>

#ifndef QT_NO_FSFILEENGINE

class QFSFileEnginePrivate;

class Q_CORE_EXPORT QFSFileEngine : public QAbstractFileEngine
{
   Q_DECLARE_PRIVATE(QFSFileEngine)

 public:
   QFSFileEngine();
   explicit QFSFileEngine(const QString &file);
   ~QFSFileEngine();

   bool open(QIODevice::OpenMode openMode) override;
   bool open(QIODevice::OpenMode openMode, FILE *fh);
   bool close() override;
   bool flush() override;
   bool syncToDisk() override;
   qint64 size() const override;
   qint64 pos() const override;
   bool seek(qint64 pos) override;
   bool isSequential() const override;
   bool remove() override;
   bool copy(const QString &newName) override;
   bool rename(const QString &newName) override;
   bool renameOverwrite(const QString &newName) override;
   bool link(const QString &newName) override;

   bool mkdir(const QString &dirName, bool createParentDirectories) const override;
   bool rmdir(const QString &dirName, bool recurseParentDirectories) const override;

   bool setSize(qint64 size) override;
   bool caseSensitive() const override;
   bool isRelativePath() const override;

   QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const override;
   FileFlags fileFlags(FileFlags type) const override;
   bool setPermissions(uint perms) override;
   QString fileName(FileName file) const override;
   uint ownerId(FileOwner own) const override;
   QString owner(FileOwner own) const override;
   QDateTime fileTime(FileTime time) const override;
   void setFileName(const QString &file) override;
   int handle() const override;

#ifndef QT_NO_FILESYSTEMITERATOR
   QAbstractFileEngineIterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames) override;
   QAbstractFileEngineIterator *endEntryList() override;
#endif

   qint64 read(char *data, qint64 maxlen) override;
   qint64 readLine(char *data, qint64 maxlen) override;
   qint64 write(const char *data, qint64 len) override;

   bool extension(Extension extension, const ExtensionOption *option = nullptr, ExtensionReturn *output = nullptr) override;
   bool supportsExtension(Extension extension) const override;

   //FS only
   bool open(QIODevice::OpenMode openMode, int fd);
   bool open(QIODevice::OpenMode openMode, int fd, QFile::FileHandleFlags handleFlags);
   bool open(QIODevice::OpenMode openMode, FILE *fh, QFile::FileHandleFlags handleFlags);

   static bool setCurrentPath(const QString &path);
   static QString currentPath(const QString &path = QString());
   static QString homePath();
   static QString rootPath();
   static QString tempPath();
   static QFileInfoList drives();

 protected:
   QFSFileEngine(QFSFileEnginePrivate &dd);
};

#endif // QT_NO_FSFILEENGINE

#endif
