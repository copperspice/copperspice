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

#include "qbuiltintypes_p.h"
#include "qpatternistlocale_p.h"
#include "qvalidationerror_p.h"

#include "qabstractduration_p.h"

using namespace QPatternist;

AbstractDuration::AbstractDuration(const bool isPos) : m_isPositive(isPos)
{
}

#define error(msg)    return ValidationError::createError(msg);
#define getCapt(sym)  ((captTable.sym == -1) ? QString() : capts.at(captTable.sym))

AtomicValue::Ptr AbstractDuration::create(const CaptureTable &captTable,
      const QString &lexical,
      bool *isPositive,
      YearProperty *years,
      MonthProperty *months,
      DayCountProperty *days,
      HourProperty *hours,
      MinuteProperty *minutes,
      SecondProperty *seconds,
      MSecondProperty *mseconds)
{
   /* We don't directly write into the arguments(eg @p years) but uses these
    * because the arguments are intended for normalized values, and therefore
    * can cause overflows. */
   MonthCountProperty monthCount = 0;
   MinuteCountProperty minCount = 0;
   HourCountProperty hourCount = 0;
   SecondCountProperty secCount = 0;

   Q_ASSERT(isPositive);

   QString pattern = captTable.regExp.pattern();
   QRegularExpression myExp(pattern, QPatternOption::ExactMatchOption);

   QRegularExpressionMatch match = myExp.match(lexical);

   if (! match.hasMatch()) {
      error(QString());
   }

   const QStringList capts = match.capturedTexts();

   if (days) {
      if (getCapt(tDelimiter).isEmpty()) {

         if ((years && getCapt(year).isEmpty() && getCapt(month).isEmpty() && getCapt(day).isEmpty()) ||
                   (! years && getCapt(day).isEmpty())) {
            error(QtXmlPatterns::tr("At least one component must be present."));
         }

      } else if (getCapt(hour).isEmpty() && getCapt(minutes).isEmpty() && getCapt(seconds).isEmpty() && getCapt(mseconds).isEmpty()) {
         error(QtXmlPatterns::tr("At least one time component must appear after the %1-delimiter.").formatArg(formatKeyword("T")));
      }

   } else if (getCapt(year).isEmpty() && getCapt(month).isEmpty()) { /* This checks xs:yearMonthDuration. */
      error(QtXmlPatterns::tr("At least one component must be present."));
   }

   /* If we got no '-', we are positive. */
   *isPositive = capts.at(1).isEmpty();

   if (days) {
      Q_ASSERT(hours);
      Q_ASSERT(minutes);
      Q_ASSERT(seconds);
      Q_ASSERT(mseconds);

      *days     = getCapt(day).toInteger<int>();
      hourCount = getCapt(hour).toInteger<int>();
      minCount  = getCapt(minutes).toInteger<int>();
      secCount  = getCapt(seconds).toInteger<int>();

      const QString msecondsStr(getCapt(mseconds));
      if (!msecondsStr.isEmpty()) {
         *mseconds = msecondsStr.leftJustified(3, '0').toInteger<int>();
      } else {
         *mseconds = msecondsStr.toInteger<int>();
      }

      if (secCount > 59) {
         minCount += secCount / 60;
         *seconds = secCount % 60;
      } else {
         *seconds = secCount;
      }

      if (minCount > 59) {
         hourCount += minCount / 60;
         *minutes = minCount % 60;
      } else {
         *minutes = minCount;
      }

      if (hourCount > 23) {
         *days += hourCount / 24;
         *hours = hourCount % 24;
      } else {
         *hours = hourCount;
      }
   }

   if (!years) {
      return AtomicValue::Ptr();
   }

   /* We're supposed to handle years/months. */
   Q_ASSERT(months);

   *years = getCapt(year).toInteger<int>();
   monthCount = getCapt(month).toInteger<int>();

   if (monthCount > 11) {
      *years += monthCount / 12;
      *months = monthCount % 12;
   } else {
      *months = monthCount;
   }

   return AtomicValue::Ptr();
}

#undef error
#undef getCapt

bool AbstractDuration::operator==(const AbstractDuration &other) const
{
   if (years() == other.years()
         && months() == other.months()
         && days() == other.days()
         && hours() == other.hours()
         && minutes() == other.minutes()
         && seconds() == other.seconds()
         && mseconds() == other.mseconds()) {

      if (isPositive() == other.isPositive()) {
         return true;

      } else if (years() == 0
                 && months() == 0
                 && days() == 0
                 && hours() == 0
                 && minutes() == 0
                 && seconds () == 0
                 && mseconds() == 0) {
         return true; /* Signedness doesn't matter if all are zero. */
      }
   }

   return false;
}

QString AbstractDuration::serializeMSeconds(const MSecondProperty mseconds)
{
   QString retval;
   retval.append('.');

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

bool AbstractDuration::isPositive() const
{
   return m_isPositive;
}

