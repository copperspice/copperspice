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

#include <qreadwritelock.h>

#include <qmutex.h>
#include <qplatformdefs.h>
#include <qthread.h>
#include <qwaitcondition.h>

#include <qreadwritelock_p.h>

QReadWriteLock::QReadWriteLock(RecursionMode recursionMode)
   : d(new QReadWriteLockPrivate(recursionMode))
{
}

QReadWriteLock::~QReadWriteLock()
{
   delete d;
}

void QReadWriteLock::lockForRead()
{
   QMutexLocker lock(&d->mutex);

   Qt::HANDLE self = nullptr;

   if (d->recursive) {
      self = QThread::currentThreadId();

      QHash<Qt::HANDLE, int>::iterator it = d->currentReaders.find(self);

      if (it != d->currentReaders.end()) {
         ++it.value();
         ++d->accessCount;
         Q_ASSERT_X(d->accessCount > 0, "QReadWriteLock::lockForRead()",
               "Overflow in lock counter");
         return;
      }
   }

   while (d->accessCount < 0 || d->waitingWriters) {
      ++d->waitingReaders;
      d->readerWait.wait(&d->mutex);
      --d->waitingReaders;
   }

   if (d->recursive) {
      d->currentReaders.insert(self, 1);
   }

   ++d->accessCount;
   Q_ASSERT_X(d->accessCount > 0, "QReadWriteLock::lockForRead()", "Overflow in lock counter");
}

bool QReadWriteLock::tryLockForRead()
{
   QMutexLocker lock(&d->mutex);

   Qt::HANDLE self = nullptr;

   if (d->recursive) {
      self = QThread::currentThreadId();

      QHash<Qt::HANDLE, int>::iterator it = d->currentReaders.find(self);

      if (it != d->currentReaders.end()) {
         ++it.value();
         ++d->accessCount;
         Q_ASSERT_X(d->accessCount > 0, "QReadWriteLock::tryLockForRead()",
               "Overflow in lock counter");
         return true;
      }
   }

   if (d->accessCount < 0) {
      return false;
   }

   if (d->recursive) {
      d->currentReaders.insert(self, 1);
   }

   ++d->accessCount;
   Q_ASSERT_X(d->accessCount > 0, "QReadWriteLock::tryLockForRead()", "Overflow in lock counter");

   return true;
}

bool QReadWriteLock::tryLockForRead(int timeout)
{
   QMutexLocker lock(&d->mutex);

   Qt::HANDLE self = nullptr;

   if (d->recursive) {
      self = QThread::currentThreadId();

      QHash<Qt::HANDLE, int>::iterator it = d->currentReaders.find(self);

      if (it != d->currentReaders.end()) {
         ++it.value();
         ++d->accessCount;
         Q_ASSERT_X(d->accessCount > 0, "QReadWriteLock::tryLockForRead()",
               "Overflow in lock counter");
         return true;
      }
   }

   while (d->accessCount < 0 || d->waitingWriters) {
      ++d->waitingReaders;
      bool success = d->readerWait.wait(&d->mutex, timeout < 0 ? ULONG_MAX : ulong(timeout));
      --d->waitingReaders;

      if (!success) {
         return false;
      }
   }

   if (d->recursive) {
      d->currentReaders.insert(self, 1);
   }

   ++d->accessCount;
   Q_ASSERT_X(d->accessCount > 0, "QReadWriteLock::tryLockForRead()", "Overflow in lock counter");

   return true;
}

void QReadWriteLock::lockForWrite()
{
   QMutexLocker lock(&d->mutex);

   Qt::HANDLE self = nullptr;

   if (d->recursive) {
      self = QThread::currentThreadId();

      if (d->currentWriter == self) {
         --d->accessCount;
         Q_ASSERT_X(d->accessCount < 0, "QReadWriteLock::lockForWrite()",
               "Overflow in lock counter");
         return;
      }
   }

   while (d->accessCount != 0) {
      ++d->waitingWriters;
      d->writerWait.wait(&d->mutex);
      --d->waitingWriters;
   }

   if (d->recursive) {
      d->currentWriter = self;
   }

   --d->accessCount;
   Q_ASSERT_X(d->accessCount < 0, "QReadWriteLock::lockForWrite()", "Overflow in lock counter");
}

