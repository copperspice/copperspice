/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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
#include <qthreadstorage.h>
#include <qmutex.h>
#include <qmutexpool_p.h>
#include <qreadwritelock.h>
#include <qabstracteventdispatcher.h>
#include <qeventloop.h>
#include <qhash.h>
#include <qthread_p.h>
#include <qcoreapplication_p.h>

QT_BEGIN_NAMESPACE

QThreadData::QThreadData(int initialRefCount)
   : _ref(initialRefCount), thread(0), threadId(0),
     quitNow(false), loopLevel(0), eventDispatcher(0), canWait(true), isAdopted(false)
{
   // fprintf(stderr, "QThreadData %p created\n", this);
}

QThreadData::~QThreadData()
{
   Q_ASSERT(_ref.load() == 0);

   // In the odd case that Qt is running on a secondary thread, the main
   // thread instance will have been dereffed asunder because of the deref in
   // QThreadData::current() and the deref in the pthread_destroy. To avoid
   // crashing during QCoreApplicationData's global static cleanup we need to
   // safeguard the main thread here.. This fix is a bit crude, but it solves
   // the problem...
   if (this->thread == QCoreApplicationPrivate::theMainThread) {
      QCoreApplicationPrivate::theMainThread = 0;
      QThreadData::clearCurrentThreadData();
   }

   QThread *t = thread;
   thread = 0;
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
   (void) _ref.ref();
   Q_ASSERT(_ref.load() != 0);
}

void QThreadData::deref()
{
   if (!_ref.deref()) {
      delete this;
   }
}

QThreadPrivate *QThreadData::get_QThreadPrivate() const
{
   return this->thread->d_func();
}


/*
  QAdoptedThread
*/

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
   qFatal("QAdoptedThread::run(): Internal error, this implementation should never be called.");
}


/*
  QThreadPrivate
*/

QThreadPrivate::QThreadPrivate(QThreadData *d)
   : running(false), finished(false), terminated(false),
     isInFinish(false), exited(false), returnCode(-1),
     stackSize(0), priority(QThread::InheritPriority), data(d)
{
#if defined (Q_OS_UNIX)
   thread_id = 0;
#elif defined (Q_OS_WIN)
   handle = 0;
   id = 0;
   waiters = 0;
#endif
#if defined (Q_OS_WIN)
   terminationEnabled = true;
   terminatePending = false;
#endif

   if (!data) {
      data = new QThreadData;
   }
}

QThreadPrivate::~QThreadPrivate()
{
   data->deref();
}

/*!
    \class QThread
    \brief The QThread class provides a platform-independent way to
    manage threads.

    \ingroup thread

    A QThread object manages one thread of control within the
    program. QThreads begin executing in run(). By default, run() starts the
    event loop by calling exec() and runs a Qt event loop inside the thread.

    You can use worker objects by moving them to the thread using
    QObject::moveToThread().

    \snippet doc/src/snippets/code/src_corelib_thread_qthread.cpp worker

    The code inside the Worker's slot would then execute in a
    separate thread. However, you are free to connect the
    Worker's slots to any signal, from any object, in any thread. It
    is safe to connect signals and slots across different threads,
    thanks to a mechanism called \l{Qt::QueuedConnection}{queued
    connections}.

    Another way to make code run in a separate thread, is to subclass QThread
    and reimplement run(). For example:

    \snippet code/src_corelib_thread_qthread.cpp reimpl-run

    In that example, the thread will exit after the run function has returned.
    There will not be any event loop running in the thread unless you call
    exec().

    It is important to remember that a QThread object usually lives
    in the thread where it was created, not in the thread that it
    manages. This oft-overlooked detail means that a QThread's slots
    will be executed in the context of its home thread, not in the
    context of the thread it is managing. For this reason,
    implementing new slots in a QThread subclass is error-prone and
    discouraged.

    \note If you interact with an object, using any technique other
    than queued signal/slot connections (e.g. direct function calls),
    then the usual multithreading precautions need to be taken.

    \note It is not possible to change the thread affinity of GUI
    objects; they must remain in the main thread.

    \section1 Managing threads

    QThread will notifiy you via a signal
    when the thread is started(), finished(), and terminated(), or
    you can use isFinished() and isRunning() to query the state of
    the thread.

    You can stop the thread by calling exit() or quit(). In extreme
    cases, you may want to forcibly terminate() an executing thread.
    However, doing so is dangerous and discouraged. Please read the
    documentation for terminate() and setTerminationEnabled() for
    detailed information.

    From Qt 4.8 onwards, it is possible to deallocate objects that
    live in a thread that has just ended, by connecting the
    finished() signal to QObject::deleteLater().

    Use wait() to block the calling thread, until the other thread
    has finished execution (or until a specified time has passed).

    The static functions currentThreadId() and currentThread() return
    identifiers for the currently executing thread. The former
    returns a platform specific ID for the thread; the latter returns
    a QThread pointer.

    To choose the name that your thread will be given (as identified
    by the command \c{ps -L} on Linux, for example), you can call
    \l{QObject::setObjectName()}{setObjectName()} before starting the thread.
    If you don't call \l{QObject::setObjectName()}{setObjectName()},
    the name given to your thread will be the class name of the runtime
    type of your thread object (for example, \c "RenderThread" in the case of the
    \l{Mandelbrot Example}, as that is the name of the QThread subclass).
    Note that this is currently not available with release builds on Windows.

    QThread also provides static, platform independent sleep
    functions: sleep(), msleep(), and usleep() allow full second,
    millisecond, and microsecond resolution respectively.

    \note wait() and the sleep() functions should be unnecessary in
    general, since Qt is an event-driven framework. Instead of
    wait(), consider listening for the finished() signal. Instead of
    the sleep() functions, consider using QTimer.

    \sa {Thread Support in Qt}, QThreadStorage, QMutex, QSemaphore, QWaitCondition,
        {Mandelbrot Example}, {Semaphores Example}, {Wait Conditions Example}
*/

