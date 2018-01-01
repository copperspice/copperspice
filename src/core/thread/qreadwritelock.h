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

#ifndef QREADWRITELOCK_H
#define QREADWRITELOCK_H

#include <qassert.h>

struct QReadWriteLockPrivate;

class Q_CORE_EXPORT QReadWriteLock
{
 public:
   enum RecursionMode { NonRecursive, Recursive };

   explicit QReadWriteLock(RecursionMode recursionMode = NonRecursive);
   ~QReadWriteLock();

   void lockForRead();
   bool tryLockForRead();
   bool tryLockForRead(int timeout);

   void lockForWrite();
   bool tryLockForWrite();
   bool tryLockForWrite(int timeout);

   void unlock();

 private:
   Q_DISABLE_COPY(QReadWriteLock)
   QReadWriteLockPrivate *d;

   friend class QWaitCondition;
};

class Q_CORE_EXPORT QReadLocker
{
 public:
   inline QReadLocker(QReadWriteLock *readWriteLock);

   inline ~QReadLocker() {
      unlock();
   }

   inline void unlock() {
      if (q_val) {
         if ((q_val & quintptr(1u)) == quintptr(1u)) {
            q_val &= ~quintptr(1u);
            readWriteLock()->unlock();
         }
      }
   }

   inline void relock() {
      if (q_val) {
         if ((q_val & quintptr(1u)) == quintptr(0u)) {
            readWriteLock()->lockForRead();
            q_val |= quintptr(1u);
         }
      }
   }

   inline QReadWriteLock *readWriteLock() const {
      return reinterpret_cast<QReadWriteLock *>(q_val & ~quintptr(1u));
   }

 private:
   Q_DISABLE_COPY(QReadLocker)
   quintptr q_val;
};

inline QReadLocker::QReadLocker(QReadWriteLock *areadWriteLock)
   : q_val(reinterpret_cast<quintptr>(areadWriteLock))
{
   Q_ASSERT_X((q_val & quintptr(1u)) == quintptr(0),
              "QReadLocker", "QReadWriteLock pointer is misaligned");
   relock();
}

class Q_CORE_EXPORT QWriteLocker
{
 public:
   inline QWriteLocker(QReadWriteLock *readWriteLock);

   inline ~QWriteLocker() {
      unlock();
   }

   inline void unlock() {
      if (q_val) {
         if ((q_val & quintptr(1u)) == quintptr(1u)) {
            q_val &= ~quintptr(1u);
            readWriteLock()->unlock();
         }
      }
   }

   inline void relock() {
      if (q_val) {
         if ((q_val & quintptr(1u)) == quintptr(0u)) {
            readWriteLock()->lockForWrite();
            q_val |= quintptr(1u);
         }
      }
   }

   inline QReadWriteLock *readWriteLock() const {
      return reinterpret_cast<QReadWriteLock *>(q_val & ~quintptr(1u));
   }


 private:
   Q_DISABLE_COPY(QWriteLocker)
   quintptr q_val;
};

inline QWriteLocker::QWriteLocker(QReadWriteLock *areadWriteLock)
   : q_val(reinterpret_cast<quintptr>(areadWriteLock))
{
   Q_ASSERT_X((q_val & quintptr(1u)) == quintptr(0),
              "QWriteLocker", "QReadWriteLock pointer is misaligned");
   relock();
}

#endif // QREADWRITELOCK_H
