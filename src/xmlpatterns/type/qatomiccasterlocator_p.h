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

#ifndef QAtomicCasterLocator_P_H
#define QAtomicCasterLocator_P_H

#include <qatomictypedispatch_p.h>

namespace QPatternist {

class AtomicCasterLocator : public AtomicTypeVisitor
{
 public:
   typedef QExplicitlySharedDataPointer<AtomicCasterLocator> Ptr;

   AtomicTypeVisitorResult::Ptr visit(const AnyAtomicType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const GDayType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const GYearType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const NOTATIONType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const QNameType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const SourceLocationReflection *const reflection) const override;
};

}

#endif
