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

#include "qbuiltintypes_p.h"

#include "qgmonthday_p.h"

using namespace QPatternist;

GMonthDay::GMonthDay(const QDateTime &dateTime) : AbstractDateTime(dateTime)
{
}

GMonthDay::Ptr GMonthDay::fromLexical(const QString &lexical)
{
   static const CaptureTable captureTable( // STATIC DATA
      /* The extra paranthesis is a build fix for GCC 3.3. */
      (QRegularExpression(QLatin1String(
                  "^\\s*"                             /* Any preceding whitespace. */
                  "--"                                /* Delimiter. */
                  "(\\d{2})"                          /* The month part. */
                  "-"                                 /* Delimiter. */
                  "(\\d{2})"                          /* The day part. */
                  "(?:(\\+|-)(\\d{2}):(\\d{2})|(Z))?" /* The zone offset, "+08:24". */
                  "\\s*$"                             /* Any terminating whitespace. */))),
      /*zoneOffsetSignP*/         3,
      /*zoneOffsetHourP*/         4,
      /*zoneOffsetMinuteP*/       5,
      /*zoneOffsetUTCSymbolP*/    6,
      /*yearP*/                   -1,
      /*monthP*/                  1,
      /*dayP*/                    2);

   AtomicValue::Ptr err;
   const QDateTime retval(create(err, lexical, captureTable));

   return err ? err : GMonthDay::Ptr(new GMonthDay(retval));
}

GMonthDay::Ptr GMonthDay::fromDateTime(const QDateTime &dt)
{
   QDateTime result(QDate(DefaultYear, dt.date().month(), dt.date().day()));
   copyTimeSpec(dt, result);

   return GMonthDay::Ptr(new GMonthDay(result));
}

QString GMonthDay::stringValue() const
{
   return m_dateTime.toString(QLatin1String("--MM-dd")) + zoneOffsetToString();
}

ItemType::Ptr GMonthDay::type() const
{
   return BuiltinTypes::xsGMonthDay;
}
