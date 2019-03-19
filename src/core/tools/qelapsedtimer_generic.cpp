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

#include <qelapsedtimer.h>
#include <qdatetime.h>

QT_BEGIN_NAMESPACE

/*!
    Returns the clock type that this QElapsedTimer implementation uses.

    \sa isMonotonic()
*/
QElapsedTimer::ClockType QElapsedTimer::clockType()
{
   return SystemTime;
}

/*!
    Returns true if this is a monotonic clock, false otherwise. See the
    information on the different clock types to understand which ones are
    monotonic.

    \sa clockType(), QElapsedTimer::ClockType
*/
bool QElapsedTimer::isMonotonic()
{
   return false;
}

/*!
    Starts this timer. Once started, a timer value can be checked with elapsed() or msecsSinceReference().

    Normally, a timer is started just before a lengthy operation, such as:
    \snippet doc/src/snippets/qelapsedtimer/main.cpp 0

    Also, starting a timer makes it valid again.

    \sa restart(), invalidate(), elapsed()
*/
void QElapsedTimer::start()
{
   restart();
}

/*!
    Restarts the timer and returns the time elapsed since the previous start.
    This function is equivalent to obtaining the elapsed time with elapsed()
    and then starting the timer again with start(), but it does so in one
    single operation, avoiding the need to obtain the clock value twice.

    The following example illustrates how to use this function to calibrate a
    parameter to a slow operation (for example, an iteration count) so that
    this operation takes at least 250 milliseconds:

    \snippet doc/src/snippets/qelapsedtimer/main.cpp 3

    \sa start(), invalidate(), elapsed()
*/
qint64 QElapsedTimer::restart()
{
   qint64 old = t1;
   t1 = QDateTime::currentMSecsSinceEpoch();
   t2 = 0;
   return t1 - old;
}

/*! \since 4.8

    Returns the number of nanoseconds since this QElapsedTimer was last
    started. Calling this function in a QElapsedTimer that was invalidated
    will result in undefined results.

    On platforms that do not provide nanosecond resolution, the value returned
    will be the best estimate available.

    \sa start(), restart(), hasExpired(), invalidate()
*/
qint64 QElapsedTimer::nsecsElapsed() const
{
   return elapsed() * 1000000;
}

/*!
    Returns the number of milliseconds since this QElapsedTimer was last
    started. Calling this function in a QElapsedTimer that was invalidated
    will result in undefined results.

    \sa start(), restart(), hasExpired(), invalidate()
*/
qint64 QElapsedTimer::elapsed() const
{
   return QDateTime::currentMSecsSinceEpoch() - t1;
}

/*!
    Returns the number of milliseconds between last time this QElapsedTimer
    object was started and its reference clock's start.

    This number is usually arbitrary for all clocks except the
    QElapsedTimer::SystemTime clock. For that clock type, this number is the
    number of milliseconds since January 1st, 1970 at 0:00 UTC (that is, it
    is the Unix time expressed in milliseconds).

    \sa clockType(), elapsed()
*/
qint64 QElapsedTimer::msecsSinceReference() const
{
   return t1;
}

/*!
    Returns the number of milliseconds between this QElapsedTimer and \a
    other. If \a other was started before this object, the returned value
    will be positive. If it was started later, the returned value will be
    negative.

    The return value is undefined if this object or \a other were invalidated.

    \sa secsTo(), elapsed()
*/
qint64 QElapsedTimer::msecsTo(const QElapsedTimer &other) const
{
   qint64 diff = other.t1 - t1;
   return diff;
}

/*!
    Returns the number of seconds between this QElapsedTimer and \a other. If
    \a other was started before this object, the returned value will be
    positive. If it was started later, the returned value will be negative.

    The return value is undefined if this object or \a other were invalidated.

    \sa msecsTo(), elapsed()
*/
qint64 QElapsedTimer::secsTo(const QElapsedTimer &other) const
{
   return msecsTo(other) / 1000;
}

/*!
    \relates QElapsedTimer

    Returns true if \a v1 was started before \a v2, false otherwise.

    The returned value is undefined if one of the two parameters is invalid
    and the other isn't. However, two invalid timers are equal and thus this
    function will return false.
*/
bool operator<(const QElapsedTimer &v1, const QElapsedTimer &v2)
{
   return v1.t1 < v2.t1;
}

QT_END_NAMESPACE
