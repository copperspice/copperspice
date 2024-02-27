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

#ifndef QTHREAD_P_H
#define QTHREAD_P_H

#include <qmap.h>
#include <qmutex.h>
#include <qplatformdefs.h>
#include <qstack.h>
#include <qthread.h>
#include <qwaitcondition.h>

#include <algorithm>

class QAbstractEventDispatcher;
class QEventLoop;

class QPostEvent
{
 public:
   QObject *receiver;
   QEvent *event;

   int priority;
   inline QPostEvent()
      : receiver(nullptr), event(nullptr), priority(0) {
   }

   inline QPostEvent(QObject *r, QEvent *e, int p)
      : receiver(r), event(e), priority(p) {
   }
};

inline bool operator<(const QPostEvent &first, const QPostEvent &second)
{
   return first.priority > second.priority;
}

// This class holds the list of posted events.
//  The list has to be kept sorted by priority
class QPostEventList : public QVector<QPostEvent>
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
      : QVector<QPostEvent>(), recursion(0), startOffset(0), insertionOffset(0)
   { }

   void addEvent(const QPostEvent &ev) {
      int priority = ev.priority;

      if (isEmpty() || last().priority >= priority || insertionOffset >= size()) {
         // optimization: we can simply append if the last event in
         // the queue has higher or equal priority
         append(ev);

      } else {
         // insert event in descending priority order, using upper
         // bound for a given priority (to ensure proper ordering
         // of events with the same priority)
         QPostEventList::iterator at = std::upper_bound(begin() + insertionOffset, end(), ev);
         insert(at, ev);
      }
   }

 private:
   // hides because they do not keep that list sorted. addEvent must be used
   using QVector<QPostEvent>::append;
   using QVector<QPostEvent>::insert;
};

class Q_CORE_EXPORT QDaemonThread : public QThread
{
 public:
   QDaemonThread(QObject *parent = nullptr);
   ~QDaemonThread();
};

class QThreadPrivate
{
   Q_DECLARE_PUBLIC(QThread)

 public:
   QThreadPrivate(QThreadData *d = nullptr);
   virtual ~QThreadPrivate();

   void setPriority(QThread::Priority prio);
   mutable QMutex mutex;
   QAtomicInt quitLockRef;

   bool running;
   bool finished;
   bool isInFinish;                   //when in QThreadPrivate::finish
   bool interruptionRequested;
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

#ifdef Q_OS_WIN
   Qt::HANDLE handle;
   unsigned int id;
   int waiters;

   static unsigned int __stdcall start(void *);
   static void finish(void *, bool lockAnyway = true);

   bool terminationEnabled, terminatePending;
#endif
   QThreadData *data;

   static void createEventDispatcher(QThreadData *data);

   void ref() {
      quitLockRef.ref();
   }

   void deref() {
      if (!quitLockRef.deref() && running) {
         QCoreApplication::instance()->postEvent(q_ptr, new QEvent(QEvent::Quit));
      }
   }

 protected:
   QThread *q_ptr;
};

class QThreadData
{
 public:
   QThreadData(int initialRefCount = 1);
   ~QThreadData();

   static QThreadData *current(bool createIfNecessary = true);
   static void clearCurrentThreadData();

   static QThreadData *get2(QThread *thread) {
      Q_ASSERT_X(thread != nullptr, "QThread", "internal error");
      return thread->d_func()->data;
   }

   void ref();
   void deref();

   bool hasEventDispatcher() const {
      return eventDispatcher.load() != nullptr;
   }

   bool canWaitLocked() {
      QMutexLocker locker(&postEventList.mutex);
      return canWait;
   }

   std::atomic<QThread *> thread;
   Qt::HANDLE threadId;

   std::atomic<QAbstractEventDispatcher *> eventDispatcher;
   QStack<QEventLoop *> eventLoops;
   QPostEventList postEventList;

   QVector<void *> tls;

   int loopLevel;
   bool quitNow;
   bool canWait;
   bool isAdopted;
   bool requiresCoreApplication;

   QThreadPrivate *get_QThreadPrivate() const;

 private:
   QAtomicInt m_ref;
};

class QScopedLoopLevelCounter
{
   QThreadData *m_threadData;

 public:
   QScopedLoopLevelCounter(QThreadData *threadData)
      : m_threadData(threadData) {
      ++m_threadData->loopLevel;
   }

   ~QScopedLoopLevelCounter() {
      --m_threadData->loopLevel;
   }
};

// thread wrapper for the main() thread
class QAdoptedThread : public QThread
{
   Q_DECLARE_PRIVATE(QThread)

 public:
   QAdoptedThread(QThreadData *data = nullptr);
   ~QAdoptedThread();
   void init();

 private:
   void run() override;
};

#endif
