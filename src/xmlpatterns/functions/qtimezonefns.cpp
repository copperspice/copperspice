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

#include <qtimezone.h>

#include "qabstractdatetime_p.h"
#include "qcontextfns_p.h"
#include "qdate_p.h"
#include "qschemadatetime_p.h"
#include "qdaytimeduration_p.h"
#include "qpatternistlocale_p.h"
#include "qschematime_p.h"
#include "qtimezonefns_p.h"

using namespace QPatternist;

Item AdjustTimezone::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   enum {
      /**
       * The maximum zone offset, @c PT14H, in milli seconds.
       */
      MSecLimit = 14 * 60/*M*/ * 60/*S*/ * 1000/*ms*/
   };


   const Item arg(m_operands.first()->evaluateSingleton(context));
   if (!arg) {
      return Item();
   }

   QDateTime dt(arg.as<AbstractDateTime>()->toDateTime());
   // TODO DT dt.setDateOnly(false);
   Q_ASSERT(dt.isValid());
   DayTimeDuration::Ptr tz;

   if (m_operands.count() == 2) {
      tz = DayTimeDuration::Ptr(m_operands.at(1)->evaluateSingleton(context).as<DayTimeDuration>());
   } else {
      tz = context->implicitTimezone();
   }

   if (tz) {
      const MSecondCountProperty tzMSecs = tz->value();

      if (tzMSecs % (1000 * 60) != 0) {
         context->error(QtXmlPatterns::tr("A zone offset must be in the "
                                          "range %1..%2 inclusive. %3 is "
                                          "out of range.")
                        .formatArg(formatData("-PT14H"))
                        .formatArg(formatData("PT14H"))
                        .formatArg(formatData(tz->stringValue())),
                        ReportContext::FODT0003, this);
         return Item();

      } else if (tzMSecs > MSecLimit ||
                 tzMSecs < -MSecLimit) {
         context->error(QtXmlPatterns::tr("%1 is not a whole number of minutes.")
                        .formatArg(formatData(tz->stringValue())),
                        ReportContext::FODT0003, this);
         return Item();
      }

      const SecondCountProperty tzSecs = tzMSecs / 1000;

      if (dt.timeZone() == QTimeZone::systemTimeZone()) {
         /* $arg has no time zone. */
         /* "If $arg does not have a timezone component and $timezone is not
          * the empty sequence, then the result is $arg with $timezone as
          * the timezone component." */

         dt.setTimeZone(QTimeZone(tzSecs));

         Q_ASSERT(dt.isValid());
         return createValue(dt);

      } else {
         /* "If $arg has a timezone component and $timezone is not the empty sequence,
          * then the result is an xs:dateTime value with a timezone component of
          * $timezone that is equal to $arg." */

         dt = dt.toUTC();
         dt = dt.addSecs(tzSecs);

         dt.setTimeZone(QTimeZone(tzSecs));

         Q_ASSERT(dt.isValid());
         return createValue(dt);
      }

   } else {
      /* $timezone is the empty sequence. */

      if (dt.timeZone() == QTimeZone::systemTimeZone()) {
         /* $arg has no time zone. */
         /* "If $arg does not have a timezone component and $timezone is
          * the empty sequence, then the result is $arg." */
         return arg;

      } else {
         /* "If $arg has a timezone component and $timezone is the empty sequence,
          * then the result is the localized value of $arg without its timezone component." */

         dt.setTimeZone(QTimeZone::systemTimeZone());

         return createValue(dt);
      }
   }
}

Item AdjustDateTimeToTimezoneFN::createValue(const QDateTime &dt) const
{
   Q_ASSERT(dt.isValid());
   return DateTime::fromDateTime(dt);
}

Item AdjustDateToTimezoneFN::createValue(const QDateTime &dt) const
{
   Q_ASSERT(dt.isValid());
   return Date::fromDateTime(dt);
}

Item AdjustTimeToTimezoneFN::createValue(const QDateTime &dt) const
{
   Q_ASSERT(dt.isValid());
   return SchemaTime::fromDateTime(dt);
}
