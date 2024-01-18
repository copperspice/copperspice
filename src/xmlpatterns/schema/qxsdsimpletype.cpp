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

#include "qxsdsimpletype_p.h"

using namespace QPatternist;

QString XsdSimpleType::displayName(const NamePool::Ptr &np) const
{
   return np->displayName(name(np));
}

void XsdSimpleType::setWxsSuperType(const SchemaType::Ptr &type)
{
   m_superType = type;
}

SchemaType::Ptr XsdSimpleType::wxsSuperType() const
{
   return m_superType;
}

void XsdSimpleType::setContext(const NamedSchemaComponent::Ptr &component)
{
   m_context = component.data();
}

NamedSchemaComponent::Ptr XsdSimpleType::context() const
{
   return NamedSchemaComponent::Ptr(m_context);
}

void XsdSimpleType::setPrimitiveType(const AnySimpleType::Ptr &type)
{
   m_primitiveType = type;
}

AnySimpleType::Ptr XsdSimpleType::primitiveType() const
{
   return m_primitiveType;
}

void XsdSimpleType::setItemType(const AnySimpleType::Ptr &type)
{
   m_itemType = type;
}

AnySimpleType::Ptr XsdSimpleType::itemType() const
{
   return m_itemType;
}

void XsdSimpleType::setMemberTypes(const AnySimpleType::List &types)
{
   m_memberTypes = types;
}

AnySimpleType::List XsdSimpleType::memberTypes() const
{
   return m_memberTypes;
}

void XsdSimpleType::setFacets(const XsdFacet::Hash &facets)
{
   m_facets = facets;
}

XsdFacet::Hash XsdSimpleType::facets() const
{
   return m_facets;
}

void XsdSimpleType::setCategory(TypeCategory category)
{
   m_typeCategory = category;
}

XsdSimpleType::TypeCategory XsdSimpleType::category() const
{
   return m_typeCategory;
}

void XsdSimpleType::setDerivationMethod(DerivationMethod method)
{
   m_derivationMethod = method;
}

XsdSimpleType::DerivationMethod XsdSimpleType::derivationMethod() const
{
   return m_derivationMethod;
}

bool XsdSimpleType::isDefinedBySchema() const
{
   return true;
}
