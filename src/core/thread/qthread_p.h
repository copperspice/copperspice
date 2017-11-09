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

#ifndef QTHREAD_P_H
#define QTHREAD_P_H

#include <algorithm>

#include <qplatformdefs.h>
#include <QtCore/qthread.h>
#include <QtCore/qmutex.h>
#include <QtCore/qstack.h>
#include <QtCore/qwaitcondition.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

class QAbstractEventDispatcher;
class QEventLoop;

class QPostEvent
{
 public:
   QObject *receiver;
   QEvent *event;
   int priority;
   inline QPostEvent()
      : receiver(0), event(0), priority(0) {
   }
   inline QPostEvent(QObject *r, QEvent *e, int p)
      : receiver(r), event(e), priority(p) {
   }
};

inline bool operator<(int priority, const QPostEvent &pe)
{
   return pe.priority < priority;
}

inline bool operator<(const QPostEvent &pe, int priority)
{
   return priority < pe.priority;
}

// This class holds the list of posted events.
//  The list has to be kept sorted by priority
class QPostEventList : public QList<QPostEvent>
{
 public:
   // recursion == recursion count for sendPostedEvents()
   int recursion;

   // sendOffset == the current event to start sending
   int startOffset;
   // insertionOffset == set by sendPostedEvents to tell postEvent() where to start insertions
   int insertionOffset;

   QMutex mutex;

   inline QPostEventList()
      : QList<QPostEvent>(), recursion(0), startOffset(0), insertionOffset(0) {
   }

   void addEvent(const QPostEvent &ev) {
      int priority = ev.priority;
      if (isEmpty() || last().priority >= priority) {
         // optimization: we can simply append if the last event in
         // the queue has higher or equal priority
         append(ev);
      } else {
         // insert event in descending priority order, using upper
         // bound for a given priority (to ensure proper ordering
         // of events with the same priority)
         QPostEventList::iterator at = std::upper_bound(begin() + insertionOffset, end(), priority);
         insert(at, ev);
      }
   }

 private:
   // hides because they do not keep that list sorted. addEvent must be used
   using QList<QPostEvent>::append;
   using QList<QPostEvent>::insert;
};

class QThreadPrivate
{
   Q_DECLARE_PUBLIC(QThread)

 public:
   QThreadPrivate(QThreadData *d = 0);
   virtual ~QThreadPrivate();

   mutable QMutex mutex;

   bool running;
   bool finished;
   bool terminated;
   bool isInFinish; //when in QThreadPrivate::finish

   bool exited;
   int returnCode;

   uint stackSize;
   QThread::Priority priority;

   static QThread *threadForId(int id);

   static QThreadPrivate *cs_getPrivate(QThread *object);

#ifdef Q_OS_UNIX
   pthread_t thread_id;
   QWaitCondition thread_done;

   static void *start(void *arg);
   static void finish(void *);
#endif

#if defined(Q_OS_WIN32)
   HANDLE handle;
   unsigned int id;
   int waiters;

   static unsigned int __stdcall start(void *);
   static void finish(void *, bool lockAnyway = true);

   bool terminationEnabled, terminatePending;
# endif

   QThreadData *data;
   static void createEventDispatcher(QThreadData *data);

 protected:
   QThread *q_ptr;

};


class QThreadData
{
   QAtomicInt _ref;

 public:
   QThreadData(int initialRefCount = 1);
   ~QThreadData();

   static QThreadData *current();
   static void clearCurrentThreadData();

   static QThreadData *get2(QThread *thread) {
      Q_ASSERT_X(thread != 0, "QThread", "internal error");
      return thread->d_func()->data;
   }

   void ref();
   void deref();

   bool canWaitLocked() {
      QMutexLocker locker(&postEventList.mutex);
      return canWait;
   }

   QThread *thread;
   Qt::HANDLE threadId;
   bool quitNow;
   int loopLevel;
   QAbstractEventDispatcher *eventDispatcher;
   QStack<QEventLoop *> eventLoops;
   QPostEventList postEventList;
   bool canWait;
   QVector<void *> tls;
   bool isAdopted;

   QThreadPrivate *get_QThreadPrivate() const;
};

class QScopedLoopLevelCounter
{
   QThreadData *threadData;

 public:
   inline QScopedLoopLevelCounter(QThreadData *threadData)
      : threadData(threadData) {
      ++threadData->loopLevel;
   }
   inline ~QScopedLoopLevelCounter() {
      --threadData->loopLevel;
   }
};

// thread wrapper for the main() thread
class QAdoptedThread : public QThread
{
   Q_DECLARE_PRIVATE(QThread)

 public:
   QAdoptedThread(QThreadData *data = 0);
   ~QAdoptedThread();
   void init();

 private:
   void run() override;
};

QT_END_NAMESPACE

#endif // QTHREAD_P_H
