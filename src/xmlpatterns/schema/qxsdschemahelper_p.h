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
   static bool isParticleEmptiable(const XsdParticle::Ptr &particle);

   static bool wildcardAllowsNamespaceName(const QString &nameSpace,
                                           const XsdWildcard::NamespaceConstraint::Ptr &constraint);

   static bool wildcardAllowsExpandedName(const QXmlName &name,
                                          const XsdWildcard::Ptr &wildcard,
                                          const NamePool::Ptr &namePool);

   static bool isWildcardSubset(const XsdWildcard::Ptr &wildcard, const XsdWildcard::Ptr &otherWildcard);

   static XsdWildcard::Ptr wildcardUnion(const XsdWildcard::Ptr &wildcard, const XsdWildcard::Ptr &otherWildcard);

   static XsdWildcard::Ptr wildcardIntersection(const XsdWildcard::Ptr &wildcard,
         const XsdWildcard::Ptr &otherWildcard);

   static bool isValidlySubstitutable(const SchemaType::Ptr &type,
                                      const SchemaType::Ptr &otherType,
                                      const SchemaType::DerivationConstraints &constraints);

   static bool isSimpleDerivationOk(const SchemaType::Ptr &derivedType,
                                    const SchemaType::Ptr &baseType,
                                    const SchemaType::DerivationConstraints &constraints);

   static bool isComplexDerivationOk(const SchemaType::Ptr &derivedType,
                                     const SchemaType::Ptr &baseType,
                                     const SchemaType::DerivationConstraints &constraints);

   static bool constructAndCompare(const DerivedString<TypeString>::Ptr &operand1,
                                   const AtomicComparator::Operator op,
                                   const DerivedString<TypeString>::Ptr &operand2,
                                   const SchemaType::Ptr &type,
                                   const ReportContext::Ptr &context,
                                   const SourceLocationReflection *const sourceLocationReflection);

   static bool checkWildcardProcessContents(const XsdWildcard::Ptr &baseWildcard,
         const XsdWildcard::Ptr &derivedWildcard);

   static bool foundSubstitutionGroupTransitive(const XsdElement::Ptr &head,
         const XsdElement::Ptr &member,
         QSet<XsdElement::Ptr> &visitedElements);

   static void foundSubstitutionGroupTypeInheritance(const SchemaType::Ptr &headType,
         const SchemaType::Ptr &memberType,
         QSet<SchemaType::DerivationMethod> &derivationSet,
         NamedSchemaComponent::BlockingConstraints &blockSet);

   static bool substitutionGroupOkTransitive(const XsdElement::Ptr &head,
         const XsdElement::Ptr &member,
         const NamePool::Ptr &namePool);

   static bool isValidAttributeGroupRestriction(const XsdAttributeGroup::Ptr &derivedAttributeGroup,
         const XsdAttributeGroup::Ptr &attributeGroup,
         const XsdSchemaContext::Ptr &context,
         QString &errorMsg);

   static bool isValidAttributeUsesRestriction(const XsdAttributeUse::List &derivedAttributeUses,
         const XsdAttributeUse::List &attributeUses,
         const XsdWildcard::Ptr &derivedWildcard,
         const XsdWildcard::Ptr &wildcard,
         const XsdSchemaContext::Ptr &context,
         QString &errorMsg);

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
