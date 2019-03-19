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

#include <qdatetime_p.h>
#include <qdatetime.h>

#include <qplatformdefs.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qlocale.h>
#include <qset.h>
#include <qregularexpression.h>
#include <qstringparser.h>

#ifdef Q_OS_WIN32
#include <qt_windows.h>
#endif

#ifndef Q_OS_WIN
#include <locale.h>
#endif

#include <time.h>

#ifdef QDATETIMEPARSER_DEBUG
#  define QDT_DEBUG  qDebug() << QString("%1:%2").formatArg(__FILE__).formatArg(__LINE__)
#  define QDT_DEBUGN qDebug
#else
#  define QDT_DEBUG  if (false) qDebug()
#  define QDT_DEBUGN if (false) qDebug
#endif

#ifdef Q_OS_DARWIN
#include <qcore_mac_p.h>
#endif

enum {
   FIRST_YEAR  = -4713,
   FIRST_MONTH = 1,
   FIRST_DAY   = 2,

   SECS_PER_DAY   = 86400,
   MSECS_PER_DAY  = 86400000,
   SECS_PER_HOUR  = 3600,
   MSECS_PER_HOUR = 3600000,
   SECS_PER_MIN   = 60,
   MSECS_PER_MIN  = 60000,
   JULIAN_DAY_FOR_EPOCH = 2440588 // result of julianDayFromGregorianDate(1970, 1, 1)
};

static inline QDate fixedDate(int y, int m, int d)
{
   QDate result(y, m, 1);
   result.setDate(y, m, qMin(d, result.daysInMonth()));
   return result;
}

