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

#include "qxsdschemahelper_p.h"

#include "qbuiltintypes_p.h"
#include "qvaluefactory_p.h"
#include "qxsdcomplextype_p.h"
#include "qxsdmodelgroup_p.h"
#include "qxsdsimpletype_p.h"
#include "qxsdtypechecker_p.h"

using namespace QPatternist;

/*
 * Calculates the effective total range minimum of the given @p particle as
 * described by the algorithm in the schema spec.
 */
static inline unsigned int effectiveTotalRangeMinimum(const XsdParticle::Ptr &particle)
{
   const XsdModelGroup::Ptr group = particle->term();

   if (group->compositor() == XsdModelGroup::ChoiceCompositor) {
      // @see http://www.w3.org/TR/xmlschema11-1/# cos-choice-range

      int minValue = -1;

      const XsdParticle::List particles = group->particles();
      if (particles.isEmpty()) {
         minValue = 0;
      }

      for (int i = 0; i < particles.count(); ++i) {
         const XsdParticle::Ptr particle = particles.at(i);

         if (particle->term()->isElement() || particle->term()->isWildcard()) {
            if (minValue == -1) {
               minValue = particle->minimumOccurs();
            } else {
               minValue = qMin((unsigned int)minValue, particle->minimumOccurs());
            }
         } else if (particle->term()->isModelGroup()) {
            if (minValue == -1) {
               minValue = effectiveTotalRangeMinimum(particle);
            } else {
               minValue = qMin((unsigned int)minValue, effectiveTotalRangeMinimum(particle));
            }
         }
      }

      return (particle->minimumOccurs() * minValue);

   } else {
      // @see http://www.w3.org/TR/xmlschema11-1/# cos-seq-range

      unsigned int sum = 0;
      const XsdParticle::List particles = group->particles();
      for (int i = 0; i < particles.count(); ++i) {
         const XsdParticle::Ptr particle = particles.at(i);

         if (particle->term()->isElement() || particle->term()->isWildcard()) {
            sum += particle->minimumOccurs();
         } else if (particle->term()->isModelGroup()) {
            sum += effectiveTotalRangeMinimum(particle);
         }
      }

      return (particle->minimumOccurs() * sum);
   }
}

bool XsdSchemaHelper::isParticleEmptiable(const XsdParticle::Ptr &particle)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#cos-group-emptiable

   if (particle->minimumOccurs() == 0) {
      return true;
   }

   if (!(particle->term()->isModelGroup())) {
      return false;
   }

   return (effectiveTotalRangeMinimum(particle) == 0);
}

bool XsdSchemaHelper::wildcardAllowsNamespaceName(const QString &nameSpace,
      const XsdWildcard::NamespaceConstraint::Ptr &constraint)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#cvc-wildcard-namespace

   // 1
   if (constraint->variety() == XsdWildcard::NamespaceConstraint::Any) {
      return true;
   }

   // 2
   if (constraint->variety() == XsdWildcard::NamespaceConstraint::Not) { // 2.1
      if (!constraint->namespaces().contains(nameSpace)) // 2.2
         if (nameSpace != XsdWildcard::absentNamespace()) { // 2.3
            return true;
         }
   }

   // 3
   if (constraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration) {
      if (constraint->namespaces().contains(nameSpace)) {
         return true;
      }
   }

   return false;
}

bool XsdSchemaHelper::wildcardAllowsExpandedName(const QXmlName &name, const XsdWildcard::Ptr &wildcard,
      const NamePool::Ptr &namePool)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#cvc-wildcard-name

   // 1
   if (!wildcardAllowsNamespaceName(namePool->stringForNamespace(name.namespaceURI()), wildcard->namespaceConstraint())) {
      return false;
   }

   // 2, 3, 4
   //TODO: we have no disallowed namespace yet

   return true;
}

template<class T>
static inline bool containsSet(const QSet<T> &super, const QSet<T> &sub)
{
   QSetIterator<T> it(sub);
   while (it.hasNext()) {
      if (!super.contains(it.next())) {
         return false;
      }
   }

   return true;
}

