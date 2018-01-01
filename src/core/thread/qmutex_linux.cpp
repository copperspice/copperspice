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

#include <qplatformdefs.h>
#include <qmutex.h>

#ifndef QT_NO_THREAD
#include <qatomic.h>
#include <qmutex_p.h>
#include <qelapsedtimer.h>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>

QT_BEGIN_NAMESPACE

static inline int _q_futex(void *addr, int op, int val, const struct timespec *timeout)
{
   volatile int *int_addr = reinterpret_cast<volatile int *>(addr);

#if (Q_BYTE_ORDER == Q_BIG_ENDIAN) && (UINTPTR_MAX == UINT64_MAX)
   // want a pointer to the 32 least significant bits of QMutex::d
   int_addr++;
#endif

   int *addr2 = 0;
   int val2 = 0;
   return syscall(SYS_futex, int_addr, op, val, timeout, addr2, val2);
}

static inline QMutexData *dummyFutexValue()
{
   return reinterpret_cast<QMutexData *>(quintptr(3));
}

bool QBasicMutex::lockInternal(int timeout)
{
   QElapsedTimer elapsedTimer;
   if (timeout >= 1) {
      elapsedTimer.start();
   }

   while (!fastTryLock()) {
      QMutexData *d = d_ptr.load();
      if (!d) { // if d is 0, the mutex is unlocked
         continue;
      }

      if (quintptr(d) <= 0x3) { //d == dummyLocked() || d == dummyFutexValue()
         if (timeout == 0) {
            return false;
         }
         while (this->d_ptr.fetchAndStoreAcquire(dummyFutexValue()) != 0) {
            struct timespec ts, *pts = 0;
            if (timeout >= 1) {
               // recalculate the timeout
               qint64 xtimeout = qint64(timeout) * 1000 * 1000;
               xtimeout -= elapsedTimer.nsecsElapsed();
               if (xtimeout <= 0) {
                  // timer expired after we returned
                  return false;
               }
               ts.tv_sec = xtimeout / Q_INT64_C(1000) / 1000 / 1000;
               ts.tv_nsec = xtimeout % (Q_INT64_C(1000) * 1000 * 1000);
               pts = &ts;
            }
            int r = _q_futex(&this->d_ptr, FUTEX_WAIT, quintptr(dummyFutexValue()), pts);
            if (r != 0 && errno == ETIMEDOUT) {
               return false;
            }
         }
         return true;
      }
      Q_ASSERT(d->recursive);
      return static_cast<QRecursiveMutexPrivate *>(d)->lock(timeout);
   }
   Q_ASSERT(this->d_ptr.load());
   return true;
}

void QBasicMutex::unlockInternal()
{
   QMutexData *d = d_ptr.load();
   Q_ASSERT(d); //we must be locked
   Q_ASSERT(d != dummyLocked()); // testAndSetRelease(dummyLocked(), 0) failed

   if (d == dummyFutexValue()) {
      this->d_ptr.fetchAndStoreRelease(0);
      _q_futex(&this->d_ptr, FUTEX_WAKE, 1, 0);
      return;
   }

   Q_ASSERT(d->recursive);
   static_cast<QRecursiveMutexPrivate *>(d)->unlock();
}


QT_END_NAMESPACE

#endif // QT_NO_THREAD
