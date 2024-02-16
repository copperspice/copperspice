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

#include <qcoreapplication.h>
#include <qmutex.h>
#include <qpointer.h>
#include <qt_windows.h>
#include <qthreadstorage.h>

#include <qcoreapplication_p.h>
#include <qeventdispatcher_win_p.h>
#include <qthread_p.h>

#ifndef _MT
#define _MT
#endif

#include <process.h>
void qt_watch_adopted_thread(const HANDLE adoptedThreadHandle, QThread *qthread);
DWORD WINAPI qt_adopted_thread_watcher_function(LPVOID);

static DWORD qt_current_thread_data_tls_index = TLS_OUT_OF_INDEXES;
void qt_create_tls()
{
   if (qt_current_thread_data_tls_index != TLS_OUT_OF_INDEXES) {
      return;
   }

   static QMutex mutex;
   QMutexLocker locker(&mutex);
   qt_current_thread_data_tls_index = TlsAlloc();
}

static void qt_free_tls()
{
   if (qt_current_thread_data_tls_index != TLS_OUT_OF_INDEXES) {
      TlsFree(qt_current_thread_data_tls_index);
      qt_current_thread_data_tls_index = TLS_OUT_OF_INDEXES;
   }
}
Q_DESTRUCTOR_FUNCTION(qt_free_tls)

void QThreadData::clearCurrentThreadData()
{
   TlsSetValue(qt_current_thread_data_tls_index, nullptr);
}

QThreadData *QThreadData::current(bool createIfNecessary)
{
   qt_create_tls();
   QThreadData *threadData = reinterpret_cast<QThreadData *>(TlsGetValue(qt_current_thread_data_tls_index));

   if (!threadData && createIfNecessary) {
      threadData = new QThreadData;

      // This needs to be called prior to new AdoptedThread() to
      // avoid recursion.

      TlsSetValue(qt_current_thread_data_tls_index, threadData);

      try {
         threadData->thread = new QAdoptedThread(threadData);
      } catch (...) {
         TlsSetValue(qt_current_thread_data_tls_index, nullptr);
         threadData->deref();
         threadData = nullptr;
         throw;
      }

      threadData->deref();
      threadData->isAdopted = true;
      threadData->threadId = reinterpret_cast<Qt::HANDLE>(quintptr(GetCurrentThreadId()));

      if (!QCoreApplicationPrivate::theMainThread) {
         QCoreApplicationPrivate::theMainThread = threadData->thread.load();
      } else {
         HANDLE realHandle = INVALID_HANDLE_VALUE;

         DuplicateHandle(GetCurrentProcess(),
               GetCurrentThread(),
               GetCurrentProcess(),
               &realHandle,
               0,
               FALSE,
               DUPLICATE_SAME_ACCESS);

         qt_watch_adopted_thread(realHandle, threadData->thread);
      }
   }

   return threadData;
}

void QAdoptedThread::init()
{
   d_func()->handle = GetCurrentThread();
   d_func()->id = GetCurrentThreadId();
}

static QVector<HANDLE> qt_adopted_thread_handles;
static QVector<QThread *> qt_adopted_qthreads;
static QMutex qt_adopted_thread_watcher_mutex;
static DWORD qt_adopted_thread_watcher_id = 0;
static HANDLE qt_adopted_thread_wakeup = nullptr;

void qt_watch_adopted_thread(const HANDLE adoptedThreadHandle, QThread *qthread)
{
   QMutexLocker lock(&qt_adopted_thread_watcher_mutex);

   if (GetCurrentThreadId() == qt_adopted_thread_watcher_id) {

      CloseHandle(adoptedThreadHandle);
      return;
   }

   qt_adopted_thread_handles.append(adoptedThreadHandle);
   qt_adopted_qthreads.append(qthread);

   // Start watcher thread if it is not already running.
   if (qt_adopted_thread_watcher_id == 0) {
      if (qt_adopted_thread_wakeup == nullptr) {
         qt_adopted_thread_wakeup = CreateEvent(nullptr, false, false, nullptr);
         qt_adopted_thread_handles.prepend(qt_adopted_thread_wakeup);
      }

      CloseHandle(CreateThread(nullptr, 0, qt_adopted_thread_watcher_function, nullptr, 0, &qt_adopted_thread_watcher_id));
   } else {
      SetEvent(qt_adopted_thread_wakeup);
   }
}

