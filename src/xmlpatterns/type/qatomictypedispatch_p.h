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

#ifndef QAtomicTypeDispatch_P_H
#define QAtomicTypeDispatch_P_H

#include <QSharedData>

namespace QPatternist {

class AnyAtomicType;
class AnyURIType;
class Base64BinaryType;
class BooleanType;
class DateTimeType;
class DateType;
class DayTimeDurationType;
class DecimalType;
class DoubleType;
class DurationType;
class FloatType;
class GDayType;
class GMonthDayType;
class GMonthType;
class GYearMonthType;
class GYearType;
class HexBinaryType;
class IntegerType;
class NOTATIONType;
class QNameType;
class SourceLocationReflection;
class StringType;
class SchemaTimeType;
class UntypedAtomicType;
class YearMonthDurationType;

enum TypeOfDerivedInteger {
   TypeByte,
   TypeInt,
   TypeLong,
   TypeNegativeInteger,
   TypeNonNegativeInteger,
   TypeNonPositiveInteger,
   TypePositiveInteger,
   TypeShort,
   TypeUnsignedByte,
   TypeUnsignedInt,
   TypeUnsignedLong,
   TypeUnsignedShort
};

template<TypeOfDerivedInteger DerivedType> class DerivedIntegerType;

enum TypeOfDerivedString {
   TypeString,
   TypeNormalizedString,
   TypeToken,
   TypeLanguage,
   TypeNMTOKEN,
   TypeName,
   TypeNCName,
   TypeID,
   TypeIDREF,
   TypeENTITY
};

template<TypeOfDerivedString DerivedType> class DerivedStringType;


class AtomicTypeVisitorResult : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<AtomicTypeVisitorResult> Ptr;
   AtomicTypeVisitorResult() {}
   virtual ~AtomicTypeVisitorResult() {}
};


class AtomicTypeVisitor : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<AtomicTypeVisitor> Ptr;
   virtual ~AtomicTypeVisitor() {}

   virtual AtomicTypeVisitorResult::Ptr visit(const AnyAtomicType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const NOTATIONType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const SourceLocationReflection *const reflection) const = 0;
};

class ParameterizedAtomicTypeVisitor : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<ParameterizedAtomicTypeVisitor> Ptr;
   virtual ~ParameterizedAtomicTypeVisitor() {}

   virtual AtomicTypeVisitorResult::Ptr visit(const AnyAtomicType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const AnyURIType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const BooleanType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateTimeType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const DateType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const DecimalType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const DoubleType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const DurationType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const FloatType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const GDayType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const GMonthType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const GYearType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const IntegerType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const NOTATIONType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const QNameType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const StringType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
   virtual AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *, const qint16 param,
         const SourceLocationReflection *const reflection) const = 0;
};

}

#endif
