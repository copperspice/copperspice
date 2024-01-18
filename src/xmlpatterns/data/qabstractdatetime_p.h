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

#ifndef QAbstractDateTime_P_H
#define QAbstractDateTime_P_H

#include <qdatetime.h>
#include <qregularexpression.h>

#include <qitem_p.h>

namespace QPatternist {

class AbstractDateTime : public AtomicValue
{
 public:
   typedef QExplicitlySharedDataPointer<AbstractDateTime> Ptr;

   AbstractDateTime(const QDateTime &dateTime);

   enum {
      DefaultYear     = 2000,
      DefaultMonth    = 1,
      DefaultDay      = 1
   };

   inline const QDateTime &toDateTime() const {
      return m_dateTime;
   }

   class CaptureTable
   {
    public:
      CaptureTable(const QRegularExpression &exp,
                   const qint8 zoneOffsetSignP,
                   const qint8 zoneOffsetHourP,
                   const qint8 zoneOffsetMinuteP,
                   const qint8 zoneOffsetUTCSymbolP,
                   const qint8 yearP,
                   const qint8 monthP = -1,
                   const qint8 dayP = -1,
                   const qint8 hourP = -1,
                   const qint8 minutesP = -1,
                   const qint8 secondsP = -1,
                   const qint8 msecondsP = -1,
                   const qint8 yearSignP = -1) : regExp(exp)
         , zoneOffsetSign(zoneOffsetSignP)
         , zoneOffsetHour(zoneOffsetHourP)
         , zoneOffsetMinute(zoneOffsetMinuteP)
         , zoneOffsetUTCSymbol(zoneOffsetUTCSymbolP)
         , year(yearP)
         , month(monthP)
         , day(dayP)
         , hour(hourP)
         , minutes(minutesP)
         , seconds(secondsP)
         , mseconds(msecondsP)
         , yearSign(yearSignP) {
         Q_ASSERT(exp.isValid());
      }

      const QRegularExpression regExp;
      const qint8 zoneOffsetSign;
      const qint8 zoneOffsetHour;
      const qint8 zoneOffsetMinute;
      const qint8 zoneOffsetUTCSymbol;
      const qint8 year;
      const qint8 month;
      const qint8 day;
      const qint8 hour;
      const qint8 minutes;
      const qint8 seconds;
      const qint8 mseconds;
      const qint8 yearSign;

    private:
      CaptureTable(const CaptureTable &) = delete;
      CaptureTable &operator=(const CaptureTable &) = delete;
   };

   QString timeToString() const;
   QString dateToString() const;

   static QString serializeMSeconds(const MSecondProperty msecs);

   virtual Item fromValue(const QDateTime &dt) const;

   static bool isRangeValid(const QDate &date, QString &message);

 protected:
   QString zoneOffsetToString() const;

   static QDateTime create(AtomicValue::Ptr &errorMessage, const QString &lexicalSource, const CaptureTable &captTable);
   static void copyTimeSpec(const QDateTime &from, QDateTime &to);
   const QDateTime m_dateTime;

 private:
   enum ZoneOffsetParseResult {
      Error,
      Offset,
      LocalTime,
      UTC
   };

   static ZOTotal parseZoneOffset(ZoneOffsetParseResult &result, const QStringList &capts, const CaptureTable &captTable);
   static inline void setUtcOffset(QDateTime &result,const ZoneOffsetParseResult zoResult, const int zoOffset);
};
}

#endif
