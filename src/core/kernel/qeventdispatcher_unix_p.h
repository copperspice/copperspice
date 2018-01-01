/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QEVENTDISPATCHER_UNIX_P_H
#define QEVENTDISPATCHER_UNIX_P_H

#include <QtCore/qabstracteventdispatcher.h>
#include <QtCore/qlist.h>
#include <qabstracteventdispatcher_p.h>
#include <qcore_unix_p.h>
#include <qpodlist_p.h>
#include <QtCore/qvarlengtharray.h>

#include <sys/time.h>
#if (!defined(Q_OS_HPUX) || defined(__ia64)) && !defined(Q_OS_NACL)
#  include <sys/select.h>
#endif


QT_BEGIN_NAMESPACE

// internal timer info
struct QTimerInfo {
   int id;              // - timer identifier
   int interval;        // - timer interval in milliseconds
   timespec timeout;    // - when to sent event
   QObject *obj;        // - object to receive event
   QTimerInfo **activateRef; // - ref from activateTimers
};

class QTimerInfoList : public QList<QTimerInfo *>
{
#if ((_POSIX_MONOTONIC_CLOCK-0 <= 0) && ! defined(Q_OS_MAC))
   timespec previousTime;
   clock_t previousTicks;
   int ticksPerSecond;
   int msPerTick;

   bool timeChanged(timespec *delta);
#endif

   // state variables used by activateTimers()
   QTimerInfo *firstTimerInfo;

 public:
   QTimerInfoList();

   timespec currentTime;
   timespec updateCurrentTime();

   // must call updateCurrentTime() first!
   void repairTimersIfNeeded();

   bool timerWait(timespec &);
   void timerInsert(QTimerInfo *);
   void timerRepair(const timespec &);

   void registerTimer(int timerId, int interval, QObject *object);
   bool unregisterTimer(int timerId);
   bool unregisterTimers(QObject *object);
   QList<std::pair<int, int> > registeredTimers(QObject *object) const;

   int activateTimers();
};

struct QSockNot {
   QSocketNotifier *obj;
   int fd;
   fd_set *queue;
};

class QSockNotType
{
 public:
   QSockNotType();
   ~QSockNotType();

   typedef QPodList<QSockNot *, 32> List;

   List list;
   fd_set select_fds;
   fd_set enabled_fds;
   fd_set pending_fds;

};

class QEventDispatcherUNIXPrivate;

class Q_CORE_EXPORT QEventDispatcherUNIX : public QAbstractEventDispatcher
{
   CORE_CS_OBJECT(QEventDispatcherUNIX)
   Q_DECLARE_PRIVATE(QEventDispatcherUNIX)

 public:
   explicit QEventDispatcherUNIX(QObject *parent = nullptr);
   ~QEventDispatcherUNIX();

   bool processEvents(QEventLoop::ProcessEventsFlags flags) override;
   bool hasPendingEvents() override;

   void registerSocketNotifier(QSocketNotifier *notifier) override;
   void unregisterSocketNotifier(QSocketNotifier *notifier) override;

   void registerTimer(int timerId, int interval, QObject *object) override;
   bool unregisterTimer(int timerId) override;
   bool unregisterTimers(QObject *object) override;
   QList<TimerInfo> registeredTimers(QObject *object) const override;

   void wakeUp() override;
   void interrupt() override;
   void flush() override;

 protected:
   QEventDispatcherUNIX(QEventDispatcherUNIXPrivate &dd, QObject *parent = nullptr);

   void setSocketNotifierPending(QSocketNotifier *notifier);

   int activateTimers();
   int activateSocketNotifiers();

   virtual int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, timespec *timeout);
};

class Q_CORE_EXPORT QEventDispatcherUNIXPrivate : public QAbstractEventDispatcherPrivate
{
   Q_DECLARE_PUBLIC(QEventDispatcherUNIX)

 public:
   QEventDispatcherUNIXPrivate();
   ~QEventDispatcherUNIXPrivate();

   int doSelect(QEventLoop::ProcessEventsFlags flags, timespec *timeout);

   bool mainThread;
   int thread_pipe[2];

   // highest fd for all socket notifiers
   int sn_highest;

   // 3 socket notifier types - read, write and exception
   QSockNotType sn_vec[3];

   QTimerInfoList timerList;

   // pending socket notifiers list
   QSockNotType::List sn_pending_list;

   QAtomicInt wakeUps;
   bool interrupt;
};

QT_END_NAMESPACE

#endif // QEVENTDISPATCHER_UNIX_P_H
