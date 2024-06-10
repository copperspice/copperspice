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

#include <qfsfileengine_p.h>

#include <qdatetime.h>
#include <qdebug.h>
#include <qdiriterator.h>
#include <qset.h>

#include <qfilesystemengine_p.h>
#include <qfsfileengine_iterator_p.h>

#ifndef QT_NO_FSFILEENGINE
#include <errno.h>

#if defined(Q_OS_UNIX)
#include <qcore_unix_p.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#if defined(Q_OS_DARWIN)
#include <qcore_mac_p.h>
#endif

#ifdef Q_OS_WIN
#  ifndef S_ISREG
#    define S_ISREG(x)   (((x) & S_IFMT) == S_IFREG)
#  endif
#  ifndef S_ISCHR
#    define S_ISCHR(x)   (((x) & S_IFMT) == S_IFCHR)
#  endif
#  ifndef S_ISFIFO
#    define S_ISFIFO(x) false
#  endif
#  ifndef S_ISSOCK
#    define S_ISSOCK(x) false
#  endif
#  ifndef INVALID_FILE_ATTRIBUTES
#    define INVALID_FILE_ATTRIBUTES (DWORD (-1))
#  endif
#endif

// QFSFileEnginePrivate
QFSFileEnginePrivate::QFSFileEnginePrivate() : QAbstractFileEnginePrivate()
{
   init();
}

void QFSFileEnginePrivate::init()
{
   is_sequential = 0;
   tried_stat    = 0;
   need_lstat    = 1;
   is_link       = 0;

   openMode = QIODevice::NotOpen;
   fd = -1;
   fh = nullptr;

   lastIOCommand = IOFlushCommand;
   lastFlushFailed = false;
   closeFileHandle = false;

#ifdef Q_OS_WIN
   fileAttrib = INVALID_FILE_ATTRIBUTES;
   fileHandle = INVALID_HANDLE_VALUE;
   mapHandle = INVALID_HANDLE_VALUE;
   cachedFd = -1;
#endif

}

/*!
    Constructs a QFSFileEngine for the file name \a file.
*/
QFSFileEngine::QFSFileEngine(const QString &file)
   : QAbstractFileEngine(*new QFSFileEnginePrivate)
{
   Q_D(QFSFileEngine);
   d->fileEntry = QFileSystemEntry(file);
}

QFSFileEngine::QFSFileEngine() : QAbstractFileEngine(*new QFSFileEnginePrivate)
{
}

QFSFileEngine::QFSFileEngine(QFSFileEnginePrivate &dd)
   : QAbstractFileEngine(dd)
{
}

QFSFileEngine::~QFSFileEngine()
{
   Q_D(QFSFileEngine);

   if (d->closeFileHandle) {
      if (d->fh) {
         int ret;

         do {
            ret = fclose(d->fh);
         } while (ret == EOF && errno == EINTR);

      } else if (d->fd != -1) {
         int ret;

         do {
            ret = QT_CLOSE(d->fd);
         } while (ret == -1 && errno == EINTR);
      }
   }

   QList<uchar *> keys = d->maps.keys();

   for (int i = 0; i < keys.count(); ++i) {
      unmap(keys.at(i));
   }
}

void QFSFileEngine::setFileName(const QString &file)
{
   Q_D(QFSFileEngine);
   d->init();
   d->fileEntry = QFileSystemEntry(file);
}

bool QFSFileEngine::open(QIODevice::OpenMode openMode)
{
   Q_D(QFSFileEngine);

   if (d->fileEntry.isEmpty()) {

#if defined(CS_SHOW_DEBUG_CORE)
      qDebug("QFSFileEngine::open() No file name specified");
#endif

      setError(QFile::OpenError, QString("No file name specified"));
      return false;
   }

   // Append implies WriteOnly.
   if (openMode & QFile::Append) {
      openMode |= QFile::WriteOnly;
   }

   // WriteOnly implies Truncate if neither ReadOnly nor Append are sent.
   if ((openMode & QFile::WriteOnly) && !(openMode & (QFile::ReadOnly | QFile::Append))) {
      openMode |= QFile::Truncate;
   }

   d->openMode = openMode;
   d->lastFlushFailed = false;
   d->tried_stat = 0;
   d->fh = nullptr;
   d->fd = -1;

   return d->nativeOpen(openMode);
}

