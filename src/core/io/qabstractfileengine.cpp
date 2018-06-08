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

#include <qabstractfileengine.h>
#include <qabstractfileengine_p.h>

#ifdef QT_BUILD_CORE_LIB
#include <qresource_p.h>
#endif

#include <qdatetime.h>
#include <qreadwritelock.h>
#include <qvariant.h>

// built-in handlers
#include <qfsfileengine.h>
#include <qdiriterator.h>

#include <qfilesystementry_p.h>
#include <qfilesystemmetadata_p.h>
#include <qfilesystemengine_p.h>

QT_BEGIN_NAMESPACE

static bool qt_file_engine_handlers_in_use = false;

/*
    All application-wide handlers are stored in this list. The mutex must be
    acquired to ensure thread safety.
 */
Q_GLOBAL_STATIC_WITH_ARGS(QReadWriteLock, fileEngineHandlerMutex, (QReadWriteLock::Recursive))
static bool qt_abstractfileenginehandlerlist_shutDown = false;
class QAbstractFileEngineHandlerList : public QList<QAbstractFileEngineHandler *>
{
 public:
   ~QAbstractFileEngineHandlerList() {
      QWriteLocker locker(fileEngineHandlerMutex());
      qt_abstractfileenginehandlerlist_shutDown = true;
   }
};
Q_GLOBAL_STATIC(QAbstractFileEngineHandlerList, fileEngineHandlers)

/*!
    Constructs a file handler and registers it. Once created this
    handler's create() function will be called (along with all the other
    handlers) for any paths used. The most recently created handler that
    recognizes the given path (i.e. that returns a QAbstractFileEngine) is
    used for the new path.

    \sa create()
 */
QAbstractFileEngineHandler::QAbstractFileEngineHandler()
{
   QWriteLocker locker(fileEngineHandlerMutex());
   qt_file_engine_handlers_in_use = true;
   fileEngineHandlers()->prepend(this);
}

/*!
    Destroys the file handler. This will automatically unregister the handler
    from Qt.
 */
QAbstractFileEngineHandler::~QAbstractFileEngineHandler()
{
   QWriteLocker locker(fileEngineHandlerMutex());

   // Remove this handler from the handler list only if the list is valid.
   if (!qt_abstractfileenginehandlerlist_shutDown) {
      QAbstractFileEngineHandlerList *handlers = fileEngineHandlers();
      handlers->removeOne(this);
      if (handlers->isEmpty()) {
         qt_file_engine_handlers_in_use = false;
      }
   }
}

/*
   \ìnternal

   Handles calls to custom file engine handlers.
*/
QAbstractFileEngine *qt_custom_file_engine_handler_create(const QString &path)
{
   QAbstractFileEngine *engine = 0;

   if (qt_file_engine_handlers_in_use) {
      QReadLocker locker(fileEngineHandlerMutex());

      // check for registered handlers that can load the file
      QAbstractFileEngineHandlerList *handlers = fileEngineHandlers();
      for (int i = 0; i < handlers->size(); i++) {
         if ((engine = handlers->at(i)->create(path))) {
            break;
         }
      }
   }

   return engine;
}

QAbstractFileEngine *QAbstractFileEngine::create(const QString &fileName)
{
   QFileSystemEntry entry(fileName);
   QFileSystemMetaData metaData;
   QAbstractFileEngine *engine = QFileSystemEngine::resolveEntryAndCreateLegacyEngine(entry, metaData);

#ifndef QT_NO_FSFILEENGINE
   if (! engine) {
      // fall back to regular file engine
      return new QFSFileEngine(entry.filePath());
   }
#endif

   return engine;
}

QAbstractFileEngine::QAbstractFileEngine() : d_ptr(new QAbstractFileEnginePrivate)
{
   d_ptr->q_ptr = this;
}

/*!
   \internal

   Constructs a QAbstractFileEngine.
 */
