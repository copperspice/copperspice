/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QAtomicComparatorLocators_P_H
#define QAtomicComparatorLocators_P_H

#include <qatomiccomparatorlocator_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class DoubleComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};

class FloatComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};

class DecimalComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class IntegerComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class BooleanComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};

class Base64BinaryComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class HexBinaryComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class QNameComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class StringComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class GYearComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class GMonthComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class GYearMonthComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class GMonthDayComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class GDayComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class DateTimeComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class SchemaTimeComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class DateComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class DurationComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class DayTimeDurationComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};


class YearMonthDurationComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const qint16 op,
         const SourceLocationReflection *const) const;
};
}

QT_END_NAMESPACE

#endif
