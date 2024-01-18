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

#include <qstringlist.h>
#include <qtimezone.h>

#include "qbuiltintypes_p.h"
#include "qitem_p.h"
#include "qpatternistlocale_p.h"
#include "qvalidationerror_p.h"

#include "qabstractdatetime_p.h"

using namespace QPatternist;

AbstractDateTime::AbstractDateTime(const QDateTime &dateTime) : m_dateTime(dateTime)
{
   Q_ASSERT(dateTime.isValid());
}

#define badData(msg)        errorMessage = ValidationError::createError(msg); return QDateTime()
#define getCapt(sym)        ((captTable.sym == -1) ? QString() : capts.at(captTable.sym))
#define getSafeCapt(sym)    ((captTable.sym == -1) ? QString() : capts.value(captTable.sym))

QDateTime AbstractDateTime::create(AtomicValue::Ptr &errorMessage, const QString &lexicalSource, const CaptureTable &captTable)
{
   QString pattern = captTable.regExp.pattern();
   QRegularExpression myExp(pattern, QPatternOption::ExactMatchOption);

   QRegularExpressionMatch match = myExp.match(lexicalSource);

   if (! match.hasMatch()) {
      badData(QString());
   }

   const QStringList capts = match.capturedTexts();
   const QString yearStr(getCapt(year));

   if (yearStr.size() > 4 && yearStr.at(0) == '0') {
      badData(QtXmlPatterns::tr("Year %1 is invalid because it begins with %2.")
              .formatArg(formatData(yearStr)).formatArg(formatData("0")));
   }

   /* If the strings are empty, load default values which are
    * guranteed to pass the validness tests. */
   const QString monthStr(getCapt(month));
   const QString dayStr(getCapt(day));

   YearProperty year = yearStr.isEmpty() ? DefaultYear : yearStr.toInteger<int>();

   if (getCapt(yearSign) == "-") {
      year = -year;
   }

   const MonthProperty month = monthStr.isEmpty() ? DefaultMonth : monthStr.toInteger<int>();
   const MonthProperty day = dayStr.isEmpty() ? DefaultDay : dayStr.toInteger<int>();

   if (!QDate::isValid(year, month, day)) {
      /* Try to give an intelligent message. */
      if (day > 31 || day < 1) {
         badData(QtXmlPatterns::tr("Day %1 is outside the range %2..%3.")
                 .formatArg(formatData(QString::number(day)))
                 .formatArg(formatData("01"))
                 .formatArg(formatData("31")));

      } else if (month > 12 || month < -12 || month == 0) {
         badData(QtXmlPatterns::tr("Month %1 is outside the range %2..%3.")
                 .formatArg(month)
                 .formatArg(formatData("01"))
                 .formatArg(formatData("12")));

      } else if (QDate::isValid(DefaultYear, month, day)) {
         /* We can't use the badData() macro here because we need a different
          * error code: FODT0001 instead of FORG0001. */
         errorMessage = ValidationError::createError(QtXmlPatterns::tr("Overflow: Can not represent date %1.")
                  .formatArg(formatData(QLatin1String("%1-%2-%3"))
                  .formatArg(year).formatArg(month).formatArg(day)), ReportContext::FODT0001);
         return QDateTime();

      } else {
         badData(QtXmlPatterns::tr("Day %1 is invalid for month %2.")
                 .formatArg(formatData(QString::number(day)))
                 .formatArg(formatData(QString::number(month))));
      }
   }

   /* Parse the zone offset. */
   ZoneOffsetParseResult zoResult;
   const ZOTotal offset = parseZoneOffset(zoResult, capts, captTable);

   if (zoResult == Error) {
      errorMessage = ValidationError::createError();
      /* We encountered an error, so stop processing. */
      return QDateTime();
   }

   QDate date(year, month, day);

   /* Only deal with time if time is needed. */
   if (captTable.hour == -1) {
      QDateTime result(date);
      setUtcOffset(result, zoResult, offset);
      return result;
   } else {
      /* Now, it's time for the time-part.
       *
       * If the strings are empty, toInt() will return 0, which
       * in all cases is valid properties. */
      const QString hourStr(getCapt(hour));
      const QString minutesStr(getCapt(minutes));
      const QString secondsStr(getCapt(seconds));

      HourProperty hour = hourStr.toInteger<int>();

      const MinuteProperty mins = minutesStr.toInteger<int>();
      const SecondProperty secs = secondsStr.toInteger<int>();

      QString msecondsStr(getSafeCapt(mseconds));

      if (!msecondsStr.isEmpty()) {
         msecondsStr = msecondsStr.leftJustified(3, QLatin1Char('0'), true);
      }

      const MSecondProperty msecs = msecondsStr.toInteger<int>();

      if (hour == 24) {
         /* 24:00:00.00 is an invalid time for QTime, so handle it here. */
         if (mins != 0 || secs != 0 || msecs != 0) {
            badData(QtXmlPatterns::tr("Time 24:%1:%2.%3 is invalid. "
                                      "Hour is 24, but minutes, seconds, "
                                      "and milliseconds are not all 0; ")
                    .formatArg(mins).formatArg(secs).formatArg(msecs));
         } else {
            hour = 0;
            date = date.addDays(1);
         }
      } else if (!QTime::isValid(hour, mins, secs, msecs)) {
         badData(QtXmlPatterns::tr("Time %1:%2:%3.%4 is invalid.")
                 .formatArg(hour).formatArg(mins).formatArg(secs).formatArg(msecs));
      }

      const QTime time(hour, mins, secs, msecs);
      Q_ASSERT(time.isValid());

      QDateTime result(date, time);
      setUtcOffset(result, zoResult, offset);
      return result;
   }
}