QAbstractFileEngine::QAbstractFileEngine(QAbstractFileEnginePrivate &dd) : d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

/*!
    Destroys the QAbstractFileEngine.
 */
QAbstractFileEngine::~QAbstractFileEngine()
{
}

/*!
    \fn bool QAbstractFileEngine::open(QIODevice::OpenMode mode)

    Opens the file in the specified \a mode. Returns true if the file
    was successfully opened; otherwise returns false.

    The \a mode is an OR combination of QIODevice::OpenMode and
    QIODevice::HandlingMode values.
*/
bool QAbstractFileEngine::open(QIODevice::OpenMode openMode)
{
   Q_UNUSED(openMode);
   return false;
}

/*!
    Closes the file, returning true if successful; otherwise returns false.

    The default implementation always returns false.
*/
bool QAbstractFileEngine::close()
{
   return false;
}

bool QAbstractFileEngine::syncToDisk()
{
   return false;
}

/*!
    Flushes the open file, returning true if successful; otherwise returns
    false.

    The default implementation always returns false.
*/
bool QAbstractFileEngine::flush()
{
   return false;
}

/*!
    Returns the size of the file.
*/
qint64 QAbstractFileEngine::size() const
{
   return 0;
}

/*!
    Returns the current file position.

    This is the position of the data read/write head of the file.
*/
qint64 QAbstractFileEngine::pos() const
{
   return 0;
}

/*!
    \fn bool QAbstractFileEngine::seek(qint64 offset)

    Sets the file position to the given \a offset. Returns true if
    the position was successfully set; otherwise returns false.

    The offset is from the beginning of the file, unless the
    file is sequential.

    \sa isSequential()
*/
bool QAbstractFileEngine::seek(qint64 pos)
{
   Q_UNUSED(pos);
   return false;
}

/*!
    Returns true if the file is a sequential access device; returns
    false if the file is a direct access device.

    Operations involving size() and seek(int) are not valid on
    sequential devices.
*/
bool QAbstractFileEngine::isSequential() const
{
   return false;
}

/*!
    Requests that the file is deleted from the file system. If the
    operation succeeds return true; otherwise return false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName() rmdir()
 */
bool QAbstractFileEngine::remove()
{
   return false;
}

/*!
    Copies the contents of this file to a file with the name \a newName.
    Returns true on success; otherwise, false is returned.
*/
bool QAbstractFileEngine::copy(const QString &newName)
{
   Q_UNUSED(newName);
   return false;
}

/*!
    Requests that the file be renamed to \a newName in the file
    system. If the operation succeeds return true; otherwise return
    false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName()
 */
bool QAbstractFileEngine::rename(const QString &newName)
{
   Q_UNUSED(newName);
   return false;
}

bool QAbstractFileEngine::renameOverwrite(const QString &newName)
{
   Q_UNUSED(newName);
   return false;
}

bool QAbstractFileEngine::link(const QString &newName)
{
   Q_UNUSED(newName);
   return false;
}

bool QAbstractFileEngine::mkdir(const QString &dirName, bool createParentDirectories) const
{
   Q_UNUSED(dirName);
   Q_UNUSED(createParentDirectories);
   return false;
}

/*!
    Requests that the directory \a dirName is deleted from the file
    system. When \a recurseParentDirectories is true, then any empty
    parent-directories in \a dirName must also be deleted. If
    \a recurseParentDirectories is false, only the \a dirName leaf-node
    should be deleted. In most file systems a directory cannot be deleted
    using this function if it is non-empty. If the operation succeeds
    return true; otherwise return false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName() remove() mkdir() isRelativePath()
 */
bool QAbstractFileEngine::rmdir(const QString &dirName, bool recurseParentDirectories) const
{
   Q_UNUSED(dirName);
   Q_UNUSED(recurseParentDirectories);
   return false;
}

