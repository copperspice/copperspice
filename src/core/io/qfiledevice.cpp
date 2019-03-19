/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qplatformdefs.h>
#include <qfiledevice.h>
#include <qfiledevice_p.h>
#include <qfsfileengine_p.h>

QT_BEGIN_NAMESPACE

static const int QFILE_WRITEBUFFER_SIZE = 16384;

QFileDevicePrivate::QFileDevicePrivate()
   : fileEngine(0), lastWasWrite(false),
     writeBuffer(QFILE_WRITEBUFFER_SIZE), error(QFile::NoError),
     cachedSize(0)
{
}

QFileDevicePrivate::~QFileDevicePrivate()
{
   delete fileEngine;
   fileEngine = 0;
}

QAbstractFileEngine *QFileDevicePrivate::engine() const
{
   if (!fileEngine) {
      fileEngine = new QFSFileEngine;
   }
   return fileEngine;
}

void QFileDevicePrivate::setError(QFileDevice::FileError err)
{
   error = err;
   errorString.clear();
}

void QFileDevicePrivate::setError(QFileDevice::FileError err, const QString &errStr)
{
   error = err;
   errorString = errStr;
}

void QFileDevicePrivate::setError(QFileDevice::FileError err, int errNum)
{
   error = err;
   errorString = qt_error_string(errNum);
}


QFileDevice::QFileDevice()
   : QIODevice(*new QFileDevicePrivate, 0)
{
}

QFileDevice::QFileDevice(QObject *parent)
   : QIODevice(*new QFileDevicePrivate, parent)
{
}

QFileDevice::QFileDevice(QFileDevicePrivate &dd, QObject *parent)
   : QIODevice(dd, parent)
{
}

QFileDevice::~QFileDevice()
{
   close();
}

bool QFileDevice::isSequential() const
{
   Q_D(const QFileDevice);
   return d->fileEngine && d->fileEngine->isSequential();
}

int QFileDevice::handle() const
{
   Q_D(const QFileDevice);
   if (!isOpen() || !d->fileEngine) {
      return -1;
   }

   return d->fileEngine->handle();
}

/*!
    Returns the name of the file.
    The default implementation in QFileDevice returns QString().
*/
QString QFileDevice::fileName() const
{
   return QString();
}

static inline qint64 _qfile_writeData(QAbstractFileEngine *engine, QRingBuffer *buffer)
{
   qint64 ret = engine->write(buffer->readPointer(), buffer->nextDataBlockSize());
   if (ret > 0) {
      buffer->free(ret);
   }
   return ret;
}

/*!
    Flushes any buffered data to the file. Returns true if successful;
    otherwise returns false.
*/
bool QFileDevice::flush()
{
   Q_D(QFileDevice);
   if (!d->fileEngine) {
      qWarning("QFileDevice::flush: No file engine. Is IODevice open?");
      return false;
   }

   if (!d->writeBuffer.isEmpty()) {
      qint64 size = d->writeBuffer.size();
      if (_qfile_writeData(d->fileEngine, &d->writeBuffer) != size) {
         QFileDevice::FileError err = d->fileEngine->error();
         if (err == QFileDevice::UnspecifiedError) {
            err = QFileDevice::WriteError;
         }
         d->setError(err, d->fileEngine->errorString());
         return false;
      }
   }

   if (!d->fileEngine->flush()) {
      QFileDevice::FileError err = d->fileEngine->error();
      if (err == QFileDevice::UnspecifiedError) {
         err = QFileDevice::WriteError;
      }
      d->setError(err, d->fileEngine->errorString());
      return false;
   }
   return true;
}

void QFileDevice::close()
{
   Q_D(QFileDevice);
   if (!isOpen()) {
      return;
   }
   bool flushed = flush();
   QIODevice::close();

   // reset write buffer
   d->lastWasWrite = false;
   d->writeBuffer.clear();

   // keep earlier error from flush
   if (d->fileEngine->close() && flushed) {
      unsetError();
   } else if (flushed) {
      d->setError(d->fileEngine->error(), d->fileEngine->errorString());
   }
}

/*!
  \reimp
*/
qint64 QFileDevice::pos() const
{
   return QIODevice::pos();
}

/*!
  Returns true if the end of the file has been reached; otherwise returns
  false.

  For regular empty files on Unix (e.g. those in \c /proc), this function
  returns true, since the file system reports that the size of such a file is
  0. Therefore, you should not depend on atEnd() when reading data from such a
  file, but rather call read() until no more data can be read.
*/
bool QFileDevice::atEnd() const
{
   Q_D(const QFileDevice);

   // If there's buffered data left, we're not at the end.
   if (!d->buffer.isEmpty()) {
      return false;
   }

   if (!isOpen()) {
      return true;
   }

   if (!d->ensureFlushed()) {
      return false;
   }

   // If the file engine knows best, say what it says.
   if (d->fileEngine->supportsExtension(QAbstractFileEngine::AtEndExtension)) {
      // Check if the file engine supports AtEndExtension, and if it does,
      // check if the file engine claims to be at the end.
      return d->fileEngine->atEnd();
   }

   // if it looks like we are at the end, or if size is not cached,
   // fall through to bytesAvailable() to make sure.
   if (pos() < d->cachedSize) {
      return false;
   }

   // Fall back to checking how much is available (will stat files).
   return bytesAvailable() == 0;
}