DWORD WINAPI qt_adopted_thread_watcher_function(LPVOID)
{
   while (true) {
      qt_adopted_thread_watcher_mutex.lock();

      if (qt_adopted_thread_handles.count() == 1) {
         qt_adopted_thread_watcher_id = 0;
         qt_adopted_thread_watcher_mutex.unlock();
         break;
      }

      QVector<HANDLE> handlesCopy = qt_adopted_thread_handles;
      qt_adopted_thread_watcher_mutex.unlock();

      DWORD ret = WAIT_TIMEOUT;
      int count;
      int offset;
      int loops = handlesCopy.size() / MAXIMUM_WAIT_OBJECTS;

      if (handlesCopy.size() % MAXIMUM_WAIT_OBJECTS) {
         ++loops;
      }

      if (loops == 1) {
         // no need to loop, no timeout
         offset = 0;
         count = handlesCopy.count();
         ret = WaitForMultipleObjects(handlesCopy.count(), handlesCopy.constData(), false, INFINITE);
      } else {
         int loop = 0;

         do {
            offset = loop * MAXIMUM_WAIT_OBJECTS;
            count = qMin(handlesCopy.count() - offset, MAXIMUM_WAIT_OBJECTS);
            ret = WaitForMultipleObjects(count, handlesCopy.constData() + offset, false, 100);
            loop = (loop + 1) % loops;
         } while (ret == WAIT_TIMEOUT);
      }

      if (ret == WAIT_FAILED || ret >= WAIT_OBJECT_0 + uint(count)) {
         qWarning("QThread() Internal error while waiting for adopted threads, %d", int(GetLastError()));
         continue;
      }

      const int handleIndex = offset + ret - WAIT_OBJECT_0;

      if (handleIndex == 0) {
         // New handle to watch was added.
         continue;

      } else {
         //          printf("(qt) - qt_adopted_thread_watcher_function... called\n");
         const int qthreadIndex = handleIndex - 1;

         qt_adopted_thread_watcher_mutex.lock();
         QThreadData *data = QThreadData::get2(qt_adopted_qthreads.at(qthreadIndex));
         qt_adopted_thread_watcher_mutex.unlock();

         if (data->isAdopted) {
            QThread *thread = data->thread;
            Q_ASSERT(thread);

            QThreadPrivate *thread_p = data->get_QThreadPrivate();
            Q_ASSERT(! thread_p->finished);

            thread_p->finish(thread);
         }

         data->deref();

         QMutexLocker lock(&qt_adopted_thread_watcher_mutex);
         CloseHandle(qt_adopted_thread_handles.at(handleIndex));

         qt_adopted_thread_handles.remove(handleIndex);
         qt_adopted_qthreads.remove(qthreadIndex);
      }
   }

   QThreadData *threadData = reinterpret_cast<QThreadData *>(TlsGetValue(qt_current_thread_data_tls_index));

   if (threadData) {
      threadData->deref();
   }

   return 0;
}

#if defined(Q_CC_MSVC)

#ifndef Q_OS_WIN64
#  define ULONG_PTR DWORD
#endif

struct tagTHREADNAME_INFO {
   DWORD dwType;      // must be 0x1000
   LPCSTR szName;     // pointer to name (in user addr space)
   HANDLE dwThreadID; // thread ID (-1=caller thread)
   DWORD dwFlags;     // reserved for future use, must be zero
};
using THREADNAME_INFO = tagTHREADNAME_INFO;

void qt_set_thread_name(HANDLE threadId, LPCSTR threadName)
{
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = threadId;
   info.dwFlags = 0;

   __try {
      RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (const ULONG_PTR *)&info);

   } __except (EXCEPTION_CONTINUE_EXECUTION) {
      // do nothing
   }
}
#endif

void QThreadPrivate::createEventDispatcher(QThreadData *data)
{
   QEventDispatcherWin32 *theEventDispatcher = new QEventDispatcherWin32;

   data->eventDispatcher.store(theEventDispatcher, std::memory_order_release);
   theEventDispatcher->startingUp();
}

