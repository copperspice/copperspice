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

#include "qatomictype_p.h"
#include "qbuiltintypes_p.h"
#include "qitem_p.h"
#include "qschematype_p.h"

#include "qnumerictype_p.h"

using namespace QPatternist;

NumericType::NumericType()
{
}

NumericType::~NumericType()
{
}

bool NumericType::itemMatches(const Item &item) const
{
   if (item.isNode()) {
      return false;
   }

   return BuiltinTypes::xsDouble->itemMatches(item)    ||
          BuiltinTypes::xsDecimal->itemMatches(item)   ||
          BuiltinTypes::xsFloat->itemMatches(item);
}

bool NumericType::xdtTypeMatches(const ItemType::Ptr &t) const
{
   return BuiltinTypes::xsDouble->xdtTypeMatches(t)    ||
          BuiltinTypes::xsDecimal->xdtTypeMatches(t)   ||
          BuiltinTypes::xsFloat->xdtTypeMatches(t)     ||
          *t == *this; /* If it's NumericType */
}

QString NumericType::displayName(const NamePool::Ptr &) const
{
   return QLatin1String("numeric");
}

SchemaType::Ptr NumericType::wxsSuperType() const
{
   return BuiltinTypes::xsAnyAtomicType;
}

ItemType::Ptr NumericType::xdtSuperType() const
{
   return BuiltinTypes::xsAnyAtomicType;
}

bool NumericType::isAbstract() const
{
   return true;
}

bool NumericType::isNodeType() const
{
   return false;
}

bool NumericType::isAtomicType() const
{
   return true;
}

ItemType::Ptr NumericType::atomizedType() const
{
   return AtomicType::Ptr();
}

AtomicTypeVisitorResult::Ptr NumericType::accept(const AtomicTypeVisitor::Ptr &,
      const SourceLocationReflection *const) const
{
   return AtomicTypeVisitorResult::Ptr();
}

AtomicTypeVisitorResult::Ptr NumericType::accept(const ParameterizedAtomicTypeVisitor::Ptr &,
      const qint16,
      const SourceLocationReflection *const) const
{
   return AtomicTypeVisitorResult::Ptr();
}

AtomicComparatorLocator::Ptr NumericType::comparatorLocator() const
{
   return AtomicComparatorLocator::Ptr();
}

AtomicMathematicianLocator::Ptr NumericType::mathematicianLocator() const
{
   return AtomicMathematicianLocator::Ptr();
}

AtomicCasterLocator::Ptr NumericType::casterLocator() const
{
   return AtomicCasterLocator::Ptr();
}
