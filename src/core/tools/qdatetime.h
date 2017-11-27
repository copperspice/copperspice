/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

class QDebug;

class Q_CORE_EXPORT QDate
{
 public:
   enum MonthNameType {
      DateFormat = 0,
      StandaloneFormat
   };

   QDate() {
      jd = 0;
   }
   QDate(int y, int m, int d);

   bool isNull() const {
      return jd == 0;
   }
   bool isValid() const;

   int year() const;
   int month() const;
   int day() const;
   int dayOfWeek() const;
   int dayOfYear() const;
   int daysInMonth() const;
   int daysInYear() const;
   int weekNumber(int *yearNum = 0) const;

#ifndef QT_NO_TEXTDATE
   // ### Qt5/merge these functions
   static QString shortMonthName(int month);
   static QString shortMonthName(int month, MonthNameType type);
   static QString shortDayName(int weekday);
   static QString shortDayName(int weekday, MonthNameType type);
   static QString longMonthName(int month);
   static QString longMonthName(int month, MonthNameType type);
   static QString longDayName(int weekday);
   static QString longDayName(int weekday, MonthNameType type);
#endif

#ifndef QT_NO_DATESTRING
   QString toString(Qt::DateFormat f = Qt::TextDate) const;
   QString toString(const QString &format) const;
#endif

   bool setYMD(int y, int m, int d);
   bool setDate(int year, int month, int day);

   void getDate(int *year, int *month, int *day);

   QDate addDays(int days) const;
   QDate addMonths(int months) const;
   QDate addYears(int years) const;
   int daysTo(const QDate &) const;

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

   // ### Qt5/remove these two functions
   static uint gregorianToJulian(int y, int m, int d);
   static void julianToGregorian(uint jd, int &y, int &m, int &d);

   static inline QDate fromJulianDay(int jd) {
      QDate d;
      d.jd = jd;
      return d;
   }
   inline int toJulianDay() const {
      return jd;
   }

 private:
   static inline qint64 nullJd() {
      return std::numeric_limits<qint64>::lowest();
   }
   static inline qint64 minJd() {
      return Q_INT64_C(-784350574879);
   }
   static inline qint64 maxJd() {
      return Q_INT64_C( 784354017364);
   }

   qint64 jd;

   friend class QDateTime;
   friend class QDateTimePrivate;

#ifndef QT_NO_DATASTREAM
   friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QDate &);
   friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QDate &);
#endif

};
Q_DECLARE_TYPEINFO(QDate, Q_MOVABLE_TYPE);

class Q_CORE_EXPORT QTime
{
 public:
   QTime()
      : mds(NullTime) {
   }

   QTime(int h, int m, int s = 0, int ms = 0);

   bool isNull() const {
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
   inline int ds() const {
      return mds == -1 ? 0 : mds;
   }
   int mds;

   friend class QDateTime;
   friend class QDateTimePrivate;

#ifndef QT_NO_DATASTREAM
   friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QTime &);
   friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QTime &);
#endif

};
Q_DECLARE_TYPEINFO(QTime, Q_MOVABLE_TYPE);

class QDateTimePrivate;

class Q_CORE_EXPORT QDateTime
{
 public:
   QDateTime();
   explicit QDateTime(const QDate &);
   QDateTime(const QDate &, const QTime &, Qt::TimeSpec spec = Qt::LocalTime);

   // ### Qt5: Merge with above with default offsetSeconds = 0
   QDateTime(const QDate &date, const QTime &time, Qt::TimeSpec spec, int offsetSeconds);
   QDateTime(const QDateTime &other);
   ~QDateTime();

   QDateTime &operator=(const QDateTime &other);

   bool isNull() const;
   bool isValid() const;

   QDate date() const;
   QTime time() const;
   Qt::TimeSpec timeSpec() const;
   int offsetFromUtc() const;

   qint64 toMSecsSinceEpoch() const;
   uint toTime_t() const;

   void setDate(const QDate &date);
   void setTime(const QTime &time);
   void setTimeSpec(Qt::TimeSpec spec);
   void setOffsetFromUtc(int offsetSeconds);
   void setMSecsSinceEpoch(qint64 msecs);
   void setTime_t(uint secs);

#ifndef QT_NO_DATESTRING
   QString toString(Qt::DateFormat f = Qt::TextDate) const;
   QString toString(const QString &format) const;
#endif

   QDateTime addDays(int days) const;
   QDateTime addMonths(int months) const;
   QDateTime addYears(int years) const;
   QDateTime addSecs(int secs) const;
   QDateTime addMSecs(qint64 msecs) const;

   QDateTime toTimeSpec(Qt::TimeSpec spec) const;
   inline QDateTime toLocalTime() const {
      return toTimeSpec(Qt::LocalTime);
   }

   inline QDateTime toUTC() const {
      return toTimeSpec(Qt::UTC);
   }
   QDateTime toOffsetFromUtc(int offsetSeconds) const;

   qint64 daysTo(const QDateTime &) const;
   qint64 secsTo(const QDateTime &) const;
   qint64 msecsTo(const QDateTime &) const;

   bool operator==(const QDateTime &other) const;
   inline bool operator!=(const QDateTime &other) const {
      return !(*this == other);
   }

   bool operator<(const QDateTime &other) const;
   inline bool operator<=(const QDateTime &other) const {
      return !(other < *this);
   }

   inline bool operator>(const QDateTime &other) const {
      return other < *this;
   }

   inline bool operator>=(const QDateTime &other) const {
      return !(*this < other);
   }

   void setUtcOffset(int seconds);
   int utcOffset() const;

   static QDateTime currentDateTime();
   static QDateTime currentDateTimeUtc();

#ifndef QT_NO_DATESTRING
   static QDateTime fromString(const QString &s, Qt::DateFormat f = Qt::TextDate);
   static QDateTime fromString(const QString &s, const QString &format);
#endif
   static QDateTime fromTime_t(uint secs);

   // ### Qt5: Merge with above with default spec = Qt::LocalTime
   static QDateTime fromTime_t(uint secs, Qt::TimeSpec spec, int offsetFromUtc = 0);
   static QDateTime fromMSecsSinceEpoch(qint64 msecs);

   // ### Qt5: Merge with above with default spec = Qt::LocalTime
   static QDateTime fromMSecsSinceEpoch(qint64 msecs, Qt::TimeSpec spec, int offsetFromUtc = 0);
   static qint64 currentMSecsSinceEpoch();

 private:
   friend class QDateTimePrivate;
   void detach();
   QExplicitlySharedDataPointer<QDateTimePrivate> d;

#ifndef QT_NO_DATASTREAM
   friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QDateTime &);
   friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QDateTime &);
#endif
};

Q_DECLARE_TYPEINFO(QDateTime, Q_MOVABLE_TYPE);

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QDate &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QDate &);
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QTime &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QTime &);
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QDateTime &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QDateTime &);
#endif

#if ! defined(QT_NO_DATESTRING)
Q_CORE_EXPORT QDebug operator<<(QDebug, const QDate &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const QTime &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const QDateTime &);
#endif


#endif // QDATETIME_H
