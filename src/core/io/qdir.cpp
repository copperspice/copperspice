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

#include <algorithm>

#include <qplatformdefs.h>
#include <qdir.h>
#include <qdir_p.h>
#include <qabstractfileengine.h>

#include <qdebug.h>
#include <qdiriterator.h>
#include <qfsfileengine.h>
#include <qdatetime.h>
#include <qregularexpression.h>
#include <qstring.h>
#include <qvector.h>
#include <qvarlengtharray.h>
#include <qfilesystementry_p.h>
#include <qfilesystemmetadata_p.h>
#include <qfilesystemengine_p.h>

#ifdef QT_BUILD_CORE_LIB
#  include <qresource.h>
#  include <qcoreglobaldata_p.h>
#endif

#include <stdlib.h>

static QString driveSpec(const QString &path)
{
#if defined(Q_OS_WIN)
   if (path.size() < 2) {
      return QString();
   }
   char c = path.at(0).toLatin1();
   if (c < 'a' && c > 'z' && c < 'A' && c > 'Z') {
      return QString();
   }

   if (path.at(1).toLatin1() != ':') {
      return QString();
   }
   return path.mid(0, 2);

#else
   Q_UNUSED(path);
   return QString();
#endif
}

QDirPrivate::QDirPrivate(const QString &path, const QStringList &nameFilters_, QDir::SortFlags sort_,
                         QDir::Filters filters_)
   : QSharedData(), nameFilters(nameFilters_), sort(sort_), filters(filters_), fileListsInitialized(false)
{
   setPath(path.isEmpty() ? QString(".") : path);

   bool empty = nameFilters.isEmpty();

   if (! empty) {
      empty = true;

      for (int i = 0; i < nameFilters.size(); ++i) {
         if (! nameFilters.at(i).isEmpty()) {
            empty = false;
            break;
         }
      }
   }

   if (empty) {
      nameFilters = QStringList("*");
   }
}

QDirPrivate::QDirPrivate(const QDirPrivate &copy)
   : QSharedData(copy)
   , nameFilters(copy.nameFilters)
   , sort(copy.sort)
   , filters(copy.filters)
   , fileListsInitialized(false)
   , dirEntry(copy.dirEntry)
   , metaData(copy.metaData)
{
}

bool QDirPrivate::exists() const
{
   if (fileEngine.isNull()) {
      QFileSystemEngine::fillMetaData(dirEntry, metaData,
                                      QFileSystemMetaData::ExistsAttribute | QFileSystemMetaData::DirectoryType); // always stat

      return metaData.exists() && metaData.isDirectory();
   }

   const QAbstractFileEngine::FileFlags info =
      fileEngine->fileFlags(QAbstractFileEngine::DirectoryType
                            | QAbstractFileEngine::ExistsFlag
                            | QAbstractFileEngine::Refresh);

   if (! (info & QAbstractFileEngine::DirectoryType)) {
      return false;
   }

   return info & QAbstractFileEngine::ExistsFlag;
}

// static
inline QChar QDirPrivate::getFilterSepChar(const QString &nameFilter)
{
   QChar sep(';');

   int i = nameFilter.indexOf(sep, 0);

   if (i == -1 && nameFilter.indexOf(' ', 0) != -1) {
      sep = ' ';
   }

   return sep;
}

// static
inline QStringList QDirPrivate::splitFilters(const QString &nameFilter, QChar sep)
{
   if (sep == 0) {
      sep = getFilterSepChar(nameFilter);
   }

   QStringList ret = nameFilter.split(sep);

   for (int i = 0; i < ret.count(); ++i) {
      ret[i] = ret[i].trimmed();
   }

   return ret;
}

inline void QDirPrivate::setPath(const QString &path)
{
   QString p = QDir::fromNativeSeparators(path);
   if (p.endsWith('/') && p.length() > 1

#if defined(Q_OS_WIN)
         && (!(p.length() == 3 && p.at(1).unicode() == ':' && p.at(0).isLetter()))
#endif
      ) {

      p.truncate(p.length() - 1);
   }

   dirEntry = QFileSystemEntry(p, QFileSystemEntry::FromInternalPath());
   metaData.clear();
   initFileEngine();
   clearFileLists();
   absoluteDirEntry = QFileSystemEntry();
}

inline void QDirPrivate::clearFileLists()
{
   fileListsInitialized = false;
   files.clear();
   fileInfos.clear();
}

inline void QDirPrivate::resolveAbsoluteEntry() const
{
   if (!absoluteDirEntry.isEmpty() || dirEntry.isEmpty()) {
      return;
   }

   QString absoluteName;
   if (fileEngine.isNull()) {
      if (!dirEntry.isRelative() && dirEntry.isClean()) {
         absoluteDirEntry = dirEntry;
         return;
      }

      absoluteName = QFileSystemEngine::absoluteName(dirEntry).filePath();
   } else {
      absoluteName = fileEngine->fileName(QAbstractFileEngine::AbsoluteName);
   }

   absoluteDirEntry = QFileSystemEntry(QDir::cleanPath(absoluteName), QFileSystemEntry::FromInternalPath());
}

/* For sorting */
struct QDirSortItem {
   mutable QString filename_cache;
   mutable QString suffix_cache;
   QFileInfo item;
};


class QDirSortItemComparator
{
   int qt_cmp_si_sort_flags;
 public:
   QDirSortItemComparator(int flags) : qt_cmp_si_sort_flags(flags) {}
   bool operator()(const QDirSortItem &, const QDirSortItem &);
};

