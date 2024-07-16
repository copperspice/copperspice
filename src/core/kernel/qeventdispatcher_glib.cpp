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

#include <qeventdispatcher_glib_p.h>

#include <qcoreapplication.h>
#include <qhash.h>
#include <qlist.h>
#include <qpair.h>
#include <qsocketnotifier.h>

#include <qeventdispatcher_unix_p.h>
#include <qthread_p.h>

#include <glib.h>

struct GPollFDWithQSocketNotifier {
   GPollFD pollfd;
   QSocketNotifier *socketNotifier;
};

struct GSocketNotifierSource {
   GSource source;
   QList<GPollFDWithQSocketNotifier *> pollfds;
};

static gboolean socketNotifierSourcePrepare(GSource *, gint *timeout)
{
   if (timeout) {
      *timeout = -1;
   }

   return false;
}

static gboolean socketNotifierSourceCheck(GSource *source)
{
   GSocketNotifierSource *src = reinterpret_cast<GSocketNotifierSource *>(source);

   bool pending = false;

   for (int i = 0; !pending && i < src->pollfds.count(); ++i) {
      GPollFDWithQSocketNotifier *p = src->pollfds.at(i);

      if (p->pollfd.revents & G_IO_NVAL) {
         // disable the invalid socket notifier
         static const char *t[] = { "Read", "Write", "Exception" };

         qWarning("QSocketNotifier::socketNotifierSourceCheck() Invalid type %s for socket %d",
               t[int(p->socketNotifier->type())], p->pollfd.fd);

         // ### note, modifies src->pollfds!
         p->socketNotifier->setEnabled(false);
      }

      pending = ((p->pollfd.revents & p->pollfd.events) != 0);
   }

   return pending;
}

static gboolean socketNotifierSourceDispatch(GSource *source, GSourceFunc, gpointer)
{
   QEvent event(QEvent::SockAct);

   GSocketNotifierSource *src = reinterpret_cast<GSocketNotifierSource *>(source);

   for (int i = 0; i < src->pollfds.count(); ++i) {
      GPollFDWithQSocketNotifier *p = src->pollfds.at(i);

      if ((p->pollfd.revents & p->pollfd.events) != 0) {
         QCoreApplication::sendEvent(p->socketNotifier, &event);
      }
   }

   return true; // ??? don't remove, right?
}

static GSourceFuncs socketNotifierSourceFuncs = {
   socketNotifierSourcePrepare,
   socketNotifierSourceCheck,
   socketNotifierSourceDispatch,
   NULL,
   NULL,
   NULL
};

struct GTimerSource {
   GSource source;
   QTimerInfoList timerList;
   QEventLoop::ProcessEventsFlags processEventsFlags;
   bool runWithIdlePriority;
};

static gboolean timerSourcePrepareHelper(GTimerSource *src, gint *timeout)
{
   timespec tv = { 0l, 0l };

   if (!(src->processEventsFlags & QEventLoop::X11ExcludeTimers) && src->timerList.timerWait(tv)) {
      *timeout = (tv.tv_sec * 1000) + ((tv.tv_nsec + 999999) / 1000 / 1000);
   } else {
      *timeout = -1;
   }

   return (*timeout == 0);
}

static gboolean timerSourceCheckHelper(GTimerSource *src)
{
   if (src->timerList.isEmpty()
         || (src->processEventsFlags & QEventLoop::X11ExcludeTimers)) {
      return false;
   }

   if (src->timerList.updateCurrentTime() < src->timerList.first()->timeout) {
      return false;
   }

   return true;
}

static gboolean timerSourcePrepare(GSource *source, gint *timeout)
{
   gint dummy;

   if (!timeout) {
      timeout = &dummy;
   }

   GTimerSource *src = reinterpret_cast<GTimerSource *>(source);

   if (src->runWithIdlePriority) {
      if (timeout) {
         *timeout = -1;
      }

      return false;
   }

   return timerSourcePrepareHelper(src, timeout);
}

static gboolean timerSourceCheck(GSource *source)
{
   GTimerSource *src = reinterpret_cast<GTimerSource *>(source);

   if (src->runWithIdlePriority) {
      return false;
   }

   return timerSourceCheckHelper(src);
}

static gboolean timerSourceDispatch(GSource *source, GSourceFunc, gpointer)
{
   GTimerSource *timerSource = reinterpret_cast<GTimerSource *>(source);

   if (timerSource->processEventsFlags & QEventLoop::X11ExcludeTimers) {
      return true;
   }

   timerSource->runWithIdlePriority = true;
   (void) timerSource->timerList.activateTimers();
   return true; // ??? don't remove, right again?
}

static GSourceFuncs timerSourceFuncs = {
   timerSourcePrepare,
   timerSourceCheck,
   timerSourceDispatch,
   NULL,
   NULL,
   NULL
};