/*!
    Requests that the file be set to size \a size. If \a size is larger
    than the current file then it is filled with 0's, if smaller it is
    simply truncated. If the operations succceeds return true; otherwise
    return false;

    This virtual function must be reimplemented by all subclasses.

    \sa size()
*/
bool QAbstractFileEngine::setSize(qint64 size)
{
   Q_UNUSED(size);
   return false;
}

/*!
    Should return true if the underlying file system is case-sensitive;
    otherwise return false.

    This virtual function must be reimplemented by all subclasses.
 */
bool QAbstractFileEngine::caseSensitive() const
{
   return false;
}

/*!
    Return true if the file referred to by this file engine has a
    relative path; otherwise return false.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName()
 */
bool QAbstractFileEngine::isRelativePath() const
{
   return false;
}

/*!
    Requests that a list of all the files matching the \a filters
    list based on the \a filterNames in the file engine's directory
    are returned.

    Should return an empty list if the file engine refers to a file
    rather than a directory, or if the directory is unreadable or does
    not exist or if nothing matches the specifications.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName()
 */
QStringList QAbstractFileEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
{
   QStringList ret;
   QDirIterator it(fileName(), filterNames, filters);
   while (it.hasNext()) {
      it.next();
      ret << it.fileName();
   }
   return ret;
}

/*!
    This function should return the set of OR'd flags that are true
    for the file engine's file, and that are in the \a type's OR'd
    members.

    In your reimplementation you can use the \a type argument as an
    optimization hint and only return the OR'd set of members that are
    true and that match those in \a type; in other words you can
    ignore any members not mentioned in \a type, thus avoiding some
    potentially expensive lookups or system calls.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName()
*/
QAbstractFileEngine::FileFlags QAbstractFileEngine::fileFlags(FileFlags type) const
{
   Q_UNUSED(type);
   return 0;
}

/*!
    Requests that the file's permissions be set to \a perms. The argument
    perms will be set to the OR-ed together combination of
    QAbstractFileEngine::FileInfo, with only the QAbstractFileEngine::PermsMask being
    honored. If the operations succceeds return true; otherwise return
    false;

    This virtual function must be reimplemented by all subclasses.

    \sa size()
*/
bool QAbstractFileEngine::setPermissions(uint perms)
{
   Q_UNUSED(perms);
   return false;
}

/*!
    Return  the file engine's current file name in the format
    specified by \a file.

    If you don't handle some \c FileName possibilities, return the
    file name set in setFileName() when an unhandled format is
    requested.

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName(), FileName
 */
QString QAbstractFileEngine::fileName(FileName file) const
{
   Q_UNUSED(file);
   return QString();
}

/*!
    If \a owner is \c OwnerUser return the ID of the user who owns
    the file. If \a owner is \c OwnerGroup return the ID of the group
    that own the file. If you can't determine the owner return -2.

    This virtual function must be reimplemented by all subclasses.

    \sa owner() setFileName(), FileOwner
 */
uint QAbstractFileEngine::ownerId(FileOwner owner) const
{
   Q_UNUSED(owner);
   return 0;
}

/*!
    If \a owner is \c OwnerUser return the name of the user who owns
    the file. If \a owner is \c OwnerGroup return the name of the group
    that own the file. If you can't determine the owner return
    QString().

    This virtual function must be reimplemented by all subclasses.

    \sa ownerId() setFileName(), FileOwner
 */
QString QAbstractFileEngine::owner(FileOwner owner) const
{
   Q_UNUSED(owner);
   return QString();
}

/*!
    If \a time is \c CreationTime, return when the file was created.
    If \a time is \c ModificationTime, return when the file was most
    recently modified. If \a time is \c AccessTime, return when the
    file was most recently accessed (e.g. read or written).
    If the time cannot be determined return QDateTime() (an invalid
    date time).

    This virtual function must be reimplemented by all subclasses.

    \sa setFileName(), QDateTime, QDateTime::isValid(), FileTime
 */
QDateTime QAbstractFileEngine::fileTime(FileTime time) const
{
   Q_UNUSED(time);
   return QDateTime();
}

