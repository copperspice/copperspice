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

#include <qplatformdefs.h>
#include <qcoreapplication.h>
#include <qsocketnotifier.h>
#include <qthread.h>
#include <qelapsedtimer.h>
#include <qeventdispatcher_unix_p.h>
#include <qthread_p.h>
#include <qcoreapplication_p.h>
#include <qcore_unix_p.h>
#include <qalgorithms.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#if (_POSIX_MONOTONIC_CLOCK-0 <= 0)
#  include <sys/times.h>
#endif

QT_BEGIN_NAMESPACE

Q_CORE_EXPORT bool qt_disable_lowpriority_timers = false;

/*****************************************************************************
 UNIX signal handling
 *****************************************************************************/

static sig_atomic_t signal_received;
static sig_atomic_t signals_fired[NSIG];

static void signalHandler(int sig)
{
   signals_fired[sig] = 1;
   signal_received = 1;
}

QEventDispatcherUNIXPrivate::QEventDispatcherUNIXPrivate()
{
   extern Qt::HANDLE qt_application_thread_id;
   mainThread = (QThread::currentThreadId() == qt_application_thread_id);
   bool pipefail = false;

   // initialize the common parts of the event loop
#if defined(Q_OS_NACL)
   // do nothing

#else
   if (qt_safe_pipe(thread_pipe, O_NONBLOCK) == -1) {
      perror("QEventDispatcherUNIXPrivate(): Unable to create thread pipe");
      pipefail = true;
   }
#endif

   if (pipefail) {
      qFatal("QEventDispatcherUNIXPrivate(): Can not continue without a thread pipe");
   }

   sn_highest = -1;

   interrupt = false;
}

QEventDispatcherUNIXPrivate::~QEventDispatcherUNIXPrivate()
{
#if defined(Q_OS_NACL)
   // do nothing.

#else
   // cleanup the common parts of the event loop
   close(thread_pipe[0]);
   close(thread_pipe[1]);

#endif

   // cleanup timers
   qDeleteAll(timerList);
}