/*!
    Opens the file handle \a fh in \a openMode mode. Returns true on
    success; otherwise returns false.
*/
bool QFSFileEngine::open(QIODevice::OpenMode openMode, FILE *fh)
{
   return open(openMode, fh, QFile::DontCloseHandle);
}

bool QFSFileEngine::open(QIODevice::OpenMode openMode, FILE *fh, QFile::FileHandleFlags handleFlags)
{
   Q_D(QFSFileEngine);

   // Append implies WriteOnly.
   if (openMode & QFile::Append) {
      openMode |= QFile::WriteOnly;
   }

   // WriteOnly implies Truncate if neither ReadOnly nor Append are sent.
   if ((openMode & QFile::WriteOnly) && !(openMode & (QFile::ReadOnly | QFile::Append))) {
      openMode |= QFile::Truncate;
   }

   d->openMode = openMode;
   d->lastFlushFailed = false;
   d->closeFileHandle = (handleFlags & QFile::AutoCloseHandle);
   d->fileEntry.clear();
   d->tried_stat = 0;
   d->fd = -1;

   return d->openFh(openMode, fh);
}

/*!
    Opens the file handle \a fh using the open mode \a flags.
*/
bool QFSFileEnginePrivate::openFh(QIODevice::OpenMode openMode, FILE *fh)
{
   Q_Q(QFSFileEngine);
   this->fh = fh;
   fd = -1;

   // Seek to the end when in Append mode.
   if (openMode & QIODevice::Append) {
      int ret;

      do {
         ret = QT_FSEEK(fh, 0, SEEK_END);
      } while (ret != 0 && errno == EINTR);

      if (ret != 0) {
         q->setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError,
               qt_error_string(int(errno)));

         this->openMode = QIODevice::NotOpen;
         this->fh = nullptr;

         return false;
      }
   }

   return true;
}

bool QFSFileEngine::open(QIODevice::OpenMode openMode, int fd)
{
   return open(openMode, fd, QFile::DontCloseHandle);
}

bool QFSFileEngine::open(QIODevice::OpenMode openMode, int fd, QFile::FileHandleFlags handleFlags)
{
   Q_D(QFSFileEngine);

   // Append implies WriteOnly
   if (openMode & QFile::Append) {
      openMode |= QFile::WriteOnly;
   }

   // WriteOnly implies Truncate if neither ReadOnly nor Append are sent.
   if ((openMode & QFile::WriteOnly) && !(openMode & (QFile::ReadOnly | QFile::Append))) {
      openMode |= QFile::Truncate;
   }

   d->openMode = openMode;
   d->lastFlushFailed = false;
   d->closeFileHandle = (handleFlags & QFile::AutoCloseHandle);
   d->fileEntry.clear();
   d->fh = nullptr;
   d->fd = -1;
   d->tried_stat = 0;

   return d->openFd(openMode, fd);
}

bool QFSFileEnginePrivate::openFd(QIODevice::OpenMode openMode, int fd)
{
   Q_Q(QFSFileEngine);
   this->fd = fd;
   fh = nullptr;

   // Seek to the end when in Append mode.
   if (openMode & QFile::Append) {
      int ret;

      do {
         ret = QT_LSEEK(fd, 0, SEEK_END);
      } while (ret == -1 && errno == EINTR);

      if (ret == -1) {
         q->setError(errno == EMFILE ? QFile::ResourceError : QFile::OpenError,
               qt_error_string(int(errno)));

         this->openMode = QIODevice::NotOpen;
         this->fd = -1;

         return false;
      }
   }

   return true;
}

bool QFSFileEngine::close()
{
   Q_D(QFSFileEngine);
   d->openMode = QIODevice::NotOpen;
   return d->nativeClose();
}

