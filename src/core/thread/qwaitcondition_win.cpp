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

#include <qwaitcondition.h>
#include <qnamespace.h>
#include <qmutex.h>
#include <qreadwritelock.h>
#include <qlist.h>
#include <qalgorithms.h>
#include <qt_windows.h>

#define Q_MUTEX_T void*
#include <qmutex_p.h>
#include <qreadwritelock_p.h>

QT_BEGIN_NAMESPACE

//***********************************************************************
// QWaitConditionPrivate
// **********************************************************************

class QWaitConditionEvent
{
 public:
   inline QWaitConditionEvent() : priority(0), wokenUp(false) {
      event = CreateEvent(NULL, TRUE, FALSE, NULL);
   }
   inline ~QWaitConditionEvent() {
      CloseHandle(event);
   }
   int priority;
   bool wokenUp;
   HANDLE event;
};

typedef QList<QWaitConditionEvent *> EventQueue;

class QWaitConditionPrivate
{
 public:
   QMutex mtx;
   EventQueue queue;
   EventQueue freeQueue;

   QWaitConditionEvent *pre();
   bool wait(QWaitConditionEvent *wce, unsigned long time);
   void post(QWaitConditionEvent *wce, bool ret);
};

QWaitConditionEvent *QWaitConditionPrivate::pre()
{
   mtx.lock();
   QWaitConditionEvent *wce =
      freeQueue.isEmpty() ? new QWaitConditionEvent : freeQueue.takeFirst();
   wce->priority = GetThreadPriority(GetCurrentThread());
   wce->wokenUp = false;

   // insert 'wce' into the queue (sorted by priority)
   int index = 0;
   for (; index < queue.size(); ++index) {
      QWaitConditionEvent *current = queue.at(index);
      if (current->priority < wce->priority) {
         break;
      }
   }
   queue.insert(index, wce);
   mtx.unlock();

   return wce;
}

bool QWaitConditionPrivate::wait(QWaitConditionEvent *wce, unsigned long time)
{
   // wait for the event
   bool ret = false;
   switch (WaitForSingleObject(wce->event, time)) {
      default:
         break;

      case WAIT_OBJECT_0:
         ret = true;
         break;
   }
   return ret;
}

void QWaitConditionPrivate::post(QWaitConditionEvent *wce, bool ret)
{
   mtx.lock();

   // remove 'wce' from the queue
   queue.removeAll(wce);
   ResetEvent(wce->event);
   freeQueue.append(wce);

   // wakeups delivered after the timeout should be forwarded to the next waiter
   if (!ret && wce->wokenUp && !queue.isEmpty()) {
      QWaitConditionEvent *other = queue.first();
      SetEvent(other->event);
      other->wokenUp = true;
   }

   mtx.unlock();
}

//***********************************************************************
// QWaitCondition implementation
//***********************************************************************

QWaitCondition::QWaitCondition()
{
   d = new QWaitConditionPrivate;
}

QWaitCondition::~QWaitCondition()
{
   if (!d->queue.isEmpty()) {
      qWarning("QWaitCondition: Destroyed while threads are still waiting");
      qDeleteAll(d->queue);
   }

   qDeleteAll(d->freeQueue);
   delete d;
}

bool QWaitCondition::wait(QMutex *mutex, unsigned long time)
{
   if (!mutex) {
      return false;
   }
   if (mutex->isRecursive()) {
      qWarning("QWaitCondition::wait: Cannot wait on recursive mutexes");
      return false;
   }

   QWaitConditionEvent *wce = d->pre();
   mutex->unlock();

   bool returnValue = d->wait(wce, time);

   mutex->lock();
   d->post(wce, returnValue);

   return returnValue;
}

bool QWaitCondition::wait(QReadWriteLock *readWriteLock, unsigned long time)
{
   if (!readWriteLock || readWriteLock->d->accessCount == 0) {
      return false;
   }
   if (readWriteLock->d->accessCount < -1) {
      qWarning("QWaitCondition: cannot wait on QReadWriteLocks with recursive lockForWrite()");
      return false;
   }

   QWaitConditionEvent *wce = d->pre();
   int previousAccessCount = readWriteLock->d->accessCount;
   readWriteLock->unlock();

   bool returnValue = d->wait(wce, time);

   if (previousAccessCount < 0) {
      readWriteLock->lockForWrite();
   } else {
      readWriteLock->lockForRead();
   }
   d->post(wce, returnValue);

   return returnValue;
}

void QWaitCondition::wakeOne()
{
   // wake up the first waiting thread in the queue
   QMutexLocker locker(&d->mtx);
   for (int i = 0; i < d->queue.size(); ++i) {
      QWaitConditionEvent *current = d->queue.at(i);
      if (current->wokenUp) {
         continue;
      }
      SetEvent(current->event);
      current->wokenUp = true;
      break;
   }
}

void QWaitCondition::wakeAll()
{
   // wake up the all threads in the queue
   QMutexLocker locker(&d->mtx);
   for (int i = 0; i < d->queue.size(); ++i) {
      QWaitConditionEvent *current = d->queue.at(i);
      SetEvent(current->event);
      current->wokenUp = true;
   }
}

QT_END_NAMESPACE