int QEventDispatcherUNIXPrivate::doSelect(QEventLoop::ProcessEventsFlags flags, timespec *timeout)
{
   Q_Q(QEventDispatcherUNIX);

   // needed in QEventDispatcherUNIX::select()
   timerList.updateCurrentTime();

   int nsel;
   do {
      if (mainThread) {
         while (signal_received) {
            signal_received = 0;
            for (int i = 0; i < NSIG; ++i) {
               if (signals_fired[i]) {
                  signals_fired[i] = 0;
                  emit QCoreApplication::instance()->unixSignal(i);
               }
            }
         }
      }

      // Process timers and socket notifiers - the common UNIX stuff
      int highest = 0;
      if (! (flags & QEventLoop::ExcludeSocketNotifiers) && (sn_highest >= 0)) {
         // return the highest fd we can wait for input on
         sn_vec[0].select_fds = sn_vec[0].enabled_fds;
         sn_vec[1].select_fds = sn_vec[1].enabled_fds;
         sn_vec[2].select_fds = sn_vec[2].enabled_fds;
         highest = sn_highest;
      } else {
         FD_ZERO(&sn_vec[0].select_fds);
         FD_ZERO(&sn_vec[1].select_fds);
         FD_ZERO(&sn_vec[2].select_fds);
      }

      FD_SET(thread_pipe[0], &sn_vec[0].select_fds);
      highest = qMax(highest, thread_pipe[0]);

      nsel = q->select(highest + 1,
                       &sn_vec[0].select_fds,
                       &sn_vec[1].select_fds,
                       &sn_vec[2].select_fds,
                       timeout);
   } while (nsel == -1 && (errno == EINTR || errno == EAGAIN));

   if (nsel == -1) {
      if (errno == EBADF) {
         // it seems a socket notifier has a bad fd... find out
         // which one it is and disable it
         fd_set fdset;
         timeval tm;
         tm.tv_sec = tm.tv_usec = 0l;

         for (int type = 0; type < 3; ++type) {
            QSockNotType::List &list = sn_vec[type].list;
            if (list.size() == 0) {
               continue;
            }

            for (int i = 0; i < list.size(); ++i) {
               QSockNot *sn = list[i];

               FD_ZERO(&fdset);
               FD_SET(sn->fd, &fdset);

               int ret = -1;
               do {
                  switch (type) {
                     case 0: // read
                        ret = select(sn->fd + 1, &fdset, 0, 0, &tm);
                        break;
                     case 1: // write
                        ret = select(sn->fd + 1, 0, &fdset, 0, &tm);
                        break;
                     case 2: // except
                        ret = select(sn->fd + 1, 0, 0, &fdset, &tm);
                        break;
                  }
               } while (ret == -1 && (errno == EINTR || errno == EAGAIN));

               if (ret == -1 && errno == EBADF) {
                  // disable the invalid socket notifier
                  static const char *t[] = { "Read", "Write", "Exception" };
                  qWarning("QSocketNotifier: Invalid socket %d and type '%s', disabling...",
                           sn->fd, t[type]);
                  sn->obj->setEnabled(false);
               }
            }
         }
      } else {
         // EINVAL... shouldn't happen, so let's complain to stderr
         // and hope someone sends us a bug report
         perror("select");
      }
   }

   // some other thread woke us up... consume the data on the thread pipe so that
   // select doesn't immediately return next time
   int nevents = 0;
   if (nsel > 0 && FD_ISSET(thread_pipe[0], &sn_vec[0].select_fds)) {

      char c[16];
      while (::read(thread_pipe[0], c, sizeof(c)) > 0)
         ;

      if (!wakeUps.testAndSetRelease(1, 0)) {
         // hopefully, this is dead code
         qWarning("QEventDispatcherUNIX: internal error, wakeUps.testAndSetRelease(1, 0) failed");
      }
      ++nevents;
   }

   // activate socket notifiers
   if (! (flags & QEventLoop::ExcludeSocketNotifiers) && nsel > 0 && sn_highest >= 0) {
      // if select says data is ready on any socket, then set the socket notifier
      // to pending
      for (int i = 0; i < 3; i++) {
         QSockNotType::List &list = sn_vec[i].list;
         for (int j = 0; j < list.size(); ++j) {
            QSockNot *sn = list[j];
            if (FD_ISSET(sn->fd, &sn_vec[i].select_fds)) {
               q->setSocketNotifierPending(sn->obj);
            }
         }
      }
   }
   return (nevents + q->activateSocketNotifiers());
}

/*
 * Internal functions for manipulating timer data structures.  The
 * timerBitVec array is used for keeping track of timer identifiers.
 */

QTimerInfoList::QTimerInfoList()
{
#if (_POSIX_MONOTONIC_CLOCK-0 <= 0) && !defined(Q_OS_MAC) && !defined(Q_OS_NACL)
   if (! QElapsedTimer::isMonotonic()) {
      // not using monotonic timers, initialize the timeChanged() machinery
      previousTime = qt_gettime();

      tms unused;
      previousTicks = times(&unused);

      ticksPerSecond = sysconf(_SC_CLK_TCK);
      msPerTick = 1000 / ticksPerSecond;
   } else {
      // detected monotonic timers
      previousTime.tv_sec = previousTime.tv_nsec = 0;
      previousTicks = 0;
      ticksPerSecond = 0;
      msPerTick = 0;
   }
#endif

   firstTimerInfo = 0;
}

timespec QTimerInfoList::updateCurrentTime()
{
   return (currentTime = qt_gettime());
}

#if ((_POSIX_MONOTONIC_CLOCK-0 <= 0) && ! defined(Q_OS_MAC))

