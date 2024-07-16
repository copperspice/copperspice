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

#include <qiodevice_p.h>

#include <qbytearray.h>
#include <qdebug.h>
#include <qfile.h>
#include <qstringlist.h>

#include <algorithm>
#include <ctype.h>
#include <limits.h>

#define CHECK_MAXLEN(function, returnType) \
   do { \
      if (maxSize < 0) { \
         qWarning("QIODevice::"#function" Called with maxSize less than 0"); \
         return returnType; \
      } \
   } while (false)

#define CHECK_WRITABLE(function, returnType) \
   do { \
      if ((d->openMode & WriteOnly) == 0) { \
         if (d->openMode == NotOpen) \
            return returnType; \
         qWarning("QIODevice::"#function": ReadOnly device"); \
         return returnType; \
      } \
   } while (false)

bool QIODevicePrivate::check_readable() const
{
   if ((openMode & QIODevice::OpenModeFlag::ReadOnly) == 0) {
      if (openMode == QIODevice::OpenModeFlag::NotOpen) {
         return false;
      }

      qWarning("QIODevice::check_readable() WriteOnly device");
      return false;
   }

   return true;
}

QIODevicePrivate::QIODevicePrivate()
   : openMode(QIODevice::NotOpen), buffer(QIODEVICE_BUFFERSIZE),
     pos(0), devicePos(0), seqDumpPos(0), pPos(&pos), pDevicePos(&devicePos),
     baseReadLineDataCalled(false), firstRead(true), accessMode(Unset)
{
}

QIODevicePrivate::~QIODevicePrivate()
{
}

QIODevice::QIODevice()
   : QObject(nullptr), d_ptr(new QIODevicePrivate)
{
   d_ptr->q_ptr = this;
}

QIODevice::QIODevice(QObject *parent)
   : QObject(parent), d_ptr(new QIODevicePrivate)
{
   d_ptr->q_ptr = this;
}

QIODevice::QIODevice(QIODevicePrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QIODevice::~QIODevice()
{
}

bool QIODevice::isSequential() const
{
   return false;
}

QIODevice::OpenMode QIODevice::openMode() const
{
   return d_func()->openMode;
}

void QIODevice::setOpenMode(OpenMode openMode)
{
   Q_D(QIODevice);

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::setOpenMode(0x%x) Device = %p", int(openMode), this);
#endif

   d->openMode   = openMode;
   d->accessMode = QIODevicePrivate::Unset;
   d->firstRead  = true;

   if (! isReadable()) {
      d->buffer.clear();
   }
}

void QIODevice::setTextModeEnabled(bool enabled)
{
   Q_D(QIODevice);

   if (! isOpen()) {
      qWarning("QIODevice::setTextModeEnabled() Device is not open");
      return;
   }

   if (enabled) {
      d->openMode |= Text;
   } else {
      d->openMode &= ~Text;
   }
}

bool QIODevice::isTextModeEnabled() const
{
   return d_func()->openMode & Text;
}

bool QIODevice::isOpen() const
{
   return d_func()->openMode != NotOpen;
}

bool QIODevice::isReadable() const
{
   return (openMode() & ReadOnly) != 0;
}

bool QIODevice::isWritable() const
{
   return (openMode() & WriteOnly) != 0;
}

bool QIODevice::open(OpenMode mode)
{
   Q_D(QIODevice);

   d->openMode = mode;
   d->pos      = (mode & Append) ? size() : qint64(0);

   d->buffer.clear();
   d->accessMode = QIODevicePrivate::Unset;
   d->firstRead  = true;

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::open(0x%x) Device = %p", quint32(mode), this);
#endif

   return true;
}

void QIODevice::close()
{
   Q_D(QIODevice);

   if (d->openMode == NotOpen) {
      return;
   }

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::close() Device = %p", this);
#endif

   emit aboutToClose();

   d->openMode = NotOpen;
   d->errorString.clear();
   d->pos = 0;
   d->seqDumpPos = 0;
   d->buffer.clear();
   d->firstRead = true;
}

qint64 QIODevice::pos() const
{
   Q_D(const QIODevice);

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::pos() Device = %p, pos = %d", this, int(d->pos));
#endif

   return d->pos;
}

qint64 QIODevice::size() const
{
   return d_func()->isSequential() ?  bytesAvailable() : qint64(0);
}

bool QIODevice::seek(qint64 pos)
{
   Q_D(QIODevice);

   if (d->openMode == NotOpen) {
      qWarning("QIODevice::seek() Device is not open");
      return false;
   }

   if (pos < 0) {
      qWarning("QIODevice::seek() Invalid position, %d", int(pos));
      return false;
   }

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::seek(%d) Device = %p, before: pos = %d, buffer.size() = %d",
         int(pos), this, int(d->pos), d->buffer.size());
#endif

   qint64 offset = pos - d->pos;

   if (! d->isSequential()) {
      d->pos = pos;
      d->devicePos = pos;
   }

   if (offset < 0 || offset >= qint64(d->buffer.size())) {
      // When seeking backwards, an operation that is only allowed for
      // random-access devices, the buffer is cleared. The next read
      // operation will then refill the buffer. We can optimize this, if we
      // find that seeking backwards becomes a significant performance hit.

      d->buffer.clear();

   } else if (! d->buffer.isEmpty()) {
      d->buffer.skip(int(offset));
   }

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::seek() Device = %p, pos == %d, buffer.size() == %d",
         this, int(d->pos), d->buffer.size());
#endif

   return true;
}

bool QIODevice::atEnd() const
{
   Q_D(const QIODevice);

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::atEnd() Device = %p, returns %s, openMode == %d, pos == %d",
      this, (d->openMode == NotOpen || d->pos == size()) ? "true" : "false", int(d->openMode), int(d->pos));
#endif

   return d->openMode == NotOpen || (d->buffer.isEmpty() && bytesAvailable() == 0);
}