bool XsdSchemaHelper::isWildcardSubset(const XsdWildcard::Ptr &wildcard, const XsdWildcard::Ptr &otherWildcard)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#cos-ns-subset
   // wildcard =^ sub
   // otherWildcard =^ super

   const XsdWildcard::NamespaceConstraint::Ptr constraint(wildcard->namespaceConstraint());
   const XsdWildcard::NamespaceConstraint::Ptr otherConstraint(otherWildcard->namespaceConstraint());

   // 1
   if (otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Any) {
      return true;
   }

   // 2
   if ((constraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration) &&
         (otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration)) {
      if (containsSet<QString>(otherConstraint->namespaces(), constraint->namespaces())) {
         return true;
      }
   }

   // 3
   if ((constraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration) &&
         (otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Not)) {
      if (constraint->namespaces().intersect(otherConstraint->namespaces()).isEmpty()) {
         return true;
      }
   }

   // 4
   if ((constraint->variety() == XsdWildcard::NamespaceConstraint::Not) &&
         (otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Not)) {
      if (containsSet<QString>(constraint->namespaces(), otherConstraint->namespaces())) {
         return true;
      }
   }

   return false;
}

XsdWildcard::Ptr XsdSchemaHelper::wildcardUnion(const XsdWildcard::Ptr &wildcard, const XsdWildcard::Ptr &otherWildcard)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#cos-aw-union

   XsdWildcard::Ptr unionWildcard(new XsdWildcard());

   const XsdWildcard::NamespaceConstraint::Ptr constraint(wildcard->namespaceConstraint());
   const XsdWildcard::NamespaceConstraint::Ptr otherConstraint(otherWildcard->namespaceConstraint());

   // 1
   if ((constraint->variety() == otherConstraint->variety()) &&
         (constraint->namespaces() == otherConstraint->namespaces())) {
      unionWildcard->namespaceConstraint()->setVariety(constraint->variety());
      unionWildcard->namespaceConstraint()->setNamespaces(constraint->namespaces());
      return unionWildcard;
   }

   // 2
   if ((constraint->variety() == XsdWildcard::NamespaceConstraint::Any) ||
         (otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Any)) {
      unionWildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Any);
      return unionWildcard;
   }

   // 3
   if ((constraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration) &&
         (otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration)) {
      unionWildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Enumeration);
      unionWildcard->namespaceConstraint()->setNamespaces(constraint->namespaces() + otherConstraint->namespaces());
      return unionWildcard;
   }

   // 4
   if ((constraint->variety() == XsdWildcard::NamespaceConstraint::Not) &&
         (otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Not)) {
      if (constraint->namespaces() != otherConstraint->namespaces()) {
         unionWildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Not);
         unionWildcard->namespaceConstraint()->setNamespaces(QSet<QString>() << XsdWildcard::absentNamespace());
         return unionWildcard;
      }
   }

   // 5
   QSet<QString> sSet, negatedSet;
   bool matches5 = false;
   if (((constraint->variety() == XsdWildcard::NamespaceConstraint::Not) &&
         !constraint->namespaces().contains(XsdWildcard::absentNamespace()))
         && (otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration)) {

      negatedSet = constraint->namespaces();
      sSet = otherConstraint->namespaces();
      matches5 = true;
   } else if (((otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Not) &&
               !otherConstraint->namespaces().contains(XsdWildcard::absentNamespace()))
              && (constraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration)) {

      negatedSet = otherConstraint->namespaces();
      sSet = constraint->namespaces();
      matches5 = true;
   }

   if (matches5) {
      if (sSet.contains(negatedSet.values().first()) && sSet.contains(XsdWildcard::absentNamespace())) { // 5.1
         unionWildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Any);
         return unionWildcard;
      }
      if (sSet.contains(negatedSet.values().first()) && !sSet.contains(XsdWildcard::absentNamespace())) { // 5.2
         unionWildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Not);
         unionWildcard->namespaceConstraint()->setNamespaces(QSet<QString>() << XsdWildcard::absentNamespace());
         return unionWildcard;
      }
      if (!sSet.contains(negatedSet.values().first()) && sSet.contains(XsdWildcard::absentNamespace())) { // 5.3
         return XsdWildcard::Ptr(); // not expressible
      }
      if (!sSet.contains(negatedSet.values().first()) && !sSet.contains(XsdWildcard::absentNamespace())) { // 5.4
         unionWildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Not);
         unionWildcard->namespaceConstraint()->setNamespaces(negatedSet);
         return unionWildcard;
      }
   }

   // 6
   bool matches6 = false;
   if (((constraint->variety() == XsdWildcard::NamespaceConstraint::Not) &&
         constraint->namespaces().contains(XsdWildcard::absentNamespace()))
         && (otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration)) {

      negatedSet = constraint->namespaces();
      sSet = otherConstraint->namespaces();
      matches6 = true;
   } else if (((otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Not) &&
               otherConstraint->namespaces().contains(XsdWildcard::absentNamespace()))
              && (constraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration)) {

      negatedSet = otherConstraint->namespaces();
      sSet = constraint->namespaces();
      matches6 = true;
   }

   if (matches6) {
      if (sSet.contains(XsdWildcard::absentNamespace())) { // 6.1
         unionWildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Any);
         return unionWildcard;
      }
      if (!sSet.contains(XsdWildcard::absentNamespace())) { // 6.2
         unionWildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Not);
         unionWildcard->namespaceConstraint()->setNamespaces(QSet<QString>() += XsdWildcard::absentNamespace());
         return unionWildcard;
      }
   }

   return XsdWildcard::Ptr();
}

