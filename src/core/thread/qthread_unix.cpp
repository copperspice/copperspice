/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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
#include <qplatformdefs.h>
#include <qcoreapplication_p.h>

#if defined(Q_OS_DARWIN)
#  include <qeventdispatcher_cf_p.h>
#  include <qeventdispatcher_unix_p.h>
#else

#if ! defined(QT_NO_GLIB)
#include <qeventdispatcher_glib_p.h>
#endif

#include <qeventdispatcher_unix_p.h>
#endif

#include <qthreadstorage.h>
#include <qthread_p.h>
#include <qdebug.h>

#include <sched.h>
#include <errno.h>

#ifdef Q_OS_BSD4
#include <sys/sysctl.h>
#endif

#if defined(Q_OS_DARWIN)
# ifdef qDebug
#   define old_qDebug qDebug
#   undef qDebug
# endif

#if ! defined(Q_OS_IOS)
# include <CoreServices/CoreServices.h> // emerald, may delete
#endif

# ifdef old_qDebug
#   undef qDebug
#   define qDebug QT_NO_QDEBUG_MACRO
#   undef old_qDebug
# endif
#endif

#if defined(Q_OS_LINUX) && !defined(QT_LINUXBASE)
#include <sys/prctl.h>
#endif

#if defined(Q_OS_LINUX) && !defined(SCHED_IDLE)
// from linux/sched.h
# define SCHED_IDLE    5
#endif

#if defined(Q_OS_DARWIN) || !defined(Q_OS_OPENBSD) && defined(_POSIX_THREAD_PRIORITY_SCHEDULING) && (_POSIX_THREAD_PRIORITY_SCHEDULING-0 >= 0)
#define QT_HAS_THREAD_PRIORITY_SCHEDULING
#endif

static_assert(sizeof(pthread_t) <= sizeof(Qt::HANDLE), "Pthread size mismatch");
enum { ThreadPriorityResetFlag = 0x80000000 };

#if defined(Q_OS_LINUX) && defined(__GLIBC__) && (defined(Q_CC_GNU) || defined(Q_CC_INTEL))
#define HAVE_TLS
#endif

#ifdef HAVE_TLS
static __thread QThreadData *currentThreadData = nullptr;
#endif

static pthread_once_t current_thread_data_once = PTHREAD_ONCE_INIT;
static pthread_key_t current_thread_data_key;

static void destroy_current_thread_data(void *p)
{
   // POSIX says the value in our key is set to zero before calling
   // this destructor function, so we need to set it back to the right value

   pthread_setspecific(current_thread_data_key, p);
   QThreadData *data = static_cast<QThreadData *>(p);

   if (data->isAdopted) {
      QThread *thread = data->thread;
      Q_ASSERT(thread);

      QThreadPrivate *thread_p = static_cast<QThreadPrivate *>(QThreadPrivate::cs_getPrivate(thread));
      Q_ASSERT(!thread_p->finished);

      thread_p->finish(thread);
   }
   data->deref();

   // reset it to zero before returning so we are not
   // called again (POSIX allows implementations to call destructor
   // functions repeatedly until all values are zero)
   pthread_setspecific(current_thread_data_key, 0);
}

static void create_current_thread_data_key()
{
   pthread_key_create(&current_thread_data_key, destroy_current_thread_data);
}

static void destroy_current_thread_data_key()
{
   pthread_once(&current_thread_data_once, create_current_thread_data_key);
   pthread_key_delete(current_thread_data_key);

   // Reset current_thread_data_once in case we end up recreating
   // the thread-data in the rare case of QObject construction
   // after destroying the QThreadData.
   pthread_once_t pthread_once_init = PTHREAD_ONCE_INIT;
   current_thread_data_once = pthread_once_init;
}
Q_DESTRUCTOR_FUNCTION(destroy_current_thread_data_key)