bool QIODevice::reset()
{
#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::reset() Device = %p", this);
#endif

   return seek(0);
}

qint64 QIODevice::bytesAvailable() const
{
   Q_D(const QIODevice);

   if (! d->isSequential()) {
      return qMax(size() - d->pos, qint64(0));
   }

   return d->buffer.size();
}

qint64 QIODevice::bytesToWrite() const
{
   return qint64(0);
}

qint64 QIODevice::read(char *data, qint64 maxSize)
{
   Q_D(QIODevice);

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::read(%p, %d) Device = %p, pos = %d, buffer.size() = %d",
         data, int(maxSize), this, int(d->pos), int(d->buffer.size()));
#endif

   // Short circuit for getChar()
   if (maxSize == 1) {
      int chint;

      while ((chint = d->buffer.getChar()) != -1) {
         ++(*d->pPos);

         char c = char(uchar(chint));

         if (c == '\r' && (d->openMode & Text)) {
            continue;
         }

         *data = c;

         return qint64(1);
      }
   }

   CHECK_MAXLEN(read, qint64(-1));
   qint64 readSoFar = 0;
   bool moreToRead = true;

   do {
      // Try reading from the buffer.
      int lastReadChunkSize = d->buffer.read(data, maxSize);

      if (lastReadChunkSize > 0) {
         *d->pPos += lastReadChunkSize;
         readSoFar += lastReadChunkSize;

         // fast exit when satisfied by buffer
         if (lastReadChunkSize == maxSize && !(d->openMode & Text)) {
            return readSoFar;
         }

         data += lastReadChunkSize;
         maxSize -= lastReadChunkSize;

      } else {
         if (d->firstRead) {
            // first time file has been read, check if valid

            if (! d->check_readable()) {
               return qint64(-1);
            }

            // set up pos pointers
            d->firstRead = false;

            if (d->isSequential()) {
               d->pPos = &d->seqDumpPos;
               d->pDevicePos = &d->seqDumpPos;
            }
         }

         if (! maxSize) {
            return readSoFar;
         }

         if ((d->openMode & Unbuffered) == 0 && maxSize < QIODEVICE_BUFFERSIZE) {
            // In buffered mode, we try to fill up the QIODevice buffer before we do anything else.
            // buffer is empty at this point, try to fill it

            int bytesToBuffer = QIODEVICE_BUFFERSIZE;
            char *writePointer = d->buffer.reserve(bytesToBuffer);

            // Make sure the device is positioned correctly.
            if (d->pos != d->devicePos && ! d->isSequential() && ! seek(d->pos)) {
               return readSoFar ? readSoFar : qint64(-1);
            }

            qint64 readFromDevice = readData(writePointer, bytesToBuffer);
            d->buffer.chop(bytesToBuffer - (readFromDevice < 0 ? 0 : int(readFromDevice)));

            if (readFromDevice > 0) {
               *d->pDevicePos += readFromDevice;

               if (! d->buffer.isEmpty()) {
                  lastReadChunkSize = d->buffer.read(data, maxSize);
                  readSoFar += lastReadChunkSize;
                  data      += lastReadChunkSize;
                  maxSize   -= lastReadChunkSize;
                  *d->pPos  += lastReadChunkSize;
               }
            }
         }
      }

      // If we need more try reading from the device
      if (maxSize > 0) {
         // Make sure the device is positioned correctly.
         if (d->pos != d->devicePos && !d->isSequential() && !seek(d->pos)) {
            return readSoFar ? readSoFar : qint64(-1);
         }

         qint64 readFromDevice = readData(data, maxSize);

         if (readFromDevice == -1 && readSoFar == 0) {
            // error and we haven't read anything: return immediately
            return -1;
         }

         if (readFromDevice > 0) {
            lastReadChunkSize += int(readFromDevice);
            readSoFar += readFromDevice;
            data      += readFromDevice;
            maxSize   -= readFromDevice;
            *d->pPos  += readFromDevice;
            *d->pDevicePos += readFromDevice;
         }
      }

      // Best attempt has been made to read data, don't try again except for text mode adjustment below
      moreToRead = false;

      if (readSoFar && d->openMode & Text) {
         char *readPtr = data - lastReadChunkSize;
         const char *endPtr = data;

         if (readPtr < endPtr) {
            // optimization to avoid initial self-assignment
            while (*readPtr != '\r') {
               if (++readPtr == endPtr) {
                  return readSoFar;
               }
            }

            char *writePtr = readPtr;

            while (readPtr < endPtr) {
               char ch = *readPtr++;

               if (ch != '\r') {
                  *writePtr++ = ch;
               } else {
                  --readSoFar;
                  --data;
                  ++maxSize;
               }
            }

            // Make sure we get more data if there is room for more. This
            // is very important for when someone seeks to the start of a
            // '\r\n' and reads one character - they should get the '\n'.
            moreToRead = (readPtr != writePtr);
         }
      }

   } while (moreToRead);

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::read() Device = %p, read %d, pos == %d, buffer.size() == %d",
         this, int(readSoFar), int(d->pos), d->buffer.size());
