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

#ifndef QDATETIME_H
#define QDATETIME_H

#include <qstring.h>
#include <qnamespace.h>
#include <qsharedpointer.h>

#include <limits>

class QTimeZone;

#ifdef Q_OS_MAC

using CFDateRef = const struct __CFDate *;

#ifdef __OBJC__
@class NSDate;

#else
class NSDate;

#endif

#endif

class Q_CORE_EXPORT QDate
{
 public:
   enum MonthNameType {
      DateFormat = 0,
      StandaloneFormat
   };

   constexpr QDate() : jd(nullJd())
   {}

   QDate(int y, int m, int d);

   bool isNull() const {
      return ! isValid();
   }

   bool isValid() const {
      return jd >= minJd() && jd <= maxJd();
   }

   int year() const;
   int month() const;
   int day() const;
   int dayOfWeek() const;
   int dayOfYear() const;
   int daysInMonth() const;
   int daysInYear() const;
   int weekNumber(int *yearNumber = nullptr) const;

#ifndef QT_NO_TEXTDATE
   static QString shortMonthName(int month, MonthNameType type = DateFormat);
   static QString shortDayName(int weekday, MonthNameType type = DateFormat);
   static QString longMonthName(int month, MonthNameType type = DateFormat);
   static QString longDayName(int weekday, MonthNameType type = DateFormat);
#endif

#ifndef QT_NO_DATESTRING
   QString toString(Qt::DateFormat f = Qt::TextDate) const;
   QString toString(const QString &format) const;
#endif

   bool setDate(int year, int month, int day);
   void getDate(int *year, int *month, int *day);

   QDate addDays(qint64 days) const;
   QDate addMonths(qint64 months) const;
   QDate addYears(qint64 years) const;
   qint64 daysTo(const QDate &) const;

   bool operator==(const QDate &other) const {
      return jd == other.jd;
   }
   bool operator!=(const QDate &other) const {
      return jd != other.jd;
   }
   bool operator<(const QDate &other) const {
      return jd < other.jd;
   }
   bool operator<=(const QDate &other) const {
      return jd <= other.jd;
   }
   bool operator>(const QDate &other) const {
      return jd > other.jd;
   }
   bool operator>=(const QDate &other) const {
      return jd >= other.jd;
   }

   static QDate currentDate();

#ifndef QT_NO_DATESTRING
   static QDate fromString(const QString &s, Qt::DateFormat f = Qt::TextDate);
   static QDate fromString(const QString &s, const QString &format);
#endif

   static bool isValid(int y, int m, int d);
   static bool isLeapYear(int year);

   static constexpr QDate fromJulianDay(qint64 dayNumber) {
      return dayNumber >= minJd() && dayNumber <= maxJd() ? QDate(dayNumber) : QDate();
   }

   constexpr qint64 toJulianDay() const {
      return jd;
   }

 private:
   explicit constexpr QDate(qint64 julianDay) : jd(julianDay)
   {}

   static constexpr qint64 nullJd() {
      return std::numeric_limits<qint64>::min();
   }

   static constexpr qint64 minJd() {
      return Q_INT64_C(-784350574879);
   }

   static constexpr qint64 maxJd() {
      return Q_INT64_C( 784354017364);
   }

   qint64 jd;

   friend class QDateTime;
   friend class QDateTimePrivate;

   friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QDate &);
   friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QDate &);
};
Q_DECLARE_TYPEINFO(QDate, Q_MOVABLE_TYPE);

class Q_CORE_EXPORT QTime
{
   explicit constexpr QTime(int ms)
      : mds(ms)
   { }

 public:
   constexpr QTime()
      : mds(NullTime)
   { }

   QTime(int h, int m, int s = 0, int ms = 0);

   constexpr bool isNull() const {
      return mds == NullTime;
   }

   bool isValid() const;

   int hour() const;
   int minute() const;
   int second() const;
   int msec() const;

#ifndef QT_NO_DATESTRING
   QString toString(Qt::DateFormat f = Qt::TextDate) const;
   QString toString(const QString &format) const;
#endif

   bool setHMS(int h, int m, int s, int ms = 0);

   QTime addSecs(int secs) const;
   int secsTo(const QTime &) const;
   QTime addMSecs(int ms) const;
   int msecsTo(const QTime &) const;

   bool operator==(const QTime &other) const {
      return mds == other.mds;
   }

   bool operator!=(const QTime &other) const {
      return mds != other.mds;
   }

   bool operator<(const QTime &other) const {
      return mds < other.mds;
   }

   bool operator<=(const QTime &other) const {
      return mds <= other.mds;
   }

   bool operator>(const QTime &other) const {
      return mds > other.mds;
   }

   bool operator>=(const QTime &other) const {
      return mds >= other.mds;
   }

   static QTime fromMSecsSinceStartOfDay(int msecs) {
      return QTime(msecs);
   }

   int msecsSinceStartOfDay() const {
      return mds == NullTime ? 0 : mds;
   }

   static QTime currentTime();

#ifndef QT_NO_DATESTRING
   static QTime fromString(const QString &s, Qt::DateFormat f = Qt::TextDate);
   static QTime fromString(const QString &s, const QString &format);
#endif

   static bool isValid(int h, int m, int s, int ms = 0);

   void start();
   int restart();
   int elapsed() const;

