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

#include "qabstractfloatmathematician_p.h"
#include "qatomicmathematicianlocators_p.h"
#include "qatomicmathematicians_p.h"

using namespace QPatternist;

#define implMathVisit(ownerClass, visitor, mather, validOps)                            \
AtomicTypeVisitorResult::Ptr                                                            \
ownerClass##MathematicianLocator::visit(const visitor *, const qint16 op,               \
                                        const SourceLocationReflection *const r) const  \
{                                                                                       \
    (void) r;                                                                           \
    /* Note the extra paranteses around validOps. */                                    \
    if(((validOps) & AtomicMathematician::Operator(op)) == AtomicMathematician::Operator(op))  \
        return AtomicTypeVisitorResult::Ptr(new mather());                              \
    else                                                                                \
        return AtomicTypeVisitorResult::Ptr();                                          \
}

#define implReportingMathVisit(ownerClass, visitor, mather, validOps)                   \
AtomicTypeVisitorResult::Ptr                                                            \
ownerClass##MathematicianLocator::visit(const visitor *, const qint16 op,               \
                                        const SourceLocationReflection *const r) const  \
{                                                                                       \
    /* Note the extra paranteses around validOps. */                                    \
    if(((validOps) & AtomicMathematician::Operator(op)) == AtomicMathematician::Operator(op))   \
        return AtomicTypeVisitorResult::Ptr(new mather(r));                             \
    else                                                                                \
        return AtomicTypeVisitorResult::Ptr();                                          \
}

#define implRevReportingMathVisit(ownerClass, visitor, mather, validOps)                \
AtomicTypeVisitorResult::Ptr                                                            \
ownerClass##MathematicianLocator::visit(const visitor *, const qint16 op,               \
                                        const SourceLocationReflection *const r) const  \
{                                                                                       \
    /* Note the extra paranteses around validOps. */                                    \
    if(((validOps) & AtomicMathematician::Operator(op)) == AtomicMathematician::Operator(op)) \
        return AtomicTypeVisitorResult::Ptr(new OperandSwitcherMathematician(           \
                                            AtomicMathematician::Ptr(new mather(r))));  \
    else                                                                                \
        return AtomicTypeVisitorResult::Ptr();                                          \
}

static const AtomicMathematician::Operators AllMathOperators(AtomicMathematician::Add       |
      AtomicMathematician::Div       |
      AtomicMathematician::IDiv      |
      AtomicMathematician::Mod       |
      AtomicMathematician::Multiply  |
      AtomicMathematician::Substract);

static const AtomicMathematician::Operators DivMultiply(AtomicMathematician::Multiply       |
      AtomicMathematician::Div);

static const AtomicMathematician::Operators DurationOps(AtomicMathematician::Div            |
      AtomicMathematician::Substract      |
      AtomicMathematician::Add);

static const AtomicMathematician::Operators DTOps(AtomicMathematician::Substract            |
      AtomicMathematician::Add);

implReportingMathVisit(Double,           DecimalType,            DoubleMathematician,    AllMathOperators)
implReportingMathVisit(Double,           DoubleType,             DoubleMathematician,    AllMathOperators)
implReportingMathVisit(Double,           FloatType,              DoubleMathematician,    AllMathOperators)
implReportingMathVisit(Double,           IntegerType,            DoubleMathematician,    AllMathOperators)
implRevReportingMathVisit(Double,           YearMonthDurationType,  DurationNumericMathematician,
                          AtomicMathematician::Multiply)
implRevReportingMathVisit(Double,           DayTimeDurationType,    DurationNumericMathematician,
                          AtomicMathematician::Multiply)

implReportingMathVisit(Float,            DecimalType,            FloatMathematician,     AllMathOperators)
implReportingMathVisit(Float,            DoubleType,    DoubleMathematician,    AllMathOperators)
implReportingMathVisit(Float,            FloatType,              FloatMathematician,     AllMathOperators)
implReportingMathVisit(Float,            IntegerType,            FloatMathematician,     AllMathOperators)
implRevReportingMathVisit(Float,            YearMonthDurationType,  DurationNumericMathematician,
                          AtomicMathematician::Multiply)
implRevReportingMathVisit(Float,            DayTimeDurationType,    DurationNumericMathematician,
                          AtomicMathematician::Multiply)

