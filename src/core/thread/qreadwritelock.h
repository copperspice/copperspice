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

#ifndef QREADWRITELOCK_H
#define QREADWRITELOCK_H

#include <qassert.h>
#include <qglobal.h>

struct QReadWriteLockPrivate;

class Q_CORE_EXPORT QReadWriteLock
{
 public:
   enum RecursionMode {
      NonRecursive,
      Recursive
   };

   explicit QReadWriteLock(RecursionMode recursionMode = NonRecursive);

   QReadWriteLock(const QReadWriteLock &) = delete;
   QReadWriteLock &operator=(const QReadWriteLock &) = delete;

   ~QReadWriteLock();

   void lockForRead();
   bool tryLockForRead();
   bool tryLockForRead(int timeout);

   void lockForWrite();
   bool tryLockForWrite();
   bool tryLockForWrite(int timeout);

   void unlock();

 private:
   QReadWriteLockPrivate *d;

   friend class QWaitCondition;
};

class Q_CORE_EXPORT QReadLocker
{
 public:
   inline QReadLocker(QReadWriteLock *lock);

   QReadLocker(const QReadLocker &) = delete;
   QReadLocker &operator=(const QReadLocker &) = delete;

   ~QReadLocker()
   {
      unlock();
   }

   void unlock() {
      if (q_val) {
         if ((q_val & quintptr(1u)) == quintptr(1u)) {
            q_val &= ~quintptr(1u);
            readWriteLock()->unlock();
         }
      }
   }

   void relock() {
      if (q_val) {
         if ((q_val & quintptr(1u)) == quintptr(0u)) {
            readWriteLock()->lockForRead();
            q_val |= quintptr(1u);
         }
      }
   }

   QReadWriteLock *readWriteLock() const {
      return reinterpret_cast<QReadWriteLock *>(q_val & ~quintptr(1u));
   }

 private:
   quintptr q_val;
};

inline QReadLocker::QReadLocker(QReadWriteLock *lock)
   : q_val(reinterpret_cast<quintptr>(lock))
{
   Q_ASSERT_X((q_val & quintptr(1u)) == quintptr(0), "QReadLocker", "QReadWriteLock pointer is misaligned");
   relock();
}

class Q_CORE_EXPORT QWriteLocker
{
 public:
   inline QWriteLocker(QReadWriteLock *lock);

   QWriteLocker(const QWriteLocker &) = delete;
   QWriteLocker &operator=(const QWriteLocker &) = delete;

   ~QWriteLocker() {
      unlock();
   }

   void unlock() {
      if (q_val) {
         if ((q_val & quintptr(1u)) == quintptr(1u)) {
            q_val &= ~quintptr(1u);
            readWriteLock()->unlock();
         }
      }
   }

   void relock() {
      if (q_val) {
         if ((q_val & quintptr(1u)) == quintptr(0u)) {
            readWriteLock()->lockForWrite();
            q_val |= quintptr(1u);
         }
      }
   }

   QReadWriteLock *readWriteLock() const {
      return reinterpret_cast<QReadWriteLock *>(q_val & ~quintptr(1u));
   }

 private:
   quintptr q_val;
};

inline QWriteLocker::QWriteLocker(QReadWriteLock *lock)
   : q_val(reinterpret_cast<quintptr>(lock))
{
   Q_ASSERT_X((q_val & quintptr(1u)) == quintptr(0), "QWriteLocker", "QReadWriteLock pointer is misaligned");
   relock();
}

#endif // QREADWRITELOCK_H
