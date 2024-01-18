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

#include "qanysimpletype_p.h"

using namespace QPatternist;

AnySimpleType::AnySimpleType()
{
}

AnySimpleType::~AnySimpleType()
{
}

QXmlName AnySimpleType::name(const NamePool::Ptr &np) const
{
   return np->allocateQName(StandardNamespaces::xs, QLatin1String("anySimpleType"));
}

QString AnySimpleType::displayName(const NamePool::Ptr &np) const
{
   return np->displayName(name(np));
}

SchemaType::Ptr AnySimpleType::wxsSuperType() const
{
   return BuiltinTypes::xsAnyType;
}

SchemaType::TypeCategory AnySimpleType::category() const
{
   return None;
}

SchemaType::DerivationMethod AnySimpleType::derivationMethod() const
{
   return DerivationRestriction;
}

bool AnySimpleType::isSimpleType() const
{
   return true;
}

bool AnySimpleType::isComplexType() const
{
   return false;
}
