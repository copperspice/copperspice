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

#ifndef QAbstractDuration_P_H
#define QAbstractDuration_P_H

#include <qregularexpression.h>
#include <qitem_p.h>

namespace QPatternist {

class AbstractDuration : public AtomicValue
{
 public:
   typedef QExplicitlySharedDataPointer<AbstractDuration> Ptr;
   typedef qint64 Value;


   class CaptureTable
   {
    public:
      CaptureTable(const QRegularExpression &exp,
                   const qint8 yearP,
                   const qint8 monthP,
                   const qint8 dayP = -1,
                   const qint8 tDelimiterP = -1,
                   const qint8 hourP = -1,
                   const qint8 minutesP = -1,
                   const qint8 secondsP = -1,
                   const qint8 msecondsP = -1) : regExp(exp),
         year(yearP),
         month(monthP),
         day(dayP),
         tDelimiter(tDelimiterP),
         hour(hourP),
         minutes(minutesP),
         seconds(secondsP),
         mseconds(msecondsP) {
         Q_ASSERT(exp.isValid());
         Q_ASSERT(yearP == -1 || yearP == 2);
      }

      const QRegularExpression regExp;
      const qint8 year;
      const qint8 month;
      const qint8 day;
      const qint8 tDelimiter;
      const qint8 hour;
      const qint8 minutes;
      const qint8 seconds;
      const qint8 mseconds;
   };


   bool operator==(const AbstractDuration &other) const;

   virtual YearProperty years() const = 0;
   virtual MonthProperty months() const = 0;
   virtual DayCountProperty days() const = 0;
   virtual HourProperty hours() const = 0;
   virtual MinuteProperty minutes() const = 0;
   virtual SecondProperty seconds() const = 0;
   virtual MSecondProperty mseconds() const = 0;

   virtual Value value() const = 0;
   virtual Item fromValue(const Value val) const = 0;

   bool isPositive() const;

 protected:

   AbstractDuration(const bool isPos);

   static QString serializeMSeconds(const MSecondProperty mseconds);
   static AtomicValue::Ptr create(const CaptureTable &captTable,
                                  const QString &lexical,
                                  bool *isPositive,
                                  YearProperty *years,
                                  MonthProperty *months,
                                  DayCountProperty *days,
                                  HourProperty *hours,
                                  MinuteProperty *minutes,
                                  SecondProperty *seconds,
                                  MSecondProperty *mseconds);
   const bool m_isPositive;
};
}

#endif
