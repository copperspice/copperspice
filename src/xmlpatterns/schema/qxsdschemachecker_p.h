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

#ifndef QXsdSchemaChecker_P_H
#define QXsdSchemaChecker_P_H

#include <qschematype_p.h>
#include <qxsdattribute_p.h>
#include <qxsdattributegroup_p.h>
#include <qxsdelement_p.h>
#include <qxsdmodelgroup_p.h>
#include <qxsdnotation_p.h>
#include <qxsdschema_p.h>
#include <qxsdsimpletype_p.h>

#include <QExplicitlySharedDataPointer>

namespace QPatternist {

class XsdSchemaContext;
class XsdSchemaParserContext;

class XsdSchemaChecker : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<XsdSchemaChecker> Ptr;

   /**
    * Creates a new schema checker.
    *
    * @param context The context that is used for customization.
    * @param parserContext The context that contains all the data structures.
    */
   XsdSchemaChecker(const QExplicitlySharedDataPointer<XsdSchemaContext> &context,
                    const XsdSchemaParserContext *parserContext);

   /**
    * Destroys the schema checker.
    */
   ~XsdSchemaChecker();

   /**
    * Starts a basic check process.
    *
    * This check only validates the basic super type inheritance
    * of simple and complex types.
    */
   void basicCheck();

   /**
    * Starts the real check process.
    */
   void check();

   /**
    * Checks the constraining facets of all global and anonymous simple types for validity.
    */
   void checkConstrainingFacets();

   /**
    * Adds the component location hash, so the checker is able to report meaning full
    * error messages.
    */
   void addComponentLocationHash(const QHash<NamedSchemaComponent::Ptr, QSourceLocation> &hash);

 private:
   void checkSimpleRestrictionBaseType();

   /**
    * Checks that no simple or complex type inherits itself.
    */
   void checkBasicCircularInheritances();

   /**
    * Checks the advanced circular inheritance.
    */
   void checkCircularInheritances();

   /**
    * Checks for inheritance restrictions given by final or finalDefault
    * attributes.
    */
   void checkInheritanceRestrictions();

   /**
    * Checks for various constraints for simple types defined by schema.
    */
   void checkBasicSimpleTypeConstraints();
   void checkSimpleTypeConstraints();

   /**
    * Checks for various constraints for complex types defined by schema.
    */
   void checkBasicComplexTypeConstraints();
   void checkComplexTypeConstraints();

   /**
    * Checks for list and union derivation restrictions given by final or finalDefault
    * attributes.
    */
   void checkSimpleDerivationRestrictions();

   /**
    * Checks the set of constraining @p facets that belongs to @p simpleType for validity.
    */
   void checkConstrainingFacets(const XsdFacet::Hash &facets, const XsdSimpleType::Ptr &simpleType);

   /**
    * Checks for duplicated attribute uses (attributes with the same name) inside a complex type.
    */
   void checkDuplicatedAttributeUses();

   /**
    * Check the element constraints.
    */
   void checkElementConstraints();

   /**
    * Check the attribute constraints.
    */
   void checkAttributeConstraints();

   /**
    * Check the attribute use constraints.
    */
   void checkAttributeUseConstraints();

   /**
    * A map used to find duplicated elements inside a model group.
    */
   typedef QHash<QXmlName, SchemaType::Ptr> DuplicatedElementMap;

   /**
    * A map used to find duplicated wildcards inside a model group.
    */
   typedef QHash<XsdWildcard::NamespaceConstraint::Variety, XsdWildcard::Ptr> DuplicatedWildcardMap;

   /**
    * Check for duplicated elements and element wildcards in all complex type particles.
    */
   void checkElementDuplicates();

   /**
    * Check for duplicated elements and element wildcards in the given @p particle.
    *
    * @param particle The particle to check.
    * @param elementMap A map to find the duplicated elements.
    * @param wildcardMap A map to find the duplicated element wildcards.
    */
   void checkElementDuplicates(const XsdParticle::Ptr &particle, DuplicatedElementMap &elementMap,
                               DuplicatedWildcardMap &wildcardMap);

   /**
    * Setup fast lookup list for allowed facets of atomic simple types.
    */
   void setupAllowedAtomicFacets();

   /**
    * Returns the source location of the given schema @p component or a dummy
    * source location if the component is not found in the component location hash.
    */
   QSourceLocation sourceLocation(const NamedSchemaComponent::Ptr &component) const;

   /**
    * Returns the source location of the given schema @p type or a dummy
    * source location if the type is not found in the component location hash.
    */
   QSourceLocation sourceLocationForType(const SchemaType::Ptr &type) const;

   /**
    * Checks that the string @p value is valid according the value space of @p type
    * for the given @p component.
    */
   bool isValidValue(const QString &value, const AnySimpleType::Ptr &type, QString &errorMsg) const;

   /**
    * Returns the list of facets for the given @p type.
    */
   XsdFacet::Hash facetsForType(const SchemaType::Ptr &type) const;

   /**
    * Returns whether the given @p list of attribute uses contains two (or more) attribute
    * uses that point to attributes with the same name. @p conflictingAttribute
    * will contain the conflicting attribute in that case.
    */
   bool hasDuplicatedAttributeUses(const XsdAttributeUse::List &list, XsdAttribute::Ptr &conflictingAttribute) const;

   /**
    * Returns whether the given @p list of attribute uses contains two (or more) attribute
    * uses that have a type inherited by xs:ID.
    */
   bool hasMultipleIDAttributeUses(const XsdAttributeUse::List &list) const;

   /**
    * Returns whether the given @p list of attribute uses contains an attribute
    * uses that has a type inherited by xs:ID with a value constraint. @p conflictingAttribute
    * will contain the conflicting attribute in that case.
    */
   bool hasConstraintIDAttributeUse(const XsdAttributeUse::List &list, XsdAttribute::Ptr &conflictingAttribute) const;

   /**
    * Checks whether the @p particle equals the @p otherParticle recursively.
    */
   bool particleEqualsRecursively(const XsdParticle::Ptr &particle, const XsdParticle::Ptr &otherParticle) const;

   /**
    * Checks whether the @p extension particle is a valid extension of the @p base particle.
    */
   bool isValidParticleExtension(const XsdParticle::Ptr &extension, const XsdParticle::Ptr &base) const;

   /**
    * Checks whether the @p sequence of elements is accepted by the given @p particle.
    */
   bool elementSequenceAccepted(const XsdModelGroup::Ptr &sequence, const XsdParticle::Ptr &particle) const;

   QExplicitlySharedDataPointer<XsdSchemaContext>       m_context;
   NamePool::Ptr                                        m_namePool;
   XsdSchema::Ptr                                       m_schema;
   QHash<QXmlName, QSet<XsdFacet::Type> >               m_allowedAtomicFacets;
   QHash<NamedSchemaComponent::Ptr, QSourceLocation>    m_componentLocationHash;
};

}

#endif
