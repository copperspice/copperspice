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

#include <qdatetime.h>

#include <qplatformdefs.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qset.h>
#include <qregularexpression.h>
#include <qstringlist.h>
#include <qstringparser.h>

#include <qdatetime_p.h>
#include <qdatetimeparser_p.h>
#include <qtimezone_p.h>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#else
#include <locale.h>
#endif

#if defined(Q_OS_DARWIN)
#include <qcore_mac_p.h>
#endif

#include <cmath>
#include <time.h>

enum {
   SECS_PER_DAY    = 86400,
   MSECS_PER_DAY   = 86400000,
   SECS_PER_HOUR   = 3600,
   MSECS_PER_HOUR  = 3600000,
   SECS_PER_MIN    = 60,
   MSECS_PER_MIN   = 60000,
   TIME_T_MAX      = 2145916799,  // int maximum 2037-12-31T23:59:59 UTC
   JULIAN_DAY_FOR_EPOCH = 2440588 // result of julianDayFromDate(1970, 1, 1)
};

static inline QDate fixedDate(int y, int m, int d)
{
   QDate result(y, m, 1);
   result.setDate(y, m, qMin(d, result.daysInMonth()));
   return result;
}

static inline qint64 floordiv(qint64 a, int b)
{
   return (a - (a < 0 ? b - 1 : 0)) / b;
}

static inline int floordiv(int a, int b)
{
   return (a - (a < 0 ? b - 1 : 0)) / b;
}

static inline qint64 julianDayFromDate(int year, int month, int day)
{
   // Adjust for no year 0
   if (year < 0) {
      ++year;
   }

   /*
    * Math from The Calendar FAQ at http://www.tondering.dk/claus/cal/julperiod.php
    * This formula is correct for all julian days, when using mathematical integer
    * division (round to negative infinity), not c++11 integer division (round to zero)
    */
   int    a = floordiv(14 - month, 12);
   qint64 y = (qint64)year + 4800 - a;
   int    m = month + 12 * a - 3;
   return day + floordiv(153 * m + 2, 5) + 365 * y + floordiv(y, 4) - floordiv(y, 100) + floordiv(y, 400) - 32045;
}

struct ParsedDate {
   int year, month, day;
};

static ParsedDate getDateFromJulianDay(qint64 julianDay)
{
   /*
    * Math from The Calendar FAQ at http://www.tondering.dk/claus/cal/julperiod.php
    * This formula is correct for all julian days, when using mathematical integer
    * division (round to negative infinity), not c++11 integer division (round to zero)
    */
   qint64 a = julianDay + 32044;
   qint64 b = floordiv(4 * a + 3, 146097);
   int    c = a - floordiv(146097 * b, 4);

   int    d = floordiv(4 * c + 3, 1461);
   int    e = c - floordiv(1461 * d, 4);
   int    m = floordiv(5 * e + 2, 153);

   int    day = e - floordiv(153 * m + 2, 5) + 1;
   int    month = m + 3 - 12 * floordiv(m, 10);
   int    year = 100 * b + d - 4800 + floordiv(m, 10);

   // Adjust for no year 0
   if (year <= 0) {
      --year ;
   }

   const ParsedDate result = { year, month, day };
   return result;
}

static const char monthDays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#ifndef QT_NO_TEXTDATE