XsdWildcard::Ptr XsdSchemaHelper::wildcardIntersection(const XsdWildcard::Ptr &wildcard,
      const XsdWildcard::Ptr &otherWildcard)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#cos-aw-intersect

   const XsdWildcard::NamespaceConstraint::Ptr constraint(wildcard->namespaceConstraint());
   const XsdWildcard::NamespaceConstraint::Ptr otherConstraint(otherWildcard->namespaceConstraint());

   const XsdWildcard::Ptr intersectionWildcard(new XsdWildcard());

   // 1
   if ((constraint->variety() == otherConstraint->variety()) &&
         (constraint->namespaces() == otherConstraint->namespaces())) {
      intersectionWildcard->namespaceConstraint()->setVariety(constraint->variety());
      intersectionWildcard->namespaceConstraint()->setNamespaces(constraint->namespaces());
      return intersectionWildcard;
   }

   // 2
   if ((constraint->variety() == XsdWildcard::NamespaceConstraint::Any) &&
         (otherConstraint->variety() != XsdWildcard::NamespaceConstraint::Any)) {
      intersectionWildcard->namespaceConstraint()->setVariety(otherConstraint->variety());
      intersectionWildcard->namespaceConstraint()->setNamespaces(otherConstraint->namespaces());
      return intersectionWildcard;
   }

   // 2
   if ((constraint->variety() != XsdWildcard::NamespaceConstraint::Any) &&
         (otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Any)) {
      intersectionWildcard->namespaceConstraint()->setVariety(constraint->variety());
      intersectionWildcard->namespaceConstraint()->setNamespaces(constraint->namespaces());
      return intersectionWildcard;
   }

   // 3
   if ((constraint->variety() == XsdWildcard::NamespaceConstraint::Not) &&
         (otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration)) {

      QSet<QString> set = otherConstraint->namespaces();
      set.subtract(constraint->namespaces());
      set.remove(XsdWildcard::absentNamespace());

      intersectionWildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Enumeration);
      intersectionWildcard->namespaceConstraint()->setNamespaces(set);

      return intersectionWildcard;
   }

   // 3
   if ((otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Not) &&
         (constraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration)) {

      QSet<QString> set = constraint->namespaces();
      set.subtract(otherConstraint->namespaces());
      set.remove(XsdWildcard::absentNamespace());

      intersectionWildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Enumeration);
      intersectionWildcard->namespaceConstraint()->setNamespaces(set);

      return intersectionWildcard;
   }

   // 4
   if ((constraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration) &&
         (otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Enumeration)) {

      QSet<QString> set = constraint->namespaces();
      set.intersect(otherConstraint->namespaces());

      intersectionWildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Enumeration);
      intersectionWildcard->namespaceConstraint()->setNamespaces(set);

      return intersectionWildcard;
   }

   // 6
   if ((constraint->variety() == XsdWildcard::NamespaceConstraint::Not) &&
         (otherConstraint->variety() == XsdWildcard::NamespaceConstraint::Not)) {
      if (!(constraint->namespaces().contains(XsdWildcard::absentNamespace())) &&
            otherConstraint->namespaces().contains(XsdWildcard::absentNamespace())) {
         return wildcard;
      }
      if (constraint->namespaces().contains(XsdWildcard::absentNamespace()) &&
            !(otherConstraint->namespaces().contains(XsdWildcard::absentNamespace()))) {
         return otherWildcard;
      }
   }

   // 5 as not expressible return empty wildcard
   return XsdWildcard::Ptr();
}