bool QFSFileEnginePrivate::closeFdFh()
{
   Q_Q(QFSFileEngine);

   if (fd == -1 && !fh) {
      return false;
   }

   // Flush the file if it's buffered, and if the last flush didn't fail.
   bool flushed = !fh || (!lastFlushFailed && q->flush());
   bool closed = true;
   tried_stat = 0;

   // Close the file if we created the handle.
   if (closeFileHandle) {
      int ret;

      do {
         if (fh) {
            // Close buffered file.
            ret = fclose(fh) != 0 ? -1 : 0;
         } else {
            // Close unbuffered file.
            ret = QT_CLOSE(fd);
         }
      } while (ret == -1 && errno == EINTR);

      // We must reset these guys regardless; calling close again after a
      // failed close causes crashes on some systems.
      fh = nullptr;
      fd = -1;
      closed = (ret == 0);
   }

   // Report errors
   if (! flushed || !closed) {
      if (flushed) {
         // if not flushed, flush all errors
         q->setError(QFile::UnspecifiedError, qt_error_string(errno));
      }

      return false;
   }

   return true;
}

bool QFSFileEngine::flush()
{
   Q_D(QFSFileEngine);

   if ((d->openMode & QIODevice::WriteOnly) == 0) {
      // Nothing in the write buffers, so flush succeeds in doing
      // nothing.
      return true;
   }

   return d->nativeFlush();
}

bool QFSFileEngine::syncToDisk()
{
   Q_D(QFSFileEngine);

   if ((d->openMode & QIODevice::WriteOnly) == 0) {
      return true;
   }

   return d->nativeSyncToDisk();
}

bool QFSFileEnginePrivate::flushFh()
{
   Q_Q(QFSFileEngine);

   // Never try to flush again if the last flush failed. Otherwise you can
   // get crashes on some systems (AIX).
   if (lastFlushFailed) {
      return false;
   }

   int ret = fflush(fh);

   lastFlushFailed = (ret != 0);
   lastIOCommand = QFSFileEnginePrivate::IOFlushCommand;

   if (ret != 0) {
      q->setError(errno == ENOSPC ? QFile::ResourceError : QFile::WriteError,
            qt_error_string(errno));
      return false;
   }

   return true;
}

qint64 QFSFileEngine::size() const
{
   Q_D(const QFSFileEngine);
   return d->nativeSize();
}

#ifndef Q_OS_WIN
qint64 QFSFileEnginePrivate::sizeFdFh() const
{
   Q_Q(const QFSFileEngine);
   const_cast<QFSFileEngine *>(q)->flush();

   tried_stat = 0;
   metaData.clearFlags(QFileSystemMetaData::SizeAttribute);

   if (!doStat(QFileSystemMetaData::SizeAttribute)) {
      return 0;
   }

   return metaData.size();
}
#endif

qint64 QFSFileEngine::pos() const
{
   Q_D(const QFSFileEngine);
   return d->nativePos();
}

qint64 QFSFileEnginePrivate::posFdFh() const
{
   if (fh) {
      return qint64(QT_FTELL(fh));
   }

   return QT_LSEEK(fd, 0, SEEK_CUR);
}

bool QFSFileEngine::seek(qint64 pos)
{
   Q_D(QFSFileEngine);
   return d->nativeSeek(pos);
}

bool QFSFileEnginePrivate::seekFdFh(qint64 pos)
{
   Q_Q(QFSFileEngine);

   // On Windows' stdlib implementation, the results of calling fread and
   // fwrite are undefined if not called either in sequence, or if preceded
   // with a call to fflush().
   if (lastIOCommand != QFSFileEnginePrivate::IOFlushCommand && !q->flush()) {
      return false;
   }

   if (pos < 0 || pos != qint64(QT_OFF_T(pos))) {
      return false;
   }

   if (fh) {
      // Buffered stdlib mode.
      int ret;

      do {
         ret = QT_FSEEK(fh, QT_OFF_T(pos), SEEK_SET);
      } while (ret != 0 && errno == EINTR);

      if (ret != 0) {
         q->setError(QFile::ReadError, qt_error_string(int(errno)));
         return false;
      }

   } else {
      // Unbuffered stdio mode.
      if (QT_LSEEK(fd, QT_OFF_T(pos), SEEK_SET) == -1) {

#if defined(CS_SHOW_DEBUG_CORE)
         qDebug() << "QFSFileEngine::seekFdFh() Unable to set file position" << pos;
#endif

         q->setError(QFile::PositionError, qt_error_string(errno));
         return false;
      }
   }

   return true;
}

