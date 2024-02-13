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

#include <qthread.h>

#include <qabstracteventdispatcher.h>
#include <qeventloop.h>
#include <qhash.h>
#include <qmutex.h>
#include <qreadwritelock.h>
#include <qthreadstorage.h>

#include <qcoreapplication_p.h>
#include <qmutexpool_p.h>
#include <qthread_p.h>

QThreadData::QThreadData(int initialRefCount)
   : thread(nullptr), threadId(nullptr), eventDispatcher(nullptr), loopLevel(0),
     quitNow(false), canWait(true), isAdopted(false), requiresCoreApplication(true), m_ref(initialRefCount)
{
   // fprintf(stderr, "QThreadData %p created\n", this);
}

QThreadData::~QThreadData()
{
   Q_ASSERT(m_ref.load() == 0);

   // In the odd case we are running on a secondary thread, the main
   // thread instance will have been dereffed asunder because of the deref in
   // QThreadData::current() and the deref in the pthread_destroy. To avoid
   // crashing during QCoreApplicationData's global static cleanup we need to
   // safeguard the main thread here.

   if (this->thread == QCoreApplicationPrivate::theMainThread) {
      QCoreApplicationPrivate::theMainThread = nullptr;
      QThreadData::clearCurrentThreadData();
   }

   QThread *t = thread;
   thread = nullptr;
   delete t;

   for (int i = 0; i < postEventList.size(); ++i) {
      const QPostEvent &pe = postEventList.at(i);

      if (pe.event) {
         CSInternalEvents::decr_PostedEvents(pe.receiver);
         pe.event->posted = false;

         delete pe.event;
      }
   }

   // fprintf(stderr, "QThreadData %p destroyed\n", this);
}

void QThreadData::ref()
{
   m_ref.ref();
   Q_ASSERT(m_ref.load() != 0);
}

void QThreadData::deref()
{
   if (! m_ref.deref()) {
      delete this;
   }
}

QThreadPrivate *QThreadData::get_QThreadPrivate() const
{
   return this->thread.load()->d_func();
}

QAdoptedThread::QAdoptedThread(QThreadData *data)
   : QThread(*new QThreadPrivate(data))
{
   // thread should be running and not finished for the lifetime
   // of the application (even if QCoreApplication goes away)

   d_func()->running = true;
   d_func()->finished = false;

   init();

   // fprintf(stderr, "new QAdoptedThread = %p\n", this);
}

QAdoptedThread::~QAdoptedThread()
{
   // fprintf(stderr, "~QAdoptedThread = %p\n", this);
}

void QAdoptedThread::run()
{
   // this function should never be called
   qFatal("QAdoptedThread::run(): Internal error, this implementation should never be called");
}

QThreadPrivate::QThreadPrivate(QThreadData *d)
   : running(false), finished(false), isInFinish(false),
     interruptionRequested(false), exited(false), returnCode(-1),
     stackSize(0), priority(QThread::InheritPriority), data(d)
{

#if defined (Q_OS_WIN)
   handle  = nullptr;
   id      = 0;
   waiters = 0;

   terminationEnabled = true;
   terminatePending = false;
#endif

   if (! data) {
      data = new QThreadData;
   }
}

QThreadPrivate::~QThreadPrivate()
{
   data->deref();
}

QThread *QThread::currentThread()
{
   QThreadData *data = QThreadData::current();
   Q_ASSERT(data != nullptr);
   return data->thread;
}

QThread::QThread(QObject *parent)
   : QObject(parent), d_ptr(new QThreadPrivate)
{
   d_ptr->q_ptr = this;

   Q_D(QThread);

   // fprintf(stderr, "QThreadData %p created for thread %p\n", d->data, this);
   d->data->thread = this;
}

QThread::QThread(QThreadPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;

   Q_D(QThread);

   // fprintf(stderr, "QThreadData %p taken from private data for thread %p\n", d->data, this);
   d->data->thread = this;
}

QThread::~QThread()
{
   Q_D(QThread);
   {
      QMutexLocker locker(&d->mutex);

      if (d->isInFinish) {
         locker.unlock();
         wait();
         locker.relock();
      }

      if (d->running && ! d->finished && ! d->data->isAdopted) {
         qWarning("QThread() Destroyed while thread is still running");
      }

      d->data->thread = nullptr;
   }
}