#endif

   return readSoFar;
}

QByteArray QIODevice::read(qint64 maxSize)
{
   Q_D(QIODevice);

   QByteArray result;
   CHECK_MAXLEN(read, result);

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::read(%d) device = %p, pos = %d, buffer.size() = %d",
         int(maxSize), this, int(d->pos), int(d->buffer.size()));
#else
   (void) d;
#endif

   if (maxSize != qint64(int(maxSize))) {
      qWarning("QIODevice::read(maxSize) Passed size exceeds QByteArray size limit");
      maxSize = INT_MAX;
   }

   qint64 readBytes = 0;

   if (maxSize) {
      result.resize(int(maxSize));

      if (! result.size()) {
         // If resize fails, read incrementally.
         qint64 readResult;

         do {
            result.resize(int(qMin(maxSize, result.size() + QIODEVICE_BUFFERSIZE)));
            readResult = read(result.data() + readBytes, result.size() - readBytes);

            if (readResult > 0 || readBytes == 0) {
               readBytes += readResult;
            }
         } while (readResult == QIODEVICE_BUFFERSIZE);

      } else {
         readBytes = read(result.data(), result.size());
      }
   }

   if (readBytes <= 0) {
      result.clear();
   } else {
      result.resize(int(readBytes));
   }

   return result;
}

