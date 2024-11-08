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

#ifndef QDayTimeDuration_P_H
#define QDayTimeDuration_P_H

#include <qabstractduration_p.h>
#include <qitem_p.h>

namespace QPatternist {
class DayTimeDuration : public AbstractDuration
{
 public:
   typedef QExplicitlySharedDataPointer<DayTimeDuration> Ptr;

   static DayTimeDuration::Ptr fromLexical(const QString &string);

   static DayTimeDuration::Ptr fromComponents(const bool isPositive,
         const DayCountProperty days,
         const HourProperty hours,
         const MinuteProperty minutes,
         const SecondProperty seconds,
         const MSecondProperty mseconds);

   static DayTimeDuration::Ptr fromSeconds(const SecondCountProperty secs, const MSecondProperty msecs = 0);

   ItemType::Ptr type() const override;
   QString stringValue() const override;

   YearProperty years() const override;
   MonthProperty months() const override;
   DayCountProperty days() const override;
   HourProperty hours() const override;
   MinuteProperty minutes() const override;
   MSecondProperty mseconds() const override;
   SecondProperty seconds() const override;
   Value value() const override;
   Item fromValue(const Value val) const override;

 protected:
   friend class CommonValues;

   DayTimeDuration(const bool isPositive, const DayCountProperty days, const HourProperty hours,
         const MinuteProperty minutes, const SecondProperty seconds, const MSecondProperty mseconds);

 private:
   const DayCountProperty  m_days;
   const HourProperty      m_hours;
   const MinuteProperty    m_minutes;
   const SecondProperty    m_seconds;
   const MSecondProperty   m_mseconds;
};
}

#endif