static inline uint julianDayFromGregorianDate(int year, int month, int day)
{
   // Gregorian calendar starting from October 15, 1582
   // Algorithm from Henry F. Fliegel and Thomas C. Van Flandern
   return (1461 * (year + 4800 + (month - 14) / 12)) / 4
          + (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12
          - (3 * ((year + 4900 + (month - 14) / 12) / 100)) / 4
          + day - 32075;
}

static uint julianDayFromDate(int year, int month, int day)
{
   if (year < 0) {
      ++year;
   }

   if (year > 1582 || (year == 1582 && (month > 10 || (month == 10 && day >= 15)))) {
      return julianDayFromGregorianDate(year, month, day);
   } else if (year < 1582 || (year == 1582 && (month < 10 || (month == 10 && day <= 4)))) {
      // Julian calendar until October 4, 1582
      // Algorithm from Frequently Asked Questions about Calendars by Claus Toendering
      int a = (14 - month) / 12;
      return (153 * (month + (12 * a) - 3) + 2) / 5
             + (1461 * (year + 4800 - a)) / 4
             + day - 32083;
   } else {
      // the day following October 4, 1582 is October 15, 1582
      return 0;
   }
}

static void getDateFromJulianDay(uint julianDay, int *year, int *month, int *day)
{
   int y, m, d;

   if (julianDay >= 2299161) {
      // Gregorian calendar starting from October 15, 1582
      // This algorithm is from Henry F. Fliegel and Thomas C. Van Flandern
      quint64 ell, n, i, j;
      ell = quint64(julianDay) + 68569;
      n = (4 * ell) / 146097;
      ell = ell - (146097 * n + 3) / 4;
      i = (4000 * (ell + 1)) / 1461001;
      ell = ell - (1461 * i) / 4 + 31;
      j = (80 * ell) / 2447;
      d = ell - (2447 * j) / 80;
      ell = j / 11;
      m = j + 2 - (12 * ell);
      y = 100 * (n - 49) + i + ell;
   } else {
      // Julian calendar until October 4, 1582
      // Algorithm from Frequently Asked Questions about Calendars by Claus Toendering
      julianDay += 32082;
      int dd = (4 * julianDay + 3) / 1461;
      int ee = julianDay - (1461 * dd) / 4;
      int mm = ((5 * ee) + 2) / 153;
      d = ee - (153 * mm + 2) / 5 + 1;
      m = mm + 3 - 12 * (mm / 10);
      y = dd - 4800 + (mm / 10);
      if (y <= 0) {
         --y;
      }
   }
   if (year) {
      *year = y;
   }
   if (month) {
      *month = m;
   }
   if (day) {
      *day = d;
   }
}


static const char monthDays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#ifndef QT_NO_TEXTDATE

static const QString qt_shortMonthNames[] = {
   "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

#endif

#ifndef QT_NO_DATESTRING
static QString fmtDateTime(const QString &f, const QTime *dt = 0, const QDate *dd = 0);
#endif

static QDateTimePrivate::Spec utcToLocal(QDate &date, QTime &time);
static void utcToOffset(QDate *date, QTime *time, qint32 offset);

// Return offset in [+-]HH:MM format
// Qt::ISODate puts : between the hours and minutes, but Qt:TextDate does not
static QString toOffsetString(Qt::DateFormat format, int offset)
{
   QString result;
   if (format == Qt::TextDate) {
      result = QString("%1%2%3");
   } else { // Qt::ISODate
      result = QString("%1%2:%3");
   }

   return result.formatArg(offset >= 0 ? QLatin1Char('+') : QLatin1Char('-'))
          .formatArg(qAbs(offset) / SECS_PER_HOUR, 2, 10, QLatin1Char('0'))
          .formatArg((offset / 60) % 60, 2, 10, QLatin1Char('0'));
}

// Parse offset in [+-]HH[:]MM format
static int fromOffsetString(const QString &offsetString, bool *valid)
{
   *valid = false;

   const int size = offsetString.size();
   if (size < 2 || size > 6) {
      return 0;
   }

   // First char must be + or -
   const QChar sign = offsetString.at(0);
   if (sign != QLatin1Char('+') && sign != QLatin1Char('-')) {
      return 0;
   }

   // Split the hour and minute parts
   QStringList parts = offsetString.split(':');

   if (parts.count() == 1) {
      // [+-]HHMM format
      parts.append(parts.at(0).mid(3));
      parts[0] = parts.at(0).left(3);
   }

   bool ok = false;
   const int hour = parts.at(0).toInteger<int>(&ok);
   if (!ok) {
      return 0;
   }

   const int minute = parts.at(1).toInteger<int>(&ok);
   if (!ok || minute < 0 || minute > 59) {
      return 0;
   }

   *valid = true;
   return ((hour * 60) + minute) * 60;
}


QDate::QDate(int y, int m, int d)
{
   setDate(y, m, d);
}

bool QDate::isValid() const
{
   return !isNull();
}

int QDate::year() const
{
   int y;
   getDateFromJulianDay(jd, &y, 0, 0);
   return y;
}

int QDate::month() const
{
   int m;
   getDateFromJulianDay(jd, 0, &m, 0);
   return m;
}

int QDate::day() const
{
   int d;
   getDateFromJulianDay(jd, 0, 0, &d);
   return d;
}

int QDate::dayOfWeek() const
{
   return (jd % 7) + 1;
}


int QDate::dayOfYear() const
{
   return jd - julianDayFromDate(year(), 1, 1) + 1;
}

int QDate::daysInMonth() const
{
   int y, m, d;
   getDateFromJulianDay(jd, &y, &m, &d);
   if (m == 2 && isLeapYear(y)) {
      return 29;
   } else {
      return monthDays[m];
   }
}

int QDate::daysInYear() const
{
   int y, m, d;
   getDateFromJulianDay(jd, &y, &m, &d);
   return isLeapYear(y) ? 366 : 365;
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
      int w = (yday - 365 - (QDate::isLeapYear(year + 1) ? 1 : 0) - wday + 10) / 7;
      if (w > 0) {
         ++year;
         week = w;
      }
      Q_ASSERT(week == 53 || week == 1);
   }

   if (yearNumber != 0) {
      *yearNumber = year;
   }
  return week;
}

#ifndef QT_NO_TEXTDATE

QString QDate::shortMonthName(int month, QDate::MonthNameType type)
{
   if (month < 1 || month > 12) {
      month = 1;
   }
   switch (type) {
      case QDate::DateFormat:
         return QLocale::system().monthName(month, QLocale::ShortFormat);
      case QDate::StandaloneFormat:
         return QLocale::system().standaloneMonthName(month, QLocale::ShortFormat);
      default:
         break;
   }
   return QString();
}

QString QDate::shortMonthName(int month)
{
   return shortMonthName(month, QDate::DateFormat);
}

QString QDate::longMonthName(int month, MonthNameType type)
{
   if (month < 1 || month > 12) {
      month = 1;
   }
   switch (type) {
      case QDate::DateFormat:
         return QLocale::system().monthName(month, QLocale::LongFormat);
      case QDate::StandaloneFormat:
         return QLocale::system().standaloneMonthName(month, QLocale::LongFormat);
      default:
         break;
   }
   return QString();
}

QString QDate::longMonthName(int month)
{
   if (month < 1 || month > 12) {
      month = 1;
   }
   return QLocale::system().monthName(month, QLocale::LongFormat);
}

QString QDate::shortDayName(int weekday, MonthNameType type)
{
   if (weekday < 1 || weekday > 7) {
      weekday = 1;
   }
   switch (type) {
      case QDate::DateFormat:
         return QLocale::system().dayName(weekday, QLocale::ShortFormat);
      case QDate::StandaloneFormat:
         return QLocale::system().standaloneDayName(weekday, QLocale::ShortFormat);
      default:
         break;
   }
   return QString();
}

QString QDate::shortDayName(int weekday)
{
   if (weekday < 1 || weekday > 7) {
      weekday = 1;
   }
   return QLocale::system().dayName(weekday, QLocale::ShortFormat);
}

QString QDate::longDayName(int weekday, MonthNameType type)
{
   if (weekday < 1 || weekday > 7) {
      weekday = 1;
   }
   switch (type) {
      case QDate::DateFormat:
         return QLocale::system().dayName(weekday, QLocale::LongFormat);
      case QDate::StandaloneFormat:
         return QLocale::system().standaloneDayName(weekday, QLocale::LongFormat);
      default:
         break;
   }
   return QLocale::system().dayName(weekday, QLocale::LongFormat);
}

QString QDate::longDayName(int weekday)
{
   if (weekday < 1 || weekday > 7) {
      weekday = 1;
   }
   return QLocale::system().dayName(weekday, QLocale::LongFormat);
}
#endif //QT_NO_TEXTDATE

#ifndef QT_NO_DATESTRING

QString QDate::toString(Qt::DateFormat f) const
{
   if (!isValid()) {
      return QString();
   }
   int y, m, d;
   getDateFromJulianDay(jd, &y, &m, &d);
   switch (f) {
      case Qt::SystemLocaleDate:
      case Qt::SystemLocaleShortDate:
      case Qt::SystemLocaleLongDate:
         return QLocale::system().toString(*this, f == Qt::SystemLocaleLongDate ? QLocale::LongFormat
                                           : QLocale::ShortFormat);
      case Qt::LocaleDate:
      case Qt::DefaultLocaleShortDate:
      case Qt::DefaultLocaleLongDate:
         return QLocale().toString(*this, f == Qt::DefaultLocaleLongDate ? QLocale::LongFormat
                                   : QLocale::ShortFormat);
      default:
#ifndef QT_NO_TEXTDATE
      case Qt::TextDate: {
         return QString::fromLatin1("%0 %1 %2 %3")
                .formatArg(shortDayName(dayOfWeek()))
                .formatArg(shortMonthName(m))
                .formatArg(d)
                .formatArg(y);
      }
#endif
      case Qt::ISODate: {
         if (year() < 0 || year() > 9999) {
            return QString();
         }
         QString month(QString::number(m).rightJustified(2, QLatin1Char('0')));
         QString day(QString::number(d).rightJustified(2, QLatin1Char('0')));
         return QString::number(y) + QLatin1Char('-') + month + QLatin1Char('-') + day;
      }
   }
}

QString QDate::toString(const QString &format) const
{
   if (year() > 9999) {
      return QString();
   }
   return fmtDateTime(format, 0, this);
}
#endif //QT_NO_DATESTRING

bool QDate::setYMD(int y, int m, int d)
{
   if (uint(y) <= 99) {
      y += 1900;
   }
   return setDate(y, m, d);
}

bool QDate::setDate(int year, int month, int day)
{
   if (!isValid(year, month, day)) {
      jd = 0;
   } else {
      jd = julianDayFromDate(year, month, day);
   }
   return jd != 0;
}

void QDate::getDate(int *year, int *month, int *day)
{
   getDateFromJulianDay(jd, year, month, day);
}

QDate QDate::addDays(int ndays) const
{
   QDate d;
   // this is basically "d.jd = jd + ndays" with checks for integer overflow
   if (ndays >= 0) {
      d.jd = (jd + ndays >= jd) ? jd + ndays : 0;
   } else {
      d.jd = (jd + ndays < jd) ? jd + ndays : 0;
   }
   return d;
}


QDate QDate::addMonths(int nmonths) const
{
   if (!isValid()) {
      return QDate();
   }
   if (!nmonths) {
      return *this;
   }

   int old_y, y, m, d;
   getDateFromJulianDay(jd, &y, &m, &d);
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

   // did we end up in the Gregorian/Julian conversion hole?
   if (y == 1582 && m == 10 && d > 4 && d < 15) {
      d = increasing ? 15 : 4;
   }

   return fixedDate(y, m, d);
}

QDate QDate::addYears(int nyears) const
{
   if (!isValid()) {
      return QDate();
   }

   int y, m, d;
   getDateFromJulianDay(jd, &y, &m, &d);

   int old_y = y;
   y += nyears;

   // was there a sign change?
   if ((old_y > 0 && y <= 0) ||
         (old_y < 0 && y >= 0))
      // yes, adjust the date by +1 or -1 years
   {
      y += nyears > 0 ? +1 : -1;
   }

   return fixedDate(y, m, d);
}

int QDate::daysTo(const QDate &d) const
{
   return d.jd - jd;
}

#ifndef QT_NO_DATESTRING

QDate QDate::fromString(const QString &s, Qt::DateFormat f)
{
   if (s.isEmpty()) {
      return QDate();
   }

   switch (f) {
      case Qt::ISODate: {
         int year(s.mid(0, 4).toInteger<int>());
         int month(s.mid(5, 2).toInteger<int>());
         int day(s.mid(8, 2).toInteger<int>());

         if (year && month && day) {
            return QDate(year, month, day);
         }
      }

      break;

      case Qt::SystemLocaleDate:
      case Qt::SystemLocaleShortDate:
      case Qt::SystemLocaleLongDate:
         return fromString(s, QLocale::system().dateFormat(f == Qt::SystemLocaleLongDate ? QLocale::LongFormat : QLocale::ShortFormat));

      case Qt::LocaleDate:
      case Qt::DefaultLocaleShortDate:
      case Qt::DefaultLocaleLongDate:
         return fromString(s, QLocale().dateFormat(f == Qt::DefaultLocaleLongDate ? QLocale::LongFormat : QLocale::ShortFormat));

      default:

#ifndef QT_NO_TEXTDATE
      case Qt::TextDate: {
         QStringList parts = s.split(' ', QStringParser::SkipEmptyParts);

         if (parts.count() != 4) {
            return QDate();
         }

         QString monthName = parts.at(1);
         int month = -1;
         // Assume that English monthnames are the default
         for (int i = 0; i < 12; ++i) {
            if (monthName == QString(qt_shortMonthNames[i])) {
               month = i + 1;
               break;
            }
         }
         // If English names can't be found, search the localized ones
         if (month == -1) {
            for (int i = 1; i <= 12; ++i) {
               if (monthName == QDate::shortMonthName(i)) {
                  month = i;
                  break;
               }
            }
         }
         if (month < 1 || month > 12) {
            return QDate();
         }

         bool ok;
         int day = parts.at(2).toInteger<int>(&ok);
         if (!ok) {
            return QDate();
         }

         int year = parts.at(3).toInteger<int>(&ok);
         if (!ok) {
            return QDate();
         }

         return QDate(year, month, day);
      }
#else
      break;
#endif
   }
   return QDate();
}

/*!
    \fn QDate::fromString(const QString &string, const QString &format)

    Returns the QDate represented by the \a string, using the \a
    format given, or an invalid date if the string cannot be parsed.

    These expressions may be used for the format:

    \table
    \header \i Expression \i Output
    \row \i d \i The day as a number without a leading zero (1 to 31)
    \row \i dd \i The day as a number with a leading zero (01 to 31)
    \row \i ddd
         \i The abbreviated localized day name (e.g. 'Mon' to 'Sun').
            Uses QDate::shortDayName().
    \row \i dddd
         \i The long localized day name (e.g. 'Monday' to 'Sunday').
            Uses QDate::longDayName().
    \row \i M \i The month as a number without a leading zero (1 to 12)
    \row \i MM \i The month as a number with a leading zero (01 to 12)
    \row \i MMM
         \i The abbreviated localized month name (e.g. 'Jan' to 'Dec').
            Uses QDate::shortMonthName().
    \row \i MMMM
         \i The long localized month name (e.g. 'January' to 'December').
            Uses QDate::longMonthName().
    \row \i yy \i The year as two digit number (00 to 99)
    \row \i yyyy \i The year as four digit number. If the year is negative,
            a minus sign is prepended in addition.
    \endtable

    All other input characters will be treated as text. Any sequence
    of characters that are enclosed in single quotes will also be
    treated as text and will not be used as an expression. For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qdatetime.cpp 1

    If the format is not satisfied, an invalid QDate is returned. The
    expressions that don't expect leading zeroes (d, M) will be
    greedy. This means that they will use two digits even if this
    will put them outside the accepted range of values and leaves too
    few digits for other sections. For example, the following format
    string could have meant January 30 but the M will grab two
    digits, resulting in an invalid date:

    \snippet doc/src/snippets/code/src_corelib_tools_qdatetime.cpp 2

    For any field that is not represented in the format the following
    defaults are used:

    \table
    \header \i Field  \i Default value
    \row    \i Year   \i 1900
    \row    \i Month  \i 1
    \row    \i Day    \i 1
    \endtable

    The following examples demonstrate the default values:

    \snippet doc/src/snippets/code/src_corelib_tools_qdatetime.cpp 3

    \sa QDateTime::fromString(), QTime::fromString(), QDate::toString(),
        QDateTime::toString(), QTime::toString()
*/

QDate QDate::fromString(const QString &string, const QString &format)
{
   QDate date;

   QDateTimeParser dt(QVariant::Date, QDateTimeParser::FromString);
   if (dt.parseFormat(format)) {
      dt.fromString(string, &date, 0);
   }

   return date;
}
#endif // QT_NO_DATESTRING

/*!
    \overload

    Returns true if the specified date (\a year, \a month, and \a
    day) is valid; otherwise returns false.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qdatetime.cpp 4

    \sa isNull(), setDate()
*/

bool QDate::isValid(int year, int month, int day)
{
   if (year < FIRST_YEAR
         || (year == FIRST_YEAR &&
             (month < FIRST_MONTH
              || (month == FIRST_MONTH && day < FIRST_DAY)))
         || year == 0) { // there is no year 0 in the Julian calendar
      return false;
   }

   // passage from Julian to Gregorian calendar
   if (year == 1582 && month == 10 && day > 4 && day < 15) {
      return 0;
   }

   return (day > 0 && month > 0 && month <= 12) &&
          (day <= monthDays[month] || (day == 29 && month == 2 && isLeapYear(year)));
}

/*!
    \fn bool QDate::isLeapYear(int year)

    Returns true if the specified \a year is a leap year; otherwise
    returns false.
*/

bool QDate::isLeapYear(int y)
{
   if (y < 1582) {
      if ( y < 1) {  // No year 0 in Julian calendar, so -1, -5, -9 etc are leap years
         ++y;
      }
      return y % 4 == 0;
   } else {
      return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
   }
}

/*!
    \internal

    This function has a confusing name and should not be part of the API
    we have toJulian() and fromJulian(). ### Qt5: remove it
*/
uint QDate::gregorianToJulian(int y, int m, int d)
{
   return julianDayFromDate(y, m, d);
}

/*!
    \internal

    This function has a confusing name and should not be part of the API
    we have toJulian() and fromJulian(). ### Qt5: remove it
*/
void QDate::julianToGregorian(uint jd, int &y, int &m, int &d)
{
   getDateFromJulianDay(jd, &y, &m, &d);
}

QTime::QTime(int h, int m, int s, int ms)
{
   setHMS(h, m, s, ms);
}

bool QTime::isValid() const
{
   return mds > NullTime && mds < MSECS_PER_DAY;
}


/*!
    Returns the hour part (0 to 23) of the time.

    \sa minute(), second(), msec()
*/

int QTime::hour() const
{
   return ds() / MSECS_PER_HOUR;
}

/*!
    Returns the minute part (0 to 59) of the time.

    \sa hour(), second(), msec()
*/

int QTime::minute() const
{
   return (ds() % MSECS_PER_HOUR) / MSECS_PER_MIN;
}

/*!
    Returns the second part (0 to 59) of the time.

    \sa hour(), minute(), msec()
*/

int QTime::second() const
{
   return (ds() / 1000) % SECS_PER_MIN;
}

/*!
    Returns the millisecond part (0 to 999) of the time.

    \sa hour(), minute(), second()
*/

int QTime::msec() const
{
   return ds() % 1000;
}

#ifndef QT_NO_DATESTRING

QString QTime::toString(Qt::DateFormat format) const
{
   if (!isValid()) {
      return QString();
   }

   switch (format) {
      case Qt::SystemLocaleDate:
      case Qt::SystemLocaleShortDate:
      case Qt::SystemLocaleLongDate:
         return QLocale::system().toString(*this, format == Qt::SystemLocaleLongDate ? QLocale::LongFormat
                                           : QLocale::ShortFormat);
      case Qt::LocaleDate:
      case Qt::DefaultLocaleShortDate:
      case Qt::DefaultLocaleLongDate:
         return QLocale().toString(*this, format == Qt::DefaultLocaleLongDate ? QLocale::LongFormat
                                   : QLocale::ShortFormat);

      default:
      case Qt::ISODate:
      case Qt::TextDate:
         return QString::fromLatin1("%1:%2:%3")
                .formatArg(hour(), 2, 10, QLatin1Char('0'))
                .formatArg(minute(), 2, 10, QLatin1Char('0'))
                .formatArg(second(), 2, 10, QLatin1Char('0'));
   }
}

/*!
    Returns the time as a string. The \a format parameter determines
    the format of the result string.

    These expressions may be used:

    \table
    \header \i Expression \i Output
    \row \i h
         \i the hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)
    \row \i hh
         \i the hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)
    \row \i H
         \i the hour without a leading zero (0 to 23, even with AM/PM display)
    \row \i HH
         \i the hour with a leading zero (00 to 23, even with AM/PM display)
    \row \i m \i the minute without a leading zero (0 to 59)
    \row \i mm \i the minute with a leading zero (00 to 59)
    \row \i s \i the second without a leading zero (0 to 59)
    \row \i ss \i the second with a leading zero (00 to 59)
    \row \i z \i the milliseconds without leading zeroes (0 to 999)
    \row \i zzz \i the milliseconds with leading zeroes (000 to 999)
    \row \i AP or A
         \i use AM/PM display. \e AP will be replaced by either "AM" or "PM".
    \row \i ap or a
         \i use am/pm display. \e ap will be replaced by either "am" or "pm".
    \row \i t \i the timezone (for example "CEST")
    \endtable

    All other input characters will be ignored. Any sequence of characters that
    are enclosed in singlequotes will be treated as text and not be used as an
    expression. Two consecutive singlequotes ("''") are replaced by a singlequote
    in the output.

    Example format strings (assuming that the QTime is 14:13:09.042)

    \table
    \header \i Format \i Result
    \row \i hh:mm:ss.zzz \i 14:13:09.042
    \row \i h:m:s ap     \i 2:13:9 pm
    \row \i H:m:s a      \i 14:13:9 pm
    \endtable

    If the datetime is invalid, an empty string will be returned.
    If \a format is empty, the default format "hh:mm:ss" is used.

    \sa QDate::toString() QDateTime::toString()
*/
QString QTime::toString(const QString &format) const
{
   return fmtDateTime(format, this, 0);
}
#endif //QT_NO_DATESTRING
/*!
    Sets the time to hour \a h, minute \a m, seconds \a s and
    milliseconds \a ms.

    \a h must be in the range 0 to 23, \a m and \a s must be in the
    range 0 to 59, and \a ms must be in the range 0 to 999.
    Returns true if the set time is valid; otherwise returns false.

    \sa isValid()
*/

bool QTime::setHMS(int h, int m, int s, int ms)
{
   if (!isValid(h, m, s, ms)) {
      mds = NullTime;                // make this invalid
      return false;
   }
   mds = (h * SECS_PER_HOUR + m * SECS_PER_MIN + s) * 1000 + ms;
   return true;
}

/*!
    Returns a QTime object containing a time \a s seconds later
    than the time of this object (or earlier if \a s is negative).

    Note that the time will wrap if it passes midnight.

    Example:

    \snippet doc/src/snippets/code/src_corelib_tools_qdatetime.cpp 5

    \sa addMSecs(), secsTo(), QDateTime::addSecs()
*/

QTime QTime::addSecs(int s) const
{
   return addMSecs(s * 1000);
}

/*!
    Returns the number of seconds from this time to \a t.
    If \a t is earlier than this time, the number of seconds returned
    is negative.

    Because QTime measures time within a day and there are 86400
    seconds in a day, the result is always between -86400 and 86400.

    secsTo() does not take into account any milliseconds.

    \sa addSecs(), QDateTime::secsTo()
*/

int QTime::secsTo(const QTime &t) const
{
   return (t.ds() - ds()) / 1000;
}

/*!
    Returns a QTime object containing a time \a ms milliseconds later
    than the time of this object (or earlier if \a ms is negative).

    Note that the time will wrap if it passes midnight. See addSecs()
    for an example.

    \sa addSecs(), msecsTo(), QDateTime::addMSecs()
*/

QTime QTime::addMSecs(int ms) const
{
   QTime t;
   if (ms < 0) {
      // % not well-defined for -ve, but / is.
      int negdays = (MSECS_PER_DAY - ms) / MSECS_PER_DAY;
      t.mds = (ds() + ms + negdays * MSECS_PER_DAY) % MSECS_PER_DAY;
   } else {
      t.mds = (ds() + ms) % MSECS_PER_DAY;
   }

   return t;
}

/*!
    Returns the number of milliseconds from this time to \a t.
    If \a t is earlier than this time, the number of milliseconds returned
    is negative.

    Because QTime measures time within a day and there are 86400
    seconds in a day, the result is always between -86400000 and
    86400000 ms.

    \sa secsTo(), addMSecs(), QDateTime::msecsTo()
*/

int QTime::msecsTo(const QTime &t) const
{
   return t.ds() - ds();
}


/*!
    \fn bool QTime::operator==(const QTime &t) const

    Returns true if this time is equal to \a t; otherwise returns false.
*/

/*!
    \fn bool QTime::operator!=(const QTime &t) const

    Returns true if this time is different from \a t; otherwise returns false.
*/

/*!
    \fn bool QTime::operator<(const QTime &t) const

    Returns true if this time is earlier than \a t; otherwise returns false.
*/

/*!
    \fn bool QTime::operator<=(const QTime &t) const

    Returns true if this time is earlier than or equal to \a t;
    otherwise returns false.
*/

/*!
    \fn bool QTime::operator>(const QTime &t) const

    Returns true if this time is later than \a t; otherwise returns false.
*/

/*!
    \fn bool QTime::operator>=(const QTime &t) const

    Returns true if this time is later than or equal to \a t;
    otherwise returns false.
*/

/*!
    \fn QTime::currentTime()

    Returns the current time as reported by the system clock.

    Note that the accuracy depends on the accuracy of the underlying
    operating system; not all systems provide 1-millisecond accuracy.
*/

#ifndef QT_NO_DATESTRING
/*!
    \fn QTime QTime::fromString(const QString &string, Qt::DateFormat format)

    Returns the time represented in the \a string as a QTime using the
    \a format given, or an invalid time if this is not possible.

    Note that fromString() uses a "C" locale encoded string to convert
    milliseconds to a float value. If the default locale is not "C",
    this may result in two conversion attempts (if the conversion
    fails for the default locale). This should be considered an
    implementation detail.
*/
QTime QTime::fromString(const QString &s, Qt::DateFormat f)
{
   if (s.isEmpty()) {
      QTime t;
      t.mds = NullTime;
      return t;
   }

   switch (f) {
      case Qt::SystemLocaleDate:
      case Qt::SystemLocaleShortDate:
      case Qt::SystemLocaleLongDate:
         return fromString(s, QLocale::system().timeFormat(f == Qt::SystemLocaleLongDate ? QLocale::LongFormat
                           : QLocale::ShortFormat));
      case Qt::LocaleDate:
      case Qt::DefaultLocaleShortDate:
      case Qt::DefaultLocaleLongDate:
         return fromString(s, QLocale().timeFormat(f == Qt::DefaultLocaleLongDate ? QLocale::LongFormat
                           : QLocale::ShortFormat));
      default: {
         bool ok = true;
         const int hour(s.mid(0, 2).toInteger<int>(&ok));
         if (!ok) {
            return QTime();
         }
         const int minute(s.mid(3, 2).toInteger<int>(&ok));
         if (!ok) {
            return QTime();
         }
         const int second(s.mid(6, 2).toInteger<int>(&ok));
         if (!ok) {
            return QTime();
         }
         const QString msec_s(QLatin1String("0.") + s.mid(9, 4));
         const float msec(msec_s.toFloat(&ok));
         if (!ok) {
            return QTime(hour, minute, second, 0);
         }
         return QTime(hour, minute, second, qMin(qRound(msec * 1000.0), 999));
      }
   }
}

/*!
    \fn QTime::fromString(const QString &string, const QString &format)

    Returns the QTime represented by the \a string, using the \a
    format given, or an invalid time if the string cannot be parsed.

    These expressions may be used for the format:

    \table
    \header \i Expression \i Output
    \row \i h
         \i the hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)
    \row \i hh
         \i the hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)
    \row \i m \i the minute without a leading zero (0 to 59)
    \row \i mm \i the minute with a leading zero (00 to 59)
    \row \i s \i the second without a leading zero (0 to 59)
    \row \i ss \i the second with a leading zero (00 to 59)
    \row \i z \i the milliseconds without leading zeroes (0 to 999)
    \row \i zzz \i the milliseconds with leading zeroes (000 to 999)
    \row \i AP
         \i interpret as an AM/PM time. \e AP must be either "AM" or "PM".
    \row \i ap
         \i Interpret as an AM/PM time. \e ap must be either "am" or "pm".
    \endtable

    All other input characters will be treated as text. Any sequence
    of characters that are enclosed in single quotes will also be
    treated as text and not be used as an expression.

    \snippet doc/src/snippets/code/src_corelib_tools_qdatetime.cpp 6

    If the format is not satisfied an invalid QTime is returned.
    Expressions that do not expect leading zeroes to be given (h, m, s
    and z) are greedy. This means that they will use two digits even if
    this puts them outside the range of accepted values and leaves too
    few digits for other sections. For example, the following string
    could have meant 00:07:10, but the m will grab two digits, resulting
    in an invalid time:

    \snippet doc/src/snippets/code/src_corelib_tools_qdatetime.cpp 7

    Any field that is not represented in the format will be set to zero.
    For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qdatetime.cpp 8

    \sa QDateTime::fromString() QDate::fromString() QDate::toString()
    QDateTime::toString() QTime::toString()
*/

QTime QTime::fromString(const QString &string, const QString &format)
{
   QTime time;

   QDateTimeParser dt(QVariant::Time, QDateTimeParser::FromString);
   if (dt.parseFormat(format)) {
      dt.fromString(string, 0, &time);
   }

   return time;
}

#endif // QT_NO_DATESTRING


/*!
    \overload

    Returns true if the specified time is valid; otherwise returns
    false.

    The time is valid if \a h is in the range 0 to 23, \a m and
    \a s are in the range 0 to 59, and \a ms is in the range 0 to 999.

    Example:

    \snippet doc/src/snippets/code/src_corelib_tools_qdatetime.cpp 9
*/

bool QTime::isValid(int h, int m, int s, int ms)
{
   return (uint)h < 24 && (uint)m < 60 && (uint)s < 60 && (uint)ms < 1000;
}


/*!
    Sets this time to the current time. This is practical for timing:

    \snippet doc/src/snippets/code/src_corelib_tools_qdatetime.cpp 10

    \sa restart(), elapsed(), currentTime()
*/

void QTime::start()
{
   *this = currentTime();
}

/*!
    Sets this time to the current time and returns the number of
    milliseconds that have elapsed since the last time start() or
    restart() was called.

    This function is guaranteed to be atomic and is thus very handy
    for repeated measurements. Call start() to start the first
    measurement, and restart() for each later measurement.

    Note that the counter wraps to zero 24 hours after the last call
    to start() or restart().

    \warning If the system's clock setting has been changed since the
    last time start() or restart() was called, the result is
    undefined. This can happen when daylight savings time is turned on
    or off.

    \sa start(), elapsed(), currentTime()
*/

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

/*!
    Returns the number of milliseconds that have elapsed since the
    last time start() or restart() was called.

    Note that the counter wraps to zero 24 hours after the last call
    to start() or restart.

    Note that the accuracy depends on the accuracy of the underlying
    operating system; not all systems provide 1-millisecond accuracy.

    \warning If the system's clock setting has been changed since the
    last time start() or restart() was called, the result is
    undefined. This can happen when daylight savings time is turned on
    or off.

    \sa start(), restart()
*/

int QTime::elapsed() const
{
   int n = msecsTo(currentTime());
   if (n < 0) {                              // passed midnight
      n += 86400 * 1000;
   }
   return n;
}
QDateTime::QDateTime()
   : d(new QDateTimePrivate)
{
}

QDateTime::QDateTime(const QDate &date)
   : d(new QDateTimePrivate(date, QTime(0, 0, 0), Qt::LocalTime, 0))
{
}

QDateTime::QDateTime(const QDate &date, const QTime &time, Qt::TimeSpec spec)
   : d(new QDateTimePrivate(date, time, spec, 0))
{
}

QDateTime::QDateTime(const QDate &date, const QTime &time, Qt::TimeSpec spec, int offsetSeconds)
   : d(new QDateTimePrivate(date, time, spec, offsetSeconds))
{
}

QDateTimePrivate::QDateTimePrivate(const QDate &toDate, const QTime &toTime, Qt::TimeSpec toSpec, int offsetSeconds)
{
   date = toDate;

   if (!toTime.isValid() && toDate.isValid()) {
      time = QTime(0, 0, 0);
   } else {
      time = toTime;
   }

   m_offsetFromUtc = 0;

   switch (toSpec) {
      case Qt::UTC :
         spec = QDateTimePrivate::UTC;
         break;
      case Qt::OffsetFromUTC :
         if (offsetSeconds == 0) {
            spec = QDateTimePrivate::UTC;
         } else {
            spec = QDateTimePrivate::OffsetFromUTC;
            m_offsetFromUtc = offsetSeconds;
         }
         break;
      case Qt::LocalTime :
         spec = QDateTimePrivate::LocalUnknown;
   }
}

QDateTime::QDateTime(const QDateTime &other)
   : d(other.d)
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
   return d->date.isNull() && d->time.isNull();
}

