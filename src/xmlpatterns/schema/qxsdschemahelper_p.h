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

#ifndef QXsdSchemaHelper_P_H
#define QXsdSchemaHelper_P_H

#include <qcomparisonfactory_p.h>
#include <qschematype_p.h>
#include <qxsdattributegroup_p.h>
#include <qxsdelement_p.h>
#include <qxsdparticle_p.h>
#include <qxsdschemacontext_p.h>
#include <qxsdwildcard_p.h>

namespace QPatternist {

class XsdSchemaHelper
{
 public:
   /**
    * Checks whether the given @p particle is emptiable as defined by the
    * algorithm in the schema spec.
    */
   static bool isParticleEmptiable(const XsdParticle::Ptr &particle);

   /**
    * Checks whether the given @p nameSpace is allowed by the given namespace @p constraint.
    */
   static bool wildcardAllowsNamespaceName(const QString &nameSpace,
                                           const XsdWildcard::NamespaceConstraint::Ptr &constraint);

   /**
    * Checks whether the given @p name is allowed by the namespace constraint of the given @p wildcard.
    */
   static bool wildcardAllowsExpandedName(const QXmlName &name,
                                          const XsdWildcard::Ptr &wildcard,
                                          const NamePool::Ptr &namePool);

   /**
    * Checks whether the @p wildcard is a subset of @p otherWildcard.
    */
   static bool isWildcardSubset(const XsdWildcard::Ptr &wildcard, const XsdWildcard::Ptr &otherWildcard);

   /**
    * Returns the union of the given @p wildcard and @p otherWildcard.
    */
   static XsdWildcard::Ptr wildcardUnion(const XsdWildcard::Ptr &wildcard, const XsdWildcard::Ptr &otherWildcard);

   /**
    * Returns the intersection of the given @p wildcard and @p otherWildcard.
    */
   static XsdWildcard::Ptr wildcardIntersection(const XsdWildcard::Ptr &wildcard,
         const XsdWildcard::Ptr &otherWildcard);

   /**
    * Returns whether the given @p type is validly substitutable for an @p otherType
    * under the given @p constraints.
    */
   static bool isValidlySubstitutable(const SchemaType::Ptr &type,
                                      const SchemaType::Ptr &otherType,
                                      const SchemaType::DerivationConstraints &constraints);

   /**
    * Returns whether the simple @p derivedType can be derived from the simple @p baseType
    * under the given @p constraints.
    */
   static bool isSimpleDerivationOk(const SchemaType::Ptr &derivedType,
                                    const SchemaType::Ptr &baseType,
                                    const SchemaType::DerivationConstraints &constraints);

   /**
    * Returns whether the complex @p derivedType can be derived from the complex @p baseType
    * under the given @p constraints.
    */
   static bool isComplexDerivationOk(const SchemaType::Ptr &derivedType,
                                     const SchemaType::Ptr &baseType,
                                     const SchemaType::DerivationConstraints &constraints);

   /**
    * This method takes the two string based operands @p operand1 and @p operand2 and converts them to instances of type @p type.
    * If the conversion fails, @c false is returned, otherwise the instances are compared by the given operator @p op and the
    * result of the comparison is returned.
    */
   static bool constructAndCompare(const DerivedString<TypeString>::Ptr &operand1,
                                   const AtomicComparator::Operator op,
                                   const DerivedString<TypeString>::Ptr &operand2,
                                   const SchemaType::Ptr &type,
                                   const ReportContext::Ptr &context,
                                   const SourceLocationReflection *const sourceLocationReflection);

   /**
    * Returns whether the process content property of the @p derivedWildcard is valid
    * according to the process content property of its @p baseWildcard.
    */
   static bool checkWildcardProcessContents(const XsdWildcard::Ptr &baseWildcard,
         const XsdWildcard::Ptr &derivedWildcard);

   /**
    * Checks whether @[ member is a member of the substitution group with the given @p head.
    */
   static bool foundSubstitutionGroupTransitive(const XsdElement::Ptr &head,
         const XsdElement::Ptr &member,
         QSet<XsdElement::Ptr> &visitedElements);

   /**
    * A helper method that iterates over the type hierarchy from @p memberType up to @p headType and collects all
    * @p derivationSet and @p blockSet constraints that exists on the way there.
    */
   static void foundSubstitutionGroupTypeInheritance(const SchemaType::Ptr &headType,
         const SchemaType::Ptr &memberType,
         QSet<SchemaType::DerivationMethod> &derivationSet,
         NamedSchemaComponent::BlockingConstraints &blockSet);

   /**
    * Checks if the @p member is transitive to @p head.
    */
   static bool substitutionGroupOkTransitive(const XsdElement::Ptr &head,
         const XsdElement::Ptr &member,
         const NamePool::Ptr &namePool);

   /**
    * Checks if @p derivedAttributeGroup is a valid restriction for @p attributeGroup.
    */
   static bool isValidAttributeGroupRestriction(const XsdAttributeGroup::Ptr &derivedAttributeGroup,
         const XsdAttributeGroup::Ptr &attributeGroup,
         const XsdSchemaContext::Ptr &context,
         QString &errorMsg);

   /**
    * Checks if @p derivedAttributeUses are a valid restriction for @p attributeUses.
    */
   static bool isValidAttributeUsesRestriction(const XsdAttributeUse::List &derivedAttributeUses,
         const XsdAttributeUse::List &attributeUses,
         const XsdWildcard::Ptr &derivedWildcard,
         const XsdWildcard::Ptr &wildcard,
         const XsdSchemaContext::Ptr &context,
         QString &errorMsg);

   /**
    * Checks if @p derivedAttributeUses are a valid extension for @p attributeUses.
    */
   static bool isValidAttributeUsesExtension(const XsdAttributeUse::List &derivedAttributeUses,
         const XsdAttributeUse::List &attributeUses,
         const XsdWildcard::Ptr &derivedWildcard,
         const XsdWildcard::Ptr &wildcard,
         const XsdSchemaContext::Ptr &context,
         QString &errorMsg);

 private:
   XsdSchemaHelper(const XsdSchemaHelper &) = delete;
   XsdSchemaHelper &operator=(const XsdSchemaHelper &) = delete;
};

}

#endif
