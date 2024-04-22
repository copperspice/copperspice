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

#include <qcoreapplication.h>
#include <qcoreapplication_p.h>

#include <qabstracteventdispatcher.h>
#include <qcoreevent.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qdir.h>
#include <qelapsedtimer.h>
#include <qeventloop.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qglobal.h>
#include <qhash.h>
#include <qlibraryinfo.h>
#include <qmutex.h>
#include <qstandardpaths.h>
#include <qtextcodec.h>
#include <qthread.h>
#include <qthreadpool.h>
#include <qthreadstorage.h>
#include <qvarlengtharray.h>
#include <qcorecmdlineargs_p.h>
#include <qfactoryloader_p.h>
#include <qlocale_p.h>
#include <qprocess_p.h>
#include <qprocess_p.h>
#include <qthread_p.h>

#if defined(Q_OS_UNIX)
#  if defined(Q_OS_DARWIN)
#    include <qeventdispatcher_cf_p.h>
#    include <qeventdispatcher_unix_p.h>
#  else
#    if ! defined(QT_NO_GLIB)
#    include <qeventdispatcher_glib_p.h>
#    endif

#include <qeventdispatcher_unix_p.h>
#  endif
#endif

#ifdef Q_OS_WIN
#include <qeventdispatcher_win_p.h>

#endif

#ifdef Q_OS_DARWIN
#include <qcore_mac_p.h>
#endif

#ifdef Q_OS_UNIX
#include <locale.h>
#include <unistd.h>
#include <sys/types.h>
#endif

#include <algorithm>
#include <stdlib.h>

class QMutexUnlocker
{
 public:
   explicit QMutexUnlocker(QMutex *m)
      : mtx(m)
   {
   }

   QMutexUnlocker(const QMutexUnlocker &) = delete;
   QMutexUnlocker &operator=(const QMutexUnlocker &) = delete;

   ~QMutexUnlocker()
   {
      unlock();
   }

   void unlock() {
      if (mtx) {
         mtx->unlock();
      }

      mtx = nullptr;
   }

 private:
   QMutex *mtx;
};

#if defined(Q_OS_WIN) || defined(Q_OS_DARWIN)
extern QString qAppFileName();
#endif

bool QCoreApplicationPrivate::setuidAllowed = false;
#if ! defined(Q_OS_WIN)

#ifdef Q_OS_DARWIN
QString QCoreApplicationPrivate::macMenuBarName()
{
   QString bundleName;
   CFTypeRef string = CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(), CFSTR("CFBundleName"));

   if (string) {
      bundleName = QCFString::toQString(static_cast<CFStringRef>(string));
   }

   return bundleName;
}
#endif

QString QCoreApplicationPrivate::appName() const
{
   QString applicationName;

#ifdef Q_OS_DARWIN
   applicationName = macMenuBarName();
#endif

   if (applicationName.isEmpty() && argv[0]) {
      char *p = strrchr(argv[0], '/');
      applicationName = QString::fromUtf8(p ? p + 1 : argv[0]);
   }

   return applicationName;
}
#endif

bool QCoreApplicationPrivate::checkInstance(const char *function)
{
   bool b = (QCoreApplication::m_self != nullptr);

   if (! b) {
      qWarning("QCoreApplication::%s() QApplication must be started before calling this method", function);
   }

   return b;
}

static QString *qmljs_debug_arguments()
{
   static QString retval;
   return &retval;
}

void QCoreApplicationPrivate::processCommandLineArguments()
{
   int j = argc ? 1 : 0;

   for (int i = 1; i < argc; ++i) {

      if (! argv[i]) {
         continue;
      }

      if (*argv[i] != '-') {
         argv[j++] = argv[i];
         continue;
      }

      QString arg = QString::fromUtf8(argv[i]);

      if (arg.startsWith("-qmljsdebugger=")) {

         *qmljs_debug_arguments() = arg.right(arg.length() - 15);

      } else if (arg == "-qmljsdebugger" && i < argc - 1) {
         ++i;
         *qmljs_debug_arguments() = QString::fromUtf8(argv[i]);

      } else {
         argv[j++] = argv[i];
      }
   }

   if (j < argc) {
      argv[j] = nullptr;
      argc    = j;
   }
}

extern "C" void Q_CORE_EXPORT qt_startup_hook()
{
}

using QStartUpFuncList = QList<FP_Void>;
using QVFuncList       = QList<FP_Void>;

static QStartUpFuncList *preRList()
{
   static QStartUpFuncList retval;
   return &retval;
}

static QVFuncList *postRList()
{
   static QVFuncList retval;
   return &retval;
}

static QMutex globalPreRoutinesMutex;

void qAddPreRoutine(FP_Void p)
{
   QStartUpFuncList *list = preRList();

   if (! list) {
      return;
   }

   // this can be called from multiple threads
   QMutexLocker locker(&globalPreRoutinesMutex);

   if (QCoreApplication::instance()) {
      p();
   }

   list->prepend(p); // in case QCoreApplication is re-initialized, refer to qt_call_pre_routines
}

void qAddPostRoutine(FP_Void p)
{
   QVFuncList *list = postRList();

   if (! list) {
      return;
   }

   list->prepend(p);
}

void qRemovePostRoutine(FP_Void p)
{
   QVFuncList *list = postRList();

   if (! list) {
      return;
   }

   list->removeAll(p);
}

static void qt_call_pre_routines()
{
   QStartUpFuncList *list = preRList();

   if (! list) {
      return;
   }

   QMutexLocker locker(&globalPreRoutinesMutex);

   // Unlike qt_call_post_routines, we do not empty the list, because Q_COREAPP_STARTUP_FUNCTION is a macro,
   // so the user expects the function to be executed every time QCoreApplication is created.

   for (int i = 0; i < list->count(); ++i) {
      list->at(i)();
   }
}

