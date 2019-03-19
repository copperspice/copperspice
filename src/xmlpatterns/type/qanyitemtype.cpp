/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qanyitemtype_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

AnyItemType::AnyItemType()
{
}

bool AnyItemType::itemMatches(const Item &) const
{
   return true;
}

bool AnyItemType::xdtTypeMatches(const ItemType::Ptr &) const
{
   return true;
}

QString AnyItemType::displayName(const NamePool::Ptr &) const
{
   return QLatin1String("item()");
}

ItemType::Ptr AnyItemType::xdtSuperType() const
{
   return ItemType::Ptr();
}

bool AnyItemType::isNodeType() const
{
   return false;
}

bool AnyItemType::isAtomicType() const
{
   return false;
}

ItemType::Ptr AnyItemType::atomizedType() const
{
   return BuiltinTypes::xsAnyAtomicType;
}

QT_END_NAMESPACE