QByteArray QIODevice::readAll()
{
   Q_D(QIODevice);

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::readAll() Device = %p, pos = %d, buffer.size() = %d",
         this, int(d->pos), int(d->buffer.size()));
#endif

   QByteArray result;
   qint64 readBytes = 0;

   // flush internal read buffer
   if (! (d->openMode & Text) && !d->buffer.isEmpty()) {
      result = d->buffer.readAll();
      readBytes = result.size();
      d->pos += readBytes;
   }

   qint64 theSize;

   if (d->isSequential() || (theSize = size()) == 0) {
      // Size is unknown, read incrementally.
      qint64 readResult;

      do {
         result.resize(result.size() + QIODEVICE_BUFFERSIZE);
         readResult = read(result.data() + readBytes, result.size() - readBytes);

         if (readResult > 0 || readBytes == 0) {
            readBytes += readResult;
         }

      } while (readResult > 0);

   } else {
      // Read it all in one go.
      // If resize fails, don't read anything.
      result.resize(int(readBytes + theSize - d->pos));
      readBytes += read(result.data() + readBytes, result.size() - readBytes);
   }

   if (readBytes <= 0) {
      result.clear();
   } else {
      result.resize(int(readBytes));
   }

   return result;
}

qint64 QIODevice::readLine(char *data, qint64 maxSize)
{
   Q_D(QIODevice);

   if (maxSize < 2) {
      qWarning("QIODevice::readLine() Unable to call with maxSize < 2");
      return qint64(-1);
   }

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::readLine(%p, %d) Device = %p, pos = %d, buffer.size() = %d",
         data, int(maxSize), this, int(d->pos), int(d->buffer.size()));
#endif

   // Leave room for a '\0'
   --maxSize;

   const bool sequential = d->isSequential();

   qint64 readSoFar = 0;

   if (! d->buffer.isEmpty()) {
      readSoFar = d->buffer.readLine(data, maxSize);

      if (! sequential) {
         d->pos += readSoFar;
      }

#if defined(CS_SHOW_DEBUG_CORE_IO)
      qDebug("QIODevice::readLine() Device = %p, read from buffer = %d bytes, last character read = %hhx",
            this, int(readSoFar), data[int(readSoFar) - 1]);
#endif

      if (readSoFar && data[readSoFar - 1] == '\n') {
         if (d->openMode & Text) {
            // QRingBuffer::readLine() isn't Text aware.
            if (readSoFar > 1 && data[readSoFar - 2] == '\r') {
               --readSoFar;
               data[readSoFar - 1] = '\n';
            }
         }

         data[readSoFar] = '\0';
         return readSoFar;
      }
   }

   if (d->pos != d->devicePos && ! sequential && !seek(d->pos)) {
      return qint64(-1);
   }

   d->baseReadLineDataCalled = false;
   qint64 readBytes = readLineData(data + readSoFar, maxSize - readSoFar);

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::readLine() Device = %p, read from readLineData = %d bytes, readSoFar = %d bytes",
         this, int(readBytes), int(readSoFar));
#endif

   if (readBytes < 0) {
      data[readSoFar] = '\0';
      return readSoFar ? readSoFar : -1;
   }

   readSoFar += readBytes;

   if (!d->baseReadLineDataCalled && !sequential) {
      d->pos += readBytes;

      // If the base implementation was not called, then we must
      // assume the device position is invalid and force a seek.

      d->devicePos = qint64(-1);
   }

   data[readSoFar] = '\0';

   if (d->openMode & Text) {

      if (readSoFar > 1 && data[readSoFar - 1] == '\n' && data[readSoFar - 2] == '\r') {
         data[readSoFar - 2] = '\n';
         data[readSoFar - 1] = '\0';
         --readSoFar;
      }
   }

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::readLine() Device = %p, returning %d, pos = %d, buffer.size() = %d, size() = %d",
         this, int(readSoFar), int(d->pos), d->buffer.size(), int(size()));
