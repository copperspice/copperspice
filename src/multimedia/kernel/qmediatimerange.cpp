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

#include <qdebug.h>

#include <qmediatimerange.h>

QMediaTimeInterval::QMediaTimeInterval()
   : s(0), e(0)
{
}

QMediaTimeInterval::QMediaTimeInterval(qint64 start, qint64 end)
   : s(start), e(end)
{
}

qint64 QMediaTimeInterval::start() const
{
   return s;
}

qint64 QMediaTimeInterval::end() const
{
   return e;
}

bool QMediaTimeInterval::contains(qint64 time) const
{
   return isNormal() ? (s <= time && time <= e) : (e <= time && time <= s);
}

bool QMediaTimeInterval::isNormal() const
{
   return s <= e;
}

QMediaTimeInterval QMediaTimeInterval::normalized() const
{
   if (s > e) {
      return QMediaTimeInterval(e, s);
   }

   return *this;
}

QMediaTimeInterval QMediaTimeInterval::translated(qint64 offset) const
{
   return QMediaTimeInterval(s + offset, e + offset);
}

bool operator==(const QMediaTimeInterval &a, const QMediaTimeInterval &b)
{
   return a.start() == b.start() && a.end() == b.end();
}

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
   : QSharedData(), intervals(other.intervals)
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

QMediaTimeRange::QMediaTimeRange()
   : d(new QMediaTimeRangePrivate)
{
}

QMediaTimeRange::QMediaTimeRange(qint64 start, qint64 end)
   : d(new QMediaTimeRangePrivate(QMediaTimeInterval(start, end)))
{
}

QMediaTimeRange::QMediaTimeRange(const QMediaTimeInterval &interval)
   : d(new QMediaTimeRangePrivate(interval))
{
}

QMediaTimeRange::QMediaTimeRange(const QMediaTimeRange &range)
   : d(range.d)
{
}

QMediaTimeRange::~QMediaTimeRange()
{
}

QMediaTimeRange &QMediaTimeRange::operator=(const QMediaTimeRange &other)
{
   d = other.d;
   return *this;
}

QMediaTimeRange &QMediaTimeRange::operator=(const QMediaTimeInterval &interval)
{
   d = new QMediaTimeRangePrivate(interval);
   return *this;
}

qint64 QMediaTimeRange::earliestTime() const
{
   if (!d->intervals.isEmpty()) {
      return d->intervals[0].s;
   }

   return 0;
}

qint64 QMediaTimeRange::latestTime() const
{
   if (!d->intervals.isEmpty()) {
      return d->intervals[d->intervals.count() - 1].e;
   }

   return 0;
}

void QMediaTimeRange::addInterval(qint64 start, qint64 end)
{
   d->addInterval(QMediaTimeInterval(start, end));
}

void QMediaTimeRange::addInterval(const QMediaTimeInterval &interval)
{
   d->addInterval(interval);
}

void QMediaTimeRange::addTimeRange(const QMediaTimeRange &range)
{
   for (const QMediaTimeInterval &i : range.intervals()) {
      d->addInterval(i);
   }
}

void QMediaTimeRange::removeInterval(qint64 start, qint64 end)
{
   d->removeInterval(QMediaTimeInterval(start, end));
}

void QMediaTimeRange::removeInterval(const QMediaTimeInterval &interval)
{
   d->removeInterval(interval);
}

void QMediaTimeRange::removeTimeRange(const QMediaTimeRange &range)
{
   for (const QMediaTimeInterval &i : range.intervals()) {
      d->removeInterval(i);
   }
}

QMediaTimeRange &QMediaTimeRange::operator+=(const QMediaTimeRange &other)
{
   addTimeRange(other);
   return *this;
}

QMediaTimeRange &QMediaTimeRange::operator+=(const QMediaTimeInterval &interval)
{
   addInterval(interval);
   return *this;
}

QMediaTimeRange &QMediaTimeRange::operator-=(const QMediaTimeRange &other)
{
   removeTimeRange(other);
   return *this;
}

QMediaTimeRange &QMediaTimeRange::operator-=(const QMediaTimeInterval &interval)
{
   removeInterval(interval);
   return *this;
}

void QMediaTimeRange::clear()
{
   d->intervals.clear();
}

QList<QMediaTimeInterval> QMediaTimeRange::intervals() const
{
   return d->intervals;
}

bool QMediaTimeRange::isEmpty() const
{
   return d->intervals.isEmpty();
}

bool QMediaTimeRange::isContinuous() const
{
   return (d->intervals.count() <= 1);
}

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

bool operator!=(const QMediaTimeRange &a, const QMediaTimeRange &b)
{
   return !(a == b);
}

QMediaTimeRange operator+(const QMediaTimeRange &r1, const QMediaTimeRange &r2)
{
   return (QMediaTimeRange(r1) += r2);
}

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