bool QDateTime::isValid() const
{
   return d->date.isValid() && d->time.isValid();
}

QDate QDateTime::date() const
{
   return d->date;
}

QTime QDateTime::time() const
{
   return d->time;
}

Qt::TimeSpec QDateTime::timeSpec() const
{
   switch (d->spec) {
      case QDateTimePrivate::UTC:
         return Qt::UTC;
      case QDateTimePrivate::OffsetFromUTC:
         return Qt::OffsetFromUTC;
      default:
         return Qt::LocalTime;
   }
}

int QDateTime::offsetFromUtc() const
{
   switch (d->spec) {
      case QDateTimePrivate::OffsetFromUTC:
         return d->m_offsetFromUtc;
      case QDateTimePrivate::UTC:
         return 0;
      default:  // Any Qt::LocalTime
         const QDateTime fakeDate(d->date, d->time, Qt::UTC);
         return (fakeDate.toMSecsSinceEpoch() - toMSecsSinceEpoch()) / 1000;
   }
}

void QDateTime::setDate(const QDate &date)
{
   detach();
   d->date = date;
   if (d->spec == QDateTimePrivate::LocalStandard
         || d->spec == QDateTimePrivate::LocalDST) {
      d->spec = QDateTimePrivate::LocalUnknown;
   }
   if (date.isValid() && !d->time.isValid()) {
      d->time = QTime(0, 0, 0);
   }
}

/*!
    Sets the time part of this datetime to \a time.

    \sa time(), setDate(), setTimeSpec()
*/

void QDateTime::setTime(const QTime &time)
{
   detach();
   if (d->spec == QDateTimePrivate::LocalStandard
         || d->spec == QDateTimePrivate::LocalDST) {
      d->spec = QDateTimePrivate::LocalUnknown;
   }
   d->time = time;
}

/*!
    Sets the time specification used in this datetime to \a spec.
    The datetime will refer to a different point in time.

    If \a spec is Qt::OffsetFromUTC then the timeSpec() will be set
    to Qt::UTC, i.e. an effective offset of 0.

    \sa timeSpec(), setDate(), setTime(), Qt::TimeSpec
*/

void QDateTime::setTimeSpec(Qt::TimeSpec spec)
{
   detach();

   d->m_offsetFromUtc = 0;
   switch (spec) {
      case Qt::UTC:
      case Qt::OffsetFromUTC:
         d->spec = QDateTimePrivate::UTC;
         break;
      default:
         d->spec = QDateTimePrivate::LocalUnknown;
         break;
   }
}

void QDateTime::setOffsetFromUtc(int offsetSeconds)
{
   detach();

   if (offsetSeconds == 0) {
      d->spec = QDateTimePrivate::UTC;
      d->m_offsetFromUtc = 0;
   } else {
      d->spec = QDateTimePrivate::OffsetFromUTC;
      d->m_offsetFromUtc = offsetSeconds;
   }
}

qint64 toMSecsSinceEpoch_helper(qint64 jd, int msecs)
{
   qint64 days = jd - JULIAN_DAY_FOR_EPOCH;
   qint64 retval = (days * MSECS_PER_DAY) + msecs;
   return retval;
}

qint64 QDateTime::toMSecsSinceEpoch() const
{
   QDate utcDate;
   QTime utcTime;
   d->getUTC(utcDate, utcTime);

   return toMSecsSinceEpoch_helper(utcDate.jd, utcTime.ds());
}

uint QDateTime::toTime_t() const
{
   qint64 retval = toMSecsSinceEpoch() / 1000;
   if (quint64(retval) >= Q_UINT64_C(0xFFFFFFFF)) {
      return uint(-1);
   }
   return uint(retval);
}

void QDateTime::setMSecsSinceEpoch(qint64 msecs)
{
   detach();

   qint64 ddays = msecs / MSECS_PER_DAY;
   msecs %= MSECS_PER_DAY;
   if (msecs < 0) {
      // negative
      --ddays;
      msecs += MSECS_PER_DAY;
   }

   d->date = QDate(1970, 1, 1).addDays(ddays);
   d->time = QTime(0, 0, 0).addMSecs(msecs);

   if (d->spec == QDateTimePrivate::OffsetFromUTC) {
      utcToOffset(&d->date, &d->time, d->m_offsetFromUtc);
   } else if (d->spec != QDateTimePrivate::UTC) {
      utcToLocal(d->date, d->time);
   }
}

void QDateTime::setTime_t(uint secsSince1Jan1970UTC)
{
   detach();

   d->date = QDate(1970, 1, 1).addDays(secsSince1Jan1970UTC / SECS_PER_DAY);
   d->time = QTime(0, 0, 0).addSecs(secsSince1Jan1970UTC % SECS_PER_DAY);

   if (d->spec == QDateTimePrivate::OffsetFromUTC) {
      utcToOffset(&d->date, &d->time, d->m_offsetFromUtc);
   } else if (d->spec != QDateTimePrivate::UTC) {
      utcToLocal(d->date, d->time);
   }
}

#ifndef QT_NO_DATESTRING

QString QDateTime::toString(Qt::DateFormat f) const
{
   QString buf;
   if (! isValid()) {
      return buf;
   }

   if (f == Qt::ISODate) {
      buf = d->date.toString(Qt::ISODate);
      if (buf.isEmpty()) {
         return QString();   // failed to convert
      }
      buf += QLatin1Char('T');
      buf += d->time.toString(Qt::ISODate);

      switch (d->spec) {
         case QDateTimePrivate::UTC:
            buf += QLatin1Char('Z');
            break;

         case QDateTimePrivate::OffsetFromUTC: {
            buf += toOffsetString(Qt::ISODate, d->m_offsetFromUtc);
            break;
         }

         default:
            break;
      }
   }

#ifndef QT_NO_TEXTDATE
   else if (f == Qt::TextDate) {

#ifndef Q_OS_WIN
      buf = d->date.shortDayName(d->date.dayOfWeek());
      buf += ' ';
      buf += d->date.shortMonthName(d->date.month());
      buf += ' ';
      buf += QString::number(d->date.day());

#else
      std::wstring out(255, L'\0');
      GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILDATE, &out[0], 255);
      QString winstr = QString::fromStdWString(out);

      switch (winstr.toInteger<int>()) {

         case 1:
            buf = d->date.shortDayName(d->date.dayOfWeek());
            buf += ' ';
            buf += QString::number(d->date.day());
            buf += ". ";
            buf += d->date.shortMonthName(d->date.month());
            break;

         default:
            buf = d->date.shortDayName(d->date.dayOfWeek());
            buf += ' ';
            buf += d->date.shortMonthName(d->date.month());
            buf += ' ';
            buf += QString::number(d->date.day());
      }

#endif
      buf += ' ';
      buf += d->time.toString();
      buf += ' ';
      buf += QString::number(d->date.year());
   }
#endif

   else {
      buf = d->date.toString(f);
      if (buf.isEmpty()) {
         return QString();   // failed to convert
      }

      buf += QLatin1Char(' ');
      buf += d->time.toString(f);
   }

   return buf;
}

QString QDateTime::toString(const QString &format) const
{
   return fmtDateTime(format, &d->time, &d->date);
}
#endif //QT_NO_DATESTRING

QDateTime QDateTime::addDays(int ndays) const
{
   QDateTime dt(*this);
   dt.detach();
   dt.d->date = d->date.addDays(ndays);
   return dt;
}

QDateTime QDateTime::addMonths(int nmonths) const
{
   QDateTime dt(*this);
   dt.detach();
   dt.d->date = d->date.addMonths(nmonths);
   return dt;
}

QDateTime QDateTime::addYears(int nyears) const
{
   QDateTime dt(*this);
   dt.detach();
   dt.d->date = d->date.addYears(nyears);
   return dt;
}

QDateTime QDateTimePrivate::addMSecs(const QDateTime &dt, qint64 msecs)
{
   QDate utcDate;
   QTime utcTime;
   dt.d->getUTC(utcDate, utcTime);
   addMSecs(utcDate, utcTime, msecs);
   QDateTime utc(utcDate, utcTime, Qt::UTC);

   if (dt.timeSpec() == Qt::OffsetFromUTC) {
      return utc.toOffsetFromUtc(dt.d->m_offsetFromUtc);
   } else {
      return utc.toTimeSpec(dt.timeSpec());
   }
}

void QDateTimePrivate::addMSecs(QDate &utcDate, QTime &utcTime, qint64 msecs)
{
   uint dd = utcDate.jd;
   int tt = utcTime.ds();
   int sign = 1;
   if (msecs < 0) {
      msecs = -msecs;
      sign = -1;
   }
   if (msecs >= int(MSECS_PER_DAY)) {
      dd += sign * (msecs / MSECS_PER_DAY);
      msecs %= MSECS_PER_DAY;
   }

   tt += sign * msecs;
   if (tt < 0) {
      tt = MSECS_PER_DAY - tt - 1;
      dd -= tt / MSECS_PER_DAY;
      tt = tt % MSECS_PER_DAY;
      tt = MSECS_PER_DAY - tt - 1;
   } else if (tt >= int(MSECS_PER_DAY)) {
      dd += tt / MSECS_PER_DAY;
      tt = tt % MSECS_PER_DAY;
   }

   utcDate.jd = dd;
   utcTime.mds = tt;
}

QDateTime QDateTime::addSecs(int s) const
{
   return d->addMSecs(*this, qint64(s) * 1000);
}

QDateTime QDateTime::addMSecs(qint64 msecs) const
{
   return d->addMSecs(*this, msecs);
}

qint64 QDateTime::daysTo(const QDateTime &other) const
{
   return d->date.daysTo(other.d->date);
}

qint64 QDateTime::secsTo(const QDateTime &other) const
{
   QDate date1, date2;
   QTime time1, time2;

   d->getUTC(date1, time1);
   other.d->getUTC(date2, time2);

   return (date1.daysTo(date2) * SECS_PER_DAY) + time1.secsTo(time2);
}

qint64 QDateTime::msecsTo(const QDateTime &other) const
{
   QDate selfDate;
   QDate otherDate;
   QTime selfTime;
   QTime otherTime;

   d->getUTC(selfDate, selfTime);
   other.d->getUTC(otherDate, otherTime);

   return (static_cast<qint64>(selfDate.daysTo(otherDate)) * static_cast<qint64>(MSECS_PER_DAY))
          + static_cast<qint64>(selfTime.msecsTo(otherTime));
}
QDateTime QDateTime::toTimeSpec(Qt::TimeSpec spec) const
{
   if (spec == Qt::UTC || spec == Qt::OffsetFromUTC) {
      QDate date;
      QTime time;
      d->getUTC(date, time);
      return QDateTime(date, time, Qt::UTC, 0);
   }

   QDateTime ret;
   ret.d->spec = d->getLocal(ret.d->date, ret.d->time);
   return ret;
}

QDateTime QDateTime::toOffsetFromUtc(int offsetSeconds) const
{
   QDate date;
   QTime time;
   d->getUTC(date, time);
   d->addMSecs(date, time, offsetSeconds * 1000);
   return QDateTime(date, time, Qt::OffsetFromUTC, offsetSeconds);
}

bool QDateTime::operator==(const QDateTime &other) const
{
   if (d->spec == other.d->spec && d->m_offsetFromUtc == other.d->m_offsetFromUtc) {
      return d->time == other.d->time && d->date == other.d->date;
   } else {
      QDate date1, date2;
      QTime time1, time2;

      d->getUTC(date1, time1);
      other.d->getUTC(date2, time2);
      return time1 == time2 && date1 == date2;
   }
}