/*!
    \fn bool QFileDevice::seek(qint64 pos)

    For random-access devices, this function sets the current position
    to \a pos, returning true on success, or false if an error occurred.
    For sequential devices, the default behavior is to do nothing and
    return false.

    Seeking beyond the end of a file:
    If the position is beyond the end of a file, then seek() shall not
    immediately extend the file. If a write is performed at this position,
    then the file shall be extended. The content of the file between the
    previous end of file and the newly written data is UNDEFINED and
    varies between platforms and file systems.
*/
bool QFileDevice::seek(qint64 off)
{
   Q_D(QFileDevice);
   if (!isOpen()) {
      qWarning("QFileDevice::seek: IODevice is not open");
      return false;
   }

   if (!d->ensureFlushed()) {
      return false;
   }

   if (!d->fileEngine->seek(off) || !QIODevice::seek(off)) {
      QFileDevice::FileError err = d->fileEngine->error();
      if (err == QFileDevice::UnspecifiedError) {
         err = QFileDevice::PositionError;
      }
      d->setError(err, d->fileEngine->errorString());
      return false;
   }
   unsetError();
   return true;
}

/*!
  \reimp
*/
qint64 QFileDevice::readLineData(char *data, qint64 maxlen)
{
   Q_D(QFileDevice);
   if (!d->ensureFlushed()) {
      return -1;
   }

   qint64 read;
   if (d->fileEngine->supportsExtension(QAbstractFileEngine::FastReadLineExtension)) {
      read = d->fileEngine->readLine(data, maxlen);
   } else {
      // Fall back to QIODevice's readLine implementation if the engine
      // cannot do it faster.
      read = QIODevice::readLineData(data, maxlen);
   }

   if (read < maxlen) {
      // failed to read all requested, may be at the end of file, stop caching size so that it's rechecked
      d->cachedSize = 0;
   }

   return read;
}

/*!
  \reimp
*/
qint64 QFileDevice::readData(char *data, qint64 len)
{
   Q_D(QFileDevice);

   unsetError();

   if (! d->ensureFlushed()) {
      return -1;
   }

   const qint64 read = d->fileEngine->read(data, len);

   if (read < 0) {
      QFileDevice::FileError err = d->fileEngine->error();

      if (err == QFileDevice::UnspecifiedError) {
         err = QFileDevice::ReadError;
      }
      d->setError(err, d->fileEngine->errorString());
   }

   if (read < len) {
      // failed to read all requested, may be at the end of file, stop caching size so that it's rechecked
      d->cachedSize = 0;
   }

   return read;
}

/*!
    \internal
*/
bool QFileDevicePrivate::putCharHelper(char c)
{
   // Cutoff for code that doesn't only touch the buffer.
   int writeBufferSize = writeBuffer.size();
   if ((openMode & QIODevice::Unbuffered) || writeBufferSize + 1 >= QFILE_WRITEBUFFER_SIZE

#ifdef Q_OS_WIN
         || ((openMode & QIODevice::Text) && c == '\n' && writeBufferSize + 2 >= QFILE_WRITEBUFFER_SIZE)
#endif
      ) {
      return QIODevicePrivate::putCharHelper(c);
   }

   if (!(openMode & QIODevice::WriteOnly)) {
      if (openMode == QIODevice::NotOpen) {
         qWarning("QIODevice::putChar: Closed device");
      } else {
         qWarning("QIODevice::putChar: ReadOnly device");
      }
      return false;
   }

   // Make sure the device is positioned correctly.
   const bool sequential = isSequential();
   if (pos != devicePos && !sequential && !q_func()->seek(pos)) {
      return false;
   }

   lastWasWrite = true;

   int len = 1;
#ifdef Q_OS_WIN
   if ((openMode & QIODevice::Text) && c == '\n') {
      ++len;
      *writeBuffer.reserve(1) = '\r';
   }
#endif

   // Write to buffer.
   *writeBuffer.reserve(1) = c;

   if (!sequential) {
      pos += len;
      devicePos += len;
      if (!buffer.isEmpty()) {
         buffer.skip(len);
      }
   }

   return true;
}

/*!
  \reimp
*/
qint64 QFileDevice::writeData(const char *data, qint64 len)
{
   Q_D(QFileDevice);
   unsetError();
   d->lastWasWrite = true;
   bool buffered = !(d->openMode & Unbuffered);

   // Flush buffered data if this read will overflow.
   if (buffered && (d->writeBuffer.size() + len) > QFILE_WRITEBUFFER_SIZE) {
      if (!flush()) {
         return -1;
      }
   }

   // Write directly to the engine if the block size is larger than
   // the write buffer size.
   if (!buffered || len > QFILE_WRITEBUFFER_SIZE) {
      const qint64 ret = d->fileEngine->write(data, len);
      if (ret < 0) {
         QFileDevice::FileError err = d->fileEngine->error();
         if (err == QFileDevice::UnspecifiedError) {
            err = QFileDevice::WriteError;
         }
         d->setError(err, d->fileEngine->errorString());
      }
      return ret;
   }

   // Write to the buffer.
   char *writePointer = d->writeBuffer.reserve(len);
   if (len == 1) {
      *writePointer = *data;
   } else {
      ::memcpy(writePointer, data, len);
   }
   return len;
}

