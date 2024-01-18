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

#ifndef QYearMonthDuration_P_H
#define QYearMonthDuration_P_H

#include <qabstractduration_p.h>

namespace QPatternist {
class YearMonthDuration : public AbstractDuration
{
 public:
   typedef AtomicValue::Ptr Ptr;

   /**
    * Creates an instance from the lexical representation @p string.
    */
   static YearMonthDuration::Ptr fromLexical(const QString &string);

   static YearMonthDuration::Ptr fromComponents(const bool isPositive,
         const YearProperty years, const MonthProperty months);

   ItemType::Ptr type() const override;
   QString stringValue() const override;

   /**
    * @returns the value of this @c xs:yearMonthDuration in months.
    * @see <a href="http://www.w3.org/TR/xpath-functions/#dt-yearMonthDuration">XQuery 1.0
    * and XPath 2.0 Functions and Operators, 10.3.2.2 Calculating the value of a
    * xs:dayTimeDuration from the lexical representation</a>
    */
   Value value() const override;

   /**
    * If @p val is zero, is CommonValues::YearMonthDurationZero returned.
    */
   Item fromValue(const Value val) const override;

   /**
    * @returns the years component. Always positive.
    */
   YearProperty years() const override;

   /**
    * @returns the months component. Always positive.
    */
   MonthProperty months() const override;

   /**
    * @returns always 0.
    */
   DayCountProperty days() const override;

   /**
    * @returns always 0.
    */
   HourProperty hours() const override;

   /**
    * @returns always 0.
    */
   MinuteProperty minutes() const override;

   /**
    * @returns always 0.
    */
   SecondProperty seconds() const override;

   /**
    * @returns always 0.
    */
   MSecondProperty mseconds() const override;

 protected:
   friend class CommonValues;

   YearMonthDuration(const bool isPositive, const YearProperty years, const MonthProperty months);

 private:
   const YearProperty  m_years;
   const MonthProperty m_months;
};
}

#endif
