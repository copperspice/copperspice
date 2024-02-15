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

#ifndef QEVENTDISPATCHER_UNIX_P_H
#define QEVENTDISPATCHER_UNIX_P_H

#include <qabstracteventdispatcher.h>
#include <qlist.h>
#include <qvarlengtharray.h>

#include <qabstracteventdispatcher_p.h>
#include <qcore_unix_p.h>
#include <qpodlist_p.h>
#include <qtimerinfo_unix_p.h>

#include <sys/time.h>
#include <sys/select.h>

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

   using List = QPodList<QSockNot *, 32>;

   List list;
   fd_set select_fds;
   fd_set enabled_fds;
   fd_set pending_fds;
};

#ifdef check
// defined in Apple's /usr/include/AssertMacros.h header
#  undef check
#endif
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

   void registerTimer(int timerId, int interval, Qt::TimerType timerType, QObject *object) override;
   bool unregisterTimer(int timerId) override;
   bool unregisterTimers(QObject *object) override;
   QList<QTimerInfo> registeredTimers(QObject *object) const override;

   int remainingTime(int timerId) override;
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
   virtual int initThreadWakeUp();
   virtual int processThreadWakeUp(int nsel);

   bool mainThread;
   // note for eventfd(7) support:
   // if thread_pipe[1] is -1, then eventfd(7) is in use and is stored in thread_pipe[0]
   int thread_pipe[2];

   // highest fd for all socket notifiers
   int sn_highest;

   // 3 socket notifier types - read, write and exception
   QSockNotType sn_vec[3];

   QTimerInfoList timerList;

   // pending socket notifiers list
   QSockNotType::List sn_pending_list;

   QAtomicInt wakeUps;
   std::atomic<bool> interrupt;
};

#endif // QEVENTDISPATCHER_UNIX_P_H
