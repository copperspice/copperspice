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

#include <qabstractfileengine.h>

#include <qdatetime.h>
#include <qdiriterator.h>
#include <qfsfileengine.h>
#include <qreadwritelock.h>
#include <qvariant.h>

#include <qabstractfileengine_p.h>
#include <qfilesystemengine_p.h>
#include <qfilesystementry_p.h>
#include <qfilesystemmetadata_p.h>
#include <qresource_p.h>

static bool qt_file_engine_handlers_in_use = false;

static QReadWriteLock *fileEngineHandlerMutex()
{
   static QReadWriteLock retval(QReadWriteLock::Recursive);
   return &retval;
}

static bool qt_abstractfileenginehandlerlist_shutDown = false;

class QAbstractFileEngineHandlerList : public QList<QAbstractFileEngineHandler *>
{
 public:
   ~QAbstractFileEngineHandlerList() {
      QWriteLocker locker(fileEngineHandlerMutex());
      qt_abstractfileenginehandlerlist_shutDown = true;
   }
};

static QAbstractFileEngineHandlerList *fileEngineHandlers()
{
   static QAbstractFileEngineHandlerList retval;
   return &retval;
}

QAbstractFileEngineHandler::QAbstractFileEngineHandler()
{
   QWriteLocker locker(fileEngineHandlerMutex());
   qt_file_engine_handlers_in_use = true;
   fileEngineHandlers()->prepend(this);
}

QAbstractFileEngineHandler::~QAbstractFileEngineHandler()
{
   QWriteLocker locker(fileEngineHandlerMutex());

   // Remove this handler from the handler list only if the list is valid.
   if (! qt_abstractfileenginehandlerlist_shutDown) {
      QAbstractFileEngineHandlerList *handlers = fileEngineHandlers();
      handlers->removeOne(this);

      if (handlers->isEmpty()) {
         qt_file_engine_handlers_in_use = false;
      }
   }
}

QAbstractFileEngine *qt_custom_file_engine_handler_create(const QString &path)
{
   QAbstractFileEngine *engine = nullptr;

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

QAbstractFileEngine::QAbstractFileEngine(QAbstractFileEnginePrivate &dd) : d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QAbstractFileEngine::~QAbstractFileEngine()
{
}

bool QAbstractFileEngine::open(QIODevice::OpenMode openMode)
{
   (void) openMode;
   return false;
}

bool QAbstractFileEngine::close()
{
   return false;
}

bool QAbstractFileEngine::syncToDisk()
{
   return false;
}

bool QAbstractFileEngine::flush()
{
   return false;
}

qint64 QAbstractFileEngine::size() const
{
   return 0;
}

qint64 QAbstractFileEngine::pos() const
{
   return 0;
}

bool QAbstractFileEngine::seek(qint64 pos)
{
   (void) pos;

   return false;
}

bool QAbstractFileEngine::isSequential() const
{
   return false;
}

bool QAbstractFileEngine::remove()
{
   return false;
}

bool QAbstractFileEngine::copy(const QString &newName)
{
   (void) newName;

   return false;
}

bool QAbstractFileEngine::rename(const QString &newName)
{
   (void) newName;

   return false;
}

bool QAbstractFileEngine::renameOverwrite(const QString &newName)
{
   (void) newName;

   return false;
}

bool QAbstractFileEngine::link(const QString &newName)
{
   (void) newName;

   return false;
}

bool QAbstractFileEngine::mkdir(const QString &dirName, bool createParentDirectories) const
{
   (void) dirName;
   (void) createParentDirectories;

   return false;
}

bool QAbstractFileEngine::rmdir(const QString &dirName, bool recurseParentDirectories) const
{
   (void) dirName;
   (void) recurseParentDirectories;

   return false;
}

bool QAbstractFileEngine::setSize(qint64 size)
{
   (void) size;

   return false;
}

bool QAbstractFileEngine::caseSensitive() const
{
   return false;
}

bool QAbstractFileEngine::isRelativePath() const
{
   return false;
}

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

QAbstractFileEngine::FileFlags QAbstractFileEngine::fileFlags(FileFlags) const
{
   return Qt::EmptyFlag;
}

bool QAbstractFileEngine::setPermissions(uint perms)
{
   (void) perms;

   return false;
}

QString QAbstractFileEngine::fileName(FileName file) const
{
   (void) file;

   return QString();
}

uint QAbstractFileEngine::ownerId(FileOwner owner) const
{
   (void) owner;

   return 0;
}

QString QAbstractFileEngine::owner(FileOwner owner) const
{
   (void) owner;

   return QString();
}

QDateTime QAbstractFileEngine::fileTime(FileTime time) const
{
   (void) time;

   return QDateTime();
}

void QAbstractFileEngine::setFileName(const QString &file)
{
   (void) file;
}

int QAbstractFileEngine::handle() const
{
   return -1;
}

bool QAbstractFileEngine::atEnd() const
{
   return const_cast<QAbstractFileEngine *>(this)->extension(AtEndExtension);
}

uchar *QAbstractFileEngine::map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags)
{
   MapExtensionOption option;
   option.offset = offset;
   option.size = size;
   option.flags = flags;
   MapExtensionReturn r;

   if (! extension(MapExtension, &option, &r)) {
      return nullptr;
   }

   return r.address;
}

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

