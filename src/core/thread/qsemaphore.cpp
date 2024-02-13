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

#include <qsemaphore.h>

#include <qdatetime.h>
#include <qelapsedtimer.h>
#include <qmutex.h>
#include <qwaitcondition.h>

class QSemaphorePrivate
{
 public:
   inline QSemaphorePrivate(int n) : avail(n) { }

   QMutex mutex;
   QWaitCondition cond;

   int avail;
};

QSemaphore::QSemaphore(int n)
{
   Q_ASSERT_X(n >= 0, "QSemaphore", "parameter 'n' must be non-negative");
   d = new QSemaphorePrivate(n);
}

QSemaphore::~QSemaphore()
{
   delete d;
}

void QSemaphore::acquire(int n)
{
   Q_ASSERT_X(n >= 0, "QSemaphore::acquire", "parameter 'n' must be non-negative");
   QMutexLocker locker(&d->mutex);

   while (n > d->avail) {
      d->cond.wait(locker.mutex());
   }

   d->avail -= n;
}

void QSemaphore::release(int n)
{
   Q_ASSERT_X(n >= 0, "QSemaphore::release", "parameter 'n' must be non-negative");
   QMutexLocker locker(&d->mutex);
   d->avail += n;
   d->cond.wakeAll();
}

int QSemaphore::available() const
{
   QMutexLocker locker(&d->mutex);
   return d->avail;
}

bool QSemaphore::tryAcquire(int n)
{
   Q_ASSERT_X(n >= 0, "QSemaphore::tryAcquire", "parameter 'n' must be non-negative");
   QMutexLocker locker(&d->mutex);

   if (n > d->avail) {
      return false;
   }

   d->avail -= n;
   return true;
}

bool QSemaphore::tryAcquire(int n, int timeout)
{
   Q_ASSERT_X(n >= 0, "QSemaphore::tryAcquire", "parameter 'n' must be non-negative");
   QMutexLocker locker(&d->mutex);

   if (timeout < 0) {
      while (n > d->avail) {
         d->cond.wait(locker.mutex());
      }
   } else {
      QElapsedTimer timer;
      timer.start();

      while (n > d->avail) {
         const qint64 elapsed = timer.elapsed();

         if (timeout - elapsed <= 0
               || !d->cond.wait(locker.mutex(), timeout - elapsed)) {
            return false;
         }
      }
   }

   d->avail -= n;

   return true;
}
