/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qatomiccomparatorlocator_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

AtomicComparatorLocator::AtomicComparatorLocator()
{
}

AtomicComparatorLocator::~AtomicComparatorLocator()
{
}

#define implCompVisit(type)                                                             \
AtomicTypeVisitorResult::Ptr                                                            \
AtomicComparatorLocator::visit(const type *,                                            \
                               const qint16,                                            \
                               const SourceLocationReflection *const) const             \
{                                                                                       \
    return AtomicTypeVisitorResult::Ptr();                                              \
}

implCompVisit(AnyAtomicType)
implCompVisit(AnyURIType)
implCompVisit(Base64BinaryType)
implCompVisit(BooleanType)
implCompVisit(DateTimeType)
implCompVisit(DateType)
implCompVisit(DayTimeDurationType)
implCompVisit(DecimalType)
implCompVisit(DoubleType)
implCompVisit(DurationType)
implCompVisit(FloatType)
implCompVisit(GDayType)
implCompVisit(GMonthDayType)
implCompVisit(GMonthType)
implCompVisit(GYearMonthType)
implCompVisit(GYearType)
implCompVisit(HexBinaryType)
implCompVisit(IntegerType)
implCompVisit(NOTATIONType)
implCompVisit(QNameType)
implCompVisit(StringType)
implCompVisit(SchemaTimeType)
implCompVisit(UntypedAtomicType)
implCompVisit(YearMonthDurationType)
#undef implCompVisit

QT_END_NAMESPACE