/*!
    Attempts to lock for writing. If the lock was obtained, this
    function returns true; otherwise, it returns false immediately.

    The lock attempt will fail if another thread has locked for
    reading or writing.

    If the lock was obtained, the lock must be unlocked with unlock()
    before another thread can successfully lock it.

    \sa unlock() lockForWrite()
*/
bool QReadWriteLock::tryLockForWrite()
{
   QMutexLocker lock(&d->mutex);

   Qt::HANDLE self = nullptr;

   if (d->recursive) {
      self = QThread::currentThreadId();

      if (d->currentWriter == self) {
         --d->accessCount;
         Q_ASSERT_X(d->accessCount < 0, "QReadWriteLock::lockForWrite()",
               "Overflow in lock counter");
         return true;
      }
   }

   if (d->accessCount != 0) {
      return false;
   }

   if (d->recursive) {
      d->currentWriter = self;
   }

   --d->accessCount;
   Q_ASSERT_X(d->accessCount < 0, "QReadWriteLock::tryLockForWrite()",
         "Overflow in lock counter");

   return true;
}

/*! \overload

    Attempts to lock for writing. This function returns true if the
    lock was obtained; otherwise it returns false. If another thread
    has locked for reading or writing, this function will wait for at
    most \a timeout milliseconds for the lock to become available.

    Note: Passing a negative number as the \a timeout is equivalent to
    calling lockForWrite(), i.e. this function will wait forever until
    lock can be locked for writing when \a timeout is negative.

    If the lock was obtained, the lock must be unlocked with unlock()
    before another thread can successfully lock it.

    \sa unlock() lockForWrite()
*/
bool QReadWriteLock::tryLockForWrite(int timeout)
{
   QMutexLocker lock(&d->mutex);

   Qt::HANDLE self = nullptr;

   if (d->recursive) {
      self = QThread::currentThreadId();

      if (d->currentWriter == self) {
         --d->accessCount;
         Q_ASSERT_X(d->accessCount < 0, "QReadWriteLock::lockForWrite()",
               "Overflow in lock counter");
         return true;
      }
   }

   while (d->accessCount != 0) {
      ++d->waitingWriters;
      bool success = d->writerWait.wait(&d->mutex, timeout < 0 ? ULONG_MAX : ulong(timeout));
      --d->waitingWriters;

      if (!success) {
         return false;
      }
   }

   if (d->recursive) {
      d->currentWriter = self;
   }

   --d->accessCount;
   Q_ASSERT_X(d->accessCount < 0, "QReadWriteLock::tryLockForWrite()",
         "Overflow in lock counter");

   return true;
}

void QReadWriteLock::unlock()
{
   QMutexLocker lock(&d->mutex);

   Q_ASSERT_X(d->accessCount != 0, "QReadWriteLock::unlock()", "Cannot unlock an unlocked lock");

   bool unlocked = false;

   if (d->accessCount > 0) {
      // releasing a read lock
      if (d->recursive) {
         Qt::HANDLE self = QThread::currentThreadId();
         QHash<Qt::HANDLE, int>::iterator it = d->currentReaders.find(self);

         if (it != d->currentReaders.end()) {
            if (--it.value() <= 0) {
               d->currentReaders.erase(it);
            }
         }
      }

      unlocked = --d->accessCount == 0;
   } else if (d->accessCount < 0 && ++d->accessCount == 0) {
      // released a write lock
      unlocked = true;
      d->currentWriter = nullptr;
   }

   if (unlocked) {
      if (d->waitingWriters) {
         d->writerWait.wakeOne();
      } else if (d->waitingReaders) {
         d->readerWait.wakeAll();
      }
   }
}