struct GIdleTimerSource {
   GSource source;
   GTimerSource *timerSource;
};

static gboolean idleTimerSourcePrepare(GSource *source, gint *timeout)
{
   GIdleTimerSource *idleTimerSource = reinterpret_cast<GIdleTimerSource *>(source);
   GTimerSource *timerSource = idleTimerSource->timerSource;

   if (! timerSource->runWithIdlePriority) {
      // Yield to the normal priority timer source
      if (timeout) {
         *timeout = -1;
      }

      return false;
   }

   return timerSourcePrepareHelper(timerSource, timeout);
}

static gboolean idleTimerSourceCheck(GSource *source)
{
   GIdleTimerSource *idleTimerSource = reinterpret_cast<GIdleTimerSource *>(source);
   GTimerSource *timerSource = idleTimerSource->timerSource;

   if (!timerSource->runWithIdlePriority) {
      // Yield to the normal priority timer source
      return false;
   }

   return timerSourceCheckHelper(timerSource);
}

static gboolean idleTimerSourceDispatch(GSource *source, GSourceFunc, gpointer)
{
   GTimerSource *timerSource = reinterpret_cast<GIdleTimerSource *>(source)->timerSource;
   (void) timerSourceDispatch(&timerSource->source, 0, 0);
   return true;
}

static GSourceFuncs idleTimerSourceFuncs = {
   idleTimerSourcePrepare,
   idleTimerSourceCheck,
   idleTimerSourceDispatch,
   NULL,
   NULL,
   NULL
};

struct GPostEventSource {
   GSource source;
   QAtomicInt serialNumber;
   int lastSerialNumber;
   QEventDispatcherGlibPrivate *d;
};

static gboolean postEventSourcePrepare(GSource *s, gint *timeout)
{
   QThreadData *data = QThreadData::current();

   if (!data) {
      return false;
   }

   gint dummy;

   if (! timeout) {
      timeout = &dummy;
   }

   const bool canWait = data->canWaitLocked();
   *timeout = canWait ? -1 : 0;

   GPostEventSource *source = reinterpret_cast<GPostEventSource *>(s);
   return (! canWait || (source->serialNumber.load() != source->lastSerialNumber));
}

static gboolean postEventSourceCheck(GSource *source)
{
   return postEventSourcePrepare(source, 0);
}

static gboolean postEventSourceDispatch(GSource *s, GSourceFunc, gpointer)
{
   GPostEventSource *source = reinterpret_cast<GPostEventSource *>(s);
   source->lastSerialNumber = source->serialNumber.load();
   QCoreApplication::sendPostedEvents();
   source->d->runTimersOnceWithNormalPriority();
   return true;
}

static GSourceFuncs postEventSourceFuncs = {
   postEventSourcePrepare,
   postEventSourceCheck,
   postEventSourceDispatch,
   NULL,
   NULL,
   NULL
};

QEventDispatcherGlibPrivate::QEventDispatcherGlibPrivate(GMainContext *context)
   : mainContext(context)
{
#if GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION < 32

   if (qgetenv("QT_NO_THREADED_GLIB").isEmpty()) {

      static QMutex mutex;
      QMutexLocker locker(&mutex);

      if (! g_thread_supported()) {
         g_thread_init(NULL);
      }
   }

#endif

   if (mainContext) {
      g_main_context_ref(mainContext);

   } else {
      QCoreApplication *app = QCoreApplication::instance();

      if (app && QThread::currentThread() == app->thread()) {
         mainContext = g_main_context_default();
         g_main_context_ref(mainContext);
      } else {
         mainContext = g_main_context_new();
      }
   }

#if GLIB_CHECK_VERSION (2, 22, 0)
   g_main_context_push_thread_default (mainContext);
#endif

   // setup post event source
   postEventSource = reinterpret_cast<GPostEventSource *>(g_source_new(&postEventSourceFuncs, sizeof(GPostEventSource)));
   postEventSource->serialNumber.store(1);
   postEventSource->d = this;
   g_source_set_can_recurse(&postEventSource->source, true);
   g_source_attach(&postEventSource->source, mainContext);

   // setup socketNotifierSource
   socketNotifierSource = reinterpret_cast<GSocketNotifierSource *>(g_source_new(&socketNotifierSourceFuncs,
         sizeof(GSocketNotifierSource)));

   (void) new (&socketNotifierSource->pollfds) QList<GPollFDWithQSocketNotifier *>();
   g_source_set_can_recurse(&socketNotifierSource->source, true);
   g_source_attach(&socketNotifierSource->source, mainContext);

   // setup normal and idle timer sources
   timerSource = reinterpret_cast<GTimerSource *>(g_source_new(&timerSourceFuncs, sizeof(GTimerSource)));
   (void) new (&timerSource->timerList) QTimerInfoList();
   timerSource->processEventsFlags = QEventLoop::AllEvents;
   timerSource->runWithIdlePriority = false;
   g_source_set_can_recurse(&timerSource->source, true);
   g_source_attach(&timerSource->source, mainContext);

   idleTimerSource = reinterpret_cast<GIdleTimerSource *>(g_source_new(&idleTimerSourceFuncs, sizeof(GIdleTimerSource)));
   idleTimerSource->timerSource = timerSource;
   g_source_set_can_recurse(&idleTimerSource->source, true);
   g_source_set_priority(&idleTimerSource->source, G_PRIORITY_DEFAULT_IDLE);
   g_source_attach(&idleTimerSource->source, mainContext);
}

