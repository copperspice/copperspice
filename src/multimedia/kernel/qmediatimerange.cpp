/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qdebug.h>

#include <qmediatimerange.h>


/*!
\class QMediaTimeInterval
\brief The QMediaTimeInterval class represents a time interval with integer precision.

An interval is specified by an inclusive start() and end() time.  These
must be set in the constructor, as this is an immutable class.  The
specific units of time represented by the class have not been defined - it
is suitable for any times which can be represented by a signed 64 bit
integer.

The isNormal() method determines if a time interval is normal (a normal
time interval has start() <= end()). A normal interval can be received
from an abnormal interval by calling the normalized() method.

The contains() method determines if a specified time lies within the time interval.

The translated() method returns a time interval which has been translated
forwards or backwards through time by a specified offset.

\sa QMediaTimeRange

*/

/*!
\fn QMediaTimeInterval::QMediaTimeInterval()

Constructs an empty interval.

*/

QMediaTimeInterval::QMediaTimeInterval()
   : s(0), e(0)
{
}

/*!
    \fn QMediaTimeInterval::QMediaTimeInterval(qint64 start, qint64 end)

    Constructs an interval with the specified \a start and \a end times.
*/
QMediaTimeInterval::QMediaTimeInterval(qint64 start, qint64 end)
   : s(start)
   , e(end)
{

}

/*!
    \fn QMediaTimeInterval::QMediaTimeInterval(const QMediaTimeInterval &other)

    Constructs an interval by taking a copy of \a other.
*/
QMediaTimeInterval::QMediaTimeInterval(const QMediaTimeInterval &other)
   : s(other.s)
   , e(other.e)
{

}

/*!
    \fn QMediaTimeInterval::start() const

    Returns the start time of the interval.

    \sa end()
*/
qint64 QMediaTimeInterval::start() const
{
   return s;
}

/*!
    \fn QMediaTimeInterval::end() const

    Returns the end time of the interval.

    \sa start()
*/
qint64 QMediaTimeInterval::end() const
{
   return e;
}

/*!
    \fn QMediaTimeInterval::contains(qint64 time) const

    Returns true if the time interval contains the specified \a time.
    That is, start() <= time <= end().
*/
bool QMediaTimeInterval::contains(qint64 time) const
{
   return isNormal() ? (s <= time && time <= e)
      : (e <= time && time <= s);
}

/*!
    \fn QMediaTimeInterval::isNormal() const

    Returns true if this time interval is normal.
    A normal time interval has start() <= end().

    \sa normalized()
*/
bool QMediaTimeInterval::isNormal() const
{
   return s <= e;
}

/*!
    \fn QMediaTimeInterval::normalized() const

    Returns a normalized version of this interval.

    If the start() time of the interval is greater than the end() time,
    then the returned interval has the start and end times swapped.
*/
QMediaTimeInterval QMediaTimeInterval::normalized() const
{
   if (s > e) {
      return QMediaTimeInterval(e, s);
   }

   return *this;
}

/*!
    \fn QMediaTimeInterval::translated(qint64 offset) const

    Returns a copy of this time interval, translated by a value of \a offset.
    An interval can be moved forward through time with a positive offset, or backward
    through time with a negative offset.
*/
QMediaTimeInterval QMediaTimeInterval::translated(qint64 offset) const
{
   return QMediaTimeInterval(s + offset, e + offset);
}

/*!
    \fn operator==(const QMediaTimeInterval &a, const QMediaTimeInterval &b)
    \relates QMediaTimeRange

    Returns true if \a a is exactly equal to \a b.
*/
bool operator==(const QMediaTimeInterval &a, const QMediaTimeInterval &b)
{
   return a.start() == b.start() && a.end() == b.end();
}

/*!
    \fn operator!=(const QMediaTimeInterval &a, const QMediaTimeInterval &b)
    \relates QMediaTimeRange

    Returns true if \a a is not exactly equal to \a b.
*/
bool operator!=(const QMediaTimeInterval &a, const QMediaTimeInterval &b)
{
   return a.start() != b.start() || a.end() != b.end();
}

class QMediaTimeRangePrivate : public QSharedData
{
 public:

   QMediaTimeRangePrivate();
   QMediaTimeRangePrivate(const QMediaTimeRangePrivate &other);
   QMediaTimeRangePrivate(const QMediaTimeInterval &interval);

   QList<QMediaTimeInterval> intervals;

   void addInterval(const QMediaTimeInterval &interval);
   void removeInterval(const QMediaTimeInterval &interval);
};

