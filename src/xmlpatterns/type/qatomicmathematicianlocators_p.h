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

#ifndef QAtomicMathematicianLocators_P_H
#define QAtomicMathematicianLocators_P_H

#include <qatomicmathematician_p.h>
#include <qatomicmathematicianlocator_p.h>

namespace QPatternist {

class DoubleMathematicianLocator : public AtomicMathematicianLocator
{
   using AtomicMathematicianLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
};

class FloatMathematicianLocator : public AtomicMathematicianLocator
{
   using AtomicMathematicianLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
};

class DecimalMathematicianLocator : public AtomicMathematicianLocator
{
   using AtomicMathematicianLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
};

class IntegerMathematicianLocator : public AtomicMathematicianLocator
{
   using AtomicMathematicianLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
};

class DateMathematicianLocator : public AtomicMathematicianLocator
{
   using AtomicMathematicianLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
};

class SchemaTimeMathematicianLocator : public AtomicMathematicianLocator
{
   using AtomicMathematicianLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
};

class DateTimeMathematicianLocator : public AtomicMathematicianLocator
{
   using AtomicMathematicianLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
};

class DayTimeDurationMathematicianLocator : public AtomicMathematicianLocator
{
   using AtomicMathematicianLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DateType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *, const qint16 op, const SourceLocationReflection *const r) const override;
};


class YearMonthDurationMathematicianLocator : public AtomicMathematicianLocator
{
   using AtomicMathematicianLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DateType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 op, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 op, const SourceLocationReflection *const r) const override;
};
}

#endif
