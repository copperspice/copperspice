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

#include <qdiriterator.h>

#include <qalgorithms.h>
#include <qabstractfileengine.h>
#include <qset.h>
#include <qstack.h>
#include <qregularexpression.h>
#include <qvariant.h>

#include <qdir_p.h>
#include <qfileinfo_p.h>
#include <qfilesystemiterator_p.h>
#include <qfilesystementry_p.h>
#include <qfilesystemmetadata_p.h>
#include <qfilesystemengine_p.h>

template <class Iterator>
class QDirIteratorPrivateIteratorStack : public QStack<Iterator *>
{
 public:
   ~QDirIteratorPrivateIteratorStack() {
      qDeleteAll(*this);
   }
};

class QDirIteratorPrivate
{
 public:
   QDirIteratorPrivate(const QFileSystemEntry &entry, const QStringList &nameFilters,
         QDir::Filters filters, QDirIterator::IteratorFlags flags, bool resolveEngine = true);

   void advance();

   bool entryMatches(const QString &fileName, const QFileInfo &fileInfo);
   void pushDirectory(const QFileInfo &fileInfo);
   void checkAndPushDirectory(const QFileInfo &);
   bool matchesFilters(const QString &fileName, const QFileInfo &fi) const;

   QScopedPointer<QAbstractFileEngine> engine;

   QFileSystemEntry dirEntry;
   const QStringList nameFilters;
   const QDir::Filters filters;
   const QDirIterator::IteratorFlags iteratorFlags;

   QVector<QRegularExpression> nameRegExps;

   QDirIteratorPrivateIteratorStack<QAbstractFileEngineIterator> fileEngineIterators;

#ifndef QT_NO_FILESYSTEMITERATOR
   QDirIteratorPrivateIteratorStack<QFileSystemIterator> nativeIterators;
#endif

   QFileInfo currentFileInfo;
   QFileInfo nextFileInfo;

   // Loop protection
   QSet<QString> visitedLinks;
};

QDirIteratorPrivate::QDirIteratorPrivate(const QFileSystemEntry &entry, const QStringList &nameFilters,
      QDir::Filters filters, QDirIterator::IteratorFlags flags, bool resolveEngine)
   : dirEntry(entry), nameFilters(nameFilters.contains("*") ? QStringList() : nameFilters),
     filters(QDir::NoFilter == filters ? QDir::AllEntries : filters), iteratorFlags(flags)
{
   nameRegExps.reserve(nameFilters.size());

   for (int i = 0; i < nameFilters.size(); ++i) {
      QPatternOptionFlags options = QPatternOption::WildcardOption | QPatternOption::ExactMatchOption;

      if (! (filters & QDir::CaseSensitive)) {
         options |= QPatternOption::CaseInsensitiveOption;
      }

      QRegularExpression regExp(nameFilters.at(i), options);
      nameRegExps.append(regExp);
   }

   QFileSystemMetaData metaData;

   if (resolveEngine) {
      engine.reset(QFileSystemEngine::resolveEntryAndCreateLegacyEngine(dirEntry, metaData));
   }

   QFileInfo fileInfo(new QFileInfoPrivate(dirEntry, metaData));

   // Populate fields for hasNext() and next()
   pushDirectory(fileInfo);
   advance();
}

void QDirIteratorPrivate::pushDirectory(const QFileInfo &fileInfo)
{
   QString path = fileInfo.filePath();

#ifdef Q_OS_WIN

   if (fileInfo.isSymLink()) {
      path = fileInfo.canonicalFilePath();
   }

#endif

   if (iteratorFlags & QDirIterator::FollowSymlinks) {
      visitedLinks << fileInfo.canonicalFilePath();
   }

   if (engine) {
      engine->setFileName(path);
      QAbstractFileEngineIterator *it = engine->beginEntryList(filters, nameFilters);

      if (it) {
         it->setPath(path);
         fileEngineIterators << it;
      } else {
         // No iterator; no entry list.
      }
   } else {
#ifndef QT_NO_FILESYSTEMITERATOR
      QFileSystemIterator *it = new QFileSystemIterator(fileInfo.d_ptr->fileEntry,
            filters, nameFilters, iteratorFlags);
      nativeIterators << it;
#endif
   }
}

inline bool QDirIteratorPrivate::entryMatches(const QString &fileName, const QFileInfo &fileInfo)
{
   checkAndPushDirectory(fileInfo);

   if (matchesFilters(fileName, fileInfo)) {
      currentFileInfo = nextFileInfo;
      nextFileInfo = fileInfo;

      //We found a matching entry.
      return true;
   }

   return false;
}

void QDirIteratorPrivate::advance()
{
   if (engine) {
      while (!fileEngineIterators.isEmpty()) {
         // Find the next valid iterator that matches the filters.
         QAbstractFileEngineIterator *it;

         while (it = fileEngineIterators.top(), it->hasNext()) {
            it->next();

            if (entryMatches(it->currentFileName(), it->currentFileInfo())) {
               return;
            }
         }

         fileEngineIterators.pop();
         delete it;
      }

   } else {
#ifndef QT_NO_FILESYSTEMITERATOR
      QFileSystemEntry nextEntry;
      QFileSystemMetaData nextMetaData;

      while (!nativeIterators.isEmpty()) {
         // Find the next valid iterator that matches the filters.
         QFileSystemIterator *it;

         while (it = nativeIterators.top(), it->advance(nextEntry, nextMetaData)) {
            QFileInfo info(new QFileInfoPrivate(nextEntry, nextMetaData));

            if (entryMatches(nextEntry.fileName(), info)) {
               return;
            }
         }

         nativeIterators.pop();
         delete it;
      }

#endif
   }

   currentFileInfo = nextFileInfo;
   nextFileInfo = QFileInfo();
}