/*!
    \fn Qt::HANDLE QThread::currentThreadId()

    Returns the thread handle of the currently executing thread.

    \warning The handle returned by this function is used for internal
    purposes and should not be used in any application code.

    \warning On Windows, the returned value is a pseudo-handle for the
    current thread. It can't be used for numerical comparison. i.e.,
    this function returns the DWORD (Windows-Thread ID) returned by
    the Win32 function getCurrentThreadId(), not the HANDLE
    (Windows-Thread HANDLE) returned by the Win32 function
    getCurrentThread().
*/

/*!
    \fn int QThread::idealThreadCount()

    Returns the ideal number of threads that can be run on the system. This is done querying
    the number of processor cores, both real and logical, in the system. This function returns -1
    if the number of processor cores could not be detected.
*/

/*!
    \fn void QThread::yieldCurrentThread()

    Yields execution of the current thread to another runnable thread,
    if any. Note that the operating system decides to which thread to
    switch.
*/

/*!
    \fn void QThread::start(Priority priority)

    Begins execution of the thread by calling run(). The
    operating system will schedule the thread according to the \a
    priority parameter. If the thread is already running, this
    function does nothing.

    The effect of the \a priority parameter is dependent on the
    operating system's scheduling policy. In particular, the \a priority
    will be ignored on systems that do not support thread priorities
    (such as on Linux, see http://linux.die.net/man/2/sched_setscheduler
    for more details).

    \sa run(), terminate()
*/

/*!
    \fn void QThread::started()

    This signal is emitted when the thread starts executing.

    \sa finished(), terminated()
*/

/*!
    \fn void QThread::finished()

    This signal is emitted when the thread has finished executing.

    \sa started(), terminated()
*/

/*!
    \fn void QThread::terminated()

    This signal is emitted when the thread is terminated.

    \sa started(), finished()
*/

/*!
    \enum QThread::Priority

    This enum type indicates how the operating system should schedule
    newly created threads.

    \value IdlePriority scheduled only when no other threads are
           running.

    \value LowestPriority scheduled less often than LowPriority.
    \value LowPriority scheduled less often than NormalPriority.

    \value NormalPriority the default priority of the operating
           system.

    \value HighPriority scheduled more often than NormalPriority.
    \value HighestPriority scheduled more often than HighPriority.

    \value TimeCriticalPriority scheduled as often as possible.

    \value InheritPriority use the same priority as the creating
           thread. This is the default.
*/

/*!
    Returns a pointer to a QThread which manages the currently
    executing thread.
*/
QThread *QThread::currentThread()
{
   QThreadData *data = QThreadData::current();
   Q_ASSERT(data != 0);
   return data->thread;
}

/*!
    Constructs a new QThread to manage a new thread. The \a parent
    takes ownership of the QThread. The thread does not begin
    executing until start() is called.

    \sa start()
*/
QThread::QThread(QObject *parent)
   : QObject(parent), d_ptr(new QThreadPrivate)
{
   d_ptr->q_ptr = this;

   Q_D(QThread);

   // fprintf(stderr, "QThreadData %p created for thread %p\n", d->data, this);
   d->data->thread = this;
}

/*! \internal
 */
QThread::QThread(QThreadPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;

   Q_D(QThread);

   // fprintf(stderr, "QThreadData %p taken from private data for thread %p\n", d->data, this);
   d->data->thread = this;
}

/*!
    Destroys the QThread.

    Note that deleting a QThread object will not stop the execution
    of the thread it manages. Deleting a running QThread (i.e.
    isFinished() returns false) will probably result in a program
    crash. Wait for the finished() signal before deleting the
    QThread.
*/
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
      if (d->running && !d->finished && !d->data->isAdopted) {
         qWarning("QThread: Destroyed while thread is still running");
      }

      d->data->thread = 0;
   }
}

