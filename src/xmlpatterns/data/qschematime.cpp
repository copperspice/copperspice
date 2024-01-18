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
#include "qitem_p.h"

#include "qschematime_p.h"

using namespace QPatternist;

SchemaTime::SchemaTime(const QDateTime &dateTime) : AbstractDateTime(dateTime)
{
}

SchemaTime::Ptr SchemaTime::fromLexical(const QString &lexical)
{
   static const CaptureTable captureTable( // STATIC DATA
      /* The extra paranthesis is a build fix for GCC 3.3. */
      (QRegularExpression(QLatin1String(
                  "^\\s*"                             /* Any preceding whitespace. */
                  "(\\d{2})"                          /* Hour part */
                  ":"                                 /* Delimiter. */
                  "(\\d{2})"                          /* Minutes part */
                  ":"                                 /* Delimiter. */
                  "(\\d{2,})"                         /* Seconds part. */
                  "(?:\\.(\\d+))?"                    /* Milli seconds part. */
                  "(?:(\\+|-)(\\d{2}):(\\d{2})|(Z))?" /* The zone offset, "+08:24". */
                  "\\s*$"                             /* Any terminating whitespace. */))),
      /*zoneOffsetSignP*/         5,
      /*zoneOffsetHourP*/         6,
      /*zoneOffsetMinuteP*/       7,
      /*zoneOffsetUTCSymbolP*/    8,
      /*yearP*/                   -1,
      /*monthP*/                  -1,
      /*dayP*/                    -1,
      /*hourP*/                   1,
      /*minutesP*/                2,
      /*secondsP*/                3,
      /*msecondsP*/               4);

   AtomicValue::Ptr err;
   const QDateTime retval(create(err, lexical, captureTable));

   return err ? err : SchemaTime::Ptr(new SchemaTime(retval));
}

SchemaTime::Ptr SchemaTime::fromDateTime(const QDateTime &dt)
{
   Q_ASSERT(dt.isValid());
   /* Singleton value, allocated once instead of each time it's needed. */
   // STATIC DATA
   static const QDate time_defaultDate(AbstractDateTime::DefaultYear,
                                       AbstractDateTime::DefaultMonth,
                                       AbstractDateTime::DefaultDay);

   QDateTime result;
   copyTimeSpec(dt, result);

   result.setDate(time_defaultDate);
   result.setTime(dt.time());

   return SchemaTime::Ptr(new SchemaTime(result));
}

Item SchemaTime::fromValue(const QDateTime &dt) const
{
   Q_ASSERT(dt.isValid());
   return fromDateTime(dt);
}

QString SchemaTime::stringValue() const
{
   return timeToString() + zoneOffsetToString();
}

ItemType::Ptr SchemaTime::type() const
{
   return BuiltinTypes::xsTime;
}