unsigned int __stdcall QT_ENSURE_STACK_ALIGNED_FOR_SSE QThreadPrivate::start(void *arg)
{
   QThread *thr = reinterpret_cast<QThread *>(arg);
   QThreadData *data = QThreadData::get2(thr);

   qt_create_tls();
   TlsSetValue(qt_current_thread_data_tls_index, data);
   data->threadId = reinterpret_cast<Qt::HANDLE>(quintptr(GetCurrentThreadId()));

   QThread::setTerminationEnabled(false);

   {
      QMutexLocker locker(&thr->d_func()->mutex);
      data->quitNow = thr->d_func()->exited;
   }

   if (data->eventDispatcher.load())  {
      // custom event dispatcher set?
      data->eventDispatcher.load()->startingUp();
   } else {
      createEventDispatcher(data);
   }

#if defined(Q_CC_MSVC)
   // sets the name of the current thread
   QString objectName = thr->objectName();

   if (objectName.isEmpty()) {
      objectName = thr->metaObject()->className();
   }

   qt_set_thread_name((HANDLE) - 1, objectName.constData());
#endif

   emit thr->started();
   QThread::setTerminationEnabled(true);
   thr->run();

   finish(arg);

   return 0;
}

void QThreadPrivate::finish(void *arg, bool lockAnyway)
{
   QThread *thr = reinterpret_cast<QThread *>(arg);
   QThreadPrivate *d = thr->d_func();

   QMutexLocker locker(lockAnyway ? &d->mutex : nullptr);
   d->isInFinish = true;
   d->priority = QThread::InheritPriority;

   void **tls_data = reinterpret_cast<void **>(&d->data->tls);
   locker.unlock();
   emit thr->finished();

   QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
   QThreadStorageData::finish(tls_data);
   locker.relock();

   QAbstractEventDispatcher *eventDispatcher = d->data->eventDispatcher.load();

   if (eventDispatcher) {
      d->data->eventDispatcher = nullptr;
      locker.unlock();
      eventDispatcher->closingDown();
      delete eventDispatcher;
      locker.relock();
   }

   d->running = false;
   d->finished = true;
   d->isInFinish = false;
   d->interruptionRequested = false;

   if (!d->waiters) {
      CloseHandle(d->handle);
      d->handle = nullptr;
   }

   d->id = 0;

}

Qt::HANDLE QThread::currentThreadId()
{
   return reinterpret_cast<Qt::HANDLE>(quintptr(GetCurrentThreadId()));
}

int QThread::idealThreadCount()
{
   SYSTEM_INFO sysinfo;
   GetSystemInfo(&sysinfo);
   return sysinfo.dwNumberOfProcessors;
}

void QThread::yieldCurrentThread()
{
   SwitchToThread();
}

void QThread::sleep(unsigned long secs)
{
   ::Sleep(secs * 1000);
}

void QThread::msleep(unsigned long msecs)
{
   ::Sleep(msecs);
}

void QThread::usleep(unsigned long usecs)
{
   ::Sleep((usecs / 1000) + 1);
}

void QThread::start(Priority priority)
{
   Q_D(QThread);
   QMutexLocker locker(&d->mutex);

   if (d->isInFinish) {
      locker.unlock();
      wait();
      locker.relock();
   }

   if (d->running) {
      return;
   }

   d->running    = true;
   d->finished   = false;
   d->exited     = false;
   d->returnCode = 0;

   d->interruptionRequested = false;

   /*
     create the thread in the suspended state, set the priority and then resume the thread.

     since threads are created with normal priority by default, we
     could get into a case where a thread (with priority less than
     NormalPriority) tries to create a new thread (also with priority
     less than NormalPriority), but the newly created thread preempts
     its 'parent' and runs at normal priority.
   */

   d->handle = (Qt::HANDLE) _beginthreadex(nullptr, d->stackSize, QThreadPrivate::start,
         this, CREATE_SUSPENDED, &(d->id));

   if (!d->handle) {
      qErrnoWarning(errno, "QThread::start: Failed to create thread");
      d->running = false;
      d->finished = true;
      return;
   }

   int prio;
   d->priority = priority;

   switch (d->priority) {
      case IdlePriority:
         prio = THREAD_PRIORITY_IDLE;
         break;

      case LowestPriority:
         prio = THREAD_PRIORITY_LOWEST;
         break;

      case LowPriority:
         prio = THREAD_PRIORITY_BELOW_NORMAL;
         break;

      case NormalPriority:
         prio = THREAD_PRIORITY_NORMAL;
         break;

      case HighPriority:
         prio = THREAD_PRIORITY_ABOVE_NORMAL;
         break;

      case HighestPriority:
         prio = THREAD_PRIORITY_HIGHEST;
         break;

      case TimeCriticalPriority:
         prio = THREAD_PRIORITY_TIME_CRITICAL;
         break;

      case InheritPriority:
      default:
         prio = GetThreadPriority(GetCurrentThread());
         break;
   }

   if (!SetThreadPriority(d->handle, prio)) {
      qErrnoWarning("QThread::start: Failed to set thread priority");
   }

   if (ResumeThread(d->handle) == (DWORD) - 1) {
      qErrnoWarning("QThread::start: Failed to resume new thread");
   }
}

