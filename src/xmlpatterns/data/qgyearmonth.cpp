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

#include "qgyearmonth_p.h"

using namespace QPatternist;

GYearMonth::GYearMonth(const QDateTime &dateTime) : AbstractDateTime(dateTime)
{
}

GYearMonth::Ptr GYearMonth::fromLexical(const QString &lexical)
{
   static const CaptureTable captureTable( // STATIC DATA
      /* The extra paranthesis is a build fix for GCC 3.3. */
      (QRegularExpression(QLatin1String(
                  "^\\s*"                             /* Any preceding whitespace. */
                  "(-?)"                              /* Any preceding minus. */
                  "(\\d{4,})"                         /* The year part. */
                  "-"                                 /* Delimiter. */
                  "(\\d{2})"                          /* The month part. */
                  "(?:(\\+|-)(\\d{2}):(\\d{2})|(Z))?" /* The zone offset, "+08:24". */
                  "\\s*$"                             /* Any terminating whitespace. */))),
      /*zoneOffsetSignP*/         4,
      /*zoneOffsetHourP*/         5,
      /*zoneOffsetMinuteP*/       6,
      /*zoneOffsetUTCSymbolP*/    7,
      /*yearP*/                   2,
      /*monthP*/                  3,
      /*dayP*/                    -1,
      /*hourP*/                   -1,
      /*minutesP*/                -1,
      /*secondsP*/                -1,
      /*msecondsP*/               -1,
      /*yearSign*/                1);

   AtomicValue::Ptr err;
   const QDateTime retval(create(err, lexical, captureTable));

   return err ? err : GYearMonth::Ptr(new GYearMonth(retval));
}

GYearMonth::Ptr GYearMonth::fromDateTime(const QDateTime &dt)
{
   QDateTime result(QDate(dt.date().year(), dt.date().month(), DefaultDay));
   copyTimeSpec(dt, result);

   return GYearMonth::Ptr(new GYearMonth(result));
}

QString GYearMonth::stringValue() const
{
   return m_dateTime.toString(QLatin1String("yyyy-MM")) + zoneOffsetToString();
}

ItemType::Ptr GYearMonth::type() const
{
   return BuiltinTypes::xsGYearMonth;
}
