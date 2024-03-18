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

#ifndef QIODEVICE_P_H
#define QIODEVICE_P_H

#include <qiodevice.h>
#include <qbytearray.h>
#include <qstring.h>

#include <qringbuffer_p.h>

#ifndef QIODEVICE_BUFFERSIZE
#define QIODEVICE_BUFFERSIZE Q_INT64_C(16384)
#endif

Q_CORE_EXPORT int qt_subtract_from_timeout(int timeout, int elapsed);

// This is QIODevice's read buffer, optimized for read(), isEmpty() and getChar()
class QIODevicePrivateLinearBuffer
{
 public:
   QIODevicePrivateLinearBuffer(int)
      : len(0), first(nullptr), buf(nullptr), capacity(0) {
   }

   ~QIODevicePrivateLinearBuffer() {
      delete [] buf;
   }

   void clear() {
      first = buf;
      len = 0;
   }

   int size() const {
      return len;
   }

   bool isEmpty() const {
      return len == 0;
   }

   void skip(int n) {
      if (n >= len) {
         clear();
      } else {
         len -= n;
         first += n;
      }
   }

   int getChar() {
      if (len == 0) {
         return -1;
      }

      int ch = uchar(*first);
      len--;
      first++;
      return ch;
   }

   int read(char *target, int size) {
      int r = qMin(size, len);

      if (r == 0) {
         return 0;
      }

      memcpy(target, first, r);
      len   -= r;
      first += r;

      return r;
   }

   int peek(char *target, int size) {
      int r = qMin(size, len);
      memcpy(target, first, r);
      return r;
   }

   char *reserve(int size) {
      makeSpace(size + len, freeSpaceAtEnd);
      char *writePtr = first + len;
      len += size;
      return writePtr;
   }

   void chop(int size) {
      if (size >= len) {
         clear();
      } else {
         len -= size;
      }
   }

   QByteArray readAll() {
      char *f = first;
      int l   = len;
      clear();

      return QByteArray(f, l);
   }

   int readLine(char *target, int size) {
      int r = qMin(size, len);
      char *eol = static_cast<char *>(memchr(first, '\n', r));

      if (eol) {
         r = 1 + (eol - first);
      }

      memcpy(target, first, r);
      len -= r;
      first += r;
      return int(r);
   }

   bool canReadLine() const {
      return memchr(first, '\n', len);
   }

   void ungetChar(char c) {
      if (first == buf) {
         // underflow, the existing valid data needs to move to the end of the (potentially bigger) buffer
         makeSpace(len + 1, freeSpaceAtStart);
      }

      first--;
      len++;
      *first = c;
   }

   void ungetBlock(const char *block, int size) {
      if ((first - buf) < size) {
         // underflow, the existing valid data needs to move to the end of the (potentially bigger) buffer
         makeSpace(len + size, freeSpaceAtStart);
      }

      first -= size;
      len += size;
      memcpy(first, block, size);
   }

 private:
   enum FreeSpacePos {
      freeSpaceAtStart,
      freeSpaceAtEnd
   };

   void makeSpace(size_t required, FreeSpacePos where) {
      size_t newCapacity = qMax(capacity, size_t(QIODEVICE_BUFFERSIZE));

      while (newCapacity < required) {
         newCapacity *= 2;
      }

      int moveOffset = (where == freeSpaceAtEnd) ? 0 : newCapacity - len;

      if (newCapacity > capacity) {
         // allocate more space
         char *newBuf = new char[newCapacity];

         if (first != nullptr && len != 0) {
            memmove(newBuf + moveOffset, first, len);
         }

         delete [] buf;

         buf = newBuf;
         capacity = newCapacity;

      } else {
         // shift any existing data to make space
         memmove(buf + moveOffset, first, len);
      }

      first = buf + moveOffset;
   }

   // length of the unread data
   int len;

   // start of the unread data
   char *first;

   // the allocated buffer
   char *buf;

   // allocated buffer size
   size_t capacity;
};

class Q_CORE_EXPORT QIODevicePrivate
{
   Q_DECLARE_PUBLIC(QIODevice)

 public:
    enum AccessMode {
      Unset,
      Sequential,
      RandomAccess
   };

   QIODevicePrivate();
   virtual ~QIODevicePrivate();

   QIODevice::OpenMode openMode;
   QString errorString;

   QIODevicePrivateLinearBuffer buffer;
   qint64 pos;
   qint64 devicePos;

   // these three are for fast position updates during read, avoiding isSequential test
   qint64 seqDumpPos;
   qint64 *pPos;
   qint64 *pDevicePos;

   bool baseReadLineDataCalled;
   bool firstRead;

   virtual bool putCharHelper(char c);

   mutable AccessMode accessMode;

   bool isSequential() const {
      if (accessMode == Unset) {
         accessMode = q_func()->isSequential() ? Sequential : RandomAccess;
      }

      return accessMode == Sequential;
   }

   virtual qint64 peek(char *data, qint64 maxSize);
   virtual QByteArray peek(qint64 maxSize);

   bool check_readable() const;

 protected:
   QIODevice *q_ptr;
};

#endif
