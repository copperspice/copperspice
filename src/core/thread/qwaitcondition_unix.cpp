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

#include <qwaitcondition.h>

#include <qatomic.h>
#include <qmutex.h>
#include <qplatformdefs.h>
#include <qreadwritelock.h>
#include <qstring.h>

#include <qreadwritelock_p.h>

#include <errno.h>

static void report_error(int code, const char *where, const char *what)
{
   if (code != 0) {
      qWarning("%s() %s failure, %s", where, what, csPrintable(qt_error_string(code)));
   }
}

class QWaitConditionPrivate
{
 public:
   pthread_mutex_t mutex;
   pthread_cond_t cond;
   int waiters;
   int wakeups;

   bool wait(unsigned long time) {
      int code;

      while (true) {
         if (time != ULONG_MAX) {
            struct timeval tv;
            gettimeofday(&tv, nullptr);

            timespec ti;
            ti.tv_nsec = (tv.tv_usec + (time % 1000) * 1000) * 1000;
            ti.tv_sec = tv.tv_sec + (time / 1000) + (ti.tv_nsec / 1000000000);
            ti.tv_nsec %= 1000000000;

            code = pthread_cond_timedwait(&cond, &mutex, &ti);
         } else {
            code = pthread_cond_wait(&cond, &mutex);
         }

         if (code == 0 && wakeups == 0) {
            // many vendors warn of spurios wakeups from
            // pthread_cond_wait(), especially after signal delivery,
            // even though POSIX doesn't allow for it... sigh
            continue;
         }

         break;
      }

      Q_ASSERT_X(waiters > 0, "QWaitCondition::wait", "internal error (waiters)");
      --waiters;

      if (code == 0) {
         Q_ASSERT_X(wakeups > 0, "QWaitCondition::wait", "internal error (wakeups)");
         --wakeups;
      }

      report_error(pthread_mutex_unlock(&mutex), "QWaitCondition::wait()", "mutex unlock");

      if (code && code != ETIMEDOUT) {
         report_error(code, "QWaitCondition::wait()", "cv wait");
      }

      return (code == 0);
   }
};

QWaitCondition::QWaitCondition()
{
   d = new QWaitConditionPrivate;
   report_error(pthread_mutex_init(&d->mutex, nullptr), "QWaitCondition", "mutex init");
   report_error(pthread_cond_init(&d->cond, nullptr), "QWaitCondition", "cv init");
   d->waiters = d->wakeups = 0;
}

QWaitCondition::~QWaitCondition()
{
   report_error(pthread_cond_destroy(&d->cond), "QWaitCondition", "cv destroy");
   report_error(pthread_mutex_destroy(&d->mutex), "QWaitCondition", "mutex destroy");
   delete d;
}

void QWaitCondition::wakeOne()
{
   report_error(pthread_mutex_lock(&d->mutex), "QWaitCondition::wakeOne()", "mutex lock");
   d->wakeups = qMin(d->wakeups + 1, d->waiters);
   report_error(pthread_cond_signal(&d->cond), "QWaitCondition::wakeOne()", "cv signal");
   report_error(pthread_mutex_unlock(&d->mutex), "QWaitCondition::wakeOne()", "mutex unlock");
}

void QWaitCondition::wakeAll()
{
   report_error(pthread_mutex_lock(&d->mutex), "QWaitCondition::wakeAll()", "mutex lock");
   d->wakeups = d->waiters;
   report_error(pthread_cond_broadcast(&d->cond), "QWaitCondition::wakeAll()", "cv broadcast");
   report_error(pthread_mutex_unlock(&d->mutex), "QWaitCondition::wakeAll()", "mutex unlock");
}

bool QWaitCondition::wait(QMutex *mutex, unsigned long time)
{
   if (! mutex) {
      return false;
   }

   report_error(pthread_mutex_lock(&d->mutex), "QWaitCondition::wait()", "mutex lock");
   ++d->waiters;
   mutex->unlock();

   bool returnValue = d->wait(time);

   mutex->lock();

   return returnValue;
}

bool QWaitCondition::wait(QReadWriteLock *readWriteLock, unsigned long time)
{
   if (!readWriteLock || readWriteLock->d->accessCount == 0) {
      return false;
   }

   if (readWriteLock->d->accessCount < -1) {
      qWarning("QWaitCondition::wait() Unable to wait on QReadWriteLocks with recursive lockForWrite()");
      return false;
   }

   report_error(pthread_mutex_lock(&d->mutex), "QWaitCondition::wait()", "mutex lock");
   ++d->waiters;

   int previousAccessCount = readWriteLock->d->accessCount;
   readWriteLock->unlock();

   bool returnValue = d->wait(time);

   if (previousAccessCount < 0) {
      readWriteLock->lockForWrite();
   } else {
      readWriteLock->lockForRead();
   }

   return returnValue;
}