void QDirIteratorPrivate::checkAndPushDirectory(const QFileInfo &fileInfo)
{
   // If we're doing flat iteration, we're done.
   if (!(iteratorFlags & QDirIterator::Subdirectories)) {
      return;
   }

   // Never follow non-directory entries
   if (!fileInfo.isDir()) {
      return;
   }

   // Follow symlinks only when asked
   if (!(iteratorFlags & QDirIterator::FollowSymlinks) && fileInfo.isSymLink()) {
      return;
   }

   // Never follow . and ..
   QString fileName = fileInfo.fileName();

   if ("." == fileName || ".." == fileName) {
      return;
   }

   // No hidden directories unless requested
   if (! (filters & QDir::AllDirs) && ! (filters & QDir::Hidden) && fileInfo.isHidden()) {
      return;
   }

   // Stop link loops
   if (! visitedLinks.isEmpty() && visitedLinks.contains(fileInfo.canonicalFilePath())) {
      return;
   }

   pushDirectory(fileInfo);
}

bool QDirIteratorPrivate::matchesFilters(const QString &fileName, const QFileInfo &fi) const
{
   Q_ASSERT(!fileName.isEmpty());

   // filter . and ..
   const int fileNameSize = fileName.size();
   const bool dotOrDotDot = fileName == "." || fileName == "..";

   if ((filters & QDir::NoDot) && dotOrDotDot && fileNameSize == 1) {
      return false;
   }

   if ((filters & QDir::NoDotDot) && dotOrDotDot && fileNameSize == 2) {
      return false;
   }

   // name filter
   // Pass all entries through name filters, except dirs if the AllDirs

   if (! nameFilters.isEmpty() && ! ((filters & QDir::AllDirs) && fi.isDir())) {
      bool matched = false;

      for (const auto &regExp : nameRegExps) {
         if (regExp.match(fileName).hasMatch()) {
            matched = true;
            break;
         }
      }

      if (! matched) {
         return false;
      }
   }

   // skip symlinks
   const bool skipSymlinks  = (filters & QDir::NoSymLinks);
   const bool includeSystem = (filters & QDir::System);

   if (skipSymlinks && fi.isSymLink()) {
      // The only reason to save this file is if it is a broken link and we are requesting system files.
      if (!includeSystem || fi.exists()) {
         return false;
      }
   }

   // filter hidden
   const bool includeHidden = (filters & QDir::Hidden);

   if (! includeHidden && ! dotOrDotDot && fi.isHidden()) {
      return false;
   }

   // filter system files
   if (!includeSystem && (!(fi.isFile() || fi.isDir() || fi.isSymLink()) || (!fi.exists() && fi.isSymLink()))) {
      return false;
   }

   // skip directories
   const bool skipDirs = !(filters & (QDir::Dirs | QDir::AllDirs));

   if (skipDirs && fi.isDir()) {
      return false;
   }

   // skip files
   const bool skipFiles = !(filters & QDir::Files);

   if (skipFiles && fi.isFile()) {
      // Basically we need a reason not to exclude this file otherwise we just eliminate it
      return false;
   }

   // filter permissions
   const bool filterPermissions = ((filters & QDir::PermissionMask) && (filters & QDir::PermissionMask) != QDir::PermissionMask);
   const bool doWritable   = !filterPermissions || (filters & QDir::Writable);
   const bool doExecutable = !filterPermissions || (filters & QDir::Executable);
   const bool doReadable   = !filterPermissions || (filters & QDir::Readable);

   if (filterPermissions && ((doReadable && ! fi.isReadable())
         || (doWritable && ! fi.isWritable()) || (doExecutable && !fi.isExecutable()))) {
      return false;
   }

   return true;
}

QDirIterator::QDirIterator(const QDir &dir, IteratorFlags flags)
{
   const QDirPrivate *other = dir.d_ptr.constData();
   d.reset(new QDirIteratorPrivate(other->dirEntry, other->nameFilters, other->filters, flags, ! other->fileEngine.isNull()));
}

QDirIterator::QDirIterator(const QString &path, QDir::Filters filters, IteratorFlags flags)
   : d(new QDirIteratorPrivate(QFileSystemEntry(path), QStringList(), filters, flags))
{
}

QDirIterator::QDirIterator(const QString &path, IteratorFlags flags)
   : d(new QDirIteratorPrivate(QFileSystemEntry(path), QStringList(), QDir::NoFilter, flags))
{
}

QDirIterator::QDirIterator(const QString &path, const QStringList &nameFilters,
      QDir::Filters filters, IteratorFlags flags)
   : d(new QDirIteratorPrivate(QFileSystemEntry(path), nameFilters, filters, flags))
{
}

QDirIterator::~QDirIterator()
{
}

QString QDirIterator::next()
{
   d->advance();
   return filePath();
}

bool QDirIterator::hasNext() const
{
   if (d->engine) {
      return !d->fileEngineIterators.isEmpty();

   } else
#ifndef QT_NO_FILESYSTEMITERATOR
      return !d->nativeIterators.isEmpty();

#else
      return false;
#endif
}

QString QDirIterator::fileName() const
{
   return d->currentFileInfo.fileName();
}

QString QDirIterator::filePath() const
{
   return d->currentFileInfo.filePath();
}

QFileInfo QDirIterator::fileInfo() const
{
   return d->currentFileInfo;
}

QString QDirIterator::path() const
{
   return d->dirEntry.filePath();
}
