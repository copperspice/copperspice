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

#include <qthreadpool.h>

#include <qelapsedtimer.h>
#include <qpair.h>

#include <qthreadpool_p.h>

static QThreadPool *theInstance()
{
   static QThreadPool retval;
   return &retval;
}

class QThreadPoolThread : public QThread
{
 public:
   QThreadPoolThread(QThreadPoolPrivate *manager);
   void run() override;
   void registerThreadInactive();

   QWaitCondition runnableReady;
   QThreadPoolPrivate *manager;
   QRunnable *runnable;
};

QThreadPoolThread::QThreadPoolThread(QThreadPoolPrivate *manager)
   : manager(manager), runnable(nullptr)
{ }

// internal
void QThreadPoolThread::run()
{
   QMutexLocker locker(&manager->mutex);

   for (;;) {
      QRunnable *r = runnable;
      runnable = nullptr;

      do {
         if (r) {
            const bool autoDelete = r->autoDelete();

            // run the task
            locker.unlock();

            try {
               r->run();

            } catch (...) {
               qWarning("QThreadPoolThread::run() An exception was thrown from a worker thread");
               registerThreadInactive();
               throw;
            }

            locker.relock();

            if (autoDelete && !--r->ref) {
               delete r;
            }
         }

         // if too many threads are active, expire this thread
         if (manager->tooManyThreadsActive()) {
            break;
         }

         r = ! manager->queue.isEmpty() ? manager->queue.takeFirst().first : nullptr;

      } while (r != nullptr);

      if (manager->isExiting) {
         registerThreadInactive();
         break;
      }

      // if too many threads are active, expire this thread
      bool expired = manager->tooManyThreadsActive();

      if (! expired) {
         manager->waitingThreads.enqueue(this);
         registerThreadInactive();

         // wait for work, exiting after the expiry timeout is reached
         runnableReady.wait(locker.mutex(), manager->expiryTimeout);
         ++manager->activeThreads;

         if (manager->waitingThreads.removeOne(this)) {
            expired = true;
         }
      }

      if (expired) {
         manager->expiredThreads.enqueue(this);
         registerThreadInactive();
         break;
      }
   }
}

void QThreadPoolThread::registerThreadInactive()
{
   if (--manager->activeThreads == 0) {
      manager->noActiveThreads.wakeAll();
   }
}

QThreadPoolPrivate:: QThreadPoolPrivate()
   : isExiting(false), expiryTimeout(30000),
     maxThreadCount(qAbs(QThread::idealThreadCount())),
     reservedThreads(0), activeThreads(0)
{ }

bool QThreadPoolPrivate::tryStart(QRunnable *task)
{
   if (allThreads.isEmpty()) {
      // always create at least one thread
      startThread(task);
      return true;
   }

   // can not do anything if we're over the limit
   if (activeThreadCount() >= maxThreadCount) {
      return false;
   }

   if (waitingThreads.count() > 0) {
      // recycle an available thread
      enqueueTask(task);
      waitingThreads.takeFirst()->runnableReady.wakeOne();
      return true;
   }

   if (! expiredThreads.isEmpty()) {
      // restart an expired thread
      QThreadPoolThread *thread = expiredThreads.dequeue();
      Q_ASSERT(thread->runnable == nullptr);

      ++activeThreads;

      if (task->autoDelete()) {
         ++task->ref;
      }

      thread->runnable = task;
      thread->start();
      return true;
   }

   // start a new thread
   startThread(task);

   return true;
}

inline bool operator<(int priority, const QPair<QRunnable *, int> &p)
{
   return p.second < priority;
}

inline bool operator<(const QPair<QRunnable *, int> &p, int priority)
{
   return priority < p.second;
}

void QThreadPoolPrivate::enqueueTask(QRunnable *runnable, int priority)
{
   if (runnable->autoDelete()) {
      ++runnable->ref;
   }

   // put it on the queue
   auto begin = queue.constBegin();
   auto iter  = queue.constEnd();

   if (iter != begin && priority > (*(iter - 1)).second) {
      iter = std::upper_bound(begin, --iter, priority);
   }

   queue.insert(iter - begin, qMakePair(runnable, priority));
}

int QThreadPoolPrivate::activeThreadCount() const
{
   return (allThreads.count() - expiredThreads.count() - waitingThreads.count() + reservedThreads);
}

void QThreadPoolPrivate::tryToStartMoreThreads()
{
   // try to push tasks on the queue to any available threads
   while (!queue.isEmpty() && tryStart(queue.first().first)) {
      queue.removeFirst();
   }
}

bool QThreadPoolPrivate::tooManyThreadsActive() const
{
   const int activeThreadCount = this->activeThreadCount();
   return activeThreadCount > maxThreadCount && (activeThreadCount - reservedThreads) > 1;
}

void QThreadPoolPrivate::startThread(QRunnable *runnable)
{
   QScopedPointer <QThreadPoolThread> thread(new QThreadPoolThread(this));
   thread->setObjectName("Thread (pooled)");
   allThreads.insert(thread.data());

   ++activeThreads;

   if (runnable->autoDelete()) {
      ++runnable->ref;
   }

   thread->runnable = runnable;
   thread.take()->start();
}

