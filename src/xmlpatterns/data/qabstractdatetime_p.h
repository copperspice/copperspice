/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QAbstractDateTime_P_H
#define QAbstractDateTime_P_H

#include <QDateTime>
#include <QRegExp>
#include <qitem_p.h>

QT_BEGIN_NAMESPACE

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

   /**
    * @returns the date time this class represents, as a QDateTime.
    */
   inline const QDateTime &toDateTime() const {
      return m_dateTime;
   }

   class CaptureTable
   {
    public:
      CaptureTable(const QRegExp &exp,
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

      const QRegExp regExp;
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
      Q_DISABLE_COPY(CaptureTable)
   };

   /**
    * @returns m_dateTime's time part converted to string. This is for
    * example "12" or "01.023".
    */
   QString timeToString() const;

   /**
    * @returns m_dateTime's date part converted to string. This is for
    * example "2004-05-12" or "-2004-05-12".
    */
   QString dateToString() const;

   /**
    * Serializes the milli seconds @p msecs into a string representation. For
    * example, if @p msecs is 1, ".001" is returned; if @p msecs is 100 then
    * is ".1" returned.
    */
   static QString serializeMSeconds(const MSecondProperty msecs);

   /**
    * A factory function for creating instances that are of the dynamic
    * type of this class, that represents @p dt.
    *
    * The default implementation performs an assert() call. This function
    * is not pure virtual because all sub-classes do not use it.
    */
   virtual Item fromValue(const QDateTime &dt) const;

   /**
    * Determines whether @p dt is a date-time that can be represented,
    * and isn't too early or too late. If it is valid, @c true is returned. Otherwise,
    * @c false is returned and @p message is set to contain a translated message for
    * human consumption, describing the error.
    */
   static bool isRangeValid(const QDate &date,
                            QString &message);

 protected:

   /**
    * @returns m_dateTime' zone offset converted to string, as per the
    * the W3C XML Schema types. This is for
    * example "Z" or "+12.00"(depending on m_dateTime).
    */
   QString zoneOffsetToString() const;

   static QDateTime create(AtomicValue::Ptr &errorMessage,
                           const QString &lexicalSource,
                           const CaptureTable &captTable);

   /**
    * @short Makes the QDateTime::timeSpec() and QDateTime::zoneOffset()
    * of @p ot * consistent to @p from.
    */
   static void copyTimeSpec(const QDateTime &from,
                            QDateTime &to);

   const QDateTime m_dateTime;

 private:
   enum ZoneOffsetParseResult {
      /**
       * syntax or logical error was encountered.
       */
      Error,
      /**
       * It's a valid offset from UTC.
       */
      Offset,

      /**
       * No zone offset was specified, it's an implementation defined zone offset.
       */
      LocalTime,
      UTC
   };

   /**
    * @short Parses the zone offset. All types use zone offsets.
    *
    * If result is set to Offset, the offset is returned, otherwise
    * the return value is undefined.
    *
    * The offset is in seconds.
    */
   static ZOTotal parseZoneOffset(ZoneOffsetParseResult &result,
                                  const QStringList &capts,
                                  const CaptureTable &captTable);

   static inline void setUtcOffset(QDateTime &result,
                                   const ZoneOffsetParseResult zoResult,
                                   const int zoOffset);
};
}

QT_END_NAMESPACE

#endif