int QFSFileEngine::handle() const
{
   Q_D(const QFSFileEngine);
   return d->nativeHandle();
}

qint64 QFSFileEngine::read(char *data, qint64 maxlen)
{
   Q_D(QFSFileEngine);

   // On Windows' stdlib implementation, the results of calling fread and
   // fwrite are undefined if not called either in sequence, or if preceded
   // with a call to fflush().

   if (d->lastIOCommand != QFSFileEnginePrivate::IOReadCommand) {
      flush();
      d->lastIOCommand = QFSFileEnginePrivate::IOReadCommand;
   }

   return d->nativeRead(data, maxlen);
}

qint64 QFSFileEnginePrivate::readFdFh(char *data, qint64 len)
{
   Q_Q(QFSFileEngine);

   if (len < 0 || len != qint64(size_t(len))) {
      q->setError(QFile::ReadError, qt_error_string(EINVAL));
      return -1;
   }

   qint64 readBytes = 0;
   bool eof = false;

   if (fh) {
      // Buffered stdlib mode.

      size_t result;
      bool retry = true;

      do {
         result = fread(data + readBytes, 1, size_t(len - readBytes), fh);
         eof = feof(fh);

         if (retry && eof && result == 0) {
            // On Mac OS, this is needed, e.g., if a file was written to
            // through another stream since our last read. See test
            // tst_QFile::appendAndRead

            QT_FSEEK(fh, QT_FTELL(fh), SEEK_SET); // re-sync stream.
            retry = false;
            continue;
         }

         readBytes += result;
      } while (!eof && (result == 0 ? errno == EINTR : readBytes < len));

   } else if (fd != -1) {
      // Unbuffered stdio mode.

#ifdef Q_OS_WIN
      int result;
#else
      ssize_t result;
#endif

      do {
         result = QT_READ(fd, data + readBytes, size_t(len - readBytes));
      } while ((result == -1 && errno == EINTR) || (result > 0 && (readBytes += result) < len));

      eof = !(result == -1);
   }

   if (!eof && readBytes == 0) {
      readBytes = -1;
      q->setError(QFile::ReadError, qt_error_string(errno));
   }

   return readBytes;
}

qint64 QFSFileEngine::readLine(char *data, qint64 maxlen)
{
   Q_D(QFSFileEngine);

   // On Windows' stdlib implementation, the results of calling fread and
   // fwrite are undefined if not called either in sequence, or if preceded
   // with a call to fflush().
   if (d->lastIOCommand != QFSFileEnginePrivate::IOReadCommand) {
      flush();
      d->lastIOCommand = QFSFileEnginePrivate::IOReadCommand;
   }

   return d->nativeReadLine(data, maxlen);
}

qint64 QFSFileEnginePrivate::readLineFdFh(char *data, qint64 maxlen)
{
   Q_Q(QFSFileEngine);

   if (!fh) {
      return q->QAbstractFileEngine::readLine(data, maxlen);
   }

   QT_OFF_T oldPos = 0;
#ifdef Q_OS_WIN
   bool seq = q->isSequential();

   if (!seq)
#endif
      oldPos = QT_FTELL(fh);

   // QIODevice::readLine() passes maxlen - 1 to QFile::readLineData()
   // because it has made space for the '\0' at the end of data.  But fgets
   // does the same, so we'd get two '\0' at the end - passing maxlen + 1 solves this
   if (! fgets(data, int(maxlen + 1), fh)) {
      if (!feof(fh)) {
         q->setError(QFile::ReadError, qt_error_string(int(errno)));
      }

      return -1;              // error
   }

#ifdef Q_OS_WIN

   if (seq) {
      return qstrlen(data);
   }

#endif

   qint64 lineLength = QT_FTELL(fh) - oldPos;
   return lineLength > 0 ? lineLength : qstrlen(data);
}

