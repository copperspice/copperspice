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

#ifndef QDuration_P_H
#define QDuration_P_H

#include <qabstractduration_p.h>

namespace QPatternist {

class Duration : public AbstractDuration
{
 public:
   typedef AtomicValue::Ptr Ptr;

   /**
    * Creates an instance from the lexical representation @p string.
    */
   static Duration::Ptr fromLexical(const QString &string);
   static Duration::Ptr fromComponents(const bool isPositive, const YearProperty years,
                  const MonthProperty months, const DayCountProperty days, const HourProperty hours,
                  const MinuteProperty minutes, const SecondProperty seconds, const MSecondProperty mseconds);

   ItemType::Ptr type() const override;
   QString stringValue() const override;

   /**
    * Always results in an assert crash. Calling this function makes no
    * sense due to that the value space of xs:duration is not well defined.
    */
   Value value() const override;

   /**
    * Always results in an assert crash. Calling this function makes no
    * sense due to that the value space of xs:duration is not well defined.
    */
   Item fromValue(const Value val) const override;

   YearProperty years() const override;
   MonthProperty months() const override;
   DayCountProperty days() const override;
   HourProperty hours() const override;
   MinuteProperty minutes() const override;
   SecondProperty seconds() const override;
   MSecondProperty mseconds() const override;

 protected:
   friend class CommonValues;

   Duration(const bool isPositive, const YearProperty years, const MonthProperty months,
                  const DayCountProperty days, const HourProperty hours, const MinuteProperty minutes,
                  const SecondProperty seconds, const MSecondProperty mseconds);
 private:
   const YearProperty      m_years;
   const MonthProperty     m_months;
   const DayCountProperty  m_days;
   const HourProperty      m_hours;
   const MinuteProperty    m_minutes;
   const SecondProperty    m_seconds;
   const MSecondProperty   m_mseconds;
};
}

#endif