static SchemaType::DerivationConstraints convertBlockingConstraints(const NamedSchemaComponent::BlockingConstraints
      &constraints)
{
   SchemaType::DerivationConstraints result = Qt::EmptyFlag;

   if (constraints & NamedSchemaComponent::RestrictionConstraint) {
      result |= SchemaType::RestrictionConstraint;
   }
   if (constraints & NamedSchemaComponent::ExtensionConstraint) {
      result |= SchemaType::ExtensionConstraint;
   }

   return result;
}

bool XsdSchemaHelper::isValidlySubstitutable(const SchemaType::Ptr &type, const SchemaType::Ptr &otherType,
      const SchemaType::DerivationConstraints &constraints)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#key-val-sub-type

   // 1
   if (type->isComplexType() && otherType->isComplexType()) {
      SchemaType::DerivationConstraints keywords = constraints;
      if (otherType->isDefinedBySchema()) {
         keywords |= convertBlockingConstraints(XsdComplexType::Ptr(otherType)->prohibitedSubstitutions());
      }

      return isComplexDerivationOk(type, otherType, keywords);
   }

   // 2
   if (type->isComplexType() && otherType->isSimpleType()) {
      return isComplexDerivationOk(type, otherType, constraints);
   }

   // 3
   if (type->isSimpleType() && otherType->isSimpleType()) {
      return isSimpleDerivationOk(type, otherType, constraints);
   }

   return false;
}

bool XsdSchemaHelper::isSimpleDerivationOk(const SchemaType::Ptr &derivedType, const SchemaType::Ptr &baseType,
      const SchemaType::DerivationConstraints &constraints)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#cos-st-derived-ok

   // 1
   if (derivedType == baseType) {
      return true;
   }

   // 2.1
   if ((constraints & SchemaType::RestrictionConstraint) ||
         derivedType->wxsSuperType()->derivationConstraints() & SchemaType::RestrictionConstraint) {
      return false;
   }

   // 2.2.1
   if (derivedType->wxsSuperType() == baseType) {
      return true;
   }

   // 2.2.2
   if (derivedType->wxsSuperType() != BuiltinTypes::xsAnyType) {
      if (isSimpleDerivationOk(derivedType->wxsSuperType(), baseType, constraints)) {
         return true;
      }
   }

   // 2.2.3
   if (derivedType->category() == SchemaType::SimpleTypeList || derivedType->category() == SchemaType::SimpleTypeUnion) {
      if (baseType == BuiltinTypes::xsAnySimpleType) {
         return true;
      }
   }

   // 2.2.4
   if (baseType->category() == SchemaType::SimpleTypeUnion && baseType->isDefinedBySchema()) { // 2.2.4.1
      const AnySimpleType::List memberTypes = XsdSimpleType::Ptr(baseType)->memberTypes();
      for (int i = 0; i < memberTypes.count(); ++i) {
         if (isSimpleDerivationOk(derivedType, memberTypes.at(i), constraints)) { // 2.2.4.2
            if (XsdSimpleType::Ptr(baseType)->facets().isEmpty()) { // 2.2.4.3
               return true;
            }
         }
      }
   }

   return false;
}

