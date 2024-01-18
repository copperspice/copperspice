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

#include <limits>

#include "qabstractfloat_p.h"
#include "qanyuri_p.h"
#include "qboolean_p.h"
#include "qdecimal_p.h"
#include "qinteger_p.h"
#include "qatomicstring_p.h"
#include "quntypedatomic_p.h"

#include "qcommonvalues_p.h"

using namespace QPatternist;

// STATIC DATA
const AtomicString::Ptr               CommonValues::EmptyString
(new AtomicString(QLatin1String("")));
const AtomicString::Ptr               CommonValues::TrueString
(new AtomicString(QLatin1String("true")));
const AtomicString::Ptr               CommonValues::FalseString
(new AtomicString(QLatin1String("false")));

const UntypedAtomic::Ptr        CommonValues::UntypedAtomicTrue
(new UntypedAtomic(QLatin1String("true")));
const UntypedAtomic::Ptr        CommonValues::UntypedAtomicFalse
(new UntypedAtomic(QLatin1String("false")));

const AtomicValue::Ptr              CommonValues::BooleanTrue
(new Boolean(true));
const AtomicValue::Ptr              CommonValues::BooleanFalse(new Boolean(false));

const AtomicValue::Ptr               CommonValues::DoubleNaN
(Double::fromValue(std::numeric_limits<xsDouble>::quiet_NaN()));

const AtomicValue::Ptr                CommonValues::FloatNaN
(Float::fromValue(std::numeric_limits<xsFloat>::quiet_NaN()));

const Item                          CommonValues::IntegerZero
(Integer::fromValue(0));

const AtomicValue::Ptr               CommonValues::EmptyAnyURI
(AnyURI::fromValue(QLatin1String("")));

const AtomicValue::Ptr               CommonValues::DoubleOne
(Double::fromValue(1));
const AtomicValue::Ptr                CommonValues::FloatOne
(Float::fromValue(1));
const AtomicValue::Ptr              CommonValues::DecimalOne
(Decimal::fromValue(1));
const Item                          CommonValues::IntegerOne
(Integer::fromValue(1));
const Item                          CommonValues::IntegerOneNegative
(Integer::fromValue(-1));

const AtomicValue::Ptr               CommonValues::DoubleZero
(Double::fromValue(0));
const AtomicValue::Ptr                CommonValues::FloatZero
(Float::fromValue(0));
const AtomicValue::Ptr              CommonValues::DecimalZero
(Decimal::fromValue(0));

const Item::EmptyIterator::Ptr  CommonValues::emptyIterator
(new Item::EmptyIterator());

const AtomicValue::Ptr               CommonValues::NegativeInfDouble
(Double::fromValue(-std::numeric_limits<xsDouble>::infinity()));
const AtomicValue::Ptr               CommonValues::InfDouble
(Double::fromValue(std::numeric_limits<xsDouble>::infinity()));
const AtomicValue::Ptr                CommonValues::NegativeInfFloat
(Float::fromValue(-std::numeric_limits<xsFloat>::infinity()));
const AtomicValue::Ptr                CommonValues::InfFloat
(Float::fromValue(std::numeric_limits<xsFloat>::infinity()));

const DayTimeDuration::Ptr      CommonValues::DayTimeDurationZero
(DayTimeDuration::fromSeconds(0));
const DayTimeDuration::Ptr      CommonValues::YearMonthDurationZero
(YearMonthDuration::fromComponents(true, 0, 0));

