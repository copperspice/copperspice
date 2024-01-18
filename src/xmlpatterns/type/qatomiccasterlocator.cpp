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

#include "qatomiccasterlocator_p.h"

using namespace QPatternist;

#define implCasterVisit(type)                                                                           \
AtomicTypeVisitorResult::Ptr AtomicCasterLocator::visit(const type *,                                   \
                                                        const SourceLocationReflection *const) const    \
{                                                                                                       \
    return AtomicTypeVisitorResult::Ptr();                                                              \
}

implCasterVisit(AnyAtomicType)
implCasterVisit(AnyURIType)
implCasterVisit(Base64BinaryType)
implCasterVisit(BooleanType)
implCasterVisit(DateTimeType)
implCasterVisit(DateType)
implCasterVisit(DayTimeDurationType)
implCasterVisit(DecimalType)
implCasterVisit(DoubleType)
implCasterVisit(DurationType)
implCasterVisit(FloatType)
implCasterVisit(GDayType)
implCasterVisit(GMonthDayType)
implCasterVisit(GMonthType)
implCasterVisit(GYearMonthType)
implCasterVisit(GYearType)
implCasterVisit(HexBinaryType)
implCasterVisit(IntegerType)
implCasterVisit(NOTATIONType)
implCasterVisit(QNameType)
implCasterVisit(StringType)
implCasterVisit(SchemaTimeType)
implCasterVisit(UntypedAtomicType)
implCasterVisit(YearMonthDurationType)

#undef implCasterVisit
