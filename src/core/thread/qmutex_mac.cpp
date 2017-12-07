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

#include <qplatformdefs.h>
#include <qmutex.h>
#include <qlog.h>

#if !defined(QT_NO_THREAD)

#include <qmutex_p.h>

#include <mach/mach.h>
#include <mach/task.h>

#include <errno.h>

QT_BEGIN_NAMESPACE

QMutexPrivate::QMutexPrivate()
{
   kern_return_t r = semaphore_create(mach_task_self(), &mach_semaphore, SYNC_POLICY_FIFO, 0);
   if (r != KERN_SUCCESS) {
      qWarning("QMutex: failed to create semaphore, error %d", r);
   }
}

QMutexPrivate::~QMutexPrivate()
{
   kern_return_t r = semaphore_destroy(mach_task_self(), mach_semaphore);
   if (r != KERN_SUCCESS) {
      qWarning("QMutex: failed to destroy semaphore, error %d", r);
   }
}

bool QMutexPrivate::wait(int timeout)
{
   kern_return_t r;
   if (timeout < 0) {
      do {
         r = semaphore_wait(mach_semaphore);
      } while (r == KERN_ABORTED);
      Q_ASSERT(r == KERN_SUCCESS);
   } else {
      mach_timespec_t ts;
      ts.tv_nsec = ((timeout % 1000) * 1000) * 1000;
      ts.tv_sec = (timeout / 1000);
      r = semaphore_timedwait(mach_semaphore, ts);
   }
   return (r == KERN_SUCCESS);
}

void QMutexPrivate::wakeUp()
{
   semaphore_signal(mach_semaphore);
}


QT_END_NAMESPACE

#endif //QT_NO_THREAD