bool QDirSortItemComparator::operator()(const QDirSortItem &n1, const QDirSortItem &n2)
{
   const QDirSortItem *f1 = &n1;
   const QDirSortItem *f2 = &n2;

   if ((qt_cmp_si_sort_flags & QDir::DirsFirst) && (f1->item.isDir() != f2->item.isDir())) {
      return f1->item.isDir();
   }

   if ((qt_cmp_si_sort_flags & QDir::DirsLast) && (f1->item.isDir() != f2->item.isDir())) {
      return !f1->item.isDir();
   }

   int r = 0;
   int sortBy = (qt_cmp_si_sort_flags & QDir::SortByMask) | (qt_cmp_si_sort_flags & QDir::Type);

   switch (sortBy) {
      case QDir::Time:
         r = f1->item.lastModified().secsTo(f2->item.lastModified());
         break;

      case QDir::Size:
         r = int(qBound(-1, f2->item.size() - f1->item.size(), 1));
         break;

      case QDir::Type: {
         bool ic = qt_cmp_si_sort_flags & QDir::IgnoreCase;

         if (f1->suffix_cache.isEmpty())
            f1->suffix_cache = ic ? f1->item.suffix().toLower() : f1->item.suffix();

         if (f2->suffix_cache.isEmpty())
            f2->suffix_cache = ic ? f2->item.suffix().toLower() : f2->item.suffix();

         r = qt_cmp_si_sort_flags & QDir::LocaleAware
             ? f1->suffix_cache.localeAwareCompare(f2->suffix_cache) : f1->suffix_cache.compare(f2->suffix_cache);
      }
      break;

      default:
         ;
   }

   if (r == 0 && sortBy != QDir::Unsorted) {
      // Still not sorted - sort by name
      bool ic = qt_cmp_si_sort_flags & QDir::IgnoreCase;

      if (f1->filename_cache.isEmpty())
         f1->filename_cache = ic ? f1->item.fileName().toLower() : f1->item.fileName();

      if (f2->filename_cache.isEmpty())
         f2->filename_cache = ic ? f2->item.fileName().toLower() : f2->item.fileName();

      r = qt_cmp_si_sort_flags & QDir::LocaleAware
          ? f1->filename_cache.localeAwareCompare(f2->filename_cache) : f1->filename_cache.compare(f2->filename_cache);
   }

   if (r == 0) { // Enforce an order - the order the items appear in the array
      r = (&n1) - (&n2);
   }

   if (qt_cmp_si_sort_flags & QDir::Reversed) {
      return r > 0;
   }

   return r < 0;
}

inline void QDirPrivate::sortFileList(QDir::SortFlags sort, QFileInfoList &l, QStringList *names, QFileInfoList *infos)
{
   // names and infos are always empty lists or 0 here
   int n = l.size();
   if (n > 0) {

      if (n == 1 || (sort & QDir::SortByMask) == QDir::Unsorted) {
         if (infos) {
            *infos = l;
         }

         if (names) {
            for (int i = 0; i < n; ++i) {
               names->append(l.at(i).fileName());
            }
         }

      } else {
         QScopedArrayPointer<QDirSortItem> si(new QDirSortItem[n]);
         for (int i = 0; i < n; ++i) {
            si[i].item = l.at(i);
         }
         std::sort(si.data(), si.data() + n, QDirSortItemComparator(sort));

         // put them back in the list(s)
         if (infos) {
            for (int i = 0; i < n; ++i) {
               infos->append(si[i].item);
            }
         }
         if (names) {
            for (int i = 0; i < n; ++i) {
               names->append(si[i].item.fileName());
            }
         }
      }
   }
}
inline void QDirPrivate::initFileLists(const QDir &dir) const
{
   if (!fileListsInitialized) {
      QFileInfoList l;
      QDirIterator it(dir);
      while (it.hasNext()) {
         it.next();
         l.append(it.fileInfo());
      }
      sortFileList(sort, l, &files, &fileInfos);
      fileListsInitialized = true;
   }
}

inline void QDirPrivate::initFileEngine()
{
   fileEngine.reset(QFileSystemEngine::resolveEntryAndCreateLegacyEngine(dirEntry, metaData));
}

QDir::QDir(const QString &path) : d_ptr(new QDirPrivate(path))
{
}

QDir::QDir(const QString &path, const QString &nameFilter, SortFlags sort, Filters filters)
   : d_ptr(new QDirPrivate(path, QDir::nameFiltersFromString(nameFilter), sort, filters))
{
}

QDir::QDir(const QDir &dir)
   : d_ptr(dir.d_ptr)
{
}

QDir::~QDir()
{
}

void QDir::setPath(const QString &path)
{
   d_ptr->setPath(path);
}

/*!
    Returns the path. This may contain symbolic links, but never
    contains redundant ".", ".." or multiple separators.

    The returned path can be either absolute or relative (see
    setPath()).

    \sa setPath(), absolutePath(), exists(), cleanPath(), dirName(),
    absoluteFilePath(), toNativeSeparators(), makeAbsolute()
*/
QString QDir::path() const
{
   const QDirPrivate *d = d_ptr.constData();
   return d->dirEntry.filePath();
}

/*!
    Returns the absolute path (a path that starts with "/" or with a
    drive specification), which may contain symbolic links, but never
    contains redundant ".", ".." or multiple separators.

    \sa setPath(), canonicalPath(), exists(), cleanPath(),
    dirName(), absoluteFilePath()
*/
QString QDir::absolutePath() const
{
   const QDirPrivate *d = d_ptr.constData();
   d->resolveAbsoluteEntry();
   return d->absoluteDirEntry.filePath();
}

/*!
    Returns the canonical path, i.e. a path without symbolic links or
    redundant "." or ".." elements.

    On systems that do not have symbolic links this function will
    always return the same string that absolutePath() returns. If the
    canonical path does not exist (normally due to dangling symbolic
    links) canonicalPath() returns an empty string.

    Example:

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 6

    \sa path(), absolutePath(), exists(), cleanPath(), dirName(),
        absoluteFilePath()
*/
QString QDir::canonicalPath() const
{
   const QDirPrivate *d = d_ptr.constData();
   if (d->fileEngine.isNull()) {
      QFileSystemEntry answer = QFileSystemEngine::canonicalName(d->dirEntry, d->metaData);
      return answer.filePath();
   }
   return d->fileEngine->fileName(QAbstractFileEngine::CanonicalName);
}

