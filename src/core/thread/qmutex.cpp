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
#include <qdebug.h>
#include <qatomic.h>
#include <qelapsedtimer.h>
#include <qthread.h>
#include <qmutex_p.h>

#ifndef Q_OS_LINUX
#include <qfreelist_p.h>
#endif

QT_BEGIN_NAMESPACE

QMutex::QMutex(RecursionMode mode)
{
   d_ptr.store(mode == Recursive ? new QRecursiveMutexPrivate : 0);
}

/*!
    Destroys the mutex.

    \warning Destroying a locked mutex may result in undefined behavior.
*/
QMutex::~QMutex()
{
   QMutexData *d = d_ptr.load();
   if (quintptr(d) > 0x3 && d->recursive) {
      delete static_cast<QRecursiveMutexPrivate *>(d);
   } else if (d) {
#ifndef Q_OS_LINUX
      if (d != dummyLocked() && static_cast<QMutexPrivate *>(d)->possiblyUnlocked.load()
            && tryLock()) {
         unlock();
         return;
      }
#endif
      qWarning("QMutex: destroying locked mutex");
   }
}

bool QBasicMutex::isRecursive()
{
   QMutexData *d = d_ptr.load();
   if (quintptr(d) <= 0x3) {
      return false;
   }
   return d->recursive;
}

#ifndef Q_OS_LINUX //linux implementation is in qmutex_linux.cpp
/*!
    \internal helper for lock()
 */
bool QBasicMutex::lockInternal(int timeout)
{
   while (!fastTryLock()) {
      QMutexData *copy = d_ptr.loadAcquire();
      if (!copy) { // if d is 0, the mutex is unlocked
         continue;
      }

      if (copy == dummyLocked()) {
         if (timeout == 0) {
            return false;
         }
         QMutexPrivate *newD = QMutexPrivate::allocate();
         if (!d_ptr.testAndSetOrdered(dummyLocked(), newD)) {
            //Either the mutex is already unlocked, or another thread already set it.
            newD->deref();
            continue;
         }
         copy = newD;
         //the d->refCount is already 1 the deref will occurs when we unlock
      } else if (copy->recursive) {
         return static_cast<QRecursiveMutexPrivate *>(copy)->lock(timeout);
      }

      QMutexPrivate *d = static_cast<QMutexPrivate *>(copy);
      if (timeout == 0 && !d->possiblyUnlocked.load()) {
         return false;
      }

      if (!d->ref()) {
         continue;   //that QMutexData was already released
      }

      if (d != d_ptr.loadAcquire()) {
         //Either the mutex is already unlocked, or relocked with another mutex
         d->deref();
         continue;
      }

      int old_waiters;
      do {
         old_waiters = d->waiters.load();
         if (old_waiters == -QMutexPrivate::BigNumber) {
            // we are unlocking, and the thread that unlocks is about to change d to 0
            // we try to aquire the mutex by changing to dummyLocked()
            if (d_ptr.testAndSetAcquire(d, dummyLocked())) {
               // Mutex acquired
               d->deref();
               return true;
            } else {
               Q_ASSERT(d != d_ptr.load()); //else testAndSetAcquire should have succeeded
               // Mutex is likely to bo 0, we should continue the outer-loop,
               //  set old_waiters to the magic value of BigNumber
               old_waiters = QMutexPrivate::BigNumber;
               break;
            }
         }
      } while (!d->waiters.testAndSetRelaxed(old_waiters, old_waiters + 1));

      if (d != d_ptr.loadAcquire()) {
         // Mutex was unlocked.
         if (old_waiters != QMutexPrivate::BigNumber) {
            //we did not break the previous loop
            Q_ASSERT(d->waiters.load() >= 1);
            d->waiters.deref();
         }
         d->deref();
         continue;
      }

      if (d->wait(timeout)) {
         if (d->possiblyUnlocked.load() && d->possiblyUnlocked.testAndSetRelaxed(true, false)) {
            d->deref();
         }
         d->derefWaiters(1);
         //we got the lock. (do not deref)
         Q_ASSERT(d == d_ptr.load());
         return true;
      } else {
         Q_ASSERT(timeout >= 0);
         //timeout
         d->derefWaiters(1);
         //There may be a race in which the mutex is unlocked right after we timed out,
         // and before we deref the waiters, so maybe the mutex is actually unlocked.
         if (!d->possiblyUnlocked.testAndSetRelaxed(false, true)) {
            d->deref();
         }
         return false;
      }
   }
   Q_ASSERT(d_ptr.load() != 0);
   return true;
}

