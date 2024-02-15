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

#include <qlog.h>

#include <qsystemlibrary_p.h>

#include <windows.h>

using PtrGetTickCount64 = ULONGLONG (WINAPI *)(void);

static PtrGetTickCount64 ptrGetTickCount64 = nullptr;

// Result of QueryPerformanceFrequency, 0 indicates that the high resolution timer is unavailable
static quint64 counterFrequency = 0;

static void resolveLibs()
{
   static volatile bool done = false;

   if (done) {
      return;
   }

   // try to get GetTickCount64 from the system
   QSystemLibrary kernel32(QLatin1String("kernel32"));

   if (! kernel32.load()) {
      return;
   }

   ptrGetTickCount64 = (PtrGetTickCount64)kernel32.resolve("GetTickCount64");

   // Retrieve the number of high-resolution performance counter ticks per second
   LARGE_INTEGER frequency;

   if (!QueryPerformanceFrequency(&frequency)) {
      counterFrequency = 0;
   } else {
      counterFrequency = frequency.QuadPart;
   }

   done = true;
}

static inline qint64 ticksToNanoseconds(qint64 ticks)
{
   if (counterFrequency > 0) {
      // QueryPerformanceCounter uses an arbitrary frequency
      qint64 seconds = ticks / counterFrequency;
      qint64 nanoSeconds = (ticks - seconds * counterFrequency) * 1000000000 / counterFrequency;
      return seconds * 1000000000 + nanoSeconds;
   } else {
      // GetTickCount(64) return milliseconds
      return ticks * 1000000;
   }
}

static quint64 getTickCount()
{
   resolveLibs();

   // This avoids a division by zero and disables the high performance counter if it's not available
   if (counterFrequency > 0) {
      LARGE_INTEGER counter;

      if (QueryPerformanceCounter(&counter)) {
         return counter.QuadPart;
      } else {
         qWarning("getTickCount() QueryPerformanceCounter failed");
         return 0;
      }
   }

   if (ptrGetTickCount64) {
      return ptrGetTickCount64();
   }

   static quint32 highdword = 0;
   static quint32 lastval = 0;
   quint32 val = GetTickCount();

   if (val < lastval) {
      ++highdword;
   }

   lastval = val;
   return val | (quint64(highdword) << 32);
}

quint64 qt_msectime()
{
   return ticksToNanoseconds(getTickCount()) / 1000000;
}

QElapsedTimer::ClockType QElapsedTimer::clockType()
{
   resolveLibs();

   if (counterFrequency > 0) {
      return PerformanceCounter;
   } else {
      return TickCounter;
   }
}

bool QElapsedTimer::isMonotonic()
{
   return true;
}

void QElapsedTimer::start()
{
   t1 = getTickCount();
   t2 = 0;
}

qint64 QElapsedTimer::restart()
{
   qint64 oldt1 = t1;
   t1 = getTickCount();
   t2 = 0;
   return ticksToNanoseconds(t1 - oldt1) / 1000000;
}

qint64 QElapsedTimer::nsecsElapsed() const
{
   qint64 elapsed = getTickCount() - t1;
   return ticksToNanoseconds(elapsed);
}

qint64 QElapsedTimer::elapsed() const
{
   qint64 elapsed = getTickCount() - t1;
   return ticksToNanoseconds(elapsed) / 1000000;
}

qint64 QElapsedTimer::msecsSinceReference() const
{
   return ticksToNanoseconds(t1) / 1000000;
}

qint64 QElapsedTimer::msecsTo(const QElapsedTimer &other) const
{
   qint64 difference = other.t1 - t1;
   return ticksToNanoseconds(difference) / 1000000;
}

qint64 QElapsedTimer::secsTo(const QElapsedTimer &other) const
{
   return msecsTo(other) / 1000;
}

bool operator<(const QElapsedTimer &v1, const QElapsedTimer &v2)
{
   return (v1.t1 - v2.t1) < 0;
}
