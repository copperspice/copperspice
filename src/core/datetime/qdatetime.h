/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qnamespace.h>
#include <qshareddatapointer.h>
#include <qstring.h>
#include <qtimezone.h>

#include <chrono>
#include <compare>
#include <limits>
#include <optional>

class QDateTimePrivate;

#ifdef Q_OS_DARWIN

using CFDateRef = const struct __CFDate *;

#ifdef __OBJC__
@class NSDate;

#else
class NSDate;

#endif

#endif

// extra parentheses around min, avoids expanding if it is a macro (MSVC issue)
static constexpr const qint64 INVALID_JD = (std::numeric_limits<qint64>::min)();
static constexpr const qint64 MAX_C_TIME = (std::numeric_limits<time_t>::max)();

static constexpr const qint64 EPOCH_JD   = 2440588;         // julianDayFromDate(1970, 1, 1)

class Q_CORE_EXPORT QDate
{
 public:
   enum MonthNameType {
      DateFormat,
      StandaloneFormat
   };

   constexpr QDate()
      : jd(INVALID_JD)
   {
   }

   QDate(int y, int m, int d);

   [[nodiscard]] QDate addDays(qint64 days) const;
   [[nodiscard]] QDate addMonths(qint64 months) const;
   [[nodiscard]] QDate addYears(qint64 years) const;

   [[nodiscard]] QDate addDuration(std::chrono::days days) const {
      return addDays(days.count());
   }

   qint64 daysTo(const QDate &value) const;

   int day() const;
   int dayOfWeek() const;
   int dayOfYear() const;
   int daysInMonth() const;
   int daysInYear() const;

   QDateTime endOfDay(const QTimeZone &zone = default_tz()) const;

   void getDate(int *year, int *month, int *day) const;

   bool isNull() const {
      return ! isValid();
   }

   bool isValid() const {
      return jd >= minJd() && jd <= maxJd();
   }

   int month() const;
   int year() const;
   int weekNumber(int *yearNumber = nullptr) const;

   static QString longMonthName(int month, MonthNameType type  = DateFormat);
   static QString longDayName(int weekday, MonthNameType type  = DateFormat);
   static QString shortMonthName(int month, MonthNameType type = DateFormat);
   static QString shortDayName(int weekday, MonthNameType type = DateFormat);

   constexpr qint64 toJulianDay() const {
      return jd;
   }

   QString toString(Qt::DateFormat format = Qt::TextDate) const;
   QString toString(const QString &format) const;

   bool setDate(int year, int month, int day);

   QDateTime startOfDay(const QTimeZone &zone = default_tz()) const;

   std::chrono::sys_days toStdSysDays() const {
      const QDate epoch(EPOCH_JD);
      return std::chrono::sys_days(std::chrono::days(epoch.daysTo(*this)));
   }

   auto operator<=>(const QDate &other) const = default;

   static QDate currentDate();
   static const QTimeZone &default_tz();

   static QDate fromStdSysDays(const std::chrono::sys_days &days) {
      const QDate epoch(EPOCH_JD);
      return epoch.addDays(days.time_since_epoch().count());
   }

   static QDate fromString(const QString &str, Qt::DateFormat format = Qt::TextDate);
   static QDate fromString(const QString &str, const QString &format);

   static bool isValid(int year, int month, int day);
   static bool isLeapYear(int year);

   static constexpr QDate fromJulianDay(qint64 dayNumber) {
      return dayNumber >= minJd() && dayNumber <= maxJd() ? QDate(dayNumber) : QDate();
   }