// Utility functions for getting, setting and clearing thread specific data.
static QThreadData *get_thread_data()
{
#ifdef HAVE_TLS
   return currentThreadData;
#else
   pthread_once(&current_thread_data_once, create_current_thread_data_key);
   return reinterpret_cast<QThreadData *>(pthread_getspecific(current_thread_data_key));
#endif
}

static void set_thread_data(QThreadData *data)
{
#ifdef HAVE_TLS
   currentThreadData = data;
#endif
   pthread_once(&current_thread_data_once, create_current_thread_data_key);
   pthread_setspecific(current_thread_data_key, data);
}

static void clear_thread_data()
{
#ifdef HAVE_TLS
   currentThreadData = 0;
#endif
   pthread_setspecific(current_thread_data_key, 0);
}

template <typename T>
static typename std::enable_if<QTypeInfo<T>::isIntegral, Qt::HANDLE>::type to_HANDLE(T id)
{
    return reinterpret_cast<Qt::HANDLE>(static_cast<intptr_t>(id));
}

template <typename T>
static typename std::enable_if<QTypeInfo<T>::isIntegral, T>::type from_HANDLE(Qt::HANDLE id)
{
    return static_cast<T>(reinterpret_cast<intptr_t>(id));
}

template <typename T>
static typename std::enable_if<QTypeInfo<T>::isPointer, Qt::HANDLE>::type to_HANDLE(T id)
{
    return id;
}

template <typename T>
static typename std::enable_if<QTypeInfo<T>::isPointer, T>::type from_HANDLE(Qt::HANDLE id)
{
    return static_cast<T>(id);
}
void QThreadData::clearCurrentThreadData()
{
   clear_thread_data();
}

QThreadData *QThreadData::current(bool createIfNecessary)
{
   QThreadData *data = get_thread_data();

   if (! data && createIfNecessary) {
      data = new QThreadData;

      try {
         set_thread_data(data);
         data->thread = new QAdoptedThread(data);

      } catch (...) {
         clear_thread_data();
         data->deref();
         data = 0;
         throw;
      }

      data->deref();
      data->isAdopted = true;
      data->threadId = to_HANDLE(pthread_self());

      if (!QCoreApplicationPrivate::theMainThread) {
         QCoreApplicationPrivate::theMainThread = data->thread.load();
      }
   }

   return data;
}

void QAdoptedThread::init()
{
   Q_D(QThread);
   d->thread_id = pthread_self();
}


#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

   typedef void *(*QtThreadCallback)(void *);

#if defined(Q_C_CALLBACKS)
}
#endif


void QThreadPrivate::createEventDispatcher(QThreadData *data)
{
#if defined(Q_OS_DARWIN)
    bool ok = false;
    int value = qgetenv("QT_EVENT_DISPATCHER_CORE_FOUNDATION").toInt(&ok);

    if (ok && value > 0)
        data->eventDispatcher.store(new QEventDispatcherCoreFoundation, std::memory_order_release);
    else
        data->eventDispatcher.store(new QEventDispatcherUNIX, std::memory_order_release);

#elif ! defined(QT_NO_GLIB)
   if (qgetenv("QT_NO_GLIB").isEmpty() && qgetenv("QT_NO_THREADED_GLIB").isEmpty()
            && QEventDispatcherGlib::versionSupported()) {
      data->eventDispatcher.store(new QEventDispatcherGlib);

   } else
#else
    data->eventDispatcher.store(new QEventDispatcherUNIX);
#endif

   data->eventDispatcher.load()->startingUp();
}

#if defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)

static void setCurrentThreadName(pthread_t threadId, const QString &name)
{
#if defined(Q_OS_LINUX) && ! defined(QT_LINUXBASE)
   prctl(PR_SET_NAME, (unsigned long)name.constData(), 0, 0, 0);

#elif defined(Q_OS_DARWIN)
   pthread_setname_np(name.constData());

#endif
}

#endif

