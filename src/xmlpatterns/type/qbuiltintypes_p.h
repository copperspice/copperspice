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

#ifndef QBuiltinTypes_P_H
#define QBuiltinTypes_P_H

#include <qanynodetype_p.h>
#include <qanysimpletype_p.h>
#include <qanytype_p.h>
#include <qbuiltinatomictype_p.h>
#include <qitemtype_p.h>
#include <qnumerictype_p.h>
#include <quntyped_p.h>

namespace QPatternist {

class BuiltinTypes
{
 public:
   static const SchemaType::Ptr        xsAnyType;
   static const SchemaType::Ptr        xsAnySimpleType;
   static const SchemaType::Ptr        xsUntyped;

   static const AtomicType::Ptr        xsAnyAtomicType;
   static const AtomicType::Ptr        xsUntypedAtomic;
   static const AtomicType::Ptr        xsDateTime;
   static const AtomicType::Ptr        xsDate;
   static const AtomicType::Ptr        xsTime;
   static const AtomicType::Ptr        xsDuration;
   static const AtomicType::Ptr        xsYearMonthDuration;
   static const AtomicType::Ptr        xsDayTimeDuration;

   /**
    * An artificial type for implementation purposes
    * that represents the XPath type @c numeric.
    */
   static const AtomicType::Ptr        numeric;
   static const AtomicType::Ptr        xsFloat;
   static const AtomicType::Ptr        xsDouble;
   static const AtomicType::Ptr        xsInteger;
   static const AtomicType::Ptr        xsDecimal;
   static const AtomicType::Ptr        xsNonPositiveInteger;
   static const AtomicType::Ptr        xsNegativeInteger;
   static const AtomicType::Ptr        xsLong;
   static const AtomicType::Ptr        xsInt;
   static const AtomicType::Ptr        xsShort;
   static const AtomicType::Ptr        xsByte;
   static const AtomicType::Ptr        xsNonNegativeInteger;
   static const AtomicType::Ptr        xsUnsignedLong;
   static const AtomicType::Ptr        xsUnsignedInt;
   static const AtomicType::Ptr        xsUnsignedShort;
   static const AtomicType::Ptr        xsUnsignedByte;
   static const AtomicType::Ptr        xsPositiveInteger;


   static const AtomicType::Ptr        xsGYearMonth;
   static const AtomicType::Ptr        xsGYear;
   static const AtomicType::Ptr        xsGMonthDay;
   static const AtomicType::Ptr        xsGDay;
   static const AtomicType::Ptr        xsGMonth;

   static const AtomicType::Ptr        xsBoolean;

   static const AtomicType::Ptr        xsBase64Binary;
   static const AtomicType::Ptr        xsHexBinary;
   static const AtomicType::Ptr        xsAnyURI;
   static const AtomicType::Ptr        xsQName;
   static const AtomicType::Ptr        xsString;
   static const AtomicType::Ptr        xsNormalizedString;
   static const AtomicType::Ptr        xsToken;
   static const AtomicType::Ptr        xsLanguage;
   static const AtomicType::Ptr        xsNMTOKEN;
   static const AtomicType::Ptr        xsName;
   static const AtomicType::Ptr        xsNCName;
   static const AtomicType::Ptr        xsID;
   static const AtomicType::Ptr        xsIDREF;
   static const AtomicType::Ptr        xsENTITY;

   static const AtomicType::Ptr        xsNOTATION;
   static const ItemType::Ptr          item;

   static const AnyNodeType::Ptr       node;

   /**
    * When the node test node() is used without axes in a pattern in
    * XSL-T, it doesn't match document nodes. See 5.5.3 The Meaning of a
    * Pattern.
    *
    * This node test does that.
    */
   static const ItemType::Ptr          xsltNodeTest;

   static const ItemType::Ptr          attribute;
   static const ItemType::Ptr          comment;
   static const ItemType::Ptr          document;
   static const ItemType::Ptr          element;
   static const ItemType::Ptr          pi;
   static const ItemType::Ptr          text;

 private:
   /**
    * The constructor is protected because this class is not meant to be instantiated,
    * but should only be used via its static const members.
    */
   BuiltinTypes();

   BuiltinTypes(const BuiltinTypes &) = delete;
   BuiltinTypes &operator=(const BuiltinTypes &) = delete;
};

}

#endif

