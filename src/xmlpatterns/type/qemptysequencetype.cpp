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

#include "qemptysequencetype_p.h"

using namespace QPatternist;

EmptySequenceType::EmptySequenceType()
{
}

bool EmptySequenceType::xdtTypeMatches(const ItemType::Ptr &other) const
{
   return *other == *this ||
          CommonSequenceTypes::None->xdtTypeMatches(other);
}

bool EmptySequenceType::itemMatches(const Item &) const
{
   return false;
}

QString EmptySequenceType::displayName(const NamePool::Ptr &) const
{
   return QLatin1String("empty-sequence()");
}

ItemType::Ptr EmptySequenceType::xdtSuperType() const
{
   return BuiltinTypes::item;
}

Cardinality EmptySequenceType::cardinality() const
{
   return Cardinality::empty();
}

ItemType::Ptr EmptySequenceType::itemType() const
{
   return ItemType::Ptr(const_cast<EmptySequenceType *>(this));
}

bool EmptySequenceType::isNodeType() const
{
   return false;
}

bool EmptySequenceType::isAtomicType() const
{
   return false;
}

ItemType::Ptr EmptySequenceType::atomizedType() const
{
   return BuiltinTypes::xsAnyAtomicType;
}