#endif

   return readSoFar;
}

QByteArray QIODevice::readLine(qint64 maxSize)
{
   QByteArray result;

   CHECK_MAXLEN(readLine, result);

#if defined(CS_SHOW_DEBUG_CORE_IO)
   Q_D(QIODevice);

   qDebug("QIODevice::readLine(%d) Device = %p, pos = %d, buffer.size() = %d",
         int(maxSize), this, int(d->pos), int(d->buffer.size()));
#endif

   if (maxSize > INT_MAX) {
      qWarning("QIODevice::readLine() maxSize exceeds QByteArray size limit");
      maxSize = INT_MAX;
   }

   result.resize(int(maxSize));
   qint64 readBytes = 0;

   if (! result.size()) {
      // If resize fails or maxSize == 0, read incrementally
      if (maxSize == 0) {
         maxSize = INT_MAX;
      }

      // The first iteration needs to leave an extra byte for the terminating null
      result.resize(1);

      qint64 readResult;

      do {
         result.resize(int(qMin(maxSize, result.size() + QIODEVICE_BUFFERSIZE)));
         readResult = readLine(result.data() + readBytes, result.size() - readBytes);

         if (readResult > 0 || readBytes == 0) {
            readBytes += readResult;
         }

      } while (readResult == QIODEVICE_BUFFERSIZE && result[int(readBytes - 1)] != '\n');

   } else {
      readBytes = readLine(result.data(), result.size());
   }

   if (readBytes <= 0) {
      result.clear();
   } else {
      result.resize(readBytes);
   }

   return result;
}

qint64 QIODevice::readLineData(char *data, qint64 maxSize)
{
   Q_D(QIODevice);

   qint64 readSoFar = 0;
   char c;
   int lastReadReturn = 0;
   d->baseReadLineDataCalled = true;

   while (readSoFar < maxSize && (lastReadReturn = read(&c, 1)) == 1) {
      *data++ = c;
      ++readSoFar;

      if (c == '\n') {
         break;
      }
   }

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::readLineData(%p, %d) Device = %p, d->pos = %d, d->buffer.size() = %d, returns %d",
         data, int(maxSize), this, int(d->pos), int(d->buffer.size()), int(readSoFar));
#endif

   if (lastReadReturn != 1 && readSoFar == 0) {
      return isSequential() ? lastReadReturn : -1;
   }

   return readSoFar;
}

bool QIODevice::canReadLine() const
{
   return d_func()->buffer.canReadLine();
}

qint64 QIODevice::write(const char *data, qint64 maxSize)
{
   Q_D(QIODevice);

   CHECK_WRITABLE(write, qint64(-1));
   CHECK_MAXLEN(write, qint64(-1));

   const bool sequential = d->isSequential();

   // Make sure the device is positioned correctly.
   if (d->pos != d->devicePos && !sequential && !seek(d->pos)) {
      return qint64(-1);
   }

#ifdef Q_OS_WIN

   if (d->openMode & Text) {
      const char *endOfData    = data + maxSize;
      const char *startOfBlock = data;

      qint64 writtenSoFar = 0;

      while (true) {
         const char *endOfBlock = startOfBlock;

         while (endOfBlock < endOfData && *endOfBlock != '\n') {
            ++endOfBlock;
         }

         qint64 blockSize = endOfBlock - startOfBlock;

         if (blockSize > 0) {
            qint64 ret = writeData(startOfBlock, blockSize);

            if (ret <= 0) {
               if (writtenSoFar && !sequential) {
                  d->buffer.skip(writtenSoFar);
               }

               return writtenSoFar ? writtenSoFar : ret;
            }

            if (! sequential) {
               d->pos += ret;
               d->devicePos += ret;
            }

            writtenSoFar += ret;
         }

         if (endOfBlock == endOfData) {
            break;
         }

         qint64 ret = writeData("\r\n", 2);

         if (ret <= 0) {
            if (writtenSoFar && !sequential) {
               d->buffer.skip(writtenSoFar);
            }

            return writtenSoFar ? writtenSoFar : ret;
         }

         if (! sequential) {
            d->pos += ret;
            d->devicePos += ret;
         }

         ++writtenSoFar;

         startOfBlock = endOfBlock + 1;
      }

      if (writtenSoFar && !sequential) {
         d->buffer.skip(writtenSoFar);
      }

      return writtenSoFar;
   }

#endif

   qint64 written = writeData(data, maxSize);

   if (written > 0) {
      if (! sequential) {
         d->pos += written;
         d->devicePos += written;
      }

      if (! d->buffer.isEmpty() && ! sequential) {
         d->buffer.skip(written);
      }
   }

   return written;
}