bool XsdSchemaHelper::isComplexDerivationOk(const SchemaType::Ptr &derivedType, const SchemaType::Ptr &baseType,
      const SchemaType::DerivationConstraints &constraints)
{
   if (!derivedType) {
      return false;
   }

   // @see http://www.w3.org/TR/xmlschema11-1/#cos-ct-derived-ok

   // 1
   if (derivedType != baseType) {
      if ((derivedType->derivationMethod() == SchemaType::DerivationRestriction) &&
            (constraints & SchemaType::RestrictionConstraint)) {
         return false;
      }
      if ((derivedType->derivationMethod() == SchemaType::DerivationExtension) &&
            (constraints & SchemaType::ExtensionConstraint)) {
         return false;
      }
   }

   // 2.1
   if (derivedType == baseType) {
      return true;
   }

   // 2.2
   if (derivedType->wxsSuperType() == baseType) {
      return true;
   }

   // 2.3
   bool isOk = true;
   if (derivedType->wxsSuperType() == BuiltinTypes::xsAnyType) { // 2.3.1
      isOk = false;
   } else { // 2.3.2
      if (!derivedType->wxsSuperType()) {
         return false;
      }

      if (derivedType->wxsSuperType()->isComplexType()) { // 2.3.2.1
         isOk = isComplexDerivationOk(derivedType->wxsSuperType(), baseType, constraints);
      } else { // 2.3.2.2
         isOk = isSimpleDerivationOk(derivedType->wxsSuperType(), baseType, constraints);
      }
   }
   if (isOk) {
      return true;
   }

   return false;
}

bool XsdSchemaHelper::constructAndCompare(const DerivedString<TypeString>::Ptr &operand1,
      const AtomicComparator::Operator op,
      const DerivedString<TypeString>::Ptr &operand2,
      const SchemaType::Ptr &type,
      const ReportContext::Ptr &context,
      const SourceLocationReflection *const sourceLocationReflection)
{
   Q_ASSERT_X(type->category() == SchemaType::SimpleTypeAtomic, Q_FUNC_INFO,
              "We can only compare atomic values.");

   // we can not cast a xs:String to a xs:QName, so lets go the safe way
   if (type->name(context->namePool()) == BuiltinTypes::xsQName->name(context->namePool())) {
      return false;
   }

   const AtomicValue::Ptr value1 = ValueFactory::fromLexical(operand1->stringValue(), type, context,
                                   sourceLocationReflection);
   if (value1->hasError()) {
      return false;
   }

   const AtomicValue::Ptr value2 = ValueFactory::fromLexical(operand2->stringValue(), type, context,
                                   sourceLocationReflection);
   if (value2->hasError()) {
      return false;
   }

   return ComparisonFactory::compare(value1, op, value2, type, context, sourceLocationReflection);
}

bool XsdSchemaHelper::checkWildcardProcessContents(const XsdWildcard::Ptr &baseWildcard,
      const XsdWildcard::Ptr &derivedWildcard)
{
   if (baseWildcard->processContents() == XsdWildcard::Strict) {
      if (derivedWildcard->processContents() == XsdWildcard::Lax || derivedWildcard->processContents() == XsdWildcard::Skip) {
         return false;
      }
   } else if (baseWildcard->processContents() == XsdWildcard::Lax) {
      if (derivedWildcard->processContents() == XsdWildcard::Skip) {
         return false;
      }
   }

   return true;
}

bool XsdSchemaHelper::foundSubstitutionGroupTransitive(const XsdElement::Ptr &head, const XsdElement::Ptr &member,
      QSet<XsdElement::Ptr> &visitedElements)
{
   if (visitedElements.contains(member)) {
      return false;
   } else {
      visitedElements.insert(member);
   }

   if (member->substitutionGroupAffiliations().isEmpty()) {
      return false;
   }

   if (member->substitutionGroupAffiliations().contains(head)) {
      return true;
   } else {
      const XsdElement::List affiliations = member->substitutionGroupAffiliations();
      for (int i = 0; i < affiliations.count(); ++i) {
         if (foundSubstitutionGroupTransitive(head, affiliations.at(i), visitedElements)) {
            return true;
         }
      }

      return false;
   }
}