/*!
    Returns the name of the directory; this is \e not the same as the
    path, e.g. a directory with the name "mail", might have the path
    "/var/spool/mail". If the directory has no name (e.g. it is the
    root directory) an empty string is returned.

    No check is made to ensure that a directory with this name
    actually exists; but see exists().

    \sa path(), filePath(), absolutePath(), absoluteFilePath()
*/
QString QDir::dirName() const
{
   const QDirPrivate *d = d_ptr.constData();
   return d->dirEntry.fileName();
}

/*!
    Returns the path name of a file in the directory. Does \e not
    check if the file actually exists in the directory; but see
    exists(). If the QDir is relative the returned path name will also
    be relative. Redundant multiple separators or "." and ".."
    directories in \a fileName are not removed (see cleanPath()).

    \sa dirName() absoluteFilePath(), isRelative(), canonicalPath()
*/
QString QDir::filePath(const QString &fileName) const
{
   const QDirPrivate *d = d_ptr.constData();

   if (isAbsolutePath(fileName)) {
      return QString(fileName);
   }

   QString ret = d->dirEntry.filePath();
   if (! fileName.isEmpty()) {

      if (! ret.isEmpty() && ret[(int)ret.length() - 1] != '/' && fileName[0] != '/') {
         ret += '/';
      }
      ret += fileName;
   }

   return ret;
}

/*!
    Returns the absolute path name of a file in the directory. Does \e
    not check if the file actually exists in the directory; but see
    exists(). Redundant multiple separators or "." and ".."
    directories in \a fileName are not removed (see cleanPath()).

    \sa relativeFilePath() filePath() canonicalPath()
*/
QString QDir::absoluteFilePath(const QString &fileName) const
{
   const QDirPrivate *d = d_ptr.constData();
   if (isAbsolutePath(fileName)) {
      return fileName;
   }

   d->resolveAbsoluteEntry();

   if (fileName.isEmpty()) {
      return d->absoluteDirEntry.filePath();
   }

   if (!d->absoluteDirEntry.isRoot()) {
      return d->absoluteDirEntry.filePath() + '/' + fileName;
   }

   return d->absoluteDirEntry.filePath() + fileName;
}