void QEventDispatcherGlibPrivate::runTimersOnceWithNormalPriority()
{
   timerSource->runWithIdlePriority = false;
}

QEventDispatcherGlib::QEventDispatcherGlib(QObject *parent)
   : QAbstractEventDispatcher(*(new QEventDispatcherGlibPrivate), parent)
{
}

QEventDispatcherGlib::QEventDispatcherGlib(GMainContext *mainContext, QObject *parent)
   : QAbstractEventDispatcher(*(new QEventDispatcherGlibPrivate(mainContext)), parent)
{ }

QEventDispatcherGlib::~QEventDispatcherGlib()
{
   Q_D(QEventDispatcherGlib);

   // destroy all timer sources
   for (auto item : d->timerSource->timerList)  {
      delete item;
   }

   d->timerSource->timerList.~QTimerInfoList();
   g_source_destroy(&d->timerSource->source);
   g_source_unref(&d->timerSource->source);
   d->timerSource = 0;
   g_source_destroy(&d->idleTimerSource->source);
   g_source_unref(&d->idleTimerSource->source);
   d->idleTimerSource = 0;

   // destroy socket notifier source
   for (int i = 0; i < d->socketNotifierSource->pollfds.count(); ++i) {
      GPollFDWithQSocketNotifier *p = d->socketNotifierSource->pollfds[i];
      g_source_remove_poll(&d->socketNotifierSource->source, &p->pollfd);
      delete p;
   }

   d->socketNotifierSource->pollfds.~QList<GPollFDWithQSocketNotifier *>();
   g_source_destroy(&d->socketNotifierSource->source);
   g_source_unref(&d->socketNotifierSource->source);
   d->socketNotifierSource = 0;

   // destroy post event source
   g_source_destroy(&d->postEventSource->source);
   g_source_unref(&d->postEventSource->source);
   d->postEventSource = 0;

   Q_ASSERT(d->mainContext != 0);

#if GLIB_CHECK_VERSION (2, 22, 0)
   g_main_context_pop_thread_default (d->mainContext);
#endif

   g_main_context_unref(d->mainContext);
   d->mainContext = 0;
}

bool QEventDispatcherGlib::processEvents(QEventLoop::ProcessEventsFlags flags)
{
   Q_D(QEventDispatcherGlib);

   const bool canWait = (flags & QEventLoop::WaitForMoreEvents);

   if (canWait) {
      emit aboutToBlock();
   } else {
      emit awake();
   }

   // tell postEventSourcePrepare() and timerSource about any new flags
   QEventLoop::ProcessEventsFlags savedFlags = d->timerSource->processEventsFlags;
   d->timerSource->processEventsFlags = flags;

   if (!(flags & QEventLoop::EventLoopExec)) {
      // force timers to be sent at normal priority
      d->timerSource->runWithIdlePriority = false;
   }

   bool result = g_main_context_iteration(d->mainContext, canWait);

   while (!result && canWait) {
      result = g_main_context_iteration(d->mainContext, canWait);
   }

   d->timerSource->processEventsFlags = savedFlags;

   if (canWait) {
      emit awake();
   }

   return result;
}

bool QEventDispatcherGlib::hasPendingEvents()
{
   Q_D(QEventDispatcherGlib);
   return g_main_context_pending(d->mainContext);
}

void QEventDispatcherGlib::registerSocketNotifier(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);
   int sockfd = notifier->socket();
   int type = notifier->type();

#if defined(CS_SHOW_DEBUG_CORE)
   if (sockfd < 0) {
      qDebug("QEventDispatcher::registerSocketNotifier() Internal error");
      return;

   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qDebug("QEventDispatcher::registerSocketNotifier() Socket notifiers can not be enabled from another thread");
      return;
   }
