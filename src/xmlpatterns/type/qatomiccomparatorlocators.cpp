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

#include <qatomiccomparators_p.h>

#include <qatomiccomparatorlocators_p.h>

using namespace QPatternist;

static const AtomicComparator::Operators AllCompOperators(AtomicComparator::OperatorNotEqual            |
      AtomicComparator::OperatorGreaterOrEqual      |
      AtomicComparator::OperatorLessOrEqual         |
      AtomicComparator::OperatorLessThanNaNLeast    |
      AtomicComparator::OperatorLessThanNaNGreatest);

/* --------------------------------------------------------------- */
#define addVisitor(owner, type, comp, validOps)                                \
AtomicTypeVisitorResult::Ptr                                                   \
owner##ComparatorLocator::visit(const type *,                                  \
                                const qint16 op,                               \
                                const SourceLocationReflection *const) const   \
{                                                                              \
    if(( (validOps) & AtomicComparator::Operator(op) ) == AtomicComparator::Operator(op)) \
        return AtomicTypeVisitorResult::Ptr(new comp());                       \
    else                                                                       \
        return AtomicTypeVisitorResult::Ptr();                                 \
}

/* --------------------------------------------------------------- */
#define visitorForDouble(owner, type)                                                                                           \
AtomicTypeVisitorResult::Ptr                                                                                                    \
owner##ComparatorLocator::visit(const type *,                                                                                   \
                                const qint16 op,                                                                                \
                                const SourceLocationReflection *const) const                                                    \
{                                                                                                                               \
    if(((AtomicComparator::OperatorNotEqual        |                                                                            \
         AtomicComparator::OperatorGreaterOrEqual  |                                                                            \
         AtomicComparator::OperatorLessOrEqual) & AtomicComparator::Operator(op)) == AtomicComparator::Operator(op))            \
        return AtomicTypeVisitorResult::Ptr(new AbstractFloatComparator());                                                     \
    else if(op == AtomicComparator::OperatorLessThanNaNLeast)                                                                   \
        return AtomicTypeVisitorResult::Ptr(new AbstractFloatSortComparator<AtomicComparator::OperatorLessThanNaNLeast>());     \
    else if(op == AtomicComparator::OperatorLessThanNaNGreatest)                                                                \
        return AtomicTypeVisitorResult::Ptr(new AbstractFloatSortComparator<AtomicComparator::OperatorLessThanNaNGreatest>());  \
    else                                                                                                                        \
        return AtomicTypeVisitorResult::Ptr();                                                                                  \
}
/* --------------------------------------------------------------- */

/* ----------- xs:string, xs:anyURI, xs:untypedAtomic  ----------- */
addVisitor(String,  StringType,         StringComparator,
           AllCompOperators)
addVisitor(String,  UntypedAtomicType,  StringComparator,
           AllCompOperators)
addVisitor(String,  AnyURIType,         StringComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* ------------------------- xs:hexBinary ------------------------ */
addVisitor(HexBinary,   HexBinaryType,        BinaryDataComparator,
           AtomicComparator::OperatorEqual |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* ----------------------- xs:base64Binary ----------------------- */
addVisitor(Base64Binary,    Base64BinaryType,    BinaryDataComparator,
           AtomicComparator::OperatorEqual |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* -------------------------- xs:boolean ------------------------- */
addVisitor(Boolean,     BooleanType,        BooleanComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* -------------------------- xs:double -------------------------- */
visitorForDouble(Double,      DoubleType)
visitorForDouble(Double,      FloatType)
visitorForDouble(Double,      DecimalType)
visitorForDouble(Double,      IntegerType)
/* --------------------------------------------------------------- */

/* --------------------------- xs:float -------------------------- */
visitorForDouble(Float,   DoubleType)
visitorForDouble(Float,   FloatType)
visitorForDouble(Float,   DecimalType)
visitorForDouble(Float,   IntegerType)
/* --------------------------------------------------------------- */

/* -------------------------- xs:decimal ------------------------- */
visitorForDouble(Decimal,     DoubleType)
visitorForDouble(Decimal,     FloatType)
addVisitor(Decimal,     DecimalType,    DecimalComparator,
           AllCompOperators)
addVisitor(Decimal,     IntegerType,    DecimalComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* ------------------------- xs:integer -------------------------- */
visitorForDouble(Integer,     DoubleType)
visitorForDouble(Integer,     FloatType)
addVisitor(Integer,     DecimalType,    DecimalComparator,
           AllCompOperators)
addVisitor(Integer,     IntegerType,    IntegerComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* -------------------------- xs:QName --------------------------- */
addVisitor(QName,       QNameType,          QNameComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* -------------------------- xs:gYear --------------------------- */
addVisitor(GYear,       GYearType,          AbstractDateTimeComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* -------------------------- xs:gDay ---------------------------- */
addVisitor(GDay,        GDayType,           AbstractDateTimeComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* -------------------------- xs:gMonth -------------------------- */
addVisitor(GMonth,      GMonthType,         AbstractDateTimeComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* ------------------------ xs:gYearMonth ------------------------ */
addVisitor(GYearMonth,  GYearMonthType,     AbstractDateTimeComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* ------------------------ xs:gMonthDay ------------------------- */
addVisitor(GMonthDay,   GMonthDayType,      AbstractDateTimeComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* ------------------------ xs:dateTime -------------------------- */
addVisitor(DateTime,    DateTimeType,    AbstractDateTimeComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* -------------------------- xs:time ---------------------------- */
addVisitor(SchemaTime,        SchemaTimeType,       AbstractDateTimeComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* -------------------------- xs:date ---------------------------- */
addVisitor(Date,        DateType,       AbstractDateTimeComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */

/* ------------------------ xs:duration -------------------------- */
addVisitor(Duration,        DayTimeDurationType,        AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
addVisitor(Duration,        DurationType,               AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
addVisitor(Duration,        YearMonthDurationType,      AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* ------------------ xs:dayTimeDuration ------------------------ */
addVisitor(DayTimeDuration,     DayTimeDurationType,    AbstractDurationComparator,
           AllCompOperators)
addVisitor(DayTimeDuration,     DurationType,           AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
addVisitor(DayTimeDuration,     YearMonthDurationType,  AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
/* --------------------------------------------------------------- */

/* ------------------- xs:yearMonthDuration --------------------- */
addVisitor(YearMonthDuration,   DayTimeDurationType,    AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
addVisitor(YearMonthDuration,   DurationType,           AbstractDurationComparator,
           AtomicComparator::OperatorEqual     |
           AtomicComparator::OperatorNotEqual)
addVisitor(YearMonthDuration,   YearMonthDurationType,  AbstractDurationComparator,
           AllCompOperators)
/* --------------------------------------------------------------- */
#undef addVisitor
