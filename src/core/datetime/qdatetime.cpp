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

#include <qdatetime.h>

#include <qdatastream.h>
#include <qdebug.h>
#include <qregularexpression.h>
#include <qset.h>
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

static const char monthDays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static QDate fixedDate(int y, int m, int d)
{
   QDate result(y, m, 1);
   result.setDate(y, m, qMin(d, result.daysInMonth()));

   return result;
}

static qint64 floordiv(qint64 a, int b)
{
   return (a - (a < 0 ? b - 1 : 0)) / b;
}

static int floordiv(int a, int b)
{
   return (a - (a < 0 ? b - 1 : 0)) / b;
}

static qint64 julianDayFromDate(int year, int month, int day)
{
   // Adjust for year 0
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
      --year;
   }

   const ParsedDate result = { year, month, day };

   return result;
}

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
   // Assume English monthnames are the default
   int month = qt_monthNumberFromShortName(monthName);

   if (month != -1) {
      return month;
   }

   // If English names can not be found search the localized ones
   for (int i = 1; i <= 12; ++i) {
      if (monthName == QDate::shortMonthName(i)) {
         return i;
      }
   }

   return -1;
}

struct ParsedRfcDateTime {
   QDate date;
   QTime time;
   int utcOffset;
};

static ParsedRfcDateTime rfcDateImpl(const QString &s)
{
   ParsedRfcDateTime result;

   // Matches "Wdy, DD Mon YYYY HH:mm:ss ±hhmm" (Wdy, being optional)
   static QRegularExpression regexp("^(?:[A-Z][a-z]+,)?[ \\t]*(\\d{1,2})[ \\t]+([A-Z][a-z]+)[ \\t]+(\\d\\d\\d\\d)(?:[ \\t]+(\\d\\d):(\\d\\d)(?::(\\d\\d))?)?[ \\t]*(?:([+-])(\\d\\d)(\\d\\d))?");

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
      static QRegularExpression regexp("^[A-Z][a-z]+[ \\t]+([A-Z][a-z]+)[ \\t]+(\\d\\d)(?:[ \\t]+(\\d\\d):(\\d\\d):(\\d\\d))?[ \\t]+(\\d\\d\\d\\d)[ \\t]*(?:([+-])(\\d\\d)(\\d\\d))?");

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

   if (! ok) {
      return 0;
   }

   const int minute = QStringParser::toInteger<int>(parts.at(1), &ok);

   if (! ok || minute < 0 || minute > 59) {
      return 0;
   }

   *valid = true;

   return sign * ((hour * 60) + minute) * 60;
}

// ** QDate

QDate::QDate(int y, int m, int d)
{
   setDate(y, m, d);
}

QDate QDate::addDays(qint64 ndays) const
{
   if (isNull()) {
      return QDate();
   }

   // Due to limits on minJd() and maxJd() any overflow
   // will be invalid and caught by fromJulianDay()

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

   int old_y;
   int y;
   int m;
   int d;

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
         --y;
         nmonths += 12;

      } else if (nmonths < 0) {
         m += nmonths;
         nmonths = 0;

         if (m <= 0) {
            --y;
            m += 12;
         }

      } else if (nmonths - 12 >= 0) {
         ++y;
         nmonths -= 12;

      } else if (m == 12) {
         ++y;
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
   if ((old_y > 0 && y <= 0) || (old_y < 0 && y >= 0)) {
      // adjust the date by plus 1 or minus 1 years
      y += increasing ? +1 : -1;
   }

   return fixedDate(y, m, d);
}

