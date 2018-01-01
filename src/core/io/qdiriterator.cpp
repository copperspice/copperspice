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

#include <qdiriterator.h>
#include <qdir_p.h>
#include <qabstractfileengine.h>
#include <qset.h>
#include <qstack.h>
#include <qvariant.h>
#include <qfilesystemiterator_p.h>
#include <qfilesystementry_p.h>
#include <qfilesystemmetadata_p.h>
#include <qfilesystemengine_p.h>
#include <qfsfileengine.h>
#include <qfileinfo_p.h>

QT_BEGIN_NAMESPACE

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

#ifndef QT_NO_REGEXP
   QVector<QRegExp> nameRegExps;
#endif

   QDirIteratorPrivateIteratorStack<QAbstractFileEngineIterator> fileEngineIterators;
#ifndef QT_NO_FILESYSTEMITERATOR
   QDirIteratorPrivateIteratorStack<QFileSystemIterator> nativeIterators;
#endif

   QFileInfo currentFileInfo;
   QFileInfo nextFileInfo;

   // Loop protection
   QSet<QString> visitedLinks;
};

/*!
    \internal
*/
QDirIteratorPrivate::QDirIteratorPrivate(const QFileSystemEntry &entry, const QStringList &nameFilters,
      QDir::Filters filters, QDirIterator::IteratorFlags flags, bool resolveEngine)
   : dirEntry(entry)
   , nameFilters(nameFilters.contains(QLatin1String("*")) ? QStringList() : nameFilters)
   , filters(QDir::NoFilter == filters ? QDir::AllEntries : filters)
   , iteratorFlags(flags)
{
#ifndef QT_NO_REGEXP
   nameRegExps.reserve(nameFilters.size());
   for (int i = 0; i < nameFilters.size(); ++i)
      nameRegExps.append(
         QRegExp(nameFilters.at(i),
                 (filters & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive,
                 QRegExp::Wildcard));
#endif
   QFileSystemMetaData metaData;
   if (resolveEngine) {
      engine.reset(QFileSystemEngine::resolveEntryAndCreateLegacyEngine(dirEntry, metaData));
   }
   QFileInfo fileInfo(new QFileInfoPrivate(dirEntry, metaData));

   // Populate fields for hasNext() and next()
   pushDirectory(fileInfo);
   advance();
}

/*!
    \internal
*/
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

/*!
    \internal
*/
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

/*!
    \internal
 */
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
   if (QLatin1String(".") == fileName || QLatin1String("..") == fileName) {
      return;
   }

   // No hidden directories unless requested
   if (!(filters & QDir::AllDirs) && !(filters & QDir::Hidden) && fileInfo.isHidden()) {
      return;
   }

   // Stop link loops
   if (!visitedLinks.isEmpty() &&
         visitedLinks.contains(fileInfo.canonicalFilePath())) {
      return;
   }

   pushDirectory(fileInfo);
}

/*!
    \internal

    This convenience function implements the iterator's filtering logics and
    applies then to the current directory entry.

    It returns true if the current entry matches the filters (i.e., the
    current entry will be returned as part of the directory iteration);
    otherwise, false is returned.
*/

bool QDirIteratorPrivate::matchesFilters(const QString &fileName, const QFileInfo &fi) const
{
   Q_ASSERT(!fileName.isEmpty());

   // filter . and ..?
   const int fileNameSize = fileName.size();
   const bool dotOrDotDot = fileName[0] == QLatin1Char('.')
                            && ((fileNameSize == 1)
                                || (fileNameSize == 2 && fileName[1] == QLatin1Char('.')));
   if ((filters & QDir::NoDot) && dotOrDotDot && fileNameSize == 1) {
      return false;
   }
   if ((filters & QDir::NoDotDot) && dotOrDotDot && fileNameSize == 2) {
      return false;
   }
   if ((filters & QDir::NoDotAndDotDot) && dotOrDotDot) { // ### Qt5/remove (NoDotAndDotDot == NoDot|NoDotDot)
      return false;
   }

   // name filter
#ifndef QT_NO_REGEXP
   // Pass all entries through name filters, except dirs if the AllDirs
   if (!nameFilters.isEmpty() && !((filters & QDir::AllDirs) && fi.isDir())) {
      bool matched = false;
      for (QVector<QRegExp>::const_iterator iter = nameRegExps.constBegin(),
            end = nameRegExps.constEnd();
            iter != end; ++iter) {

         if (iter->exactMatch(fileName)) {
            matched = true;
            break;
         }
      }
      if (!matched) {
         return false;
      }
   }
#endif
   // skip symlinks
   const bool skipSymlinks = (filters & QDir::NoSymLinks);
   const bool includeSystem = (filters & QDir::System);
   if (skipSymlinks && fi.isSymLink()) {
      // The only reason to save this file is if it is a broken link and we are requesting system files.
      if (!includeSystem || fi.exists()) {
         return false;
      }
   }

   // filter hidden
   const bool includeHidden = (filters & QDir::Hidden);
   if (!includeHidden && !dotOrDotDot && fi.isHidden()) {
      return false;
   }

   // filter system files
   if (!includeSystem && (!(fi.isFile() || fi.isDir() || fi.isSymLink())
                          || (!fi.exists() && fi.isSymLink()))) {
      return false;
   }

   // skip directories
   const bool skipDirs = !(filters & (QDir::Dirs | QDir::AllDirs));
   if (skipDirs && fi.isDir()) {
      return false;
   }

   // skip files
   const bool skipFiles    = !(filters & QDir::Files);
   if (skipFiles && fi.isFile())
      // Basically we need a reason not to exclude this file otherwise we just eliminate it.
   {
      return false;
   }

   // filter permissions
   const bool filterPermissions = ((filters & QDir::PermissionMask)
                                   && (filters & QDir::PermissionMask) != QDir::PermissionMask);
   const bool doWritable = !filterPermissions || (filters & QDir::Writable);
   const bool doExecutable = !filterPermissions || (filters & QDir::Executable);
   const bool doReadable = !filterPermissions || (filters & QDir::Readable);
   if (filterPermissions
         && ((doReadable && !fi.isReadable())
             || (doWritable && !fi.isWritable())
             || (doExecutable && !fi.isExecutable()))) {
      return false;
   }

   return true;
}