timespec qAbsTimespec(const timespec &t)
{
    timespec tmp = t;
    if (tmp.tv_sec < 0) {
        tmp.tv_sec = -tmp.tv_sec - 1;
        tmp.tv_nsec -= 1000000000;
    }
    if (tmp.tv_sec == 0 && tmp.tv_nsec < 0) {
        tmp.tv_nsec = -tmp.tv_nsec;
    }
    return normalizedTimespec(tmp);
}

/*
  Returns true if the real time clock has changed by more than 10%
  relative to the processor time since the last time this function was
  called. This presumably means that the system time has been changed.

  If /a delta is nonzero, delta is set to our best guess at how much the system clock was changed.
*/
bool QTimerInfoList::timeChanged(timespec *delta)
{
#ifdef Q_OS_NACL
   Q_UNUSED(delta)
   return false; // Calling "times" crashes.
#endif

   struct tms unused;
   clock_t currentTicks = times(&unused);

   clock_t elapsedTicks = currentTicks - previousTicks;
   timespec elapsedTime = currentTime - previousTime;

   timespec elapsedTimeTicks;
   elapsedTimeTicks.tv_sec  = elapsedTicks / ticksPerSecond;
   elapsedTimeTicks.tv_nsec = (((elapsedTicks * 1000) / ticksPerSecond) % 1000) * 1000 * 1000;

   timespec dummy;
   if (!delta) {
      delta = &dummy;
   }
   *delta = elapsedTime - elapsedTimeTicks;

   previousTicks = currentTicks;
   previousTime  = currentTime;

   // If tick drift is more than 10% off compared to realtime, we assume that the clock has
   // been set. Of course, we have to allow for the tick granularity as well.
   timespec tickGranularity;
   tickGranularity.tv_sec = 0;
   tickGranularity.tv_nsec = msPerTick * 1000 * 1000;

   return elapsedTimeTicks < ((qAbsTimespec(*delta) - tickGranularity) * 10);
}

void QTimerInfoList::repairTimersIfNeeded()
{
   if (QElapsedTimer::isMonotonic()) {
      return;
   }

   timespec delta;
   if (timeChanged(&delta)) {
      timerRepair(delta);
   }
}

#else // !(_POSIX_MONOTONIC_CLOCK-0 <= 0)

void QTimerInfoList::repairTimersIfNeeded()
{
}

#endif

/*
  insert timer info into list
*/
void QTimerInfoList::timerInsert(QTimerInfo *ti)
{
   int index = size();
   while (index--) {
      const QTimerInfo *const t = at(index);
      if (!(ti->timeout < t->timeout)) {
         break;
      }
   }
   insert(index + 1, ti);
}

inline timespec &operator+=(timespec &t1, int ms)
{
    t1.tv_sec += ms / 1000;
    t1.tv_nsec += ms % 1000 * 1000 * 1000;
    return normalizedTimespec(t1);
}

inline timespec operator+(const timespec &t1, int ms)
{
    timespec t2 = t1;
    return t2 += ms;
}

static timespec roundToMillisecond(timespec val)
{
    // always round up
    // worst case scenario is that the first trigger of a 1-ms timer is 0.999 ms late

    int ns = val.tv_nsec % (1000 * 1000);
    val.tv_nsec += 1000 * 1000 - ns;
    return normalizedTimespec(val);
}

/*
  repair broken timer
*/
void QTimerInfoList::timerRepair(const timespec &diff)
{
   // repair all timers
   for (int i = 0; i < size(); ++i) {
      QTimerInfo *t = at(i);
      t->timeout = t->timeout + diff;
   }
}

/*
  Returns the time to wait for the next timer, or null if no timers
  are waiting.
*/
bool QTimerInfoList::timerWait(timespec &tm)
{
   timespec currentTime = updateCurrentTime();
   repairTimersIfNeeded();

   // Find first waiting timer not already active
   QTimerInfo *t = 0;
   for (QTimerInfoList::const_iterator it = constBegin(); it != constEnd(); ++it) {
      if (!(*it)->activateRef) {
         t = *it;
         break;
      }
   }

   if (!t) {
      return false;
   }

   if (currentTime < t->timeout) {
      // time to wait
      tm = roundToMillisecond(t->timeout - currentTime);

   } else {
      // no time to wait
      tm.tv_sec  = 0;
      tm.tv_nsec = 0;
   }

   return true;
}