QMediaTimeRangePrivate::QMediaTimeRangePrivate()
   : QSharedData()
{

}

QMediaTimeRangePrivate::QMediaTimeRangePrivate(const QMediaTimeRangePrivate &other)
   : QSharedData()
   , intervals(other.intervals)
{

}

QMediaTimeRangePrivate::QMediaTimeRangePrivate(const QMediaTimeInterval &interval)
   : QSharedData()
{
   if (interval.isNormal()) {
      intervals << interval;
   }
}

void QMediaTimeRangePrivate::addInterval(const QMediaTimeInterval &interval)
{
   // Handle normalized intervals only
   if (!interval.isNormal()) {
      return;
   }

   // Find a place to insert the interval
   int i;
   for (i = 0; i < intervals.count(); i++) {
      // Insert before this element
      if (interval.s < intervals[i].s) {
         intervals.insert(i, interval);
         break;
      }
   }

   // Interval needs to be added to the end of the list
   if (i == intervals.count()) {
      intervals.append(interval);
   }

   // Do we need to correct the element before us?
   if (i > 0 && intervals[i - 1].e >= interval.s - 1) {
      i--;
   }

   // Merge trailing ranges
   while (i < intervals.count() - 1
      && intervals[i].e >= intervals[i + 1].s - 1) {
      intervals[i].e = qMax(intervals[i].e, intervals[i + 1].e);
      intervals.removeAt(i + 1);
   }
}

void QMediaTimeRangePrivate::removeInterval(const QMediaTimeInterval &interval)
{
   // Handle normalized intervals only
   if (!interval.isNormal()) {
      return;
   }

   for (int i = 0; i < intervals.count(); i++) {
      QMediaTimeInterval r = intervals[i];

      if (r.e < interval.s) {
         // Before the removal interval
         continue;
      } else if (interval.e < r.s) {
         // After the removal interval - stop here
         break;
      } else if (r.s < interval.s && interval.e < r.e) {
         // Split case - a single range has a chunk removed
         intervals[i].e = interval.s - 1;
         addInterval(QMediaTimeInterval(interval.e + 1, r.e));
         break;
      } else if (r.s < interval.s) {
         // Trimming Tail Case
         intervals[i].e = interval.s - 1;
      } else if (interval.e < r.e) {
         // Trimming Head Case - we can stop after this
         intervals[i].s = interval.e + 1;
         break;
      } else {
         // Complete coverage case
         intervals.removeAt(i);
         --i;
      }
   }
}

/*!
    \class QMediaTimeRange
    \brief The QMediaTimeRange class represents a set of zero or more disjoint
    time intervals.
    \ingroup multimedia
    \inmodule QtMultimedia

    \reentrant

    The earliestTime(), latestTime(), intervals() and isEmpty()
    methods are used to get information about the current time range.

    The addInterval(), removeInterval() and clear() methods are used to modify
    the current time range.

    When adding or removing intervals from the time range, existing intervals
    within the range may be expanded, trimmed, deleted, merged or split to ensure
    that all intervals within the time range remain distinct and disjoint. As a
    consequence, all intervals added or removed from a time range must be
    \l{QMediaTimeInterval::isNormal()}{normal}.

    \sa QMediaTimeInterval
*/

/*!
    \fn QMediaTimeRange::QMediaTimeRange()

    Constructs an empty time range.
*/
QMediaTimeRange::QMediaTimeRange()
   : d(new QMediaTimeRangePrivate)
{

}

/*!
    \fn QMediaTimeRange::QMediaTimeRange(qint64 start, qint64 end)

    Constructs a time range that contains an initial interval from
    \a start to \a end inclusive.

    If the interval is not \l{QMediaTimeInterval::isNormal()}{normal},
    the resulting time range will be empty.

    \sa addInterval()
*/
QMediaTimeRange::QMediaTimeRange(qint64 start, qint64 end)
   : d(new QMediaTimeRangePrivate(QMediaTimeInterval(start, end)))
{

}

/*!
    \fn QMediaTimeRange::QMediaTimeRange(const QMediaTimeInterval &interval)

    Constructs a time range that contains an initial interval, \a interval.

    If \a interval is not \l{QMediaTimeInterval::isNormal()}{normal},
    the resulting time range will be empty.

    \sa addInterval()
*/
QMediaTimeRange::QMediaTimeRange(const QMediaTimeInterval &interval)
   : d(new QMediaTimeRangePrivate(interval))
{

}