void *QThreadPrivate::start(void *arg)
{
   pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
   pthread_cleanup_push(QThreadPrivate::finish, arg);

   QThread *thr = reinterpret_cast<QThread *>(arg);
   QThreadData *data = QThreadData::get2(thr);

   {
      QMutexLocker locker(&thr->d_func()->mutex);
      // do we need to reset the thread priority?
      if (int(thr->d_func()->priority) & ThreadPriorityResetFlag) {
         thr->setPriority(QThread::Priority(thr->d_func()->priority & ~ThreadPriorityResetFlag));
      }

      data->threadId = to_HANDLE(pthread_self());
      set_thread_data(data);

      data->ref();
      data->quitNow = thr->d_func()->exited;
   }

   if (data->eventDispatcher.load()) // custom event dispatcher set?
      data->eventDispatcher.load()->startingUp();
   else
      createEventDispatcher(data);

#if defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
   // sets the name of the current thread.
   QString objectName = thr->objectName();

   pthread_t thread_id = from_HANDLE<pthread_t>(data->threadId);
   if (objectName.isEmpty()) {
      setCurrentThreadName(thread_id, thr->metaObject()->className());
   } else {
      setCurrentThreadName(thread_id, objectName);
   }

#endif

   emit thr->started();
   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   pthread_testcancel();
   thr->run();

   pthread_cleanup_pop(1);

   return 0;
}

void QThreadPrivate::finish(void *arg)
{
   QThread *thr = reinterpret_cast<QThread *>(arg);
   QThreadPrivate *d = thr->d_func();

   QMutexLocker locker(&d->mutex);

   d->isInFinish = true;
   d->priority = QThread::InheritPriority;

   void *data = &d->data->tls;
   locker.unlock();

   emit thr->finished();
   QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
   QThreadStorageData::finish((void **)data);
   locker.relock();

   QAbstractEventDispatcher *eventDispatcher = d->data->eventDispatcher.load();
   if (eventDispatcher) {
      d->data->eventDispatcher = 0;
      locker.unlock();
      eventDispatcher->closingDown();
      delete eventDispatcher;
      locker.relock();
   }

   d->running = false;
   d->finished = true;
   d->interruptionRequested = false;

   d->isInFinish = false;
   d->thread_done.wakeAll();
}


Qt::HANDLE QThread::currentThreadId()
{
   // requires a C cast here otherwise we run into trouble on AIX
    return to_HANDLE(pthread_self());
}

#if defined(QT_LINUXBASE) && !defined(_SC_NPROCESSORS_ONLN)
// LSB doesn't define _SC_NPROCESSORS_ONLN.
#  define _SC_NPROCESSORS_ONLN 84
#endif

int QThread::idealThreadCount()
{
   int cores = -1;

#if defined(Q_OS_BSD4)
   // FreeBSD, OpenBSD, NetBSD, BSD/OS
   size_t len = sizeof(cores);
   int mib[2];
   mib[0] = CTL_HW;
   mib[1] = HW_NCPU;

   if (sysctl(mib, 2, &cores, &len, NULL, 0) != 0) {
       perror("sysctl");
   }

#else
   // the rest: Linux, Solaris, AIX, Tru64
   cores = (int)sysconf(_SC_NPROCESSORS_ONLN);

   if (cores == -1) {
      return 1;
   }
#endif

   return cores;
}

void QThread::yieldCurrentThread()
{
   sched_yield();
}

/*  \internal
    helper function to do thread sleeps, since usleep()/nanosleep()
    aren't reliable enough (in terms of behavior and availability)
*/
static void thread_sleep(struct timespec *ti)
{
   pthread_mutex_t mtx;
   pthread_cond_t cnd;

   pthread_mutex_init(&mtx, 0);
   pthread_cond_init(&cnd, 0);

   pthread_mutex_lock(&mtx);
   (void) pthread_cond_timedwait(&cnd, &mtx, ti);
   pthread_mutex_unlock(&mtx);

   pthread_cond_destroy(&cnd);
   pthread_mutex_destroy(&mtx);
}

void QThread::sleep(unsigned long secs)
{
   struct timeval tv;
   gettimeofday(&tv, 0);
   struct timespec ti;
   ti.tv_sec = tv.tv_sec + secs;
   ti.tv_nsec = (tv.tv_usec * 1000);
   thread_sleep(&ti);
}

