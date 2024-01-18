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
#include "qitem_p.h"

#include "qanynodetype_p.h"

using namespace QPatternist;

bool AnyNodeType::xdtTypeMatches(const ItemType::Ptr &other) const
{
   return other->isNodeType();
}

bool AnyNodeType::itemMatches(const Item &item) const
{
   return item.isNode();
}

ItemType::Ptr AnyNodeType::atomizedType() const
{
   return BuiltinTypes::xsAnyAtomicType;
}

QString AnyNodeType::displayName(const NamePool::Ptr &) const
{
   return QLatin1String("node()");
}

ItemType::Ptr AnyNodeType::xdtSuperType() const
{
   return BuiltinTypes::item;
}

bool AnyNodeType::isNodeType() const
{
   return true;
}

bool AnyNodeType::isAtomicType() const
{
   return false;
}

QXmlNodeModelIndex::NodeKind AnyNodeType::nodeKind() const
{
   /* node() is an abstract type, so we don't have a value for it in
    * QXmlNodeModelIndex::NodeKind. */
   return QXmlNodeModelIndex::NodeKind(0);
}

PatternPriority AnyNodeType::patternPriority() const
{
   return -0.5;
}