bool QDateTime::operator<(const QDateTime &other) const
{
   if (d->spec == other.d->spec && d->spec != QDateTimePrivate::OffsetFromUTC) {
      if (d->date != other.d->date) {
         return d->date < other.d->date;
      }
      return d->time < other.d->time;
   } else {
      QDate date1, date2;
      QTime time1, time2;
      d->getUTC(date1, time1);
      other.d->getUTC(date2, time2);
      if (date1 != date2) {
         return date1 < date2;
      }
      return time1 < time2;
   }
}

static inline uint msecsFromDecomposed(int hour, int minute, int sec, int msec = 0)
{
   return MSECS_PER_HOUR * hour + MSECS_PER_MIN * minute + 1000 * sec + msec;
}

static void internal_tzset()
{
   static std::atomic<bool> setTimeZone(false);

   if (! setTimeZone)  {
      static std::mutex timeZoneMutex;
      std::lock_guard<std::mutex> lock(timeZoneMutex);

      if (! setTimeZone) {
         tzset();
         setTimeZone = true;
      }
   }
}

#if defined(Q_OS_WIN)
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
   ct.mds = msecsFromDecomposed(st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

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
   QDate d;
   QTime t;
   SYSTEMTIME st;
   memset(&st, 0, sizeof(SYSTEMTIME));
   GetSystemTime(&st);

   return msecsFromDecomposed(st.wHour, st.wMinute, st.wSecond, st.wMilliseconds) +
          qint64(julianDayFromGregorianDate(st.wYear, st.wMonth, st.wDay)
                 - julianDayFromGregorianDate(1970, 1, 1)) * Q_INT64_C(86400000);
}

#elif defined(Q_OS_UNIX)