/*!
    \internal
*/
void QBasicMutex::unlockInternal()
{
   QMutexData *copy = d_ptr.loadAcquire();
   Q_ASSERT(copy); //we must be locked
   Q_ASSERT(copy != dummyLocked()); // testAndSetRelease(dummyLocked(), 0) failed

   if (copy->recursive) {
      static_cast<QRecursiveMutexPrivate *>(copy)->unlock();
      return;
   }

   QMutexPrivate *d = reinterpret_cast<QMutexPrivate *>(copy);

   if (d->waiters.fetchAndAddRelease(-QMutexPrivate::BigNumber) == 0) {
      //there is no one waiting on this mutex anymore, set the mutex as unlocked (d = 0)
      if (d_ptr.testAndSetRelease(d, 0)) {
         if (d->possiblyUnlocked.load() && d->possiblyUnlocked.testAndSetRelaxed(true, false)) {
            d->deref();
         }
      }
      d->derefWaiters(0);
   } else {
      d->derefWaiters(0);
      //there are thread waiting, transfer the lock.
      d->wakeUp();
   }
   d->deref();
}

//The freelist managment
namespace {
struct FreeListConstants : QFreeListDefaultConstants {
   enum { BlockCount = 4, MaxIndex = 0xffff };
   static const int Sizes[BlockCount];
};
const int FreeListConstants::Sizes[FreeListConstants::BlockCount] = {
   16,
   128,
   1024,
   FreeListConstants::MaxIndex - (16 - 128 - 1024)
};

typedef QFreeList<QMutexPrivate, FreeListConstants> FreeList;
Q_GLOBAL_STATIC(FreeList, freelist);
}

QMutexPrivate *QMutexPrivate::allocate()
{
   int i = freelist()->next();
   QMutexPrivate *d = &(*freelist())[i];
   d->id = i;
   Q_ASSERT(d->refCount.load() == 0);
   Q_ASSERT(!d->recursive);
   Q_ASSERT(!d->possiblyUnlocked.load());
   Q_ASSERT(d->waiters.load() == 0);
   d->refCount.store(1);
   return d;
}

void QMutexPrivate::release()
{
   Q_ASSERT(!recursive);
   Q_ASSERT(refCount.load() == 0);
   Q_ASSERT(!possiblyUnlocked.load());
   Q_ASSERT(waiters.load() == 0);
   freelist()->release(id);
}

// atomically substract "value" to the waiters, and remove the QMutexPrivate::BigNumber flag
void QMutexPrivate::derefWaiters(int value)
{
   int old_waiters;
   int new_waiters;
   do {
      old_waiters = waiters.load();
      new_waiters = old_waiters;
      if (new_waiters < 0) {
         new_waiters += QMutexPrivate::BigNumber;
      }
      new_waiters -= value;
   } while (!waiters.testAndSetRelaxed(old_waiters, new_waiters));
}
#endif

/*!
   \internal
 */
bool QRecursiveMutexPrivate::lock(int timeout)
{
   Qt::HANDLE self = QThread::currentThreadId();
   if (owner == self) {
      ++count;
      Q_ASSERT_X(count != 0, "QMutex::lock", "Overflow in recursion counter");
      return true;
   }
   bool success = true;
   if (timeout == -1) {
      mutex.lock();
   } else {
      success = mutex.tryLock(timeout);
   }

   if (success) {
      owner = self;
   }
   return success;
}

/*!
   \internal
 */
void QRecursiveMutexPrivate::unlock()
{
   if (count > 0) {
      count--;
   } else {
      owner = 0;
      mutex.unlock();
   }
}

QT_END_NAMESPACE