/*!
    Returns true if the thread is finished; otherwise returns false.

    \sa isRunning()
*/
bool QThread::isFinished() const
{
   Q_D(const QThread);
   QMutexLocker locker(&d->mutex);
   return d->finished || d->isInFinish;
}

/*!
    Returns true if the thread is running; otherwise returns false.

    \sa isFinished()
*/
bool QThread::isRunning() const
{
   Q_D(const QThread);
   QMutexLocker locker(&d->mutex);

   return d->running && !d->isInFinish;
}

/*!
    Sets the maximum stack size for the thread to \a stackSize. If \a
    stackSize is greater than zero, the maximum stack size is set to
    \a stackSize bytes, otherwise the maximum stack size is
    automatically determined by the operating system.

    \warning Most operating systems place minimum and maximum limits
    on thread stack sizes. The thread will fail to start if the stack
    size is outside these limits.

    \sa stackSize()
*/
void QThread::setStackSize(uint stackSize)
{
   Q_D(QThread);
   QMutexLocker locker(&d->mutex);
   Q_ASSERT_X(!d->running, "QThread::setStackSize",
              "cannot change stack size while the thread is running");
   d->stackSize = stackSize;
}

/*!
    Returns the maximum stack size for the thread (if set with
    setStackSize()); otherwise returns zero.

    \sa setStackSize()
*/
uint QThread::stackSize() const
{
   Q_D(const QThread);
   QMutexLocker locker(&d->mutex);
   return d->stackSize;
}

/*!
    Enters the event loop and waits until exit() is called, returning the value
    that was passed to exit(). The value returned is 0 if exit() is called via
    quit().

    It is necessary to call this function to start event handling.

    \sa quit(), exit()
*/
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

/*!
    Tells the thread's event loop to exit with a return code.

    After calling this function, the thread leaves the event loop and
    returns from the call to QEventLoop::exec(). The
    QEventLoop::exec() function returns \a returnCode.

    By convention, a \a returnCode of 0 means success, any non-zero value
    indicates an error.

    Note that unlike the C library function of the same name, this
    function \e does return to the caller -- it is event processing
    that stops.

    No QEventLoops will be started anymore in this thread  until
    QThread::exec() has been called again. If the eventloop in QThread::exec()
    is not running then the next call to QThread::exec() will also return
    immediately.

    \sa quit() QEventLoop
*/
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

/*!
    Tells the thread's event loop to exit with return code 0 (success).
    Equivalent to calling QThread::exit(0).

    This function does nothing if the thread does not have an event
    loop.

    \sa exit() QEventLoop
*/
void QThread::quit()
{
   exit();
}

/*!
    The starting point for the thread. After calling start(), the
    newly created thread calls this function. The default
    implementation simply calls exec().

    You can reimplement this function to facilitate advanced thread
    management. Returning from this method will end the execution of
    the thread.

    \sa start() wait()
*/
void QThread::run()
{
   (void) exec();
}

/*! \internal
    Initializes the QThread system.
*/
#if defined (Q_OS_WIN)
void qt_create_tls();
#endif

void QThread::initialize()
{
   if (qt_global_mutexpool) {
      return;
   }
   qt_global_mutexpool = QMutexPool::instance();

#if defined (Q_OS_WIN)
   qt_create_tls();
#endif
}


/*! \internal
    Cleans up the QThread system.
*/
void QThread::cleanup()
{
   qt_global_mutexpool = 0;
}

/*!
    \fn bool QThread::finished() const

    Use isFinished() instead.
*/

/*!
    \fn bool QThread::running() const

    Use isRunning() instead.
*/

/*! \fn void QThread::setPriority(Priority priority)
    \since 4.1

    This function sets the \a priority for a running thread. If the
    thread is not running, this function does nothing and returns
    immediately.  Use start() to start a thread with a specific
    priority.

    The \a priority argument can be any value in the \c
    QThread::Priority enum except for \c InheritPriorty.

    The effect of the \a priority parameter is dependent on the
    operating system's scheduling policy. In particular, the \a priority
    will be ignored on systems that do not support thread priorities
    (such as on Linux, see http://linux.die.net/man/2/sched_setscheduler
    for more details).

    \sa Priority priority() start()
*/

/*!
    \since 4.1

    Returns the priority for a running thread.  If the thread is not
    running, this function returns \c InheritPriority.

    \sa Priority setPriority() start()
*/
QThread::Priority QThread::priority() const
{
   Q_D(const QThread);
   QMutexLocker locker(&d->mutex);

   // mask off the high bits that are used for flags
   return Priority(d->priority & 0xffff);
}

QThreadPrivate *QThreadPrivate::cs_getPrivate(QThread *object)
{
   return object->d_ptr.data();
}


QT_END_NAMESPACE
