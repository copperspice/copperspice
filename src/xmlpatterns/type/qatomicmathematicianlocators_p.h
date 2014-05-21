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

#ifndef Patternist_AtomicMathematicianLocators_P_H
#define Patternist_AtomicMathematicianLocators_P_H

#include "qatomicmathematician_p.h"
#include "qatomicmathematicianlocator_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist
{   
    class DoubleMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };
   
    class FloatMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };
    
    class DecimalMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };
   
    class IntegerMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };
    
    class DateMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };
    
    class SchemaTimeMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };
  
    class DateTimeMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };
 
    class DayTimeDurationMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };

   
    class YearMonthDurationMathematicianLocator : public AtomicMathematicianLocator
    {
        using AtomicMathematicianLocator::visit;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DateType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
        virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op,
                                                   const SourceLocationReflection *const r) const;
    };
}

QT_END_NAMESPACE

#endif
