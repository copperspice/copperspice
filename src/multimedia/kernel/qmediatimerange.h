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

#ifndef QMEDIATIMERANGE_H
#define QMEDIATIMERANGE_H

#include <qmultimedia.h>
#include <qshareddata.h>

class QMediaTimeRangePrivate;

class Q_MULTIMEDIA_EXPORT QMediaTimeInterval
{
 public:
   QMediaTimeInterval();
   QMediaTimeInterval(qint64 start, qint64 end);

   QMediaTimeInterval(const QMediaTimeInterval &other) = default;

   qint64 start() const;
   qint64 end() const;

   bool contains(qint64 time) const;

   bool isNormal() const;
   QMediaTimeInterval normalized() const;
   QMediaTimeInterval translated(qint64 offset) const;

 private:
   friend class QMediaTimeRangePrivate;
   friend class QMediaTimeRange;

   qint64 s;
   qint64 e;
};

Q_MULTIMEDIA_EXPORT bool operator==(const QMediaTimeInterval &, const QMediaTimeInterval &);
Q_MULTIMEDIA_EXPORT bool operator!=(const QMediaTimeInterval &, const QMediaTimeInterval &);

class Q_MULTIMEDIA_EXPORT QMediaTimeRange
{
 public:

   QMediaTimeRange();
   QMediaTimeRange(qint64 start, qint64 end);
   QMediaTimeRange(const QMediaTimeInterval &interval);

   QMediaTimeRange(const QMediaTimeRange &other);

   ~QMediaTimeRange();

   QMediaTimeRange &operator=(const QMediaTimeRange &other);
   QMediaTimeRange &operator=(const QMediaTimeInterval &interval);

   qint64 earliestTime() const;
   qint64 latestTime() const;

   QList<QMediaTimeInterval> intervals() const;
   bool isEmpty() const;
   bool isContinuous() const;

   bool contains(qint64 time) const;

   void addInterval(qint64 start, qint64 end);
   void addInterval(const QMediaTimeInterval &interval);
   void addTimeRange(const QMediaTimeRange &range);

   void removeInterval(qint64 start, qint64 end);
   void removeInterval(const QMediaTimeInterval &interval);
   void removeTimeRange(const QMediaTimeRange &range);

   QMediaTimeRange &operator+=(const QMediaTimeRange &other);
   QMediaTimeRange &operator+=(const QMediaTimeInterval &interval);
   QMediaTimeRange &operator-=(const QMediaTimeRange &other);
   QMediaTimeRange &operator-=(const QMediaTimeInterval &interval);

   void clear();

 private:
   QSharedDataPointer<QMediaTimeRangePrivate> d;
};

Q_MULTIMEDIA_EXPORT bool operator==(const QMediaTimeRange &, const QMediaTimeRange &);
Q_MULTIMEDIA_EXPORT bool operator!=(const QMediaTimeRange &, const QMediaTimeRange &);
Q_MULTIMEDIA_EXPORT QMediaTimeRange operator+(const QMediaTimeRange &, const QMediaTimeRange &);
Q_MULTIMEDIA_EXPORT QMediaTimeRange operator-(const QMediaTimeRange &, const QMediaTimeRange &);

Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, const QMediaTimeRange &);

#endif