void QTimerInfoList::registerTimer(int timerId, int interval, QObject *object)
{
   QTimerInfo *t = new QTimerInfo;
   t->id       = timerId;
   t->interval = interval;
   t->timeout  = updateCurrentTime() + t->interval;
   t->obj      = object;
   t->activateRef = 0;

   timerInsert(t);
}

bool QTimerInfoList::unregisterTimer(int timerId)
{
   // set timer inactive
   for (int i = 0; i < count(); ++i) {
      QTimerInfo *t = at(i);

      if (t->id == timerId) {
         // found it
         removeAt(i);

         if (t == firstTimerInfo) {
            firstTimerInfo = 0;
         }

         if (t->activateRef) {
            *(t->activateRef) = 0;
         }

         // release the timer id
         std::atomic<bool> &inThreadChangeEvent = CSInternalEvents::get_m_inThreadChangeEvent(t->obj);

         if (! inThreadChangeEvent.load())  {
            QAbstractEventDispatcherPrivate::releaseTimerId(timerId);
         }

         delete t;
         return true;
      }
   }

   // id not found
   return false;
}

bool QTimerInfoList::unregisterTimers(QObject *object)
{
   if (isEmpty()) {
      return false;
   }
   for (int i = 0; i < count(); ++i) {
      QTimerInfo *t = at(i);

      if (t->obj == object) {
         // object found
         removeAt(i);

         if (t == firstTimerInfo) {
            firstTimerInfo = 0;
         }

         if (t->activateRef) {
            *(t->activateRef) = 0;
         }

         // release the timer id
         std::atomic<bool> &inThreadChangeEvent = CSInternalEvents::get_m_inThreadChangeEvent(t->obj);

         if (! inThreadChangeEvent.load() ) {
            QAbstractEventDispatcherPrivate::releaseTimerId(t->id);
         }

         delete t;
         // move back one so that we don't skip the new current item
         --i;
      }
   }
   return true;
}

QList<std::pair<int, int> > QTimerInfoList::registeredTimers(QObject *object) const
{
   QList<std::pair<int, int> > list;

   for (int i = 0; i < count(); ++i) {
      const QTimerInfo *const t = at(i);
      if (t->obj == object) {
         list << std::pair<int, int>(t->id, t->interval);
      }
   }

   return list;
}

static void calculateNextTimeout(QTimerInfo *t, timespec currentTime)
{
   // Qt::PreciseTimer:
   t->timeout += t->interval;

   if (t->timeout < currentTime) {
      t->timeout = currentTime;
      t->timeout += t->interval;
   }
}

/*
    Activate pending timers, returning how many where activated.
*/
int QTimerInfoList::activateTimers()
{
   if (qt_disable_lowpriority_timers || isEmpty()) {
      return 0;   // nothing to do
   }

   int n_act = 0, maxCount = 0;
   firstTimerInfo = 0;

   timespec currentTime = updateCurrentTime();
   repairTimersIfNeeded();

   // Find out how many timer have expired
   for (QTimerInfoList::const_iterator it = constBegin(); it != constEnd(); ++it) {
      if (currentTime < (*it)->timeout) {
         break;
      }
      maxCount++;
   }

   //fire the timers
   while (maxCount--) {
      if (isEmpty()) {
         break;
      }

      QTimerInfo *currentTimerInfo = first();
      if (currentTime < currentTimerInfo->timeout) {
         break;   // no timer has expired
      }

      if (! firstTimerInfo) {
         firstTimerInfo = currentTimerInfo;

      } else if (firstTimerInfo == currentTimerInfo) {
         // avoid sending the same timer multiple times
         break;

      } else if (currentTimerInfo->interval <  firstTimerInfo->interval
                 || currentTimerInfo->interval == firstTimerInfo->interval) {
         firstTimerInfo = currentTimerInfo;
      }

      // remove from list
      removeFirst();

      // determine next timeout time
      calculateNextTimeout(currentTimerInfo, currentTime);

      // reinsert timer
      timerInsert(currentTimerInfo);
      if (currentTimerInfo->interval > 0) {
         n_act++;
      }

      if (! currentTimerInfo->activateRef) {
         // send event, but don't allow it to recurse
         currentTimerInfo->activateRef = &currentTimerInfo;

         QTimerEvent e(currentTimerInfo->id);
         QCoreApplication::sendEvent(currentTimerInfo->obj, &e);

         if (currentTimerInfo) {
            currentTimerInfo->activateRef = 0;
         }
      }
   }

   firstTimerInfo = 0;
   return n_act;
}