/*!
    Constructs a QDirIterator that can iterate over \a dir's entrylist, using
    \a dir's name filters and regular filters. You can pass options via \a
    flags to decide how the directory should be iterated.

    By default, \a flags is NoIteratorFlags, which provides the same behavior
    as in QDir::entryList().

    The sorting in \a dir is ignored.

    \note To list symlinks that point to non existing files, QDir::System must be
     passed to the flags.

    \sa hasNext(), next(), IteratorFlags
*/
QDirIterator::QDirIterator(const QDir &dir, IteratorFlags flags)
{
   // little trick to get hold of the QDirPrivate while there is no API on QDir to give it to us
   class MyQDir : public QDir
   {
    public:
      const QDirPrivate *priv() const {
         return d_ptr.constData();
      }
   };
   const QDirPrivate *other = static_cast<const MyQDir *>(&dir)->priv();
   d.reset(new QDirIteratorPrivate(other->dirEntry, other->nameFilters, other->filters, flags,
                                   !other->fileEngine.isNull()));
}

/*!
    Constructs a QDirIterator that can iterate over \a path, with no name
    filtering and \a filters for entry filtering. You can pass options via \a
    flags to decide how the directory should be iterated.

    By default, \a filters is QDir::NoFilter, and \a flags is NoIteratorFlags,
    which provides the same behavior as in QDir::entryList().

    \note To list symlinks that point to non existing files, QDir::System must be
     passed to the flags.

    \sa hasNext(), next(), IteratorFlags
*/
QDirIterator::QDirIterator(const QString &path, QDir::Filters filters, IteratorFlags flags)
   : d(new QDirIteratorPrivate(QFileSystemEntry(path), QStringList(), filters, flags))
{
}

/*!
    Constructs a QDirIterator that can iterate over \a path. You can pass
    options via \a flags to decide how the directory should be iterated.

    By default, \a flags is NoIteratorFlags, which provides the same behavior
    as in QDir::entryList().

    \note To list symlinks that point to non existing files, QDir::System must be
     passed to the flags.

    \sa hasNext(), next(), IteratorFlags
*/
QDirIterator::QDirIterator(const QString &path, IteratorFlags flags)
   : d(new QDirIteratorPrivate(QFileSystemEntry(path), QStringList(), QDir::NoFilter, flags))
{
}

/*!
    Constructs a QDirIterator that can iterate over \a path, using \a
    nameFilters and \a filters. You can pass options via \a flags to decide
    how the directory should be iterated.

    By default, \a flags is NoIteratorFlags, which provides the same behavior
    as QDir::entryList().

    \note To list symlinks that point to non existing files, QDir::System must be
     passed to the flags.

    \sa hasNext(), next(), IteratorFlags
*/
QDirIterator::QDirIterator(const QString &path, const QStringList &nameFilters,
                           QDir::Filters filters, IteratorFlags flags)
   : d(new QDirIteratorPrivate(QFileSystemEntry(path), nameFilters, filters, flags))
{
}

/*!
    Destroys the QDirIterator.
*/
QDirIterator::~QDirIterator()
{
}

/*!
    Advances the iterator to the next entry, and returns the file path of this
    new entry. If hasNext() returns false, this function does nothing, and
    returns a null QString.

    You can call fileName() or filePath() to get the current entry file name
    or path, or fileInfo() to get a QFileInfo for the current entry.

    \sa hasNext(), fileName(), filePath(), fileInfo()
*/
QString QDirIterator::next()
{
   d->advance();
   return filePath();
}

/*!
    Returns true if there is at least one more entry in the directory;
    otherwise, false is returned.

    \sa next(), fileName(), filePath(), fileInfo()
*/
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

/*!
    Returns the file name for the current directory entry, without the path
    prepended.

    This function is convenient when iterating a single directory. When using
    the QDirIterator::Subdirectories flag, you can use filePath() to get the
    full path.

    \sa filePath(), fileInfo()
*/
QString QDirIterator::fileName() const
{
   return d->currentFileInfo.fileName();
}

/*!
    Returns the full file path for the current directory entry.

    \sa fileInfo(), fileName()
*/
QString QDirIterator::filePath() const
{
   return d->currentFileInfo.filePath();
}

/*!
    Returns a QFileInfo for the current directory entry.

    \sa filePath(), fileName()
*/
QFileInfo QDirIterator::fileInfo() const
{
   return d->currentFileInfo;
}

/*!
    Returns the base directory of the iterator.
*/
QString QDirIterator::path() const
{
   return d->dirEntry.filePath();
}

QT_END_NAMESPACE
