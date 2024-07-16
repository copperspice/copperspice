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

#include <qeventdispatcher_unix_p.h>

#include <qcoreapplication.h>
#include <qelapsedtimer.h>
#include <qplatformdefs.h>
#include <qsocketnotifier.h>
#include <qthread.h>


#include <qcoreapplication_p.h>
#include <qcore_unix_p.h>
#include <qthread_p.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_SYS_EVENTFD_H
#  include <sys/eventfd.h>
#endif

#if (_POSIX_MONOTONIC_CLOCK-0 <= 0)
#  include <sys/times.h>
#endif

QEventDispatcherUNIXPrivate::QEventDispatcherUNIXPrivate()
{
   extern Qt::HANDLE qt_application_thread_id;
   mainThread = (QThread::currentThreadId() == qt_application_thread_id);
   bool pipefail = false;

   // initialize the common parts of the event loop

#if defined(Q_OS_NACL)
   // do nothing

#else

#ifdef HAVE_SYS_EVENTFD_H
   thread_pipe[0] = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

   if (thread_pipe[0] != -1) {
      thread_pipe[1] = -1;
   } else
      // continue
#endif

      if (qt_safe_pipe(thread_pipe, O_NONBLOCK) == -1) {
         perror("QEventDispatcherUNIXPrivate(): Unable to create thread pipe");
         pipefail = true;
      }

#endif

   if (pipefail) {
      qFatal("QEventDispatcherUNIXPrivate(): Can not continue without a thread pipe");
   }

   sn_highest = -1;
}

QEventDispatcherUNIXPrivate::~QEventDispatcherUNIXPrivate()
{
#if defined(Q_OS_NACL)
   // do nothing.

#else
   // cleanup the common parts of the event loop
   close(thread_pipe[0]);

   if (thread_pipe[1] != -1) {
      close(thread_pipe[1]);
   }

#endif

   // cleanup timers
   for (auto item : timerList)  {
      delete item;
   }
}

int QEventDispatcherUNIXPrivate::doSelect(QEventLoop::ProcessEventsFlags flags, timespec *timeout)
{
   Q_Q(QEventDispatcherUNIX);

   // needed in QEventDispatcherUNIX::select()
   timerList.updateCurrentTime();

   int nsel;

   do {
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

      int wakeUpFd = initThreadWakeUp();
      highest = qMax(highest, wakeUpFd);

      nsel = q->select(highest + 1, &sn_vec[0].select_fds, &sn_vec[1].select_fds,
         &sn_vec[2].select_fds, timeout);

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
                        ret = select(sn->fd + 1, &fdset, nullptr, nullptr, &tm);
                        break;

                     case 1: // write
                        ret = select(sn->fd + 1, nullptr, &fdset, nullptr, &tm);
                        break;

                     case 2: // except
                        ret = select(sn->fd + 1, nullptr, nullptr, &fdset, &tm);
                        break;
                  }

               } while (ret == -1 && (errno == EINTR || errno == EAGAIN));

               if (ret == -1 && errno == EBADF) {
                  // disable the invalid socket notifier
                  static const char *t[] = { "Read", "Write", "Exception" };
                  qWarning("QEventDispatcher::doSelect() Invalid type %s for socket %d", t[type], sn->fd);
                  sn->obj->setEnabled(false);
               }
            }
         }

      } else {
         // should not happen, wait for this to be reported
         perror("select");
      }
   }

   int nevents = processThreadWakeUp(nsel);

   // activate socket notifiers
   if (! (flags & QEventLoop::ExcludeSocketNotifiers) && nsel > 0 && sn_highest >= 0) {
      // if select says data is ready on any socket, then set the socket notifier to pending
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

// Internal functions for manipulating timer data structures.  The
// timerBitVec array is used for keeping track of timer identifiers.

int QEventDispatcherUNIXPrivate::initThreadWakeUp()
{
   FD_SET(thread_pipe[0], &sn_vec[0].select_fds);
   return thread_pipe[0];
}

int QEventDispatcherUNIXPrivate::processThreadWakeUp(int nsel)
{
   if (nsel > 0 && FD_ISSET(thread_pipe[0], &sn_vec[0].select_fds)) {
      // some other thread woke us up... consume the data on the thread pipe so that
      // select does not immediately return next time

#ifdef HAVE_SYS_EVENTFD_H
      if (thread_pipe[1] == -1) {
         // eventfd
         eventfd_t value;
         eventfd_read(thread_pipe[0], &value);
      } else

#endif
      {
         char c[16];

         while (::read(thread_pipe[0], c, sizeof(c)) > 0) {
         }
      }

      int expected = 1;

      if (! wakeUps.compareExchange(expected, 0, std::memory_order_release)) {
         qWarning("QEventDispatcher::processThreadWakeUp() Internal error");
      }

      return 1;
   }

   return 0;
}

QEventDispatcherUNIX::QEventDispatcherUNIX(QObject *parent)
   : QAbstractEventDispatcher(*new QEventDispatcherUNIXPrivate, parent)
{
}

QEventDispatcherUNIX::QEventDispatcherUNIX(QEventDispatcherUNIXPrivate &dd, QObject *parent)
   : QAbstractEventDispatcher(dd, parent)
{
}

QEventDispatcherUNIX::~QEventDispatcherUNIX()
{
}

int QEventDispatcherUNIX::select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
      timespec *timeout)
{
   return qt_safe_select(nfds, readfds, writefds, exceptfds, timeout);
}

