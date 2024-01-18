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

#include "qanytype_p.h"

using namespace QPatternist;

AnyType::~AnyType()
{
}

bool AnyType::wxsTypeMatches(const SchemaType::Ptr &other) const
{
   if (other) {
      return this == other.data() ? true : wxsTypeMatches(other->wxsSuperType());
   } else {
      return false;
   }
}

bool AnyType::isAbstract() const
{
   return false;
}

QXmlName AnyType::name(const NamePool::Ptr &np) const
{
   return np->allocateQName(StandardNamespaces::xs, QLatin1String("anyType"));
}

QString AnyType::displayName(const NamePool::Ptr &) const
{
   /* A bit faster than calling name()->displayName() */
   return QLatin1String("xs:anyType");
}

SchemaType::Ptr AnyType::wxsSuperType() const
{
   return SchemaType::Ptr();
}

SchemaType::TypeCategory AnyType::category() const
{
   return None;
}

bool AnyType::isComplexType() const
{
   return true;
}

SchemaType::DerivationMethod AnyType::derivationMethod() const
{
   return NoDerivation;
}

SchemaType::DerivationConstraints AnyType::derivationConstraints() const
{
   return SchemaType::DerivationConstraints();
}

