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

   /**
    * Returns the display name of the simple type.
    *
    * @param namePool The name pool the type name is stored in.
    */
   QString displayName(const NamePool::Ptr &namePool) const override;

   /**
    * Sets the base @p type of the simple type.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema-2/#defn-basetype">Base Type Definition</a>
    */
   void setWxsSuperType(const SchemaType::Ptr &type);

   /**
    * Returns the base type of the simple type or an empty pointer if no base type is
    * set.
    */
   SchemaType::Ptr wxsSuperType() const override;

   /**
    * Sets the context @p component of the simple type.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema11-1/#std-context">Context Definition</a>
    */
   void setContext(const NamedSchemaComponent::Ptr &component);

   /**
    * Returns the context component of the simple type.
    */
   NamedSchemaComponent::Ptr context() const;

   /**
    * Sets the primitive @p type of the simple type.
    *
    * The primitive type is only specified if the category is SimpleTypeAtomic.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema-2/#defn-primitive">Primitive Type Definition</a>
    */
   void setPrimitiveType(const AnySimpleType::Ptr &type);

   /**
    * Returns the primitive type of the simple type or an empty pointer if the category is
    * not SimpleTypeAtomic.
    */
   AnySimpleType::Ptr primitiveType() const;

   /**
    * Sets the list item @p type of the simple type.
    *
    * The list item type is only specified if the category is SimpleTypeList.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema-2/#defn-itemType">Item Type Definition</a>
    */
   void setItemType(const AnySimpleType::Ptr &type);

   /**
    * Returns the list item type of the simple type or an empty pointer if the category is
    * not SimpleTypeList.
    */
   AnySimpleType::Ptr itemType() const;

   /**
    * Sets the member @p types of the simple type.
    *
    * The member types are only specified if the category is SimpleTypeUnion.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema-2/#defn-memberTypes">Member Types Definition</a>
    */
   void setMemberTypes(const AnySimpleType::List &types);

   /**
    * Returns the list member types of the simple type or an empty list if the category is
    * not SimpleTypeUnion.
    */
   AnySimpleType::List memberTypes() const;

   /**
    * Sets the @p facets of the simple type.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema-2/#defn-facets">Facets Definition</a>
    */
   void setFacets(const XsdFacet::Hash &facets);

   /**
    * Returns the facets of the simple type.
    */
   XsdFacet::Hash facets() const;

   /**
    * Sets the @p category (variety) of the simple type.
    *
    * @see <a href="http://www.w3.org/TR/xmlschema-2/#defn-variety">Variety Definition</a>
    */
   void setCategory(TypeCategory category);

   /**
    * Returns the category (variety) of the simple type.
    */
   TypeCategory category() const override;

   /**
    * Sets the derivation @p method of the simple type.
    *
    * @see DerivationMethod
    */
   void setDerivationMethod(DerivationMethod method);

   /**
    * Returns the derivation method of the simple type.
    */
   DerivationMethod derivationMethod() const override;

   /**
    * Always returns @c true.
    */
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