ZOTotal AbstractDateTime::parseZoneOffset(ZoneOffsetParseResult &result,
      const QStringList &capts,
      const CaptureTable &captTable)
{
   const QString zoneOffsetSignStr(getCapt(zoneOffsetSign));

   if (zoneOffsetSignStr.isEmpty()) {
      const QString zoneOffsetUTCStr(getCapt(zoneOffsetUTCSymbol));
      Q_ASSERT(zoneOffsetUTCStr.isEmpty() || zoneOffsetUTCStr == QLatin1String("Z"));

      if (zoneOffsetUTCStr.isEmpty()) {
         result = LocalTime;
      } else {
         result = UTC;
      }

      return 0;
   }

   Q_ASSERT(zoneOffsetSignStr == QLatin1String("-") || zoneOffsetSignStr == QLatin1String("+"));

   const QString zoneOffsetHourStr(getCapt(zoneOffsetHour));
   Q_ASSERT(!zoneOffsetHourStr.isEmpty());

   const ZOHourProperty zoHour = zoneOffsetHourStr.toInteger<int>();

   if (zoHour > 14 || zoHour < -14) {
      result = Error;
      return 0;
      /*
      badZOData(QtXmlPatterns::tr("%1 it is not a valid hour property in a zone offset. "
                     "It must be less than or equal to 14.").formatArg(zoHour));
                     */
   }

   const QString zoneOffsetMinuteStr(getCapt(zoneOffsetMinute));
   Q_ASSERT(!zoneOffsetMinuteStr.isEmpty());

   const ZOHourProperty zoMins = zoneOffsetMinuteStr.toInteger<int>();

   if (zoHour == 14 && zoMins != 0) {
      /*
      badZOData(QtXmlPatterns::tr("When the hour property in a zone offset is 14, the minute property "
                     "must be 0, not %1.").formatArg(zoMins));
                     */
      result = Error;
      return 0;
   } else if (zoMins > 59 || zoMins < -59) {
      /*
      badZOData(QtXmlPatterns::tr("The minute property in a zone offset cannot be larger than 59. "
                     "%1 is therefore invalid.").formatArg(zoMins));
                     */
      result = Error;
      return 0;
   }

   if (zoHour == 0 && zoMins == 0) { /* "-00:00" and "+00:00" is equal to 'Z'. */
      result = UTC;
      return 0;
   } else {
      ZOTotal zoneOffset = (zoHour * 60 + zoMins) * 60;

      if (zoneOffsetSignStr == "-") {
         zoneOffset = -zoneOffset;
      }

      result = Offset;
      return zoneOffset;
   }
}
//#undef badZOData

void AbstractDateTime::setUtcOffset(QDateTime &result,
                                    const ZoneOffsetParseResult zoResult,
                                    const int zoOffset)
{
   if (zoResult == UTC) {
      result.setTimeZone(QTimeZone::utc());

   } else if (zoResult == LocalTime) {
      result.setTimeZone(QTimeZone::systemTimeZone());

   } else {
      Q_ASSERT(zoResult == Offset);
      result.setTimeZone(QTimeZone(zoOffset));
   }
}

#undef badData
#undef getCapt
#undef getSafeCapt

bool AbstractDateTime::isRangeValid(const QDate &date,
                                    QString &message)
{
   if (date.isValid()) {
      return true;
   } else {
      message = QtXmlPatterns::tr("Overflow: Date can not be represented.");
      return false;
   }
}

QString AbstractDateTime::dateToString() const
{
   return m_dateTime.toString(QLatin1String("yyyy-MM-dd"));
}

QString AbstractDateTime::serializeMSeconds(const MSecondProperty mseconds)
{
   QString retval;
   retval.append(QLatin1Char('.'));
   int div = 100;
   MSecondProperty msecs = mseconds;

   while (msecs > 0) {
      int d = msecs / div;
      retval.append(QLatin1Char(d + '0'));
      msecs = msecs % div;
      div = div / 10;
   }

   return retval;
}

QString AbstractDateTime::timeToString() const
{
   QString base(m_dateTime.toString(QLatin1String("hh:mm:ss")));
   const MSecondProperty msecs = m_dateTime.time().msec();

   if (msecs) {
      base.append(serializeMSeconds(msecs));
   }

   return base;
}

QString AbstractDateTime::zoneOffsetToString() const
{
   if (m_dateTime.timeZone() == QTimeZone::systemTimeZone()) {
      return QString();

   } else if (m_dateTime.timeZone() == QTimeZone::utc()) {
      return QString("Z");

   } else {
      const int zoneOffset = m_dateTime.offsetFromUtc();
      Q_ASSERT(zoneOffset != 0);

      const int posZoneOffset = qAbs(zoneOffset);

      /* zoneOffset is in seconds. */
      const int hours   = posZoneOffset / (60 * 60);
      const int minutes = (posZoneOffset % (60 * 60)) / 60;

      QString result;
      result.append(zoneOffset < 0 ? QLatin1Char('-') : '+');
      result.append(QString::number(hours).rightJustified(2, '0'));
      result.append(QLatin1Char(':'));
      result.append(QString::number(minutes).rightJustified(2, '0'));

      return result;
   }
}

void AbstractDateTime::copyTimeSpec(const QDateTime &from, QDateTime &to)
{
   to.setTimeZone(from.timeZone());
}

Item AbstractDateTime::fromValue(const QDateTime &) const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return Item();
}

