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

#include "qitem_p.h"
#include "qbuiltintypes_p.h"
#include "qitem_p.h"
#include "qschematypefactory_p.h"
#include "qxmlname.h"

#include "qatomictype_p.h"

using namespace QPatternist;

AtomicType::AtomicType()
{
}

AtomicType::~AtomicType()
{
}

bool AtomicType::xdtTypeMatches(const ItemType::Ptr &other) const
{
   if (other->isAtomicType()) {
      if (*other == *this) {
         return true;
      } else {
         return xdtTypeMatches(other->xdtSuperType());
      }
   } else {
      return false;
   }
}

bool AtomicType::itemMatches(const Item &item) const
{
   Q_ASSERT(item);
   if (item.isNode()) {
      return false;
   } else {
      const SchemaType::Ptr t(static_cast<AtomicType *>(item.type().data()));
      return wxsTypeMatches(t);
   }
}

ItemType::Ptr AtomicType::atomizedType() const
{
   return AtomicType::Ptr(const_cast<AtomicType *>(this));
}

QString AtomicType::displayName(const NamePool::Ptr &) const
{
   /* A bit faster than calling name()->displayName() */
   return QLatin1String("xs:anyAtomicType");
}

bool AtomicType::isNodeType() const
{
   return false;
}

bool AtomicType::isAtomicType() const
{
   return true;
}

SchemaType::TypeCategory AtomicType::category() const
{
   return SimpleTypeAtomic;
}

SchemaType::DerivationMethod AtomicType::derivationMethod() const
{
   return DerivationRestriction;
}
