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

   /**
    * Creates a new schema resolver.
    *
    * @param context The schema context used for error reporting etc..
    * @param parserContext The schema parser context where all objects to resolve belong to.
    */
   XsdSchemaResolver(const QExplicitlySharedDataPointer<XsdSchemaContext> &context,
                     const XsdSchemaParserContext *parserContext);

   /**
    * Destroys the schema resolver.
    */
   ~XsdSchemaResolver();

   /**
    * Starts the resolve process.
    */
   void resolve();

   /**
    * Adds a resolve task for key references.
    *
    * The resolver will try to set the referencedKey property of @p keyRef to the <em>key</em> or <em>unique</em> object
    * of @p element that has the given @p name.
    */
   void addKeyReference(const XsdElement::Ptr &element, const XsdIdentityConstraint::Ptr &keyRef,
                        const QXmlName &name, const QSourceLocation &location);

   /**
    * Adds a resolve task for the base type of restriction of a simple type.
    *
    * The resolver will set the base type of @p simpleType to the type named by @p baseName.
    */
   void addSimpleRestrictionBase(const XsdSimpleType::Ptr &simpleType, const QXmlName &baseName,
                                 const QSourceLocation &location);

   /**
    * Removes the resolve task for the base type of restriction of the simple @p type.
    */
   void removeSimpleRestrictionBase(const XsdSimpleType::Ptr &type);

   /**
    * Adds a resolve task for the list type of a simple type.
    *
    * The resolver will set the itemType property of @p simpleType to the type named by @p typeName.
    */
   void addSimpleListType(const XsdSimpleType::Ptr &simpleType, const QXmlName &typeName, const QSourceLocation &location);

   /**
    * Adds a resolve task for the member types of a simple type.
    *
    * The resolver will set the memberTypes property of @p simpleType to the types named by @p typeNames.
    */
   void addSimpleUnionTypes(const XsdSimpleType::Ptr &simpleType, const QList<QXmlName> &typeNames,
                            const QSourceLocation &location);

   /**
    * Adds a resolve task for the type of an element.
    *
    * The resolver will set the type of the @p element to the type named by @p typeName.
    */
   void addElementType(const XsdElement::Ptr &element, const QXmlName &typeName, const QSourceLocation &location);

   /**
    * Adds a resolve task for the base type of a complex type.
    *
    * The resolver will set the base type of @p complexType to the type named by @p baseName.
    */
   void addComplexBaseType(const XsdComplexType::Ptr &complexType, const QXmlName &baseName,
                           const QSourceLocation &location, const XsdFacet::Hash &facets = XsdFacet::Hash());

   /**
    * Removes the resolve task for the base type of the complex @p type.
    */
   void removeComplexBaseType(const XsdComplexType::Ptr &type);

   /**
    * Adds a resolve task for the content type of a complex type.
    *
    * The resolver will set the content type properties for @p complexType based on the
    * given explicit @p content and effective @p mixed value.
    */
   void addComplexContentType(const XsdComplexType::Ptr &complexType, const XsdParticle::Ptr &content, bool mixed);

   /**
    * Adds a resolve task for the type of an attribute.
    *
    * The resolver will set the type of the @p attribute to the type named by @p typeName.
    */
   void addAttributeType(const XsdAttribute::Ptr &attribute, const QXmlName &typeName, const QSourceLocation &location);

   /**
    * Adds a resolve task for the type of an alternative.
    *
    * The resolver will set the type of the @p alternative to the type named by @p typeName.
    */
   void addAlternativeType(const XsdAlternative::Ptr &alternative, const QXmlName &typeName,
                           const QSourceLocation &location);

   /**
    * Adds a resolve task for the type of an alternative.
    *
    * The resolver will set the type of the @p alternative to the type of the @p element after
    * the type of the @p element has been resolved.
    */
   void addAlternativeType(const XsdAlternative::Ptr &alternative, const XsdElement::Ptr &element);

   /**
    * Adds a resolve task for the substituion group affiliations of an element.
    *
    * The resolver will set the substitution group affiliations of the @p element to the
    * top-level element named by @p elementNames.
    */
   void addSubstitutionGroupAffiliation(const XsdElement::Ptr &element, const QList<QXmlName> &elementName,
                                        const QSourceLocation &location);

   /**
    * Adds a resolve task for an element that has no type specified, only a substitution group
    * affiliation.
    *
    * The resolver will set the type of the substitution group affiliation as type for the element.
    */
   void addSubstitutionGroupType(const XsdElement::Ptr &element);

   /**
    * Adds the component location hash, so the resolver is able to report meaning full
    * error messages.
    */
   void addComponentLocationHash(const QHash<NamedSchemaComponent::Ptr, QSourceLocation> &hash);

   /**
    * Add a resolve task for enumeration facet values.
    *
    * In case the enumeration is of type QName or NOTATION, we have to resolve the QName later,
    * so we store the namespace bindings together with the facet value here and resolve it as soon as
    * we have all type information available.
    */
   void addEnumerationFacetValue(const AtomicValue::Ptr &facetValue, const NamespaceSupport &namespaceSupport);

   /**
    * Add a check job for redefined groups.
    *
    * When an element group is redefined, we have to check whether the redefined group is a valid
    * restriction of the group it redefines. As we need all type information for that, we keep them
    * here for later checking.
    */
   void addRedefinedGroups(const XsdModelGroup::Ptr &redefinedGroup, const XsdModelGroup::Ptr &group);

   /**
    * Add a check job for redefined attribute groups.
    *
    * When an attribute group is redefined, we have to check whether the redefined group is a valid
    * restriction of the group it redefines. As we need all type information for that, we keep them
    * here for later checking.
    */
   void addRedefinedAttributeGroups(const XsdAttributeGroup::Ptr &redefinedGroup, const XsdAttributeGroup::Ptr &group);

   /**
    * Adds a check for nested <em>all</em> groups.
    */
   void addAllGroupCheck(const XsdReference::Ptr &reference);

   /**
    * Copies the data to resolve to an @p other resolver.
    *
    * @note That functionality is only used by the redefine algorithm in the XsdSchemaParser.
    */
   void copyDataTo(const XsdSchemaResolver::Ptr &other) const;

   /**
    * Returns the to resolve base type name for the given @p type.
    *
    * @note That functionality is only used by the redefine algorithm in the XsdSchemaParser.
    */
   QXmlName baseTypeNameOfType(const SchemaType::Ptr &type) const;

   /**
    * Returns the to resolve type name for the given @p attribute.
    *
    * @note That functionality is only used by the redefine algorithm in the XsdSchemaParser.
    */
   QXmlName typeNameOfAttribute(const XsdAttribute::Ptr &attribute) const;

   /**
    * Sets the defaultOpenContent object from the schema parser.
    */
   void setDefaultOpenContent(const XsdComplexType::OpenContent::Ptr &openContent, bool appliesToEmpty);

 private:
   /**
    * Resolves key references.
    */
   void resolveKeyReferences();

   /**
    * Resolves the base types of simple types derived by restriction.
    */
   void resolveSimpleRestrictionBaseTypes();

   /**
    * Resolves the other properties except the base type
    * of all simple restrictions.
    */
   void resolveSimpleRestrictions();

   /**
    * Resolves the other properties except the base type
    * of the given simple restriction.
    *
    * @param simpleType The restricted type to resolve.
    * @param visitedTypes A set of already resolved types, used for termination of recursion.
    */
   void resolveSimpleRestrictions(const XsdSimpleType::Ptr &simpleType, QSet<XsdSimpleType::Ptr> &visitedTypes);

   /**
    * Resolves the item type property of simple types derived by list.
    */
   void resolveSimpleListType();

   /**
    * Resolves the member types property of simple types derived by union.
    */
   void resolveSimpleUnionTypes();

   /**
    * Resolves element types.
    */
   void resolveElementTypes();

   /**
    * Resolves base type of complex types.
    */
   void resolveComplexBaseTypes();

   /**
    * Resolves the simple content model of a complex type
    * depending on its base type.
    */
   void resolveSimpleContentComplexTypes();

   /**
    * Resolves the complex content model of a complex type
    * depending on its base type.
    */
   void resolveComplexContentComplexTypes();

   /**
    * Resolves the simple content model of a complex type
    * depending on its base type.
    *
    * @param complexType The complex type to resolve.
    * @param visitedTypes A set of already resolved types, used for termination of recursion.
    */
   void resolveSimpleContentComplexTypes(const XsdComplexType::Ptr &complexType, QSet<XsdComplexType::Ptr> &visitedTypes);

   /**
    * Resolves the complex content model of a complex type
    * depending on its base type.
    *
    * @param complexType The complex type to resolve.
    * @param visitedTypes A set of already resolved types, used for termination of recursion.
    */
   void resolveComplexContentComplexTypes(const XsdComplexType::Ptr &complexType, QSet<XsdComplexType::Ptr> &visitedTypes);

   /**
    * Resolves attribute types.
    */
   void resolveAttributeTypes();

   /**
    * Resolves alternative types.
    */
   void resolveAlternativeTypes();

   /**
    * Resolves substitution group affiliations.
    */
   void resolveSubstitutionGroupAffiliations();

   /**
    * Resolves substitution groups.
    */
   void resolveSubstitutionGroups();

   /**
    * Resolves all XsdReferences in the schema by their corresponding XsdElement or XsdModelGroup terms.
    */
   void resolveTermReferences();

   /**
    * Resolves all XsdReferences in the @p particle recursive by their corresponding XsdElement or XsdModelGroup terms.
    */
   void resolveTermReference(const XsdParticle::Ptr &particle, QSet<QXmlName> visitedGroups);

   /**
    * Resolves all XsdAttributeReferences in the schema by their corresponding XsdAttributeUse objects.
    */
   void resolveAttributeTermReferences();

   /**
    * Resolves all XsdAttributeReferences in the list of @p attributeUses by their corresponding XsdAttributeUse objects.
    */
   XsdAttributeUse::List resolveAttributeTermReferences(const XsdAttributeUse::List &attributeUses,
         XsdWildcard::Ptr &wildcard, QSet<QXmlName> visitedAttributeGroups);

   /**
    * Resolves the attribute inheritance of complex types.
    *
    * @note This method must be called after all base types have been resolved.
    */
   void resolveAttributeInheritance();

   /**
    * Resolves the attribute inheritance of the given complex types.
    *
    * @param complexType The complex type to resolve.
    * @param visitedTypes A set of already resolved types, used for termination of recursion.
    *
    * @note This method must be called after all base types have been resolved.
    */
   void resolveAttributeInheritance(const XsdComplexType::Ptr &complexType, QSet<XsdComplexType::Ptr> &visitedTypes);

   /**
    * Resolves the enumeration facet values for QName and NOTATION based facets.
    */
   void resolveEnumerationFacetValues();

   /**
    * Returns the source location of the given schema @p component or a dummy
    * source location if the component is not found in the component location hash.
    */
   QSourceLocation sourceLocation(const NamedSchemaComponent::Ptr component) const;

   /**
    * Returns the facets that are marked for the given complex @p type with a simple
    * type restriction.
    */
   XsdFacet::Hash complexTypeFacets(const XsdComplexType::Ptr &complexType) const;

   /**
    * Finds the primitive type for the given simple @p type.
    *
    * The type is found by walking up the inheritance tree, until one of the builtin
    * primitive type definitions is reached.
    */
   AnySimpleType::Ptr findPrimitiveType(const AnySimpleType::Ptr &type, QSet<AnySimpleType::Ptr> &visitedTypes);

   /**
    * Checks the redefined groups.
    */
   void checkRedefinedGroups();

   /**
    * Checks the redefined attribute groups.
    */
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
