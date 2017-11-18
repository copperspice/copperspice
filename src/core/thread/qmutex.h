/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QMUTEX_H
#define QMUTEX_H

#include <qglobal.h>
#include <qassert.h>
#include <qatomic.h>

#include <new>

class QMutexData;

class Q_CORE_EXPORT QBasicMutex
{
 public:
   inline void lock() {
      if (!fastTryLock()) {
         lockInternal();
      }
   }

   inline void unlock() {
      Q_ASSERT(d_ptr.load()); //mutex must be locked
      if (!d_ptr.testAndSetRelease(dummyLocked(), 0)) {
         unlockInternal();
      }
   }

   bool tryLock(int timeout = 0) {
      return fastTryLock() || lockInternal(timeout);
   }

   bool isRecursive();

 private:
   inline bool fastTryLock() {
      return d_ptr.testAndSetAcquire(0, dummyLocked());
   }
   bool lockInternal(int timeout = -1);
   void unlockInternal();

   QAtomicPointer<QMutexData> d_ptr;
   static inline QMutexData *dummyLocked() {
      return reinterpret_cast<QMutexData *>(quintptr(1));
   }

   friend class QMutex;
   friend class QMutexData;
};

class Q_CORE_EXPORT QMutex : public QBasicMutex
{
 public:
   enum RecursionMode { NonRecursive, Recursive };
   explicit QMutex(RecursionMode mode = NonRecursive);
   ~QMutex();

 private:
   Q_DISABLE_COPY(QMutex)
};

class Q_CORE_EXPORT QMutexLocker
{
 public:
   inline explicit QMutexLocker(QBasicMutex *m) {
      Q_ASSERT_X((reinterpret_cast<quintptr>(m) & quintptr(1u)) == quintptr(0),
                 "QMutexLocker", "QMutex pointer is misaligned");
      if (m) {
         m->lock();
         val = reinterpret_cast<quintptr>(m) | quintptr(1u);
      } else {
         val = 0;
      }
   }
   inline ~QMutexLocker() {
      unlock();
   }

   inline void unlock() {
      if ((val & quintptr(1u)) == quintptr(1u)) {
         val &= ~quintptr(1u);
         mutex()->unlock();
      }
   }

   inline void relock() {
      if (val) {
         if ((val & quintptr(1u)) == quintptr(0u)) {
            mutex()->lock();
            val |= quintptr(1u);
         }
      }
   }

   inline QMutex *mutex() const {
      return reinterpret_cast<QMutex *>(val & ~quintptr(1u));
   }

 private:
   Q_DISABLE_COPY(QMutexLocker)

   quintptr val;
};

#endif // QMUTEX_H
