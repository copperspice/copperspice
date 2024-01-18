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

#include "qnonetype_p.h"

using namespace QPatternist;

NoneType::NoneType()
{
}

bool NoneType::itemMatches(const Item &) const
{
   return false;
}

bool NoneType::xdtTypeMatches(const ItemType::Ptr &t) const
{
   return *this == *t;
}

const ItemType &NoneType::operator|(const ItemType &other) const
{
   return other;
}

QString NoneType::displayName(const NamePool::Ptr &) const
{
   return QLatin1String("none");
}

Cardinality NoneType::cardinality() const
{
   return Cardinality::zeroOrMore();
}

ItemType::Ptr NoneType::itemType() const
{
   return ItemType::Ptr(const_cast<NoneType *>(this));
}

bool NoneType::isAtomicType() const
{
   return false;
}

bool NoneType::isNodeType() const
{
   return false;
}

ItemType::Ptr NoneType::atomizedType() const
{
   return BuiltinTypes::xsAnyAtomicType;
}

ItemType::Ptr NoneType::xdtSuperType() const
{
   return BuiltinTypes::item;
}
