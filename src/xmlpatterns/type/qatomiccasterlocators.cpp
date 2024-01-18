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

#include "qabstractfloatcasters_p.h"

#include "qatomiccasterlocators_p.h"

using namespace QPatternist;

#define impl(owner, mather, type)                                                           \
AtomicTypeVisitorResult::Ptr owner::visit(const type *,                                     \
                                          const SourceLocationReflection *const) const      \
{                                                                                           \
    return AtomicTypeVisitorResult::Ptr(new mather());                                      \
}

#define implSelf(owner) impl(To##owner##CasterLocator, SelfToSelfCaster, owner##Type)

/* xs:string */
implSelf(String)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             AnyURIType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             Base64BinaryType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             BooleanType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             DateTimeType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             DateType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             DayTimeDurationType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             DecimalType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             DoubleType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             DurationType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             FloatType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             GDayType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             GMonthDayType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             GMonthType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             GYearMonthType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             GYearType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             HexBinaryType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             IntegerType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             NOTATIONType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             QNameType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             SchemaTimeType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             UntypedAtomicType)
impl(ToStringCasterLocator,             ToStringCaster<TypeString>,             YearMonthDurationType)

/* xs:untypedAtomic */
implSelf(UntypedAtomic)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  AnyURIType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  Base64BinaryType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  BooleanType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  DateTimeType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  DateType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  DayTimeDurationType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  DecimalType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  DoubleType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  DurationType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  FloatType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  GDayType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  GMonthDayType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  GMonthType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  GYearMonthType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  GYearType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  HexBinaryType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  IntegerType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  NOTATIONType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  QNameType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  StringType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  SchemaTimeType)
impl(ToUntypedAtomicCasterLocator,      ToUntypedAtomicCaster,                  YearMonthDurationType)

/* xs:anyURI */
implSelf(AnyURI)
impl(ToAnyURICasterLocator,             ToAnyURICaster,                         StringType)
impl(ToAnyURICasterLocator,             ToAnyURICaster,                         UntypedAtomicType)

/* xs:boolean */
implSelf(Boolean)
impl(ToBooleanCasterLocator,            NumericToBooleanCaster,                 DoubleType)
impl(ToBooleanCasterLocator,            NumericToBooleanCaster,                 FloatType)
impl(ToBooleanCasterLocator,            NumericToBooleanCaster,                 DecimalType)
impl(ToBooleanCasterLocator,            NumericToBooleanCaster,                 IntegerType)
impl(ToBooleanCasterLocator,            StringToBooleanCaster,                  StringType)
impl(ToBooleanCasterLocator,            StringToBooleanCaster,                  UntypedAtomicType)

/* xs:double */
implSelf(Double)
impl(ToDoubleCasterLocator,             BooleanToDoubleCaster,                  BooleanType)
impl(ToDoubleCasterLocator,             NumericToDoubleCaster,                  FloatType)
impl(ToDoubleCasterLocator,             NumericToDoubleCaster,                  DecimalType)
impl(ToDoubleCasterLocator,             NumericToDoubleCaster,                  IntegerType)
impl(ToDoubleCasterLocator,             StringToDoubleCaster,                   StringType)
impl(ToDoubleCasterLocator,             StringToDoubleCaster,                   UntypedAtomicType)

/* xs:float */
implSelf(Float)
impl(ToFloatCasterLocator,              BooleanToFloatCaster,                   BooleanType)
impl(ToFloatCasterLocator,              NumericToFloatCaster,                   DoubleType)
impl(ToFloatCasterLocator,              NumericToFloatCaster,                   DecimalType)
impl(ToFloatCasterLocator,              NumericToFloatCaster,                   IntegerType)
impl(ToFloatCasterLocator,              StringToFloatCaster,                    StringType)
impl(ToFloatCasterLocator,              StringToFloatCaster,                    UntypedAtomicType)

/* xs:decimal */
implSelf(Decimal)
impl(ToDecimalCasterLocator,            BooleanToDecimalCaster,                 BooleanType)
impl(ToDecimalCasterLocator,            NumericToDecimalCaster<false>,          DoubleType)
impl(ToDecimalCasterLocator,            NumericToDecimalCaster<false>,          FloatType)
impl(ToDecimalCasterLocator,            NumericToDecimalCaster<false>,          IntegerType)
impl(ToDecimalCasterLocator,            StringToDecimalCaster,                  StringType)
impl(ToDecimalCasterLocator,            StringToDecimalCaster,                  UntypedAtomicType)

/* xs:integer */
implSelf(Integer)
impl(ToIntegerCasterLocator,            BooleanToIntegerCaster,                 BooleanType)
impl(ToIntegerCasterLocator,            NumericToDecimalCaster<true>,           DoubleType)
impl(ToIntegerCasterLocator,            NumericToDecimalCaster<true>,           FloatType)
impl(ToIntegerCasterLocator,            NumericToDecimalCaster<true>,           DecimalType)
impl(ToIntegerCasterLocator,            StringToIntegerCaster,                  StringType)
impl(ToIntegerCasterLocator,            StringToIntegerCaster,                  UntypedAtomicType)

/* xs:base64binary */
implSelf(Base64Binary)
impl(ToBase64BinaryCasterLocator,       HexBinaryToBase64BinaryCaster,          HexBinaryType)
impl(ToBase64BinaryCasterLocator,       StringToBase64BinaryCaster,             StringType)
impl(ToBase64BinaryCasterLocator,       StringToBase64BinaryCaster,             UntypedAtomicType)