#endif

   Q_D(QEventDispatcherGlib);

   GPollFDWithQSocketNotifier *p = new GPollFDWithQSocketNotifier;
   p->pollfd.fd = sockfd;

   switch (type) {
      case QSocketNotifier::Read:
         p->pollfd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
         break;

      case QSocketNotifier::Write:
         p->pollfd.events = G_IO_OUT | G_IO_ERR;
         break;

      case QSocketNotifier::Exception:
         p->pollfd.events = G_IO_PRI | G_IO_ERR;
         break;
   }

   p->socketNotifier = notifier;

   d->socketNotifierSource->pollfds.append(p);

   g_source_add_poll(&d->socketNotifierSource->source, &p->pollfd);
}

void QEventDispatcherGlib::unregisterSocketNotifier(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);

#if defined(CS_SHOW_DEBUG_CORE)
   int sockfd = notifier->socket();

   if (sockfd < 0) {
      qDebug("QEventDispatcher::unregisterSocketNotifier() Internal error");
      return;

   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qDebug("QEventDispatcher::unregisterSocketNotifier() Socket notifiers can not be disabled from another thread");
      return;
   }
#endif

   Q_D(QEventDispatcherGlib);

   for (int i = 0; i < d->socketNotifierSource->pollfds.count(); ++i) {
      GPollFDWithQSocketNotifier *p = d->socketNotifierSource->pollfds.at(i);

      if (p->socketNotifier == notifier) {
         // found it
         g_source_remove_poll(&d->socketNotifierSource->source, &p->pollfd);

         d->socketNotifierSource->pollfds.removeAt(i);
         delete p;

         return;
      }
   }
}

void QEventDispatcherGlib::registerTimer(int timerId, int interval, Qt::TimerType timerType, QObject *object)
{
#if defined(CS_SHOW_DEBUG_CORE)
   if (timerId < 1 || interval < 0 || ! object) {
      qDebug("QEventDispatcher::registerTimer() Invalid arguments");
      return;

   } else if (object->thread() != thread() || thread() != QThread::currentThread()) {
      qDebug("QEventDispatcher::registerTimer() Timers can not be started from another thread");
      return;
   }
#endif

   Q_D(QEventDispatcherGlib);
   d->timerSource->timerList.registerTimer(timerId, interval, timerType, object);
}

bool QEventDispatcherGlib::unregisterTimer(int timerId)
{
#if defined(CS_SHOW_DEBUG_CORE)
   if (timerId < 1) {
      qDebug("QEventDispatcher::unregisterTimer() Invalid argument");
      return false;

   } else if (thread() != QThread::currentThread()) {
      qDebug("QEventDispatcher::unregisterTimer() Timers can not be stopped from another thread");
      return false;
   }
#endif

   Q_D(QEventDispatcherGlib);
   return d->timerSource->timerList.unregisterTimer(timerId);
}

bool QEventDispatcherGlib::unregisterTimers(QObject *object)
{
#if defined(CS_SHOW_DEBUG_CORE)
   if (! object) {
      qDebug("QEventDispatcher::unregisterTimers() Invalid argument");
      return false;

   } else if (object->thread() != thread() || thread() != QThread::currentThread()) {
      qDebug("QEventDispatcher::unregisterTimers() Timers can not be stopped from another thread");
      return false;
   }
#endif

   Q_D(QEventDispatcherGlib);

   return d->timerSource->timerList.unregisterTimers(object);
}

QList<QTimerInfo> QEventDispatcherGlib::registeredTimers(QObject *object) const
{
   if (! object) {
      qWarning("QEventDispatcher:registeredTimers() Invalid argument");
      return QList<QTimerInfo>();
   }

   Q_D(const QEventDispatcherGlib);

   return d->timerSource->timerList.registeredTimers(object);
}

int QEventDispatcherGlib::remainingTime(int timerId)
{
#if defined(CS_SHOW_DEBUG_CORE)
   if (timerId < 1) {
      qDebug("QEventDispatcher::remainingTime() Invalid argument");
      return -1;
   }
#endif

   Q_D(QEventDispatcherGlib);
   return d->timerSource->timerList.timerRemainingTime(timerId);
}

void QEventDispatcherGlib::interrupt()
{
   wakeUp();
}

void QEventDispatcherGlib::wakeUp()
{
   Q_D(QEventDispatcherGlib);

   d->postEventSource->serialNumber.ref();
   g_main_context_wakeup(d->mainContext);
}

void QEventDispatcherGlib::flush()
{
}

bool QEventDispatcherGlib::versionSupported()
{
#if ! defined(GLIB_MAJOR_VERSION) || ! defined(GLIB_MINOR_VERSION) || ! defined(GLIB_MICRO_VERSION)
   return false;
#else
   return ((GLIB_MAJOR_VERSION << 16) + (GLIB_MINOR_VERSION << 8) + GLIB_MICRO_VERSION) >= 0x020301;
#endif
}

QEventDispatcherGlib::QEventDispatcherGlib(QEventDispatcherGlibPrivate &dd, QObject *parent)
   : QAbstractEventDispatcher(dd, parent)
{
}
