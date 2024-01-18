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

#include "qdaytimeduration_p.h"

#include "qabstractdatetime_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonvalues_p.h"

using namespace QPatternist;

DayTimeDuration::DayTimeDuration(const bool isPositiveP, const DayCountProperty daysP, const HourProperty hoursP,
            const MinuteProperty minutesP, const SecondProperty secs, const MSecondProperty msecs)
   : AbstractDuration(isPositiveP), m_days(daysP), m_hours(hoursP), m_minutes(minutesP),
     m_seconds(secs), m_mseconds(msecs)
{
}

DayTimeDuration::Ptr DayTimeDuration::fromLexical(const QString &lexical)
{
   static const CaptureTable captureTable(

      QRegularExpression(
                  "^\\s*"                         /* Any preceding whitespace. */
                  "(-)?"                          /* Any minus sign. */
                  "P"                             /* Delimiter. */
                  "(?:(\\d+)D)?"                  /* Day part. */
                  "(?:"                           /* Here starts the optional time part. */
                  "(T)"                           /* SchemaTime delimiter. */
                  "(?:(\\d+)H)?"                  /* Hour part. */
                  "(?:(\\d+)M)?"                  /* Minute part. */
                  "(?:(\\d+)(?:\\.(\\d+))?S)?"    /* Seconds & milli seconds. */
                  ")?"                            /* End of optional time part. */
                  "\\s*$"                         /* Any terminating whitespace. */),

      -1,       /*yearP*/
      -1,       /*monthP*/
       2,       /*dayP*/
       3,       /*tDelimiterP*/
       4,       /*hourP*/
       5,       /*minutesP*/
       6,       /*secondsP*/
       7);      /*msecondsP*/

   DayCountProperty days  = 0;
   HourProperty hours     = 0;
   MinuteProperty minutes = 0;
   SecondProperty sec     = 0;
   MSecondProperty msec   = 0;

   bool isPos;

   const DayTimeDuration::Ptr err(create(captureTable, lexical, &isPos, nullptr, nullptr, &days,
            &hours, &minutes, &sec, &msec));

   return err ? err : DayTimeDuration::Ptr(new DayTimeDuration(isPos, days, hours, minutes, sec, msec));
}

DayTimeDuration::Ptr DayTimeDuration::fromComponents(const bool isPositive, const DayCountProperty days,
      const HourProperty hours, const MinuteProperty minutes, const SecondProperty seconds,
      const MSecondProperty mseconds)
{
   return DayTimeDuration::Ptr(new DayTimeDuration(isPositive, days, hours, minutes, seconds, mseconds));
}

DayTimeDuration::Ptr DayTimeDuration::fromSeconds(const SecondCountProperty sourceSecs, const MSecondProperty msecs)
{
   Q_ASSERT(msecs >= 0);

   const SecondCountProperty source = qAbs(sourceSecs);
   const bool isPos = sourceSecs >= 0;
   const SecondCountProperty secs = source % 60;
   const MinuteCountProperty mins = (source / 60) % 60;
   const HourCountProperty hours = source / (60 * 60) % 24;
   const DayCountProperty days = source / (60 * 60) / 24;

   return DayTimeDuration::Ptr(new DayTimeDuration(isPos, days, hours, mins, secs, msecs));
}

QString DayTimeDuration::stringValue() const
{
   QString retval;

   if (!m_isPositive) {
      retval.append(QLatin1Char('-'));
   }

   retval.append(QLatin1Char('P'));

   if (m_days) {
      retval.append(QString::number(m_days));
      retval.append(QLatin1Char('D'));
   }

   if (!m_hours && !m_minutes && !m_seconds && !m_seconds) {
      if (!m_days) {
         return QLatin1String("PT0S");
      } else {
         return retval;
      }
   }

   retval.append(QLatin1Char('T'));

   if (m_hours) {
      retval.append(QString::number(m_hours));
      retval.append(QLatin1Char('H'));
   }

   if (m_minutes) {
      retval.append(QString::number(m_minutes));
      retval.append(QLatin1Char('M'));
   }

   if (m_seconds || m_seconds) {
      retval.append(QString::number(m_seconds));

      if (m_mseconds) {
         retval.append(serializeMSeconds(m_mseconds));
      }

      retval.append(QLatin1Char('S'));
   } else if (!m_days && !m_hours && !m_minutes) {
      retval.append(QLatin1String("0S"));
   }

   return retval;
}

AbstractDuration::Value DayTimeDuration::value() const
{
   return ((m_days * 24 * 60 * 60 * 1000) +
           (m_hours * 60 * 60 * 1000) +
           (m_minutes * 60 * 1000) +
           (m_seconds * 1000) +
           m_mseconds) * (m_isPositive ? 1 : -1);
}

Item DayTimeDuration::fromValue(const Value val) const
{
   if (val == 0) {
      return toItem(CommonValues::DayTimeDurationZero);
   } else {
      return toItem(fromSeconds(val / 1000, qAbs(val) % 1000));
   }
}

ItemType::Ptr DayTimeDuration::type() const
{
   return BuiltinTypes::xsDayTimeDuration;
}

YearProperty DayTimeDuration::years() const
{
   return 0;
}

MonthProperty DayTimeDuration::months() const
{
   return 0;
}

DayCountProperty DayTimeDuration::days() const
{
   return m_days;
}

HourProperty DayTimeDuration::hours() const
{
   return m_hours;
}

MinuteProperty DayTimeDuration::minutes() const
{
   return m_minutes;
}

SecondProperty DayTimeDuration::seconds() const
{
   return m_seconds;
}

MSecondProperty DayTimeDuration::mseconds() const
{
   return m_mseconds;
}
