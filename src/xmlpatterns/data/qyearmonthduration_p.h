/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef Patternist_YearMonthDuration_H
#define Patternist_YearMonthDuration_H

#include "qabstractduration_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist
{   
    class YearMonthDuration : public AbstractDuration
    {
    public:
        typedef AtomicValue::Ptr Ptr;

        /**
         * Creates an instance from the lexical representation @p string.
         */
        static YearMonthDuration::Ptr fromLexical(const QString &string);

        static YearMonthDuration::Ptr fromComponents(const bool isPositive,
                                                     const YearProperty years,
                                                     const MonthProperty months);

        virtual ItemType::Ptr type() const;
        virtual QString stringValue() const;

        /**
         * @returns the value of this @c xs:yearMonthDuration in months.
         * @see <a href="http://www.w3.org/TR/xpath-functions/#dt-yearMonthDuration">XQuery 1.0
         * and XPath 2.0 Functions and Operators, 10.3.2.2 Calculating the value of a
         * xs:dayTimeDuration from the lexical representation</a>
         */
        virtual Value value() const;

        /**
         * If @p val is zero, is CommonValues::YearMonthDurationZero returned.
         */
        virtual Item fromValue(const Value val) const;

        /**
         * @returns the years component. Always positive.
         */
        virtual YearProperty years() const;

        /**
         * @returns the months component. Always positive.
         */
        virtual MonthProperty months() const;

        /**
         * @returns always 0.
         */
        virtual DayCountProperty days() const;

        /**
         * @returns always 0.
         */
        virtual HourProperty hours() const;

        /**
         * @returns always 0.
         */
        virtual MinuteProperty minutes() const;

        /**
         * @returns always 0.
         */
        virtual SecondProperty seconds() const;

        /**
         * @returns always 0.
         */
        virtual MSecondProperty mseconds() const;

    protected:
        friend class CommonValues;

        YearMonthDuration(const bool isPositive,
                          const YearProperty years,
                          const MonthProperty months);

    private:
        const YearProperty  m_years;
        const MonthProperty m_months;
    };
}

QT_END_NAMESPACE

#endif
