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

#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qitem_p.h"

#include "qebvtype_p.h"

using namespace QPatternist;

EBVType::EBVType()
{
}

bool EBVType::itemMatches(const Item &item) const
{
   if (item.isNode()) {
      return false;
   }

   return BuiltinTypes::xsBoolean->itemMatches(item)       ||
          BuiltinTypes::numeric->itemMatches(item)         ||
          BuiltinTypes::xsString->itemMatches(item)        ||
          BuiltinTypes::xsAnyURI->itemMatches(item)        ||
          CommonSequenceTypes::Empty->itemMatches(item)    ||
          BuiltinTypes::xsUntypedAtomic->itemMatches(item);
}

bool EBVType::xdtTypeMatches(const ItemType::Ptr &t) const
{
   return BuiltinTypes::node->xdtTypeMatches(t)            ||
          BuiltinTypes::xsBoolean->xdtTypeMatches(t)       ||
          BuiltinTypes::numeric->xdtTypeMatches(t)         ||
          BuiltinTypes::xsString->xdtTypeMatches(t)        ||
          BuiltinTypes::xsAnyURI->xdtTypeMatches(t)        ||
          *CommonSequenceTypes::Empty == *t                ||
          BuiltinTypes::xsUntypedAtomic->xdtTypeMatches(t) ||
          /* Item & xs:anyAtomicType is ok, we do the checking at runtime. */
          *BuiltinTypes::item == *t                        ||
          *BuiltinTypes::xsAnyAtomicType == *t;
}

QString EBVType::displayName(const NamePool::Ptr &) const
{
   /* Some QName-lexical is not used here because it makes little sense
    * for this strange type. Instead the operand type of the fn:boolean()'s
    * argument is used. */
   return QLatin1String("item()*(: EBV extractable type :)");
}

Cardinality EBVType::cardinality() const
{
   return Cardinality::zeroOrMore();
}

ItemType::Ptr EBVType::xdtSuperType() const
{
   return BuiltinTypes::item;
}

ItemType::Ptr EBVType::itemType() const
{
   return ItemType::Ptr(const_cast<EBVType *>(this));
}

bool EBVType::isAtomicType() const
{
   return false;
}

bool EBVType::isNodeType() const
{
   return true;
}

ItemType::Ptr EBVType::atomizedType() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return AtomicType::Ptr();
}