void XsdSchemaHelper::foundSubstitutionGroupTypeInheritance(const SchemaType::Ptr &headType,
      const SchemaType::Ptr &memberType,
      QSet<SchemaType::DerivationMethod> &derivationSet, NamedSchemaComponent::BlockingConstraints &blockSet)
{
   if (!memberType) {
      return;
   }

   if (memberType == headType) {
      return;
   }

   derivationSet.insert(memberType->derivationMethod());

   if (memberType->isComplexType()) {
      const XsdComplexType::Ptr complexType(memberType);
      blockSet |= complexType->prohibitedSubstitutions();
   }

   foundSubstitutionGroupTypeInheritance(headType, memberType->wxsSuperType(), derivationSet, blockSet);
}

bool XsdSchemaHelper::substitutionGroupOkTransitive(const XsdElement::Ptr &head, const XsdElement::Ptr &member,
      const NamePool::Ptr &namePool)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#cos-equiv-derived-ok-rec

   // 1
   if ((member->name(namePool) == head->name(namePool)) && (member->type() == head->type())) {
      return true;
   }

   // 2.1
   if (head->disallowedSubstitutions() & NamedSchemaComponent::SubstitutionConstraint) {
      return false;
   }

   // 2.2
   {
      QSet<XsdElement::Ptr> visitedElements;
      if (!foundSubstitutionGroupTransitive(head, member, visitedElements)) {
         return false;
      }
   }

   // 2.3
   {
      QSet<SchemaType::DerivationMethod> derivationSet;
      NamedSchemaComponent::BlockingConstraints blockSet;

      foundSubstitutionGroupTypeInheritance(head->type(), member->type(), derivationSet, blockSet);

      NamedSchemaComponent::BlockingConstraints checkSet(blockSet);
      checkSet |= head->disallowedSubstitutions();
      if (head->type()->isComplexType()) {
         const XsdComplexType::Ptr complexType(head->type());
         checkSet |= complexType->prohibitedSubstitutions();
      }

      if ((checkSet & NamedSchemaComponent::RestrictionConstraint) &&
            derivationSet.contains(SchemaType::DerivationRestriction)) {
         return false;
      }
      if ((checkSet & NamedSchemaComponent::ExtensionConstraint) && derivationSet.contains(SchemaType::DerivationExtension)) {
         return false;
      }
      if (checkSet & NamedSchemaComponent::SubstitutionConstraint) {
         return false;
      }
   }

   return true;
}

bool XsdSchemaHelper::isValidAttributeGroupRestriction(const XsdAttributeGroup::Ptr &derivedAttributeGroup,
      const XsdAttributeGroup::Ptr &attributeGroup, const XsdSchemaContext::Ptr &context, QString &errorMsg)
{
   // @see http://www.w3.org/TR/xmlschema-1/#derivation-ok-restriction

   const XsdAttributeUse::List derivedAttributeUses = derivedAttributeGroup->attributeUses();
   const XsdAttributeUse::List baseAttributeUses = attributeGroup->attributeUses();

   return isValidAttributeUsesRestriction(derivedAttributeUses, baseAttributeUses,
                                          derivedAttributeGroup->wildcard(), attributeGroup->wildcard(), context, errorMsg);
}