QEventDispatcherUNIX::QEventDispatcherUNIX(QObject *parent)
   : QAbstractEventDispatcher(*new QEventDispatcherUNIXPrivate, parent)
{ }

QEventDispatcherUNIX::QEventDispatcherUNIX(QEventDispatcherUNIXPrivate &dd, QObject *parent)
   : QAbstractEventDispatcher(dd, parent)
{ }

QEventDispatcherUNIX::~QEventDispatcherUNIX()
{
   Q_D(QEventDispatcherUNIX);

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);
   threadData->eventDispatcher = 0;
}

int QEventDispatcherUNIX::select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                  timespec *timeout)
{
   return qt_safe_select(nfds, readfds, writefds, exceptfds, timeout);
}

/*!
    \internal
*/
void QEventDispatcherUNIX::registerTimer(int timerId, int interval, QObject *obj)
{
#ifndef QT_NO_DEBUG
   if (timerId < 1 || interval < 0 || !obj) {
      qWarning("QEventDispatcherUNIX::registerTimer: invalid arguments");
      return;

   } else if (obj->thread() != thread() || thread() != QThread::currentThread()) {
      qWarning("QObject::startTimer: timers cannot be started from another thread");
      return;
   }
#endif

   Q_D(QEventDispatcherUNIX);
   d->timerList.registerTimer(timerId, interval, obj);
}

/*!
    \internal
*/
bool QEventDispatcherUNIX::unregisterTimer(int timerId)
{
#ifndef QT_NO_DEBUG
   if (timerId < 1) {
      qWarning("QEventDispatcherUNIX::unregisterTimer: invalid argument");
      return false;
   } else if (thread() != QThread::currentThread()) {
      qWarning("QObject::killTimer: timers cannot be stopped from another thread");
      return false;
   }
#endif

   Q_D(QEventDispatcherUNIX);
   return d->timerList.unregisterTimer(timerId);
}

/*!
    \internal
*/
bool QEventDispatcherUNIX::unregisterTimers(QObject *object)
{
#ifndef QT_NO_DEBUG
   if (!object) {
      qWarning("QEventDispatcherUNIX::unregisterTimers: invalid argument");
      return false;
   } else if (object->thread() != thread() || thread() != QThread::currentThread()) {
      qWarning("QObject::killTimers: timers cannot be stopped from another thread");
      return false;
   }
#endif

   Q_D(QEventDispatcherUNIX);
   return d->timerList.unregisterTimers(object);
}

QList<QEventDispatcherUNIX::TimerInfo>
QEventDispatcherUNIX::registeredTimers(QObject *object) const
{
   if (!object) {
      qWarning("QEventDispatcherUNIX:registeredTimers: invalid argument");
      return QList<TimerInfo>();
   }

   Q_D(const QEventDispatcherUNIX);
   return d->timerList.registeredTimers(object);
}

/*****************************************************************************
 Socket notifier type
 *****************************************************************************/
QSockNotType::QSockNotType()
{
   FD_ZERO(&select_fds);
   FD_ZERO(&enabled_fds);
   FD_ZERO(&pending_fds);
}