void Q_CORE_EXPORT qt_call_post_routines()
{
   QVFuncList *list = nullptr;

   try {
      list = postRList();

   } catch (const std::bad_alloc &) {
      // ignore - if we can not allocate a post routine list,
      // there's a high probability that there's no post routine to be executed
   }

   if (!list) {
      return;
   }

   while (! list->isEmpty()) {
      (list->takeFirst())();
   }
}

// app starting up if false
bool QCoreApplicationPrivate::is_app_running = false;

// app closing down if true
bool QCoreApplicationPrivate::is_app_closing = false;

// initialized in qcoreapplication and in qtextstream autotest when setlocale is called.
Q_CORE_EXPORT bool qt_locale_initialized = false;

//  Create an instance of cs.conf. Ensures settings will not be thrown out of QSetting's cache for unused settings
static QSettings *internal_csConf()
{
   static QSettings retval(QSettings::UserScope, "CopperSpice");
   return &retval;
}

QSettings *QCoreApplicationPrivate::copperspiceConf()
{
   return internal_csConf();
}

Q_CORE_EXPORT uint qGlobalPostedEventsCount()
{
   QThreadData *currentThreadData = QThreadData::current();
   return currentThreadData->postEventList.size() - currentThreadData->postEventList.startOffset;
}

QCoreApplication *QCoreApplication::m_self = nullptr;
QAbstractEventDispatcher *QCoreApplicationPrivate::eventDispatcher = nullptr;
uint QCoreApplicationPrivate::attribs;

#ifdef Q_OS_UNIX
Qt::HANDLE qt_application_thread_id = nullptr;
#endif

struct QCoreApplicationData {
   QCoreApplicationData() {
      app_libpaths = nullptr;
   }

   ~QCoreApplicationData() {
      delete app_libpaths;

      // cleanup the QAdoptedThread created for the main() thread
      if (QCoreApplicationPrivate::theMainThread) {
         QThreadData *data = QThreadData::get2(QCoreApplicationPrivate::theMainThread);
         data->deref(); // deletes the data and the adopted thread
      }

   }
   QString orgName;
   QString orgDomain;
   QString application;
   QString applicationVersion;

   bool applicationNameSet;      // true if setApplicationName was called

   QStringList *app_libpaths;
};

static QCoreApplicationData *coreappdata()
{
   static QCoreApplicationData retval;
   return &retval;
}

static bool quitLockRefEnabled = true;

QCoreApplicationPrivate::QCoreApplicationPrivate(int &aargc, char **aargv, uint flags)
   : argc(aargc), argv(aargv), application_type(QCoreApplicationPrivate::Tty),
     in_exec(false), aboutToQuitEmitted(false)
{
   (void) flags;

   static const char *const empty = "";

   if (argc == 0 || argv == nullptr) {
      argc = 0;
      argv = const_cast<char **>(&empty);
   }

   QCoreApplicationPrivate::is_app_closing = false;

#ifdef Q_OS_UNIX
   qt_application_thread_id = QThread::currentThreadId();
#endif

   // this call to QThread::currentThread() may end up setting theMainThread
   if (QThread::currentThread() != theMainThread) {
      qWarning("QCoreApplication() QApplication must be created in the main() thread");
   }
}

QCoreApplicationPrivate::~QCoreApplicationPrivate()
{
   Q_Q(QCoreApplication);

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(q);

   if (threadData) {
      void *data = &threadData->tls;
      QThreadStorageData::finish((void **)data);

      // need to clear the state of the mainData, just in case a new QCoreApplication comes along.
      QMutexLocker locker(&threadData->postEventList.mutex);

      for (int i = 0; i < threadData->postEventList.size(); ++i) {
         const QPostEvent &pe = threadData->postEventList.at(i);

         if (pe.event) {
            CSInternalEvents::decr_PostedEvents(pe.receiver);

            pe.event->posted = false;
            delete pe.event;
         }
      }

      threadData->postEventList.clear();
      threadData->postEventList.recursion = 0;
      threadData->quitNow = false;
   }
}

void QCoreApplicationPrivate::createEventDispatcher()
{
   Q_Q(QCoreApplication);

#if defined(Q_OS_UNIX)

#if defined(Q_OS_DARWIN)
   bool ok   = false;
   int value = qgetenv("QT_EVENT_DISPATCHER_CORE_FOUNDATION").toInt(&ok);

   if (ok && value > 0) {
      eventDispatcher = new QEventDispatcherCoreFoundation(q);
   } else {
      eventDispatcher = new QEventDispatcherUNIX(q);
   }

#elif ! defined(QT_NO_GLIB)

   if (qgetenv("QT_NO_GLIB").isEmpty() && QEventDispatcherGlib::versionSupported()) {
      eventDispatcher = new QEventDispatcherGlib(q);
   } else {
      eventDispatcher = new QEventDispatcherUNIX(q);
   }

#else
   eventDispatcher = new QEventDispatcherUNIX(q);

#endif

#elif defined(Q_OS_WIN)
   eventDispatcher = new QEventDispatcherWin32(q);

#else
#error "QEventDispatcher has not been ported to this platform."

#endif
}

void QCoreApplicationPrivate::eventDispatcherReady()
{
}

QThread *QCoreApplicationPrivate::theMainThread = nullptr;

QThread *QCoreApplicationPrivate::mainThread()
{
   Q_ASSERT(theMainThread != nullptr);
   return theMainThread;
}

void QCoreApplicationPrivate::checkReceiverThread(QObject *receiver)
{
   QThread *currentThread = QThread::currentThread();
   QThread *thr = receiver->thread();

   Q_ASSERT_X(currentThread == thr || ! thr, "QCoreApplication::sendEvent",
         QString::fromLatin1("Unable to send events to objects owned by a different thread. "
         "Current thread %1. Receiver '%2' (of type '%3') was created in thread %4")
         .formatArg(QString::number((quintptr) currentThread, 16))
         .formatArg(receiver->objectName())
         .formatArg(receiver->metaObject()->className())
         .formatArg(QString::number((quintptr) thr, 16))
         .toUtf8().constData());
}