bool XsdSchemaHelper::isValidAttributeUsesRestriction(const XsdAttributeUse::List &derivedAttributeUses,
      const XsdAttributeUse::List &baseAttributeUses,
      const XsdWildcard::Ptr &derivedWildcard, const XsdWildcard::Ptr &wildcard,  const XsdSchemaContext::Ptr &context,
      QString &errorMsg)
{
   const NamePool::Ptr namePool(context->namePool());

   QHash<QXmlName, XsdAttributeUse::Ptr> baseAttributeUsesLookup;
   for (int i = 0; i < baseAttributeUses.count(); ++i) {
      baseAttributeUsesLookup.insert(baseAttributeUses.at(i)->attribute()->name(namePool), baseAttributeUses.at(i));
   }

   QHash<QXmlName, XsdAttributeUse::Ptr> derivedAttributeUsesLookup;
   for (int i = 0; i < derivedAttributeUses.count(); ++i) {
      derivedAttributeUsesLookup.insert(derivedAttributeUses.at(i)->attribute()->name(namePool), derivedAttributeUses.at(i));
   }

   // 2
   for (int i = 0; i < derivedAttributeUses.count(); ++i) {
      const XsdAttributeUse::Ptr derivedAttributeUse = derivedAttributeUses.at(i);

      // prohibited attributes are no real attributes, so skip them in that test here
      if (derivedAttributeUse->useType() == XsdAttributeUse::ProhibitedUse) {
         continue;
      }

      if (baseAttributeUsesLookup.contains(derivedAttributeUse->attribute()->name(namePool))) {
         const XsdAttributeUse::Ptr baseAttributeUse(baseAttributeUsesLookup.value(derivedAttributeUse->attribute()->name(
                  namePool)));

         // 2.1.1
         if (baseAttributeUse->isRequired() == true && derivedAttributeUse->isRequired() == false) {
            errorMsg = QtXmlPatterns::tr("Base attribute %1 is required but derived attribute is not.").formatArg(formatAttribute(
                          baseAttributeUse->attribute()->displayName(namePool)));
            return false;
         }

         // 2.1.2
         if (!isSimpleDerivationOk(derivedAttributeUse->attribute()->type(), baseAttributeUse->attribute()->type(),
                                   SchemaType::DerivationConstraints())) {
            errorMsg = QtXmlPatterns::tr("Type of derived attribute %1 cannot be validly derived from type of base attribute.").formatArg(
                          formatAttribute(derivedAttributeUse->attribute()->displayName(namePool)));
            return false;
         }

         // 2.1.3
         XsdAttributeUse::ValueConstraint::Ptr derivedConstraint;
         if (derivedAttributeUse->valueConstraint()) {
            derivedConstraint = derivedAttributeUse->valueConstraint();
         } else if (derivedAttributeUse->attribute()->valueConstraint()) {
            derivedConstraint = XsdAttributeUse::ValueConstraint::fromAttributeValueConstraint(
                                   derivedAttributeUse->attribute()->valueConstraint());
         }

         XsdAttributeUse::ValueConstraint::Ptr baseConstraint;
         if (baseAttributeUse->valueConstraint()) {
            baseConstraint = baseAttributeUse->valueConstraint();
         } else if (baseAttributeUse->attribute()->valueConstraint()) {
            baseConstraint = XsdAttributeUse::ValueConstraint::fromAttributeValueConstraint(
                                baseAttributeUse->attribute()->valueConstraint());
         }

         bool ok = false;
         if (!baseConstraint || baseConstraint->variety() == XsdAttributeUse::ValueConstraint::Default) {
            ok = true;
         }

         if (derivedConstraint && baseConstraint) {
            const XsdTypeChecker checker(context, QVector<QXmlName>(), QSourceLocation(QUrl(QLatin1String("http://dummy.org")), 1,
                                         1));
            if (derivedConstraint->variety() == XsdAttributeUse::ValueConstraint::Fixed &&
                  checker.valuesAreEqual(derivedConstraint->value(), baseConstraint->value(), baseAttributeUse->attribute()->type())) {
               ok = true;
            }
         }

         if (!ok) {
            errorMsg =
               QtXmlPatterns::tr("Value constraint of derived attribute %1 does not match value constraint of base attribute.").formatArg(
                  formatAttribute(derivedAttributeUse->attribute()->displayName(namePool)));
            return false;
         }
      } else {
         if (!wildcard) {
            errorMsg = QtXmlPatterns::tr("Derived attribute %1 does not exist in the base definition.").formatArg(formatAttribute(
                          derivedAttributeUse->attribute()->displayName(namePool)));
            return false;
         }

         QXmlName name = derivedAttributeUse->attribute()->name(namePool);

         // wildcards using XsdWildcard::absentNamespace, so we have to fix that here
         if (name.namespaceURI() == StandardNamespaces::empty) {
            name.setNamespaceURI(namePool->allocateNamespace(XsdWildcard::absentNamespace()));
         }

         if (!wildcardAllowsExpandedName(name, wildcard, namePool)) {
            errorMsg = QtXmlPatterns::tr("Derived attribute %1 does not match the wildcard in the base definition.").formatArg(
                          formatAttribute(derivedAttributeUse->attribute()->displayName(namePool)));
            return false;
         }
      }
   }

   // 3
   for (int i = 0; i < baseAttributeUses.count(); ++i) {
      const XsdAttributeUse::Ptr baseAttributeUse = baseAttributeUses.at(i);

      if (baseAttributeUse->isRequired()) {
         if (derivedAttributeUsesLookup.contains(baseAttributeUse->attribute()->name(namePool))) {
            if (!derivedAttributeUsesLookup.value(baseAttributeUse->attribute()->name(namePool))->isRequired()) {
               errorMsg = QtXmlPatterns::tr("Base attribute %1 is required but derived attribute is not.").formatArg(formatAttribute(
                             baseAttributeUse->attribute()->displayName(namePool)));
               return false;
            }
         } else {
            errorMsg = QtXmlPatterns::tr("Base attribute %1 is required but missing in derived definition.").formatArg(formatAttribute(
                          baseAttributeUse->attribute()->displayName(namePool)));
            return false;
         }
      }
   }

   // 4
   if (derivedWildcard) {
      if (!wildcard) {
         errorMsg =
            QtXmlPatterns::tr("Derived definition contains an %1 element that does not exists in the base definition").formatArg(
               formatElement("anyAttribute."));
         return false;
      }

      if (!isWildcardSubset(derivedWildcard, wildcard)) {
         errorMsg = QtXmlPatterns::tr("Derived wildcard is not a subset of the base wildcard.");
         return false;
      }

      if (!checkWildcardProcessContents(wildcard, derivedWildcard)) {
         errorMsg = QtXmlPatterns::tr("%1 of derived wildcard is not a valid restriction of %2 of base wildcard").formatArg(
                       formatKeyword("processContents")).formatArg(formatKeyword("processContents."));
         return false;
      }
   }

   return true;
}

