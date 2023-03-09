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

#ifndef QDATETIME_H
#define QDATETIME_H

#include <qstring.h>
#include <qnamespace.h>
#include <qshareddatapointer.h>

#include <limits>

class QTimeZone;
class QDateTimePrivate;

#ifdef Q_OS_DARWIN

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

   constexpr QDate()
      : jd(nullJd())
   {
   }

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

   QString toString(Qt::DateFormat format = Qt::TextDate) const;
   QString toString(const QString &format) const;

   bool setDate(int year, int month, int day);
   void getDate(int *year, int *month, int *day);

   [[nodiscard]] QDate addDays(qint64 days) const;
   [[nodiscard]] QDate addMonths(qint64 months) const;
   [[nodiscard]] QDate addYears(qint64 years) const;
   qint64 daysTo(const QDate &value) const;

   bool operator==(const QDate &value) const {
      return jd == value.jd;
   }

   bool operator!=(const QDate &value) const {
      return jd != value.jd;
   }

   bool operator<(const QDate &value) const {
      return jd < value.jd;
   }

   bool operator<=(const QDate &value) const {
      return jd <= value.jd;
   }

   bool operator>(const QDate &value) const {
      return jd > value.jd;
   }

   bool operator>=(const QDate &value) const {
      return jd >= value.jd;
   }

   static QDate currentDate();

   static QDate fromString(const QString &str, Qt::DateFormat format = Qt::TextDate);
   static QDate fromString(const QString &str, const QString &format);

   static bool isValid(int year, int month, int day);
   static bool isLeapYear(int year);

   static constexpr QDate fromJulianDay(qint64 dayNumber) {
      return dayNumber >= minJd() && dayNumber <= maxJd() ? QDate(dayNumber) : QDate();
   }

   constexpr qint64 toJulianDay() const {
      return jd;
   }

 private:
   explicit constexpr QDate(qint64 julianDay)
      : jd(julianDay)
   {
   }

   // extra parentheses around min, avoids expanding if it is a macro (MSVC issue)
   static constexpr qint64 nullJd() {
      return (std::numeric_limits<qint64>::min)();
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

   friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QDate &date);
   friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QDate &date);
};

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

   QString toString(Qt::DateFormat format = Qt::TextDate) const;
   QString toString(const QString &format) const;

   bool setHMS(int h, int m, int s, int ms = 0);

   [[nodiscard]] QTime addSecs(int secs) const;
   int secsTo(const QTime &value) const;

   [[nodiscard]] QTime addMSecs(int ms) const;
   int msecsTo(const QTime &value) const;

   bool operator==(const QTime &value) const {
      return mds == value.mds;
   }

   bool operator!=(const QTime &value) const {
      return mds != value.mds;
   }

   bool operator<(const QTime &value) const {
      return mds < value.mds;
   }

   bool operator<=(const QTime &value) const {
      return mds <= value.mds;
   }

   bool operator>(const QTime &value) const {
      return mds > value.mds;
   }

   bool operator>=(const QTime &value) const {
      return mds >= value.mds;
   }

   static QTime fromMSecsSinceStartOfDay(int msecs) {
      return QTime(msecs);
   }

   int msecsSinceStartOfDay() const {
      return mds == NullTime ? 0 : mds;
   }

   static QTime currentTime();

   static QTime fromString(const QString &str, Qt::DateFormat format = Qt::TextDate);
   static QTime fromString(const QString &str, const QString &format);

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

   friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QTime &time);
   friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QTime &time);
};

class Q_CORE_EXPORT QDateTime
{
 public:
   QDateTime();
   explicit QDateTime(const QDate &date);
   QDateTime(const QDate &date, const QTime &time, Qt::TimeSpec spec = Qt::LocalTime, int offsetSeconds = 0);
   QDateTime(const QDate &date, const QTime &time, const QTimeZone &timeZone);

   QDateTime(const QDateTime &other);
   QDateTime(QDateTime &&other);

   ~QDateTime();

   QDateTime &operator=(const QDateTime &other);

   QDateTime &operator=(QDateTime &&other) {
      swap(other);
      return *this;
   }

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
   void setTime_t(quint64 seconds);

   QString toString(Qt::DateFormat format = Qt::TextDate) const;
   QString toString(const QString &format) const;

   [[nodiscard]] QDateTime addDays(qint64 days) const;
   [[nodiscard]] QDateTime addMonths(qint64 months) const;
   [[nodiscard]] QDateTime addYears(qint64 years) const;
   [[nodiscard]] QDateTime addSecs(qint64 secs) const;
   [[nodiscard]] QDateTime addMSecs(qint64 msecs) const;

   QDateTime toTimeSpec(Qt::TimeSpec specification) const;

   QDateTime toLocalTime() const {
      return toTimeSpec(Qt::LocalTime);
   }

   QDateTime toUTC() const {
      return toTimeSpec(Qt::UTC);
   }

   QDateTime toOffsetFromUtc(qint64 offsetSeconds) const;
   QDateTime toTimeZone(const QTimeZone &timeZone) const;

   qint64 daysTo(const QDateTime &value) const;
   qint64 secsTo(const QDateTime &value) const;
   qint64 msecsTo(const QDateTime &value) const;

   bool operator==(const QDateTime &value) const;
   bool operator!=(const QDateTime &value) const {
      return !(*this == value);
   }

   bool operator<(const QDateTime &value) const;
   bool operator<=(const QDateTime &value) const {
      return !(value < *this);
   }

   bool operator>(const QDateTime &value) const {
      return value < *this;
   }

   bool operator>=(const QDateTime &value) const {
      return !(*this < value);
   }

   static QDateTime currentDateTime();
   static QDateTime currentDateTimeUtc();

   static QDateTime fromString(const QString &str, Qt::DateFormat format = Qt::TextDate);
   static QDateTime fromString(const QString &str, const QString &format);

   static QDateTime fromTime_t(quint64 seconds, Qt::TimeSpec spec = Qt::LocalTime, int offsetFromUtc = 0);
   static QDateTime fromTime_t(quint64 seconds, const QTimeZone &timeZone);

   static QDateTime fromMSecsSinceEpoch(qint64 msecs, Qt::TimeSpec spec = Qt::LocalTime, int offsetFromUtc = 0);
   static QDateTime fromMSecsSinceEpoch(qint64 msecs, const QTimeZone &timeZone);
   static qint64 currentMSecsSinceEpoch();

#if defined(Q_OS_DARWIN)
   static QDateTime fromCFDate(CFDateRef date);
   CFDateRef toCFDate() const;

#if defined(__OBJC__)
   static QDateTime fromNSDate(const NSDate *date);
   NSDate *toNSDate() const;
#endif

#endif

 private:
   QSharedDataPointer<QDateTimePrivate> d;

   friend class QDateTimePrivate;

   friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QDateTime &dateTime);
   friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QDateTime &dateTime);
   friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QDateTime &data);
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QDate &date);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QDate &date);
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QTime &time);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QTime &time);
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QDateTime &dateTime);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QDateTime &dateTime);

Q_CORE_EXPORT QDebug operator<<(QDebug, const QDate &data);
Q_CORE_EXPORT QDebug operator<<(QDebug, const QTime &data);
Q_CORE_EXPORT QDebug operator<<(QDebug, const QDateTime &value);

Q_CORE_EXPORT uint qHash(const QDateTime &key, uint seed = 0);
Q_CORE_EXPORT uint qHash(const QDate &key, uint seed = 0);
Q_CORE_EXPORT uint qHash(const QTime &key, uint seed = 0);

#endif