 private:
   enum TimeFlag { NullTime = -1 };

   constexpr int ds() const {
      return mds == -1 ? 0 : mds;
   }

   int mds;

   friend class QDateTime;
   friend class QDateTimePrivate;

   friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QTime &);
   friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QTime &);
};
Q_DECLARE_TYPEINFO(QTime, Q_MOVABLE_TYPE);

class QDateTimePrivate;

class Q_CORE_EXPORT QDateTime
{
 public:
   QDateTime();
   explicit QDateTime(const QDate &);
   QDateTime(const QDate &date, const QTime &time, Qt::TimeSpec spec = Qt::LocalTime, int offsetSeconds = 0);

   QDateTime(const QDate &date, const QTime &time, const QTimeZone &timeZone);
   QDateTime(const QDateTime &other);
   ~QDateTime();

   QDateTime &operator=(QDateTime &&other) {
      swap(other);
      return *this;
   }

   QDateTime &operator=(const QDateTime &other);

   void swap(QDateTime &other) {
      qSwap(d, other.d);
   }

   bool isNull() const;
   bool isValid() const;

   QDate date() const;
   QTime time() const;
   Qt::TimeSpec timeSpec() const;
   int offsetFromUtc() const;
   QTimeZone timeZone() const;
   QString timeZoneAbbreviation() const;
   bool isDaylightTime() const;

   qint64 toMSecsSinceEpoch() const;
   quint64 toTime_t() const;

   void setDate(const QDate &date);
   void setTime(const QTime &time);
   void setTimeSpec(Qt::TimeSpec spec);
   void setOffsetFromUtc(int offsetSeconds);
   void setTimeZone(const QTimeZone &toZone);
   void setMSecsSinceEpoch(qint64 msecs);
   void setTime_t(quint64 secsSince1Jan1970UTC);

#ifndef QT_NO_DATESTRING
   QString toString(Qt::DateFormat f = Qt::TextDate) const;
   QString toString(const QString &format) const;
#endif

   QDateTime addDays(qint64 days) const;
   QDateTime addMonths(qint64 months) const;
   QDateTime addYears(qint64 years) const;
   QDateTime addSecs(qint64 secs) const;
   QDateTime addMSecs(qint64 msecs) const;

   QDateTime toTimeSpec(Qt::TimeSpec spec) const;

   QDateTime toLocalTime() const {
      return toTimeSpec(Qt::LocalTime);
   }

   QDateTime toUTC() const {
      return toTimeSpec(Qt::UTC);
   }

   QDateTime toOffsetFromUtc(qint64 offsetSeconds) const;
   QDateTime toTimeZone(const QTimeZone &toZone) const;

   qint64 daysTo(const QDateTime &) const;
   qint64 secsTo(const QDateTime &) const;
   qint64 msecsTo(const QDateTime &) const;

   bool operator==(const QDateTime &other) const;
   bool operator!=(const QDateTime &other) const {
      return !(*this == other);
   }

   bool operator<(const QDateTime &other) const;
   bool operator<=(const QDateTime &other) const {
      return !(other < *this);
   }

   bool operator>(const QDateTime &other) const {
      return other < *this;
   }

   inline bool operator>=(const QDateTime &other) const {
      return !(*this < other);
   }

   static QDateTime currentDateTime();
   static QDateTime currentDateTimeUtc();

#ifndef QT_NO_DATESTRING
   static QDateTime fromString(const QString &s, Qt::DateFormat f = Qt::TextDate);
   static QDateTime fromString(const QString &s, const QString &format);
#endif

   static QDateTime fromTime_t(quint64 secsSince1Jan1970UTC, Qt::TimeSpec spec = Qt::LocalTime, int offsetFromUtc = 0);
   static QDateTime fromTime_t(quint64 secsSince1Jan1970UTC, const QTimeZone &timeZone);

   static QDateTime fromMSecsSinceEpoch(qint64 msecs, Qt::TimeSpec spec = Qt::LocalTime, int offsetFromUtc = 0);
   static QDateTime fromMSecsSinceEpoch(qint64 msecs, const QTimeZone &timeZone);
   static qint64 currentMSecsSinceEpoch();

#if defined(Q_OS_MAC)
   static QDateTime fromCFDate(CFDateRef date);
   CFDateRef toCFDate() const;

#  if defined(__OBJC__)
   static QDateTime fromNSDate(const NSDate *date);
   NSDate *toNSDate() const;
#  endif

#endif
 private:
   friend class QDateTimePrivate;

   // emerald - for performance add the qdatetimePrivate data members directly into this class
   QSharedDataPointer<QDateTimePrivate> d;

   friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QDateTime &);
   friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QDateTime &);
   friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QDateTime &);
};

Q_DECLARE_TYPEINFO(QDateTime, Q_MOVABLE_TYPE);

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QDate &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QDate &);
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QTime &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QTime &);
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QDateTime &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QDateTime &);

#if ! defined(QT_NO_DATESTRING)
Q_CORE_EXPORT QDebug operator<<(QDebug, const QDate &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const QTime &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const QDateTime &);
#endif

Q_CORE_EXPORT uint qHash(const QDateTime &key, uint seed = 0);
Q_CORE_EXPORT uint qHash(const QDate &key, uint seed = 0);
Q_CORE_EXPORT uint qHash(const QTime &key, uint seed = 0);
#endif