/*!
    Sets the file engine's file name to \a file. This file name is the
    file that the rest of the virtual functions will operate on.

    This virtual function must be reimplemented by all subclasses.

    \sa rename()
 */
void QAbstractFileEngine::setFileName(const QString &file)
{
   Q_UNUSED(file);
}

/*!
    Returns the native file handle for this file engine. This handle must be
    used with care; its value and type are platform specific, and using it
    will most likely lead to non-portable code.
*/
int QAbstractFileEngine::handle() const
{
   return -1;
}

/*!
    \since 4.3

    Returns true if the current position is at the end of the file; otherwise,
    returns false.

    This function bases its behavior on calling extension() with
    AtEndExtension. If the engine does not support this extension, false is
    returned.

    \sa extension(), supportsExtension(), QFile::atEnd()
*/
bool QAbstractFileEngine::atEnd() const
{
   return const_cast<QAbstractFileEngine *>(this)->extension(AtEndExtension);
}

/*!
    \since 4.4

    Maps \a size bytes of the file into memory starting at \a offset.
    Returns a pointer to the memory if successful; otherwise returns false
    if, for example, an error occurs.

    This function bases its behavior on calling extension() with
    MapExtensionOption. If the engine does not support this extension, 0 is
    returned.

    \a flags is currently not used, but could be used in the future.

    \sa unmap(), supportsExtension()
 */

uchar *QAbstractFileEngine::map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags)
{
   MapExtensionOption option;
   option.offset = offset;
   option.size = size;
   option.flags = flags;
   MapExtensionReturn r;
   if (!extension(MapExtension, &option, &r)) {
      return 0;
   }
   return r.address;
}

/*!
    \since 4.4

    Unmaps the memory \a address.  Returns true if the unmap succeeds; otherwise
    returns false.

    This function bases its behavior on calling extension() with
    UnMapExtensionOption. If the engine does not support this extension, false is
    returned.

    \sa map(), supportsExtension()
 */
bool QAbstractFileEngine::unmap(uchar *address)
{
   UnMapExtensionOption options;
   options.address = address;
   return extension(UnMapExtension, &options);
}

class QAbstractFileEngineIteratorPrivate
{
 public:
   QString path;
   QDir::Filters filters;
   QStringList nameFilters;
   QFileInfo fileInfo;
};

/*!
    Constructs a QAbstractFileEngineIterator, using the entry filters \a
    filters, and wildcard name filters \a nameFilters.
*/
QAbstractFileEngineIterator::QAbstractFileEngineIterator(QDir::Filters filters,
      const QStringList &nameFilters)
   : d(new QAbstractFileEngineIteratorPrivate)
{
   d->nameFilters = nameFilters;
   d->filters = filters;
}

/*!
    Destroys the QAbstractFileEngineIterator.

    \sa QDirIterator
*/
QAbstractFileEngineIterator::~QAbstractFileEngineIterator()
{
}

/*!
    Returns the path for this iterator. QDirIterator is responsible for
    assigning this path; it cannot change during the iterator's lifetime.

    \sa nameFilters(), filters()
*/
QString QAbstractFileEngineIterator::path() const
{
   return d->path;
}

/*!
    \internal

    Sets the iterator path to \a path. This function is called from within
    QDirIterator.
*/
void QAbstractFileEngineIterator::setPath(const QString &path)
{
   d->path = path;
}

/*!
    Returns the name filters for this iterator.

    \sa QDir::nameFilters(), filters(), path()
*/
QStringList QAbstractFileEngineIterator::nameFilters() const
{
   return d->nameFilters;
}

/*!
    Returns the entry filters for this iterator.

    \sa QDir::filter(), nameFilters(), path()
*/
QDir::Filters QAbstractFileEngineIterator::filters() const
{
   return d->filters;
}

/*!
    \fn QString QAbstractFileEngineIterator::currentFileName() const = 0

    This pure virtual function returns the name of the current directory
    entry, excluding the path.

    \sa currentFilePath()
*/

