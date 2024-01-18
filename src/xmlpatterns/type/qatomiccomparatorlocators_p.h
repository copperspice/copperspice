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

#ifndef QAtomicComparatorLocators_P_H
#define QAtomicComparatorLocators_P_H

#include <qatomiccomparatorlocator_p.h>

namespace QPatternist {

class DoubleComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};

class FloatComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};

class DecimalComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class IntegerComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class BooleanComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};

class Base64BinaryComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class HexBinaryComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class QNameComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const QNameType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class StringComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class GYearComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const GYearType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class GMonthComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class GYearMonthComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class GMonthDayComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class GDayComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const GDayType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class DateTimeComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class SchemaTimeComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class DateComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class DurationComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class DayTimeDurationComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};


class YearMonthDurationComparatorLocator : public AtomicComparatorLocator
{
   using AtomicComparatorLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const qint16 op, const SourceLocationReflection *const) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const qint16 op, const SourceLocationReflection *const) const override;
};
}

#endif
