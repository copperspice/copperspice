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

#include "quntyped_p.h"

using namespace QPatternist;

Untyped::Untyped()
{
}

SchemaType::Ptr Untyped::wxsSuperType() const
{
   return BuiltinTypes::xsAnyType;
}

QXmlName Untyped::name(const NamePool::Ptr &np) const
{
   return np->allocateQName(StandardNamespaces::xs, QLatin1String("untyped"));
}

ItemType::Ptr Untyped::atomizedType() const
{
   return BuiltinTypes::xsUntypedAtomic;
}

SchemaType::TypeCategory Untyped::category() const
{
   return SchemaType::ComplexType;
}

SchemaType::DerivationMethod Untyped::derivationMethod() const
{
   return NoDerivation;
}
