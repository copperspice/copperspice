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

#ifndef QABSTRACTFILEENGINE_H
#define QABSTRACTFILEENGINE_H

#include <qdir.h>

#ifdef open
#error qabstractfileengine.h must be included before any header file that defines open
#endif

class QFileExtension;
class QFileExtensionResult;
class QVariant;
class QAbstractFileEngineIterator;
class QAbstractFileEnginePrivate;
class QAbstractFileEngineIteratorPrivate;

class Q_CORE_EXPORT QAbstractFileEngine
{
 public:
   enum FileFlag {
      //perms (overlaps the QFile::Permission)
      ReadOwnerPerm = 0x4000, WriteOwnerPerm = 0x2000, ExeOwnerPerm = 0x1000,
      ReadUserPerm  = 0x0400, WriteUserPerm  = 0x0200, ExeUserPerm  = 0x0100,
      ReadGroupPerm = 0x0040, WriteGroupPerm = 0x0020, ExeGroupPerm = 0x0010,
      ReadOtherPerm = 0x0004, WriteOtherPerm = 0x0002, ExeOtherPerm = 0x0001,

      //types
      LinkType      = 0x10000,
      FileType      = 0x20000,
      DirectoryType = 0x40000,
      BundleType    = 0x80000,

      //flags
      HiddenFlag     = 0x0100000,
      LocalDiskFlag  = 0x0200000,
      ExistsFlag     = 0x0400000,
      RootFlag       = 0x0800000,
      Refresh        = 0x1000000,

      //masks
      PermsMask  = 0x0000FFFF,
      TypesMask  = 0x000F0000,
      FlagsMask  = 0x0FF00000,
      FileInfoAll = FlagsMask | PermsMask | TypesMask
   };
   using FileFlags = QFlags<FileFlag>;

   enum FileName {
      DefaultName,
      BaseName,
      PathName,
      AbsoluteName,
      AbsolutePathName,
      LinkName,
      CanonicalName,
      CanonicalPathName,
      BundleName,
      NFileNames = 9
   };
   enum FileOwner {
      OwnerUser,
      OwnerGroup
   };
   enum FileTime {
      CreationTime,
      ModificationTime,
      AccessTime
   };

   QAbstractFileEngine(const QAbstractFileEngine &) = delete;
   QAbstractFileEngine &operator=(const QAbstractFileEngine &) = delete;

   virtual ~QAbstractFileEngine();

   virtual bool open(QIODevice::OpenMode mode);
   virtual bool close();
   virtual bool flush();
   virtual qint64 size() const;
   virtual qint64 pos() const;
   virtual bool seek(qint64 offset);
   virtual bool isSequential() const;
   virtual bool remove();
   virtual bool copy(const QString &newName);
   virtual bool rename(const QString &newName);
   virtual bool renameOverwrite(const QString &newName);
   virtual bool syncToDisk();
   virtual bool link(const QString &newName);
   virtual bool mkdir(const QString &dirName, bool createParentDirectories) const;
   virtual bool rmdir(const QString &dirName, bool recurseParentDirectories) const;
   virtual bool setSize(qint64 size);
   virtual bool caseSensitive() const;
   virtual bool isRelativePath() const;
   virtual QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const;
   virtual FileFlags fileFlags(FileFlags type = FileInfoAll) const;
   virtual bool setPermissions(uint perms);
   virtual QString fileName(FileName file = DefaultName) const;
   virtual uint ownerId(FileOwner owner) const;
   virtual QString owner(FileOwner owner) const;
   virtual QDateTime fileTime(FileTime time) const;
   virtual void setFileName(const QString &file);
   virtual int handle() const;

   bool atEnd() const;
   uchar *map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags);
   bool unmap(uchar *address);

   virtual QAbstractFileEngineIterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames);
   virtual QAbstractFileEngineIterator *endEntryList();

   virtual qint64 read(char *data, qint64 maxlen);
   virtual qint64 readLine(char *data, qint64 maxlen);
   virtual qint64 write(const char *data, qint64 len);

   QFile::FileError error() const;
   QString errorString() const;

   enum Extension {
      AtEndExtension,
      FastReadLineExtension,
      MapExtension,
      UnMapExtension
   };

   class ExtensionOption
   { };

   class ExtensionReturn
   { };

   class MapExtensionOption : public ExtensionOption
   {
    public:
      qint64 offset;
      qint64 size;
      QFile::MemoryMapFlags flags;
   };

   class MapExtensionReturn : public ExtensionReturn
   {
    public:
      uchar *address;
   };

   class UnMapExtensionOption : public ExtensionOption
   {
    public:
      uchar *address;
   };

   virtual bool extension(Extension extension, const ExtensionOption *option = nullptr, ExtensionReturn *output = nullptr);
   virtual bool supportsExtension(Extension extension) const;

   // Factory
   static QAbstractFileEngine *create(const QString &fileName);

 protected:
   void setError(QFile::FileError error, const QString &errorString);

   QAbstractFileEngine();
   QAbstractFileEngine(QAbstractFileEnginePrivate &);

   QScopedPointer<QAbstractFileEnginePrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QAbstractFileEngine)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractFileEngine::FileFlags)

class Q_CORE_EXPORT QAbstractFileEngineHandler
{
 public:
   QAbstractFileEngineHandler();
   virtual ~QAbstractFileEngineHandler();
   virtual QAbstractFileEngine *create(const QString &fileName) const = 0;
};

class Q_CORE_EXPORT QAbstractFileEngineIterator
{
 public:
   QAbstractFileEngineIterator(QDir::Filters filters, const QStringList &nameFilters);

   QAbstractFileEngineIterator(const QAbstractFileEngineIterator &) = delete;
   QAbstractFileEngineIterator &operator=(const QAbstractFileEngineIterator &) = delete;

   virtual ~QAbstractFileEngineIterator();

   virtual QString next() = 0;
   virtual bool hasNext() const = 0;

   QString path() const;
   QStringList nameFilters() const;
   QDir::Filters filters() const;

   virtual QString currentFileName() const = 0;
   virtual QFileInfo currentFileInfo() const;
   QString currentFilePath() const;

 private:
   friend class QDirIterator;
   friend class QDirIteratorPrivate;
   void setPath(const QString &path);
   QScopedPointer<QAbstractFileEngineIteratorPrivate> d;
};

#endif
