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

#ifndef QAtomicMathematicianLocator_P_H
#define QAtomicMathematicianLocator_P_H

#include <qatomictypedispatch_p.h>

namespace QPatternist {

class AtomicMathematicianLocator : public ParameterizedAtomicTypeVisitor
{
 public:
   typedef QExplicitlySharedDataPointer<AtomicMathematicianLocator> Ptr;

   inline AtomicMathematicianLocator() {
   }

   virtual ~AtomicMathematicianLocator();

   AtomicTypeVisitorResult::Ptr visit(const AnyAtomicType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const AnyURIType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const Base64BinaryType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const BooleanType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const DateTimeType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const DateType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const DayTimeDurationType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const DecimalType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const DoubleType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const DurationType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const FloatType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const GDayType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const GMonthDayType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const GMonthType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const GYearMonthType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const GYearType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const HexBinaryType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const IntegerType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const NOTATIONType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const QNameType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const StringType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const SchemaTimeType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const UntypedAtomicType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;

   AtomicTypeVisitorResult::Ptr visit(const YearMonthDurationType *,
         const qint16 op, const SourceLocationReflection *const reflection) const override;
};

}

#endif
