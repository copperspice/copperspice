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

#ifndef QTIMERINFO_UNIX_P_H
#define QTIMERINFO_UNIX_P_H

#include <qabstracteventdispatcher.h>

#include <sys/time.h>

// internal timer info
struct QTimerInfo_Unix {
   int id;                           // timer identifier
   int interval;                     // timer interval in milliseconds
   Qt::TimerType timerType;          // timer type
   timespec timeout;                 // when to actually fire
   QObject *obj;                     // object to receive event
   QTimerInfo_Unix **activateRef;    // ref from activateTimers

#if defined(CS_SHOW_DEBUG_CORE)
   timeval expected;                 // when timer is expected to fire
   float cumulativeError;
   uint count;
#endif
};

class Q_CORE_EXPORT QTimerInfoList : public QList<QTimerInfo_Unix *>
{
#if (_POSIX_MONOTONIC_CLOCK-0 <= 0) && ! defined(Q_OS_DARWIN)
   timespec previousTime;
   clock_t previousTicks;
   int ticksPerSecond;
   int msPerTick;

   bool timeChanged(timespec *delta);
   void timerRepair(const timespec &);
#endif

   // state variables used by activateTimers()
   QTimerInfo_Unix *firstTimerInfo;

 public:
   QTimerInfoList();

   timespec currentTime;
   timespec updateCurrentTime();

   // must call updateCurrentTime() first!
   void repairTimersIfNeeded();

   bool timerWait(timespec &);
   void timerInsert(QTimerInfo_Unix *);

   int timerRemainingTime(int timerId);

   void registerTimer(int timerId, int interval, Qt::TimerType timerType, QObject *object);
   bool unregisterTimer(int timerId);
   bool unregisterTimers(QObject *object);
   QList<QTimerInfo> registeredTimers(QObject *object) const;

   int activateTimers();
};

#endif
