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

#include <qcore_unix_p.h>

#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#if defined(QT_NO_CLOCK_MONOTONIC)
// turn off the monotonic clock
# ifdef _POSIX_MONOTONIC_CLOCK
#  undef _POSIX_MONOTONIC_CLOCK
# endif
# define _POSIX_MONOTONIC_CLOCK -1
#endif

#if (_POSIX_MONOTONIC_CLOCK-0 != 0)
static constexpr const bool monotonicClockChecked   = true;
static constexpr const bool monotonicClockAvailable = _POSIX_MONOTONIC_CLOCK > 0;
#else
static int monotonicClockChecked = false;
static int monotonicClockAvailable = false;
#endif

#ifdef Q_CC_GNU
# define is_likely(x) __builtin_expect((x), 1)
#else
# define is_likely(x) (x)
#endif
#define load_acquire(x) ((volatile const int&)(x))
#define store_release(x,v) ((volatile int&)(x) = (v))

static void unixCheckClockType()
{
#if (_POSIX_MONOTONIC_CLOCK-0 == 0)

   if (is_likely(load_acquire(monotonicClockChecked))) {
      return;
   }

# if defined(_SC_MONOTONIC_CLOCK)
   // detect if the system support monotonic timers
   long x = sysconf(_SC_MONOTONIC_CLOCK);
   store_release(monotonicClockAvailable, x >= 200112L);
# endif

   store_release(monotonicClockChecked, true);
#endif
}

bool QElapsedTimer::isMonotonic()
{
   unixCheckClockType();
   return monotonicClockAvailable;
}

QElapsedTimer::ClockType QElapsedTimer::clockType()
{
   unixCheckClockType();
   return monotonicClockAvailable ? MonotonicClock : SystemTime;
}

static inline void do_gettime(qint64 *sec, qint64 *frac)
{
#if (_POSIX_MONOTONIC_CLOCK-0 >= 0)
   unixCheckClockType();

   if (is_likely(monotonicClockAvailable)) {
      timespec ts;
      clock_gettime(CLOCK_MONOTONIC, &ts);
      *sec = ts.tv_sec;
      *frac = ts.tv_nsec;
      return;
   }

#endif
   // use gettimeofday
   struct timeval tv;
   ::gettimeofday(&tv, nullptr);
   *sec = tv.tv_sec;
   *frac = tv.tv_usec;
}

// used in qcore_unix.cpp and qeventdispatcher_unix.cpp
struct timespec qt_gettime()
{
   qint64 sec, frac;
   do_gettime(&sec, &frac);

   timespec tv;
   tv.tv_sec = sec;
   tv.tv_nsec = frac;

   return tv;
}

void qt_nanosleep(timespec amount)
{
   // like to use clock_nanosleep.
   //
   // But clock_nanosleep is from POSIX.1-2001 and both are *not*
   // affected by clock changes when using relative sleeps, even for
   // CLOCK_REALTIME.
   //
   // nanosleep is POSIX.1-1993

   int r;
   EINTR_LOOP(r, nanosleep(&amount, &amount));
}
static qint64 elapsedAndRestart(qint64 sec, qint64 frac, qint64 *nowsec, qint64 *nowfrac)
{
   do_gettime(nowsec, nowfrac);
   sec = *nowsec - sec;
   frac = *nowfrac - frac;
   return (sec * Q_INT64_C(1000000000) + frac) / Q_INT64_C(1000000);
}

void QElapsedTimer::start()
{
   do_gettime(&t1, &t2);
}

qint64 QElapsedTimer::restart()
{
   return elapsedAndRestart(t1, t2, &t1, &t2);
}

qint64 QElapsedTimer::nsecsElapsed() const
{
   qint64 sec, frac;
   do_gettime(&sec, &frac);
   sec = sec - t1;
   frac = frac - t2;

   return sec * Q_INT64_C(1000000000) + frac;
}

qint64 QElapsedTimer::elapsed() const
{
   return nsecsElapsed() / Q_INT64_C(1000000);
}

qint64 QElapsedTimer::msecsSinceReference() const
{
   return t1 * Q_INT64_C(1000) + t2 / Q_INT64_C(1000000);
}

qint64 QElapsedTimer::msecsTo(const QElapsedTimer &other) const
{
   qint64 secs = other.t1 - t1;
   qint64 fraction = other.t2 - t2;

   return (secs * Q_INT64_C(1000000000) + fraction) / Q_INT64_C(1000000);
}

qint64 QElapsedTimer::secsTo(const QElapsedTimer &other) const
{
   return other.t1 - t1;
}

bool operator<(const QElapsedTimer &v1, const QElapsedTimer &v2)
{
   return v1.t1 < v2.t1 || (v1.t1 == v2.t1 && v1.t2 < v2.t2);
}