qint64 QFSFileEngine::write(const char *data, qint64 len)
{
   Q_D(QFSFileEngine);

   // On Windows' stdlib implementation, the results of calling fread and
   // fwrite are undefined if not called either in sequence, or if preceded
   // with a call to fflush().
   if (d->lastIOCommand != QFSFileEnginePrivate::IOWriteCommand) {
      flush();
      d->lastIOCommand = QFSFileEnginePrivate::IOWriteCommand;
   }

   return d->nativeWrite(data, len);
}

qint64 QFSFileEnginePrivate::writeFdFh(const char *data, qint64 len)
{
   Q_Q(QFSFileEngine);

   if (len < 0 || len != qint64(size_t(len))) {
      q->setError(QFile::WriteError, qt_error_string(EINVAL));
      return -1;
   }

   qint64 writtenBytes = 0;

   if (fh) {
      // Buffered stdlib mode.

      size_t result;

      do {
         result = fwrite(data + writtenBytes, 1, size_t(len - writtenBytes), fh);
         writtenBytes += result;
      } while (result == 0 ? errno == EINTR : writtenBytes < len);

   } else if (fd != -1) {
      // Unbuffered stdio mode.

#ifdef Q_OS_WIN
      int result;
#else
      ssize_t result;
#endif

      do {
         result = QT_WRITE(fd, data + writtenBytes, size_t(len - writtenBytes));
      } while ((result == -1 && errno == EINTR)
            || (result > 0 && (writtenBytes += result) < len));
   }

   if (len &&  writtenBytes == 0) {
      writtenBytes = -1;
      q->setError(errno == ENOSPC ? QFile::ResourceError : QFile::WriteError, qt_error_string(errno));
   }

   return writtenBytes;
}

#ifndef QT_NO_FILESYSTEMITERATOR
QAbstractFileEngineIterator *QFSFileEngine::beginEntryList(QDir::Filters filters, const QStringList &filterNames)
{
   return new QFSFileEngineIterator(filters, filterNames);
}

QAbstractFileEngineIterator *QFSFileEngine::endEntryList()
{
   return nullptr;
}
#endif

QStringList QFSFileEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
{
   return QAbstractFileEngine::entryList(filters, filterNames);
}

bool QFSFileEngine::isSequential() const
{
   Q_D(const QFSFileEngine);

   if (d->is_sequential == 0) {
      d->is_sequential = d->nativeIsSequential() ? 1 : 2;
   }

   return d->is_sequential == 1;
}

#ifdef Q_OS_UNIX
bool QFSFileEnginePrivate::isSequentialFdFh() const
{
   if (doStat(QFileSystemMetaData::SequentialType)) {
      return metaData.isSequential();
   }

   return true;
}
#endif

bool QFSFileEngine::extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
{
   Q_D(QFSFileEngine);

   if (extension == AtEndExtension && d->fh && isSequential()) {
      return feof(d->fh);
   }

   if (extension == MapExtension) {
      const MapExtensionOption *options = (MapExtensionOption *)(option);
      MapExtensionReturn *returnValue = static_cast<MapExtensionReturn *>(output);
      returnValue->address = d->map(options->offset, options->size, options->flags);
      return (returnValue->address != nullptr);
   }

   if (extension == UnMapExtension) {
      UnMapExtensionOption *options = (UnMapExtensionOption *)option;
      return d->unmap(options->address);
   }

   return false;
}

bool QFSFileEngine::supportsExtension(Extension extension) const
{
   Q_D(const QFSFileEngine);

   if (extension == AtEndExtension && d->fh && isSequential()) {
      return true;
   }

   if (extension == FastReadLineExtension && d->fh) {
      return true;
   }

   if (extension == FastReadLineExtension && d->fd != -1 && isSequential()) {
      return true;
   }

   if (extension == UnMapExtension || extension == MapExtension) {
      return true;
   }

   return false;
}

#endif // QT_NO_FSFILEENGINE
