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

#ifndef QAtomicCasterLocators_P_H
#define QAtomicCasterLocators_P_H

#include <qatomiccasterlocator_p.h>
#include <qatomiccasters_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class ToStringCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const NOTATIONType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const SourceLocationReflection *const r) const;
};


class ToUntypedAtomicCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const NOTATIONType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const SourceLocationReflection *const r) const;
};

class ToAnyURICasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
         const SourceLocationReflection *const r) const;
};

class ToBooleanCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
};

class ToDoubleCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
};


class ToFloatCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
};

class ToDecimalCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
};

class ToIntegerCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
};

class ToBase64BinaryCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
};

class ToHexBinaryCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
};

class ToQNameCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
};

class ToGYearCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
};

class ToGDayCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
};

class ToGMonthCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
};

class ToGYearMonthCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
};

class ToGMonthDayCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
};

class ToDateTimeCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
};

class ToDateCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
};

class ToSchemaTimeCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
};

class ToDurationCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const SourceLocationReflection *const r) const;
};

class ToDayTimeDurationCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const SourceLocationReflection *const r) const;
};

class ToYearMonthDurationCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;
   virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const;
   virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const SourceLocationReflection *const r) const;
};

template<TypeOfDerivedInteger type>
class ToDerivedIntegerCasterLocator : public ToIntegerCasterLocator
{
 public:
   using ToIntegerCasterLocator::visit;

   virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new BooleanToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new StringToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new StringToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new StringToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeByte> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeInt> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeLong> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeNegativeInteger> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeNonNegativeInteger> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeNonPositiveInteger> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypePositiveInteger> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeShort> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeUnsignedByte> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeUnsignedInt> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeUnsignedLong> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeUnsignedShort> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }
};

template<TypeOfDerivedString type>
class ToDerivedStringCasterLocator : public ToStringCasterLocator
{
 public:
   using ToStringCasterLocator::visit;

   virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   // TODO TypeString not handled
   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeNormalizedString> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeToken> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeLanguage> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeNMTOKEN> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeName> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeNCName> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeID> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeIDREF> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeENTITY> *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *,
         const SourceLocationReflection *const r) const {
      Q_UNUSED(r);
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }
};
}

QT_END_NAMESPACE

#endif