void QCoreApplicationPrivate::appendApplicationPathToLibraryPaths()
{
#if ! defined(QT_NO_SETTINGS)
   QStringList *app_libpaths = coreappdata()->app_libpaths;
   Q_ASSERT(app_libpaths);

   QString app_location( QCoreApplication::applicationFilePath() );
   app_location.truncate(app_location.lastIndexOf('/'));
   app_location = QDir(app_location).canonicalPath();

   if (QFile::exists(app_location) && !app_libpaths->contains(app_location)) {
      app_libpaths->append(app_location);
   }

#endif
}

QString QCoreApplicationPrivate::qmljsDebugArguments()
{
   return *qmljs_debug_arguments();
}

QString qAppName()
{
   if (! QCoreApplicationPrivate::checkInstance("qAppName")) {
      return QString();
   }

   return QCoreApplication::instance()->d_func()->appName();
}

void QCoreApplicationPrivate::initLocale()
{
   if (qt_locale_initialized) {
      return;
   }

   qt_locale_initialized = true;

#ifdef Q_OS_UNIX
   setlocale(LC_ALL, "");
#endif
}

// internal
QCoreApplication::QCoreApplication(QCoreApplicationPrivate &p)
   : QObject(nullptr), d_ptr(&p)
{
   d_ptr->q_ptr = this;

   // subclasses should call QCoreApplicationPrivate::eventDispatcher->startingUp();
}

void QCoreApplication::flush()
{
   if (m_self && m_self->d_func()->eventDispatcher) {
      m_self->d_func()->eventDispatcher->flush();
   }
}

QCoreApplication::QCoreApplication(int &argc, char **argv, int _internal)
   : QObject(nullptr), d_ptr(new QCoreApplicationPrivate(argc, argv, _internal) )
{
   d_ptr->q_ptr = this;
   d_ptr->init();
   QCoreApplicationPrivate::eventDispatcher->startingUp();
}

void QCoreApplicationPrivate::init()
{
   Q_Q(QCoreApplication);

   initLocale();

   Q_ASSERT_X(! QCoreApplication::m_self, "QCoreApplication", "There should be only one QApplication running");
   QCoreApplication::m_self = q;

   // Store app name (so it is available after QCoreApplication is destroyed)
   if (! coreappdata()->applicationNameSet) {
      coreappdata()->application = appName();
   }

   // emerald - may want to adjust the library path
#if ! defined(QT_NO_SETTINGS)

   if (! coreappdata()->app_libpaths) {
      // make sure library paths is initialized
      q->libraryPaths();

   } else {
      appendApplicationPathToLibraryPaths();
   }

#endif

   // threads
   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(q);

   // use the event dispatcher created by the app programmer (if any)
   if (! eventDispatcher) {
      eventDispatcher = threadData->eventDispatcher;
   }

   // otherwise we create one
   if (eventDispatcher == nullptr) {
      createEventDispatcher();
   }

   Q_ASSERT(eventDispatcher != nullptr);

   if (! eventDispatcher->parent()) {
      eventDispatcher->moveToThread(threadData->thread);
      eventDispatcher->setParent(q);
   }

   threadData->eventDispatcher = eventDispatcher;
   eventDispatcherReady();

   processCommandLineArguments();

   qt_call_pre_routines();
   qt_startup_hook();

   is_app_running = true;       // No longer starting up
}

QCoreApplication::~QCoreApplication()
{
   qt_call_post_routines();

   m_self = nullptr;
   QCoreApplicationPrivate::is_app_closing = true;
   QCoreApplicationPrivate::is_app_running = false;

   // Synchronize and stop the global thread pool threads.
   QThreadPool *globalThreadPool = nullptr;

   try {
      globalThreadPool = QThreadPool::globalInstance();

   } catch (...) {
      // consume the exception since destructors should not throw
   }

   if (globalThreadPool) {
      globalThreadPool->waitForDone();
   }

   // threads
   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   threadData->eventDispatcher = nullptr;

   if (QCoreApplicationPrivate::eventDispatcher) {
      QCoreApplicationPrivate::eventDispatcher->closingDown();
   }

   QCoreApplicationPrivate::eventDispatcher = nullptr;

   delete coreappdata()->app_libpaths;
   coreappdata()->app_libpaths = nullptr;
}

void QCoreApplication::setSetuidAllowed(bool allow)
{
   QCoreApplicationPrivate::setuidAllowed = allow;
}

bool QCoreApplication::isSetuidAllowed()
{
   return QCoreApplicationPrivate::setuidAllowed;
}

void QCoreApplication::setAttribute(Qt::ApplicationAttribute attribute, bool on)
{
   if (on) {
      QCoreApplicationPrivate::attribs |= 1 << attribute;
   } else {
      QCoreApplicationPrivate::attribs &= ~(1 << attribute);
   }
}

bool QCoreApplication::testAttribute(Qt::ApplicationAttribute attribute)
{
   return QCoreApplicationPrivate::testAttribute(attribute);
}

bool QCoreApplication::isQuitLockEnabled()
{
   return quitLockRefEnabled;
}

void QCoreApplication::setQuitLockEnabled(bool enabled)
{
   quitLockRefEnabled = enabled;
}

bool QCoreApplication::notifyInternal(QObject *receiver, QEvent *event)
{
   // enforces the rule that events can only be sent to objects in
   // the current thread, so receiver->d_func()->threadData is
   // equivalent to QThreadData::current(), just without the function call overhead.

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(receiver);

   ++threadData->loopLevel;

   bool returnValue;

   try {
      returnValue = notify(receiver, event);

   } catch (...) {
      --threadData->loopLevel;
      throw;
   }

   --threadData->loopLevel;
   return returnValue;
}

bool QCoreApplication::notify(QObject *receiver, QEvent *event)
{
   Q_D(QCoreApplication);

   // no events are delivered after ~QCoreApplication() has started
   if (QCoreApplicationPrivate::is_app_closing) {
      return true;
   }

   if (receiver == nullptr) {                        // serious error
      qWarning("QCoreApplication::notify() Receiver invalid (nullptr)");
      return true;
   }

#if defined(CS_SHOW_DEBUG_CORE)
   d->checkReceiverThread(receiver);
#endif

   if (receiver->isWidgetType()) {
      return false;

   } else {
      return d->notify_helper(receiver, event);

   }
}