QSockNotType::~QSockNotType()
{
   for (int i = 0; i < list.size(); ++i) {
      delete list[i];
   }
}

/*****************************************************************************
 QEventDispatcher implementations for UNIX
 *****************************************************************************/

void QEventDispatcherUNIX::registerSocketNotifier(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);
   int sockfd = notifier->socket();
   int type = notifier->type();

#ifndef QT_NO_DEBUG
   if (sockfd < 0 || unsigned(sockfd) >= FD_SETSIZE) {
      qWarning("QSocketNotifier: Internal error");
      return;

   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qWarning("QSocketNotifier: socket notifiers cannot be enabled from another thread");
      return;
   }
#endif

   Q_D(QEventDispatcherUNIX);
   QSockNotType::List &list = d->sn_vec[type].list;
   fd_set *fds  = &d->sn_vec[type].enabled_fds;

   QSockNot *sn;

   sn = new QSockNot;
   sn->obj = notifier;
   sn->fd = sockfd;
   sn->queue = &d->sn_vec[type].pending_fds;

   int i;
   for (i = 0; i < list.size(); ++i) {
      QSockNot *p = list[i];
      if (p->fd < sockfd) {
         break;
      }

      if (p->fd == sockfd) {
         static const char *t[] = { "Read", "Write", "Exception" };
         qWarning("QSocketNotifier: Multiple socket notifiers for same socket %d and type %s", sockfd, t[type]);
      }
   }

   list.insert(i, sn);

   FD_SET(sockfd, fds);
   d->sn_highest = qMax(d->sn_highest, sockfd);
}

void QEventDispatcherUNIX::unregisterSocketNotifier(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);
   int sockfd = notifier->socket();
   int type = notifier->type();

#ifndef QT_NO_DEBUG
   if (sockfd < 0 || unsigned(sockfd) >= FD_SETSIZE) {
      qWarning("QSocketNotifier: Internal error");
      return;

   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qWarning("QSocketNotifier: socket notifiers cannot be disabled from another thread");
      return;
   }
#endif

   Q_D(QEventDispatcherUNIX);
   QSockNotType::List &list = d->sn_vec[type].list;
   fd_set *fds  =  &d->sn_vec[type].enabled_fds;

   QSockNot *sn = 0;
   int i;

   for (i = 0; i < list.size(); ++i) {
      sn = list[i];
      if (sn->obj == notifier && sn->fd == sockfd) {
         break;
      }
   }

   if (i == list.size()) {
      // not found
      return;
   }

   FD_CLR(sockfd, fds);                             // clear fd bit
   FD_CLR(sockfd, sn->queue);
   d->sn_pending_list.removeAll(sn);                // remove from activation list
   list.removeAt(i);                                // remove notifier found above
   delete sn;

   if (d->sn_highest == sockfd) {                // find highest fd
      d->sn_highest = -1;

      for (int i = 0; i < 3; i++) {
         if (!d->sn_vec[i].list.isEmpty()) {
            // list is fd-sorted
            d->sn_highest = qMax(d->sn_highest, d->sn_vec[i].list[0]->fd);
         }
      }
   }
}

void QEventDispatcherUNIX::setSocketNotifierPending(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);
   int sockfd = notifier->socket();
   int type = notifier->type();

#ifndef QT_NO_DEBUG
   if (sockfd < 0 || unsigned(sockfd) >= FD_SETSIZE) {
      qWarning("QSocketNotifier: Internal error");
      return;
   }
   Q_ASSERT(notifier->thread() == thread() && thread() == QThread::currentThread());