qint64 QIODevice::write(const char *data)
{
   return write(data, qstrlen(data));
}

void QIODevice::ungetChar(char c)
{
   Q_D(QIODevice);

   if (! d->check_readable()) {
      return;
   }

#if defined(CS_SHOW_DEBUG_CORE_IO)
   qDebug("QIODevice::ungetChar(0x%hhx '%c') Device = %p", c, isprint(c) ? c : '?', this);
#endif

   d->buffer.ungetChar(c);

   if (! d->isSequential()) {
      --(d->pos);
   }
}

bool QIODevice::putChar(char c)
{
   return d_func()->putCharHelper(c);
}

bool QIODevicePrivate::putCharHelper(char c)
{
   return q_func()->write(&c, 1) == 1;
}

qint64 QIODevicePrivate::peek(char *data, qint64 maxSize)
{
   qint64 readBytes = q_func()->read(data, maxSize);

   if (readBytes <= 0) {
      return readBytes;
   }

   buffer.ungetBlock(data, readBytes);
   *pPos -= readBytes;

   return readBytes;
}

QByteArray QIODevicePrivate::peek(qint64 maxSize)
{
   QByteArray result = q_func()->read(maxSize);

   if (result.isEmpty()) {
      return result;
   }

   buffer.ungetBlock(result.constData(), result.size());
   *pPos -= result.size();

   return result;
}

bool QIODevice::getChar(char *c)
{
   // readability checked in read()
   char ch;
   return (1 == read(c ? c : &ch, 1));
}

qint64 QIODevice::peek(char *data, qint64 maxSize)
{
   return d_func()->peek(data, maxSize);
}

QByteArray QIODevice::peek(qint64 maxSize)
{
   return d_func()->peek(maxSize);
}

bool QIODevice::waitForReadyRead(int msecs)
{
   (void) msecs;
   return false;
}

bool QIODevice::waitForBytesWritten(int msecs)
{
   (void) msecs;
   return false;
}

void QIODevice::setErrorString(const QString &str)
{
   d_func()->errorString = str;
}

QString QIODevice::errorString() const
{
   Q_D(const QIODevice);

   if (d->errorString.isEmpty()) {
      return tr("Unknown error");
   }

   return d->errorString;
}

int qt_subtract_from_timeout(int timeout, int elapsed)
{
   if (timeout == -1) {
      return -1;
   }

   timeout = timeout - elapsed;

   return timeout < 0 ? 0 : timeout;
}

QDebug operator<<(QDebug debug, QIODevice::OpenMode modes)
{
   debug << "OpenMode(";
   QStringList modeList;

   if (modes == QIODevice::NotOpen) {
      modeList << QLatin1String("NotOpen");

   } else {
      if (modes & QIODevice::ReadOnly) {
         modeList << QLatin1String("ReadOnly");
      }

      if (modes & QIODevice::WriteOnly) {
         modeList << QLatin1String("WriteOnly");
      }

      if (modes & QIODevice::Append) {
         modeList << QLatin1String("Append");
      }

      if (modes & QIODevice::Truncate) {
         modeList << QLatin1String("Truncate");
      }

      if (modes & QIODevice::Text) {
         modeList << QLatin1String("Text");
      }

      if (modes & QIODevice::Unbuffered) {
         modeList << QLatin1String("Unbuffered");
      }
   }

   std::sort(modeList.begin(), modeList.end());
   debug << modeList.join("|");
   debug << ')';

   return debug;
}