bool QCoreApplicationPrivate::sendThroughApplicationEventFilters(QObject *receiver, QEvent *event)
{
   Q_Q(QCoreApplication);

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(q);
   QThreadData *threadData_Receiver = CSInternalThreadData::get_m_ThreadData(receiver);

   QList<QPointer<QObject>> &eventFilters = CSInternalEvents::get_m_EventFilters(q);

   if (threadData_Receiver == threadData) {

      // application event filters are only called for objects in the GUI thread
      for (int i = 0; i < eventFilters.size(); ++i) {
         QObject *obj = eventFilters.at(i);

         if (!obj) {
            continue;
         }

         QThreadData *threadData_Obj = CSInternalThreadData::get_m_ThreadData(obj);

         if (threadData_Obj != threadData) {
            qWarning("QCoreApplication::sendThroughApplicationEventFilters() Application event filter must be in the same thread");
            continue;
         }

         if (obj->eventFilter(receiver, event)) {
            return true;
         }
      }
   }

   return false;
}

bool QCoreApplicationPrivate::sendThroughObjectEventFilters(QObject *receiver, QEvent *event)
{
   Q_Q(QCoreApplication);

   if (receiver != q) {
      QThreadData *threadData_Receiver = CSInternalThreadData::get_m_ThreadData(receiver);

      QList<QPointer<QObject>> &eventFilters = CSInternalEvents::get_m_EventFilters(receiver);

      for (int i = 0; i < eventFilters.size(); ++i) {
         QObject *obj = eventFilters.at(i);

         if (! obj) {
            continue;
         }

         QThreadData *threadData_Obj = CSInternalThreadData::get_m_ThreadData(obj);

         if (threadData_Obj != threadData_Receiver) {
            qWarning("QCoreApplication::sendThroughObjectEventFilters() Object event filter must be in the same thread");
            continue;
         }

         if (obj->eventFilter(receiver, event)) {
            return true;
         }
      }
   }

   return false;
}

// internal methods called by notify()
bool QCoreApplicationPrivate::notify_helper(QObject *receiver, QEvent *event)
{
   // send to all application event filters
   if (sendThroughApplicationEventFilters(receiver, event)) {
      return true;
   }

   // send to all receiver event filters
   if (sendThroughObjectEventFilters(receiver, event)) {
      return true;
   }

   // deliver the event
   return receiver->event(event);
}

bool QCoreApplication::startingUp()
{
   return ! QCoreApplicationPrivate::is_app_running;
}

bool QCoreApplication::closingDown()
{
   return QCoreApplicationPrivate::is_app_closing;
}

void QCoreApplication::processEvents(QEventLoop::ProcessEventsFlags flags)
{
   QThreadData *data = QThreadData::current();
   auto tmp = data->eventDispatcher.load();

   if (! tmp) {
      return;
   }

   if (flags & QEventLoop::DeferredDeletion) {
      QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
   }

   tmp->processEvents(flags);
}

void QCoreApplication::processEvents(QEventLoop::ProcessEventsFlags flags, int maxtime)
{
   QThreadData *data = QThreadData::current();

   if (! data->hasEventDispatcher()) {
      return;
   }

   QElapsedTimer start;
   start.start();

   if (flags & QEventLoop::DeferredDeletion) {
      QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
   }

   while (data->eventDispatcher.load()->processEvents(flags & ~QEventLoop::WaitForMoreEvents)) {
      if (start.elapsed() > maxtime) {
         break;
      }

      if (flags & QEventLoop::DeferredDeletion) {
         QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
      }
   }
}

int QCoreApplication::exec()
{
   if (! QCoreApplicationPrivate::checkInstance("exec")) {
      return -1;
   }

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(m_self);

   if (threadData != QThreadData::current()) {
      qWarning("%s::exec() Method must be called from the main() thread",
            csPrintable(m_self->metaObject()->className()));
      return -1;
   }

   if (! threadData->eventLoops.isEmpty()) {
      qWarning("QCoreApplication::exec() Event loop is already running");
      return -1;
   }

   threadData->quitNow = false;

   QEventLoop eventLoop;
   m_self->d_func()->in_exec            = true;
   m_self->d_func()->aboutToQuitEmitted = false;

   int returnCode = eventLoop.exec();
   threadData->quitNow = false;

   if (m_self) {
      m_self->d_func()->in_exec = false;

      if (! m_self->d_func()->aboutToQuitEmitted) {
         emit m_self->aboutToQuit();
      }

      m_self->d_func()->aboutToQuitEmitted = true;
      sendPostedEvents(nullptr, QEvent::DeferredDelete);
   }

   return returnCode;
}

void QCoreApplication::exit(int returnCode)
{
   if (! m_self) {
      return;
   }

   QThreadData *data = CSInternalThreadData::get_m_ThreadData(m_self);
   data->quitNow = true;

   for (int i = 0; i < data->eventLoops.size(); ++i) {
      QEventLoop *eventLoop = data->eventLoops.at(i);
      eventLoop->exit(returnCode);
   }
}