/* xs:hexBinary */
implSelf(HexBinary)
impl(ToHexBinaryCasterLocator,          Base64BinaryToHexBinaryCaster,          Base64BinaryType)
impl(ToHexBinaryCasterLocator,          StringToHexBinaryCaster,                StringType)
impl(ToHexBinaryCasterLocator,          StringToHexBinaryCaster,                UntypedAtomicType)

/* xs:QName */
implSelf(QName)
impl(ToQNameCasterLocator,              ToStringCaster<TypeString>,             StringType)

/* xs:gYear */
implSelf(GYear)
impl(ToGYearCasterLocator,              StringToGYearCaster,                    StringType)
impl(ToGYearCasterLocator,              StringToGYearCaster,                    UntypedAtomicType)
impl(ToGYearCasterLocator,              AbstractDateTimeToGYearCaster,          DateType)
impl(ToGYearCasterLocator,              AbstractDateTimeToGYearCaster,          DateTimeType)

/* xs:gDay */
implSelf(GDay)
impl(ToGDayCasterLocator,               StringToGDayCaster,                     StringType)
impl(ToGDayCasterLocator,               StringToGDayCaster,                     UntypedAtomicType)
impl(ToGDayCasterLocator,               AbstractDateTimeToGDayCaster,           DateType)
impl(ToGDayCasterLocator,               AbstractDateTimeToGDayCaster,           DateTimeType)

/* xs:gMonth */
implSelf(GMonth)
impl(ToGMonthCasterLocator,             StringToGMonthCaster,                   StringType)
impl(ToGMonthCasterLocator,             StringToGMonthCaster,                   UntypedAtomicType)
impl(ToGMonthCasterLocator,             AbstractDateTimeToGMonthCaster,         DateType)
impl(ToGMonthCasterLocator,             AbstractDateTimeToGMonthCaster,         DateTimeType)

/* xs:gYearMonth */
implSelf(GYearMonth)
impl(ToGYearMonthCasterLocator,         StringToGYearMonthCaster,               StringType)
impl(ToGYearMonthCasterLocator,         StringToGYearMonthCaster,               UntypedAtomicType)
impl(ToGYearMonthCasterLocator,         AbstractDateTimeToGYearMonthCaster,     DateType)
impl(ToGYearMonthCasterLocator,         AbstractDateTimeToGYearMonthCaster,     DateTimeType)

/* xs:gMonthDay */
implSelf(GMonthDay)
impl(ToGMonthDayCasterLocator,          StringToGMonthDayCaster,                StringType)
impl(ToGMonthDayCasterLocator,          StringToGMonthDayCaster,                UntypedAtomicType)
impl(ToGMonthDayCasterLocator,          AbstractDateTimeToGMonthDayCaster,      DateType)
impl(ToGMonthDayCasterLocator,          AbstractDateTimeToGMonthDayCaster,      DateTimeType)

/* xs:dateTime */
implSelf(DateTime)
impl(ToDateTimeCasterLocator,           StringToDateTimeCaster,                 StringType)
impl(ToDateTimeCasterLocator,           AbstractDateTimeToDateTimeCaster,       DateType)
impl(ToDateTimeCasterLocator,           StringToDateTimeCaster,                 UntypedAtomicType)

/* xs:time */
implSelf(SchemaTime)
impl(ToSchemaTimeCasterLocator,         StringToTimeCaster,                     StringType)
impl(ToSchemaTimeCasterLocator,         AbstractDateTimeToTimeCaster,           DateTimeType)
impl(ToSchemaTimeCasterLocator,         StringToTimeCaster,                     UntypedAtomicType)

/* xs:date */
implSelf(Date)
impl(ToDateCasterLocator,               StringToDateCaster,                     StringType)
impl(ToDateCasterLocator,               AbstractDateTimeToDateCaster,           DateTimeType)
impl(ToDateCasterLocator,               StringToDateCaster,                     UntypedAtomicType)

/* xs:duration */
implSelf(Duration)
impl(ToDurationCasterLocator,           AbstractDurationToDurationCaster,       DayTimeDurationType)
impl(ToDurationCasterLocator,           AbstractDurationToDurationCaster,       YearMonthDurationType)
impl(ToDurationCasterLocator,           StringToDurationCaster,                 StringType)
impl(ToDurationCasterLocator,           StringToDurationCaster,                 UntypedAtomicType)

/* xs:dayTimeDuration */
implSelf(DayTimeDuration)
impl(ToDayTimeDurationCasterLocator,    AbstractDurationToDayTimeDurationCaster,    DurationType)
impl(ToDayTimeDurationCasterLocator,    AbstractDurationToDayTimeDurationCaster,    YearMonthDurationType)
impl(ToDayTimeDurationCasterLocator,    StringToDayTimeDurationCaster,              StringType)
impl(ToDayTimeDurationCasterLocator,    StringToDayTimeDurationCaster,              UntypedAtomicType)

/* xs:yearMonthDuration */
implSelf(YearMonthDuration)
impl(ToYearMonthDurationCasterLocator,  AbstractDurationToYearMonthDurationCaster,  DayTimeDurationType)
impl(ToYearMonthDurationCasterLocator,  AbstractDurationToYearMonthDurationCaster,  DurationType)
impl(ToYearMonthDurationCasterLocator,  StringToYearMonthDurationCaster,            StringType)
impl(ToYearMonthDurationCasterLocator,  StringToYearMonthDurationCaster,            UntypedAtomicType)

#undef implSelf
#undef impl
