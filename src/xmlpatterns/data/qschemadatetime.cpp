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

#include "qschemadatetime_p.h"

using namespace QPatternist;

DateTime::DateTime(const QDateTime &dateTime) : AbstractDateTime(dateTime)
{
}

DateTime::Ptr DateTime::fromLexical(const QString &lexical)
{
   static const CaptureTable captureTable( // STATIC DATA
      /* The extra paranthesis is a build fix for GCC 3.3. */
      (QRegularExpression(QLatin1String(
                  "^\\s*"                                     /* Any preceding whitespace. */
                  "(-?)"                                      /* Any preceding minus. */
                  "(\\d{4,})"                                 /* The year part. */
                  "-"                                         /* Delimiter. */
                  "(\\d{2})"                                  /* The month part. */
                  "-"                                         /* Delimiter. */
                  "(\\d{2})"                                  /* The day part. */
                  "T"                                         /* Delimiter. */
                  "(\\d{2})"                                  /* Hour part */
                  ":"                                         /* Delimiter. */
                  "(\\d{2})"                                  /* Minutes part */
                  ":"                                         /* Delimiter. */
                  "(\\d{2,})"                                 /* Seconds part. */
                  "(?:\\.(\\d+))?"                            /* Milli seconds part. */
                  "(?:(\\+|-)(\\d{2}):(\\d{2})|(Z))?"         /* The zone offset, "+08:24". */
                  "\\s*$"                                     /* Any whitespace at the end. */))),
      /*zoneOffsetSignP*/         9,
      /*zoneOffsetHourP*/         10,
      /*zoneOffsetMinuteP*/       11,
      /*zoneOffsetUTCSymbolP*/    12,
      /*yearP*/                   2,
      /*monthP*/                  3,
      /*dayP*/                    4,
      /*hourP*/                   5,
      /*minutesP*/                6,
      /*secondsP*/                7,
      /*msecondsP*/               8,
      /*yearSignP*/               1);

   AtomicValue::Ptr err;
   const QDateTime retval(create(err, lexical, captureTable));

   return err ? err : DateTime::Ptr(new DateTime(retval));
}

DateTime::Ptr DateTime::fromDateTime(const QDateTime &dt)
{
   Q_ASSERT(dt.isValid());
   return DateTime::Ptr(new DateTime(dt));
}

Item DateTime::fromValue(const QDateTime &dt) const
{
   Q_ASSERT(dt.isValid());
   return fromDateTime(dt);
}

QString DateTime::stringValue() const
{
   return dateToString() + QLatin1Char('T') + timeToString() + zoneOffsetToString();
}

ItemType::Ptr DateTime::type() const
{
   return BuiltinTypes::xsDateTime;
}