void QCoreApplication::postEvent(QObject *receiver, QEvent *event, int priority)
{
   if (receiver == nullptr) {
      qWarning("QCoreApplication::postEvent() Receiver invalid (nullptr)");
      delete event;
      return;
   }

   QThreadData *data = CSInternalThreadData::get_m_ThreadData(receiver);

   if (! data) {
      // posting during destruction, delete the event to prevent a leak
      delete event;
      return;
   }

   std::atomic<QThreadData *> &pdata = CSInternalThreadData::get_AtomicThreadData(receiver);

   // lock the post event mutex
   data->postEventList.mutex.lock();

   // if object has moved to another thread, follow it
   while (data != pdata.load()) {
      data->postEventList.mutex.unlock();
      data = pdata.load();

      if (! data) {
         // posting during destruction, delete the event to prevent a leak
         delete event;
         return;
      }

      data->postEventList.mutex.lock();
   }

   QMutexUnlocker locker(&data->postEventList.mutex);

   int peCount = CSInternalEvents::get_m_PostedEvents(receiver);

   // if this is one of the compressible events, do compression
   if (peCount != 0 && m_self && m_self->compressEvent(event, receiver, &data->postEventList)) {
      return;
   }

   if (event->type() == QEvent::DeferredDelete && data == QThreadData::current()) {
      // remember the current running eventloop for DeferredDelete
      // events posted in the receiver's thread
      static_cast<QDeferredDeleteEvent *>(event)->level = data->loopLevel;
   }

   // delete the event on exceptions to protect against memory leaks till the event is
   // properly owned in the postEventList
   QScopedPointer<QEvent> eventDeleter(event);
   data->postEventList.addEvent(QPostEvent(receiver, event, priority));
   eventDeleter.take();
   event->posted = true;

   CSInternalEvents::incr_PostedEvents(receiver);

   data->canWait = false;
   locker.unlock();

   auto tmp = data->eventDispatcher.load();

   if (tmp) {
      tmp->wakeUp();
   }
}

/*!
  \internal
  Returns true if event was compressed away (possibly deleted) and should not be added to the list.
*/
bool QCoreApplication::compressEvent(QEvent *event, QObject *receiver, QPostEventList *postedEvents)
{
   int peCount = CSInternalEvents::get_m_PostedEvents(receiver);

#ifdef Q_OS_WIN
   Q_ASSERT(event);
   Q_ASSERT(receiver);
   Q_ASSERT(postedEvents);

   // compress posted timers to this object

   if (event->type() == QEvent::Timer && peCount > 0) {

      int timerId = ((QTimerEvent *) event)->timerId();

      for (int i = 0; i < postedEvents->size(); ++i) {
         const QPostEvent &e = postedEvents->at(i);

         if (e.receiver == receiver && e.event && e.event->type() == QEvent::Timer
               && ((QTimerEvent *) e.event)->timerId() == timerId) {
            delete event;
            return true;
         }
      }
   } else

#endif
      if ((event->type() == QEvent::DeferredDelete || event->type() == QEvent::Quit) && peCount > 0) {

         for (int i = 0; i < postedEvents->size(); ++i) {
            const QPostEvent &cur = postedEvents->at(i);

            if (cur.receiver != receiver || cur.event == nullptr || cur.event->type() != event->type()) {
               continue;
            }

            // found an event for this receiver
            delete event;
            return true;
         }
      }

   return false;
}

void QCoreApplication::sendPostedEvents(QObject *receiver, int event_type)
{
   QThreadData *data = QThreadData::current();

   QCoreApplicationPrivate::sendPostedEvents(receiver, event_type, data);
}

void QCoreApplicationPrivate::sendPostedEvents(QObject *receiver, int event_type, QThreadData *data)
{
   if (event_type == -1) {
      // we were called by an obsolete event dispatcher.
      event_type = 0;
   }

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(receiver);

   if (receiver && threadData != data) {
      qWarning("QCoreApplication::sendPostedEvents() Unable to send posted events to objects in another thread");
      return;
   }

   ++data->postEventList.recursion;

   QMutexLocker locker(&data->postEventList.mutex);

   // by default, we assume that the event dispatcher can go to sleep after
   // processing all events. if any new events are posted while we send
   // events, canWait will be set to false.
   data->canWait = (data->postEventList.size() == 0);

   int peCount = CSInternalEvents::get_m_PostedEvents(receiver);

   if (data->postEventList.size() == 0 || (receiver && peCount == 0)) {
      --data->postEventList.recursion;
      return;
   }

   data->canWait = true;

   // okay. here is the tricky loop. be careful about optimizing
   // this, it looks the way it does for good reasons.
   int startOffset = data->postEventList.startOffset;
   int &i = (!event_type && !receiver) ? data->postEventList.startOffset : startOffset;
   data->postEventList.insertionOffset = data->postEventList.size();

   while (i < data->postEventList.size()) {
      // avoid live-lock
      if (i >= data->postEventList.insertionOffset) {
         break;
      }

      const QPostEvent &pe = data->postEventList.at(i);
      ++i;

      if (!pe.event) {
         continue;
      }

      if ((receiver && receiver != pe.receiver) || (event_type && event_type != pe.event->type())) {
         data->canWait = false;
         continue;
      }

      if (pe.event->type() == QEvent::DeferredDelete) {
         // DeferredDelete events are only sent when we are explicitly asked to
         // (s.a. QEvent::DeferredDelete), and then only if the event loop that
         // posted the event has returned.

         const bool tmp1 = quintptr(pe.event->d) > unsigned(data->loopLevel);
         const bool tmp2 = pe.event->d == nullptr && (data->loopLevel > 0);
         const bool tmp3 = (event_type == QEvent::DeferredDelete) && (quintptr(pe.event->d) == unsigned(data->loopLevel));

         const bool allowDeferredDelete = tmp1 || tmp2 || tmp3;

         if (! allowDeferredDelete) {
            // can not send deferred delete

            if (! event_type && ! receiver) {
               // copy the event
               QPostEvent pe_copy = pe;

               // null out the event so if sendPostedEvents recurses, it
               // will ignore this one, as it's been re-posted
               const_cast<QPostEvent &>(pe).event = nullptr;

               // re-post the copied event so it is not lost
               data->postEventList.addEvent(pe_copy);
            }

            continue;
         }
      }

      // first, adjust the event so that we can deliver it
      // so no one will try to touch it later
      pe.event->posted = false;

      QEvent *e  = pe.event;
      QObject *r = pe.receiver;

      CSInternalEvents::decr_PostedEvents(r);
      Q_ASSERT(CSInternalEvents::get_m_PostedEvents(r) >= 0);

      // next, update the data structure so that we're ready for the next event
      const_cast<QPostEvent &>(pe).event = nullptr;

      locker.unlock();
      // time to deliver the event

      try {
         QCoreApplication::sendEvent(r, e);

      } catch (...) {
         delete e;
         locker.relock();

         // since we were interrupted, we need another pass to make sure we clean everything up
         data->canWait = false;

         // ugly hack, copied from below
         --data->postEventList.recursion;

         auto tmp = data->eventDispatcher.load();

         if (! data->postEventList.recursion && ! data->canWait && tmp) {
            tmp->wakeUp();
         }

         throw;              // rethrow
      }

      delete e;
      locker.relock();

      // careful when adding anything below this point - the
      // sendEvent() call might invalidate any invariants this
      // function depends on.
   }

   --data->postEventList.recursion;
   auto tmp = data->eventDispatcher.load();

   if (!data->postEventList.recursion && !data->canWait && tmp) {
      tmp->wakeUp();
   }

   // clear the global list, i.e. remove everything that was delivered
   if (! event_type && !receiver && data->postEventList.startOffset >= 0) {
      const QPostEventList::iterator it = data->postEventList.begin();
      data->postEventList.erase(it, it + data->postEventList.startOffset);
      data->postEventList.insertionOffset -= data->postEventList.startOffset;

      Q_ASSERT(data->postEventList.insertionOffset >= 0);

      data->postEventList.startOffset = 0;
   }
}

