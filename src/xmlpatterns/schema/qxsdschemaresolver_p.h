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

#ifndef QXsdSchemaResolver_P_H
#define QXsdSchemaResolver_P_H

#include <qnamespacesupport_p.h>
#include <qschematype_p.h>
#include <qschematypefactory_p.h>
#include <qxsdalternative_p.h>
#include <qxsdattribute_p.h>
#include <qxsdattributegroup_p.h>
#include <qxsdelement_p.h>
#include <qxsdmodelgroup_p.h>
#include <qxsdnotation_p.h>
#include <qxsdreference_p.h>
#include <qxsdschema_p.h>
#include <qxsdschemachecker_p.h>
#include <qxsdsimpletype_p.h>
#include <QExplicitlySharedDataPointer>

namespace QPatternist {

class XsdSchemaContext;
class XsdSchemaParserContext;

class XsdSchemaResolver : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<XsdSchemaResolver> Ptr;

   XsdSchemaResolver(const QExplicitlySharedDataPointer<XsdSchemaContext> &context,
                     const XsdSchemaParserContext *parserContext);

   ~XsdSchemaResolver();

   void resolve();

   void addKeyReference(const XsdElement::Ptr &element, const XsdIdentityConstraint::Ptr &keyRef,
                        const QXmlName &name, const QSourceLocation &location);

   void addSimpleRestrictionBase(const XsdSimpleType::Ptr &simpleType, const QXmlName &baseName,
                                 const QSourceLocation &location);

   void removeSimpleRestrictionBase(const XsdSimpleType::Ptr &type);

   void addSimpleListType(const XsdSimpleType::Ptr &simpleType, const QXmlName &typeName, const QSourceLocation &location);

   void addSimpleUnionTypes(const XsdSimpleType::Ptr &simpleType, const QList<QXmlName> &typeNames,
                            const QSourceLocation &location);

   void addElementType(const XsdElement::Ptr &element, const QXmlName &typeName, const QSourceLocation &location);

   void addComplexBaseType(const XsdComplexType::Ptr &complexType, const QXmlName &baseName,
                           const QSourceLocation &location, const XsdFacet::Hash &facets = XsdFacet::Hash());
   void removeComplexBaseType(const XsdComplexType::Ptr &type);

   void addComplexContentType(const XsdComplexType::Ptr &complexType, const XsdParticle::Ptr &content, bool mixed);
   void addAttributeType(const XsdAttribute::Ptr &attribute, const QXmlName &typeName, const QSourceLocation &location);
   void addAlternativeType(const XsdAlternative::Ptr &alternative, const QXmlName &typeName,
                           const QSourceLocation &location);
   void addAlternativeType(const XsdAlternative::Ptr &alternative, const XsdElement::Ptr &element);
   void addSubstitutionGroupAffiliation(const XsdElement::Ptr &element, const QList<QXmlName> &elementName,
                                        const QSourceLocation &location);
   void addSubstitutionGroupType(const XsdElement::Ptr &element);
   void addComponentLocationHash(const QHash<NamedSchemaComponent::Ptr, QSourceLocation> &hash);
   void addEnumerationFacetValue(const AtomicValue::Ptr &facetValue, const NamespaceSupport &namespaceSupport);
   void addRedefinedGroups(const XsdModelGroup::Ptr &redefinedGroup, const XsdModelGroup::Ptr &group);
   void addRedefinedAttributeGroups(const XsdAttributeGroup::Ptr &redefinedGroup, const XsdAttributeGroup::Ptr &group);
   void addAllGroupCheck(const XsdReference::Ptr &reference);

   void copyDataTo(const XsdSchemaResolver::Ptr &other) const;

   QXmlName baseTypeNameOfType(const SchemaType::Ptr &type) const;
   QXmlName typeNameOfAttribute(const XsdAttribute::Ptr &attribute) const;

   void setDefaultOpenContent(const XsdComplexType::OpenContent::Ptr &openContent, bool appliesToEmpty);

 private:
   void resolveKeyReferences();
   void resolveSimpleRestrictionBaseTypes();
   void resolveSimpleRestrictions();
   void resolveSimpleRestrictions(const XsdSimpleType::Ptr &simpleType, QSet<XsdSimpleType::Ptr> &visitedTypes);
   void resolveSimpleListType();
   void resolveSimpleUnionTypes();
   void resolveElementTypes();
   void resolveComplexBaseTypes();
   void resolveSimpleContentComplexTypes();
   void resolveComplexContentComplexTypes();
   void resolveSimpleContentComplexTypes(const XsdComplexType::Ptr &complexType, QSet<XsdComplexType::Ptr> &visitedTypes);
   void resolveComplexContentComplexTypes(const XsdComplexType::Ptr &complexType, QSet<XsdComplexType::Ptr> &visitedTypes);
   void resolveAttributeTypes();
   void resolveAlternativeTypes();
   void resolveSubstitutionGroupAffiliations();
   void resolveSubstitutionGroups();
   void resolveTermReferences();
   void resolveTermReference(const XsdParticle::Ptr &particle, QSet<QXmlName> visitedGroups);
   void resolveAttributeTermReferences();

