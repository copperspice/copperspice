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

#include "qatomicmathematicianlocator_p.h"

using namespace QPatternist;

AtomicMathematicianLocator::~AtomicMathematicianLocator()
{
}

#define implVisit(type)                                                                                     \
AtomicTypeVisitorResult::Ptr AtomicMathematicianLocator::visit(const type *, const qint16,                  \
                                                               const SourceLocationReflection *const) const \
{                                                                                                           \
    return AtomicTypeVisitorResult::Ptr();                                                                  \
}

implVisit(AnyAtomicType)
implVisit(AnyURIType)
implVisit(Base64BinaryType)
implVisit(BooleanType)
implVisit(DateTimeType)
implVisit(DateType)
implVisit(DayTimeDurationType)
implVisit(DecimalType)
implVisit(DoubleType)
implVisit(DurationType)
implVisit(FloatType)
implVisit(GDayType)
implVisit(GMonthDayType)
implVisit(GMonthType)
implVisit(GYearMonthType)
implVisit(GYearType)
implVisit(HexBinaryType)
implVisit(IntegerType)
implVisit(NOTATIONType)
implVisit(QNameType)
implVisit(StringType)
implVisit(SchemaTimeType)
implVisit(UntypedAtomicType)
implVisit(YearMonthDurationType)
#undef implVisit