void QCoreApplication::removePostedEvents(QObject *receiver, int eventType)
{
   QThreadData *data;

   if (receiver) {
      data = CSInternalThreadData::get_m_ThreadData(receiver);

   } else {
      data = QThreadData::current();

   }

   QMutexLocker locker(&data->postEventList.mutex);

   // the QObject destructor calls this function directly.  this can
   // happen while the event loop is in the middle of posting events,
   // and when we get here, we may not have any more posted events for this object
   int peCount = CSInternalEvents::get_m_PostedEvents(receiver);

   if (receiver && peCount == 0) {
      return;
   }

   // collect all the posted events for the QObject
   // and we will delete after the mutex was unlocked
   QVarLengthArray<QEvent *> events;

   int n = data->postEventList.size();
   int j = 0;

   for (int i = 0; i < n; ++i) {
      const QPostEvent &pe = data->postEventList.at(i);

      if ((! receiver || pe.receiver == receiver) && (pe.event && (eventType == 0 || pe.event->type() == eventType))) {

         CSInternalEvents::decr_PostedEvents(pe.receiver);

         pe.event->posted = false;
         events.append(pe.event);
         const_cast<QPostEvent &>(pe).event = nullptr;

      } else if (! data->postEventList.recursion) {
         if (i != j) {
            qSwap(data->postEventList[i], data->postEventList[j]);
         }

         ++j;
      }
   }

#if defined(CS_SHOW_DEBUG_CORE)
   if (receiver && eventType == 0) {
      Q_ASSERT(CSInternalEvents::get_m_PostedEvents(receiver) == 0);
   }
#endif

   if (! data->postEventList.recursion) {
      // truncate list
      data->postEventList.erase(data->postEventList.begin() + j, data->postEventList.end());
   }

   locker.unlock();

   for (int i = 0; i < events.count(); ++i) {
      delete events[i];
   }
}

void QCoreApplicationPrivate::removePostedEvent(QEvent *event)
{
   if (! event || ! event->posted) {
      return;
   }

   QThreadData *data = QThreadData::current();

   QMutexLocker locker(&data->postEventList.mutex);

   if (data->postEventList.size() == 0) {

#if defined(CS_SHOW_DEBUG_CORE)
      qDebug("QCoreApplication::removePostedEvent() Internal error, %p %d is posted", (void *)event, event->type());
      return;
#endif

   }

   for (int i = 0; i < data->postEventList.size(); ++i) {
      const QPostEvent &pe = data->postEventList.at(i);

      if (pe.event == event) {
         qWarning("QCoreApplication::removePostedEvent() Event of type %d deleted while posted to %s %s",
               event->type(), csPrintable(pe.receiver->metaObject()->className()), csPrintable(pe.receiver->objectName()));

         CSInternalEvents::decr_PostedEvents(pe.receiver);
         pe.event->posted = false;

         delete pe.event;
         const_cast<QPostEvent &>(pe).event = nullptr;

         return;
      }
   }
}

bool QCoreApplication::event(QEvent *e)
{
   if (e->type() == QEvent::Quit) {
      quit();
      return true;
   }

   return QObject::event(e);
}

void QCoreApplication::cs_internal_maybeQuit()
{
   Q_D(QCoreApplication);
   d->maybeQuit();
}

bool QCoreApplication::cs_isRealGuiApp()
{

   Q_D(QCoreApplication);
   return d->application_type == QCoreApplicationPrivate::Type::Gui;
}

void QCoreApplicationPrivate::maybeQuit()
{
   if (quitLockRef.load() == 0 && in_exec && quitLockRefEnabled && shouldQuit()) {
      QCoreApplication::postEvent(QCoreApplication::instance(), new QEvent(QEvent::Quit));
   }
}

void QCoreApplication::quit()
{
   exit(0);
}

void QCoreApplication::installTranslator(QTranslator *translationFile)
{
   if (! translationFile) {
      return;
   }

   if (! QCoreApplicationPrivate::checkInstance("installTranslator")) {
      return;
   }

   QCoreApplicationPrivate *d = m_self->d_func();
   d->translators.prepend(translationFile);

   if (translationFile->isEmpty()) {
      return;
   }

   QEvent ev(QEvent::LanguageChange);
   QCoreApplication::sendEvent(m_self, &ev);
}

void QCoreApplication::removeTranslator(QTranslator *translationFile)
{
   if (! translationFile) {
      return;
   }

   if (! QCoreApplicationPrivate::checkInstance("removeTranslator")) {
      return;
   }

   QCoreApplicationPrivate *d = m_self->d_func();

   if (d->translators.removeAll(translationFile) && ! m_self->closingDown()) {
      QEvent ev(QEvent::LanguageChange);
      QCoreApplication::sendEvent(m_self, &ev);
   }
}