/*!
    \fn QMediaTimeRange::QMediaTimeRange(const QMediaTimeRange &range)

    Constructs a time range by copying another time \a range.
*/
QMediaTimeRange::QMediaTimeRange(const QMediaTimeRange &range)
   : d(range.d)
{

}

/*!
    \fn QMediaTimeRange::~QMediaTimeRange()

    Destructor.
*/
QMediaTimeRange::~QMediaTimeRange()
{

}

/*!
    \fn QMediaTimeRange::operator=(const QMediaTimeRange &other)

    Takes a copy of the \a other time range and returns itself.
*/
QMediaTimeRange &QMediaTimeRange::operator=(const QMediaTimeRange &other)
{
   d = other.d;
   return *this;
}

/*!
    \fn QMediaTimeRange::operator=(const QMediaTimeInterval &interval)

    Sets the time range to a single continuous interval, \a interval.
*/
QMediaTimeRange &QMediaTimeRange::operator=(const QMediaTimeInterval &interval)
{
   d = new QMediaTimeRangePrivate(interval);
   return *this;
}

/*!
    \fn QMediaTimeRange::earliestTime() const

    Returns the earliest time within the time range.

    For empty time ranges, this value is equal to zero.

    \sa latestTime()
*/
qint64 QMediaTimeRange::earliestTime() const
{
   if (!d->intervals.isEmpty()) {
      return d->intervals[0].s;
   }

   return 0;
}

/*!
    \fn QMediaTimeRange::latestTime() const

    Returns the latest time within the time range.

    For empty time ranges, this value is equal to zero.

    \sa earliestTime()
*/
qint64 QMediaTimeRange::latestTime() const
{
   if (!d->intervals.isEmpty()) {
      return d->intervals[d->intervals.count() - 1].e;
   }

   return 0;
}

/*!
    \fn QMediaTimeRange::addInterval(qint64 start, qint64 end)
    \overload

    Adds the interval specified by \a start and \a end
    to the time range.

    \sa addInterval()
*/
void QMediaTimeRange::addInterval(qint64 start, qint64 end)
{
   d->addInterval(QMediaTimeInterval(start, end));
}

/*!
    \fn QMediaTimeRange::addInterval(const QMediaTimeInterval &interval)

    Adds the specified \a interval to the time range.

    Adding intervals which are not \l{QMediaTimeInterval::isNormal()}{normal}
    is invalid, and will be ignored.

    If the specified interval is adjacent to, or overlaps existing
    intervals within the time range, these intervals will be merged.

    This operation takes linear time.

    \sa removeInterval()
*/
void QMediaTimeRange::addInterval(const QMediaTimeInterval &interval)
{
   d->addInterval(interval);
}

/*!
    \fn QMediaTimeRange::addTimeRange(const QMediaTimeRange &range)

    Adds each of the intervals in \a range to this time range.

    Equivalent to calling addInterval() for each interval in \a range.
*/
void QMediaTimeRange::addTimeRange(const QMediaTimeRange &range)
{
   for (const QMediaTimeInterval &i : range.intervals()) {
      d->addInterval(i);
   }
}

/*!
    \fn QMediaTimeRange::removeInterval(qint64 start, qint64 end)
    \overload

    Removes the interval specified by \a start and \a end
    from the time range.

    \sa removeInterval()
*/
void QMediaTimeRange::removeInterval(qint64 start, qint64 end)
{
   d->removeInterval(QMediaTimeInterval(start, end));
}

/*!
    \fn QMediaTimeRange::removeInterval(const QMediaTimeInterval &interval)

    Removes the specified \a interval from the time range.

    Removing intervals which are not \l{QMediaTimeInterval::isNormal()}{normal}
    is invalid, and will be ignored.

    Intervals within the time range will be trimmed, split or deleted
    such that no intervals within the time range include any part of the
    target interval.

    This operation takes linear time.

    \sa addInterval()
*/
void QMediaTimeRange::removeInterval(const QMediaTimeInterval &interval)
{
   d->removeInterval(interval);
}

/*!
    \fn QMediaTimeRange::removeTimeRange(const QMediaTimeRange &range)

    Removes each of the intervals in \a range from this time range.

    Equivalent to calling removeInterval() for each interval in \a range.
*/
void QMediaTimeRange::removeTimeRange(const QMediaTimeRange &range)
{
   for (const QMediaTimeInterval &i : range.intervals()) {
      d->removeInterval(i);
   }
}