void QThread::msleep(unsigned long msecs)
{
   struct timeval tv;
   gettimeofday(&tv, 0);
   struct timespec ti;

   ti.tv_nsec = (tv.tv_usec + (msecs % 1000) * 1000) * 1000;
   ti.tv_sec = tv.tv_sec + (msecs / 1000) + (ti.tv_nsec / 1000000000);
   ti.tv_nsec %= 1000000000;
   thread_sleep(&ti);
}

void QThread::usleep(unsigned long usecs)
{
   struct timeval tv;
   gettimeofday(&tv, 0);
   struct timespec ti;

   ti.tv_nsec = (tv.tv_usec + (usecs % 1000000)) * 1000;
   ti.tv_sec = tv.tv_sec + (usecs / 1000000) + (ti.tv_nsec / 1000000000);
   ti.tv_nsec %= 1000000000;
   thread_sleep(&ti);
}

#ifdef QT_HAS_THREAD_PRIORITY_SCHEDULING
// Does some magic and calculate the Unix scheduler priorities
// sched_policy is IN/OUT: it must be set to a valid policy before calling this function
// sched_priority is OUT only
static bool calculateUnixPriority(int priority, int *sched_policy, int *sched_priority)
{

#ifdef SCHED_IDLE
   if (priority == QThread::IdlePriority) {
      *sched_policy = SCHED_IDLE;
      *sched_priority = 0;
      return true;
   }
   const int lowestPriority = QThread::LowestPriority;
#else
   const int lowestPriority = QThread::IdlePriority;
#endif
   const int highestPriority = QThread::TimeCriticalPriority;

   int prio_min = sched_get_priority_min(*sched_policy);
   int prio_max = sched_get_priority_max(*sched_policy);
   if (prio_min == -1 || prio_max == -1) {
      return false;
   }

   int prio;
   // crudely scale our priority enum values to the prio_min/prio_max
   prio = ((priority - lowestPriority) * (prio_max - prio_min) / highestPriority) + prio_min;
   prio = qMax(prio_min, qMin(prio_max, prio));

   *sched_priority = prio;
   return true;
}
#endif

void QThread::start(Priority priority)
{
   Q_D(QThread);
   QMutexLocker locker(&d->mutex);

   if (d->isInFinish) {
      d->thread_done.wait(locker.mutex());
   }

   if (d->running) {
      return;
   }

   d->running = true;
   d->finished = false;

   d->returnCode = 0;
   d->exited = false;
   d->interruptionRequested = false;

   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

   d->priority = priority;

#if defined(QT_HAS_THREAD_PRIORITY_SCHEDULING)
   switch (priority) {
      case InheritPriority: {
         pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
         break;
      }

      default: {
         int sched_policy;
         if (pthread_attr_getschedpolicy(&attr, &sched_policy) != 0) {
            // failed to get the scheduling policy, don't bother
            // setting the priority
            qWarning("QThread::start: Cannot determine default scheduler policy");
            break;
         }

         int prio;
         if (!calculateUnixPriority(priority, &sched_policy, &prio)) {
            // failed to get the scheduling parameters, don't
            // bother setting the priority
            qWarning("QThread::start: Cannot determine scheduler priority range");
            break;
         }

         sched_param sp;
         sp.sched_priority = prio;

         if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0
               || pthread_attr_setschedpolicy(&attr, sched_policy) != 0
               || pthread_attr_setschedparam(&attr, &sp) != 0) {
            // could not set scheduling hints, fallback to inheriting them
            // we'll try again from inside the thread
            pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
            d->priority = Priority(priority | ThreadPriorityResetFlag);
         }
         break;
      }
   }