/*!
    Returns the file error status.

    The I/O device status returns an error code. For example, if open()
    returns false, or a read/write operation returns -1, this function can
    be called to find out the reason why the operation failed.

    \sa unsetError()
*/
QFileDevice::FileError QFileDevice::error() const
{
   Q_D(const QFileDevice);
   return d->error;
}

/*!
    Sets the file's error to QFileDevice::NoError.

    \sa error()
*/
void QFileDevice::unsetError()
{
   Q_D(QFileDevice);
   d->setError(QFileDevice::NoError);
}

/*!
  Returns the size of the file.

  For regular empty files on Unix (e.g. those in \c /proc), this function
  returns 0; the contents of such a file are generated on demand in response
  to you calling read().
*/
qint64 QFileDevice::size() const
{
   Q_D(const QFileDevice);
   if (!d->ensureFlushed()) {
      return 0;
   }
   d->cachedSize = d->engine()->size();
   return d->cachedSize;
}

/*!
    Sets the file size (in bytes) \a sz. Returns true if the file if the
    resize succeeds; false otherwise. If \a sz is larger than the file
    currently is the new bytes will be set to 0, if \a sz is smaller the
    file is simply truncated.

    \sa size()
*/
bool QFileDevice::resize(qint64 sz)
{
   Q_D(QFileDevice);
   if (!d->ensureFlushed()) {
      return false;
   }
   d->engine();
   if (isOpen() && d->fileEngine->pos() > sz) {
      seek(sz);
   }
   if (d->fileEngine->setSize(sz)) {
      unsetError();
      d->cachedSize = sz;
      return true;
   }
   d->cachedSize = 0;
   d->setError(QFile::ResizeError, d->fileEngine->errorString());
   return false;
}

/*!
    Returns the complete OR-ed together combination of
    QFile::Permission for the file.

    \sa setPermissions()
*/
QFile::Permissions QFileDevice::permissions() const
{
   Q_D(const QFileDevice);
   QAbstractFileEngine::FileFlags perms = d->engine()->fileFlags(QAbstractFileEngine::PermsMask) &
                                          QAbstractFileEngine::PermsMask;
   return QFile::Permissions((int)perms); //ewww
}

/*!
    Sets the permissions for the file to the \a permissions specified.
    Returns true if successful, or false if the permissions cannot be
    modified.

    \sa permissions()
*/
bool QFileDevice::setPermissions(Permissions permissions)
{
   Q_D(QFileDevice);
   if (d->engine()->setPermissions(permissions)) {
      unsetError();
      return true;
   }
   d->setError(QFile::PermissionsError, d->fileEngine->errorString());
   return false;
}

/*!
    \enum QFileDevice::MemoryMapFlags
    \since 4.4

    This enum describes special options that may be used by the map()
    function.

    \value NoOptions        No options.
*/

/*!
    Maps \a size bytes of the file into memory starting at \a offset.  A file
    should be open for a map to succeed but the file does not need to stay
    open after the memory has been mapped.  When the QFile is destroyed
    or a new file is opened with this object, any maps that have not been
    unmapped will automatically be unmapped.

    Any mapping options can be passed through \a flags.

    Returns a pointer to the memory or 0 if there is an error.

    \note On Windows CE 5.0 the file will be closed before mapping occurs.

    \sa unmap()
 */
uchar *QFileDevice::map(qint64 offset, qint64 size, MemoryMapFlags flags)
{
   Q_D(QFileDevice);
   if (d->engine()
         && d->fileEngine->supportsExtension(QAbstractFileEngine::MapExtension)) {
      unsetError();
      uchar *address = d->fileEngine->map(offset, size, flags);
      if (address == 0) {
         d->setError(d->fileEngine->error(), d->fileEngine->errorString());
      }
      return address;
   }
   return 0;
}

/*!
    Unmaps the memory \a address.

    Returns true if the unmap succeeds; false otherwise.

    \sa map()
 */
bool QFileDevice::unmap(uchar *address)
{
   Q_D(QFileDevice);
   if (d->engine()
         && d->fileEngine->supportsExtension(QAbstractFileEngine::UnMapExtension)) {
      unsetError();
      bool success = d->fileEngine->unmap(address);
      if (!success) {
         d->setError(d->fileEngine->error(), d->fileEngine->errorString());
      }
      return success;
   }
   d->setError(PermissionsError, tr("No file engine available or engine does not support UnMapExtension"));
   return false;
}

QT_END_NAMESPACE
