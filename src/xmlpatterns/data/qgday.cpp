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

#include "qgday_p.h"

using namespace QPatternist;

GDay::GDay(const QDateTime &dateTime) : AbstractDateTime(dateTime)
{
}

GDay::Ptr GDay::fromLexical(const QString &lexical)
{
   static const CaptureTable captureTable( // STATIC DATA
      /* The extra paranthesis is a build fix for GCC 3.3. */
      (QRegularExpression(QLatin1String(
                  "^\\s*"                                 /* Any preceding whitespace. */
                  "---"                                   /* Delimiter. */
                  "(\\d{2})"                              /* The day part, "03". */
                  "(?:(?:(\\+|-))(\\d{2}):(\\d{2})|(Z))?" /* Timezone, "+08:24". */
                  "\\s*$"                                 /* Any whitespace at the end. */))),
      /*zoneOffsetSignP*/         2,
      /*zoneOffsetHourP*/         3,
      /*zoneOffsetMinuteP*/       4,
      /*zoneOffsetUTCSymbolP*/    5,
      /*yearP*/                   -1,
      /*monthP*/                  -1,
      /*dayP*/                    1);

   AtomicValue::Ptr err;
   const QDateTime retval(create(err, lexical, captureTable));

   return err ? err : GDay::Ptr(new GDay(retval));
}

GDay::Ptr GDay::fromDateTime(const QDateTime &dt)
{
   QDateTime result(QDate(DefaultYear, DefaultMonth, dt.date().day()));
   copyTimeSpec(dt, result);

   return GDay::Ptr(new GDay(result));
}

QString GDay::stringValue() const
{
   return m_dateTime.toString(QLatin1String("---dd")) + zoneOffsetToString();
}

ItemType::Ptr GDay::type() const
{
   return BuiltinTypes::xsGDay;
}
