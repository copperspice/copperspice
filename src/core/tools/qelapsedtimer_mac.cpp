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

#include <qelapsedtimer.h>

#include <sys/time.h>
#include <unistd.h>

#include <mach/mach_time.h>

QElapsedTimer::ClockType QElapsedTimer::clockType()
{
   return MachAbsoluteTime;
}

bool QElapsedTimer::isMonotonic()
{
   return true;
}

static mach_timebase_info_data_t info = {0, 0};

static qint64 absoluteToNSecs(qint64 cpuTime)
{
   if (info.denom == 0) {
      mach_timebase_info(&info);
   }

   qint64 nsecs = cpuTime * info.numer / info.denom;
   return nsecs;
}

static qint64 absoluteToMSecs(qint64 cpuTime)
{
   return absoluteToNSecs(cpuTime) / 1000000;
}

timeval qt_gettime()
{
   timeval tv;

   uint64_t cpu_time = mach_absolute_time();
   uint64_t nsecs = absoluteToNSecs(cpu_time);
   tv.tv_sec = nsecs / 1000000000ull;
   tv.tv_usec = (nsecs / 1000) - (tv.tv_sec * 1000000);
   return tv;
}

void QElapsedTimer::start()
{
   t1 = mach_absolute_time();
   t2 = 0;
}

qint64 QElapsedTimer::restart()
{
   qint64 old = t1;
   t1 = mach_absolute_time();
   t2 = 0;

   return absoluteToMSecs(t1 - old);
}

qint64 QElapsedTimer::nsecsElapsed() const
{
   uint64_t cpu_time = mach_absolute_time();
   return absoluteToNSecs(cpu_time - t1);
}

qint64 QElapsedTimer::elapsed() const
{
   uint64_t cpu_time = mach_absolute_time();
   return absoluteToMSecs(cpu_time - t1);
}

qint64 QElapsedTimer::msecsSinceReference() const
{
   return absoluteToMSecs(t1);
}

qint64 QElapsedTimer::msecsTo(const QElapsedTimer &other) const
{
   return absoluteToMSecs(other.t1 - t1);
}

qint64 QElapsedTimer::secsTo(const QElapsedTimer &other) const
{
   return msecsTo(other) / 1000;
}

bool operator<(const QElapsedTimer &v1, const QElapsedTimer &v2)
{
   return v1.t1 < v2.t1;
}