QString QCoreApplication::translate(const char *context, const char *text, const char *comment,
      std::optional<int> numArg)
{
   QString retval;

   if (! text) {
      return retval;
   }

   if (m_self != nullptr) {

      for (auto item : m_self->d_func()->translators) {
         retval = item->translate(context, text, comment, numArg);

         if (! retval.isEmpty()) {
            break;
         }
      }
   }

   if (retval.isEmpty()) {
      retval = QString::fromUtf8(text);

      if (numArg.has_value()) {
         retval = QTranslator::replacePercentN(retval, numArg.value());
      }
   }

   return retval;
}

QString QCoreApplication::translate(const QString &context, const QString &text, const QString &comment,
      std::optional<int> numArg)
{
   QString retval;

   if (text.isEmpty()) {
      return retval;
   }

   if (m_self != nullptr) {

      for (auto item : m_self->d_func()->translators) {
         retval = item->translate(context, text, comment, numArg);

         if (! retval.isEmpty()) {
            break;
         }
      }
   }

   if (retval.isEmpty()) {
      retval = text;

      if (numArg.has_value()) {
         retval = QTranslator::replacePercentN(retval, numArg.value());
      }
   }

   return retval;
}

// Declared in qglobal.h
QString qtTrId(const char *id, std::optional<int> n)
{
   return QCoreApplication::translate(nullptr, id, nullptr, n);
}

bool QCoreApplicationPrivate::isTranslatorInstalled(QTranslator *translator)
{
   return QCoreApplication::m_self && QCoreApplication::m_self->d_func()->translators.contains(translator);
}

QString QCoreApplication::applicationDirPath()
{
   if (! m_self) {
      qWarning("QCoreApplication::applicationDirPath() QApplication must be started before calling this method");
      return QString();
   }

   QCoreApplicationPrivate *d = m_self->d_func();

   if (d->cachedApplicationDirPath.isEmpty()) {
      d->cachedApplicationDirPath = QFileInfo(applicationFilePath()).path();
   }

   return d->cachedApplicationDirPath;
}

QString QCoreApplication::applicationFilePath()
{
   if (! m_self) {
      qWarning("QCoreApplication::applicationFilePath() QApplication must be started before calling this method");
      return QString();
   }

   QCoreApplicationPrivate *d = m_self->d_func();

   if (! d->cachedApplicationFilePath.isEmpty()) {
      return d->cachedApplicationFilePath;
   }

#if defined(Q_OS_WIN)
   d->cachedApplicationFilePath = QFileInfo(qAppFileName()).filePath();
   return d->cachedApplicationFilePath;

#elif defined(Q_OS_DARWIN)
   QString qAppFileName_str = qAppFileName();

   if (! qAppFileName_str.isEmpty()) {
      QFileInfo fi(qAppFileName_str);
      d->cachedApplicationFilePath = fi.exists() ? fi.canonicalFilePath() : QString();
      return d->cachedApplicationFilePath;
   }

#endif

#if defined( Q_OS_UNIX )

#ifdef Q_OS_LINUX
   // Try looking for a /proc/<pid>/exe symlink first which points to the absolute path of the executable
   QFileInfo pfi(QString::fromLatin1("/proc/%1/exe").formatArg(getpid()));

   if (pfi.exists() && pfi.isSymLink()) {
      d->cachedApplicationFilePath = pfi.canonicalFilePath();
      return d->cachedApplicationFilePath;
   }

#endif

   QString argv0 = arguments().at(0);

   QString absPath;

   if (! argv0.isEmpty() && argv0.at(0) == '/') {
      // If argv0 starts with a slash, it is already an absolute file path.
      absPath = argv0;

   } else if (argv0.contains('/')) {
      // If argv0 contains one or more slashes, it is a file path relative to the current directory.
      absPath = QDir::current().absoluteFilePath(argv0);

   } else {
      // Otherwise, the file path has to be determined using the PATH environment variable.
      QByteArray pEnv   = qgetenv("PATH");
      QDir currentDir   = QDir::current();
      QStringList paths = QString::fromUtf8(pEnv.constData()).split(QChar(':'));

      for (QStringList::const_iterator p = paths.constBegin(); p != paths.constEnd(); ++p) {
         if ((*p).isEmpty()) {
            continue;
         }

         QString candidate = currentDir.absoluteFilePath(*p + QChar('/') + argv0);
         QFileInfo candidate_fi(candidate);

         if (candidate_fi.exists() && !candidate_fi.isDir()) {
            absPath = candidate;
            break;
         }
      }
   }

   absPath = QDir::cleanPath(absPath);

   QFileInfo fi(absPath);
   d->cachedApplicationFilePath = fi.exists() ? fi.canonicalFilePath() : QString();
   return d->cachedApplicationFilePath;
#endif
}

qint64 QCoreApplication::applicationPid()
{
#if defined(Q_OS_WIN)
   return GetCurrentProcessId();
#else
   return getpid();
#endif
}

QStringList QCoreApplication::arguments()
{
   QStringList list;

   if (! m_self) {
      qWarning("QCoreApplication::arguments() QApplication must be started before calling this method");
      return list;
   }

#ifdef Q_OS_WIN
   list = qCmdLineArgs(0, nullptr);

   if (m_self->d_func()->application_type) {
      // GUI app? Skip known, refer to qapplication.cpp
      QStringList stripped;

      for (int a = 0; a < list.count(); ++a) {
         QString arg      = list.at(a);
         QByteArray l1arg = arg.toLatin1();

         if (l1arg == "-qdevel" || l1arg == "-qdebug" || l1arg == "-reverse" ||
               l1arg == "-stylesheet" || l1arg == "-widgetcount") {

            // no code should appear here

         } else if (l1arg.startsWith("-style=") || l1arg.startsWith("-qmljsdebugger=")) {

            // no code should appear here

         } else if (l1arg == "-style" || l1arg == "-qmljsdebugger" || l1arg == "-session" ||
               l1arg == "-graphicssystem" || l1arg == "-testability") {
            ++a;

         } else {
            stripped += arg;

         }
      }

      list = stripped;
   }

#else
   const int ac = m_self->d_func()->argc;
   char **const av = m_self->d_func()->argv;

   for (int index = 0; index < ac; ++index) {
      list << QString::fromUtf8(av[index]);
   }

#endif

   return list;
}

