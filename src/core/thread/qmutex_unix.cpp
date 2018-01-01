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
#include <qstring.h>
#include <qatomic.h>
#include <qmutex_p.h>
#include <errno.h>

QT_BEGIN_NAMESPACE

static void report_error(int code, const char *where, const char *what)
{
   if (code != 0) {
      qWarning("%s: %s failure: %s", where, what, qPrintable(qt_error_string(code)));
   }
}

QMutexPrivate::QMutexPrivate()
   : wakeup(false)
{
   report_error(pthread_mutex_init(&mutex, NULL), "QMutex", "mutex init");
   report_error(pthread_cond_init(&cond, NULL), "QMutex", "cv init");
}

QMutexPrivate::~QMutexPrivate()
{
   report_error(pthread_cond_destroy(&cond), "QMutex", "cv destroy");
   report_error(pthread_mutex_destroy(&mutex), "QMutex", "mutex destroy");
}

bool QMutexPrivate::wait(int timeout)
{
   report_error(pthread_mutex_lock(&mutex), "QMutex::lock", "mutex lock");
   int errorCode = 0;
   while (!wakeup) {
      if (timeout < 0) {
         errorCode = pthread_cond_wait(&cond, &mutex);
      } else {
         struct timeval tv;
         gettimeofday(&tv, 0);
         timespec ti;
         ti.tv_nsec = (tv.tv_usec + (timeout % 1000) * 1000) * 1000;
         ti.tv_sec = tv.tv_sec + (timeout / 1000) + (ti.tv_nsec / 1000000000);
         ti.tv_nsec %= 1000000000;
         errorCode = pthread_cond_timedwait(&cond, &mutex, &ti);
      }
      if (errorCode) {
         if (errorCode == ETIMEDOUT) {
            if (wakeup) {
               errorCode = 0;
            }
            break;
         }
         report_error(errorCode, "QMutex::lock()", "cv wait");
      }
   }
   bool ret = wakeup;
   wakeup = false;
   report_error(pthread_mutex_unlock(&mutex), "QMutex::lock", "mutex unlock");
   return ret;
}

void QMutexPrivate::wakeUp()
{
   report_error(pthread_mutex_lock(&mutex), "QMutex::unlock", "mutex lock");
   wakeup = true;
   report_error(pthread_cond_signal(&cond), "QMutex::unlock", "cv signal");
   report_error(pthread_mutex_unlock(&mutex), "QMutex::unlock", "mutex unlock");
}

QT_END_NAMESPACE