void QEventDispatcherUNIX::registerTimer(int timerId, int interval, Qt::TimerType timerType, QObject *obj)
{
#if defined(CS_SHOW_DEBUG_CORE)
   if (timerId < 1 || interval < 0 || !obj) {
      qDebug("QEventDispatcher::registerTimer() Invalid arguments");
      return;

   } else if (obj->thread() != thread() || thread() != QThread::currentThread()) {
      qDebug("QEventDispatcher::registerTimer() Timers can not be started from another thread");
      return;
   }
#endif

   Q_D(QEventDispatcherUNIX);
   d->timerList.registerTimer(timerId, interval, timerType, obj);
}

bool QEventDispatcherUNIX::unregisterTimer(int timerId)
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

   Q_D(QEventDispatcherUNIX);
   return d->timerList.unregisterTimer(timerId);
}

bool QEventDispatcherUNIX::unregisterTimers(QObject *object)
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

   Q_D(QEventDispatcherUNIX);
   return d->timerList.unregisterTimers(object);
}

QList<QTimerInfo> QEventDispatcherUNIX::registeredTimers(QObject *object) const
{
   if (! object) {
      qWarning("QEventDispatcher:registeredTimers() Invalid argument");
      return QList<QTimerInfo>();
   }

   Q_D(const QEventDispatcherUNIX);

   return d->timerList.registeredTimers(object);
}

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

void QEventDispatcherUNIX::registerSocketNotifier(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);
   int sockfd = notifier->socket();
   int type = notifier->type();

#if defined(CS_SHOW_DEBUG_CORE)
   if (sockfd < 0 || unsigned(sockfd) >= FD_SETSIZE) {
      qDebug("QEventDispatcher::registerSocketNotifier() Internal error");
      return;

   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qDebug("QEventDispatcher::registerSocketNotifier() Socket notifiers can not be enabled from another thread");
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
         qWarning("QSocketNotifier::registerSocketNotifier() Multiple socket notifiers for same socket %d and type %s", sockfd, t[type]);
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

#if defined(CS_SHOW_DEBUG_CORE)
   if (sockfd < 0 || unsigned(sockfd) >= FD_SETSIZE) {
      qDebug("QSocketNotifier::unregisterSocketNotifier() Internal error");
      return;

   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qDebug("QSocketNotifier::unregisterSocketNotifier() Socket notifiers can not be disabled from another thread");
      return;
   }
#endif

   Q_D(QEventDispatcherUNIX);
   QSockNotType::List &list = d->sn_vec[type].list;
   fd_set *fds  =  &d->sn_vec[type].enabled_fds;

   QSockNot *sn = nullptr;
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

#if defined(CS_SHOW_DEBUG_CORE)
   if (sockfd < 0 || unsigned(sockfd) >= FD_SETSIZE) {
      qDebug("QEventDispatcher::setSocketNotifierPending() Internal error");
      return;
   }

   Q_ASSERT(notifier->thread() == thread() && thread() == QThread::currentThread());
#endif

   Q_D(QEventDispatcherUNIX);
   QSockNotType::List &list = d->sn_vec[type].list;
   QSockNot *sn = nullptr;
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

   // We choose a random activation order to be more fair under high load.
   // If a constant order is used and a peer early in the list can
   // saturate the IO, it might grab our attention completely.
   // Also, if we're using a straight list, the callback routines may
   // delete other entries from the list before those other entries are processed.

   if (! FD_ISSET(sn->fd, sn->queue)) {
      if (d->sn_pending_list.isEmpty()) {
         d->sn_pending_list.append(sn);
      } else {
         d->sn_pending_list.insert((qrand() & 0xff) % (d->sn_pending_list.size() + 1), sn);
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

   while (! d->sn_pending_list.isEmpty()) {
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
   d->interrupt.store(0);

   // we are awake, broadcast it
   emit awake();

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);
   QCoreApplicationPrivate::sendPostedEvents(nullptr, 0, threadData);

   int nevents = 0;

   const bool canWait = (threadData->canWaitLocked() && ! d->interrupt.load() && (flags & QEventLoop::WaitForMoreEvents));

   if (canWait) {
      emit aboutToBlock();
   }

   if (! d->interrupt.load()) {
      // return the maximum time we can wait for an event.
      timespec *tm = nullptr;
      timespec wait_tm = { 0l, 0l };

      if (! (flags & QEventLoop::X11ExcludeTimers)) {
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

int QEventDispatcherUNIX::remainingTime(int timerId)
{
#if defined(CS_SHOW_DEBUG_CORE)
   if (timerId < 1) {
      qDebug("QEventDispatcher::remainingTime() Invalid argument");
      return -1;
   }
#endif

   Q_D(QEventDispatcherUNIX);

   return d->timerList.timerRemainingTime(timerId);
}

void QEventDispatcherUNIX::wakeUp()
{
   Q_D(QEventDispatcherUNIX);

   int expected = 0;

   if (d->wakeUps.compareExchange(expected, 1, std::memory_order_acquire)) {

#ifdef HAVE_SYS_EVENTFD_H

      if (d->thread_pipe[1] == -1) {
         // eventfd
         eventfd_t value = 1;
         int ret;
         EINTR_LOOP(ret, eventfd_write(d->thread_pipe[0], value));
         return;
      }

#endif

      char c = 0;
      qt_safe_write( d->thread_pipe[1], &c, 1 );
   }
}

void QEventDispatcherUNIX::interrupt()
{
   Q_D(QEventDispatcherUNIX);
   d->interrupt.store(1);
   wakeUp();
}

void QEventDispatcherUNIX::flush()
{
}