static const QString qt_shortMonthNames[] = {
   "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static int qt_monthNumberFromShortName(QStringView shortName)
{
   for (unsigned int i = 0; i < sizeof(qt_shortMonthNames) / sizeof(qt_shortMonthNames[0]); ++i) {
      if (shortName == qt_shortMonthNames[i]) {
         return i + 1;
      }
   }

   return -1;
}

static int fromShortMonthName(QStringView monthName)
{
   // Assume that English monthnames are the default
   int month = qt_monthNumberFromShortName(monthName);
   if (month != -1) {
      return month;
   }
   // If English names can't be found, search the localized ones
   for (int i = 1; i <= 12; ++i) {
      if (monthName == QDate::shortMonthName(i)) {
         return i;
      }
   }
   return -1;
}
#endif // QT_NO_TEXTDATE

struct ParsedRfcDateTime {
   QDate date;
   QTime time;
   int utcOffset;
};

static ParsedRfcDateTime rfcDateImpl(const QString &s)
{
   ParsedRfcDateTime result;

   // Matches "Wdy, DD Mon YYYY HH:mm:ss ±hhmm" (Wdy, being optional)
   static QRegularExpression
   regexp("^(?:[A-Z][a-z]+,)?[ \\t]*(\\d{1,2})[ \\t]+([A-Z][a-z]+)[ \\t]+(\\d\\d\\d\\d)(?:[ \\t]+(\\d\\d):(\\d\\d)(?::(\\d\\d))?)?[ \\t]*(?:([+-])(\\d\\d)(\\d\\d))?");
   QRegularExpressionMatch match = regexp.match(s);

   if (match.hasMatch() && match.capturedStart() == s.begin()) {
      const QStringList cap = match.capturedTexts();
      result.date = QDate(cap[3].toInteger<int>(), qt_monthNumberFromShortName(cap[2]), cap[1].toInteger<int>());

      if (! cap[4].isEmpty()) {
         result.time = QTime(cap[4].toInteger<int>(), cap[5].toInteger<int>(), cap[6].toInteger<int>());
      }

      const bool positiveOffset = (cap[7] == "+");
      const int hourOffset      = cap[8].toInteger<int>();
      const int minOffset       = cap[9].toInteger<int>();
      result.utcOffset          = ((hourOffset * 60 + minOffset) * (positiveOffset ? 60 : -60));

   } else {
      // Matches "Wdy Mon DD HH:mm:ss YYYY"
      static QRegularExpression
      regexp("^[A-Z][a-z]+[ \\t]+([A-Z][a-z]+)[ \\t]+(\\d\\d)(?:[ \\t]+(\\d\\d):(\\d\\d):(\\d\\d))?[ \\t]+(\\d\\d\\d\\d)[ \\t]*(?:([+-])(\\d\\d)(\\d\\d))?");
      QRegularExpressionMatch match = regexp.match(s);

      if (match.hasMatch() && match.capturedStart() == s.begin()) {
         const QStringList cap = match.capturedTexts();

         result.date = QDate(cap[6].toInteger<int>(), qt_monthNumberFromShortName(cap[1]), cap[2].toInteger<int>());

         if (! cap[3].isEmpty()) {
            result.time = QTime(cap[3].toInteger<int>(), cap[4].toInteger<int>(), cap[5].toInteger<int>());
         }

         const bool positiveOffset = (cap[7] == "+");
         const int hourOffset      = cap[8].toInteger<int>();
         const int minOffset       = cap[9].toInteger<int>();
         result.utcOffset          = ((hourOffset * 60 + minOffset) * (positiveOffset ? 60 : -60));
      }
   }

   return result;
}

// Return offset in [+-]HH:mm format
static QString toOffsetString(Qt::DateFormat format, int offset)
{
   QString result;

   if (format == Qt::TextDate) {
      result = QString("%1%2%3");

   } else {
      result = QString("%1%2:%3");
   }

   return result.formatArg(offset >= 0 ? '+' : '-')
      .formatArg(qAbs(offset) / SECS_PER_HOUR, 2, 10, '0')
      .formatArg((offset / 60) % 60, 2, 10, '0');
}

// Parse offset in [+-]HH[[:]mm] format
static int fromOffsetString(QStringView offsetString, bool *valid)
{
   *valid = false;

   const int size = offsetString.size();
   if (size < 2 || size > 6) {
      return 0;
   }

   // sign will be +1 for a positive and -1 for a negative offset
   int sign;

   // First char must be + or -
   const QChar signChar = offsetString.at(0);
   if (signChar == '+') {
      sign = 1;
   } else if (signChar == '-') {
      sign = -1;
   } else {
      return 0;
   }

   // Split the hour and minute parts
   QList<QStringView> parts = QStringParser::split(offsetString.mid(1), ':');

   if (parts.count() == 1) {
      // [+-]HHmm or [+-]HH format
      parts.append(parts.first().mid(2));
      parts[0] = parts.first().left(2);
   }

   bool ok = false;
   const int hour = QStringParser::toInteger<int>(parts.at(0), &ok);
   if (!ok) {
      return 0;
   }

   const int minute = QStringParser::toInteger<int>(parts.at(1), &ok);
   if (!ok || minute < 0 || minute > 59) {
      return 0;
   }

   *valid = true;
   return sign * ((hour * 60) + minute) * 60;
}

QDate::QDate(int y, int m, int d)
{
   setDate(y, m, d);
}

int QDate::year() const
{
   if (isNull()) {
      return 0;
   }

   return getDateFromJulianDay(jd).year;
}

int QDate::month() const
{
   if (isNull()) {
      return 0;
   }

   return getDateFromJulianDay(jd).month;
}

int QDate::day() const
{
   if (isNull()) {
      return 0;
   }

   return getDateFromJulianDay(jd).day;
}


int QDate::dayOfWeek() const
{
   if (isNull()) {
      return 0;
   }

   if (jd >= 0) {
      return (jd % 7) + 1;
   } else {
      return ((jd + 1) % 7) + 7;
   }
}

int QDate::dayOfYear() const
{
   if (isNull()) {
      return 0;
   }

   return jd - julianDayFromDate(year(), 1, 1) + 1;
}

int QDate::daysInMonth() const
{
   if (isNull()) {
      return 0;
   }

   const ParsedDate pd = getDateFromJulianDay(jd);

   if (pd.month == 2 && isLeapYear(pd.year)) {
      return 29;
   } else {
      return monthDays[pd.month];
   }
}

int QDate::daysInYear() const
{
   if (isNull()) {
      return 0;
   }

   return isLeapYear(getDateFromJulianDay(jd).year) ? 366 : 365;
}

int QDate::weekNumber(int *yearNumber) const
{
   if (!isValid()) {
      return 0;
   }

   int year = QDate::year();
   int yday = dayOfYear();
   int wday = dayOfWeek();

   int week = (yday - wday + 10) / 7;

   if (week == 0) {
      // last week of previous year
      --year;

      week = (yday + 365 + (QDate::isLeapYear(year) ? 1 : 0) - wday + 10) / 7;
      Q_ASSERT(week == 52 || week == 53);

   } else if (week == 53) {
      // maybe first week of next year
      int w = (yday - 365 - (QDate::isLeapYear(year) ? 1 : 0) - wday + 10) / 7;
      if (w > 0) {
         ++year;
         week = w;
      }
      Q_ASSERT(week == 53 || week == 1);
   }

   if (yearNumber != nullptr) {
      *yearNumber = year;
   }
   return week;
}

#ifndef QT_NO_TEXTDATE
QString QDate::shortMonthName(int month, QDate::MonthNameType type)
{
   if (month >= 1 && month <= 12) {
      switch (type) {
         case QDate::DateFormat:
            return QLocale::system().monthName(month, QLocale::ShortFormat);

         case QDate::StandaloneFormat:
            return QLocale::system().standaloneMonthName(month, QLocale::ShortFormat);
      }
   }

   return QString();
}

QString QDate::longMonthName(int month, MonthNameType type)
{
   if (month >= 1 && month <= 12) {
      switch (type) {
         case QDate::DateFormat:
            return QLocale::system().monthName(month, QLocale::LongFormat);

         case QDate::StandaloneFormat:
            return QLocale::system().standaloneMonthName(month, QLocale::LongFormat);
      }
   }

   return QString();
}

QString QDate::shortDayName(int weekday, MonthNameType type)
{
   if (weekday >= 1 && weekday <= 7) {
      switch (type) {
         case QDate::DateFormat:
            return QLocale::system().dayName(weekday, QLocale::ShortFormat);

         case QDate::StandaloneFormat:
            return QLocale::system().standaloneDayName(weekday, QLocale::ShortFormat);
      }
   }
   return QString();
}

QString QDate::longDayName(int weekday, MonthNameType type)
{
   if (weekday >= 1 && weekday <= 7) {
      switch (type) {
         case QDate::DateFormat:
            return QLocale::system().dayName(weekday, QLocale::LongFormat);
         case QDate::StandaloneFormat:
            return QLocale::system().standaloneDayName(weekday, QLocale::LongFormat);
      }
   }

   return QString();
}
#endif

#ifndef QT_NO_TEXTDATE
static QString toStringTextDate(QDate date)
{
   const ParsedDate pd = getDateFromJulianDay(date.toJulianDay());
   static const QChar sp(' ');

   return date.shortDayName(date.dayOfWeek()) + sp
      + date.shortMonthName(pd.month) + sp + QString::number(pd.day) + sp + QString::number(pd.year);
}
#endif

static QString toStringIsoDate(qint64 jd)
{
   const ParsedDate pd = getDateFromJulianDay(jd);

   if (pd.year >= 0 && pd.year <= 9999) {
      QString result = QString("%1-%2-%3");

      return result.formatArg(pd.year, 4, 10, '0')
         .formatArg(pd.month, 2, 10, '0').formatArg(pd.day, 2, 10, '0');

   } else {
      return QString();
   }
}

QString QDate::toString(Qt::DateFormat format) const
{
   if (! isValid()) {
      return QString();
   }

   switch (format) {
      case Qt::SystemLocaleDate:
      case Qt::SystemLocaleShortDate:
         return QLocale::system().toString(*this, QLocale::ShortFormat);

      case Qt::SystemLocaleLongDate:
         return QLocale::system().toString(*this, QLocale::LongFormat);

      case Qt::LocaleDate:
      case Qt::DefaultLocaleShortDate:
         return QLocale().toString(*this, QLocale::ShortFormat);

      case Qt::DefaultLocaleLongDate:
         return QLocale().toString(*this, QLocale::LongFormat);

      case Qt::RFC2822Date:
         return QLocale::c().toString(*this, "dd MMM yyyy");

      default:

#ifndef QT_NO_TEXTDATE
      case Qt::TextDate:
         return toStringTextDate(*this);
#endif

      case Qt::ISODate:
         return toStringIsoDate(jd);
   }
}

QString QDate::toString(const QString &format) const
{
   return QLocale::system().toString(*this, format);
}

bool QDate::setDate(int year, int month, int day)
{
   if (isValid(year, month, day)) {
      jd = julianDayFromDate(year, month, day);

   } else {
      jd = nullJd();
   }

   return isValid();
}

void QDate::getDate(int *year, int *month, int *day)
{
   ParsedDate pd = { 0, 0, 0 };
   if (isValid()) {
      pd = getDateFromJulianDay(jd);
   }

   if (year) {
      *year = pd.year;
   }

   if (month) {
      *month = pd.month;
   }

   if (day) {
      *day = pd.day;
   }
}

QDate QDate::addDays(qint64 ndays) const
{
   if (isNull()) {
      return QDate();
   }

   // Due to limits on minJd() and maxJd() we know that any overflow
   // will be invalid and caught by fromJulianDay().
   return fromJulianDay(jd + ndays);
}

QDate QDate::addMonths(qint64 nmonths) const
{
   if (! isValid()) {
      return QDate();
   }
   if (! nmonths) {
      return *this;
   }

   int old_y, y, m, d;
   {
      const ParsedDate pd = getDateFromJulianDay(jd);
      y = pd.year;
      m = pd.month;
      d = pd.day;
   }

   old_y = y;

   bool increasing = nmonths > 0;

   while (nmonths != 0) {
      if (nmonths < 0 && nmonths + 12 <= 0) {
         y--;
         nmonths += 12;

      } else if (nmonths < 0) {
         m += nmonths;
         nmonths = 0;
         if (m <= 0) {
            --y;
            m += 12;
         }

      } else if (nmonths - 12 >= 0) {
         y++;
         nmonths -= 12;

      } else if (m == 12) {
         y++;
         m = 0;

      } else {
         m += nmonths;
         nmonths = 0;

         if (m > 12) {
            ++y;
            m -= 12;
         }
      }
   }

   // was there a sign change?
   if ((old_y > 0 && y <= 0) ||
      (old_y < 0 && y >= 0))
      // yes, adjust the date by +1 or -1 years
   {
      y += increasing ? +1 : -1;
   }

   return fixedDate(y, m, d);
}

QDate QDate::addYears(qint64 nyears) const
{
   if (!isValid()) {
      return QDate();
   }

   ParsedDate pd = getDateFromJulianDay(jd);

   int old_y = pd.year;
   pd.year += nyears;

   // was there a sign change?
   if ((old_y > 0 && pd.year <= 0) || (old_y < 0 && pd.year >= 0)) {
      // yes, adjust the date by +1 or -1 years
      pd.year += nyears > 0 ? +1 : -1;
   }

   return fixedDate(pd.year, pd.month, pd.day);
}

qint64 QDate::daysTo(const QDate &d) const
{
   if (isNull() || d.isNull()) {
      return 0;
   }

   // Due to limits on minJd() and maxJd() we know this will never overflow
   return d.jd - jd;
}

QDate QDate::fromString(const QString &string, Qt::DateFormat format)
{
   if (string.isEmpty()) {
      return QDate();
   }

   switch (format) {
      case Qt::SystemLocaleDate:
      case Qt::SystemLocaleShortDate:
         return QLocale::system().toDate(string, QLocale::ShortFormat);

      case Qt::SystemLocaleLongDate:
         return QLocale::system().toDate(string, QLocale::LongFormat);

      case Qt::LocaleDate:
      case Qt::DefaultLocaleShortDate:
         return QLocale().toDate(string, QLocale::ShortFormat);

      case Qt::DefaultLocaleLongDate:
         return QLocale().toDate(string, QLocale::LongFormat);

      case Qt::RFC2822Date:
         return rfcDateImpl(string).date;

      default:

#ifndef QT_NO_TEXTDATE
      case Qt::TextDate: {
         QList<QStringView> parts = QStringParser::split<QStringView>(string, ' ', QStringParser::SkipEmptyParts);

         if (parts.count() != 4) {
            return QDate();
         }

         QStringView monthName = parts.at(1);

         const int month = fromShortMonthName(monthName);
         if (month == -1) {
            // Month name matches neither English nor other localised name.
            return QDate();
         }

         bool ok = false;
         int year = QStringParser::toInteger<int>(parts.at(3), &ok);

         if (!ok) {
            return QDate();
         }

         return QDate(year, month, QStringParser::toInteger<int>(parts.at(2)));
      }

#endif

      case Qt::ISODate: {
         // Semi-strict parsing, must be long enough and have non-numeric separators
         if (string.size() < 10 || string.at(4).isDigit() || string.at(7).isDigit()
               || (string.size() > 10 && string.at(10).isDigit())) {
            return QDate();
         }

         const int year = QStringParser::toInteger<int>(string.midView(0, 4));

         if (year <= 0 || year > 9999) {
            return QDate();
         }

         return QDate(year, QStringParser::toInteger<int>(string.midView(5, 2)),
               QStringParser::toInteger<int>(string.midView(8, 2)));
      }
   }
   return QDate();
}

QDate QDate::fromString(const QString &string, const QString &format)
{
   QDate date;

   QDateTimeParser dt(QVariant::Date, QDateTimeParser::FromString);
   if (dt.parseFormat(format)) {
      dt.fromString(string, &date, nullptr);
   }

   return date;
}

bool QDate::isValid(int year, int month, int day)
{
   // there is no year 0 in the Gregorian calendar
   if (year == 0) {
      return false;
   }

   return (day > 0 && month > 0 && month <= 12) &&
      (day <= monthDays[month] || (day == 29 && month == 2 && isLeapYear(year)));
}

bool QDate::isLeapYear(int y)
{
   // No year 0 in Gregorian calendar, so -1, -5, -9 etc are leap years
   if ( y < 1) {
      ++y;
   }

   return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

QTime::QTime(int h, int m, int s, int ms)
{
   setHMS(h, m, s, ms);
}

bool QTime::isValid() const
{
   return mds > NullTime && mds < MSECS_PER_DAY;
}

int QTime::hour() const
{
   if (! isValid()) {
      return -1;
   }

   return ds() / MSECS_PER_HOUR;
}

int QTime::minute() const
{
   if (! isValid()) {
      return -1;
   }

   return (ds() % MSECS_PER_HOUR) / MSECS_PER_MIN;
}

int QTime::second() const
{
   if (!isValid()) {
      return -1;
   }

   return (ds() / 1000) % SECS_PER_MIN;
}

int QTime::msec() const
{
   if (!isValid()) {
      return -1;
   }

   return ds() % 1000;
}

QString QTime::toString(Qt::DateFormat format) const
{
   if (! isValid()) {
      return QString();
   }

   switch (format) {
      case Qt::SystemLocaleDate:
      case Qt::SystemLocaleShortDate:
         return QLocale::system().toString(*this, QLocale::ShortFormat);

      case Qt::SystemLocaleLongDate:
         return QLocale::system().toString(*this, QLocale::LongFormat);

      case Qt::LocaleDate:
      case Qt::DefaultLocaleShortDate:
         return QLocale().toString(*this, QLocale::ShortFormat);

      case Qt::DefaultLocaleLongDate:
         return QLocale().toString(*this, QLocale::LongFormat);

      case Qt::RFC2822Date:
      case Qt::ISODate:
      case Qt::TextDate:

      default:
         QString result = QString("%1:%2:%3");

         return result.formatArg(hour(), 2, 10, '0')
            .formatArg(minute(), 2, 10, '0')
            .formatArg(second(), 2, 10, '0');
   }
}

QString QTime::toString(const QString &format) const
{
   return QLocale::system().toString(*this, format);
}

bool QTime::setHMS(int h, int m, int s, int ms)
{
   if (! isValid(h, m, s, ms)) {
      mds = NullTime;                // make this invalid
      return false;
   }
   mds = (h * SECS_PER_HOUR + m * SECS_PER_MIN + s) * 1000 + ms;

   return true;
}

QTime QTime::addSecs(int s) const
{
   s %= SECS_PER_DAY;
   return addMSecs(s * 1000);
}

int QTime::secsTo(const QTime &t) const
{
   if (!isValid() || !t.isValid()) {
      return 0;
   }

   // Truncate milliseconds as we do not want to consider them.
   int ourSeconds = ds() / 1000;
   int theirSeconds = t.ds() / 1000;

   return theirSeconds - ourSeconds;
}

QTime QTime::addMSecs(int ms) const
{
   QTime t;
   if (isValid()) {
      if (ms < 0) {
         // %,/ not well-defined for -ve, so always work with +ve.
         int negdays = (MSECS_PER_DAY - ms) / MSECS_PER_DAY;
         t.mds = (ds() + ms + negdays * MSECS_PER_DAY) % MSECS_PER_DAY;
      } else {
         t.mds = (ds() + ms) % MSECS_PER_DAY;
      }
   }

   return t;
}

int QTime::msecsTo(const QTime &t) const
{
   if (!isValid() || !t.isValid()) {
      return 0;
   }

   return t.ds() - ds();
}

static QTime fromIsoTimeString(QStringView string, Qt::DateFormat format, bool *isMidnight24)
{
   if (isMidnight24) {
      *isMidnight24 = false;
   }

   const int size = string.size();
   if (size < 5) {
      return QTime();
   }

   bool ok = false;
   int hour = QStringParser::toInteger<int>(string.mid(0, 2), &ok);

   if (!ok) {
      return QTime();
   }

   const int minute = QStringParser::toInteger<int>(string.mid(3, 2), &ok);

   if (!ok) {
      return QTime();
   }

   int second = 0;
   int msec = 0;

   if (size == 5) {
      // HH:mm format
      second = 0;
      msec = 0;

   } else if (string.at(5) == ',' || string.at(5) == '.') {
      if (format == Qt::TextDate) {
         return QTime();
      }

      // ISODate HH:mm.ssssss format
      // We only want 5 digits worth of fraction of minute. This follows the existing
      // behavior that determines how milliseconds are read; 4 millisecond digits are
      // read and then rounded to 3. If we read at most 5 digits for fraction of minute,
      // the maximum amount of millisecond digits it will expand to once converted to
      // seconds is 4. E.g. 12:34,99999 will expand to 12:34:59.9994. The milliseconds
      // will then be rounded up AND clamped to 999.

      const QStringView minuteFractionStr = string.mid(6, 5);

      const long minuteFractionInt = QStringParser::toInteger<long>(minuteFractionStr, &ok);

      if (!ok) {
         return QTime();
      }

      const float minuteFraction = double(minuteFractionInt) / (std::pow(double(10), minuteFractionStr.count()));

      const float secondWithMs = minuteFraction * 60;
      const float secondNoMs = std::floor(secondWithMs);
      const float secondFraction = secondWithMs - secondNoMs;

      second = secondNoMs;
      msec   = qMin(qRound(secondFraction * 1000.0), 999);

   } else {
      // HH:mm:ss or HH:mm:ss.zzz
      second = QStringParser::toInteger<int>(string.mid(6, 2), &ok);

      if (!ok) {
         return QTime();
      }

      if (size > 8 && (string.at(8) == ',' || string.at(8) == '.')) {
         const QStringView msecStr(string.mid(9, 4));

         int msecInt = msecStr.isEmpty() ? 0 : QStringParser::toInteger<int>(msecStr, &ok);
         if (!ok) {
            return QTime();
         }

         const double secondFraction(msecInt / (std::pow(double(10), msecStr.count())));
         msec = qMin(qRound(secondFraction * 1000.0), 999);
      }
   }

   if (format == Qt::ISODate && hour == 24 && minute == 0 && second == 0 && msec == 0) {
      if (isMidnight24) {
         *isMidnight24 = true;
      }

      hour = 0;
   }

   return QTime(hour, minute, second, msec);
}

QTime QTime::fromString(const QString &string, Qt::DateFormat format)
{
   if (string.isEmpty()) {
      return QTime();
   }

   switch (format) {
      case Qt::SystemLocaleDate:
      case Qt::SystemLocaleShortDate:
         return QLocale::system().toTime(string, QLocale::ShortFormat);

      case Qt::SystemLocaleLongDate:
         return QLocale::system().toTime(string, QLocale::LongFormat);

      case Qt::LocaleDate:
      case Qt::DefaultLocaleShortDate:
         return QLocale().toTime(string, QLocale::ShortFormat);

      case Qt::DefaultLocaleLongDate:
         return QLocale().toTime(string, QLocale::LongFormat);

      case Qt::RFC2822Date:
         return rfcDateImpl(string).time;

      case Qt::ISODate:
      case Qt::TextDate:
      default:
         return fromIsoTimeString(string, format, nullptr);
   }
}

QTime QTime::fromString(const QString &string, const QString &format)
{
   QTime time;

   QDateTimeParser dt(QVariant::Time, QDateTimeParser::FromString);

   if (dt.parseFormat(format)) {
      dt.fromString(string, nullptr, &time);
   }

   return time;
}

bool QTime::isValid(int h, int m, int s, int ms)
{
   return (uint)h < 24 && (uint)m < 60 && (uint)s < 60 && (uint)ms < 1000;
}

void QTime::start()
{
   *this = currentTime();
}

int QTime::restart()
{
   QTime t = currentTime();
   int n = msecsTo(t);

   if (n < 0) {                              // passed midnight
      n += 86400 * 1000;
   }
   *this = t;

   return n;
}

int QTime::elapsed() const
{
   int n = msecsTo(currentTime());

   if (n < 0) {                              // passed midnight
      n += 86400 * 1000;
   }

   return n;
}

// Calls the platform variant of tzset
static void qt_tzset()
{
#if defined(Q_OS_WIN)
   _tzset();
#else
   // tzname, timezone, daylight
   tzset();
#endif
}

// Returns the platform timezone which is the standard time offset
// Relies on tzset, mktime, or localtime having been called to populate timezone
static int qt_timezone()
{
#if defined(Q_CC_MSVC)
   long offset;
   _get_timezone(&offset);

   return offset;

#elif defined(Q_OS_BSD4) && ! defined(Q_OS_DARWIN)
   time_t clock = time(nullptr);
   struct tm t;
   localtime_r(&clock, &t);

   return -t.tm_gmtoff + (t.tm_isdst ? (long)SECS_PER_HOUR : 0L);

#else
   return timezone;

#endif
}

// Returns the tzname, assume tzset has been called already
static QString qt_tzname(QDateTimePrivate::DaylightStatus daylightStatus)
{
   int isDst = (daylightStatus == QDateTimePrivate::DaylightTime) ? 1 : 0;

#if defined(Q_CC_MSVC)
   size_t s = 0;
   char name[512];

   if (_get_tzname(&s, name, 512, isDst)) {
      return QString();
   }

   return QString::fromUtf8(name);

#else
   return QString::fromUtf8(tzname[isDst]);

#endif

}

// Calls the platform variant of mktime for the given date, time and daylightStatus,
// and updates the date, time, daylightStatus and abbreviation with the returned values
// If the date falls outside the 1970 to 2037 range supported by mktime / time_t
// then null date/time will be returned, you should adjust the date first if
// you need a guaranteed result.

static qint64 qt_mktime(QDate *date, QTime *time, QDateTimePrivate::DaylightStatus *daylightStatus,
   QString *abbreviation, bool *ok = nullptr)
{
   const qint64 msec = time->msec();
   int yy, mm, dd;
   date->getDate(&yy, &mm, &dd);

   // All other platforms provide standard C library time functions
   tm local;
   memset(&local, 0, sizeof(local)); // tm_[wy]day plus any non-standard fields
   local.tm_sec  = time->second();
   local.tm_min  = time->minute();
   local.tm_hour = time->hour();
   local.tm_mday = dd;
   local.tm_mon  = mm - 1;
   local.tm_year = yy - 1900;

   if (daylightStatus) {
      local.tm_isdst = int(*daylightStatus);
   } else {
      local.tm_isdst = -1;
   }

#if defined(Q_OS_WIN)
   int hh = local.tm_hour;
#endif

   time_t secsSinceEpoch = mktime(&local);
   if (secsSinceEpoch != time_t(-1)) {
      *date = QDate(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
      *time = QTime(local.tm_hour, local.tm_min, local.tm_sec, msec);

#if defined(Q_OS_WIN)
      // Windows mktime for the missing hour subtracts 1 hour from the time
      // instead of adding 1 hour. If time differs and is standard time then
      // this has happened, so add 2 hours to the time and 1 hour to the msecs

      if (local.tm_isdst == 0 && local.tm_hour != hh) {
         if (time->hour() >= 22) {
            *date = date->addDays(1);
         }

         *time = time->addSecs(2 * SECS_PER_HOUR);
         secsSinceEpoch += SECS_PER_HOUR;
         local.tm_isdst = 1;
      }
#endif

      if (local.tm_isdst >= 1) {
         if (daylightStatus) {
            *daylightStatus = QDateTimePrivate::DaylightTime;
         }

         if (abbreviation) {
            *abbreviation = qt_tzname(QDateTimePrivate::DaylightTime);
         }

      } else if (local.tm_isdst == 0) {
         if (daylightStatus) {
            *daylightStatus = QDateTimePrivate::StandardTime;
         }

         if (abbreviation) {
            *abbreviation = qt_tzname(QDateTimePrivate::StandardTime);
         }

      } else {
         if (daylightStatus) {
            *daylightStatus = QDateTimePrivate::UnknownDaylightTime;
         }

         if (abbreviation) {
            *abbreviation = qt_tzname(QDateTimePrivate::StandardTime);
         }
      }

      if (ok) {
         *ok = true;
      }

   } else {
      *date = QDate();
      *time = QTime();

      if (daylightStatus) {
         *daylightStatus = QDateTimePrivate::UnknownDaylightTime;
      }

      if (abbreviation) {
         *abbreviation = QString();
      }

      if (ok) {
         *ok = false;
      }
   }

   return ((qint64)secsSinceEpoch * 1000) + msec;
}

// Calls the platform variant of localtime for the given msecs, and updates
// the date, time, and DST status with the returned values.
static bool qt_localtime(qint64 msecsSinceEpoch, QDate *localDate, QTime *localTime,
   QDateTimePrivate::DaylightStatus *daylightStatus)
{
   const time_t secsSinceEpoch = msecsSinceEpoch / 1000;
   const int msec = msecsSinceEpoch % 1000;

   tm local;
   bool valid = false;

#if ! defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
   // localtime() is required to work as if tzset() was called before it.
   // localtime_r() does not have this requirement, so make an explicit call.
   qt_tzset();

   // Use the reentrant version of localtime() where available
   // as is thread-safe and doesn't use a shared static data area
   tm *res = nullptr;
   res = localtime_r(&secsSinceEpoch, &local);

   if (res) {
      valid = true;
   }

#elif defined(Q_CC_MSVC)
   if (! _localtime64_s(&local, &secsSinceEpoch)) {
      valid = true;
   }
#else
   // Returns shared static data which may be overwritten at any time
   // So copy the result asap
   tm *res = nullptr;
   res = localtime(&secsSinceEpoch);

   if (res) {
      local = *res;
      valid = true;
   }
#endif

   if (valid) {
      *localDate = QDate(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
      *localTime = QTime(local.tm_hour, local.tm_min, local.tm_sec, msec);

      if (daylightStatus) {
         if (local.tm_isdst > 0) {
            *daylightStatus = QDateTimePrivate::DaylightTime;
         } else if (local.tm_isdst < 0) {
            *daylightStatus = QDateTimePrivate::UnknownDaylightTime;
         } else {
            *daylightStatus = QDateTimePrivate::StandardTime;
         }
      }

      return true;

   } else {
      *localDate = QDate();
      *localTime = QTime();

      if (daylightStatus) {
         *daylightStatus = QDateTimePrivate::UnknownDaylightTime;
      }

      return false;
   }
}

// Converts an msecs value into a date and time
static void msecsToTime(qint64 msecs, QDate *date, QTime *time)
{
   qint64 jd = JULIAN_DAY_FOR_EPOCH;
   qint64 ds = 0;

   if (qAbs(msecs) >= MSECS_PER_DAY) {
      jd += (msecs / MSECS_PER_DAY);
      msecs %= MSECS_PER_DAY;
   }

   if (msecs < 0) {
      ds = MSECS_PER_DAY - msecs - 1;
      jd -= ds / MSECS_PER_DAY;
      ds = ds % MSECS_PER_DAY;
      ds = MSECS_PER_DAY - ds - 1;
   } else {
      ds = msecs;
   }

   if (date) {
      *date = QDate::fromJulianDay(jd);
   }

   if (time) {
      *time = QTime::fromMSecsSinceStartOfDay(ds);
   }
}

// Converts a date/time value into msecs
static qint64 timeToMSecs(const QDate &date, const QTime &time)
{
   return ((date.toJulianDay() - JULIAN_DAY_FOR_EPOCH) * MSECS_PER_DAY)
      + time.msecsSinceStartOfDay();
}

// Convert an MSecs Since Epoch into Local Time
static bool epochMSecsToLocalTime(qint64 msecs, QDate *localDate, QTime *localTime,
   QDateTimePrivate::DaylightStatus *daylightStatus = nullptr)
{
   if (msecs < 0) {
      // any LocalTime before 1970-01-01 will *not* have any Daylight Time applied
      // Instead just use the standard offset from UTC to convert to UTC time

      qt_tzset();
      msecsToTime(msecs - qt_timezone() * 1000, localDate, localTime);

      if (daylightStatus) {
         *daylightStatus = QDateTimePrivate::StandardTime;
      }

      return true;

   } else if (msecs > (qint64(TIME_T_MAX) * 1000)) {
      // Docs state any LocalTime after 2037-12-31 *will* have any DST applied
      // but this may fall outside the supported time_t range

      // emerald: use QTimeZone when available to apply the future rule correctly

      QDate utcDate;
      QTime utcTime;
      msecsToTime(msecs, &utcDate, &utcTime);
      int year, month, day;
      utcDate.getDate(&year, &month, &day);

      // 2037 is not a leap year, so make sure date isn't Feb 29
      if (month == 2 && day == 29) {
         --day;
      }

      QDate fakeDate(2037, month, day);
      qint64 fakeMsecs = QDateTime(fakeDate, utcTime, Qt::UTC).toMSecsSinceEpoch();

      bool res = qt_localtime(fakeMsecs, localDate, localTime, daylightStatus);
      *localDate = localDate->addDays(fakeDate.daysTo(utcDate));

      return res;

   } else {
      // Falls inside time_t suported range so can use localtime
      return qt_localtime(msecs, localDate, localTime, daylightStatus);
   }
}

// Convert a LocalTime expressed in local msecs encoding and the corresponding
// DST status into a UTC epoch msecs. Optionally populate the returned
// values from mktime for the adjusted local date and time.
static qint64 localMSecsToEpochMSecs(qint64 localMsecs,
   QDateTimePrivate::DaylightStatus *daylightStatus, QDate *localDate = nullptr,
   QTime *localTime = nullptr, QString *abbreviation = nullptr)
{
   QDate dt;
   QTime tm;
   msecsToTime(localMsecs, &dt, &tm);

   const qint64 msecsMax = qint64(TIME_T_MAX) * 1000;

   if (localMsecs <= qint64(MSECS_PER_DAY)) {

      // Docs state any LocalTime before 1970-01-01 will *not* have any DST applied

      // First, if localMsecs is within +/- 1 day of minimum time_t try mktime in case it does
      // fall after minimum and needs proper DST conversion
      if (localMsecs >= -qint64(MSECS_PER_DAY)) {
         bool valid;
         qint64 utcMsecs = qt_mktime(&dt, &tm, daylightStatus, abbreviation, &valid);

         if (valid && utcMsecs >= 0) {
            // mktime worked and falls in valid range, so use it
            if (localDate) {
               *localDate = dt;
            }

            if (localTime) {
               *localTime = tm;
            }

            return utcMsecs;
         }

      } else {
         // If we don't call mktime then need to call tzset to get offset
         qt_tzset();
      }

      // Time is clearly before 1970-01-01 so just use standard offset to convert
      qint64 utcMsecs = localMsecs + qt_timezone() * 1000;
      if (localDate || localTime) {
         msecsToTime(localMsecs, localDate, localTime);
      }

      if (daylightStatus) {
         *daylightStatus = QDateTimePrivate::StandardTime;
      }

      if (abbreviation) {
         *abbreviation = qt_tzname(QDateTimePrivate::StandardTime);
      }

      return utcMsecs;

   } else if (localMsecs >= msecsMax - MSECS_PER_DAY) {

      // Any LocalTime after 2037-12-31 *will* have any DST applied
      // but this may fall outside the supported time_t range
      // First, if localMsecs is within +/- 1 day of maximum time_t try mktime in case it does
      // fall before maximum and can use proper DST conversion

      if (localMsecs <= msecsMax + MSECS_PER_DAY) {
         bool valid;
         qint64 utcMsecs = qt_mktime(&dt, &tm, daylightStatus, abbreviation, &valid);
         if (valid && utcMsecs <= msecsMax) {
            // mktime worked and falls in valid range, so use it
            if (localDate) {
               *localDate = dt;
            }
            if (localTime) {
               *localTime = tm;
            }
            return utcMsecs;
         }
      }

      // emerald -Use QTimeZone when available to apply the future rule correctly

      int year, month, day;
      dt.getDate(&year, &month, &day);

      // 2037 is not a leap year, so make sure date isn't Feb 29
      if (month == 2 && day == 29) {
         --day;
      }

      QDate fakeDate(2037, month, day);
      qint64 fakeDiff = fakeDate.daysTo(dt);
      qint64 utcMsecs = qt_mktime(&fakeDate, &tm, daylightStatus, abbreviation);

      if (localDate) {
         *localDate = fakeDate.addDays(fakeDiff);
      }

      if (localTime) {
         *localTime = tm;
      }

      QDate utcDate;
      QTime utcTime;
      msecsToTime(utcMsecs, &utcDate, &utcTime);
      utcDate = utcDate.addDays(fakeDiff);
      utcMsecs = timeToMSecs(utcDate, utcTime);
      return utcMsecs;

   } else {

      // Clearly falls inside 1970-2037 suported range so can use mktime
      qint64 utcMsecs = qt_mktime(&dt, &tm, daylightStatus, abbreviation);
      if (localDate) {
         *localDate = dt;
      }

      if (localTime) {
         *localTime = tm;
      }

      return utcMsecs;
   }
}

QDateTimePrivate::QDateTimePrivate(const QDate &toDate, const QTime &toTime, Qt::TimeSpec toSpec,
      int offsetSeconds)
   : m_msecs(0), m_spec(Qt::LocalTime), m_offsetFromUtc(0), m_status(Qt::EmptyFlag)
{
   setTimeSpec(toSpec, offsetSeconds);
   setDateTime(toDate, toTime);
}

QDateTimePrivate::QDateTimePrivate(const QDate &toDate, const QTime &toTime, const QTimeZone &toTimeZone)
   : m_spec(Qt::TimeZone), m_offsetFromUtc(0), m_timeZone(toTimeZone), m_status(Qt::EmptyFlag)
{
   setDateTime(toDate, toTime);
}

void QDateTimePrivate::setTimeSpec(Qt::TimeSpec spec, int offsetSeconds)
{
   clearValidDateTime();
   clearSetToDaylightStatus();

   m_timeZone = QTimeZone();

   switch (spec) {
      case Qt::OffsetFromUTC:
         if (offsetSeconds == 0) {
            m_spec = Qt::UTC;
            m_offsetFromUtc = 0;
         } else {
            m_spec = Qt::OffsetFromUTC;
            m_offsetFromUtc = offsetSeconds;
         }
         break;

      case Qt::TimeZone:
         // Use system time zone instead
         m_spec = Qt::LocalTime;
         m_offsetFromUtc = 0;
         break;

      case Qt::UTC:
      case Qt::LocalTime:
         m_spec = spec;
         m_offsetFromUtc = 0;
         break;
   }
}

void QDateTimePrivate::setDateTime(const QDate &date, const QTime &time)
{
   // If the date is valid and the time is not we set time to 00:00:00
   QTime useTime = time;
   if (!useTime.isValid() && date.isValid()) {
      useTime = QTime::fromMSecsSinceStartOfDay(0);
   }

   StatusFlags newStatus;

   // Set date value and status
   qint64 days = 0;
   if (date.isValid()) {
      days = date.toJulianDay() - JULIAN_DAY_FOR_EPOCH;
      newStatus = ValidDate;
   } else if (date.isNull()) {
      newStatus = NullDate;
   }

   // Set time value and status
   int ds = 0;
   if (useTime.isValid()) {
      ds = useTime.msecsSinceStartOfDay();
      newStatus |= ValidTime;
   } else if (time.isNull()) {
      newStatus |= NullTime;
   }

   // Set msecs serial value
   m_msecs = (days * MSECS_PER_DAY) + ds;
   m_status = newStatus;

   // Set if date and time are valid
   checkValidDateTime();
}

QPair<QDate, QTime> QDateTimePrivate::getDateTime() const
{
   QPair<QDate, QTime> result;
   msecsToTime(m_msecs, &result.first, &result.second);

   if (isNullDate()) {
      result.first = QDate();
   }

   if (isNullTime()) {
      result.second = QTime();
   }

   return result;
}

// Set the Daylight Status if LocalTime set via msecs
void QDateTimePrivate::setDaylightStatus(QDateTimePrivate::DaylightStatus status)
{
   if (status == DaylightTime) {
      m_status = m_status & ~SetToStandardTime;
      m_status = m_status | SetToDaylightTime;

   } else if (status == StandardTime) {
      m_status = m_status & ~SetToDaylightTime;
      m_status = m_status | SetToStandardTime;

   } else {
      clearSetToDaylightStatus();
   }
}

// Get the DST Status if LocalTime set via msecs
QDateTimePrivate::DaylightStatus QDateTimePrivate::daylightStatus() const
{
   if ((m_status & SetToDaylightTime) == SetToDaylightTime) {
      return DaylightTime;
   }
   if ((m_status & SetToStandardTime) == SetToStandardTime) {
      return StandardTime;
   }

   return UnknownDaylightTime;
}

qint64 QDateTimePrivate::toMSecsSinceEpoch() const
{
   switch (m_spec) {
      case Qt::OffsetFromUTC:
      case Qt::UTC:
         return (m_msecs - (m_offsetFromUtc * 1000));

      case Qt::LocalTime: {
         // recalculate the local timezone
         DaylightStatus status = daylightStatus();
         return localMSecsToEpochMSecs(m_msecs, &status);
      }

      case Qt::TimeZone:
         return zoneMSecsToEpochMSecs(m_msecs, m_timeZone);

   }

   // error, may want to throw
   return 0;
}

// Check the UTC / offsetFromUTC validity
void QDateTimePrivate::checkValidDateTime()
{
   switch (m_spec) {
      case Qt::OffsetFromUTC:
      case Qt::UTC:
         // for these a valid date and a valid time imply a valid QDateTime
         if (isValidDate() && isValidTime()) {
            setValidDateTime();
         } else {
            clearValidDateTime();
         }
         break;

      case Qt::TimeZone:
      case Qt::LocalTime:
         // for these, we need to check whether the timezone is valid and whether
         // the time is valid in that timezone. Expensive, but no other option.
         refreshDateTime();
         break;
   }
}

// Refresh the LocalTime validity and offset
void QDateTimePrivate::refreshDateTime()
{
   switch (m_spec) {
      case Qt::OffsetFromUTC:
      case Qt::UTC:
         // Always set by setDateTime so just return
         return;

      case Qt::TimeZone:
      case Qt::LocalTime:
         break;
   }

   // If not valid date and time then is invalid
   if (!isValidDate() || !isValidTime()) {
      clearValidDateTime();
      m_offsetFromUtc = 0;
      return;
   }

   // If not valid time zone then is invalid
   if (m_spec == Qt::TimeZone && !m_timeZone.isValid()) {
      clearValidDateTime();
      m_offsetFromUtc = 0;
      return;
   }

   // We have a valid date and time and a Qt::LocalTime or Qt::TimeZone that needs calculating
   // LocalTime and TimeZone might fall into a "missing" DST transition hour
   // Calling toEpochMSecs will adjust the returned date/time if it does
   QDate testDate;
   QTime testTime;
   qint64 epochMSecs = 0;
   if (m_spec == Qt::LocalTime) {
      DaylightStatus status = daylightStatus();
      epochMSecs = localMSecsToEpochMSecs(m_msecs, &status, &testDate, &testTime);

   } else {
      epochMSecs = zoneMSecsToEpochMSecs(m_msecs, m_timeZone, &testDate, &testTime);

   }
   if (timeToMSecs(testDate, testTime) == m_msecs) {
      setValidDateTime();
      // Cache the offset to use in toMSecsSinceEpoch()
      m_offsetFromUtc = (m_msecs - epochMSecs) / 1000;
   } else {
      clearValidDateTime();
      m_offsetFromUtc = 0;
   }
}

// Convert a TimeZone time expressed in zone msecs encoding into a UTC epoch msecs
qint64 QDateTimePrivate::zoneMSecsToEpochMSecs(qint64 zoneMSecs, const QTimeZone &zone,
   QDate *localDate, QTime *localTime)
{
   // Get the effective data from QTimeZone
   QTimeZonePrivate::Data data = zone.d->dataForLocalTime(zoneMSecs);
   // Docs state any LocalTime before 1970-01-01 will *not* have any DST applied
   // but all affected times afterwards will have DST applied.
   if (data.atMSecsSinceEpoch >= 0) {
      msecsToTime(data.atMSecsSinceEpoch + (data.offsetFromUtc * 1000), localDate, localTime);
      return data.atMSecsSinceEpoch;
   } else {
      msecsToTime(zoneMSecs, localDate, localTime);
      return zoneMSecs - (data.standardTimeOffset * 1000);
   }
}

QDateTime::QDateTime()
   : d(new QDateTimePrivate)
{
}

QDateTime::QDateTime(const QDate &date)
   : d(new QDateTimePrivate(date, QTime(0, 0, 0), Qt::LocalTime, 0))
{
}

QDateTime::QDateTime(const QDate &date, const QTime &time, Qt::TimeSpec spec, int offsetSeconds)
   : d(new QDateTimePrivate(date, time, spec, offsetSeconds))
{
}

QDateTime::QDateTime(const QDate &date, const QTime &time, const QTimeZone &timeZone)
   : d(new QDateTimePrivate(date, time, timeZone))
{
}

QDateTime::QDateTime(const QDateTime &other)
   : d(other.d)
{
}

QDateTime::QDateTime(QDateTime &&other)
   : d(std::move(other.d))
{
}

QDateTime::~QDateTime()
{
}

QDateTime &QDateTime::operator=(const QDateTime &other)
{
   d = other.d;
   return *this;
}

bool QDateTime::isNull() const
{
   return d->isNullDate() && d->isNullTime();
}

bool QDateTime::isValid() const
{
   return (d->isValidDateTime());
}

QDate QDateTime::date() const
{
   if (d->isNullDate()) {
      return QDate();
   }

   QDate dt;
   msecsToTime(d->m_msecs, &dt, nullptr);

   return dt;
}

QTime QDateTime::time() const
{
   if (d->isNullTime()) {
      return QTime();
   }

   QTime tm;
   msecsToTime(d->m_msecs, nullptr, &tm);
   return tm;
}

Qt::TimeSpec QDateTime::timeSpec() const
{
   return d->m_spec;
}

QTimeZone QDateTime::timeZone() const
{
   switch (d->m_spec) {
      case Qt::UTC:
         return QTimeZone::utc();
      case Qt::OffsetFromUTC:
         return QTimeZone(d->m_offsetFromUtc);
      case Qt::TimeZone:
         Q_ASSERT(d->m_timeZone.isValid());
         return d->m_timeZone;
      case Qt::LocalTime:
         return QTimeZone::systemTimeZone();
   }
   return QTimeZone();
}

int QDateTime::offsetFromUtc() const
{
   return d->m_offsetFromUtc;
}

QString QDateTime::timeZoneAbbreviation() const
{
   switch (d->m_spec) {
      case Qt::UTC:
         return QTimeZonePrivate::utcQString();

      case Qt::OffsetFromUTC:
         return QTimeZonePrivate::utcQString() + toOffsetString(Qt::ISODate, d->m_offsetFromUtc);

      case Qt::TimeZone:
         return d->m_timeZone.d->abbreviation(d->toMSecsSinceEpoch());

      case Qt::LocalTime:  {
         QString abbrev;
         QDateTimePrivate::DaylightStatus status = d->daylightStatus();
         localMSecsToEpochMSecs(d->m_msecs, &status, nullptr, nullptr, &abbrev);
         return abbrev;
      }
   }

   return QString();
}



bool QDateTime::isDaylightTime() const
{
   switch (d->m_spec) {
      case Qt::UTC:
      case Qt::OffsetFromUTC:
         return false;

      case Qt::TimeZone:
         return d->m_timeZone.d->isDaylightTime(toMSecsSinceEpoch());

      case Qt::LocalTime: {
         QDateTimePrivate::DaylightStatus status = d->daylightStatus();
         if (status == QDateTimePrivate::UnknownDaylightTime) {
            localMSecsToEpochMSecs(d->m_msecs, &status);
         }
         return (status == QDateTimePrivate::DaylightTime);
      }
   }
   return false;
}

void QDateTime::setDate(const QDate &date)
{
   d->setDateTime(date, time());
}

void QDateTime::setTime(const QTime &time)
{
   d->setDateTime(date(), time);
}

void QDateTime::setTimeSpec(Qt::TimeSpec spec)
{
   QDateTimePrivate *d = this->d.data(); // detaches (and shadows d)
   d->setTimeSpec(spec, 0);
   d->checkValidDateTime();
}

void QDateTime::setOffsetFromUtc(int offsetSeconds)
{
   QDateTimePrivate *d = this->d.data(); // detaches (and shadows d)
   d->setTimeSpec(Qt::OffsetFromUTC, offsetSeconds);
   d->checkValidDateTime();
}

void QDateTime::setTimeZone(const QTimeZone &toZone)
{
   QDateTimePrivate *d = this->d.data(); // detaches (and shadows d)
   d->m_spec = Qt::TimeZone;
   d->m_offsetFromUtc = 0;
   d->m_timeZone = toZone;
   d->refreshDateTime();
}

qint64 QDateTime::toMSecsSinceEpoch() const
{
   return d->toMSecsSinceEpoch();
}

quint64 QDateTime::toTime_t() const
{
   if (!isValid()) {
      return quint64(-1);
   }

   qint64 retval = d->toMSecsSinceEpoch() / 1000;

   if (quint64(retval) >= Q_UINT64_C(0xFFFFFFFF)) {
      return quint64(-1);
   }

   return quint64(retval);
}

void QDateTime::setMSecsSinceEpoch(qint64 msecs)
{
   QDateTimePrivate *d = this->d.data(); // detaches (and shadows d)

   d->m_status = Qt::EmptyFlag;

   switch (d->m_spec) {
      case Qt::UTC:
         d->m_msecs = msecs;
         d->m_status = d->m_status
            | QDateTimePrivate::ValidDate
            | QDateTimePrivate::ValidTime
            | QDateTimePrivate::ValidDateTime;
         break;

      case Qt::OffsetFromUTC:
         d->m_msecs = msecs + (d->m_offsetFromUtc * 1000);
         d->m_status = d->m_status
            | QDateTimePrivate::ValidDate
            | QDateTimePrivate::ValidTime
            | QDateTimePrivate::ValidDateTime;
         break;

      case Qt::TimeZone:

         // Docs state any LocalTime before 1970-01-01 will *not* have any DST applied
         // but all affected times afterwards will have DST applied.
         if (msecs >= 0) {
            d->m_offsetFromUtc = d->m_timeZone.d->offsetFromUtc(msecs);
         } else {
            d->m_offsetFromUtc = d->m_timeZone.d->standardTimeOffset(msecs);
         }

         d->m_msecs = msecs + (d->m_offsetFromUtc * 1000);
         d->m_status = d->m_status
            | QDateTimePrivate::ValidDate
            | QDateTimePrivate::ValidTime
            | QDateTimePrivate::ValidDateTime;
         d->refreshDateTime();

         break;

      case Qt::LocalTime: {
         QDate dt;
         QTime tm;
         QDateTimePrivate::DaylightStatus status;
         epochMSecsToLocalTime(msecs, &dt, &tm, &status);
         d->setDateTime(dt, tm);
         d->setDaylightStatus(status);
         d->refreshDateTime();
         break;
      }
   }
}

void QDateTime::setTime_t(quint64  secsSince1Jan1970UTC)
{
   setMSecsSinceEpoch((qint64)secsSince1Jan1970UTC * 1000);
}

QString QDateTime::toString(Qt::DateFormat format) const
{
   QString buf;

   if (!isValid()) {
      return buf;
   }

   switch (format) {
      case Qt::SystemLocaleDate:
      case Qt::SystemLocaleShortDate:
         return QLocale::system().toString(*this, QLocale::ShortFormat);

      case Qt::SystemLocaleLongDate:
         return QLocale::system().toString(*this, QLocale::LongFormat);

      case Qt::LocaleDate:
      case Qt::DefaultLocaleShortDate:
         return QLocale().toString(*this, QLocale::ShortFormat);

      case Qt::DefaultLocaleLongDate:
         return QLocale().toString(*this, QLocale::LongFormat);

      case Qt::RFC2822Date: {
         buf = QLocale::c().toString(*this, "dd MMM yyyy hh:mm:ss ");
         buf += toOffsetString(Qt::TextDate, d->m_offsetFromUtc);
         return buf;
      }

      default:
#ifndef QT_NO_TEXTDATE
      case Qt::TextDate: {
         const QPair<QDate, QTime> p = d->getDateTime();
         const QDate &dt = p.first;
         const QTime &tm = p.second;

         //We cant use date.toString(Qt::TextDate) as we need to insert the time before the year
         buf = QString("%1 %2 %3 %4 %5").formatArg(dt.shortDayName(dt.dayOfWeek()))
            .formatArg(dt.shortMonthName(dt.month()))
            .formatArg(dt.day())
            .formatArg(tm.toString(Qt::TextDate))
            .formatArg(dt.year());

         if (timeSpec() != Qt::LocalTime) {
            buf += " GMT";
            if (d->m_spec == Qt::OffsetFromUTC) {
               buf += toOffsetString(Qt::TextDate, d->m_offsetFromUtc);
            }
         }
         return buf;
      }
#endif

      case Qt::ISODate: {
         const QPair<QDate, QTime> p = d->getDateTime();
         const QDate &dt = p.first;
         const QTime &tm = p.second;
         buf = dt.toString(Qt::ISODate);
         if (buf.isEmpty()) {
            return QString();   // failed to convert
         }
         buf += QLatin1Char('T');
         buf += tm.toString(Qt::ISODate);

         switch (d->m_spec) {
            case Qt::UTC:
               buf += QLatin1Char('Z');
               break;

            case Qt::OffsetFromUTC:
               buf += toOffsetString(Qt::ISODate, d->m_offsetFromUtc);
               break;

            default:
               break;
         }
         return buf;
      }
   }
}

QString QDateTime::toString(const QString &format) const
{
   return QLocale::system().toString(*this, format);
}

static void massageAdjustedDateTime(Qt::TimeSpec spec,
   const QTimeZone &zone, QDate *date, QTime *time)
{
   /*
     If we have just adjusted to a day with a DST transition, our given time
     may lie in the transition hour (either missing or duplicated).  For any
     other time, telling mktime (deep in the bowels of localMSecsToEpochMSecs)
     we don't know its DST-ness will produce no adjustment (just a decision as
     to its DST-ness); but for a time in spring's missing hour it'll adjust the
     time while picking a DST-ness.  (Handling of autumn is trickier, as either
     DST-ness is valid, without adjusting the time.  We might want to propagate
     d->daylightStatus() in that case, but it's hard to do so without breaking
     (far more common) other cases; and it makes little difference, as the two
     answers do then differ only in DST-ness.)
   */

   if (spec == Qt::LocalTime) {
      QDateTimePrivate::DaylightStatus status = QDateTimePrivate::UnknownDaylightTime;
      localMSecsToEpochMSecs(timeToMSecs(*date, *time), &status, date, time);
   } else if (spec == Qt::TimeZone) {
      QDateTimePrivate::zoneMSecsToEpochMSecs(timeToMSecs(*date, *time), zone, date, time);
   }
}

#define MASSAGEADJUSTEDDATETIME(s, z, d, t) massageAdjustedDateTime(s, z, d, t)

QDateTime QDateTime::addDays(qint64 ndays) const
{
   QDateTime dt(*this);
   QPair<QDate, QTime> p = d->getDateTime();
   QDate &date = p.first;
   QTime &time = p.second;
   date = date.addDays(ndays);
   MASSAGEADJUSTEDDATETIME(d->m_spec, d->m_timeZone, &date, &time);
   dt.d->setDateTime(date, time);

   return dt;
}

QDateTime QDateTime::addMonths(qint64 nmonths) const
{
   QDateTime dt(*this);
   QPair<QDate, QTime> p = d->getDateTime();
   QDate &date = p.first;
   QTime &time = p.second;
   date = date.addMonths(nmonths);
   MASSAGEADJUSTEDDATETIME(d->m_spec, d->m_timeZone, &date, &time);
   dt.d->setDateTime(date, time);

   return dt;
}

QDateTime QDateTime::addYears(qint64 nyears) const
{
   QDateTime dt(*this);
   QPair<QDate, QTime> p = d->getDateTime();
   QDate &date = p.first;
   QTime &time = p.second;
   date = date.addYears(nyears);
   MASSAGEADJUSTEDDATETIME(d->m_spec, d->m_timeZone, &date, &time);
   dt.d->setDateTime(date, time);
   return dt;
}

#undef MASSAGEADJUSTEDDATETIME

QDateTime QDateTime::addSecs(qint64 s) const
{
   return addMSecs(s * 1000);
}

QDateTime QDateTime::addMSecs(qint64 msecs) const
{
   if (!isValid()) {
      return QDateTime();
   }

   QDateTime dt(*this);

   if (d->m_spec == Qt::LocalTime || d->m_spec == Qt::TimeZone) {
      // Convert to real UTC first in case crosses DST transition

      dt.setMSecsSinceEpoch(d->toMSecsSinceEpoch() + msecs);

   } else {
      // No need to convert, just add on  {
      dt.d->m_msecs = dt.d->m_msecs + msecs;
   }

   return dt;
}

qint64 QDateTime::daysTo(const QDateTime &other) const
{
   return date().daysTo(other.date());
}

qint64 QDateTime::secsTo(const QDateTime &other) const
{
   return (msecsTo(other) / 1000);
}

qint64 QDateTime::msecsTo(const QDateTime &other) const
{
   if (!isValid() || !other.isValid()) {
      return 0;
   }

   return other.d->toMSecsSinceEpoch() - d->toMSecsSinceEpoch();
}

QDateTime QDateTime::toTimeSpec(Qt::TimeSpec spec) const
{
   if (d->m_spec == spec && (spec == Qt::UTC || spec == Qt::LocalTime)) {
      return *this;
   }

   if (! isValid()) {
      QDateTime ret = *this;
      ret.setTimeSpec(spec);
      return ret;
   }

   return fromMSecsSinceEpoch(d->toMSecsSinceEpoch(), spec, 0);
}

QDateTime QDateTime::toOffsetFromUtc(qint64 offsetSeconds) const
{
   if (d->m_spec == Qt::OffsetFromUTC && d->m_offsetFromUtc == offsetSeconds) {
      return *this;
   }

   if (!isValid()) {
      QDateTime ret = *this;
      ret.setOffsetFromUtc(offsetSeconds);
      return ret;
   }

   return fromMSecsSinceEpoch(d->toMSecsSinceEpoch(), Qt::OffsetFromUTC, offsetSeconds);
}

QDateTime QDateTime::toTimeZone(const QTimeZone &timeZone) const
{
   if (d->m_spec == Qt::TimeZone && d->m_timeZone == timeZone) {
      return *this;
   }

   if (!isValid()) {
      QDateTime ret = *this;
      ret.setTimeZone(timeZone);
      return ret;
   }

   return fromMSecsSinceEpoch(d->toMSecsSinceEpoch(), timeZone);
}

bool QDateTime::operator==(const QDateTime &other) const
{
   if (d->m_spec == Qt::LocalTime
      && other.d->m_spec == Qt::LocalTime
      && d->m_status == other.d->m_status) {
      return (d->m_msecs == other.d->m_msecs);
   }
   // Convert to UTC and compare
   return (toMSecsSinceEpoch() == other.toMSecsSinceEpoch());
}


bool QDateTime::operator<(const QDateTime &other) const
{
   if (d->m_spec == Qt::LocalTime && other.d->m_spec == Qt::LocalTime && d->m_status == other.d->m_status) {
      return (d->m_msecs < other.d->m_msecs);
   }

   // Convert to UTC and compare
   return (toMSecsSinceEpoch() < other.toMSecsSinceEpoch());
}

#if defined(Q_OS_WIN)
static inline uint msecsFromDecomposed(int hour, int minute, int sec, int msec = 0)
{
   return MSECS_PER_HOUR * hour + MSECS_PER_MIN * minute + 1000 * sec + msec;
}

QDate QDate::currentDate()
{
   QDate d;
   SYSTEMTIME st;
   memset(&st, 0, sizeof(SYSTEMTIME));
   GetLocalTime(&st);
   d.jd = julianDayFromDate(st.wYear, st.wMonth, st.wDay);
   return d;
}

QTime QTime::currentTime()
{
   QTime ct;
   SYSTEMTIME st;
   memset(&st, 0, sizeof(SYSTEMTIME));
   GetLocalTime(&st);
   ct.setHMS(st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

   return ct;
}

QDateTime QDateTime::currentDateTime()
{
   QDate d;
   QTime t;
   SYSTEMTIME st;
   memset(&st, 0, sizeof(SYSTEMTIME));
   GetLocalTime(&st);
   d.jd = julianDayFromDate(st.wYear, st.wMonth, st.wDay);
   t.mds = msecsFromDecomposed(st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
   return QDateTime(d, t);
}

QDateTime QDateTime::currentDateTimeUtc()
{
   QDate d;
   QTime t;
   SYSTEMTIME st;
   memset(&st, 0, sizeof(SYSTEMTIME));
   GetSystemTime(&st);
   d.jd = julianDayFromDate(st.wYear, st.wMonth, st.wDay);
   t.mds = msecsFromDecomposed(st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
   return QDateTime(d, t, Qt::UTC);
}

qint64 QDateTime::currentMSecsSinceEpoch()
{
   SYSTEMTIME st;
   memset(&st, 0, sizeof(SYSTEMTIME));
   GetSystemTime(&st);

   return msecsFromDecomposed(st.wHour, st.wMinute, st.wSecond, st.wMilliseconds) +
      qint64(julianDayFromDate(st.wYear, st.wMonth, st.wDay)
         - julianDayFromDate(1970, 1, 1)) * Q_INT64_C(86400000);
}

#elif defined(Q_OS_UNIX)

QDate QDate::currentDate()
{
   return QDateTime::currentDateTime().date();
}

QTime QTime::currentTime()
{
   return QDateTime::currentDateTime().time();
}

QDateTime QDateTime::currentDateTime()
{
   return fromMSecsSinceEpoch(currentMSecsSinceEpoch(), Qt::LocalTime);
}

QDateTime QDateTime::currentDateTimeUtc()
{
   return fromMSecsSinceEpoch(currentMSecsSinceEpoch(), Qt::UTC);
}

qint64 QDateTime::currentMSecsSinceEpoch()
{
   // posix compliant system
   // we have milliseconds
   struct timeval tv;
   gettimeofday(&tv, nullptr);
   return qint64(tv.tv_sec) * Q_INT64_C(1000) + tv.tv_usec / 1000;
}

#else
#error "Unknown Operating System type"

#endif

QDateTime QDateTime::fromTime_t(quint64 seconds, Qt::TimeSpec spec, int offsetSeconds)
{
   return fromMSecsSinceEpoch((qint64)seconds * 1000, spec, offsetSeconds);
}

QDateTime QDateTime::fromTime_t(quint64  seconds, const QTimeZone &timeZone)
{
   return fromMSecsSinceEpoch((qint64)seconds * 1000, timeZone);
}

QDateTime QDateTime::fromMSecsSinceEpoch(qint64 msecs, Qt::TimeSpec spec, int offsetSeconds)
{
   QDateTime dt;
   dt.d->setTimeSpec(spec, offsetSeconds);
   dt.setMSecsSinceEpoch(msecs);
   return dt;
}

QDateTime QDateTime::fromMSecsSinceEpoch(qint64 msecs, const QTimeZone &timeZone)
{
   QDateTime dt;
   dt.setTimeZone(timeZone);
   dt.setMSecsSinceEpoch(msecs);
   return dt;
}

QDateTime QDateTime::fromString(const QString &string, Qt::DateFormat format)
{
   if (string.isEmpty()) {
      return QDateTime();
   }

   switch (format) {
      case Qt::SystemLocaleDate:
      case Qt::SystemLocaleShortDate:
         return QLocale::system().toDateTime(string, QLocale::ShortFormat);

      case Qt::SystemLocaleLongDate:
         return QLocale::system().toDateTime(string, QLocale::LongFormat);

      case Qt::LocaleDate:
      case Qt::DefaultLocaleShortDate:
         return QLocale().toDateTime(string, QLocale::ShortFormat);

      case Qt::DefaultLocaleLongDate:
         return QLocale().toDateTime(string, QLocale::LongFormat);

      case Qt::RFC2822Date: {
         const ParsedRfcDateTime rfc = rfcDateImpl(string);

         if (! rfc.date.isValid() || ! rfc.time.isValid()) {
            return QDateTime();
         }

         QDateTime dateTime(rfc.date, rfc.time, Qt::UTC);
         dateTime.setOffsetFromUtc(rfc.utcOffset);
         return dateTime;
      }

      case Qt::ISODate: {
         const int size = string.size();
         if (size < 10) {
            return QDateTime();
         }

         QStringView isoString(string);
         Qt::TimeSpec spec = Qt::LocalTime;

         QDate date = QDate::fromString(string.left(10), Qt::ISODate);
         if (! date.isValid()) {
            return QDateTime();
         }
         if (size == 10) {
            return QDateTime(date);
         }

         isoString = isoString.right(isoString.length() - 11);
         int offset = 0;

         // Check end of string for Time Zone definition, either Z for UTC or [+-]HH:mm for Offset
         if (isoString.endsWith(QChar('Z'))) {
            spec = Qt::UTC;
            isoString = isoString.left(isoString.size() - 1);

         } else {
            // the loop below is faster but functionally equal to:
            // const int signIndex = isoString.indexOf(QRegExp(QStringLiteral("[+-]")));
            int signIndex = isoString.size() - 1;

            bool found = false;
            {
               const QChar plus  = QChar('+');
               const QChar minus = QChar('-');

               do {
                  QChar character(isoString.at(signIndex));
                  found = character == plus || character == minus;

               } while (--signIndex >= 0 && !found);
               ++signIndex;
            }

            if (found) {
               bool ok;
               offset = fromOffsetString(isoString.mid(signIndex), &ok);
               if (! ok) {
                  return QDateTime();
               }
               isoString = isoString.left(signIndex);
               spec = Qt::OffsetFromUTC;
            }
         }

         // Might be end of day (24:00, including variants), which QTime considers invalid.
         // ISO 8601 (section 4.2.3) says that 24:00 is equivalent to 00:00 the next day.
         bool isMidnight24 = false;
         QTime time = fromIsoTimeString(isoString, Qt::ISODate, &isMidnight24);

         if (! time.isValid()) {
            return QDateTime();
         }

         if (isMidnight24) {
            date = date.addDays(1);
         }

         return QDateTime(date, time, spec, offset);
      }

#if ! defined(QT_NO_TEXTDATE)
      case Qt::TextDate: {
         QList<QStringView> parts = QStringParser::split<QStringView>(string, ' ', QStringParser::SkipEmptyParts);

         if ((parts.count() < 5) || (parts.count() > 6)) {
            return QDateTime();
         }

         // Accept "Sun Dec 1 13:02:00 1974" and "Sun 1. Dec 13:02:00 1974"
         int month = 0;
         int day   = 0;
         bool ok   = false;

         // First try month then day
         month = fromShortMonthName(parts.at(1));
         if (month) {
            day = QStringParser::toInteger<int>(parts.at(2));
         }

         // If failed try day then month
         if (! month || ! day) {
            month = fromShortMonthName(parts.at(2));

            if (month) {
               QStringView dayStr = parts.at(1);

               if (dayStr.endsWith('.')) {
                  dayStr = dayStr.left(dayStr.size() - 1);
                  day = QStringParser::toInteger<int>(dayStr);
               }
            }
         }

         // If both failed, give up
         if (! month || ! day) {
            return QDateTime();
         }

         // Year can be before or after time, "Sun Dec 1 1974 13:02:00" or "Sun Dec 1 13:02:00 1974"
         // Guess which by looking for ':' in the time
         int year     = 0;
         int yearPart = 0;
         int timePart = 0;

         if (parts.at(3).contains(':')) {
            yearPart = 4;
            timePart = 3;

         } else if (parts.at(4).contains(':')) {
            yearPart = 3;
            timePart = 4;

         } else {
            return QDateTime();
         }

         year = QStringParser::toInteger<int>(parts.at(yearPart), &ok);
         if (! ok) {
            return QDateTime();
         }

         QDate date(year, month, day);
         if (! date.isValid()) {
            return QDateTime();
         }

         QList<QStringView> timeParts = QStringParser::split(parts.at(timePart), ':');

         if (timeParts.count() < 2 || timeParts.count() > 3) {
            return QDateTime();
         }

         int hour = QStringParser::toInteger<int>(timeParts.at(0), &ok);
         if (! ok) {
            return QDateTime();
         }

         int minute = QStringParser::toInteger<int>(timeParts.at(1), &ok);
         if (! ok) {
            return QDateTime();
         }

         int second = 0;
         int millisecond = 0;

         if (timeParts.count() > 2) {
            QList<QStringView> secondParts = QStringParser::split(timeParts.at(2), '.');
            if (secondParts.size() > 2) {
               return QDateTime();
            }

            second = QStringParser::toInteger<int>(secondParts.first(), &ok);
            if (! ok) {
               return QDateTime();
            }

            if (secondParts.size() > 1) {
               millisecond = QStringParser::toInteger<int>(secondParts.last(), &ok);
               if (! ok) {
                  return QDateTime();
               }
            }
         }

         QTime time(hour, minute, second, millisecond);
         if (!time.isValid()) {
            return QDateTime();
         }

         if (parts.count() == 5) {
            return QDateTime(date, time, Qt::LocalTime);
         }

         QStringView tz = parts.at(5);

         if (! tz.startsWith("GMT", Qt::CaseInsensitive)) {
            return QDateTime();
         }

         tz = tz.mid(3);
         if (! tz.isEmpty()) {
            int offset = fromOffsetString(tz, &ok);

            if (!ok) {
               return QDateTime();
            }
            return QDateTime(date, time, Qt::OffsetFromUTC, offset);

         } else {
            return QDateTime(date, time, Qt::UTC);
         }
      }
#endif //QT_NO_TEXTDATE
   }

   return QDateTime();
}

QDateTime QDateTime::fromString(const QString &string, const QString &format)
{
   QTime time;
   QDate date;

   QDateTimeParser dt(QVariant::DateTime, QDateTimeParser::FromString);

   if (dt.parseFormat(format) && dt.fromString(string, &date, &time)) {
      return QDateTime(date, time);
   }

   return QDateTime(QDate(), QTime(-1, -1, -1));
}

QDataStream &operator<<(QDataStream &stream, const QDate &date)
{
   return stream << qint64(date.jd);
}

QDataStream &operator>>(QDataStream &stream, QDate &date)
{
   qint64 jd;
   stream >> jd;
   date.jd = jd;

   return stream;
}

QDataStream &operator<<(QDataStream &stream, const QTime &time)
{
   return stream << quint32(time.mds);
}

QDataStream &operator>>(QDataStream &stream, QTime &time)
{
   quint32 ds;
   stream >> ds;

   time.mds = int(ds);

   return stream;
}

QDataStream &operator<<(QDataStream &stream, const QDateTime &dateTime)
{
   QPair<QDate, QTime> dateAndTime;
   // switched to using Qt::TimeSpec and added offset support

   dateAndTime = dateTime.d->getDateTime();
   stream << dateAndTime << qint8(dateTime.timeSpec());

   if (dateTime.timeSpec() == Qt::OffsetFromUTC) {
      stream << qint32(dateTime.offsetFromUtc());

   } else if (dateTime.timeSpec() == Qt::TimeZone) {
      stream << dateTime.timeZone();
   }

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QDateTime &dateTime)
{
   QDate dt;
   QTime tm;
   qint8 ts = 0;

   Qt::TimeSpec spec = Qt::LocalTime;
   qint32 offset = 0;

   QTimeZone tz;

   // switched to using Qt::TimeSpec and added offset support
   stream >> dt >> tm >> ts;
   spec = static_cast<Qt::TimeSpec>(ts);

   if (spec == Qt::OffsetFromUTC) {
      stream >> offset;
      dateTime = QDateTime(dt, tm, spec, offset);

   } else if (spec == Qt::TimeZone) {
      stream >> tz;
      dateTime = QDateTime(dt, tm, tz);

   } else {
      dateTime = QDateTime(dt, tm, spec);
   }

   return stream;
}

QDebug operator<<(QDebug dbg, const QDate &date)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QDate(" << date.toString(Qt::ISODate) << ')';
   return dbg;
}

QDebug operator<<(QDebug dbg, const QTime &time)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QTime(" << time.toString("HH:mm:ss.zzz") << ')';
   return dbg;
}

QDebug operator<<(QDebug dbg, const QDateTime &date)
{
   QDebugStateSaver saver(dbg);
   const Qt::TimeSpec ts = date.timeSpec();

   dbg.nospace() << "QDateTime(";
   dbg.noquote() << date.toString("yyyy-MM-dd HH:mm:ss.zzz t")
      << ' ' << ts;

   switch (ts) {
      case Qt::UTC:
         break;
      case Qt::OffsetFromUTC:
         dbg << ' ' << date.offsetFromUtc() << 's';
         break;
      case Qt::TimeZone:
         dbg << ' ' << date.timeZone().id();
         break;
      case Qt::LocalTime:
         break;
   }
   return dbg << ')';
}

uint qHash(const QDateTime &key, uint seed)
{
   // Use to toMSecsSinceEpoch instead of individual qHash functions for
   // QDate/QTime/spec/offset because QDateTime::operator== converts both arguments
   // to the same timezone. If we don't qHash would return different hashes for
   // two QDateTimes that are equivalent once converted to the same timezone.
   return qHash(key.toMSecsSinceEpoch(), seed);
}

uint qHash(const QDate &key, uint seed)
{
   return qHash(key.toJulianDay(), seed);
}

uint qHash(const QTime &key, uint seed)
{
   return qHash(key.msecsSinceStartOfDay(), seed);
}