bool QThread::isFinished() const
{
   Q_D(const QThread);
   QMutexLocker locker(&d->mutex);
   return d->finished || d->isInFinish;
}

bool QThread::isRunning() const
{
   Q_D(const QThread);
   QMutexLocker locker(&d->mutex);

   return d->running && ! d->isInFinish;
}

void QThread::setStackSize(uint stackSize)
{
   Q_D(QThread);
   QMutexLocker locker(&d->mutex);

   Q_ASSERT_X(! d->running, "QThread::setStackSize", "Unable to change stack size while the thread is running");

   d->stackSize = stackSize;
}

uint QThread::stackSize() const
{
   Q_D(const QThread);
   QMutexLocker locker(&d->mutex);
   return d->stackSize;
}

int QThread::exec()
{
   Q_D(QThread);

   QMutexLocker locker(&d->mutex);
   d->data->quitNow = false;

   if (d->exited) {
      d->exited = false;
      return d->returnCode;
   }

   locker.unlock();

   QEventLoop eventLoop;
   int returnCode = eventLoop.exec();

   locker.relock();
   d->exited = false;
   d->returnCode = -1;
   return returnCode;
}

void QThread::exit(int returnCode)
{
   Q_D(QThread);
   QMutexLocker locker(&d->mutex);

   d->exited = true;
   d->returnCode = returnCode;
   d->data->quitNow = true;

   for (int i = 0; i < d->data->eventLoops.size(); ++i) {
      QEventLoop *eventLoop = d->data->eventLoops.at(i);
      eventLoop->exit(returnCode);
   }
}

void QThread::quit()
{
   exit();
}

void QThread::run()
{
   (void) exec();
}

void QThread::setPriority(Priority priority)
{
   Q_D(QThread);
   QMutexLocker locker(&d->mutex);

   if (! d->running) {
      qWarning("QThread::setPriority() Unable to set priority while thread is not running");
      return;
   }

   d->setPriority(priority);
}

QThread::Priority QThread::priority() const
{
   Q_D(const QThread);
   QMutexLocker locker(&d->mutex);

   // mask off the high bits that are used for flags
   return Priority(d->priority & 0xffff);
}

int QThread::loopLevel() const
{
   Q_D(const QThread);
   return d->data->eventLoops.size();
}

QAbstractEventDispatcher *QThread::eventDispatcher() const
{
   Q_D(const QThread);
   return d->data->eventDispatcher.load();
}

void QThread::setEventDispatcher(QAbstractEventDispatcher *eventDispatcher)
{
   Q_D(QThread);

   if (d->data->hasEventDispatcher()) {
      qWarning("QThread::setEventDispatcher() Event dispatcher has already been created for this thread");
   } else {
      eventDispatcher->moveToThread(this);

      if (eventDispatcher->thread() == this)  {
         // was the move successful?
         d->data->eventDispatcher = eventDispatcher;
      } else {
         qWarning("QThread::setEventDispatcher() Unable to move event dispatcher to target thread");
      }
   }
}

bool QThread::event(QEvent *event)
{
   if (event->type() == QEvent::Quit) {
      quit();
      return true;
   } else {
      return QObject::event(event);
   }
}

void QThread::requestInterruption()
{
   Q_D(QThread);
   QMutexLocker locker(&d->mutex);

   if (! d->running || d->finished || d->isInFinish) {
      return;
   }

   if (this == QCoreApplicationPrivate::theMainThread) {
      qWarning("QThread::requestInterruption() Unable to interrupt main thread");
      return;
   }

   d->interruptionRequested = true;
}

bool QThread::isInterruptionRequested() const
{
   Q_D(const QThread);
   QMutexLocker locker(&d->mutex);

   if (! d->running || d->finished || d->isInFinish) {
      return false;
   }

   return d->interruptionRequested;
}

static void setThreadDoesNotRequireCoreApplication()
{
   QThreadData::current()->requiresCoreApplication = false;
}

QDaemonThread::QDaemonThread(QObject *parent)
   : QThread(parent)
{
   connect(this, &QThread::started, this, setThreadDoesNotRequireCoreApplication);
}

QDaemonThread::~QDaemonThread()
{
}

QThreadPrivate *QThreadPrivate::cs_getPrivate(QThread *object)
{
   return object->d_ptr.data();
}