bool XsdSchemaHelper::isValidAttributeUsesExtension(const XsdAttributeUse::List &derivedAttributeUses,
      const XsdAttributeUse::List &attributeUses,
      const XsdWildcard::Ptr &derivedWildcard, const XsdWildcard::Ptr &wildcard, const XsdSchemaContext::Ptr &context,
      QString &errorMsg)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#cos-ct-extends

   const NamePool::Ptr namePool(context->namePool());

   // 1.2
   QHash<QXmlName, XsdAttribute::Ptr> lookupHash;
   for (int i = 0; i < derivedAttributeUses.count(); ++i) {
      lookupHash.insert(derivedAttributeUses.at(i)->attribute()->name(namePool), derivedAttributeUses.at(i)->attribute());
   }

   for (int i = 0; i < attributeUses.count(); ++i) {
      const QXmlName attributeName = attributeUses.at(i)->attribute()->name(namePool);
      if (!lookupHash.contains(attributeName)) {
         errorMsg = QtXmlPatterns::tr("Attribute %1 from base type is missing in derived type.").formatArg(formatKeyword(
                       namePool->displayName(attributeName)));
         return false;
      }

      if (lookupHash.value(attributeName)->type() != attributeUses.at(i)->attribute()->type()) {
         errorMsg = QtXmlPatterns::tr("Type of derived attribute %1 differs from type of base attribute.").formatArg(formatKeyword(
                       namePool->displayName(attributeName)));
         return false;
      }
   }

   // 1.3
   if (wildcard) {
      if (!derivedWildcard) {
         errorMsg = QtXmlPatterns::tr("Base definition contains an %1 element that is missing in the derived definition").formatArg(
                       formatElement("anyAttribute."));
         return false;
      }
   }

   return true;
}