#endif

   Q_D(QEventDispatcherUNIX);
   QSockNotType::List &list = d->sn_vec[type].list;
   QSockNot *sn = 0;
   int i;
   for (i = 0; i < list.size(); ++i) {
      sn = list[i];
      if (sn->obj == notifier && sn->fd == sockfd) {
         break;
      }
   }
   if (i == list.size()) { // not found
      return;
   }

   // We choose a random activation order to be more fair under high load.
   // If a constant order is used and a peer early in the list can
   // saturate the IO, it might grab our attention completely.
   // Also, if we're using a straight list, the callback routines may
   // delete other entries from the list before those other entries are
   // processed.
   if (! FD_ISSET(sn->fd, sn->queue)) {
      if (d->sn_pending_list.isEmpty()) {
         d->sn_pending_list.append(sn);
      } else {
         d->sn_pending_list.insert((qrand() & 0xff) %
                                   (d->sn_pending_list.size() + 1), sn);
      }
      FD_SET(sn->fd, sn->queue);
   }
}

int QEventDispatcherUNIX::activateTimers()
{
   Q_ASSERT(thread() == QThread::currentThread());
   Q_D(QEventDispatcherUNIX);
   return d->timerList.activateTimers();
}

int QEventDispatcherUNIX::activateSocketNotifiers()
{
   Q_D(QEventDispatcherUNIX);
   if (d->sn_pending_list.isEmpty()) {
      return 0;
   }

   // activate entries
   int n_act = 0;
   QEvent event(QEvent::SockAct);
   while (!d->sn_pending_list.isEmpty()) {
      QSockNot *sn = d->sn_pending_list.takeFirst();
      if (FD_ISSET(sn->fd, sn->queue)) {
         FD_CLR(sn->fd, sn->queue);
         QCoreApplication::sendEvent(sn->obj, &event);
         ++n_act;
      }
   }
   return n_act;
}

bool QEventDispatcherUNIX::processEvents(QEventLoop::ProcessEventsFlags flags)
{
   Q_D(QEventDispatcherUNIX);
   d->interrupt = false;

   // we are awake, broadcast it
   emit awake();

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);
   QCoreApplicationPrivate::sendPostedEvents(0, 0, threadData);

   int nevents = 0;
   const bool canWait = (threadData->canWaitLocked()
                         && !d->interrupt && (flags & QEventLoop::WaitForMoreEvents));

   if (canWait) {
      emit aboutToBlock();
   }

   if (!d->interrupt) {
      // return the maximum time we can wait for an event.
      timespec *tm = 0;
      timespec wait_tm = { 0l, 0l };

      if (!(flags & QEventLoop::X11ExcludeTimers)) {
         if (d->timerList.timerWait(wait_tm)) {
            tm = &wait_tm;
         }
      }

      if (!canWait) {
         if (!tm) {
            tm = &wait_tm;
         }

         // no time to wait
         tm->tv_sec  = 0l;
         tm->tv_nsec = 0l;
      }

      nevents = d->doSelect(flags, tm);

      // activate timers
      if (! (flags & QEventLoop::X11ExcludeTimers)) {
         nevents += activateTimers();
      }
   }
   // return true if we handled events, false otherwise
   return (nevents > 0);
}

bool QEventDispatcherUNIX::hasPendingEvents()
{
   extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
   return qGlobalPostedEventsCount();
}

void QEventDispatcherUNIX::wakeUp()
{
   Q_D(QEventDispatcherUNIX);
   if (d->wakeUps.testAndSetAcquire(0, 1)) {
      char c = 0;
      qt_safe_write( d->thread_pipe[1], &c, 1 );
   }
}

void QEventDispatcherUNIX::interrupt()
{
   Q_D(QEventDispatcherUNIX);
   d->interrupt = true;
   wakeUp();
}

void QEventDispatcherUNIX::flush()
{ }




void QCoreApplication::watchUnixSignal(int sig, bool watch)
{
   if (sig < NSIG) {
      struct sigaction sa;
      sigemptyset(&(sa.sa_mask));
      sa.sa_flags = 0;
      if (watch) {
         sa.sa_handler = signalHandler;
      } else {
         sa.sa_handler = SIG_DFL;
      }
      sigaction(sig, &sa, 0);
   }
}

QT_END_NAMESPACE
