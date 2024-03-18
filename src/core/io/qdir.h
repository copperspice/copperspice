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

#ifndef QDIR_H
#define QDIR_H

#include <qfileinfo.h>
#include <qshareddata.h>
#include <qstring.h>
#include <qstringlist.h>

class QDebug;
class QDirIterator;
class QDirPrivate;

class Q_CORE_EXPORT QDir
{
 public:
   enum Filter {
      Dirs            = 0x001,
      Files           = 0x002,
      Drives          = 0x004,
      NoSymLinks      = 0x008,
      AllEntries      = Dirs | Files | Drives,
      TypeMask        = 0x00f,
      Readable        = 0x010,
      Writable        = 0x020,
      Executable      = 0x040,
      PermissionMask  = 0x070,
      Modified        = 0x080,
      Hidden          = 0x100,
      System          = 0x200,

      AccessMask      = 0x3F0,

      AllDirs         = 0x400,
      CaseSensitive   = 0x800,
      NoDot           = 0x2000,
      NoDotDot        = 0x4000,
      NoDotAndDotDot  = NoDot | NoDotDot,

      NoFilter = -1
   };

   using Filters = QFlags<Filter>;

   enum SortFlag {
      Name        = 0x00,
      Time        = 0x01,
      Size        = 0x02,
      Unsorted    = 0x03,
      SortByMask  = 0x03,

      DirsFirst   = 0x04,
      Reversed    = 0x08,
      IgnoreCase  = 0x10,
      DirsLast    = 0x20,
      LocaleAware = 0x40,
      Type        = 0x80,
      NoSort      = -1
   };

   using SortFlags = QFlags<SortFlag>;

   QDir(const QDir &dir);
   QDir(const QString &path = QString());
   QDir(const QString &path, const QString &nameFilter, SortFlags sort = SortFlags(Name | IgnoreCase),
         Filters filters = AllEntries);

   ~QDir();

   QDir &operator=(const QDir &other);
   QDir &operator=(const QString &path);

   QDir &operator=(QDir &&other) {
      swap(other);
      return *this;
   }

   void setPath(const QString &path);
   QString path() const;
   QString absolutePath() const;
   QString canonicalPath() const;

   static void addResourceSearchPath(const QString &path);

   static void setSearchPaths(const QString &prefix, const QStringList &searchPaths);
   static void addSearchPath(const QString &prefix, const QString &path);
   static QStringList searchPaths(const QString &prefix);

   QString dirName() const;
   QString filePath(const QString &fileName) const;
   QString absoluteFilePath(const QString &fileName) const;
   QString relativeFilePath(const QString &fileName) const;

   static QString toNativeSeparators(const QString &pathName);
   static QString fromNativeSeparators(const QString &pathName);

   bool cd(const QString &dirName);
   bool cdUp();

   QStringList nameFilters() const;
   void setNameFilters(const QStringList &nameFilters);

   Filters filter() const;
   void setFilter(Filters filters);
   SortFlags sorting() const;
   void setSorting(SortFlags sort);

   uint count() const;
   QString operator[](int pos) const;

   static QStringList nameFiltersFromString(const QString &nameFilter);

   QStringList entryList(Filters filters = NoFilter, SortFlags sort = NoSort) const;
   QStringList entryList(const QStringList &nameFilters, Filters filters = NoFilter,
         SortFlags sort = NoSort) const;

   QFileInfoList entryInfoList(Filters filters = NoFilter, SortFlags sort = NoSort) const;
   QFileInfoList entryInfoList(const QStringList &nameFilters, Filters filters = NoFilter,
         SortFlags sort = NoSort) const;

   bool mkdir(const QString &dirName) const;
   bool rmdir(const QString &dirName) const;
   bool mkpath(const QString &dirPath) const;
   bool rmpath(const QString &dirPath) const;

   bool removeRecursively();

   bool isReadable() const;
   bool exists() const;
   bool isRoot() const;

   void swap(QDir &other) {
      qSwap(d_ptr, other.d_ptr);
   }

   static bool isRelativePath(const QString &path);
   static bool isAbsolutePath(const QString &path) {
      return !isRelativePath(path);
   }

   bool isRelative() const;
   bool isAbsolute() const {
      return !isRelative();
   }

   bool makeAbsolute();

   bool operator==(const QDir &other) const;
   bool operator!=(const QDir &other) const {
      return !operator==(other);
   }

   bool remove(const QString &fileName);
   bool rename(const QString &oldName, const QString &newName);
   bool exists(const QString &name) const;

   static QFileInfoList drives();

   static QChar listSeparator() {
#if defined(Q_OS_WIN)
      return ';';
#else
      return ':';
#endif
   }
   static QChar separator();

   static bool setCurrent(const QString &path);
   static QDir current() {
      return QDir(currentPath());
   }

   static QString currentPath();

   static QDir home() {
      return QDir(homePath());
   }

   static QString homePath();

   static QDir root() {
      return QDir(rootPath());
   }

   static QString rootPath();
   static QDir temp() {
      return QDir(tempPath());
   }

   static QString tempPath();

   static bool match(const QStringList &filters, const QString &fileName);
   static bool match(const QString &filters, const QString &fileName);

   static QString cleanPath(const QString &path);
   void refresh() const;

 protected:
   explicit QDir(QDirPrivate &d);
   QSharedDataPointer<QDirPrivate> d_ptr;

   friend class QDirIterator;
   QDirPrivate *d_func();

   const QDirPrivate *d_func() const {
      return d_ptr.constData();
   }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDir::Filters)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDir::SortFlags)

Q_CORE_EXPORT QDebug operator<<(QDebug debug, QDir::Filters filters);
Q_CORE_EXPORT QDebug operator<<(QDebug debug, const QDir &dir);

#endif
