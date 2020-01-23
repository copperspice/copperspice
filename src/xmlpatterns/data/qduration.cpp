/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include "qabstractdatetime_p.h"
#include "qbuiltintypes_p.h"
#include "qitem_p.h"

#include "qduration_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

Duration::Duration(const bool isPositiveP,
                   const YearProperty yearsP,
                   const MonthProperty monthsP,
                   const DayCountProperty daysP,
                   const HourProperty hoursP,
                   const MinuteProperty mins,
                   const SecondProperty secs,
                   const MSecondProperty msecs) : AbstractDuration(isPositiveP),
   m_years(yearsP),
   m_months(monthsP),
   m_days(daysP),
   m_hours(hoursP),
   m_minutes(mins),
   m_seconds(secs),
   m_mseconds(msecs)
{
}

Duration::Ptr Duration::fromLexical(const QString &lexical)
{
   static const CaptureTable captureTable(
      /* The extra paranthesis is a build fix for GCC 3.3. */
      (QRegularExpression(QLatin1String(
                  "^\\s*"                         /* Any preceding whitespace. */
                  "(-)?"                          /* Any minus sign. */
                  "P"                             /* Delimiter. */
                  "(?:(\\d+)Y)?"                  /* Year part. */
                  "(?:(\\d+)M)?"                  /* Month part. */
                  "(?:(\\d+)D)?"                  /* Day part. */
                  "(?:"                           /* Here starts the optional time part. */
                  "(T)"                           /* SchemaTime delimiter. */
                  "(?:(\\d+)H)?"                  /* Hour part. */
                  "(?:(\\d+)M)?"                  /* Minute part. */
                  "(?:(\\d+)(?:\\.(\\d+))?S)?"    /* Seconds & milli seconds. */
                  ")?"                            /* End of optional time part. */
                  "\\s*$"                         /* Any terminating whitespace. */))),
      /*yearP*/       2,
      /*monthP*/      3,
      /*dayP*/        4,
      /*tDelimiterP*/ 5,
      /*hourP*/       6,
      /*minutesP*/    7,
      /*secondsP*/    8,
      /*msecondsP*/   9);

   YearProperty years = 0;
   MonthProperty months = 0;
   DayCountProperty days = 0;
   HourProperty hours = 0;
   MinuteProperty minutes = 0;
   SecondProperty sec = 0;
   MSecondProperty msec = 0;
   bool isPos;

   const AtomicValue::Ptr err(create(captureTable, lexical, &isPos, &years, &months,
                                     &days, &hours, &minutes, &sec, &msec));

   return err ? err : Duration::Ptr(new Duration(isPos, years, months, days, hours,
                                    minutes, sec, msec));
}

Duration::Ptr Duration::fromComponents(const bool isPositive,
                                       const YearProperty years,
                                       const MonthProperty months,
                                       const DayCountProperty days,
                                       const HourProperty hours,
                                       const MinuteProperty minutes,
                                       const SecondProperty seconds,
                                       const MSecondProperty mseconds)
{
   return Duration::Ptr(new Duration(isPositive, years, months, days,
                                     hours, minutes, seconds, mseconds));
}

AbstractDuration::Value Duration::value() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return 0;
}

Item Duration::fromValue(const Value) const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return Item();
}

QString Duration::stringValue() const
{
   QString retval;

   if (!m_isPositive) {
      retval.append(QLatin1Char('-'));
   }

   retval.append(QLatin1Char('P'));

   if (m_years) {
      retval.append(QString::number(m_years));
      retval.append(QLatin1Char('Y'));
   }

   if (m_months) {
      retval.append(QString::number(m_months));
      retval.append(QLatin1Char('M'));
   }

   if (m_days) {
      retval.append(QString::number(m_days));
      retval.append(QLatin1Char('D'));
   }

   if (!m_hours && !m_minutes && !m_seconds && !m_seconds) {
      if (!m_years && !m_months && !m_days) {
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
   } else if (!m_years && !m_months && !m_days && !m_hours && !m_minutes) {
      retval.append(QLatin1String("0S"));
   }

   return retval;
}

YearProperty Duration::years() const
{
   return m_years;
}

MonthProperty Duration::months() const
{
   return m_months;
}

DayCountProperty Duration::days() const
{
   return m_days;
}

HourProperty Duration::hours() const
{
   return m_hours;
}

MinuteProperty Duration::minutes() const
{
   return m_minutes;
}

SecondProperty Duration::seconds() const
{
   return m_seconds;
}

MSecondProperty Duration::mseconds() const
{
   return m_mseconds;
}

ItemType::Ptr Duration::type() const
{
   return BuiltinTypes::xsDuration;
}

QT_END_NAMESPACE