QDate QDate::addYears(qint64 nyears) const
{
   if (! isValid()) {
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

// undocumented
const QTimeZone &QDate::default_tz()
{
   static QTimeZone retval = QTimeZone::systemTimeZone();

   return retval;
}

QDateTime QDate::endOfDay(const QTimeZone &zone) const
{
   if (! isValid() || ! zone.isValid()) {
      return QDateTime();
   }

   QDateTime retval = QDateTime(*this, QTime(23, 59, 59, 999), zone);

   if (retval.isValid()) {
      return retval;
   }

   // need to look up more IANN information
   // time zone has a DST change at midnight or not one hour

   return retval;
}

int QDate::month() const
{
   if (isNull()) {
      return 0;
   }

   return getDateFromJulianDay(jd).month;
}

int QDate::weekNumber(int *yearNumber) const
{
   if (! isValid()) {
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

int QDate::year() const
{
   if (isNull()) {
      return 0;
   }

   return getDateFromJulianDay(jd).year;
}

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

static QString toStringTextDate(QDate date)
{
   const ParsedDate pd = getDateFromJulianDay(date.toJulianDay());
   static const QChar sp(' ');

   return date.shortDayName(date.dayOfWeek()) + sp
         + date.shortMonthName(pd.month) + sp + QString::number(pd.day) + sp + QString::number(pd.year);
}

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
         [[fallthrough]];

      case Qt::TextDate:
         return toStringTextDate(*this);

      case Qt::ISODate:
         return toStringIsoDate(jd);
   }
}

QString QDate::toString(const QString &format) const
{
   return QLocale::system().toString(*this, format);
}

void QDate::getDate(int *year, int *month, int *day) const
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

bool QDate::setDate(int year, int month, int day)
{
   if (isValid(year, month, day)) {
      jd = julianDayFromDate(year, month, day);

   } else {
      jd = INVALID_JD;
   }

   return isValid();
}

QDateTime QDate::startOfDay(const QTimeZone &zone) const
{
   if (! isValid() || ! zone.isValid()) {
      return QDateTime();
   }

   QDateTime retval = QDateTime(*this, QTime(0, 0, 0, 0), zone);

   if (retval.isValid()) {
      return retval;
   }

   // need to look up more IANN information
   // time zone has a DST change at midnight or not one hour

   return QDateTime();
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
         [[fallthrough]];

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

         if (! ok) {
            return QDate();
         }

         return QDate(year, month, QStringParser::toInteger<int>(parts.at(2)));
      }

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

bool QDate::isLeapYear(int y)
{
   // No year 0 in Gregorian calendar, so -1, -5, -9 etc are leap years
   if ( y < 1) {
      ++y;
   }

   return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
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

// ** QTime

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
   if (! isValid()) {
      return -1;
   }

   return (ds() / MSECS_PER_SEC) % SECS_PER_MIN;
}

int QTime::msec() const
{
   if (! isValid()) {
      return -1;
   }

   return ds() % MSECS_PER_SEC;
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

         return result.formatArg(hour(), 2, 10, '0').formatArg(minute(), 2, 10, '0').formatArg(second(), 2, 10, '0');
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

   mds = (h * SECS_PER_HOUR + m * SECS_PER_MIN + s) * MSECS_PER_SEC + ms;

   return true;
}

QTime QTime::addSecs(int s) const
{
   s %= SECS_PER_DAY;
   return addMSecs(s * MSECS_PER_SEC);
}

int QTime::secsTo(const QTime &t) const
{
   if (! isValid() || ! t.isValid()) {
      return 0;
   }

   // Truncate milliseconds, do not want to consider them
   int ourSeconds   = ds() / MSECS_PER_SEC;
   int theirSeconds = t.ds() / MSECS_PER_SEC;

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

   if (! ok) {
      return QTime();
   }

   const int minute = QStringParser::toInteger<int>(string.mid(3, 2), &ok);

   if (! ok) {
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

      if (! ok) {
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

      if (! ok) {
         return QTime();
      }

      if (size > 8 && (string.at(8) == ',' || string.at(8) == '.')) {
         const QStringView msecStr(string.mid(9, 4));

         int msecInt = msecStr.isEmpty() ? 0 : QStringParser::toInteger<int>(msecStr, &ok);

         if (! ok) {
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
      n += MSECS_PER_DAY;
   }

   *this = t;

   return n;
}

int QTime::elapsed() const
{
   int n = msecsTo(currentTime());

   if (n < 0) {                              // passed midnight
      n += MSECS_PER_DAY;
   }

   return n;
}

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

// Calls the platform variant of mktime for the given date, time, and daylightStatus,
// and updates the date, time, daylightStatus and abbreviation with the returned values.

// If the date falls outside the 1970 to 2037 range supported by mktime a
// then null date/time will be returned and you should adjust the date first if
// you need a guaranteed result.

static qint64 qt_mktime(QDate *date, QTime *time, QDateTimePrivate::DaylightStatus *daylightStatus,
      QString *abbreviation, bool *ok = nullptr)
{
   const qint64 msec = time->msec();
   int yy;
   int mm;
   int dd;

   date->getDate(&yy, &mm, &dd);

   // All other platforms provide standard C library time functions
   tm local = {};

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
         if (daylightStatus != nullptr) {
            *daylightStatus = QDateTimePrivate::DaylightTime;
         }

         if (abbreviation != nullptr) {
            *abbreviation = qt_tzname(QDateTimePrivate::DaylightTime);
         }

      } else if (local.tm_isdst == 0) {
         if (daylightStatus != nullptr) {
            *daylightStatus = QDateTimePrivate::StandardTime;
         }

         if (abbreviation != nullptr) {
            *abbreviation = qt_tzname(QDateTimePrivate::StandardTime);
         }

      } else {
         if (daylightStatus != nullptr) {
            *daylightStatus = QDateTimePrivate::UnknownDaylightTime;
         }

         if (abbreviation != nullptr) {
            *abbreviation = qt_tzname(QDateTimePrivate::StandardTime);
         }
      }

      if (ok) {
         *ok = true;
      }

   } else {
      *date = QDate();
      *time = QTime();

      if (daylightStatus != nullptr) {
         *daylightStatus = QDateTimePrivate::UnknownDaylightTime;
      }

      if (abbreviation != nullptr) {
         *abbreviation = QString();
      }

      if (ok) {
         *ok = false;
      }
   }

   return ((qint64)secsSinceEpoch * MSECS_PER_SEC) + msec;
}

// Calls the platform variant of localtime for the given msecs, and updates
// the date, time, and DST status with the returned values.
static bool qt_localtime(qint64 msecsSinceEpoch, QDate *localDate, QTime *localTime,
      QDateTimePrivate::DaylightStatus *daylightStatus)
{
   const time_t secsSinceEpoch = msecsSinceEpoch / MSECS_PER_SEC;
   const int msec = msecsSinceEpoch % MSECS_PER_SEC;

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

// Converts msecs value into a date and time
static void msecsToTime(qint64 msecs, QDate *date, QTime *time)
{
   qint64 jd = EPOCH_JD;
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
   return ((date.toJulianDay() - EPOCH_JD) * MSECS_PER_DAY)
         + time.msecsSinceStartOfDay();
}

// Convert an MSecs Since Epoch into Local Time
static bool epochMSecsToLocalTime(qint64 msecs, QDate *localDate, QTime *localTime,
      QDateTimePrivate::DaylightStatus *daylightStatus = nullptr)
{
   if (msecs < 0) {
      // any LocalTime before 1970-01-01 will *not* have any Daylight Time applied
      // use the standard offset from UTC to convert to UTC time

      qt_tzset();
      msecsToTime(msecs - qt_timezone() * MSECS_PER_SEC, localDate, localTime);

      if (daylightStatus) {
         *daylightStatus = QDateTimePrivate::StandardTime;
      }

      return true;

   } else {

      if constexpr (sizeof(time_t) < sizeof(qint64)) {
         if (msecs > (MAX_C_TIME * MSECS_PER_SEC)) {
            // any LocalTime after 2037-12-31 *will* have any DST applied
            // but this may fall outside the supported time_t range

            QDate utcDate;
            QTime utcTime;

            msecsToTime(msecs, &utcDate, &utcTime);
            int year, month, day;
            utcDate.getDate(&year, &month, &day);

            // 2037 is not a leap year, so make sure date is not Feb 29
            if (month == 2 && day == 29) {
               --day;
            }

            QDate fakeDate(2037, month, day);
            qint64 fakeMsecs = QDateTime(fakeDate, utcTime, QTimeZone::utc()).toMSecsSinceEpoch();

            bool retval = qt_localtime(fakeMsecs, localDate, localTime, daylightStatus);
            *localDate = localDate->addDays(fakeDate.daysTo(utcDate));

            return retval;
         }
      }

      // value of msecs fits in the data type time_t
      return qt_localtime(msecs, localDate, localTime, daylightStatus);
   }
}

// Convert a LocalTime expressed in local msecs encoding and the corresponding
// DST status into a UTC epoch msecs. Optionally populate the returned
// values from mktime for the adjusted local date and time.

static qint64 localMSecsToEpochMSecs(qint64 localMsecs, QDateTimePrivate::DaylightStatus *daylightStatus,
      QDate *localDate = nullptr, QTime *localTime = nullptr, QString *abbreviation = nullptr)
{
   QDate dt;
   QTime tm;

   msecsToTime(localMsecs, &dt, &tm);

   if (localMsecs <= MSECS_PER_DAY) {
      // any LocalTime before 1970-01-01 will *not* have any DST applied

      // if localMsecs is within +/- 1 day of minimum time_t try mktime in case it does
      // fall after minimum and needs proper DST conversion

      if (localMsecs >= (0 - MSECS_PER_DAY)) {
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
         // If we do not call mktime then need to call tzset to get offset
         qt_tzset();
      }

      // Time is before 1970-01-01, use standard offset to convert
      qint64 utcMsecs = localMsecs + qt_timezone() * MSECS_PER_SEC;

      if (localDate || localTime) {
         msecsToTime(localMsecs, localDate, localTime);
      }

      if (daylightStatus != nullptr) {
         *daylightStatus = QDateTimePrivate::StandardTime;
      }

      if (abbreviation != nullptr) {
         *abbreviation = qt_tzname(QDateTimePrivate::StandardTime);
      }

      return utcMsecs;

   } else {

      if constexpr (sizeof(time_t) < sizeof(qint64)) {

         const qint64 msecsMax = qint64(MAX_C_TIME) * MSECS_PER_SEC;

         if (localMsecs >= msecsMax - MSECS_PER_DAY) {
            // Any LocalTime after 2037-12-31 *will* have any DST applied
            // but this may fall outside the supported time_t range

            // if localMsecs is within +/- 1 day of maximum time_t try mktime in case it does
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

            int year, month, day;
            dt.getDate(&year, &month, &day);

            // 2037 is not a leap year so make sure date is not Feb 29
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
         }
      }

      // inside 1970-2037 suported range
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

// ** QDateTimePrivate

QDateTimePrivate::QDateTimePrivate(const QDate &toDate, const QTime &toTime, const QTimeZone &toTimeZone)
   : m_timeZone(toTimeZone), m_tzUserDefined(false), m_status(Qt::EmptyFlag)
{
   setDateTime(toDate, toTime);
}

void QDateTimePrivate::setDateTime(const QDate &date, const QTime &time)
{
   // if the date is valid and the time is not, set time to 00:00:00
   QTime useTime = time;

   if (! useTime.isValid() && date.isValid()) {
      useTime = QTime::fromMSecsSinceStartOfDay(0);
   }

   StatusFlags newStatus;

   // Set date value and status
   qint64 days = 0;

   if (date.isValid()) {
      days = date.toJulianDay() - EPOCH_JD;
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
   m_msecs  = (days * MSECS_PER_DAY) + ds;
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

void QDateTimePrivate::checkValidDateTime()
{
   refreshDateTime();
}

void QDateTimePrivate::refreshDateTime()
{
   if (! isValidDate() || ! isValidTime()) {
      clearValidDateTime();
      return;
   }

   if (m_tzUserDefined && ! m_timeZone.isValid()) {
      clearValidDateTime();
      return;
   }

   QDate testDate;
   QTime testTime;

   if (m_tzUserDefined) {
      zoneMSecsToEpochMSecs(m_msecs, m_timeZone, &testDate, &testTime);

   } else {
      auto status = daylightStatus();
      localMSecsToEpochMSecs(m_msecs, &status, &testDate, &testTime);
   }

   if (timeToMSecs(testDate, testTime) == m_msecs) {
      setValidDateTime();
   } else {
      clearValidDateTime();
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
      msecsToTime(data.atMSecsSinceEpoch + (data.offsetFromUtc * MSECS_PER_SEC), localDate, localTime);
      return data.atMSecsSinceEpoch;

   } else {
      msecsToTime(zoneMSecs, localDate, localTime);
      return zoneMSecs - (data.standardTimeOffset * MSECS_PER_SEC);
   }
}

QDateTime::QDateTime()
   : d(new QDateTimePrivate)
{
}

QDateTime::QDateTime(const QDate &date)
   : d(new QDateTimePrivate(date, QTime(0, 0, 0), QTimeZone()))
{
   d->m_tzUserDefined = false;
}

QDateTime::QDateTime(const QDate &date, const QTime &time, std::optional<QTimeZone> timeZone)
   : d(new QDateTimePrivate(date, time, timeZone.value_or(QTimeZone())))
{
   d->m_tzUserDefined = timeZone.has_value();
}

QDateTime::QDateTime(const QDateTime &other)
   : d(new QDateTimePrivate(*other.d))
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
   if (other.d == nullptr) {
      d.reset();

   } else {
      if (d == nullptr) {
         d = QMakeUnique<QDateTimePrivate>(*other.d);

      } else {
         *d = *other.d;
      }
   }

   return *this;
}

QDateTime QDateTime::addDuration(std::chrono::milliseconds msecs) const
{
   return addMSecs(msecs.count());
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

bool QDateTime::isNull() const
{
   return d->isNullDate() && d->isNullTime();
}

bool QDateTime::isValid() const
{
   return (d->isValidDateTime());
}

bool QDateTime::isDaylightTime() const
{
   return d->m_timeZone.d->isDaylightTime(toMSecsSinceEpoch());
}

int QDateTime::offsetFromUtc() const
{
   return d->m_timeZone.offsetFromUtc(*this);
}

void QDateTime::setDate(const QDate &date)
{
   d->setDateTime(date, time());
}

void QDateTime::setMSecsSinceEpoch(qint64 msecs)
{
   d->m_status = Qt::EmptyFlag;

   if (d->m_tzUserDefined) {
      int offset;

      if (msecs >= 0) {
         offset = d->m_timeZone.d->offsetFromUtc(msecs);
      } else {
         offset = d->m_timeZone.d->standardTimeOffset(msecs);
      }

      d->m_msecs = msecs + (offset * MSECS_PER_SEC);

      d->m_status = d->m_status | QDateTimePrivate::ValidDate |
            QDateTimePrivate::ValidTime | QDateTimePrivate::ValidDateTime;

   } else {
      QDate localDate;
      QTime localTime;
      QDateTimePrivate::DaylightStatus status;

      epochMSecsToLocalTime(msecs, &localDate, &localTime, &status);

      d->setDateTime(localDate, localTime);
      d->setDaylightStatus(status);
   }

   d->refreshDateTime();
}

void QDateTime::setSecsSinceEpoch(qint64 seconds)
{
   setMSecsSinceEpoch(seconds * MSECS_PER_SEC);
}

void QDateTime::setTime(const QTime &time)
{
   d->setDateTime(date(), time);
}

void QDateTime::setTimeZone(const QTimeZone &toZone)
{
   d->m_timeZone = toZone;
   d->m_tzUserDefined = true;

   d->refreshDateTime();
}

void QDateTime::setTime_t(quint64 seconds)
{
   setMSecsSinceEpoch((qint64)seconds * MSECS_PER_SEC);
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

QTimeZone QDateTime::timeRepresentation() const
{
   return timeZone();
}

QTimeZone QDateTime::timeZone() const
{
   Q_ASSERT_X(d->m_timeZone.isValid(), "QDateTime::timeZone()", "QDateTime timeZone is invalid");

   return d->m_timeZone;
}

QString QDateTime::timeZoneAbbreviation() const
{
   Q_ASSERT_X(isValid(), "QDateTime::timeZoneAbbreviation()", "QDateTime is invalid");

   Q_ASSERT_X(d->m_timeZone.isValid(), "QDateTime::timeZoneAbbreviation()",
         "QDateTime timeZone is invalid");

   return d->m_timeZone.d->abbreviation(toMSecsSinceEpoch());
}

QDateTime QDateTime::toLocalTime() const
{
   return toTimeZone(QTimeZone::systemTimeZone());
}

qint64 QDateTime::toMSecsSinceEpoch() const
{
   if (d->m_tzUserDefined) {
      return QDateTimePrivate::zoneMSecsToEpochMSecs(d->m_msecs, d->m_timeZone);

   } else {
      auto status = d->daylightStatus();
      return localMSecsToEpochMSecs(d->m_msecs, &status);
   }
}

qint64 QDateTime::toSecsSinceEpoch() const
{
   return toMSecsSinceEpoch() / MSECS_PER_SEC;
}

QDateTime QDateTime::toOffsetFromUtc(qint64 offsetSeconds) const
{
   if (offsetFromUtc() == offsetSeconds) {
      return *this;
   }

   if (! isValid()) {
      QDateTime dt = *this;
      dt.setTimeZone(QTimeZone(offsetSeconds));

      return dt;
   }

   return fromMSecsSinceEpoch(toMSecsSinceEpoch(), QTimeZone(offsetSeconds));
}

QString QDateTime::toString(Qt::DateFormat format) const
{
   QString retval;

   if (! isValid()) {
      return retval;
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
         retval = QLocale::c().toString(*this, "dd MMM yyyy hh:mm:ss ");
         retval += toOffsetString(Qt::TextDate, offsetFromUtc());

         return retval;
      }

      default:
         [[fallthrough]];

      case Qt::TextDate: {
         const QPair<QDate, QTime> p = d->getDateTime();
         const QDate &dt = p.first;
         const QTime &tm = p.second;

         // can not use date.toString(Qt::TextDate) since we need to insert the time before the year
         retval = QString("%1 %2 %3 %4 %5").formatArg(dt.shortDayName(dt.dayOfWeek()))
               .formatArg(dt.shortMonthName(dt.month()))
               .formatArg(dt.day())
               .formatArg(tm.toString(Qt::TextDate))
               .formatArg(dt.year());

         if (! d->m_tzUserDefined) {
            // defaulted time zone

         } else {
            retval += " UTC";

            int offset = offsetFromUtc();

            if (offset != 0) {
               retval += toOffsetString(Qt::TextDate, offset);
            }
         }

         return retval;
      }

      case Qt::ISODate: {
         const QPair<QDate, QTime> p = d->getDateTime();
         const QDate &dt = p.first;
         const QTime &tm = p.second;

         retval = dt.toString(Qt::ISODate);

         if (retval.isEmpty()) {
            return QString();   // failed to convert
         }

         retval += 'T';
         retval += tm.toString(Qt::ISODate);

         if (! d->m_tzUserDefined) {
            // defaulted time zone

         } else if (timeZone() == QTimeZone::utc()) {
            retval += 'Z';

         } else  {
            retval += toOffsetString(Qt::ISODate, offsetFromUtc());

         }

         return retval;
      }
   }
}

QString QDateTime::toString(const QString &format) const
{
   return QLocale::system().toString(*this, format);
}

quint64 QDateTime::toTime_t() const
{
   if (! isValid()) {
      return quint64(-1);
   }

   qint64 retval = toMSecsSinceEpoch() / MSECS_PER_SEC;

   if (quint64(retval) >= Q_UINT64_C(0xFFFFFFFF)) {
      return quint64(-1);
   }

   return quint64(retval);
}

QDateTime QDateTime::toTimeZone(const QTimeZone &timeZone) const
{
   if (d->m_timeZone == timeZone) {
      return *this;
   }

   if (! isValid()) {
      QDateTime dt = *this;
      dt.setTimeZone(timeZone);

      return dt;
   }

   return fromMSecsSinceEpoch(toMSecsSinceEpoch(), timeZone);
}

QDateTime QDateTime::toUTC() const
{
   return toTimeZone(QTimeZone::utc());
}

static void massageAdjustedDateTime(const QTimeZone &zone, QDate *date, QTime *time)
{
   if (zone.isValid()) {
      QDateTimePrivate::zoneMSecsToEpochMSecs(timeToMSecs(*date, *time), zone, date, time);

   } else {
      auto status = QDateTimePrivate::UnknownDaylightTime;
      localMSecsToEpochMSecs(timeToMSecs(*date, *time), &status, date, time);
   }
}

QDateTime QDateTime::addDays(qint64 ndays) const
{
   QDateTime dt(*this);

   QPair<QDate, QTime> p = d->getDateTime();
   QDate &date = p.first;
   QTime &time = p.second;

   date = date.addDays(ndays);
   massageAdjustedDateTime(d->m_timeZone, &date, &time);
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
   massageAdjustedDateTime(d->m_timeZone, &date, &time);
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
   massageAdjustedDateTime(d->m_timeZone, &date, &time);
   dt.d->setDateTime(date, time);

   return dt;
}

QDateTime QDateTime::addSecs(qint64 s) const
{
   return addMSecs(s * MSECS_PER_SEC);
}

QDateTime QDateTime::addMSecs(qint64 msecs) const
{
   if (! isValid()) {
      return QDateTime();
   }

   QDateTime dt(*this);

   // Convert to real UTC first in case it crosses DST transition
   dt.setMSecsSinceEpoch(toMSecsSinceEpoch() + msecs);

   return dt;
}

qint64 QDateTime::daysTo(const QDateTime &other) const
{
   return date().daysTo(other.date());
}

qint64 QDateTime::secsTo(const QDateTime &other) const
{
   return (msecsTo(other) / MSECS_PER_SEC);
}

qint64 QDateTime::msecsTo(const QDateTime &other) const
{
   if (! isValid() || ! other.isValid()) {
      return 0;
   }

   return other.toMSecsSinceEpoch() - toMSecsSinceEpoch();
}

bool QDateTime::operator==(const QDateTime &other) const
{
   if (! isValid() && ! other.isValid()) {
      return true;
   }

   if (! isValid() || ! other.isValid()) {
      return false;
   }

   if (d->m_timeZone == other.d->m_timeZone && d->m_status == other.d->m_status) {
      return (d->m_msecs == other.d->m_msecs);
   }

   // convert to UTC and compare
   return (toMSecsSinceEpoch() == other.toMSecsSinceEpoch());
}

bool QDateTime::operator<(const QDateTime &other) const
{
   if (d->m_timeZone == other.d->m_timeZone && d->m_status == other.d->m_status) {
      return (d->m_msecs < other.d->m_msecs);
   }

   // convert to UTC and compare
   return (toMSecsSinceEpoch() < other.toMSecsSinceEpoch());
}

#if defined(Q_OS_WIN)
static uint msecsFromDecomposed(int hour, int minute, int sec, int msec = 0)
{
   return MSECS_PER_HOUR * hour + MSECS_PER_MIN * minute + MSECS_PER_SEC * sec + msec;
}

QDate QDate::currentDate()
{
   QDate retval;

   SYSTEMTIME st = {};
   GetLocalTime(&st);

   retval.jd = julianDayFromDate(st.wYear, st.wMonth, st.wDay);

   return retval;
}

QTime QTime::currentTime()
{
   QTime retval;

   SYSTEMTIME st = {};
   GetLocalTime(&st);

   retval.setHMS(st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

   return retval;
}

QDateTime QDateTime::currentDateTime(const QTimeZone &zone)
{
   SYSTEMTIME st = {};
   GetSystemTime(&st);

   QDate d(st.wYear, st.wMonth, st.wDay);
   QTime t(msecsFromDecomposed(st.wHour, st.wMinute, st.wSecond, st.wMilliseconds));

   QDateTime retval(d, t, QTimeZone::utc());

   if (zone != QTimeZone::utc()) {
      retval = retval.toTimeZone(zone);
   }

   return retval;
}

qint64 QDateTime::currentMSecsSinceEpoch()
{
   SYSTEMTIME st = {};
   GetSystemTime(&st);

   return msecsFromDecomposed(st.wHour, st.wMinute, st.wSecond, st.wMilliseconds) +
         qint64(julianDayFromDate(st.wYear, st.wMonth, st.wDay) -
         julianDayFromDate(1970, 1, 1)) * MSECS_PER_DAY;
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

QDateTime QDateTime::currentDateTime(const QTimeZone &zone)
{
   return fromMSecsSinceEpoch(currentMSecsSinceEpoch(), zone);
}

qint64 QDateTime::currentMSecsSinceEpoch()
{
   struct timeval tv;
   gettimeofday(&tv, nullptr);

   return qint64(tv.tv_sec) * MSECS_PER_SEC + tv.tv_usec / MSECS_PER_SEC;
}

#else
#error "Unknown Operating System type"

#endif

// ** QDateTime

QDateTime QDateTime::currentDateTimeUtc()
{
   return currentDateTime(QTimeZone::utc());
}

qint64 QDateTime::currentSecsSinceEpoch()
{
   return currentMSecsSinceEpoch() / MSECS_PER_SEC;
}

QDateTime QDateTime::fromTime_t(qint64 seconds, const QTimeZone &timeZone)
{
   return fromMSecsSinceEpoch(seconds * MSECS_PER_SEC, timeZone);
}

QDateTime QDateTime::fromMSecsSinceEpoch(qint64 msecs, const QTimeZone &timeZone)
{
   QDateTime dt;

   dt.setTimeZone(timeZone);
   dt.setMSecsSinceEpoch(msecs);

   return dt;
}

QDateTime QDateTime::fromSecsSinceEpoch(qint64 seconds, const QTimeZone &timeZone)
{
   QDateTime dt;

   dt.setTimeZone(timeZone);
   dt.setMSecsSinceEpoch(seconds * MSECS_PER_SEC);

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

         QDateTime dateTime(rfc.date, rfc.time, QTimeZone(rfc.utcOffset));

         return dateTime;
      }

      case Qt::ISODate: {
         const int size = string.size();

         if (size < 10) {
            return QDateTime();
         }

         QStringView isoString(string);
         QTimeZone timeZone = QTimeZone::systemTimeZone();

         QDate date = QDate::fromString(string.left(10), Qt::ISODate);

         if (! date.isValid()) {
            return QDateTime();
         }

         if (size == 10) {
            return QDateTime(date);
         }

         isoString = isoString.right(isoString.length() - 11);

         if (isoString.endsWith('Z')) {
            timeZone  = QTimeZone::utc();
            isoString = isoString.left(isoString.size() - 1);

         } else {
            // loop below is faster but functionally equal to:
            // const int signIndex = isoString.indexOf(QRegExp("[+-]"));

            int signIndex = isoString.size() - 1;
            bool found    = false;

            {
               const QChar plus  = QChar('+');
               const QChar minus = QChar('-');

               do {
                  QChar character(isoString.at(signIndex));
                  found = character == plus || character == minus;

               } while (--signIndex >= 0 && ! found);

               ++signIndex;
            }

            if (found) {
               bool ok;
               int offset = fromOffsetString(isoString.mid(signIndex), &ok);

               if (! ok) {
                  return QDateTime();
               }

               isoString = isoString.left(signIndex);
               timeZone = QTimeZone(offset);
            }
         }

         // Might be end of day (24:00, including variants) which QTime considers invalid
         // ISO 8601 (section 4.2.3) says 24:00 is equivalent to 00:00 the next day

         bool isMidnight24 = false;
         QTime time = fromIsoTimeString(isoString, Qt::ISODate, &isMidnight24);

         if (! time.isValid()) {
            return QDateTime();
         }

         if (isMidnight24) {
            date = date.addDays(1);
         }

         return QDateTime(date, time, timeZone);
      }

      case Qt::TextDate: {
         QList<QStringView> parts = QStringParser::split<QStringView>(string, ' ', QStringParser::SkipEmptyParts);

         if ((parts.count() < 5) || (parts.count() > 6)) {
            return QDateTime();
         }

         // accept "Sun Dec 1 13:02:00 1974" and "Sun 1. Dec 13:02:00 1974"
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

         // If both fail give up
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

         int second      = 0;
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
            return QDateTime(date, time, QTimeZone::systemTimeZone());
         }

         QStringView tz = parts.at(5);

         if (! tz.startsWith("GMT", Qt::CaseInsensitive)) {
            return QDateTime();
         }

         tz = tz.mid(3);

         if (! tz.isEmpty()) {
            int offset = fromOffsetString(tz, &ok);

            if (! ok) {
               return QDateTime();
            }

            return QDateTime(date, time, QTimeZone(offset));

         } else {
            return QDateTime(date, time, QTimeZone::utc());
         }
      }
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

   dateAndTime = dateTime.d->getDateTime();
   stream << dateAndTime << dateTime.timeZone();

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QDateTime &dateTime)
{
   QDate dt;
   QTime tm;

   QTimeZone tz;

   stream >> dt >> tm >> tz;
   dateTime = QDateTime(dt, tm, tz);

   return stream;
}

QDebug operator<<(QDebug debug, const QDate &date)
{
   QDebugStateSaver saver(debug);
   debug.nospace();

   debug << "QDate(" << date.toString(Qt::ISODate) << ')';

   return debug;
}

QDebug operator<<(QDebug debug, const QTime &time)
{
   QDebugStateSaver saver(debug);
   debug.nospace();

   debug << "QTime(" << time.toString("HH:mm:ss.zzz") << ')';

   return debug;
}

QDebug operator<<(QDebug debug, const QDateTime &date)
{
   QDebugStateSaver saver(debug);
   debug.nospace();
   debug.noquote();

   debug << "QDateTime(";
   debug << date.toString("yyyy-MM-dd HH:mm:ss.zzz t");
   debug << ' ' << date.timeZone().id() << ')';

   return debug;
}

uint qHash(const QDateTime &key, uint seed)
{
   // Use toMSecsSinceEpoch() instead of individual qHash functions for
   // QDate/QTime/spec/offset because QDateTime::operator== converts both arguments
   // to the same timezone. If we do not, qHash would return different hashes for
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
