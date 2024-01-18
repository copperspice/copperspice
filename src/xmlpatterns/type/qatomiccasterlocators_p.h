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

#ifndef QAtomicCasterLocators_P_H
#define QAtomicCasterLocators_P_H

#include <qatomiccasterlocator_p.h>
#include <qatomiccasters_p.h>

namespace QPatternist {
class ToStringCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const GDayType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const GYearType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const NOTATIONType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const QNameType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const r) const override;

   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const SourceLocationReflection *const r) const override;

};

class ToUntypedAtomicCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const AnyURIType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const BooleanType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DateType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DurationType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const GDayType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const GMonthType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const GYearType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const NOTATIONType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const QNameType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const SourceLocationReflection *const r) const override;
};

class ToAnyURICasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const AnyURIType *, const SourceLocationReflection *const r) const override;
};

class ToBooleanCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const BooleanType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
};

class ToDoubleCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const BooleanType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
};


class ToFloatCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const BooleanType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
};

class ToDecimalCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const BooleanType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
};

class ToIntegerCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const BooleanType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
};

class ToBase64BinaryCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
};

class ToHexBinaryCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
};

class ToQNameCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const QNameType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
};

class ToGYearCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DateType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const GYearType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
};

class ToGDayCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DateType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const GDayType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
};

class ToGMonthCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DateType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const GMonthType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
};

class ToGYearMonthCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DateType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
};

class ToGMonthDayCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DateType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
};

class ToDateTimeCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DateType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
};

class ToDateCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DateType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
};

class ToSchemaTimeCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
};

class ToDurationCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DurationType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const SourceLocationReflection *const r) const override;
};

class ToDayTimeDurationCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DurationType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const SourceLocationReflection *const r) const override;
};

class ToYearMonthDurationCasterLocator : public AtomicCasterLocator
{
 public:
   using AtomicCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const DurationType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override;
   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const SourceLocationReflection *const r) const override;
};

template<TypeOfDerivedInteger type>
class ToDerivedIntegerCasterLocator : public ToIntegerCasterLocator
{
 public:
   using ToIntegerCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const BooleanType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new BooleanToDerivedIntegerCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new StringToDerivedIntegerCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const AnyURIType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new StringToDerivedIntegerCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new StringToDerivedIntegerCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeByte> *,
         const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeInt> *,
         const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeLong> *,
         const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeNegativeInteger> *,
         const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeNonNegativeInteger> *,
         const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeNonPositiveInteger> *,
         const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypePositiveInteger> *,
         const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeShort> *,
         const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeUnsignedByte> *,
         const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeUnsignedInt> *,
         const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeUnsignedLong> *,
         const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedIntegerType<TypeUnsignedShort> *,
         const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new NumericToDerivedIntegerCaster<type>());
   }
};

template<TypeOfDerivedString type>
class ToDerivedStringCasterLocator : public ToStringCasterLocator
{
 public:
   using ToStringCasterLocator::visit;

   AtomicTypeVisitorResult::Ptr visit(const BooleanType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const StringType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const AnyURIType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   // TODO TypeString not handled
   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeNormalizedString> *, const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeToken> *, const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeLanguage> *, const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeNMTOKEN> *, const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeName> *, const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeNCName> *, const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeID> *, const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeIDREF> *, const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   virtual AtomicTypeVisitorResult::Ptr visit(const DerivedStringType<TypeENTITY> *, const SourceLocationReflection *const r) const {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const DateType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const FloatType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const DurationType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const GYearType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const GDayType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const GMonthType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }

   AtomicTypeVisitorResult::Ptr visit(const QNameType *, const SourceLocationReflection *const r) const override {
      (void) r;
      return AtomicTypeVisitorResult::Ptr(new AnyToDerivedStringCaster<type>());
   }
};
}

#endif