 private:
   explicit constexpr QDate(qint64 julianDay)
      : jd(julianDay)
   {
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
   {
   }

 public:
   constexpr QTime()
      : mds(NullTime)
   {
   }

   QTime(int h, int m, int s = 0, int ms = 0);

   [[nodiscard]] QTime addSecs(int seconds) const;
   [[nodiscard]] QTime addMSecs(int msecs) const;


   int elapsed() const;

   int hour() const;
   int minute() const;
   int second() const;
   int msec() const;

   constexpr bool isNull() const {
      return mds == NullTime;
   }

   bool isValid() const;

   constexpr int msecsSinceStartOfDay() const {
      return mds == NullTime ? 0 : mds;
   }

   int msecsTo(const QTime &value) const;
   int restart();

   int secsTo(const QTime &value) const;
   bool setHMS(int h, int m, int s, int ms = 0);

   void start();

   QString toString(Qt::DateFormat format = Qt::TextDate) const;
   QString toString(const QString &format) const;

   auto operator<=>(const QTime &other) const = default;

   static constexpr QTime fromMSecsSinceStartOfDay(int msecs) {
      return QTime(msecs);
   }

   static QTime currentTime();

   static QTime fromString(const QString &str, Qt::DateFormat format = Qt::TextDate);
   static QTime fromString(const QString &str, const QString &format);

   static bool isValid(int h, int m, int s, int ms = 0);

 private:
   static constexpr const int NullTime = -1;

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
   QDateTime(const QDate &date, const QTime &time, std::optional<QTimeZone> timeZone = std::nullopt);

   QDateTime(const QDateTime &other);
   QDateTime(QDateTime &&other);

   ~QDateTime();

   QDateTime &operator=(const QDateTime &other);

   QDateTime &operator=(QDateTime &&other) {
      swap(other);
      return *this;
   }

   QDateTime addDuration(std::chrono::milliseconds msecs) const;

   [[nodiscard]] QDateTime addDays(qint64 days) const;
   [[nodiscard]] QDateTime addMonths(qint64 months) const;
   [[nodiscard]] QDateTime addYears(qint64 years) const;
   [[nodiscard]] QDateTime addSecs(qint64 seconds) const;
   [[nodiscard]] QDateTime addMSecs(qint64 msecs) const;

   QDate date() const;
   QTime time() const;

   qint64 daysTo(const QDateTime &value) const;
   qint64 msecsTo(const QDateTime &value) const;
   qint64 secsTo(const QDateTime &value) const;

   bool isDaylightTime() const;
   bool isNull() const;
   bool isValid() const;

   int offsetFromUtc() const;

   void setDate(const QDate &date);
   void setTime(const QTime &time);

   void setTimeZone(const QTimeZone &toZone);

   void setMSecsSinceEpoch(qint64 msecs);
   void setSecsSinceEpoch(qint64 seconds);

   void setTime_t(quint64 seconds);

   void swap(QDateTime &other) {
      d.swap(other.d);
   }

   QTimeZone timeRepresentation() const;
   QTimeZone timeZone() const;
   QString timeZoneAbbreviation() const;

   QDateTime toLocalTime() const;
   qint64 toMSecsSinceEpoch() const;
   qint64 toSecsSinceEpoch() const;

   std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> toStdSysMilliseconds() const {
      return std::chrono::time_point<std::chrono::system_clock,
            std::chrono::milliseconds>(std::chrono::milliseconds(toMSecsSinceEpoch()));
   }

   std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> toStdSysSeconds() const {
      return std::chrono::time_point<std::chrono::system_clock,
            std::chrono::seconds>(std::chrono::seconds(toSecsSinceEpoch()));
   }

   QDateTime toOffsetFromUtc(qint64 offsetSeconds) const;

   QString toString(Qt::DateFormat format = Qt::TextDate) const;
   QString toString(const QString &format) const;

   quint64 toTime_t() const;
   QDateTime toTimeZone(const QTimeZone &timeZone) const;

   QDateTime toUTC() const;

   bool operator==(const QDateTime &other) const;
   bool operator<(const QDateTime &other) const;

   bool operator!=(const QDateTime &other) const {
      return ! (*this == other);
   }

   bool operator<=(const QDateTime &other) const {
      return ! (other < *this);
   }

   bool operator>(const QDateTime &other) const {
      return other < *this;
   }

   bool operator>=(const QDateTime &other) const {
      return ! (*this < other);
   }

   static QDateTime currentDateTime(const QTimeZone &zone = QDate::default_tz());
   static QDateTime currentDateTimeUtc();

   static QDateTime fromMSecsSinceEpoch(qint64 msecs, const QTimeZone &timeZone = QDate::default_tz());
   static QDateTime fromSecsSinceEpoch(qint64 seconds, const QTimeZone &timeZone = QDate::default_tz());

   static qint64 currentMSecsSinceEpoch();
   static qint64 currentSecsSinceEpoch();

   static QDateTime fromTime_t(qint64 seconds, const QTimeZone &timeZone = QDate::default_tz());

   static QDateTime fromString(const QString &str, Qt::DateFormat format = Qt::TextDate);
   static QDateTime fromString(const QString &str, const QString &format);

   static QDateTime fromStdLocalTime(const std::chrono::local_time<std::chrono::milliseconds> &msecs) {
      QDateTime retval(QDate(1970, 1, 1), QTime(0, 0, 0));
      return retval.addMSecs(msecs.time_since_epoch().count());
   }

#if defined(Q_OS_DARWIN)
   static QDateTime fromCFDate(CFDateRef date);
   CFDateRef toCFDate() const;

#if defined(__OBJC__)
   static QDateTime fromNSDate(const NSDate *date);
   NSDate *toNSDate() const;
#endif

#endif

 private:
   QUniquePointer<QDateTimePrivate> d;

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