implReportingMathVisit(Decimal, DecimalType,            DecimalMathematician,   AllMathOperators)
implReportingMathVisit(Decimal,          DoubleType,    DoubleMathematician,    AllMathOperators)
implReportingMathVisit(Decimal,          FloatType,              FloatMathematician,     AllMathOperators)
implReportingMathVisit(Decimal, IntegerType,            DecimalMathematician,   AllMathOperators)
implRevReportingMathVisit(Decimal,          YearMonthDurationType,  DurationNumericMathematician,
                          AtomicMathematician::Multiply)
implRevReportingMathVisit(Decimal,          DayTimeDurationType,    DurationNumericMathematician,
                          AtomicMathematician::Multiply)

implReportingMathVisit(Integer, DecimalType,            DecimalMathematician,   AllMathOperators)
implReportingMathVisit(Integer,          DoubleType,    DoubleMathematician,    AllMathOperators)
implReportingMathVisit(Integer,          FloatType,              FloatMathematician,     AllMathOperators)
implReportingMathVisit(Integer, IntegerType,            IntegerMathematician,   AllMathOperators)
implRevReportingMathVisit(Integer,          YearMonthDurationType,  DurationNumericMathematician,
                          AtomicMathematician::Multiply)
implRevReportingMathVisit(Integer,          DayTimeDurationType,    DurationNumericMathematician,
                          AtomicMathematician::Multiply)

implRevReportingMathVisit(DayTimeDuration,  DateTimeType,           DateTimeDurationMathematician,
                          AtomicMathematician::Add)
implRevReportingMathVisit(DayTimeDuration,  DateType,               DateTimeDurationMathematician,
                          AtomicMathematician::Add)
implMathVisit(DayTimeDuration,  DayTimeDurationType,    DurationDurationMathematician, DurationOps)
implReportingMathVisit(DayTimeDuration,  DecimalType,   DurationNumericMathematician,  DivMultiply)
implReportingMathVisit(DayTimeDuration,  DoubleType,    DurationNumericMathematician,  DivMultiply)
implReportingMathVisit(DayTimeDuration,  FloatType,     DurationNumericMathematician,  DivMultiply)
implReportingMathVisit(DayTimeDuration,  IntegerType,   DurationNumericMathematician,  DivMultiply)
implRevReportingMathVisit(DayTimeDuration,  SchemaTimeType,               DateTimeDurationMathematician,
                          AtomicMathematician::Add)

implRevReportingMathVisit(YearMonthDuration, DateTimeType,          DateTimeDurationMathematician,
                          AtomicMathematician::Add)
implRevReportingMathVisit(YearMonthDuration, DateType,              DateTimeDurationMathematician,
                          AtomicMathematician::Add)
implReportingMathVisit(YearMonthDuration, DecimalType,  DurationNumericMathematician,  DivMultiply)
implReportingMathVisit(YearMonthDuration, DoubleType,   DurationNumericMathematician,  DivMultiply)
implReportingMathVisit(YearMonthDuration, FloatType,    DurationNumericMathematician,  DivMultiply)
implReportingMathVisit(YearMonthDuration, IntegerType,  DurationNumericMathematician,  DivMultiply)
implMathVisit(YearMonthDuration, YearMonthDurationType, DurationDurationMathematician, DurationOps)

implMathVisit(Date,              DateType,              AbstractDateTimeMathematician,
              AtomicMathematician::Substract)
implReportingMathVisit(Date,     YearMonthDurationType, DateTimeDurationMathematician,       DTOps)
implReportingMathVisit(Date,     DayTimeDurationType,   DateTimeDurationMathematician,       DTOps)

implMathVisit(SchemaTime,              SchemaTimeType,              AbstractDateTimeMathematician,
              AtomicMathematician::Substract)
implReportingMathVisit(SchemaTime,     DayTimeDurationType,   DateTimeDurationMathematician,       DTOps)

implMathVisit(DateTime,          DateTimeType,          AbstractDateTimeMathematician,
              AtomicMathematician::Substract)
implReportingMathVisit(DateTime, YearMonthDurationType, DateTimeDurationMathematician,       DTOps)
implReportingMathVisit(DateTime, DayTimeDurationType,   DateTimeDurationMathematician,       DTOps)

#undef implMathVisit
#undef implReportingMathVisit
#undef implRevReportingMathVisit