   XsdAttributeUse::List resolveAttributeTermReferences(const XsdAttributeUse::List &attributeUses,
         XsdWildcard::Ptr &wildcard, QSet<QXmlName> visitedAttributeGroups);

   void resolveAttributeInheritance();
   void resolveAttributeInheritance(const XsdComplexType::Ptr &complexType, QSet<XsdComplexType::Ptr> &visitedTypes);
   void resolveEnumerationFacetValues();

   QSourceLocation sourceLocation(const NamedSchemaComponent::Ptr component) const;

   XsdFacet::Hash complexTypeFacets(const XsdComplexType::Ptr &complexType) const;

   AnySimpleType::Ptr findPrimitiveType(const AnySimpleType::Ptr &type, QSet<AnySimpleType::Ptr> &visitedTypes);

   void checkRedefinedGroups();
   void checkRedefinedAttributeGroups();

   class KeyReference
   {
    public:
      XsdElement::Ptr element;
      XsdIdentityConstraint::Ptr keyRef;
      QXmlName reference;
      QSourceLocation location;
   };

   class SimpleRestrictionBase
   {
    public:
      XsdSimpleType::Ptr simpleType;
      QXmlName baseName;
      QSourceLocation location;
   };

   class SimpleListType
   {
    public:
      XsdSimpleType::Ptr simpleType;
      QXmlName typeName;
      QSourceLocation location;
   };

   class SimpleUnionType
   {
    public:
      XsdSimpleType::Ptr simpleType;
      QList<QXmlName> typeNames;
      QSourceLocation location;
   };

   class ElementType
   {
    public:
      XsdElement::Ptr element;
      QXmlName typeName;
      QSourceLocation location;
   };

   class ComplexBaseType
   {
    public:
      XsdComplexType::Ptr complexType;
      QXmlName baseName;
      QSourceLocation location;
      XsdFacet::Hash facets;
   };

   class ComplexContentType
   {
    public:
      XsdComplexType::Ptr complexType;
      XsdParticle::Ptr explicitContent;
      bool effectiveMixed;
   };

   class AttributeType
   {
    public:
      XsdAttribute::Ptr attribute;
      QXmlName typeName;
      QSourceLocation location;
   };

   class AlternativeType
   {
    public:
      XsdAlternative::Ptr alternative;
      QXmlName typeName;
      QSourceLocation location;
   };

   class AlternativeTypeElement
   {
    public:
      XsdAlternative::Ptr alternative;
      XsdElement::Ptr element;
   };

   class SubstitutionGroupAffiliation
   {
    public:
      XsdElement::Ptr element;
      QList<QXmlName> elementNames;
      QSourceLocation location;
   };

   class RedefinedGroups
   {
    public:
      XsdModelGroup::Ptr redefinedGroup;
      XsdModelGroup::Ptr group;
   };

   class RedefinedAttributeGroups
   {
    public:
      XsdAttributeGroup::Ptr redefinedGroup;
      XsdAttributeGroup::Ptr group;
   };

   QVector<KeyReference>                                m_keyReferences;
   QVector<SimpleRestrictionBase>                       m_simpleRestrictionBases;
   QVector<SimpleListType>                              m_simpleListTypes;
   QVector<SimpleUnionType>                             m_simpleUnionTypes;
   QVector<ElementType>                                 m_elementTypes;
   QVector<ComplexBaseType>                             m_complexBaseTypes;
   QVector<ComplexContentType>                          m_complexContentTypes;
   QVector<AttributeType>                               m_attributeTypes;
   QVector<AlternativeType>                             m_alternativeTypes;
   QVector<AlternativeTypeElement>                      m_alternativeTypeElements;
   QVector<SubstitutionGroupAffiliation>                m_substitutionGroupAffiliations;
   QVector<XsdElement::Ptr>                             m_substitutionGroupTypes;
   QVector<RedefinedGroups>                             m_redefinedGroups;
   QVector<RedefinedAttributeGroups>                    m_redefinedAttributeGroups;
   QHash<AtomicValue::Ptr, NamespaceSupport>            m_enumerationFacetValues;
   QSet<XsdReference::Ptr>                              m_allGroups;

   QExplicitlySharedDataPointer<XsdSchemaContext>       m_context;
   QExplicitlySharedDataPointer<XsdSchemaChecker>       m_checker;
   NamePool::Ptr                                        m_namePool;
   XsdSchema::Ptr                                       m_schema;
   QHash<NamedSchemaComponent::Ptr, QSourceLocation>    m_componentLocationHash;
   XsdComplexType::OpenContent::Ptr                     m_defaultOpenContent;
   bool                                                 m_defaultOpenContentAppliesToEmpty;
   SchemaType::List                                     m_predefinedSchemaTypes;
};

}

#endif
