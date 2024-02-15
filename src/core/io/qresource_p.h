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

#ifndef QRESOURCE_P_H
#define QRESOURCE_P_H

#include <qabstractfileengine.h>

class QResourceFileEnginePrivate;

class QResourceFileEngine : public QAbstractFileEngine
{
   Q_DECLARE_PRIVATE(QResourceFileEngine)

 public:
   explicit QResourceFileEngine(const QString &path);
   ~QResourceFileEngine();

   void setFileName(const QString &file) override;

   bool open(QIODevice::OpenMode flags) override;
   bool close() override;
   bool flush() override;
   qint64 size() const override;
   qint64 pos() const override;
   bool atEnd() const;
   bool seek(qint64) override;
   qint64 read(char *data, qint64 maxlen) override;
   qint64 write(const char *data, qint64 len) override;

   bool remove() override;
   bool copy(const QString &newName) override;
   bool rename(const QString &newName) override;
   bool link(const QString &newName) override;

   bool isSequential() const override;
   bool isRelativePath() const override;

   bool mkdir(const QString &dirName, bool createParentDirectories) const override;
   bool rmdir(const QString &dirName, bool recurseParentDirectories) const override;

   bool setSize(qint64 size) override;

   QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const override;

   bool caseSensitive() const override;

   FileFlags fileFlags(FileFlags type) const override;

   bool setPermissions(uint perms) override;

   QString fileName(QAbstractFileEngine::FileName file) const override;

   uint ownerId(FileOwner) const override;
   QString owner(FileOwner) const override;

   QDateTime fileTime(FileTime time) const override;

   QAbstractFileEngineIterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames) override;
   QAbstractFileEngineIterator *endEntryList() override;

   bool extension(Extension extension, const ExtensionOption *option = nullptr, ExtensionReturn *output = nullptr) override;
   bool supportsExtension(Extension extension) const override;
};

#endif