void QThreadPoolPrivate::reset()
{
   QMutexLocker locker(&mutex);
   isExiting = true;

   while (!allThreads.empty()) {
      // make a copy of the set so that we can iterate without the lock
      QSet<QThreadPoolThread *> allThreadsCopy;
      allThreadsCopy.swap(allThreads);
      locker.unlock();

      for (QThreadPoolThread *thread : allThreadsCopy) {
         thread->runnableReady.wakeAll();
         thread->wait();
         delete thread;
      }

      locker.relock();
      // repeat until all newly arrived threads have also completed
   }

   waitingThreads.clear();
   expiredThreads.clear();

   isExiting = false;
}

bool QThreadPoolPrivate::waitForDone(int msecs)
{
   QMutexLocker locker(&mutex);

   if (msecs < 0) {
      while (!(queue.isEmpty() && activeThreads == 0)) {
         noActiveThreads.wait(locker.mutex());
      }

   } else {
      QElapsedTimer timer;
      timer.start();
      int t;

      while (!(queue.isEmpty() && activeThreads == 0) && ((t = msecs - timer.elapsed()) > 0)) {
         noActiveThreads.wait(locker.mutex(), t);
      }
   }

   return queue.isEmpty() && activeThreads == 0;
}

void QThreadPoolPrivate::clear()
{
   QMutexLocker locker(&mutex);

   for (QVector<QPair<QRunnable *, int>>::const_iterator it = queue.constBegin(); it != queue.constEnd(); ++it) {
      QRunnable *r = it->first;

      if (r->autoDelete() && ! --r->ref) {
         delete r;
      }
   }

   queue.clear();
}

bool QThreadPoolPrivate::stealRunnable(QRunnable *runnable)
{
   if (runnable == nullptr) {
      return false;
   }

   {
      QMutexLocker locker(&mutex);
      QVector<QPair<QRunnable *, int>>::iterator it = queue.begin();
      QVector<QPair<QRunnable *, int>>::iterator end = queue.end();

      while (it != end) {
         if (it->first == runnable) {
            queue.erase(it);
            return true;
         }

         ++it;
      }
   }

   return false;
}

void QThreadPoolPrivate::stealAndRunRunnable(QRunnable *runnable)
{
   if (!stealRunnable(runnable)) {
      return;
   }

   const bool autoDelete = runnable->autoDelete();
   bool del = autoDelete && !--runnable->ref;

   runnable->run();

   if (del) {
      delete runnable;
   }
}

QThreadPool::QThreadPool(QObject *parent)
   : QObject(parent), d_ptr(new QThreadPoolPrivate)
{
   d_ptr->q_ptr = this;
}

QThreadPool::~QThreadPool()
{
   waitForDone();
}

QThreadPool *QThreadPool::globalInstance()
{
   return theInstance();
}

void QThreadPool::start(QRunnable *runnable, int priority)
{
   if (! runnable) {
      return;
   }

   Q_D(QThreadPool);
   QMutexLocker locker(&d->mutex);

   if (! d->tryStart(runnable)) {
      d->enqueueTask(runnable, priority);

      if (! d->waitingThreads.isEmpty()) {
         d->waitingThreads.takeFirst()->runnableReady.wakeOne();
      }
   }
}

bool QThreadPool::tryStart(QRunnable *runnable)
{
   if (! runnable) {
      return false;
   }

   Q_D(QThreadPool);

   QMutexLocker locker(&d->mutex);

   if (d->allThreads.isEmpty() == false && d->activeThreadCount() >= d->maxThreadCount) {
      return false;
   }

   return d->tryStart(runnable);
}

int QThreadPool::expiryTimeout() const
{
   Q_D(const QThreadPool);
   return d->expiryTimeout;
}

void QThreadPool::setExpiryTimeout(int expiryTimeout)
{
   Q_D(QThreadPool);

   if (d->expiryTimeout == expiryTimeout) {
      return;
   }

   d->expiryTimeout = expiryTimeout;
}

int QThreadPool::maxThreadCount() const
{
   Q_D(const QThreadPool);
   return d->maxThreadCount;
}

void QThreadPool::setMaxThreadCount(int maxThreadCount)
{
   Q_D(QThreadPool);
   QMutexLocker locker(&d->mutex);

   if (maxThreadCount == d->maxThreadCount) {
      return;
   }

   d->maxThreadCount = maxThreadCount;
   d->tryToStartMoreThreads();
}

int QThreadPool::activeThreadCount() const
{
   Q_D(const QThreadPool);

   QMutexLocker locker(&d->mutex);
   return d->activeThreadCount();
}

void QThreadPool::reserveThread()
{
   Q_D(QThreadPool);

   QMutexLocker locker(&d->mutex);
   ++d->reservedThreads;
}

void QThreadPool::releaseThread()
{
   Q_D(QThreadPool);

   QMutexLocker locker(&d->mutex);
   --d->reservedThreads;
   d->tryToStartMoreThreads();
}

bool QThreadPool::waitForDone(int msecs)
{
   Q_D(QThreadPool);

   bool rc = d->waitForDone(msecs);

   if (rc) {
      d->reset();
   }

   return rc;
}

void QThreadPool::clear()
{
   Q_D(QThreadPool);
   d->clear();
}

void QThreadPool::cancel(QRunnable *runnable)
{
   Q_D(QThreadPool);

   if (! d->stealRunnable(runnable)) {
      return;
   }

   if (runnable->autoDelete() && !--runnable->ref) {
      delete runnable;
   }
}