QDate QDate::currentDate()
{
   QDate d;
   // posix compliant system
   time_t ltime;
   time(&ltime);
   struct tm *t = 0;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
   // use the reentrant version of localtime() where available
   internal_tzset();

   struct tm res;
   t = localtime_r(&ltime, &res);
#else
   t = localtime(&ltime);
#endif // POSIX_THREAD_SAFE_FUNCTIONS

   d.jd = julianDayFromDate(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
   return d;
}

QTime QTime::currentTime()
{
   QTime ct;
   // posix compliant system
   struct timeval tv;
   gettimeofday(&tv, 0);
   time_t ltime = tv.tv_sec;
   struct tm *t = 0;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
   // use the reentrant version of localtime() where available
   internal_tzset();
   struct tm res;
   t = localtime_r(&ltime, &res);
#else
   t = localtime(&ltime);
#endif
   Q_CHECK_PTR(t);

   ct.mds = msecsFromDecomposed(t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec / 1000);
   return ct;
}

QDateTime QDateTime::currentDateTime()
{
   // posix compliant system
   // we have milliseconds
   struct timeval tv;
   gettimeofday(&tv, 0);
   time_t ltime = tv.tv_sec;
   struct tm *t = 0;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
   // use the reentrant version of localtime() where available
   internal_tzset();
   struct tm res;
   t = localtime_r(&ltime, &res);
#else
   t = localtime(&ltime);
#endif

   QDateTime dt;
   dt.d->time.mds = msecsFromDecomposed(t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec / 1000);

   dt.d->date.jd = julianDayFromDate(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
   dt.d->spec = t->tm_isdst > 0  ? QDateTimePrivate::LocalDST :
                t->tm_isdst == 0 ? QDateTimePrivate::LocalStandard :
                QDateTimePrivate::LocalUnknown;
   return dt;
}

QDateTime QDateTime::currentDateTimeUtc()
{
   // posix compliant system
   // we have milliseconds
   struct timeval tv;
   gettimeofday(&tv, 0);
   time_t ltime = tv.tv_sec;
   struct tm *t = 0;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
   // use the reentrant version of localtime() where available
   struct tm res;
   t = gmtime_r(&ltime, &res);
#else
   t = gmtime(&ltime);
#endif

   QDateTime dt;
   dt.d->time.mds = msecsFromDecomposed(t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec / 1000);

   dt.d->date.jd = julianDayFromDate(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
   dt.d->spec = QDateTimePrivate::UTC;
   return dt;
}

qint64 QDateTime::currentMSecsSinceEpoch()
{
   // posix compliant system
   // we have milliseconds
   struct timeval tv;
   gettimeofday(&tv, 0);
   return qint64(tv.tv_sec) * Q_INT64_C(1000) + tv.tv_usec / 1000;
}

#else
#error "What system is this?"
#endif

QDateTime QDateTime::fromTime_t(uint seconds)
{
   return fromMSecsSinceEpoch((qint64)seconds * 1000, Qt::LocalTime);
}

QDateTime QDateTime::fromTime_t(uint seconds, Qt::TimeSpec spec, int offsetSeconds)
{
   return fromMSecsSinceEpoch((qint64)seconds * 1000, spec, offsetSeconds);
}

QDateTime QDateTime::fromMSecsSinceEpoch(qint64 msecs)
{
   return fromMSecsSinceEpoch(msecs, Qt::LocalTime);
}

QDateTime QDateTime::fromMSecsSinceEpoch(qint64 msecs, Qt::TimeSpec spec, int offsetSeconds)
{
   QDate newDate = QDate(1970, 1, 1);
   QTime newTime = QTime(0, 0, 0);
   QDateTimePrivate::addMSecs(newDate, newTime, msecs);

   switch (spec) {
      case Qt::UTC:
         return QDateTime(newDate, newTime, Qt::UTC);
      case Qt::OffsetFromUTC:
         utcToOffset(&newDate, &newTime, offsetSeconds);
         return QDateTime(newDate, newTime, Qt::OffsetFromUTC, offsetSeconds);
      default:
         utcToLocal(newDate, newTime);
         return QDateTime(newDate, newTime, Qt::LocalTime);
   }
}

void QDateTime::setUtcOffset(int seconds)
{
   detach();

   /* The motivation to also setting d->spec is to ensure that the QDateTime
    * instance stay in well-defined states all the time, instead of that
    * we instruct the user to ensure it. */
   if (seconds == 0) {
      d->spec = QDateTimePrivate::UTC;
   } else {
      d->spec = QDateTimePrivate::OffsetFromUTC;
   }

   /* Even if seconds is 0 we assign it to utcOffset. */
   d->m_offsetFromUtc = seconds;
}

int QDateTime::utcOffset() const
{
   return offsetFromUtc();
}

#ifndef QT_NO_DATESTRING

static int fromShortMonthName(const QString &monthName)
{
   // Assume that English monthnames are the default
   for (int i = 0; i < 12; ++i) {
      if (monthName == QString(qt_shortMonthNames[i])) {
         return i + 1;
      }
   }

   // If English names can't be found, search the localized ones
   for (int i = 1; i <= 12; ++i) {
      if (monthName == QDate::shortMonthName(i)) {
         return i;
      }
   }

   return -1;
}

QDateTime QDateTime::fromString(const QString &str, Qt::DateFormat format)
{
   if (str.isEmpty()) {
      return QDateTime();
   }

   switch (format) {
      case Qt::ISODate: {
         QString tmp = str;

         Qt::TimeSpec ts = Qt::LocalTime;
         QDate date      = QDate::fromString(tmp.left(10), Qt::ISODate);

         if (tmp.size() == 10) {
            return QDateTime(date);
         }

         tmp = tmp.mid(11);

         // Recognize UTC specifications
         if (tmp.endsWith('Z')) {
            ts = Qt::UTC;
            tmp.chop(1);
         }

         // Recognize timezone specifications
         int offset = 0;
         QString::const_iterator sign_iter = tmp.indexOfFast(QRegularExpression("[+-]"));

         if (sign_iter != tmp.end()) {
            bool ok;
            offset = fromOffsetString(QString(sign_iter, tmp.end()), &ok);

            if (! ok) {
               return QDateTime();
            }

            tmp = QString(tmp.begin(), sign_iter);
            ts  = Qt::OffsetFromUTC;
         }

         bool isMidnight24 = false;

         // Might be end of day (24:00, including variants), which QTime considers invalid.
         QTime time(fromString(tmp, Qt::ISODate).time());

         if (isMidnight24) {
            // ISO 8601 (section 4.2.3) says that 24:00 is equivalent to 00:00 the next day.
            date = date.addDays(1);
         }

         return QDateTime(date, time, ts, offset);
      }

      case Qt::SystemLocaleDate:
      case Qt::SystemLocaleShortDate:
      case Qt::SystemLocaleLongDate:
         return fromString(str, QLocale::system().dateTimeFormat(format == Qt::SystemLocaleLongDate ? QLocale::LongFormat : QLocale::ShortFormat));

      case Qt::LocaleDate:
      case Qt::DefaultLocaleShortDate:
      case Qt::DefaultLocaleLongDate:
         return fromString(str, QLocale().dateTimeFormat(format == Qt::DefaultLocaleLongDate ? QLocale::LongFormat: QLocale::ShortFormat));

#if ! defined(QT_NO_TEXTDATE)
      case Qt::TextDate: {
         QStringList parts = str.split(' ', QStringParser::SkipEmptyParts);

         if ((parts.count() < 5) || (parts.count() > 6)) {
            return QDateTime();
         }

         // Accept "Sun Dec 1 13:02:00 1974" and "Sun 1. Dec 13:02:00 1974"
         int month = -1;
         int day   = -1;

         bool ok;

         month = fromShortMonthName(parts.at(1));
         if (month != -1) {
            day = parts.at(2).toInteger<int>(&ok);

            if (! ok) {
               day = -1;
            }
         }

         if (month == -1 || day == -1) {
            // first variant failed, lets try the other
            month = fromShortMonthName(parts.at(2));

            if (month != -1) {
               QString dayStr = parts.at(1);

               if (dayStr.endsWith('.')) {
                  dayStr.chop(1);
                  day = dayStr.toInteger<int>(&ok);
                  if (!ok) {
                     day = -1;
                  }
               } else {
                  day = -1;
               }
            }
         }

         if (month == -1 || day == -1) {
            // both variants failed, give up
            return QDateTime();
         }

         int year;
         QStringList timeParts = parts.at(3).split(':');

         if ((timeParts.count() == 3) || (timeParts.count() == 2)) {
            year = parts.at(4).toInteger<int>(&ok);

            if (!ok) {
               return QDateTime();
            }

         } else {
            timeParts = parts.at(4).split(':');

            if ((timeParts.count() != 3) && (timeParts.count() != 2)) {
               return QDateTime();
            }

            year = parts.at(3).toInteger<int>(&ok);
            if (! ok) {
               return QDateTime();
            }
         }

         int hour = timeParts.at(0).toInteger<int>(&ok);
         if (! ok) {
            return QDateTime();
         }

         int minute = timeParts.at(1).toInteger<int>(&ok);
         if (! ok) {
            return QDateTime();
         }

         int second      = 0;
         int millisecond = 0;

         if (timeParts.count() > 2) {
            QStringList secondParts = timeParts.at(2).split('.');
            if (secondParts.size() > 2) {
               return QDateTime();
            }

            second = secondParts.first().toInteger<int>(&ok);
            if (!ok) {
               return QDateTime();
            }

            if (secondParts.size() > 1) {
               millisecond = secondParts.last().toInteger<int>(&ok);
               if (!ok) {
                  return QDateTime();
               }
            }
         }

         QDate date(year, month, day);
         QTime time(hour, minute, second, millisecond);

         if (parts.count() == 5) {
            return QDateTime(date, time, Qt::LocalTime);
         }

         QString tz = parts.at(5);
         if (!tz.startsWith(QLatin1String("GMT"), Qt::CaseInsensitive)) {
            return QDateTime();
         }

         tz.remove(0, 3);

         if (!tz.isEmpty()) {
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

#endif // QT_NO_DATESTRING

/*! \internal
 */
void QDateTime::detach()
{
   d.detach();
}

QDataStream &operator<<(QDataStream &out, const QDate &date)
{
   return out << (quint32)(date.jd);
}

QDataStream &operator>>(QDataStream &in, QDate &date)
{
   quint32 jd;
   in >> jd;
   date.jd = jd;

   return in;
}

QDataStream &operator<<(QDataStream &out, const QTime &time)
{
   return out << quint32(time.mds);
}

QDataStream &operator>>(QDataStream &in, QTime &time)
{
   quint32 ds;
   in >> ds;
   time.mds = int(ds);

   return in;
}

QDataStream &operator<<(QDataStream &out, const QDateTime &dateTime)
{
   out << dateTime.d->date << dateTime.d->time;
   out << (qint8)dateTime.d->spec;

   return out;
}

QDataStream &operator>>(QDataStream &in, QDateTime &dateTime)
{
   qint8 ts = (qint8)QDateTimePrivate::LocalUnknown;

   in >> ts;
   dateTime.d->spec = (QDateTimePrivate::Spec)ts;

   return in;
}

// checks if there is an unqoted 'AP' or 'ap' in the string
static bool hasUnquotedAP(const QString &f)
{
   bool inquote  = false;
   const int max = f.size();

   for (int i = 0; i < max; ++i) {

      if (f.at(i) == '\'') {
         inquote = !inquote;

      } else if (! inquote && f.at(i).toUpper() == "A") {
         return true;
      }
   }

   return false;
}

#ifndef QT_NO_DATESTRING

/*****************************************************************************
  Some static function used by QDate, QTime and QDateTime
*****************************************************************************/

// Replaces tokens by their value. See QDateTime::toString() for a list of valid tokens
static QString getFmtString(const QString &f, const QTime *dt = 0, const QDate *dd = 0, bool am_pm = false)
{
   if (f.isEmpty()) {
      return QString();
   }

   QString buf = f;
   int removed = 0;

   if (dt) {
      if (f.startsWith(QLatin1String("hh")) || f.startsWith(QLatin1String("HH"))) {
         const bool hour12 = f.at(0) == QLatin1Char('h') && am_pm;
         if (hour12 && dt->hour() > 12) {
            buf = QString::number(dt->hour() - 12).rightJustified(2, QLatin1Char('0'), true);
         } else if (hour12 && dt->hour() == 0) {
            buf = QLatin1String("12");
         } else {
            buf = QString::number(dt->hour()).rightJustified(2, QLatin1Char('0'), true);
         }
         removed = 2;
      } else if (f.at(0) == QLatin1Char('h') || f.at(0) == QLatin1Char('H')) {
         const bool hour12 = f.at(0) == QLatin1Char('h') && am_pm;
         if (hour12 && dt->hour() > 12) {
            buf = QString::number(dt->hour() - 12);
         } else if (hour12 && dt->hour() == 0) {
            buf = QLatin1String("12");
         } else {
            buf = QString::number(dt->hour());
         }
         removed = 1;
      } else if (f.startsWith(QLatin1String("mm"))) {
         buf = QString::number(dt->minute()).rightJustified(2, QLatin1Char('0'), true);
         removed = 2;
      } else if (f.at(0) == (QLatin1Char('m'))) {
         buf = QString::number(dt->minute());
         removed = 1;
      } else if (f.startsWith(QLatin1String("ss"))) {
         buf = QString::number(dt->second()).rightJustified(2, QLatin1Char('0'), true);
         removed = 2;
      } else if (f.at(0) == QLatin1Char('s')) {
         buf = QString::number(dt->second());
      } else if (f.startsWith(QLatin1String("zzz"))) {
         buf = QString::number(dt->msec()).rightJustified(3, QLatin1Char('0'), true);
         removed = 3;
      } else if (f.at(0) == QLatin1Char('z')) {
         buf = QString::number(dt->msec());
         removed = 1;
      } else if (f.at(0).toUpper() == QLatin1Char('A')) {
         const bool upper = f.at(0) == QLatin1Char('A');
         buf = dt->hour() < 12 ? QLatin1String("am") : QLatin1String("pm");
         if (upper) {
            buf = buf.toUpper();
         }
         if (f.size() > 1 && f.at(1).toUpper() == QLatin1Char('P') &&
               f.at(0).isUpper() == f.at(1).isUpper()) {
            removed = 2;
         } else {
            removed = 1;
         }
      }
   }

   if (dd) {
      if (f.startsWith(QLatin1String("dddd"))) {
         buf = dd->longDayName(dd->dayOfWeek());
         removed = 4;
      } else if (f.startsWith(QLatin1String("ddd"))) {
         buf = dd->shortDayName(dd->dayOfWeek());
         removed = 3;
      } else if (f.startsWith(QLatin1String("dd"))) {
         buf = QString::number(dd->day()).rightJustified(2, QLatin1Char('0'), true);
         removed = 2;
      } else if (f.at(0) == QLatin1Char('d')) {
         buf = QString::number(dd->day());
         removed = 1;
      } else if (f.startsWith(QLatin1String("MMMM"))) {
         buf = dd->longMonthName(dd->month());
         removed = 4;
      } else if (f.startsWith(QLatin1String("MMM"))) {
         buf = dd->shortMonthName(dd->month());
         removed = 3;
      } else if (f.startsWith(QLatin1String("MM"))) {
         buf = QString::number(dd->month()).rightJustified(2, QLatin1Char('0'), true);
         removed = 2;
      } else if (f.at(0) == QLatin1Char('M')) {
         buf = QString::number(dd->month());
         removed = 1;
      } else if (f.startsWith(QLatin1String("yyyy"))) {
         const int year = dd->year();
         buf = QString::number(qAbs(year)).rightJustified(4, QLatin1Char('0'));
         if (year > 0) {
            removed = 4;
         } else {
            buf.prepend(QLatin1Char('-'));
            removed = 5;
         }

      } else if (f.startsWith(QLatin1String("yy"))) {
         buf = QString::number(dd->year()).right(2).rightJustified(2, QLatin1Char('0'));
         removed = 2;
      }
   }
   if (removed == 0 || removed >= f.size()) {
      return buf;
   }

   return buf + getFmtString(f.mid(removed), dt, dd, am_pm);
}

// Parses the format string and uses getFmtString to get the values for the tokens. Ret
static QString fmtDateTime(const QString &f, const QTime *dt, const QDate *dd)
{
   const QLatin1Char quote('\'');
   if (f.isEmpty()) {
      return QString();
   }
   if (dt && !dt->isValid()) {
      return QString();
   }
   if (dd && !dd->isValid()) {
      return QString();
   }

   const bool ap = hasUnquotedAP(f);

   QString buf;
   QString frm;
   QChar status(QLatin1Char('0'));

   for (int i = 0; i < (int)f.length(); ++i) {
      if (f.at(i) == quote) {
         if (status == quote) {
            if (i > 0 && f.at(i - 1) == quote) {
               buf += QLatin1Char('\'');
            }
            status = QLatin1Char('0');
         } else {
            if (!frm.isEmpty()) {
               buf += getFmtString(frm, dt, dd, ap);
               frm.clear();
            }
            status = quote;
         }
      } else if (status == quote) {
         buf += f.at(i);
      } else if (f.at(i) == status) {
         if ((ap) && ((f.at(i) == QLatin1Char('P')) || (f.at(i) == QLatin1Char('p')))) {
            status = QLatin1Char('0');
         }
         frm += f.at(i);
      } else {
         buf += getFmtString(frm, dt, dd, ap);
         frm.clear();
         if ((f.at(i) == QLatin1Char('h')) || (f.at(i) == QLatin1Char('m'))
               || (f.at(i) == QLatin1Char('H'))
               || (f.at(i) == QLatin1Char('s')) || (f.at(i) == QLatin1Char('z'))) {
            status = f.at(i);
            frm += f.at(i);
         } else if ((f.at(i) == QLatin1Char('d')) || (f.at(i) == QLatin1Char('M')) || (f.at(i) == QLatin1Char('y'))) {
            status = f.at(i);
            frm += f.at(i);
         } else if ((ap) && (f.at(i) == QLatin1Char('A'))) {
            status = QLatin1Char('P');
            frm += f.at(i);
         } else  if ((ap) && (f.at(i) == QLatin1Char('a'))) {
            status = QLatin1Char('p');
            frm += f.at(i);
         } else {
            buf += f.at(i);
            status = QLatin1Char('0');
         }
      }
   }

   buf += getFmtString(frm, dt, dd, ap);

   return buf;
}
#endif // QT_NO_DATESTRING

#ifdef Q_OS_WIN
static const int LowerYear = 1980;
#else
static const int LowerYear = 1970;
#endif
static const int UpperYear = 2037;

static QDate adjustDate(QDate date)
{
   QDate lowerLimit(LowerYear, 1, 2);
   QDate upperLimit(UpperYear, 12, 30);

   if (date > lowerLimit && date < upperLimit) {
      return date;
   }

   int month = date.month();
   int day = date.day();

   // neither 1970 nor 2037 are leap years, so make sure date isn't Feb 29
   if (month == 2 && day == 29) {
      --day;
   }

   if (date < lowerLimit) {
      date.setDate(LowerYear, month, day);
   } else {
      date.setDate(UpperYear, month, day);
   }

   return date;
}

// Convert passed in UTC datetime into LocalTime and return spec
static QDateTimePrivate::Spec utcToLocal(QDate &date, QTime &time)
{
   QDate fakeDate = adjustDate(date);

   // won't overflow because of fakeDate
   time_t secsSince1Jan1970UTC = toMSecsSinceEpoch_helper(fakeDate.toJulianDay(), QTime().msecsTo(time)) / 1000;
   tm *brokenDown = 0;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
   // use the reentrant version of localtime() where available
   internal_tzset();
   tm res;
   brokenDown = localtime_r(&secsSince1Jan1970UTC, &res);
#else
   brokenDown = localtime(&secsSince1Jan1970UTC);
#endif

   if (! brokenDown) {
      date = QDate(1970, 1, 1);
      time = QTime();
      return QDateTimePrivate::LocalUnknown;
   } else {
      qint64 deltaDays = fakeDate.daysTo(date);
      date = QDate(brokenDown->tm_year + 1900, brokenDown->tm_mon + 1, brokenDown->tm_mday);
      time = QTime(brokenDown->tm_hour, brokenDown->tm_min, brokenDown->tm_sec, time.msec());
      date = date.addDays(deltaDays);
      if (brokenDown->tm_isdst > 0) {
         return QDateTimePrivate::LocalDST;
      } else if (brokenDown->tm_isdst < 0) {
         return QDateTimePrivate::LocalUnknown;
      } else {
         return QDateTimePrivate::LocalStandard;
      }
   }
}

// Convert passed in LocalTime datetime into UTC
static void localToUtc(QDate &date, QTime &time, int isdst)
{
   if (!date.isValid()) {
      return;
   }

   QDate fakeDate = adjustDate(date);

   tm localTM;
   localTM.tm_sec = time.second();
   localTM.tm_min = time.minute();
   localTM.tm_hour = time.hour();
   localTM.tm_mday = fakeDate.day();
   localTM.tm_mon = fakeDate.month() - 1;
   localTM.tm_year = fakeDate.year() - 1900;
   localTM.tm_isdst = (int)isdst;

#if defined(Q_OS_WIN)
   _tzset();
#endif

   time_t secsSince1Jan1970UTC = mktime(&localTM);
   tm *brokenDown = 0;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
   // use the reentrant version of gmtime() where available
   tm res;
   brokenDown = gmtime_r(&secsSince1Jan1970UTC, &res);
#else
   brokenDown = gmtime(&secsSince1Jan1970UTC);
#endif // _POSIX_THREAD_SAFE_FUNCTIONS

   if (! brokenDown) {
      date = QDate(1970, 1, 1);
      time = QTime();
   } else {
      qint64 deltaDays = fakeDate.daysTo(date);
      date = QDate(brokenDown->tm_year + 1900, brokenDown->tm_mon + 1, brokenDown->tm_mday);
      time = QTime(brokenDown->tm_hour, brokenDown->tm_min, brokenDown->tm_sec, time.msec());
      date = date.addDays(deltaDays);
   }
}

// Convert passed in OffsetFromUTC datetime and offset into UTC
static void offsetToUtc(QDate *outDate, QTime *outTime, qint32 offset)
{
   QDateTimePrivate::addMSecs(*outDate, *outTime, -(qint64(offset) * 1000));
}

// Convert passed in UTC datetime and offset into OffsetFromUTC
static void utcToOffset(QDate *outDate, QTime *outTime, qint32 offset)
{
   QDateTimePrivate::addMSecs(*outDate, *outTime, (qint64(offset) * 1000));
}

// Get current date/time in LocalTime and put result in outDate and outTime
QDateTimePrivate::Spec QDateTimePrivate::getLocal(QDate &outDate, QTime &outTime) const
{
   outDate = date;
   outTime = time;
   if (spec == QDateTimePrivate::UTC) {
      return utcToLocal(outDate, outTime);
   }
   if (spec == QDateTimePrivate::OffsetFromUTC) {
      offsetToUtc(&outDate, &outTime, m_offsetFromUtc);
      return utcToLocal(outDate, outTime);
   }
   return spec;
}

// Get current date/time in UTC and put result in outDate and outTime
void QDateTimePrivate::getUTC(QDate &outDate, QTime &outTime) const
{
   outDate = date;
   outTime = time;

   if (spec == QDateTimePrivate::OffsetFromUTC) {
      offsetToUtc(&outDate, &outTime, m_offsetFromUtc);
   } else if (spec != QDateTimePrivate::UTC) {
      localToUtc(outDate, outTime, (int)spec);
   }
}

#if ! defined(QT_NO_DATESTRING)
QDebug operator<<(QDebug dbg, const QDate &date)
{
   dbg.nospace() << "QDate(" << date.toString() << ')';
   return dbg.space();
}

QDebug operator<<(QDebug dbg, const QTime &time)
{
   dbg.nospace() << "QTime(" << time.toString() << ')';
   return dbg.space();
}

QDebug operator<<(QDebug dbg, const QDateTime &date)
{
   dbg.nospace() << "QDateTime(" << date.toString() << ')';
   return dbg.space();
}
#endif

int QDateTimeParser::getDigit(const QDateTime &t, int index) const
{
   if (index < 0 || index >= sectionNodes.size()) {
#ifndef QT_NO_DATESTRING
      qWarning("QDateTimeParser::getDigit() Internal error (%s %d)",
               qPrintable(t.toString()), index);
#else
      qWarning("QDateTimeParser::getDigit() Internal error (%d)", index);
#endif
      return -1;
   }
   const SectionNode &node = sectionNodes.at(index);
   switch (node.type) {
      case Hour24Section:
      case Hour12Section:
         return t.time().hour();
      case MinuteSection:
         return t.time().minute();
      case SecondSection:
         return t.time().second();
      case MSecSection:
         return t.time().msec();
      case YearSection2Digits:
      case YearSection:
         return t.date().year();
      case MonthSection:
         return t.date().month();
      case DaySection:
         return t.date().day();
      case DayOfWeekSection:
         return t.date().day();
      case AmPmSection:
         return t.time().hour() > 11 ? 1 : 0;

      default:
         break;
   }

#ifndef QT_NO_DATESTRING
   qWarning("QDateTimeParser::getDigit() Internal error 2 (%s %d)",
            qPrintable(t.toString()), index);
#else
   qWarning("QDateTimeParser::getDigit() Internal error 2 (%d)", index);
#endif
   return -1;
}


bool QDateTimeParser::setDigit(QDateTime &v, int index, int newVal) const
{
   if (index < 0 || index >= sectionNodes.size()) {
#ifndef QT_NO_DATESTRING
      qWarning("QDateTimeParser::setDigit() Internal error (%s %d %d)",
               qPrintable(v.toString()), index, newVal);
#else
      qWarning("QDateTimeParser::setDigit() Internal error (%d %d)", index, newVal);
#endif
      return false;
   }
   const SectionNode &node = sectionNodes.at(index);

   int year, month, day, hour, minute, second, msec;
   year = v.date().year();
   month = v.date().month();
   day = v.date().day();
   hour = v.time().hour();
   minute = v.time().minute();
   second = v.time().second();
   msec = v.time().msec();

   switch (node.type) {
      case Hour24Section:
      case Hour12Section:
         hour = newVal;
         break;
      case MinuteSection:
         minute = newVal;
         break;
      case SecondSection:
         second = newVal;
         break;
      case MSecSection:
         msec = newVal;
         break;
      case YearSection2Digits:
      case YearSection:
         year = newVal;
         break;
      case MonthSection:
         month = newVal;
         break;
      case DaySection:
      case DayOfWeekSection:
         if (newVal > 31) {
            // have to keep legacy behavior. setting the
            // date to 32 should return false. Setting it
            // to 31 for february should return true
            return false;
         }
         day = newVal;
         break;

      case AmPmSection:
         hour = (newVal == 0 ? hour % 12 : (hour % 12) + 12);
         break;

      default:
         qWarning("QDateTimeParser::setDigit() Internal error (%s)", qPrintable(sectionName(node.type)));
         break;
   }

   if (!(node.type & (DaySection | DayOfWeekSection))) {
      if (day < cachedDay) {
         day = cachedDay;
      }
      const int max = QDate(year, month, 1).daysInMonth();
      if (day > max) {
         day = max;
      }
   }
   if (QDate::isValid(year, month, day) && QTime::isValid(hour, minute, second, msec)) {
      v = QDateTime(QDate(year, month, day), QTime(hour, minute, second, msec), spec);
      return true;
   }
   return false;
}



/*!
  \

  Returns the absolute maximum for a section
*/

int QDateTimeParser::absoluteMax(int s, const QDateTime &cur) const
{
   const SectionNode &sn = sectionNode(s);
   switch (sn.type) {
      case Hour24Section:
      case Hour12Section:
         return 23; // this is special-cased in
      // parseSection. We want it to be
      // 23 for the stepBy case.
      case MinuteSection:
      case SecondSection:
         return 59;
      case MSecSection:
         return 999;
      case YearSection2Digits:
      case YearSection:
         return 9999; // sectionMaxSize will prevent
      // people from typing in a larger
      // number in count == 2 sections.
      // stepBy() will work on real years anyway
      case MonthSection:
         return 12;
      case DaySection:
      case DayOfWeekSection:
         return cur.isValid() ? cur.date().daysInMonth() : 31;
      case AmPmSection:
         return 1;
      default:
         break;
   }
   qWarning("QDateTimeParser::absoluteMax() Internal error (%s)",
            qPrintable(sectionName(sn.type)));
   return -1;
}

/*!
  \internal

  Returns the absolute minimum for a section
*/

int QDateTimeParser::absoluteMin(int s) const
{
   const SectionNode &sn = sectionNode(s);
   switch (sn.type) {
      case Hour24Section:
      case Hour12Section:
      case MinuteSection:
      case SecondSection:
      case MSecSection:
      case YearSection2Digits:
      case YearSection:
         return 0;
      case MonthSection:
      case DaySection:
      case DayOfWeekSection:
         return 1;
      case AmPmSection:
         return 0;
      default:
         break;
   }
   qWarning("QDateTimeParser::absoluteMin() Internal error (%s, %0x)",
            qPrintable(sectionName(sn.type)), sn.type);
   return -1;
}

/*!
  \internal

  Returns the sectionNode for the Section \a s.
*/

const QDateTimeParser::SectionNode &QDateTimeParser::sectionNode(int sectionIndex) const
{
   if (sectionIndex < 0) {
      switch (sectionIndex) {
         case FirstSectionIndex:
            return first;
         case LastSectionIndex:
            return last;
         case NoSectionIndex:
            return none;
      }
   } else if (sectionIndex < sectionNodes.size()) {
      return sectionNodes.at(sectionIndex);
   }

   qWarning("QDateTimeParser::sectionNode() Internal error (%d)",
            sectionIndex);
   return none;
}

QDateTimeParser::Section QDateTimeParser::sectionType(int sectionIndex) const
{
   return sectionNode(sectionIndex).type;
}


/*!
  \internal

  Returns the starting position for section \a s.
*/

int QDateTimeParser::sectionPos(int sectionIndex) const
{
   return sectionPos(sectionNode(sectionIndex));
}

int QDateTimeParser::sectionPos(const SectionNode &sn) const
{
   switch (sn.type) {
      case FirstSection:
         return 0;
      case LastSection:
         return displayText().size() - 1;
      default:
         break;
   }
   if (sn.pos == -1) {
      qWarning("QDateTimeParser::sectionPos Internal error (%s)", qPrintable(sectionName(sn.type)));
      return -1;
   }
   return sn.pos;
}


/*!
  \internal helper function for parseFormat. removes quotes that are
  not escaped and removes the escaping on those that are escaped

*/

static QString unquote(const QString &str)
{
   const QChar quote(QLatin1Char('\''));
   const QChar slash(QLatin1Char('\\'));
   const QChar zero(QLatin1Char('0'));
   QString ret;
   QChar status(zero);
   const int max = str.size();
   for (int i = 0; i < max; ++i) {
      if (str.at(i) == quote) {
         if (status != quote) {
            status = quote;
         } else if (!ret.isEmpty() && str.at(i - 1) == slash) {
            ret[ret.size() - 1] = quote;
         } else {
            status = zero;
         }
      } else {
         ret += str.at(i);
      }
   }
   return ret;
}
/*!
  \internal

  Parses the format \a newFormat. If successful, returns true and
  sets up the format. Else keeps the old format and returns false.

*/

static inline int countRepeat(const QString &str, int index, int maxCount)
{
   int count = 1;
   const QChar ch(str.at(index));
   const int max = qMin(index + maxCount, str.size());
   while (index + count < max && str.at(index + count) == ch) {
      ++count;
   }
   return count;
}

static inline void appendSeparator(QStringList *list, const QString &string, int from, int size, int lastQuote)
{
   QString str(string.mid(from, size));
   if (lastQuote >= from) {
      str = unquote(str);
   }
   list->append(str);
}


bool QDateTimeParser::parseFormat(const QString &newFormat)
{
   const QLatin1Char quote('\'');
   const QLatin1Char slash('\\');
   const QLatin1Char zero('0');

   if (newFormat == displayFormat && !newFormat.isEmpty()) {
      return true;
   }

   QDT_DEBUGN("parseFormat: %s", newFormat.toLatin1().constData());

   QVector<SectionNode> newSectionNodes;
   Sections newDisplay = 0;
   QStringList newSeparators;
   int i, index = 0;
   int add = 0;
   QChar status(zero);
   const int max = newFormat.size();
   int lastQuote = -1;
   for (i = 0; i < max; ++i) {
      if (newFormat.at(i) == quote) {
         lastQuote = i;
         ++add;
         if (status != quote) {
            status = quote;
         } else if (newFormat.at(i - 1) != slash) {
            status = zero;
         }
      } else if (status != quote) {
         const char sect = newFormat.at(i).toLatin1();
         switch (sect) {
            case 'H':
            case 'h':
               if (parserType != QVariant::Date) {
                  const Section hour = (sect == 'h') ? Hour12Section : Hour24Section;
                  const SectionNode sn = { hour, i - add, countRepeat(newFormat, i, 2), 0 };
                  newSectionNodes.append(sn);
                  appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                  i += sn.count - 1;
                  index = i + 1;
                  newDisplay |= hour;
               }
               break;
            case 'm':
               if (parserType != QVariant::Date) {
                  const SectionNode sn = { MinuteSection, i - add, countRepeat(newFormat, i, 2), 0 };
                  newSectionNodes.append(sn);
                  appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                  i += sn.count - 1;
                  index = i + 1;
                  newDisplay |= MinuteSection;
               }
               break;
            case 's':
               if (parserType != QVariant::Date) {
                  const SectionNode sn = { SecondSection, i - add, countRepeat(newFormat, i, 2), 0 };
                  newSectionNodes.append(sn);
                  appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                  i += sn.count - 1;
                  index = i + 1;
                  newDisplay |= SecondSection;
               }
               break;

            case 'z':
               if (parserType != QVariant::Date) {
                  const SectionNode sn = { MSecSection, i - add, countRepeat(newFormat, i, 3) < 3 ? 1 : 3, 0 };
                  newSectionNodes.append(sn);
                  appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                  i += sn.count - 1;
                  index = i + 1;
                  newDisplay |= MSecSection;
               }
               break;
            case 'A':
            case 'a':
               if (parserType != QVariant::Date) {
                  const bool cap = (sect == 'A');
                  const SectionNode sn = { AmPmSection, i - add, (cap ? 1 : 0), 0 };
                  newSectionNodes.append(sn);
                  appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                  newDisplay |= AmPmSection;
                  if (i + 1 < newFormat.size()
                        && newFormat.at(i + 1) == (cap ? QLatin1Char('P') : QLatin1Char('p'))) {
                     ++i;
                  }
                  index = i + 1;
               }
               break;
            case 'y':
               if (parserType != QVariant::Time) {
                  const int repeat = countRepeat(newFormat, i, 4);
                  if (repeat >= 2) {
                     const SectionNode sn = { repeat == 4 ? YearSection : YearSection2Digits,
                                              i - add, repeat == 4 ? 4 : 2, 0
                                            };
                     newSectionNodes.append(sn);
                     appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                     i += sn.count - 1;
                     index = i + 1;
                     newDisplay |= sn.type;
                  }
               }
               break;
            case 'M':
               if (parserType != QVariant::Time) {
                  const SectionNode sn = { MonthSection, i - add, countRepeat(newFormat, i, 4), 0 };
                  newSectionNodes.append(sn);
                  newSeparators.append(unquote(newFormat.mid(index, i - index)));
                  i += sn.count - 1;
                  index = i + 1;
                  newDisplay |= MonthSection;
               }
               break;
            case 'd':
               if (parserType != QVariant::Time) {
                  const int repeat = countRepeat(newFormat, i, 4);
                  const SectionNode sn = { repeat >= 3 ? DayOfWeekSection : DaySection, i - add, repeat, 0 };
                  newSectionNodes.append(sn);
                  appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                  i += sn.count - 1;
                  index = i + 1;
                  newDisplay |= sn.type;
               }
               break;

            default:
               break;
         }
      }
   }
   if (newSectionNodes.isEmpty() && context == DateTimeEdit) {
      return false;
   }

   if ((newDisplay & (AmPmSection | Hour12Section)) == Hour12Section) {
      const int max = newSectionNodes.size();
      for (int i = 0; i < max; ++i) {
         SectionNode &node = newSectionNodes[i];
         if (node.type == Hour12Section) {
            node.type = Hour24Section;
         }
      }
   }

   if (index < newFormat.size()) {
      appendSeparator(&newSeparators, newFormat, index, index - max, lastQuote);
   } else {
      newSeparators.append(QString());
   }

   displayFormat = newFormat;
   separators = newSeparators;
   sectionNodes = newSectionNodes;
   display = newDisplay;
   last.pos = -1;

   QDT_DEBUG << newFormat << displayFormat;
   QDT_DEBUGN("separators:\n'%s'", separators.join("\n").toLatin1().constData());

   return true;
}

/*!
  \internal

  Returns the size of section \a s.
*/

int QDateTimeParser::sectionSize(int sectionIndex) const
{
   if (sectionIndex < 0) {
      return 0;
   }

   if (sectionIndex >= sectionNodes.size()) {
      qWarning("QDateTimeParser::sectionSize Internal error (%d)", sectionIndex);
      return -1;
   }

   if (sectionIndex == sectionNodes.size() - 1) {
      // In some cases there is a difference between displayText() and text.
      // e.g. when text is 2000/01/31 and displayText() is "2000/2/31" - text
      // is the previous value and displayText() is the new value.
      // The size difference is always due to leading zeroes.
      int sizeAdjustment = 0;

      if (displayText().size() != text.size()) {
         // Any zeroes added before this section will affect our size.
         int preceedingZeroesAdded = 0;

         if (sectionNodes.size() > 1 && context == DateTimeEdit) {

            for (QVector<SectionNode>::const_iterator sectionIt = sectionNodes.constBegin();
                  sectionIt != sectionNodes.constBegin() + sectionIndex; ++sectionIt) {
               preceedingZeroesAdded += sectionIt->zeroesAdded;
            }
         }
         sizeAdjustment = preceedingZeroesAdded;
      }

      return displayText().size() + sizeAdjustment - sectionPos(sectionIndex) - separators.last().size();
   } else {
      return sectionPos(sectionIndex + 1) - sectionPos(sectionIndex)
             - separators.at(sectionIndex + 1).size();
   }
}


int QDateTimeParser::sectionMaxSize(Section s, int count) const
{
#ifndef QT_NO_TEXTDATE
   int mcount = 12;
#endif

   switch (s) {
      case FirstSection:
      case NoSection:
      case LastSection:
         return 0;

      case AmPmSection: {
         const int lowerMax = qMin(getAmPmText(AmText, LowerCase).size(),
                                   getAmPmText(PmText, LowerCase).size());
         const int upperMax = qMin(getAmPmText(AmText, UpperCase).size(),
                                   getAmPmText(PmText, UpperCase).size());
         return qMin(4, qMin(lowerMax, upperMax));
      }

      case Hour24Section:
      case Hour12Section:
      case MinuteSection:
      case SecondSection:
      case DaySection:
         return 2;
      case DayOfWeekSection:
#ifdef QT_NO_TEXTDATE
         return 2;
#else
         mcount = 7;
         // fall through
#endif
      case MonthSection:
         if (count <= 2) {
            return 2;
         }

#ifdef QT_NO_TEXTDATE
         return 2;
#else
         {
            int ret = 0;
            const QLocale l = locale();
            for (int i = 1; i <= mcount; ++i) {
               const QString str = (s == MonthSection
                                    ? l.monthName(i, count == 4 ? QLocale::LongFormat : QLocale::ShortFormat)
                                    : l.dayName(i, count == 4 ? QLocale::LongFormat : QLocale::ShortFormat));
               ret = qMax(str.size(), ret);
            }
            return ret;
         }
#endif
      case MSecSection:
         return 3;
      case YearSection:
         return 4;
      case YearSection2Digits:
         return 2;

      case CalendarPopupSection:
      case Internal:
      case TimeSectionMask:
      case DateSectionMask:
         qWarning("QDateTimeParser::sectionMaxSize: Invalid section %s",
                  sectionName(s).toLatin1().constData());

      case NoSectionIndex:
      case FirstSectionIndex:
      case LastSectionIndex:
      case CalendarPopupIndex:
         // these cases can't happen
         break;
   }
   return -1;
}


int QDateTimeParser::sectionMaxSize(int index) const
{
   const SectionNode &sn = sectionNode(index);
   return sectionMaxSize(sn.type, sn.count);
}

/*!
  \internal

  Returns the text of section \a s. This function operates on the
  arg text rather than edit->text().
*/


QString QDateTimeParser::sectionText(const QString &text, int sectionIndex, int index) const
{
   const SectionNode &sn = sectionNode(sectionIndex);
   switch (sn.type) {
      case NoSectionIndex:
      case FirstSectionIndex:
      case LastSectionIndex:
         return QString();
      default:
         break;
   }

   return text.mid(index, sectionSize(sectionIndex));
}

QString QDateTimeParser::sectionText(int sectionIndex) const
{
   const SectionNode &sn = sectionNode(sectionIndex);
   switch (sn.type) {
      case NoSectionIndex:
      case FirstSectionIndex:
      case LastSectionIndex:
         return QString();
      default:
         break;
   }

   return displayText().mid(sn.pos, sectionSize(sectionIndex));
}


#ifndef QT_NO_TEXTDATE
/*!
  \internal:skipToNextSection

  Parses the part of \a text that corresponds to \a s and returns
  the value of that field. Sets *stateptr to the right state if
  stateptr != 0.
*/

int QDateTimeParser::parseSection(const QDateTime &currentValue, int sectionIndex,
                                  QString &text, int &cursorPosition, int index,
                                  State &state, int *usedptr) const
{
   state = Invalid;
   int num = 0;
   const SectionNode &sn = sectionNode(sectionIndex);
   if ((sn.type & Internal) == Internal) {
      qWarning("QDateTimeParser::parseSection Internal error (%s %d)",
               qPrintable(sectionName(sn.type)), sectionIndex);
      return -1;
   }

   const int sectionmaxsize = sectionMaxSize(sectionIndex);
   QString sectiontext = text.mid(index, sectionmaxsize);
   int sectiontextSize = sectiontext.size();

   QDT_DEBUG << "sectionValue for" << sectionName(sn.type)
             << "with text" << text << "and st" << sectiontext
             << text.mid(index, sectionmaxsize)
             << index;

   int used = 0;
   switch (sn.type) {
      case AmPmSection: {
         const int ampm = findAmPm(sectiontext, sectionIndex, &used);
         switch (ampm) {
            case AM: // sectiontext == AM
            case PM: // sectiontext == PM
               num = ampm;
               state = Acceptable;
               break;
            case PossibleAM: // sectiontext => AM
            case PossiblePM: // sectiontext => PM
               num = ampm - 2;
               state = Intermediate;
               break;
            case PossibleBoth: // sectiontext => AM|PM
               num = 0;
               state = Intermediate;
               break;
            case Neither:
               state = Invalid;
               QDT_DEBUG << "invalid because findAmPm(" << sectiontext << ") returned -1";
               break;
            default:
               QDT_DEBUGN("This should never happen (findAmPm returned %d)", ampm);
               break;
         }

         if (state != Invalid) {
            QString str = text;
            text.replace(index, used, sectiontext.left(used));
         }
         break;
      }

      case MonthSection:
      case DayOfWeekSection:
         if (sn.count >= 3) {
            if (sn.type == MonthSection) {
               int min = 1;
               const QDate minDate = getMinimum().date();
               if (currentValue.date().year() == minDate.year()) {
                  min = minDate.month();
               }
               num = findMonth(sectiontext.toLower(), min, sectionIndex, &sectiontext, &used);
            } else {
               num = findDay(sectiontext.toLower(), 1, sectionIndex, &sectiontext, &used);
            }

            if (num != -1) {
               state = (used == sectiontext.size() ? Acceptable : Intermediate);
               QString str = text;
               text.replace(index, used, sectiontext.left(used));
            } else {
               state = Intermediate;
            }
            break;
         }
      // fall through
      case DaySection:
      case YearSection:
      case YearSection2Digits:
      case Hour12Section:
      case Hour24Section:
      case MinuteSection:
      case SecondSection:
      case MSecSection: {
         if (sectiontextSize == 0) {
            num = 0;
            used = 0;
            state = Intermediate;
         } else {
            const int absMax = absoluteMax(sectionIndex);
            QLocale loc;
            bool ok = true;
            int last = -1;
            used = -1;

            QString digitsStr(sectiontext);
            for (int i = 0; i < sectiontextSize; ++i) {
               if (digitsStr.at(i).isSpace()) {
                  sectiontextSize = i;
                  break;
               }
            }

            const int max = qMin(sectionmaxsize, sectiontextSize);
            for (int digits = max; digits >= 1; --digits) {
               digitsStr.truncate(digits);
               int tmp = (int)loc.toUInt(digitsStr, &ok, 10);
               if (ok && sn.type == Hour12Section) {
                  if (tmp > 12) {
                     tmp = -1;
                     ok = false;
                  } else if (tmp == 12) {
                     tmp = 0;
                  }
               }
               if (ok && tmp <= absMax) {
                  QDT_DEBUG << sectiontext.left(digits) << tmp << digits;
                  last = tmp;
                  used = digits;
                  break;
               }
            }

            if (last == -1) {
               QChar first(sectiontext.at(0));
               if (separators.at(sectionIndex + 1).startsWith(first)) {
                  used = 0;
                  state = Intermediate;
               } else {
                  state = Invalid;
                  QDT_DEBUG << "invalid because" << sectiontext << "can't become a uint" << last << ok;
               }
            } else {
               num += last;
               const FieldInfo fi = fieldInfo(sectionIndex);
               const bool done = (used == sectionmaxsize);
               if (!done && fi & Fraction) { // typing 2 in a zzz field should be .200, not .002
                  for (int i = used; i < sectionmaxsize; ++i) {
                     num *= 10;
                  }
               }
               const int absMin = absoluteMin(sectionIndex);
               if (num < absMin) {
                  state = done ? Invalid : Intermediate;
                  if (done) {
                     QDT_DEBUG << "invalid because" << num << "is less than absoluteMin" << absMin;
                  }
               } else if (num > absMax) {
                  state = Intermediate;
               } else if (!done && (fi & (FixedWidth | Numeric)) == (FixedWidth | Numeric)) {
                  if (skipToNextSection(sectionIndex, currentValue, digitsStr)) {
                     state = Acceptable;
                     const int missingZeroes = sectionmaxsize - digitsStr.size();
                     text.insert(index, QString().fill(QLatin1Char('0'), missingZeroes));
                     used = sectionmaxsize;
                     cursorPosition += missingZeroes;
                     ++(const_cast<QDateTimeParser *>(this)->sectionNodes[sectionIndex].zeroesAdded);
                  } else {
                     state = Intermediate;;
                  }
               } else {
                  state = Acceptable;
               }
            }
         }
         break;
      }
      default:
         qWarning("QDateTimeParser::parseSection Internal error (%s %d)",
                  qPrintable(sectionName(sn.type)), sectionIndex);
         return -1;
   }

   if (usedptr) {
      *usedptr = used;
   }

   return (state != Invalid ? num : -1);
}
#endif // QT_NO_TEXTDATE

#ifndef QT_NO_DATESTRING
/*!
  \internal
*/

QDateTimeParser::StateNode QDateTimeParser::parse(QString &input, int &cursorPosition,
                  const QDateTime &currentValue, bool fixup) const
{
   const QDateTime minimum = getMinimum();
   const QDateTime maximum = getMaximum();

   State state = Acceptable;

   QDateTime newCurrentValue;
   int pos = 0;
   bool conflicts = false;
   const int sectionNodesCount = sectionNodes.size();

   QDT_DEBUG << "parse" << input;

   {
      int year, month, day, hour12, hour, minute, second, msec, ampm, dayofweek, year2digits;
      getDateFromJulianDay(currentValue.date().toJulianDay(), &year, &month, &day);
      year2digits = year % 100;
      hour = currentValue.time().hour();
      hour12 = -1;
      minute = currentValue.time().minute();
      second = currentValue.time().second();
      msec = currentValue.time().msec();
      dayofweek = currentValue.date().dayOfWeek();

      ampm = -1;
      Sections isSet = NoSection;
      int num;
      State tmpstate;

      for (int index = 0; state != Invalid && index < sectionNodesCount; ++index) {

         QStringView tmpView(input.begin() + pos, input.begin() + pos + separators.at(index).size());

         if (tmpView != separators.at(index)) {

            QDT_DEBUG << "invalid because" << input.mid(pos, separators.at(index).size())
                      << "!=" << separators.at(index)
                      << index << pos << currentSectionIndex;

            state = Invalid;
            goto end;
         }

         pos += separators.at(index).size();
         sectionNodes[index].pos = pos;
         int *current = 0;
         const SectionNode sn = sectionNodes.at(index);
         int used;

         num = parseSection(currentValue, index, input, cursorPosition, pos, tmpstate, &used);
         QDT_DEBUG << "sectionValue" << sectionName(sectionType(index)) << input
                   << "pos" << pos << "used" << used << stateName(tmpstate);

         if (fixup && tmpstate == Intermediate && used < sn.count) {
            const FieldInfo fi = fieldInfo(index);

            if ((fi & (Numeric | FixedWidth)) == (Numeric | FixedWidth)) {
               const QString newText = QString::fromLatin1("%1").formatArg(num, sn.count, 10, QLatin1Char('0'));
               input.replace(pos, used, newText);
               used = sn.count;
            }
         }

         pos  += qMax(0, used);
         state = qMin(state, tmpstate);

         if (state == Intermediate && context == FromString) {
            state = Invalid;
            break;
         }

         QDT_DEBUG << index << sectionName(sectionType(index)) << "is set to"
                   << pos << "state is" << stateName(state);

         if (state != Invalid) {
            switch (sn.type) {
               case Hour24Section:
                  current = &hour;
                  break;
               case Hour12Section:
                  current = &hour12;
                  break;
               case MinuteSection:
                  current = &minute;
                  break;
               case SecondSection:
                  current = &second;
                  break;
               case MSecSection:
                  current = &msec;
                  break;
               case YearSection:
                  current = &year;
                  break;
               case YearSection2Digits:
                  current = &year2digits;
                  break;
               case MonthSection:
                  current = &month;
                  break;
               case DayOfWeekSection:
                  current = &dayofweek;
                  break;
               case DaySection:
                  current = &day;
                  num = qMax(1, num);
                  break;
               case AmPmSection:
                  current = &ampm;
                  break;

               default:
                  qWarning("QDateTimeParser::parse Internal error (%s)", csPrintable(sectionName(sn.type)));
                  break;
            }

            if (!current) {
               qWarning("QDateTimeParser::parse Internal error 2");
               return StateNode();
            }

            if (isSet & sn.type && *current != num) {
               QDT_DEBUG << "CONFLICT " << sectionName(sn.type) << *current << num;
               conflicts = true;

               if (index != currentSectionIndex || num == -1) {
                  continue;
               }
            }

            if (num != -1) {
               *current = num;
            }

            isSet |= sn.type;
         }
      }

      QStringView tmpView(input.begin() + pos, input.end());

      if (state != Invalid && tmpView != separators.last()) {
         QDT_DEBUG << "invalid because" << tmpView << "!=" << separators.last() << pos;
         state = Invalid;
      }

      if (state != Invalid) {
         if (parserType != QVariant::Time) {

            if (year % 100 != year2digits) {

               switch (isSet & (YearSection2Digits | YearSection)) {
                  case YearSection2Digits:
                     year = (year / 100) * 100;
                     year += year2digits;
                     break;

                  case ((uint)YearSection2Digits|(uint)YearSection): {
                     conflicts = true;
                     const SectionNode &sn = sectionNode(currentSectionIndex);
                     if (sn.type == YearSection2Digits) {
                        year = (year / 100) * 100;
                        year += year2digits;
                     }
                     break;
                  }

                  default:
                     break;
               }
            }

            const QDate date(year, month, day);
            const int diff = dayofweek - date.dayOfWeek();

            if (diff != 0 && state == Acceptable && isSet & DayOfWeekSection) {
               conflicts = isSet & DaySection;
               const SectionNode &sn = sectionNode(currentSectionIndex);

               if (sn.type == DayOfWeekSection || currentSectionIndex == -1) {
                  // dayofweek should be preferred
                  day += diff;

                  if (day <= 0) {
                     day += 7;
                  } else if (day > date.daysInMonth()) {
                     day -= 7;
                  }

                  QDT_DEBUG << year << month << day << dayofweek
                            << diff << QDate(year, month, day).dayOfWeek();
               }
            }

            bool needfixday = false;

            if (sectionType(currentSectionIndex) & (DaySection | DayOfWeekSection)) {
               cachedDay = day;

            } else if (cachedDay > day) {
               day = cachedDay;
               needfixday = true;
            }

            if (!QDate::isValid(year, month, day)) {
               if (day < 32) {
                  cachedDay = day;
               }
               if (day > 28 && QDate::isValid(year, month, 1)) {
                  needfixday = true;
               }
            }
            if (needfixday) {
               if (context == FromString) {
                  state = Invalid;
                  goto end;
               }
               if (state == Acceptable && fixday) {
                  day = qMin(day, QDate(year, month, 1).daysInMonth());

                  const QLocale loc = locale();
                  for (int i = 0; i < sectionNodesCount; ++i) {
                     if (sectionType(i) & (DaySection | DayOfWeekSection)) {
                        input.replace(sectionPos(i), sectionSize(i), loc.toString(day));
                     }
                  }
               } else {
                  state = qMin(Intermediate, state);
               }
            }
         }

         if (parserType != QVariant::Date) {
            if (isSet & Hour12Section) {
               const bool hasHour = isSet & Hour24Section;
               if (ampm == -1) {
                  if (hasHour) {
                     ampm = (hour < 12 ? 0 : 1);
                  } else {
                     ampm = 0; // no way to tell if this is am or pm so I assume am
                  }
               }
               hour12 = (ampm == 0 ? hour12 % 12 : (hour12 % 12) + 12);
               if (!hasHour) {
                  hour = hour12;
               } else if (hour != hour12) {
                  conflicts = true;
               }
            } else if (ampm != -1) {
               if (!(isSet & (Hour24Section))) {
                  hour = (12 * ampm); // special case. Only ap section
               } else if ((ampm == 0) != (hour < 12)) {
                  conflicts = true;
               }
            }

         }

         newCurrentValue = QDateTime(QDate(year, month, day), QTime(hour, minute, second, msec), spec);
         QDT_DEBUG << year << month << day << hour << minute << second << msec;
      }

      QDT_DEBUGN("'%s' => '%s'(%s)", input.toLatin1().constData(),
                 newCurrentValue.toString("yyyy/MM/dd hh:mm:ss.zzz").toLatin1().constData(),
                 stateName(state).toLatin1().constData());
   }

end:
   if (newCurrentValue.isValid()) {
      if (context != FromString && state != Invalid && newCurrentValue < minimum) {
         const QLatin1Char space(' ');

         if (newCurrentValue >= minimum)
            qWarning("QDateTimeParser::parse Internal error 3 (%s %s)",
                     qPrintable(newCurrentValue.toString()), qPrintable(minimum.toString()));

         bool done = false;
         state     = Invalid;

         for (int i = 0; i < sectionNodesCount && !done; ++i) {
            const SectionNode &sn = sectionNodes.at(i);
            QString t = sectionText(input, i, sn.pos).toLower();

            if ((t.size() < sectionMaxSize(i) && (((int)fieldInfo(i) & (FixedWidth | Numeric)) != Numeric))
                  || t.contains(space)) {

               switch (sn.type) {
                  case AmPmSection:
                     switch (findAmPm(t, i)) {
                        case AM:
                        case PM:
                           state = Acceptable;
                           done = true;
                           break;
                        case Neither:
                           state = Invalid;
                           done = true;
                           break;
                        case PossibleAM:
                        case PossiblePM:
                        case PossibleBoth: {
                           const QDateTime copy(newCurrentValue.addSecs(12 * 60 * 60));
                           if (copy >= minimum && copy <= maximum) {
                              state = Intermediate;
                              done = true;
                           }
                           break;
                        }
                     }
                  case MonthSection:
                     if (sn.count >= 3) {
                        int tmp = newCurrentValue.date().month();
                        // I know the first possible month makes the date too early
                        while ((tmp = findMonth(t, tmp + 1, i)) != -1) {
                           const QDateTime copy(newCurrentValue.addMonths(tmp - newCurrentValue.date().month()));
                           if (copy >= minimum && copy <= maximum) {
                              break;   // break out of while
                           }
                        }
                        if (tmp == -1) {
                           break;
                        }
                        state = Intermediate;
                        done = true;
                        break;
                     }
                  // fallthrough
                  default: {
                     int toMin;
                     int toMax;

                     if (sn.type & TimeSectionMask) {
                        if (newCurrentValue.daysTo(minimum) != 0) {
                           break;
                        }
                        toMin = newCurrentValue.time().msecsTo(minimum.time());
                        if (newCurrentValue.daysTo(maximum) > 0) {
                           toMax = -1; // can't get to max
                        } else {
                           toMax = newCurrentValue.time().msecsTo(maximum.time());
                        }
                     } else {
                        toMin = newCurrentValue.daysTo(minimum);
                        toMax = newCurrentValue.daysTo(maximum);
                     }
                     const int maxChange = QDateTimeParser::maxChange(i);
                     if (toMin > maxChange) {
                        QDT_DEBUG << "invalid because toMin > maxChange" << toMin
                                  << maxChange << t << newCurrentValue << minimum;
                        state = Invalid;
                        done = true;
                        break;
                     } else if (toMax > maxChange) {
                        toMax = -1; // can't get to max
                     }

                     const int min = getDigit(minimum, i);
                     if (min == -1) {
                        qWarning("QDateTimeParser::parse Internal error 4 (%s)",
                                 qPrintable(sectionName(sn.type)));
                        state = Invalid;
                        done = true;
                        break;
                     }

                     int max = toMax != -1 ? getDigit(maximum, i) : absoluteMax(i, newCurrentValue);
                     int pos = cursorPosition - sn.pos;
                     if (pos < 0 || pos >= t.size()) {
                        pos = -1;
                     }
                     if (!potentialValue(t.simplified(), min, max, i, newCurrentValue, pos)) {
                        QDT_DEBUG << "invalid because potentialValue(" << t.simplified() << min << max
                                  << sectionName(sn.type) << "returned" << toMax << toMin << pos;
                        state = Invalid;
                        done = true;
                        break;
                     }
                     state = Intermediate;
                     done = true;
                     break;
                  }
               }
            }
         }

      } else {
         if (context == FromString) {
            // optimization
            Q_ASSERT(getMaximum().date().toJulianDay() == 4642999);
            if (newCurrentValue.date().toJulianDay() > 4642999) {
               state = Invalid;
            }
         } else {
            if (newCurrentValue > getMaximum()) {
               state = Invalid;
            }
         }

         QDT_DEBUG << "not checking intermediate because newCurrentValue is" << newCurrentValue << getMinimum() << getMaximum();
      }
   }
   StateNode node;
   node.input = input;
   node.state = state;
   node.conflicts = conflicts;
   node.value = newCurrentValue.toTimeSpec(spec);
   text = input;
   return node;
}
#endif // QT_NO_DATESTRING

#ifndef QT_NO_TEXTDATE
/*!
  \internal finds the first possible monthname that \a str1 can
  match. Starting from \a index; str should already by lowered
*/

int QDateTimeParser::findMonth(const QString &str1, int startMonth, int sectionIndex,
                               QString *usedMonth, int *used) const
{
   int bestMatch = -1;
   int bestCount = 0;
   if (!str1.isEmpty()) {
      const SectionNode &sn = sectionNode(sectionIndex);
      if (sn.type != MonthSection) {
         qWarning("QDateTimeParser::findMonth Internal error");
         return -1;
      }

      QLocale::FormatType type = sn.count == 3 ? QLocale::ShortFormat : QLocale::LongFormat;
      QLocale l = locale();

      for (int month = startMonth; month <= 12; ++month) {
         QString str2 = l.monthName(month, type).toLower();

         if (str1.startsWith(str2)) {
            if (used) {
               QDT_DEBUG << "used is set to" << str2.size();
               *used = str2.size();
            }
            if (usedMonth) {
               *usedMonth = l.monthName(month, type);
            }

            return month;
         }
         if (context == FromString) {
            continue;
         }

         const int limit = qMin(str1.size(), str2.size());

         QDT_DEBUG << "limit is" << limit << str1 << str2;
         bool equal = true;
         for (int i = 0; i < limit; ++i) {
            if (str1.at(i) != str2.at(i)) {
               equal = false;
               if (i > bestCount) {
                  bestCount = i;
                  bestMatch = month;
               }
               break;
            }
         }
         if (equal) {
            if (used) {
               *used = limit;
            }
            if (usedMonth) {
               *usedMonth = l.monthName(month, type);
            }
            return month;
         }
      }
      if (usedMonth && bestMatch != -1) {
         *usedMonth = l.monthName(bestMatch, type);
      }
   }

   if (used) {
      QDT_DEBUG << "used is set to" << bestCount;
      *used = bestCount;
   }

   return bestMatch;
}

int QDateTimeParser::findDay(const QString &str1, int startDay, int sectionIndex, QString *usedDay, int *used) const
{
   int bestMatch = -1;
   int bestCount = 0;

   if (!str1.isEmpty()) {
      const SectionNode &sn = sectionNode(sectionIndex);
      if (!(sn.type & (DaySection | DayOfWeekSection))) {
         qWarning("QDateTimeParser::findDay Internal error");
         return -1;
      }
      const QLocale l = locale();
      for (int day = startDay; day <= 7; ++day) {
         const QString str2 = l.dayName(day, sn.count == 4 ? QLocale::LongFormat : QLocale::ShortFormat);

         if (str1.startsWith(str2.toLower())) {
            if (used) {
               *used = str2.size();
            }
            if (usedDay) {
               *usedDay = str2;
            }
            return day;
         }
         if (context == FromString) {
            continue;
         }

         const int limit = qMin(str1.size(), str2.size());
         bool found = true;
         for (int i = 0; i < limit; ++i) {
            if (str1.at(i) != str2.at(i) && !str1.at(i).isSpace()) {
               if (i > bestCount) {
                  bestCount = i;
                  bestMatch = day;
               }
               found = false;
               break;
            }

         }
         if (found) {
            if (used) {
               *used = limit;
            }
            if (usedDay) {
               *usedDay = str2;
            }

            return day;
         }
      }
      if (usedDay && bestMatch != -1) {
         *usedDay = l.dayName(bestMatch, sn.count == 4 ? QLocale::LongFormat : QLocale::ShortFormat);
      }
   }
   if (used) {
      *used = bestCount;
   }

   return bestMatch;
}
#endif // QT_NO_TEXTDATE

/*!
  \internal

  returns
  0 if str == QDateTimeEdit::tr("AM")
  1 if str == QDateTimeEdit::tr("PM")
  2 if str can become QDateTimeEdit::tr("AM")
  3 if str can become QDateTimeEdit::tr("PM")
  4 if str can become QDateTimeEdit::tr("PM") and can become QDateTimeEdit::tr("AM")
  -1 can't become anything sensible

*/

int QDateTimeParser::findAmPm(QString &str, int index, int *used) const
{
   const SectionNode &s = sectionNode(index);
   if (s.type != AmPmSection) {
      qWarning("QDateTimeParser::findAmPm Internal error");
      return -1;
   }
   if (used) {
      *used = str.size();
   }
   if (str.trimmed().isEmpty()) {
      return PossibleBoth;
   }
   const QLatin1Char space(' ');
   int size = sectionMaxSize(index);

   enum {
      amindex = 0,
      pmindex = 1
   };
   QString ampm[2];
   ampm[amindex] = getAmPmText(AmText, s.count == 1 ? UpperCase : LowerCase);
   ampm[pmindex] = getAmPmText(PmText, s.count == 1 ? UpperCase : LowerCase);
   for (int i = 0; i < 2; ++i) {
      ampm[i].truncate(size);
   }

   QDT_DEBUG << "findAmPm" << str << ampm[0] << ampm[1];

   if (str.indexOf(ampm[amindex], 0, Qt::CaseInsensitive) == 0) {
      str = ampm[amindex];
      return AM;
   } else if (str.indexOf(ampm[pmindex], 0, Qt::CaseInsensitive) == 0) {
      str = ampm[pmindex];
      return PM;
   } else if (context == FromString || (str.count(space) == 0 && str.size() >= size)) {
      return Neither;
   }
   size = qMin(size, str.size());

   bool broken[2] = {false, false};
   for (int i = 0; i < size; ++i) {
      if (str.at(i) != space) {
         for (int j = 0; j < 2; ++j) {

            if (! broken[j]) {
               int index = ampm[j].indexOf(str.at(i));
               QDT_DEBUG << "looking for" << str.at(i)
                         << "in" << ampm[j] << "and got" << index;

               if (index == -1) {
                  if (str.at(i).category() == QChar::Letter_Uppercase) {
                     index = ampm[j].indexOf(str.at(i).toLower());
                     QDT_DEBUG << "trying with" << str.at(i).toLower()
                               << "in" << ampm[j] << "and got" << index;

                  } else if (str.at(i).category() == QChar::Letter_Lowercase) {
                     index = ampm[j].indexOf(str.at(i).toUpper());
                     QDT_DEBUG << "trying with" << str.at(i).toUpper()
                               << "in" << ampm[j] << "and got" << index;
                  }

                  if (index == -1) {
                     broken[j] = true;
                     if (broken[amindex] && broken[pmindex]) {
                        QDT_DEBUG << str << "didn't make it";
                        return Neither;
                     }

                     continue;
                  } else {
                     str[i] = ampm[j].at(index); // fix case
                  }
               }

               ampm[j].remove(index, 1);
            }
         }
      }
   }

   if (!broken[pmindex] && !broken[amindex]) {
      return PossibleBoth;
   }

   return (!broken[amindex] ? PossibleAM : PossiblePM);
}

/*!
  \internal
  Max number of units that can be changed by this section.
*/

int QDateTimeParser::maxChange(int index) const
{
   const SectionNode &sn = sectionNode(index);
   switch (sn.type) {
      // Time. unit is msec
      case MSecSection:
         return 999;
      case SecondSection:
         return 59 * 1000;
      case MinuteSection:
         return 59 * 60 * 1000;
      case Hour24Section:
      case Hour12Section:
         return 59 * 60 * 60 * 1000;

      // Date. unit is day
      case DayOfWeekSection:
         return 7;
      case DaySection:
         return 30;
      case MonthSection:
         return 365 - 31;
      case YearSection:
         return 9999 * 365;
      case YearSection2Digits:
         return 100 * 365;
      default:
         qWarning("QDateTimeParser::maxChange() Internal error (%s)",
                  qPrintable(sectionName(sectionType(index))));
   }

   return -1;
}

QDateTimeParser::FieldInfo QDateTimeParser::fieldInfo(int index) const
{
   FieldInfo ret = 0;
   const SectionNode &sn = sectionNode(index);
   const Section s = sn.type;
   switch (s) {
      case MSecSection:
         ret |= Fraction;
      // fallthrough
      case SecondSection:
      case MinuteSection:
      case Hour24Section:
      case Hour12Section:
      case YearSection:
      case YearSection2Digits:
         ret |= Numeric;
         if (s != YearSection) {
            ret |= AllowPartial;
         }
         if (sn.count != 1) {
            ret |= FixedWidth;
         }
         break;
      case MonthSection:
      case DaySection:
         switch (sn.count) {
            case 2:
               ret |= FixedWidth;
            // fallthrough
            case 1:
               ret |= (Numeric | AllowPartial);
               break;
         }
         break;
      case DayOfWeekSection:
         if (sn.count == 3) {
            ret |= FixedWidth;
         }
         break;
      case AmPmSection:
         ret |= FixedWidth;
         break;
      default:
         qWarning("QDateTimeParser::fieldInfo Internal error 2 (%d %s %d)",
                  index, qPrintable(sectionName(sn.type)), sn.count);
         break;
   }
   return ret;
}

/*!
  \internal Get a number that str can become which is between min
  and max or -1 if this is not possible.
*/


QString QDateTimeParser::sectionFormat(int index) const
{
   const SectionNode &sn = sectionNode(index);
   return sectionFormat(sn.type, sn.count);
}

QString QDateTimeParser::sectionFormat(Section s, int count) const
{
   QChar fillChar;
   switch (s) {
      case AmPmSection:
         return count == 1 ? QLatin1String("AP") : QLatin1String("ap");
      case MSecSection:
         fillChar = QLatin1Char('z');
         break;
      case SecondSection:
         fillChar = QLatin1Char('s');
         break;
      case MinuteSection:
         fillChar = QLatin1Char('m');
         break;
      case Hour24Section:
         fillChar = QLatin1Char('H');
         break;
      case Hour12Section:
         fillChar = QLatin1Char('h');
         break;
      case DayOfWeekSection:
      case DaySection:
         fillChar = QLatin1Char('d');
         break;
      case MonthSection:
         fillChar = QLatin1Char('M');
         break;
      case YearSection2Digits:
      case YearSection:
         fillChar = QLatin1Char('y');
         break;
      default:
         qWarning("QDateTimeParser::sectionFormat Internal error (%s)",
                  qPrintable(sectionName(s)));
         return QString();
   }
   if (fillChar.isNull()) {
      qWarning("QDateTimeParser::sectionFormat Internal error 2");
      return QString();
   }

   QString str;
   str.fill(fillChar, count);
   return str;
}


/*! \internal Returns true if str can be modified to represent a
  number that is within min and max.
*/

bool QDateTimeParser::potentialValue(const QString &str, int min, int max, int index,
                                     const QDateTime &currentValue, int insert) const
{
   if (str.isEmpty()) {
      return true;
   }
   const int size = sectionMaxSize(index);
   int val = (int)locale().toUInt(str);
   const SectionNode &sn = sectionNode(index);
   if (sn.type == YearSection2Digits) {
      val += currentValue.date().year() - (currentValue.date().year() % 100);
   }
   if (val >= min && val <= max && str.size() == size) {
      return true;
   } else if (val > max) {
      return false;
   } else if (str.size() == size && val < min) {
      return false;
   }

   const int len = size - str.size();
   for (int i = 0; i < len; ++i) {
      for (int j = 0; j < 10; ++j) {
         if (potentialValue(str + QLatin1Char('0' + j), min, max, index, currentValue, insert)) {
            return true;
         } else if (insert >= 0) {
            QString tmp = str;
            tmp.insert(insert, QLatin1Char('0' + j));
            if (potentialValue(tmp, min, max, index, currentValue, insert)) {
               return true;
            }
         }
      }
   }

   return false;
}

bool QDateTimeParser::skipToNextSection(int index, const QDateTime &current, const QString &text) const
{
   Q_ASSERT(current >= getMinimum() && current <= getMaximum());

   const SectionNode &node = sectionNode(index);
   Q_ASSERT(text.size() < sectionMaxSize(index));

   const QDateTime maximum = getMaximum();
   const QDateTime minimum = getMinimum();
   QDateTime tmp = current;
   int min = absoluteMin(index);
   setDigit(tmp, index, min);
   if (tmp < minimum) {
      min = getDigit(minimum, index);
   }

   int max = absoluteMax(index, current);
   setDigit(tmp, index, max);
   if (tmp > maximum) {
      max = getDigit(maximum, index);
   }
   int pos = cursorPosition() - node.pos;
   if (pos < 0 || pos >= text.size()) {
      pos = -1;
   }

   const bool potential = potentialValue(text, min, max, index, current, pos);
   return !potential;

   /* If the value potentially can become another valid entry we
    * don't want to skip to the next. E.g. In a M field (month
    * without leading 0 if you type 1 we don't want to autoskip but
    * if you type 3 we do
   */
}

/*!
  \internal
  For debugging. Returns the name of the section \a s.
*/

QString QDateTimeParser::sectionName(int s) const
{
   switch (s) {
      case QDateTimeParser::AmPmSection:
         return QLatin1String("AmPmSection");
      case QDateTimeParser::DaySection:
         return QLatin1String("DaySection");
      case QDateTimeParser::DayOfWeekSection:
         return QLatin1String("DayOfWeekSection");
      case QDateTimeParser::Hour24Section:
         return QLatin1String("Hour24Section");
      case QDateTimeParser::Hour12Section:
         return QLatin1String("Hour12Section");
      case QDateTimeParser::MSecSection:
         return QLatin1String("MSecSection");
      case QDateTimeParser::MinuteSection:
         return QLatin1String("MinuteSection");
      case QDateTimeParser::MonthSection:
         return QLatin1String("MonthSection");
      case QDateTimeParser::SecondSection:
         return QLatin1String("SecondSection");
      case QDateTimeParser::YearSection:
         return QLatin1String("YearSection");
      case QDateTimeParser::YearSection2Digits:
         return QLatin1String("YearSection2Digits");
      case QDateTimeParser::NoSection:
         return QLatin1String("NoSection");
      case QDateTimeParser::FirstSection:
         return QLatin1String("FirstSection");
      case QDateTimeParser::LastSection:
         return QLatin1String("LastSection");
      default:
         return QLatin1String("Unknown section ") + QString::number(s);
   }
}

/*!
  \internal
  For debugging. Returns the name of the state \a s.
*/

QString QDateTimeParser::stateName(int s) const
{
   switch (s) {
      case Invalid:
         return QLatin1String("Invalid");
      case Intermediate:
         return QLatin1String("Intermediate");
      case Acceptable:
         return QLatin1String("Acceptable");
      default:
         return QLatin1String("Unknown state ") + QString::number(s);
   }
}

#ifndef QT_NO_DATESTRING
bool QDateTimeParser::fromString(const QString &t, QDate *date, QTime *time) const
{
   QDateTime val(QDate(1900, 1, 1), QDATETIMEEDIT_TIME_MIN);
   QString text = t;
   int copy = -1;
   const StateNode tmp = parse(text, copy, val, false);
   if (tmp.state != Acceptable || tmp.conflicts) {
      return false;
   }
   if (time) {
      const QTime t = tmp.value.time();
      if (!t.isValid()) {
         return false;
      }
      *time = t;
   }

   if (date) {
      const QDate d = tmp.value.date();
      if (!d.isValid()) {
         return false;
      }
      *date = d;
   }
   return true;
}
#endif // QT_NO_DATESTRING

QDateTime QDateTimeParser::getMinimum() const
{
   return QDateTime(QDATETIMEEDIT_DATE_MIN, QDATETIMEEDIT_TIME_MIN, spec);
}

QDateTime QDateTimeParser::getMaximum() const
{
   return QDateTime(QDATETIMEEDIT_DATE_MAX, QDATETIMEEDIT_TIME_MAX, spec);
}

QString QDateTimeParser::getAmPmText(AmPm ap, Case cs) const
{
   if (ap == AmText) {
      return (cs == UpperCase ? QLatin1String("AM") : QLatin1String("am"));
   } else {
      return (cs == UpperCase ? QLatin1String("PM") : QLatin1String("pm"));
   }
}

/*
  \internal

  I give arg2 preference because arg1 is always a QDateTime.
*/

bool operator==(const QDateTimeParser::SectionNode &s1, const QDateTimeParser::SectionNode &s2)
{
   return (s1.type == s2.type) && (s1.pos == s2.pos) && (s1.count == s2.count);
}


