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

#include "qebvtype_p.h"
#include "qgenericsequencetype_p.h"
#include "qnonetype_p.h"

#include "qcommonsequencetypes_p.h"

/* To avoid the static initialization fiasco, we put the builtin types in this compilation unit, since
 * the sequence types depends on them. */
#include "qbuiltintypes.cpp"

using namespace QPatternist;

// STATIC DATA
#define st(var, type, card)                                             \
const SequenceType::Ptr                                                 \
CommonSequenceTypes::var(new GenericSequenceType(BuiltinTypes::type,    \
                                                 Cardinality::card()))

/* Alphabetically. */
st(ExactlyOneAnyURI,                xsAnyURI,               exactlyOne);
st(ExactlyOneAtomicType,            xsAnyAtomicType,        exactlyOne);
st(ExactlyOneAttribute,             attribute,              exactlyOne);
st(ExactlyOneBase64Binary,          xsBase64Binary,         exactlyOne);
st(ExactlyOneBoolean,               xsBoolean,              exactlyOne);
st(ExactlyOneComment,               comment,                exactlyOne);
st(ExactlyOneDateTime,              xsDateTime,             exactlyOne);
st(ExactlyOneDate,                  xsDate,                 exactlyOne);
st(ExactlyOneDayTimeDuration,       xsDayTimeDuration,      exactlyOne);
st(ExactlyOneDecimal,               xsDecimal,              exactlyOne);
st(ExactlyOneDocumentNode,          document,               exactlyOne);
st(OneOrMoreDocumentNodes,          document,               oneOrMore);
st(ExactlyOneDouble,                xsDouble,               exactlyOne);
st(ExactlyOneDuration,              xsDuration,             exactlyOne);
st(ExactlyOneElement,               element,                exactlyOne);
st(ExactlyOneFloat,                 xsFloat,                exactlyOne);
st(ExactlyOneGDay,                  xsGDay,                 exactlyOne);
st(ExactlyOneGMonthDay,             xsGMonthDay,            exactlyOne);
st(ExactlyOneGMonth,                xsGMonth,               exactlyOne);
st(ExactlyOneGYearMonth,            xsGYearMonth,           exactlyOne);
st(ExactlyOneGYear,                 xsGYear,                exactlyOne);
st(ExactlyOneHexBinary,             xsHexBinary,            exactlyOne);
st(ExactlyOneInteger,               xsInteger,              exactlyOne);
st(ExactlyOneItem,                  item,                   exactlyOne);
st(ExactlyOneNCName,                xsNCName,               exactlyOne);
st(ExactlyOneNode,                  node,                   exactlyOne);
st(ExactlyOneNumeric,               numeric,                exactlyOne);
st(ExactlyOneProcessingInstruction, pi,                     exactlyOne);
st(ExactlyOneQName,                 xsQName,                exactlyOne);
st(ExactlyOneString,                xsString,               exactlyOne);
st(ExactlyOneTextNode,              text,                   exactlyOne);
st(ExactlyOneTime,                  xsTime,                 exactlyOne);
st(ExactlyOneUntypedAtomic,         xsUntypedAtomic,        exactlyOne);
st(ExactlyOneYearMonthDuration,     xsYearMonthDuration,    exactlyOne);
st(OneOrMoreItems,                  item,                   oneOrMore);
st(ZeroOrMoreAtomicTypes,           xsAnyAtomicType,        zeroOrMore);
st(ZeroOrMoreElements,              element,                zeroOrMore);
st(ZeroOrMoreIntegers,              xsInteger,              zeroOrMore);
st(ZeroOrMoreItems,                 item,                   zeroOrMore);
st(ZeroOrMoreNodes,                 node,                   zeroOrMore);
st(ZeroOrMoreStrings,               xsString,               zeroOrMore);
st(ZeroOrOneAnyURI,                 xsAnyURI,               zeroOrOne);
st(ZeroOrOneAtomicType,             xsAnyAtomicType,        zeroOrOne);
st(ZeroOrOneBoolean,                xsBoolean,              zeroOrOne);
st(ZeroOrOneDateTime,               xsDateTime,             zeroOrOne);
st(ZeroOrOneDate,                   xsDate,                 zeroOrOne);
st(ZeroOrOneDayTimeDuration,        xsDayTimeDuration,      zeroOrOne);
st(ZeroOrOneDecimal,                xsDecimal,              zeroOrOne);
st(ZeroOrOneDocumentNode,           document,               zeroOrOne);
st(ZeroOrOneDuration,               xsDuration,             zeroOrOne);
st(ZeroOrOneInteger,                xsInteger,              zeroOrOne);
st(ZeroOrOneItem,                   item,                   zeroOrOne);
st(ZeroOrOneNCName,                 xsNCName,               zeroOrOne);
st(ZeroOrOneNode,                   node,                   zeroOrOne);
st(ZeroOrOneNumeric,                numeric,                zeroOrOne);
st(ZeroOrOneQName,                  xsQName,                zeroOrOne);
st(ZeroOrOneString,                 xsString,               zeroOrOne);
st(ZeroOrOneTextNode,               text,                   zeroOrOne);
st(ZeroOrOneTime,                   xsTime,                 zeroOrOne);
st(ZeroOrOneYearMonthDuration,      xsYearMonthDuration,    zeroOrOne);

#undef st

/* Special cases. */
const EmptySequenceType::Ptr    CommonSequenceTypes::Empty  (new EmptySequenceType());
const NoneType::Ptr             CommonSequenceTypes::None   (new NoneType());
const SequenceType::Ptr         CommonSequenceTypes::EBV    (new EBVType());