/*!
    Returns the path to the current directory entry. It's the same as
    prepending path() to the return value of currentFileName().

    \sa currentFileName()
*/
QString QAbstractFileEngineIterator::currentFilePath() const
{
   QString name = currentFileName();

   if (! name.isEmpty()) {
      QString tmp = path();

      if (! tmp.isEmpty()) {
         if (!tmp.endsWith('/')) {
            tmp.append('/');
         }
         name.prepend(tmp);
      }
   }
   return name;
}

/*!
    The virtual function returns a QFileInfo for the current directory
    entry. This function is provided for convenience. It can also be slightly
    faster than creating a QFileInfo object yourself, as the object returned
    by this function might contain cached information that QFileInfo otherwise
    would have to access through the file engine.

    \sa currentFileName()
*/
QFileInfo QAbstractFileEngineIterator::currentFileInfo() const
{
   QString path = currentFilePath();
   if (d->fileInfo.filePath() != path) {
      d->fileInfo.setFile(path);
   }

   // return a shallow copy
   return d->fileInfo;
}

/*!
    \internal

    Returns the entry info \a type for this iterator's current directory entry
    as a QVariant. If \a type is undefined for this entry, a null QVariant is
    returned.

    \sa QAbstractFileEngine::beginEntryList(), QDir::beginEntryList()
*/
QVariant QAbstractFileEngineIterator::entryInfo(EntryInfoType type) const
{
   Q_UNUSED(type)
   return QVariant();
}

QAbstractFileEngine::Iterator *QAbstractFileEngine::beginEntryList(QDir::Filters filters,
      const QStringList &filterNames)
{
   Q_UNUSED(filters);
   Q_UNUSED(filterNames);
   return 0;
}

/*!
    \internal
*/
QAbstractFileEngine::Iterator *QAbstractFileEngine::endEntryList()
{
   return 0;
}

qint64 QAbstractFileEngine::read(char *data, qint64 maxlen)
{
   Q_UNUSED(data);
   Q_UNUSED(maxlen);
   return -1;
}

qint64 QAbstractFileEngine::write(const char *data, qint64 len)
{
   Q_UNUSED(data);
   Q_UNUSED(len);
   return -1;
}

qint64 QAbstractFileEngine::readLine(char *data, qint64 maxlen)
{
   qint64 readSoFar = 0;

   while (readSoFar < maxlen) {
      char c;
      qint64 readResult = read(&c, 1);

      if (readResult <= 0) {
         return (readSoFar > 0) ? readSoFar : -1;
      }

      ++readSoFar;
      *data++ = c;

      if (c == '\n') {
         return readSoFar;
      }
   }

   return readSoFar;
}

bool QAbstractFileEngine::extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
{
   Q_UNUSED(extension);
   Q_UNUSED(option);
   Q_UNUSED(output);
   return false;
}

bool QAbstractFileEngine::supportsExtension(Extension extension) const
{
   Q_UNUSED(extension);
   return false;
}

QFile::FileError QAbstractFileEngine::error() const
{
   Q_D(const QAbstractFileEngine);
   return d->fileError;
}

/*!
  Returns the human-readable message appropriate to the current error
  reported by error(). If no suitable string is available, an
  empty string is returned.

  \sa error()
 */
QString QAbstractFileEngine::errorString() const
{
   Q_D(const QAbstractFileEngine);
   return d->errorString;
}

/*!
    Sets the error type to \a error, and the error string to \a errorString.
    Call this function to set the error values returned by the higher-level
    classes.

    \sa QFile::error(), QIODevice::errorString(), QIODevice::setErrorString()
*/
void QAbstractFileEngine::setError(QFile::FileError error, const QString &errorString)
{
   Q_D(QAbstractFileEngine);
   d->fileError = error;
   d->errorString = errorString;
}

QT_END_NAMESPACE