void QThread::terminate()
{
   Q_D(QThread);
   QMutexLocker locker(&d->mutex);

   if (!d->running) {
      return;
   }

   if (!d->terminationEnabled) {
      d->terminatePending = true;
      return;
   }

   TerminateThread(d->handle, 0);
   QThreadPrivate::finish(this, false);
}

bool QThread::wait(unsigned long time)
{
   Q_D(QThread);
   QMutexLocker locker(&d->mutex);

   if (d->id == GetCurrentThreadId()) {
      qWarning("QThread::wait() Thread tried to wait on itself");
      return false;
   }

   if (d->finished || ! d->running) {
      return true;
   }

   ++d->waiters;
   locker.mutex()->unlock();

   bool ret = false;

   switch (WaitForSingleObject(d->handle, time)) {
      case WAIT_OBJECT_0:
         ret = true;
         break;

      case WAIT_FAILED:
         qErrnoWarning("QThread::wait: Thread wait failure");
         break;

      case WAIT_ABANDONED:
      case WAIT_TIMEOUT:
      default:
         break;
   }

   locker.mutex()->lock();
   --d->waiters;

   if (ret && ! d->finished) {
      // thread was terminated by someone else
      QThreadPrivate::finish(this, false);
   }

   if (d->finished && ! d->waiters) {
      CloseHandle(d->handle);
      d->handle = nullptr;
   }

   return ret;
}

void QThread::setTerminationEnabled(bool enabled)
{
   QThread *thr = currentThread();
   Q_ASSERT_X(thr != nullptr, "QThread::setTerminationEnabled()",
         "Current thread was not started with QThread.");

   QThreadPrivate *d = thr->d_func();
   QMutexLocker locker(&d->mutex);
   d->terminationEnabled = enabled;

   if (enabled && d->terminatePending) {
      QThreadPrivate::finish(thr, false);
      locker.unlock(); // don't leave the mutex locked!
      _endthreadex(0);
   }
}

void QThreadPrivate::setPriority(QThread::Priority threadPriority)
{
   // copied from start() with a few modifications:

   int prio;
   priority = threadPriority;

   switch (priority) {
      case QThread::IdlePriority:
         prio = THREAD_PRIORITY_IDLE;
         break;

      case QThread::LowestPriority:
         prio = THREAD_PRIORITY_LOWEST;
         break;

      case QThread::LowPriority:
         prio = THREAD_PRIORITY_BELOW_NORMAL;
         break;

      case QThread::NormalPriority:
         prio = THREAD_PRIORITY_NORMAL;
         break;

      case QThread::HighPriority:
         prio = THREAD_PRIORITY_ABOVE_NORMAL;
         break;

      case QThread::HighestPriority:
         prio = THREAD_PRIORITY_HIGHEST;
         break;

      case QThread::TimeCriticalPriority:
         prio = THREAD_PRIORITY_TIME_CRITICAL;
         break;

      case QThread::InheritPriority:
      default:
         qWarning("QThread::setPriority() Thread priority can not be QThread::InheritPriority");
         return;
   }

   if (!SetThreadPriority(handle, prio)) {
      qErrnoWarning("QThread::setPriority() Failed to set thread priority");
   }
}