/*!
    \fn QMediaTimeRange::operator+=(const QMediaTimeRange &other)

    Adds each interval in \a other to the time range and returns the result.
*/
QMediaTimeRange &QMediaTimeRange::operator+=(const QMediaTimeRange &other)
{
   addTimeRange(other);
   return *this;
}

/*!
    \fn QMediaTimeRange::operator+=(const QMediaTimeInterval &interval)

    Adds the specified \a interval to the time range and returns the result.
*/
QMediaTimeRange &QMediaTimeRange::operator+=(const QMediaTimeInterval &interval)
{
   addInterval(interval);
   return *this;
}

/*!
    \fn QMediaTimeRange::operator-=(const QMediaTimeRange &other)

    Removes each interval in \a other from the time range and returns the result.
*/
QMediaTimeRange &QMediaTimeRange::operator-=(const QMediaTimeRange &other)
{
   removeTimeRange(other);
   return *this;
}

/*!
    \fn QMediaTimeRange::operator-=(const QMediaTimeInterval &interval)

    Removes the specified \a interval from the time range and returns the result.
*/
QMediaTimeRange &QMediaTimeRange::operator-=(const QMediaTimeInterval &interval)
{
   removeInterval(interval);
   return *this;
}

/*!
    \fn QMediaTimeRange::clear()

    Removes all intervals from the time range.

    \sa removeInterval()
*/
void QMediaTimeRange::clear()
{
   d->intervals.clear();
}

/*!
    \fn QMediaTimeRange::intervals() const

    Returns the list of intervals covered by this time range.
*/
QList<QMediaTimeInterval> QMediaTimeRange::intervals() const
{
   return d->intervals;
}

/*!
    \fn QMediaTimeRange::isEmpty() const

    Returns true if there are no intervals within the time range.

    \sa intervals()
*/
bool QMediaTimeRange::isEmpty() const
{
   return d->intervals.isEmpty();
}

/*!
    \fn QMediaTimeRange::isContinuous() const

    Returns true if the time range consists of a continuous interval.
    That is, there is one or fewer disjoint intervals within the time range.
*/
bool QMediaTimeRange::isContinuous() const
{
   return (d->intervals.count() <= 1);
}

/*!
    \fn QMediaTimeRange::contains(qint64 time) const

    Returns true if the specified \a time lies within the time range.
*/
bool QMediaTimeRange::contains(qint64 time) const
{
   for (int i = 0; i < d->intervals.count(); i++) {
      if (d->intervals[i].contains(time)) {
         return true;
      }

      if (time < d->intervals[i].s) {
         break;
      }
   }

   return false;
}

/*!
    \fn operator==(const QMediaTimeRange &a, const QMediaTimeRange &b)
    \relates QMediaTimeRange

    Returns true if all intervals in \a a are present in \a b.
*/
bool operator==(const QMediaTimeRange &a, const QMediaTimeRange &b)
{
   if (a.intervals().count() != b.intervals().count()) {
      return false;
   }

   for (int i = 0; i < a.intervals().count(); i++) {
      if (a.intervals()[i] != b.intervals()[i]) {
         return false;
      }
   }

   return true;
}

/*!
    \fn operator!=(const QMediaTimeRange &a, const QMediaTimeRange &b)
    \relates QMediaTimeRange

    Returns true if one or more intervals in \a a are not present in \a b.
*/
bool operator!=(const QMediaTimeRange &a, const QMediaTimeRange &b)
{
   return !(a == b);
}

/*!
    \fn operator+(const QMediaTimeRange &r1, const QMediaTimeRange &r2)
    \relates QMediaTimeRange

    Returns a time range containing the union between \a r1 and \a r2.
 */
QMediaTimeRange operator+(const QMediaTimeRange &r1, const QMediaTimeRange &r2)
{
   return (QMediaTimeRange(r1) += r2);
}

/*!
    \fn operator-(const QMediaTimeRange &r1, const QMediaTimeRange &r2)
    \relates QMediaTimeRange

    Returns a time range containing \a r2 subtracted from \a r1.
 */
QMediaTimeRange operator-(const QMediaTimeRange &r1, const QMediaTimeRange &r2)
{
   return (QMediaTimeRange(r1) -= r2);
}

QDebug operator<<(QDebug dbg, const QMediaTimeRange &range)
{
   QDebugStateSaver saver(dbg);

   dbg.nospace();
   dbg << "QMediaTimeRange( ";

   for (const QMediaTimeInterval &interval : range.intervals()) {
      dbg << '(' <<  interval.start() << ", " << interval.end() << ") ";
   }

   dbg.space();
   dbg << ')';

   return dbg;
}