#endif // QT_HAS_THREAD_PRIORITY_SCHEDULING


   if (d->stackSize > 0) {
#if defined(_POSIX_THREAD_ATTR_STACKSIZE) && (_POSIX_THREAD_ATTR_STACKSIZE-0 > 0)
      int code = pthread_attr_setstacksize(&attr, d->stackSize);
#else
      int code = ENOSYS; // stack size not supported, automatically fail
#endif

      if (code) {
         qWarning("QThread::start: Thread stack size error: %s",
                  qPrintable(qt_error_string(code)));

         // we failed to set the stacksize, and as the documentation states,
         // the thread will fail to run...
         d->running = false;
         d->finished = false;
         return;
      }
   }

   pthread_t threadId;
   int code = pthread_create(&threadId, &attr, QThreadPrivate::start, this);

   if (code == EPERM) {
      // caller does not have permission to set the scheduling
      // parameters/policy
#if defined(QT_HAS_THREAD_PRIORITY_SCHEDULING)
      pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
#endif
      code = pthread_create(&threadId, &attr, QThreadPrivate::start, this);
   }

   d->data->threadId = to_HANDLE(threadId);

   pthread_attr_destroy(&attr);

   if (code) {
      qWarning("QThread::start: Thread creation error: %s", qPrintable(qt_error_string(code)));

      d->running = false;
      d->finished = false;
      d->data->threadId = 0;
   }
}

void QThread::terminate()
{
   Q_D(QThread);
   QMutexLocker locker(&d->mutex);

   if (!d->data->threadId) {
      return;
   }

   int code = pthread_cancel(from_HANDLE<pthread_t>(d->data->threadId));
   if (code) {
      qWarning("QThread::start: Thread termination error: %s",
               csPrintable(qt_error_string((code))));
   }
}

bool QThread::wait(unsigned long time)
{
   Q_D(QThread);
   QMutexLocker locker(&d->mutex);

    if (from_HANDLE<pthread_t>(d->data->threadId) == pthread_self()) {
      qWarning("QThread::wait: Thread tried to wait on itself");
      return false;
   }

   if (d->finished || !d->running) {
      return true;
   }

   while (d->running) {
      if (!d->thread_done.wait(locker.mutex(), time)) {
         return false;
      }
   }
   return true;
}

void QThread::setTerminationEnabled(bool enabled)
{
   QThread *thr = currentThread();
   Q_ASSERT_X(thr != 0, "QThread::setTerminationEnabled()",
              "Current thread was not started with QThread.");

   pthread_setcancelstate(enabled ? PTHREAD_CANCEL_ENABLE : PTHREAD_CANCEL_DISABLE, NULL);
   if (enabled) {
      pthread_testcancel();
   }
}

// Caller must lock the mutex
void QThreadPrivate::setPriority(QThread::Priority threadPriority)
{
    priority = threadPriority;

   // copied from start() with a few modifications:

#ifdef QT_HAS_THREAD_PRIORITY_SCHEDULING
   int sched_policy;
   sched_param param;

   if (pthread_getschedparam(from_HANDLE<pthread_t>(data->threadId), &sched_policy, &param) != 0) {
      // failed to get the scheduling policy, don't bother setting
      // the priority
      qWarning("QThread::setPriority: Cannot get scheduler parameters");
      return;
   }

   int prio;
   if (! calculateUnixPriority(priority, &sched_policy, &prio)) {
      // failed to get the scheduling parameters, don't
      // bother setting the priority
      qWarning("QThread::setPriority: Cannot determine scheduler priority range");
      return;
   }

   param.sched_priority = prio;
    int status = pthread_setschedparam(from_HANDLE<pthread_t>(data->threadId), sched_policy, &param);

# ifdef SCHED_IDLE
   // were we trying to set to idle priority and failed?
   if (status == -1 && sched_policy == SCHED_IDLE && errno == EINVAL) {
      // reset to lowest priority possible
        pthread_getschedparam(from_HANDLE<pthread_t>(data->threadId), &sched_policy, &param);
      param.sched_priority = sched_get_priority_min(sched_policy);
        pthread_setschedparam(from_HANDLE<pthread_t>(data->threadId), sched_policy, &param);
   }
# endif

#endif
}


