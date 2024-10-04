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

#ifndef QXsdSimpleType_P_H
#define QXsdSimpleType_P_H

#include <qanysimpletype_p.h>
#include <qxsdfacet_p.h>
#include <qxsduserschematype_p.h>
#include <QSet>

namespace QPatternist {

class XsdSimpleType : public XsdUserSchemaType<AnySimpleType>
{
 public:
   typedef QExplicitlySharedDataPointer<XsdSimpleType> Ptr;

   QString displayName(const NamePool::Ptr &namePool) const override;

   void setWxsSuperType(const SchemaType::Ptr &type);
   SchemaType::Ptr wxsSuperType() const override;

   void setContext(const NamedSchemaComponent::Ptr &component);
   NamedSchemaComponent::Ptr context() const;

   void setPrimitiveType(const AnySimpleType::Ptr &type);
   AnySimpleType::Ptr primitiveType() const;

   void setItemType(const AnySimpleType::Ptr &type);
   AnySimpleType::Ptr itemType() const;

   void setMemberTypes(const AnySimpleType::List &types);
   AnySimpleType::List memberTypes() const;

   void setFacets(const XsdFacet::Hash &facets);
   XsdFacet::Hash facets() const;

   void setCategory(TypeCategory category);
   TypeCategory category() const override;

   void setDerivationMethod(DerivationMethod method);
   DerivationMethod derivationMethod() const override;

   bool isDefinedBySchema() const override;

 private:
   SchemaType::Ptr           m_superType;
   NamedSchemaComponent     *m_context;
   AnySimpleType::Ptr        m_primitiveType;
   AnySimpleType::Ptr        m_itemType;
   AnySimpleType::List       m_memberTypes;
   XsdFacet::Hash            m_facets;
   TypeCategory              m_typeCategory;
   DerivationMethod          m_derivationMethod;
};

}

#endif