QAbstractFileEngineIterator::QAbstractFileEngineIterator(QDir::Filters filters, const QStringList &nameFilters)
   : d(new QAbstractFileEngineIteratorPrivate)
{
   d->nameFilters = nameFilters;
   d->filters = filters;
}

QAbstractFileEngineIterator::~QAbstractFileEngineIterator()
{
}

QString QAbstractFileEngineIterator::path() const
{
   return d->path;
}

// internal
void QAbstractFileEngineIterator::setPath(const QString &path)
{
   d->path = path;
}

QStringList QAbstractFileEngineIterator::nameFilters() const
{
   return d->nameFilters;
}

QDir::Filters QAbstractFileEngineIterator::filters() const
{
   return d->filters;
}

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

QFileInfo QAbstractFileEngineIterator::currentFileInfo() const
{
   QString path = currentFilePath();

   if (d->fileInfo.filePath() != path) {
      d->fileInfo.setFile(path);
   }

   // return a shallow copy
   return d->fileInfo;
}

QAbstractFileEngineIterator *QAbstractFileEngine::beginEntryList(QDir::Filters filters,
      const QStringList &filterNames)
{
   (void) filters;
   (void) filterNames;

   return nullptr;
}

// internal
QAbstractFileEngineIterator *QAbstractFileEngine::endEntryList()
{
   return nullptr;
}

qint64 QAbstractFileEngine::read(char *data, qint64 maxlen)
{
   (void) data;
   (void) maxlen;

   return -1;
}

qint64 QAbstractFileEngine::write(const char *data, qint64 len)
{
   (void) data;
   (void) len;

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
      *data = c;

      ++data;

      if (c == '\n') {
         return readSoFar;
      }
   }

   return readSoFar;
}

bool QAbstractFileEngine::extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
{
   (void) extension;
   (void) option;
   (void) output;

   return false;
}

bool QAbstractFileEngine::supportsExtension(Extension extension) const
{
   (void) extension;

   return false;
}

QFile::FileError QAbstractFileEngine::error() const
{
   Q_D(const QAbstractFileEngine);
   return d->fileError;
}

QString QAbstractFileEngine::errorString() const
{
   Q_D(const QAbstractFileEngine);
   return d->errorString;
}

void QAbstractFileEngine::setError(QFile::FileError error, const QString &errorString)
{
   Q_D(QAbstractFileEngine);
   d->fileError = error;
   d->errorString = errorString;
}
