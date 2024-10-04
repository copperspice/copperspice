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

#ifndef QCommonSequenceTypes_P_H
#define QCommonSequenceTypes_P_H

#include <qemptysequencetype_p.h>
#include <qnonetype_p.h>

namespace QPatternist {

class CommonSequenceTypes
{
 public:
   static const SequenceType::Ptr ZeroOrOneAtomicType;
   static const SequenceType::Ptr ExactlyOneAtomicType;
   static const SequenceType::Ptr ZeroOrMoreAtomicTypes;
   static const SequenceType::Ptr ExactlyOneItem;
   static const SequenceType::Ptr ZeroOrMoreItems;
   static const SequenceType::Ptr ZeroOrOneItem;
   static const SequenceType::Ptr OneOrMoreItems;

   static const EmptySequenceType::Ptr Empty;
   static const NoneType::Ptr None;

   static const SequenceType::Ptr ExactlyOneAnyURI;
   static const SequenceType::Ptr ExactlyOneBoolean;
   static const SequenceType::Ptr ZeroOrOneBoolean;
   static const SequenceType::Ptr ExactlyOneUntypedAtomic;
   static const SequenceType::Ptr ExactlyOneInteger;
   static const SequenceType::Ptr ZeroOrOneInteger;
   static const SequenceType::Ptr ZeroOrOneDecimal;
   static const SequenceType::Ptr ZeroOrMoreIntegers;
   static const SequenceType::Ptr ExactlyOneDouble;
   static const SequenceType::Ptr ExactlyOneDecimal;
   static const SequenceType::Ptr ExactlyOneFloat;
   static const SequenceType::Ptr ExactlyOneQName;
   static const SequenceType::Ptr ExactlyOneString;
   static const SequenceType::Ptr ZeroOrOneString;
   static const SequenceType::Ptr ZeroOrMoreStrings;
   static const SequenceType::Ptr ZeroOrOneNCName;
   static const SequenceType::Ptr ExactlyOneNCName;
   static const SequenceType::Ptr ZeroOrOneQName;

   static const SequenceType::Ptr ZeroOrOneNumeric;
   static const SequenceType::Ptr ExactlyOneNumeric;
   static const SequenceType::Ptr ZeroOrOneNode;
   static const SequenceType::Ptr ExactlyOneNode;
   static const SequenceType::Ptr ZeroOrMoreNodes;

   static const SequenceType::Ptr ExactlyOneElement;
   static const SequenceType::Ptr ExactlyOneProcessingInstruction;
   static const SequenceType::Ptr ExactlyOneAttribute;
   static const SequenceType::Ptr ExactlyOneTextNode;

   static const SequenceType::Ptr ZeroOrOneTextNode;
   static const SequenceType::Ptr ExactlyOneComment;
   static const SequenceType::Ptr ZeroOrMoreElements;
   static const SequenceType::Ptr ZeroOrOneDocumentNode;
   static const SequenceType::Ptr ExactlyOneDocumentNode;
   static const SequenceType::Ptr OneOrMoreDocumentNodes;
   static const SequenceType::Ptr EBV;
   static const SequenceType::Ptr ZeroOrOneAnyURI;
   static const SequenceType::Ptr ExactlyOneHexBinary;
   static const SequenceType::Ptr ExactlyOneBase64Binary;
   static const SequenceType::Ptr ExactlyOneDate;
   static const SequenceType::Ptr ExactlyOneDateTime;
   static const SequenceType::Ptr ExactlyOneDayTimeDuration;
   static const SequenceType::Ptr ExactlyOneDuration;
   static const SequenceType::Ptr ExactlyOneGDay;
   static const SequenceType::Ptr ExactlyOneGMonth;
   static const SequenceType::Ptr ExactlyOneGMonthDay;
   static const SequenceType::Ptr ExactlyOneGYear;
   static const SequenceType::Ptr ExactlyOneGYearMonth;
   static const SequenceType::Ptr ExactlyOneYearMonthDuration;
   static const SequenceType::Ptr ExactlyOneTime;

   static const SequenceType::Ptr ZeroOrOneDate;
   static const SequenceType::Ptr ZeroOrOneDateTime;
   static const SequenceType::Ptr ZeroOrOneDayTimeDuration;
   static const SequenceType::Ptr ZeroOrOneDuration;
   static const SequenceType::Ptr ZeroOrOneTime;
   static const SequenceType::Ptr ZeroOrOneYearMonthDuration;

 private:
   inline CommonSequenceTypes();

   CommonSequenceTypes(const CommonSequenceTypes &) = delete;
   CommonSequenceTypes &operator=(const CommonSequenceTypes &) = delete;
};

}

#endif