void QCoreApplication::setOrganizationName(const QString &orgName)
{
   coreappdata()->orgName = orgName;
}

QString QCoreApplication::organizationName()
{
   return coreappdata()->orgName;
}

void QCoreApplication::setOrganizationDomain(const QString &orgDomain)
{
   coreappdata()->orgDomain = orgDomain;
}

QString QCoreApplication::organizationDomain()
{
   return coreappdata()->orgDomain;
}

void QCoreApplication::setApplicationName(const QString &application)
{
   coreappdata()->application = application;
}

QString QCoreApplication::applicationName()
{
   return coreappdata()->application;
}

// Exported for QDesktopServices (backward compatibility)
Q_CORE_EXPORT QString qt_applicationName_noFallback()
{
   return coreappdata()->applicationNameSet ? coreappdata()->application : QString();
}

void QCoreApplication::setApplicationVersion(const QString &version)
{
   coreappdata()->applicationVersion = version;
}

QString QCoreApplication::applicationVersion()
{
   return coreappdata()->applicationVersion;
}

static QRecursiveMutex *libraryPathMutex()
{
   static QRecursiveMutex retval;
   return &retval;
}

QStringList QCoreApplication::libraryPaths()
{
   QRecursiveMutexLocker locker(libraryPathMutex());

   if (! coreappdata()->app_libpaths) {
      QStringList *app_libpaths   = new QStringList;
      coreappdata()->app_libpaths = app_libpaths;

      // retrives the plugins path from cs.conf
      QString installPathPlugins = QLibraryInfo::location(QLibraryInfo::PluginsPath);

      if (QFile::exists(installPathPlugins)) {
         // make sure we convert from backslashes to slashes
         installPathPlugins = QDir(installPathPlugins).canonicalPath();

         if (! app_libpaths->contains(installPathPlugins)) {
            app_libpaths->append(installPathPlugins);
         }
      }

      // if QCoreApplication is not yet instantiated, make sure we add the
      // application path when we construct the QCoreApplication
      if (m_self) {
         m_self->d_func()->appendApplicationPathToLibraryPaths();
      }

      const QByteArray libPathEnv = qgetenv("CS_PLUGIN_PATH");

      if (! libPathEnv.isEmpty()) {

#if defined(Q_OS_WIN)
         QChar pathSep(';');
#else
         QChar pathSep(':');
#endif
         QStringList paths = QString::fromLatin1(libPathEnv).split(pathSep, QStringParser::SkipEmptyParts);

         for (QStringList::const_iterator it = paths.constBegin(); it != paths.constEnd(); ++it) {
            QString canonicalPath = QDir(*it).canonicalPath();

            if (! canonicalPath.isEmpty() && !app_libpaths->contains(canonicalPath)) {
               app_libpaths->append(canonicalPath);
            }
         }
      }
   }

   return *(coreappdata()->app_libpaths);
}

void QCoreApplication::setLibraryPaths(const QStringList &paths)
{
   QRecursiveMutexLocker locker(libraryPathMutex());

   if (! coreappdata()->app_libpaths) {
      coreappdata()->app_libpaths = new QStringList;
   }

   *(coreappdata()->app_libpaths) = paths;
   locker.unlock();

   QFactoryLoader::refreshAll();
}

void QCoreApplication::addLibraryPath(const QString &path)
{
   if (path.isEmpty()) {
      return;
   }

   QRecursiveMutexLocker locker(libraryPathMutex());

   // make sure that library paths is initialized
   libraryPaths();

   QString canonicalPath = QDir(path).canonicalPath();

   if (! canonicalPath.isEmpty() && !coreappdata()->app_libpaths->contains(canonicalPath)) {
      coreappdata()->app_libpaths->prepend(canonicalPath);
      locker.unlock();

      QFactoryLoader::refreshAll();
   }
}

void QCoreApplication::removeLibraryPath(const QString &path)
{
   if (path.isEmpty()) {
      return;
   }

   QRecursiveMutexLocker locker(libraryPathMutex());

   // make sure that library paths is initialized
   libraryPaths();

   QString canonicalPath = QDir(path).canonicalPath();
   coreappdata()->app_libpaths->removeAll(canonicalPath);
   QFactoryLoader::refreshAll();
}

void QCoreApplication::installNativeEventFilter(QAbstractNativeEventFilter *filterObj)
{
   if (QCoreApplication::testAttribute(Qt::AA_MacPluginApplication)) {
      qWarning("QCoreApplication::installNativeEventFilter() Native event filters are not applied when "
            "Qt::AA_MacPluginApplication is set");

      return;
   }

   QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance(QCoreApplicationPrivate::theMainThread);

   if (! filterObj || ! eventDispatcher) {
      return;
   }

   eventDispatcher->installNativeEventFilter(filterObj);
}

void QCoreApplication::removeNativeEventFilter(QAbstractNativeEventFilter *filterObject)
{
   QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance();

   if (! filterObject || ! eventDispatcher) {
      return;
   }

   eventDispatcher->removeNativeEventFilter(filterObject);
}

QAbstractEventDispatcher *QCoreApplication::eventDispatcher()
{
   if (QCoreApplicationPrivate::theMainThread) {
      return QCoreApplicationPrivate::theMainThread->eventDispatcher();
   }

   return nullptr;
}

void QCoreApplication::setEventDispatcher(QAbstractEventDispatcher *eventDispatcher)
{
   QThread *mainThread = QCoreApplicationPrivate::theMainThread;

   if (! mainThread) {
      mainThread = QThread::currentThread(); // will also setup theMainThread
   }

   mainThread->setEventDispatcher(eventDispatcher);
}
