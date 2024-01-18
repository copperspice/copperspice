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

#include "qxsdschematypesfactory_p.h"

#include "qbasictypesfactory_p.h"
#include "qbuiltintypes_p.h"
#include "qderivedinteger_p.h"
#include "qderivedstring_p.h"
#include "qcommonnamespaces_p.h"
#include "qxsdschematoken_p.h"
#include "qxsdsimpletype_p.h"

using namespace QPatternist;

XsdSchemaTypesFactory::XsdSchemaTypesFactory(const NamePool::Ptr &namePool)
   : m_namePool(namePool)
{
   m_types.reserve(3);

   const XsdFacet::Ptr whiteSpaceFacet(new XsdFacet());
   whiteSpaceFacet->setType(XsdFacet::WhiteSpace);
   whiteSpaceFacet->setFixed(true);
   whiteSpaceFacet->setValue(DerivedString<TypeString>::fromLexical(m_namePool,
                             XsdSchemaToken::toString(XsdSchemaToken::Collapse)));

   const XsdFacet::Ptr minLengthFacet(new XsdFacet());
   minLengthFacet->setType(XsdFacet::MinimumLength);
   minLengthFacet->setValue(DerivedInteger<TypeNonNegativeInteger>::fromLexical(namePool, QLatin1String("1")));

   XsdFacet::Hash facets;
   facets.insert(whiteSpaceFacet->type(), whiteSpaceFacet);
   facets.insert(minLengthFacet->type(), minLengthFacet);

   {
      const QXmlName typeName = m_namePool->allocateQName(CommonNamespaces::WXS, QLatin1String("NMTOKENS"));
      const XsdSimpleType::Ptr type(new XsdSimpleType());
      type->setName(typeName);
      type->setWxsSuperType(BuiltinTypes::xsAnySimpleType);
      type->setCategory(XsdSimpleType::SimpleTypeList);
      type->setItemType(BuiltinTypes::xsNMTOKEN);
      type->setDerivationMethod(XsdSimpleType::DerivationRestriction);
      type->setFacets(facets);
      m_types.insert(typeName, type);
   }
   {
      const QXmlName typeName = m_namePool->allocateQName(CommonNamespaces::WXS, QLatin1String("IDREFS"));
      const XsdSimpleType::Ptr type(new XsdSimpleType());
      type->setName(typeName);
      type->setWxsSuperType(BuiltinTypes::xsAnySimpleType);
      type->setCategory(XsdSimpleType::SimpleTypeList);
      type->setItemType(BuiltinTypes::xsIDREF);
      type->setDerivationMethod(XsdSimpleType::DerivationRestriction);
      type->setFacets(facets);
      m_types.insert(typeName, type);
   }
   {
      const QXmlName typeName = m_namePool->allocateQName(CommonNamespaces::WXS, QLatin1String("ENTITIES"));
      const XsdSimpleType::Ptr type(new XsdSimpleType());
      type->setName(typeName);
      type->setWxsSuperType(BuiltinTypes::xsAnySimpleType);
      type->setCategory(XsdSimpleType::SimpleTypeList);
      type->setItemType(BuiltinTypes::xsENTITY);
      type->setDerivationMethod(XsdSimpleType::DerivationRestriction);
      type->setFacets(facets);
      m_types.insert(typeName, type);
   }
}

SchemaType::Ptr XsdSchemaTypesFactory::createSchemaType(const QXmlName name) const
{
   if (m_types.contains(name)) {
      return m_types.value(name);
   } else {
      if (!m_basicTypesFactory) {
         m_basicTypesFactory = BasicTypesFactory::self(m_namePool);
      }

      return m_basicTypesFactory->createSchemaType(name);
   }
}

SchemaType::Hash XsdSchemaTypesFactory::types() const
{
   return m_types;
}