QString QDir::relativeFilePath(const QString &fileName) const
{
   QString dir = cleanPath(absolutePath());
   QString file = cleanPath(fileName);

   if (isRelativePath(file) || isRelativePath(dir)) {
      return file;
   }

   QString dirDrive = driveSpec(dir);
   QString fileDrive = driveSpec(file);

   bool fileDriveMissing = false;

   if (fileDrive.isEmpty()) {
      fileDrive = dirDrive;
      fileDriveMissing = true;
   }

#ifdef Q_OS_WIN
   if (fileDrive.toLower() != dirDrive.toLower() || (file.startsWith("//") && ! dir.startsWith("//"))) {

#else
   if (fileDrive != dirDrive) {

#endif
      return file;
   }

   dir.remove(0, dirDrive.size());
   if (! fileDriveMissing) {
      file.remove(0, fileDrive.size());
   }

   QString result;
   QStringList dirElts = dir.split('/', QStringParser::SkipEmptyParts);
   QStringList fileElts = file.split('/', QStringParser::SkipEmptyParts);

   int i = 0;
   while (i < dirElts.size() && i < fileElts.size() &&

#if defined(Q_OS_WIN)
          dirElts.at(i).toLower() == fileElts.at(i).toLower()) {
#else
          dirElts.at(i) == fileElts.at(i)) {
#endif
      ++i;
   }

   for (int j = 0; j < dirElts.size() - i; ++j) {
      result += "../";
   }

   for (int j = i; j < fileElts.size(); ++j) {
      result += fileElts.at(j);

      if (j < fileElts.size() - 1) {
         result += '/';
      }
   }

   return result;
}

QString QDir::toNativeSeparators(const QString &pathName)
{

#if defined(Q_OS_WIN)
   QString retval(pathName);
   retval.replace('/', '\\');

   return retval;
#else
   return pathName;

#endif

}


QString QDir::fromNativeSeparators(const QString &pathName)
{
#if defined(Q_OS_WIN)
   QString retval(pathName);
   retval.replace('\\', '/');

   return retval;
#else
   return pathName;

#endif
}

bool QDir::cd(const QString &dirName)
{
   const QDirPrivate *const d = d_ptr.constData();

   if (dirName.isEmpty() || dirName == ".") {
      return true;
   }

   QString newPath;
   if (isAbsolutePath(dirName)) {
      newPath = cleanPath(dirName);

   } else {
      if (isRoot()) {
         if (dirName == "..") {
            return false;
         }
         newPath = d->dirEntry.filePath();

      } else {
         newPath = d->dirEntry.filePath() + '/';
      }

      newPath += dirName;

      if (dirName.indexOf('/') >= 0 || dirName == ".." || d->dirEntry.filePath() == ".") {
         newPath = cleanPath(newPath);

         if (newPath.startsWith("..")) {
            newPath = QFileInfo(newPath).absoluteFilePath();
         }
      }
   }

   QScopedPointer<QDirPrivate> dir(new QDirPrivate(*d_ptr.constData()));
   dir->setPath(newPath);

   if (!dir->exists()) {
      return false;
   }

   d_ptr = dir.take();

   return true;
}

bool QDir::cdUp()
{
   return cd("..");
}

QStringList QDir::nameFilters() const
{
   const QDirPrivate *d = d_ptr.constData();
   return d->nameFilters;
}


void QDir::setNameFilters(const QStringList &nameFilters)
{
   QDirPrivate *d = d_ptr.data();
   d->initFileEngine();
   d->clearFileLists();

   d->nameFilters = nameFilters;
}

// obsolete
void QDir::addResourceSearchPath(const QString &path)
{
#ifdef QT_BUILD_CORE_LIB
   QResource::addSearchPath(path);
#else
   Q_UNUSED(path)
#endif
}

#ifdef QT_BUILD_CORE_LIB

void QDir::setSearchPaths(const QString &prefix, const QStringList &searchPaths)
{
   if (prefix.length() < 2) {
      qWarning("QDir::setSearchPaths: Prefix must be longer than 1 character");
      return;
   }

   for (int i = 0; i < prefix.count(); ++i) {
      if (!prefix.at(i).isLetterOrNumber()) {
         qWarning("QDir::setSearchPaths: Prefix can only contain letters or numbers");
         return;
      }
   }

   QWriteLocker lock(&QCoreGlobalData::instance()->dirSearchPathsLock);
   QMap<QString, QStringList> &paths = QCoreGlobalData::instance()->dirSearchPaths;
   if (searchPaths.isEmpty()) {
      paths.remove(prefix);
   } else {
      paths.insert(prefix, searchPaths);
   }
}

/*!
    \since 4.3

    Adds \a path to the search path for \a prefix.

    \sa setSearchPaths()
*/
void QDir::addSearchPath(const QString &prefix, const QString &path)
{
   if (path.isEmpty()) {
      return;
   }

   QWriteLocker lock(&QCoreGlobalData::instance()->dirSearchPathsLock);
   QCoreGlobalData::instance()->dirSearchPaths[prefix] += path;
}

/*!
    \since 4.3

    Returns the search paths for \a prefix.

    \sa setSearchPaths(), addSearchPath()
*/
QStringList QDir::searchPaths(const QString &prefix)
{
   QReadLocker lock(&QCoreGlobalData::instance()->dirSearchPathsLock);
   return QCoreGlobalData::instance()->dirSearchPaths.value(prefix);
}

#endif // QT_BUILD_CORE_LIB

/*!
    Returns the value set by setFilter()
*/
QDir::Filters QDir::filter() const
{
   const QDirPrivate *d = d_ptr.constData();
   return d->filters;
}

/*!
    \enum QDir::Filter

    This enum describes the filtering options available to QDir; e.g.
    for entryList() and entryInfoList(). The filter value is specified
    by combining values from the following list using the bitwise OR
    operator:

    \value Dirs    List directories that match the filters.
    \value AllDirs  List all directories; i.e. don't apply the filters
                    to directory names.
    \value Files   List files.
    \value Drives  List disk drives (ignored under Unix).
    \value NoSymLinks  Do not list symbolic links (ignored by operating
                       systems that don't support symbolic links).
    \value NoDotAndDotDot Do not list the special entries "." and "..".
    \value NoDot       Do not list the special entry ".".
    \value NoDotDot    Do not list the special entry "..".
    \value AllEntries  List directories, files, drives and symlinks (this does not list
                broken symlinks unless you specify System).
    \value Readable    List files for which the application has read
                       access. The Readable value needs to be combined
                       with Dirs or Files.
    \value Writable    List files for which the application has write
                       access. The Writable value needs to be combined
                       with Dirs or Files.
    \value Executable  List files for which the application has
                       execute access. The Executable value needs to be
                       combined with Dirs or Files.
    \value Modified  Only list files that have been modified (ignored
                     on Unix).
    \value Hidden  List hidden files (on Unix, files starting with a ".").
    \value System  List system files (on Unix, FIFOs, sockets and
                   device files are included; on Windows, \c {.lnk}
                   files are included)
    \value CaseSensitive  The filter should be case sensitive.

    \omitvalue DefaultFilter
    \omitvalue TypeMask
    \omitvalue All
    \omitvalue RWEMask
    \omitvalue AccessMask
    \omitvalue PermissionMask
    \omitvalue NoFilter

    Functions that use Filter enum values to filter lists of files
    and directories will include symbolic links to files and directories
    unless you set the NoSymLinks value.

    A default constructed QDir will not filter out files based on
    their permissions, so entryList() and entryInfoList() will return
    all files that are readable, writable, executable, or any
    combination of the three.  This makes the default easy to write,
    and at the same time useful.

    For example, setting the \c Readable, \c Writable, and \c Files
    flags allows all files to be listed for which the application has read
    access, write access or both. If the \c Dirs and \c Drives flags are
    also included in this combination then all drives, directories, all
    files that the application can read, write, or execute, and symlinks
    to such files/directories can be listed.

    To retrieve the permissons for a directory, use the
    entryInfoList() function to get the associated QFileInfo objects
    and then use the QFileInfo::permissons() to obtain the permissions
    and ownership for each file.
*/

/*!
    Sets the filter used by entryList() and entryInfoList() to \a
    filters. The filter is used to specify the kind of files that
    should be returned by entryList() and entryInfoList(). See
    \l{QDir::Filter}.

    \sa filter(), setNameFilters()
*/
void QDir::setFilter(Filters filters)
{
   QDirPrivate *d = d_ptr.data();
   d->initFileEngine();
   d->clearFileLists();

   d->filters = filters;
}

/*!
    Returns the value set by setSorting()

    \sa setSorting() SortFlag
*/
QDir::SortFlags QDir::sorting() const
{
   const QDirPrivate *d = d_ptr.constData();
   return d->sort;
}

/*!
    \enum QDir::SortFlag

    This enum describes the sort options available to QDir, e.g. for
    entryList() and entryInfoList(). The sort value is specified by
    OR-ing together values from the following list:

    \value Name  Sort by name.
    \value Time  Sort by time (modification time).
    \value Size  Sort by file size.
    \value Type  Sort by file type (extension).
    \value Unsorted  Do not sort.
    \value NoSort Not sorted by default.

    \value DirsFirst  Put the directories first, then the files.
    \value DirsLast Put the files first, then the directories.
    \value Reversed  Reverse the sort order.
    \value IgnoreCase  Sort case-insensitively.
    \value LocaleAware Sort items appropriately using the current locale settings.

    \omitvalue SortByMask
    \omitvalue DefaultSort

    You can only specify one of the first four.

    If you specify both DirsFirst and Reversed, directories are
    still put first, but in reverse order; the files will be listed
    after the directories, again in reverse order.
*/

/*!
    Sets the sort order used by entryList() and entryInfoList().

    The \a sort is specified by OR-ing values from the enum
    \l{QDir::SortFlag}.

    \sa sorting() SortFlag
*/
void QDir::setSorting(SortFlags sort)
{
   QDirPrivate *d = d_ptr.data();
   d->initFileEngine();
   d->clearFileLists();

   d->sort = sort;
}

uint QDir::count() const
{
   const QDirPrivate *d = d_ptr.constData();
   d->initFileLists(*this);

   return d->files.count();
}

QString QDir::operator[](int pos) const
{
   const QDirPrivate *d = d_ptr.constData();
   d->initFileLists(*this);

   return d->files[pos];
}

QStringList QDir::entryList(Filters filters, SortFlags sort) const
{
   const QDirPrivate *d = d_ptr.constData();
   return entryList(d->nameFilters, filters, sort);
}

QFileInfoList QDir::entryInfoList(Filters filters, SortFlags sort) const
{
   const QDirPrivate *d = d_ptr.constData();
   return entryInfoList(d->nameFilters, filters, sort);
}

QStringList QDir::entryList(const QStringList &nameFilters, Filters filters, SortFlags sort) const
{
   const QDirPrivate *d = d_ptr.constData();

   if (filters == NoFilter) {
      filters = d->filters;
   }

   if (sort == NoSort) {
      sort = d->sort;
   }

   if (filters == d->filters && sort == d->sort && nameFilters == d->nameFilters) {
      d->initFileLists(*this);
      return d->files;
   }

   QFileInfoList list;
   QDirIterator it(d->dirEntry.filePath(), nameFilters, filters);

   while (it.hasNext()) {
      it.next();
      list.append(it.fileInfo());
   }

   QStringList ret;
   d->sortFileList(sort, list, &ret, 0);

   return ret;
}

/*!
    Returns a list of QFileInfo objects for all the files and
    directories in the directory, ordered according to the name
    and attribute filters previously set with setNameFilters()
    and setFilter(), and sorted according to the flags set with
    setSorting().

    The name filter, file attribute filter, and sorting specification
    can be overridden using the \a nameFilters, \a filters, and \a sort
    arguments.

    Returns an empty list if the directory is unreadable, does not
    exist, or if nothing matches the specification.

    \sa entryList(), setNameFilters(), setSorting(), setFilter(), isReadable(), exists()
*/
QFileInfoList QDir::entryInfoList(const QStringList &nameFilters, Filters filters,
                                  SortFlags sort) const
{
   const QDirPrivate *d = d_ptr.constData();

   if (filters == NoFilter) {
      filters = d->filters;
   }

   if (sort == NoSort) {
      sort = d->sort;
   }

   if (filters == d->filters && sort == d->sort && nameFilters == d->nameFilters) {
      d->initFileLists(*this);
      return d->fileInfos;
   }

   QFileInfoList l;
   QDirIterator it(d->dirEntry.filePath(), nameFilters, filters);
   while (it.hasNext()) {
      it.next();
      l.append(it.fileInfo());
   }
   QFileInfoList ret;
   d->sortFileList(sort, l, 0, &ret);
   return ret;
}

/*!
    Creates a sub-directory called \a dirName.

    Returns true on success; otherwise returns false.

    If the directory already exists when this function is called, it will return false.

    \sa rmdir()
*/
// ### Qt5/behaviour when directory already exists should be made consistent for mkdir and mkpath
bool QDir::mkdir(const QString &dirName) const
{
   const QDirPrivate *d = d_ptr.constData();

   if (dirName.isEmpty()) {
      qWarning("QDir::mkdir: Empty or null file name(s)");
      return false;
   }

   QString fn = filePath(dirName);
   if (d->fileEngine.isNull()) {
      return QFileSystemEngine::createDirectory(QFileSystemEntry(fn), false);
   }
   return d->fileEngine->mkdir(fn, false);
}

/*!
    Removes the directory specified by \a dirName.

    The directory must be empty for rmdir() to succeed.

    Returns true if successful; otherwise returns false.

    \sa mkdir()
*/
bool QDir::rmdir(const QString &dirName) const
{
   const QDirPrivate *d = d_ptr.constData();

   if (dirName.isEmpty()) {
      qWarning("QDir::rmdir: Empty or null file name(s)");
      return false;
   }

   QString fn = filePath(dirName);
   if (d->fileEngine.isNull()) {
      return QFileSystemEngine::removeDirectory(QFileSystemEntry(fn), false);
   }

   return d->fileEngine->rmdir(fn, false);
}

/*!
    Creates the directory path \a dirPath.

    The function will create all parent directories necessary to
    create the directory.

    Returns true if successful; otherwise returns false.

    If the path already exists when this function is called, it will return true.

    \sa rmpath()
*/
// ### Qt5/behaviour when directory already exists should be made consistent for mkdir and mkpath
bool QDir::mkpath(const QString &dirPath) const
{
   const QDirPrivate *d = d_ptr.constData();

   if (dirPath.isEmpty()) {
      qWarning("QDir::mkpath: Empty or null file name(s)");
      return false;
   }

   QString fn = filePath(dirPath);
   if (d->fileEngine.isNull()) {
      return QFileSystemEngine::createDirectory(QFileSystemEntry(fn), true);
   }
   return d->fileEngine->mkdir(fn, true);
}

/*!
    Removes the directory path \a dirPath.

    The function will remove all parent directories in \a dirPath,
    provided that they are empty. This is the opposite of
    mkpath(dirPath).

    Returns true if successful; otherwise returns false.

    \sa mkpath()
*/
bool QDir::rmpath(const QString &dirPath) const
{
   const QDirPrivate *d = d_ptr.constData();

   if (dirPath.isEmpty()) {
      qWarning("QDir::rmpath: Empty or null file name(s)");
      return false;
   }

   QString fn = filePath(dirPath);
   if (d->fileEngine.isNull()) {
      return QFileSystemEngine::removeDirectory(QFileSystemEntry(fn), true);
   }
   return d->fileEngine->rmdir(fn, true);
}

/*!
    Removes the directory, including all its contents.

    Returns true if successful, otherwise false.

    If a file or directory cannot be removed, removeRecursively() keeps going
    and attempts to delete as many files and sub-directories as possible,
    then returns false.

    If the directory was already removed, the method returns true
    (expected result already reached).

    Note: this function is meant for removing a small application-internal
    directory (such as a temporary directory), but not user-visible
    directories. For user-visible operations, it is rather recommended
    to report errors more precisely to the user, to offer solutions
    in case of errors, to show progress during the deletion since it
    could take several minutes, etc.
*/
bool QDir::removeRecursively()
{
   if (!d_ptr->exists()) {
      return true;
   }

   bool success = true;
   const QString dirPath = path();

   // not empty -- we must empty it first
   QDirIterator di(dirPath, QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

   while (di.hasNext()) {
      di.next();
      const QFileInfo &fi = di.fileInfo();
      bool ok;
      if (fi.isDir() && !fi.isSymLink()) {
         ok = QDir(di.filePath()).removeRecursively();   // recursive
      } else {
         ok = QFile::remove(di.filePath());
      }
      if (!ok) {
         success = false;
      }
   }

   if (success) {
      success = rmdir(absolutePath());
   }

   return success;
}

/*!
    Returns true if the directory is readable \e and we can open files
    by name; otherwise returns false.

    \warning A false value from this function is not a guarantee that
    files in the directory are not accessible.

    \sa QFileInfo::isReadable()
*/
bool QDir::isReadable() const
{
   const QDirPrivate *d = d_ptr.constData();

   if (d->fileEngine.isNull()) {
      if (!d->metaData.hasFlags(QFileSystemMetaData::UserReadPermission)) {
         QFileSystemEngine::fillMetaData(d->dirEntry, d->metaData, QFileSystemMetaData::UserReadPermission);
      }

      return (d->metaData.permissions() & QFile::ReadUser) != 0;
   }

   const QAbstractFileEngine::FileFlags info =
      d->fileEngine->fileFlags(QAbstractFileEngine::DirectoryType
                               | QAbstractFileEngine::PermsMask);
   if (!(info & QAbstractFileEngine::DirectoryType)) {
      return false;
   }
   return info & QAbstractFileEngine::ReadUserPerm;
}

/*!
    \overload

    Returns true if the directory exists; otherwise returns false.
    (If a file with the same name is found this function will return false).

    The overload of this function that accepts an argument is used to test
    for the presence of files and directories within a directory.

    \sa QFileInfo::exists(), QFile::exists()
*/
bool QDir::exists() const
{
   return d_ptr->exists();
}

/*!
    Returns true if the directory is the root directory; otherwise
    returns false.

    Note: If the directory is a symbolic link to the root directory
    this function returns false. If you want to test for this use
    canonicalPath(), e.g.

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 9

    \sa root(), rootPath()
*/
bool QDir::isRoot() const
{
   if (d_ptr->fileEngine.isNull()) {
      return d_ptr->dirEntry.isRoot();
   }
   return d_ptr->fileEngine->fileFlags(QAbstractFileEngine::FlagsMask) & QAbstractFileEngine::RootFlag;
}

/*!
    \fn bool QDir::isAbsolute() const

    Returns true if the directory's path is absolute; otherwise
    returns false. See isAbsolutePath().

    \sa isRelative() makeAbsolute() cleanPath()
*/

/*!
   \fn bool QDir::isAbsolutePath(const QString &)

    Returns true if \a path is absolute; returns false if it is
    relative.

    \sa isAbsolute() isRelativePath() makeAbsolute() cleanPath()
*/

/*!
    Returns true if the directory path is relative; otherwise returns
    false. (Under Unix a path is relative if it does not start with a
    "/").

    \sa makeAbsolute() isAbsolute() isAbsolutePath() cleanPath()
*/
bool QDir::isRelative() const
{
   if (d_ptr->fileEngine.isNull()) {
      return d_ptr->dirEntry.isRelative();
   }
   return d_ptr->fileEngine->isRelativePath();
}


/*!
    Converts the directory path to an absolute path. If it is already
    absolute nothing happens. Returns true if the conversion
    succeeded; otherwise returns false.

    \sa isAbsolute() isAbsolutePath() isRelative() cleanPath()
*/
bool QDir::makeAbsolute()
{
   const QDirPrivate *d = d_ptr.constData();
   QScopedPointer<QDirPrivate> dir;
   if (!d->fileEngine.isNull()) {
      QString absolutePath = d->fileEngine->fileName(QAbstractFileEngine::AbsoluteName);
      if (QDir::isRelativePath(absolutePath)) {
         return false;
      }

      dir.reset(new QDirPrivate(*d_ptr.constData()));
      dir->setPath(absolutePath);
   } else { // native FS
      d->resolveAbsoluteEntry();
      dir.reset(new QDirPrivate(*d_ptr.constData()));
      dir->setPath(d->absoluteDirEntry.filePath());
   }
   d_ptr = dir.take(); // actually detach
   return true;
}

/*!
    Returns true if directory \a dir and this directory have the same
    path and their sort and filter settings are the same; otherwise
    returns false.

    Example:

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 10
*/
bool QDir::operator==(const QDir &dir) const
{
   const QDirPrivate *d = d_ptr.constData();
   const QDirPrivate *other = dir.d_ptr.constData();

   if (d == other) {
      return true;
   }
   Qt::CaseSensitivity sensitive;
   if (d->fileEngine.isNull() || other->fileEngine.isNull()) {
      if (d->fileEngine.data() != other->fileEngine.data()) { // one is native, the other is a custom file-engine
         return false;
      }

      sensitive = QFileSystemEngine::isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive;
   } else {
      if (d->fileEngine->caseSensitive() != other->fileEngine->caseSensitive()) {
         return false;
      }
      sensitive = d->fileEngine->caseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive;
   }

   if (d->filters == other->filters
         && d->sort == other->sort
         && d->nameFilters == other->nameFilters) {

      // Fallback to expensive canonical path computation
      return canonicalPath().compare(dir.canonicalPath(), sensitive) == 0;
   }
   return false;
}

/*!
    Makes a copy of the \a dir object and assigns it to this QDir
    object.
*/
QDir &QDir::operator=(const QDir &dir)
{
   d_ptr = dir.d_ptr;
   return *this;
}

/*!
    \overload
    \obsolete

    Sets the directory path to the given \a path.

    Use setPath() instead.
*/
QDir &QDir::operator=(const QString &path)
{
   d_ptr->setPath(path);
   return *this;
}

/*!
    \fn bool QDir::operator!=(const QDir &dir) const

    Returns true if directory \a dir and this directory have different
    paths or different sort or filter settings; otherwise returns
    false.

    Example:

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 11
*/

/*!
    Removes the file, \a fileName.

    Returns true if the file is removed successfully; otherwise
    returns false.
*/
bool QDir::remove(const QString &fileName)
{
   if (fileName.isEmpty()) {
      qWarning("QDir::remove: Empty or null file name");
      return false;
   }
   return QFile::remove(filePath(fileName));
}

/*!
    Renames a file or directory from \a oldName to \a newName, and returns
    true if successful; otherwise returns false.

    On most file systems, rename() fails only if \a oldName does not
    exist, if \a newName and \a oldName are not on the same
    partition or if a file with the new name already exists.
    However, there are also other reasons why rename() can
    fail. For example, on at least one file system rename() fails if
    \a newName points to an open file.
*/
bool QDir::rename(const QString &oldName, const QString &newName)
{
   if (oldName.isEmpty() || newName.isEmpty()) {
      qWarning("QDir::rename: Empty or null file name(s)");
      return false;
   }

   QFile file(filePath(oldName));
   if (!file.exists()) {
      return false;
   }
   return file.rename(filePath(newName));
}

/*!
    Returns true if the file called \a name exists; otherwise returns
    false.

    Unless \a name contains an absolute file path, the file name is assumed
    to be relative to the directory itself, so this function is typically used
    to check for the presence of files within a directory.

    \sa QFileInfo::exists(), QFile::exists()
*/
bool QDir::exists(const QString &name) const
{
   if (name.isEmpty()) {
      qWarning("QDir::exists: Empty or null file name");
      return false;
   }
   return QFile::exists(filePath(name));
}

/*!
    Returns a list of the root directories on this system.

    On Windows this returns a list of QFileInfo objects containing "C:/",
    "D:/", etc. On other operating systems, it returns a list containing
    just one root directory (i.e. "/").

    \sa root(), rootPath()
*/
QFileInfoList QDir::drives()
{
#ifdef QT_NO_FSFILEENGINE
   return QFileInfoList();
#else
   return QFSFileEngine::drives();
#endif
}

QChar QDir::separator()
{
#if defined (Q_OS_WIN)
   return QChar('\\');
#else
   return QChar('/');
#endif
}

bool QDir::setCurrent(const QString &path)
{
   return QFileSystemEngine::setCurrentPath(QFileSystemEntry(path));
}

QString QDir::currentPath()
{
   return QFileSystemEngine::currentPath().filePath();
}

QString QDir::homePath()
{
   return QFileSystemEngine::homePath();
}

QString QDir::homeDirPath()
{
   return homePath();
}

QString QDir::tempPath()
{
   return QFileSystemEngine::tempPath();
}

QString QDir::rootPath()
{
   return QFileSystemEngine::rootPath();
}

bool QDir::match(const QStringList &filters, const QString &fileName)
{
   for (QStringList::const_iterator sit = filters.constBegin(); sit != filters.constEnd(); ++sit) {

      QPatternOptionFlags options = QPatternOption::ExactMatchOption | QPatternOption::WildcardOption
                  | QPatternOption::CaseInsensitiveOption;

      QRegularExpression8 regExp(*sit, options);

      if (regExp.match(fileName).hasMatch()) {
         return true;
      }
   }

   return false;
}

bool QDir::match(const QString &filter, const QString &fileName)
{
   return match(nameFiltersFromString(filter), fileName);
}

QString cs_internal_normalizePath(const QString &name, bool allowUncPaths)
{
   if (name.isEmpty()) {
      return name;
   }

   QString retval;

   auto len    = name.size();
   int up      = 0;
   int prefix  = 0;

   QString::const_iterator iter = name.begin();
   QString::const_iterator end  = name.end() - 1;

   if (allowUncPaths && name.startsWith("//")) {
      // starts with double slash
      prefix = 2;

#ifdef Q_OS_WIN
   } else if (len >= 2 && name[1] == ':') {
      // ignore the drive letter
      prefix = (len > 2 && name[2] == '/') ? 3 : 2;

#endif

   } else if (name.startsWith('/')) {
      prefix = 1;

   }

   iter += prefix;

   if (name.endsWith('/')) {

      if (iter == name.end()) {
         // entire path is the prefix
         return name;
      }

      retval.prepend('/');

      if (iter != end) {
         --end;
      }
   }

   bool done = false;

   while (! done) {
      // remove trailing slashes
      if (*end == '/') {

         if (end == iter) {
            break;
         }

         --end;
         continue;
      }

      // remove current directory
      if (*end == '.' && (end == iter || *(end - 1) == '/')) {

         if (end == iter) {
            break;
         }

         --end;
         continue;
      }

      // detect up directory
      if (end != iter && *end == '.' && *(end - 1) == '.' && (end == iter + 1 || *(end - 2) == '/')) {
         ++up;

         if (end == iter || end - 1 == iter) {
            break;
         }

         end -= 2;
         continue;
      }

      // prepend a slash before copying when not empty
      if (up == 0 && ! retval.isEmpty() && ! retval.startsWith('/')) {
         retval.prepend('/');
      }

      // skip or copy
      while (true) {

         if (*end == '/') {
            // do not copy slashes

            if (end == iter) {
               done = true;
               break;
            }

            --end;
            break;
         }

         if (up == 0) {
            retval.prepend(*end);
         }

         if (end == iter) {
            done = true;
            break;
         }

         --end;
      }

      // decrement after copying/skipping
      if (up != 0) {
         --up;
      }
   }

   // add remaining '..'
   while (up) {

      if (! retval.isEmpty() && ! retval.startsWith('/')) {
         // not empty and there is not already a '/'
         retval.prepend('/');
      }

      retval.prepend("..");
      --up;
   }

   if (prefix > 0) {

      if (! retval.isEmpty() && retval.startsWith('/')) {
         // happens if the input string only consists of a prefix followed by one or more slashes
         retval.erase(retval.begin(), retval.begin() + 1);
      }

      retval.prepend(name.left(prefix));

   } else {

      if (retval.isEmpty()) {
         // after resolving the input path the resulting string is empty (e.g. "foo/..")
         retval.prepend('.');

      } else if (retval.startsWith('/')) {
         // happens whenever all parts are resolved and there is a trailing slash ("./" or "foo/../" for example)
         retval.prepend('.');
      }
   }

   return retval;
}

QString QDir::cleanPath(const QString &path)
{
   if (path.isEmpty()) {
      return path;
   }

   QString name = path;
   QChar dir_separator = separator();

   if (dir_separator != '/') {
      name.replace(dir_separator, '/');
   }

   bool allowUncPaths = false;

#if defined(Q_OS_WIN)
   // allow unc paths
   allowUncPaths = true;
#endif

   QString retval = cs_internal_normalizePath(name, allowUncPaths);

   // Strip away last slash except for root directories
   if (retval.length() > 1 && retval.endsWith('/')) {

#if defined (Q_OS_WIN)
      if (! (retval.length() == 3 && retval.at(1) == ':'))
#endif
         retval.chop(1);
   }

   return retval;
}

bool QDir::isRelativePath(const QString &path)
{
   return QFileInfo(path).isRelative();
}

void QDir::refresh() const
{
   QDirPrivate *d = const_cast<QDir *>(this)->d_ptr.data();
   d->metaData.clear();
   d->initFileEngine();
   d->clearFileLists();
}

QStringList QDir::nameFiltersFromString(const QString &nameFilter)
{
   return QDirPrivate::splitFilters(nameFilter);
}

QDebug operator<<(QDebug debug, QDir::Filters filters)
{
   QStringList flags;
   if (filters == QDir::NoFilter) {
      flags << "NoFilter";

   } else {
      if (filters & QDir::Dirs) {
         flags << "Dirs";
      }
      if (filters & QDir::AllDirs) {
         flags << "AllDirs";
      }
      if (filters & QDir::Files) {
         flags << "Files";
      }
      if (filters & QDir::Drives) {
         flags << "Drives";
      }
      if (filters & QDir::NoSymLinks) {
         flags << "NoSymLinks";
      }
      if (filters & QDir::NoDotAndDotDot) {
         flags << "AndDotDot";   // ### Qt5/remove (because NoDotAndDotDot=NoDot|NoDotDo  if (filters & QDir::NoDot){s << "NoDot";
      }
      if (filters & QDir::NoDotDot) {
         flags << "NoDotDot";
      }
      if ((filters & QDir::AllEntries) == QDir::AllEntries) {
         flags << "AllEntries";
      }
      if (filters & QDir::Readable) {
         flags << "Readable";
      }
      if (filters & QDir::Writable) {
         flags << "Writable";
      }
      if (filters & QDir::Executable) {
         flags << "Executable";
      }
      if (filters & QDir::Modified) {
         flags << "Modified";
      }
      if (filters & QDir::Hidden) {
         flags << "Hidden";
      }
      if (filters & QDir::System) {
         flags << "System";
      }
      if (filters & QDir::CaseSensitive) {
         flags << "CaseSensitive";
      }
   }

   debug << "QDir::Filters(" << csPrintable(flags.join("|")) << ')';
   return debug;
}

static QDebug operator<<(QDebug debug, QDir::SortFlags sorting)
{
   if (sorting == QDir::NoSort) {
      debug << "QDir::SortFlags(NoSort)";

   } else {
      QString type;

      if ((sorting & 3) == QDir::Name) {
         type = "Name";
      }
      if ((sorting & 3) == QDir::Time) {
         type = "Time";
      }
      if ((sorting & 3) == QDir::Size) {
         type = "Size";
      }
      if ((sorting & 3) == QDir::Unsorted) {
         type = "Unsorted";
      }

      QStringList flags;
      if (sorting & QDir::DirsFirst) {
         flags << "DirsFirst";
      }
      if (sorting & QDir::DirsLast) {
         flags << "DirsLast";
      }
      if (sorting & QDir::IgnoreCase) {
         flags << "IgnoreCase";
      }
      if (sorting & QDir::LocaleAware) {
         flags << "LocaleAware";
      }
      if (sorting & QDir::Type) {
         flags << "Type";
      }
      debug << "QDir::SortFlags(" << qPrintable(type)
            << '|'
            << qPrintable(flags.join("|")) << ')';
   }
   return debug;
}

QDebug operator<<(QDebug debug, const QDir &dir)
{
   debug.maybeSpace() << "QDir(" << dir.path()
                      << ", nameFilters = {"
                      << qPrintable(dir.nameFilters().join(","))
                      << "}, "
                      << dir.sorting()
                      << ','
                      << dir.filter()
                      << ')';
   return debug.space();
}

