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

#include "qdate_p.h"

using namespace QPatternist;

Date::Date(const QDateTime &dateTime) : AbstractDateTime(dateTime)
{
}

Date::Ptr Date::fromLexical(const QString &lexical)
{
   static const CaptureTable captureTable( // STATIC DATA
      /* The extra paranthesis is a build fix for GCC 3.3. */
      (QRegularExpression(QLatin1String(
                  "^\\s*"                             /* Any preceding whitespace. */
                  "(-?)"                              /* Any preceding minus. */
                  "(\\d{4,})"                         /* The year part. */
                  "-"                                 /* Delimiter. */
                  "(\\d{2})"                          /* The month part. */
                  "-"                                 /* Delimiter. */
                  "(\\d{2})"                          /* The day part. */
                  "(?:(\\+|-)(\\d{2}):(\\d{2})|(Z))?" /* The zone offset, "+08:24". */
                  "\\s*$"                             /* Any terminating whitespace. */))),
      /*zoneOffsetSignP*/         5,
      /*zoneOffsetHourP*/         6,
      /*zoneOffsetMinuteP*/       7,
      /*zoneOffsetUTCSymbolP*/    8,
      /*yearP*/                   2,
      /*monthP*/                  3,
      /*dayP*/                    4,
      /*hourP*/                   -1,
      /*minutesP*/                -1,
      /*secondsP*/                -1,
      /*msecondsP*/               -1,
      /*yearSign*/                1);

   AtomicValue::Ptr err;
   const QDateTime retval(create(err, lexical, captureTable));

   return err ? err : Date::Ptr(new Date(retval));
}

Date::Ptr Date::fromDateTime(const QDateTime &date)
{
   /* Don't include the QTime; "reset" the time. */
   QDateTime result;
   copyTimeSpec(date, result);
   result.setDate(date.date());
   Q_ASSERT(date.isValid());

   return Date::Ptr(new Date(result));
}

Item Date::fromValue(const QDateTime &dt) const
{
   Q_ASSERT(dt.isValid());
   return fromDateTime(dt);
}

QString Date::stringValue() const
{
   return dateToString() + zoneOffsetToString();
}

ItemType::Ptr Date::type() const
{
   return BuiltinTypes::xsDate;
}
