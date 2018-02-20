/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qxsdschemachecker_p.h"

#include "qderivedinteger_p.h"
#include "qderivedstring_p.h"
#include "qpatternplatform_p.h"
#include "qqnamevalue_p.h"
#include "qsourcelocationreflection_p.h"
#include "qvaluefactory_p.h"
#include "qxsdattributereference_p.h"
#include "qxsdparticlechecker_p.h"
#include "qxsdreference_p.h"
#include "qxsdschemacontext_p.h"
#include "qxsdschemahelper_p.h"
#include "qxsdschemaparsercontext_p.h"
#include "qxsdschematypesfactory_p.h"
#include "qxsdtypechecker_p.h"

#include "qxsdschemachecker_helper.cpp"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

XsdSchemaChecker::XsdSchemaChecker(const QExplicitlySharedDataPointer<XsdSchemaContext> &context,
                                   const XsdSchemaParserContext *parserContext)
   : m_context(context)
   , m_namePool(parserContext->namePool())
   , m_schema(parserContext->schema())
{
   setupAllowedAtomicFacets();
}

XsdSchemaChecker::~XsdSchemaChecker()
{
}

/*
 * This method is called after the resolver has set the base type for every
 * type and information about deriavtion and 'is simple type vs. is complex type'
 * are available.
 */
void XsdSchemaChecker::basicCheck()
{
   // first check that there is no circular inheritance, only the
   // wxsSuperType is used here
   checkBasicCircularInheritances();

   // check the basic constraints like simple type can not inherit from complex type etc.
   checkBasicSimpleTypeConstraints();
   checkBasicComplexTypeConstraints();
}

void XsdSchemaChecker::check()
{
   checkCircularInheritances();
   checkInheritanceRestrictions();
   checkSimpleDerivationRestrictions();
   checkSimpleTypeConstraints();
   checkComplexTypeConstraints();
   checkDuplicatedAttributeUses();

   checkElementConstraints();
   checkAttributeConstraints();
   checkAttributeUseConstraints();
   //    checkElementDuplicates();
}

void XsdSchemaChecker::addComponentLocationHash(const ComponentLocationHash &hash)
{
   m_componentLocationHash.unite(hash);
}

/**
 * Checks whether the @p otherType is the same as @p myType or if one of its
 * ancestors is the same as @p myType.
 */
static bool matchesType(const SchemaType::Ptr &myType, const SchemaType::Ptr &otherType,
                        QSet<SchemaType::Ptr> visitedTypes)
{
   bool retval = false;

   if (otherType) {
      if (visitedTypes.contains(otherType)) {
         return true;
      } else {
         visitedTypes.insert(otherType);
      }
      // simple types can have different varieties, so we have to check each of them
      if (otherType->isSimpleType()) {
         const XsdSimpleType::Ptr simpleType = otherType;
         if (simpleType->category() == XsdSimpleType::SimpleTypeAtomic) {
            // for atomic type we use the same test as in SchemaType::wxsTypeMatches
            retval = (myType == simpleType ? true : matchesType(myType, simpleType->wxsSuperType(), visitedTypes));
         } else if (simpleType->category() == XsdSimpleType::SimpleTypeList) {
            // for list type we test against the itemType property
            retval = (myType == simpleType->itemType() ? true : matchesType(myType, simpleType->itemType()->wxsSuperType(),
                      visitedTypes));
         } else if (simpleType->category() == XsdSimpleType::SimpleTypeUnion) {
            // for union type we test against each member type
            const XsdSimpleType::List members = simpleType->memberTypes();
            for (int i = 0; i < members.count(); ++i) {
               if (myType == members.at(i) ? true : matchesType(myType, members.at(i)->wxsSuperType(), visitedTypes)) {
                  retval = true;
                  break;
               }
            }
         } else {
            // reached xsAnySimple type whichs category is None
            retval = false;
         }
      } else {
         // if no simple type we handle it like in SchemaType::wxsTypeMatches
         retval = (myType == otherType ? true : matchesType(myType, otherType->wxsSuperType(), visitedTypes));
      }
   } else { // if otherType is null it doesn't match
      retval = false;
   }

   return retval;
}

/**
 * Checks whether there is a circular inheritance for the union inheritance.
 */
static bool hasCircularUnionInheritance(const XsdSimpleType::Ptr &type, const SchemaType::Ptr &otherType,
                                        NamePool::Ptr &namePool)
{
   if (type == otherType) {
      return true;
   }

   if (!otherType->isSimpleType() || !otherType->isDefinedBySchema()) {
      return false;
   }

   const XsdSimpleType::Ptr simpleOtherType = otherType;

   if (simpleOtherType->category() == XsdSimpleType::SimpleTypeUnion) {
      const XsdSimpleType::List memberTypes = simpleOtherType->memberTypes();
      for (int i = 0; i < memberTypes.count(); ++i) {
         if (otherType->wxsSuperType() == type) {
            return true;
         }
         if (hasCircularUnionInheritance(type, memberTypes.at(i), namePool)) {
            return true;
         }
      }
   }

   return false;
}

static inline bool wxsTypeMatches(const SchemaType::Ptr &type, const SchemaType::Ptr &otherType,
                                  QSet<SchemaType::Ptr> &visitedTypes, SchemaType::Ptr &conflictingType)
{
   if (!otherType) {
      return false;
   }

   if (visitedTypes.contains(otherType)) { // inheritance loop detected
      conflictingType = otherType;
      return true;
   } else {
      visitedTypes.insert(otherType);
   }

   if (type == otherType) {
      return true;
   }

   return wxsTypeMatches(type, otherType->wxsSuperType(), visitedTypes, conflictingType);
}

void XsdSchemaChecker::checkBasicCircularInheritances()
{
   // check all global types...
   SchemaType::List types = m_schema->types();

   // .. and anonymous types
   types << m_schema->anonymousTypes();

   for (int i = 0; i < types.count(); ++i) {
      const SchemaType::Ptr type = types.at(i);
      const QSourceLocation location = sourceLocationForType(type);

      // @see http://www.w3.org/TR/xmlschema11-1/#ct-props-correct 3)

      // check normal base type inheritance
      QSet<SchemaType::Ptr> visitedTypes;
      SchemaType::Ptr conflictingType;

      if (wxsTypeMatches(type, type->wxsSuperType(), visitedTypes, conflictingType)) {
         if (conflictingType)
            m_context->error(QtXmlPatterns::tr("%1 has inheritance loop in its base type %2.")
                             .arg(formatType(m_namePool, type))
                             .arg(formatType(m_namePool, conflictingType)),
                             XsdSchemaContext::XSDError, location);
         else {
            m_context->error(QtXmlPatterns::tr("Circular inheritance of base type %1.").arg(formatType(m_namePool, type)),
                             XsdSchemaContext::XSDError, location);
         }

         return;
      }
   }
}

void XsdSchemaChecker::checkCircularInheritances()
{
   // check all global types...
   SchemaType::List types = m_schema->types();

   // .. and anonymous types
   types << m_schema->anonymousTypes();

   for (int i = 0; i < types.count(); ++i) {
      const SchemaType::Ptr type = types.at(i);
      const QSourceLocation location = sourceLocationForType(type);

      // @see http://www.w3.org/TR/xmlschema11-1/#ct-props-correct 3)

      // check normal base type inheritance
      QSet<SchemaType::Ptr> visitedTypes;
      if (matchesType(type, type->wxsSuperType(), visitedTypes)) {
         m_context->error(QtXmlPatterns::tr("Circular inheritance of base type %1.").arg(formatType(m_namePool, type)),
                          XsdSchemaContext::XSDError, location);
         return;
      }

      // check union member inheritance
      if (type->isSimpleType() && type->isDefinedBySchema()) {
         const XsdSimpleType::Ptr simpleType = type;
         if (simpleType->category() == XsdSimpleType::SimpleTypeUnion) {
            const XsdSimpleType::List memberTypes = simpleType->memberTypes();
            for (int j = 0; j < memberTypes.count(); ++j) {
               if (hasCircularUnionInheritance(simpleType, memberTypes.at(j), m_namePool)) {
                  m_context->error(QtXmlPatterns::tr("Circular inheritance of union %1.").arg(formatType(m_namePool, type)),
                                   XsdSchemaContext::XSDError, location);
                  return;
               }
            }
         }
      }
   }
}

void XsdSchemaChecker::checkInheritanceRestrictions()
{
   // check all global types...
   SchemaType::List types = m_schema->types();

   // .. and anonymous types
   types << m_schema->anonymousTypes();

   for (int i = 0; i < types.count(); ++i) {
      const SchemaType::Ptr type = types.at(i);
      const QSourceLocation location = sourceLocationForType(type);

      // check inheritance restrictions given by final property of base class
      const SchemaType::Ptr baseType = type->wxsSuperType();
      if (baseType->isDefinedBySchema()) {
         if ((type->derivationMethod() == SchemaType::DerivationRestriction) &&
               (baseType->derivationConstraints() & SchemaType::RestrictionConstraint)) {
            m_context->error(
               QtXmlPatterns::tr("%1 is not allowed to derive from %2 by restriction as the latter defines it as final.")
               .arg(formatType(m_namePool, type))
               .arg(formatType(m_namePool, baseType)), XsdSchemaContext::XSDError, location);
            return;
         } else if ((type->derivationMethod() == SchemaType::DerivationExtension) &&
                    (baseType->derivationConstraints() & SchemaType::ExtensionConstraint)) {
            m_context->error(
               QtXmlPatterns::tr("%1 is not allowed to derive from %2 by extension as the latter defines it as final.")
               .arg(formatType(m_namePool, type))
               .arg(formatType(m_namePool, baseType)), XsdSchemaContext::XSDError, location);
            return;
         }
      }
   }
}

void XsdSchemaChecker::checkBasicSimpleTypeConstraints()
{
   // check all global types...
   SchemaType::List types = m_schema->types();

   // .. and anonymous types
   types << m_schema->anonymousTypes();

   for (int i = 0; i < types.count(); ++i) {
      const SchemaType::Ptr type = types.at(i);

      if (!type->isSimpleType()) {
         continue;
      }

      const XsdSimpleType::Ptr simpleType = type;

      const QSourceLocation location = sourceLocation(simpleType);

      // check inheritance restrictions of simple type defined by schema constraints
      const SchemaType::Ptr baseType = simpleType->wxsSuperType();

      if (baseType->isComplexType() && (simpleType->name(m_namePool) != BuiltinTypes::xsAnySimpleType->name(m_namePool))) {
         m_context->error(QtXmlPatterns::tr("Base type of simple type %1 cannot be complex type %2.")
                          .arg(formatType(m_namePool, simpleType))
                          .arg(formatType(m_namePool, baseType)),
                          XsdSchemaContext::XSDError, location);
         return;
      }

      if (baseType == BuiltinTypes::xsAnyType) {
         if (type->name(m_namePool) != BuiltinTypes::xsAnySimpleType->name(m_namePool)) {
            m_context->error(QtXmlPatterns::tr("Simple type %1 cannot have direct base type %2.")
                             .arg(formatType(m_namePool, simpleType))
                             .arg(formatType(m_namePool, BuiltinTypes::xsAnyType)),
                             XsdSchemaContext::XSDError, location);
            return;
         }
      }
   }
}

void XsdSchemaChecker::checkSimpleTypeConstraints()
{
   // check all global types...
   SchemaType::List types = m_schema->types();

   // .. and anonymous types
   types << m_schema->anonymousTypes();

   for (int i = 0; i < types.count(); ++i) {
      const SchemaType::Ptr type = types.at(i);

      if (!type->isSimpleType()) {
         continue;
      }

      const XsdSimpleType::Ptr simpleType = type;

      const QSourceLocation location = sourceLocation(simpleType);

      if (simpleType->category() == XsdSimpleType::None) {
         // additional checks
         // check that no user defined type has xs:AnySimpleType as base type (except xs:AnyAtomicType)
         if (simpleType->wxsSuperType()->name(m_namePool) == BuiltinTypes::xsAnySimpleType->name(m_namePool)) {
            if (simpleType->name(m_namePool) != BuiltinTypes::xsAnyAtomicType->name(m_namePool)) {
               m_context->error(QtXmlPatterns::tr("Simple type %1 is not allowed to have base type %2.")
                                .arg(formatType(m_namePool, simpleType))
                                .arg(formatType(m_namePool, simpleType->wxsSuperType())),
                                XsdSchemaContext::XSDError, location);
               return;
            }
         }
         // check that no user defined type has xs:AnyAtomicType as base type
         if (simpleType->wxsSuperType()->name(m_namePool) == BuiltinTypes::xsAnyAtomicType->name(m_namePool)) {
            m_context->error(QtXmlPatterns::tr("Simple type %1 is not allowed to have base type %2.")
                             .arg(formatType(m_namePool, simpleType))
                             .arg(formatType(m_namePool, simpleType->wxsSuperType())),
                             XsdSchemaContext::XSDError, location);
            return;
         }
      }

      // @see http://www.w3.org/TR/xmlschema11-1/#d0e37310
      if (simpleType->category() == XsdSimpleType::SimpleTypeAtomic) {
         // 1.1
         if ((simpleType->wxsSuperType()->category() != XsdSimpleType::SimpleTypeAtomic) &&
               (simpleType->name(m_namePool) != BuiltinTypes::xsAnyAtomicType->name(m_namePool))) {
            m_context->error(QtXmlPatterns::tr("Simple type %1 can only have simple atomic type as base type.")
                             .arg(formatType(m_namePool, simpleType)),
                             XsdSchemaContext::XSDError, location);
         }
         // 1.2
         if (simpleType->wxsSuperType()->derivationConstraints() & SchemaType::RestrictionConstraint) {
            m_context->error(QtXmlPatterns::tr("Simple type %1 cannot derive from %2 as the latter defines restriction as final.")
                             .arg(formatType(m_namePool, simpleType->wxsSuperType()))
                             .arg(formatType(m_namePool, simpleType)),
                             XsdSchemaContext::XSDError, location);
         }

         // 1.3
         // checked by checkConstrainingFacets  already
      } else if (simpleType->category() == XsdSimpleType::SimpleTypeList) {
         const AnySimpleType::Ptr itemType = simpleType->itemType();

         // 2.1 or @see http://www.w3.org/TR/xmlschema-2/#cos-list-of-atomic
         if (itemType->category() != SchemaType::SimpleTypeAtomic && itemType->category() != SchemaType::SimpleTypeUnion) {
            m_context->error(QtXmlPatterns::tr("Variety of item type of %1 must be either atomic or union.").arg(formatType(
                                m_namePool, simpleType)), XsdSchemaContext::XSDError, location);
            return;
         }

         // 2.1 second part
         if (itemType->category() == SchemaType::SimpleTypeUnion && itemType->isDefinedBySchema()) {
            const XsdSimpleType::Ptr simpleItemType = itemType;
            const AnySimpleType::List memberTypes = simpleItemType->memberTypes();
            for (int j = 0; j < memberTypes.count(); ++j) {
               if (memberTypes.at(j)->category() != SchemaType::SimpleTypeAtomic) {
                  m_context->error(QtXmlPatterns::tr("Variety of member types of %1 must be atomic.").arg(formatType(m_namePool,
                                   simpleItemType)), XsdSchemaContext::XSDError, location);
                  return;
               }
            }
         }

         // 2.2.1
         if (simpleType->wxsSuperType()->name(m_namePool) == BuiltinTypes::xsAnySimpleType->name(m_namePool)) {
            if (itemType->isSimpleType() && itemType->isDefinedBySchema()) {
               const XsdSimpleType::Ptr simpleItemType = itemType;

               // 2.2.1.1
               if (simpleItemType->derivationConstraints() & XsdSimpleType::ListConstraint) {
                  m_context->error(QtXmlPatterns::tr("%1 is not allowed to derive from %2 by list as the latter defines it as final.")
                                   .arg(formatType(m_namePool, simpleType))
                                   .arg(formatType(m_namePool, simpleItemType)), XsdSchemaContext::XSDError, location);
                  return;
               }

               // 2.2.1.2
               const XsdFacet::Hash facets = simpleType->facets();
               XsdFacet::HashIterator it(facets);

               bool invalidFacetFound = false;
               while (it.hasNext()) {
                  it.next();
                  if (it.key() != XsdFacet::WhiteSpace) {
                     invalidFacetFound = true;
                     break;
                  }
               }

               if (invalidFacetFound) {
                  m_context->error(QtXmlPatterns::tr("Simple type %1 is only allowed to have %2 facet.")
                                   .arg(formatType(m_namePool, simpleType))
                                   .arg(formatKeyword("whiteSpace")),
                                   XsdSchemaContext::XSDError, location);
                  return;
               }
            }
         } else { // 2.2.2
            // 2.2.2.1
            if (simpleType->wxsSuperType()->category() != XsdSimpleType::SimpleTypeList) {
               m_context->error(QtXmlPatterns::tr("Base type of simple type %1 must have variety of type list.").arg(formatType(
                                   m_namePool, simpleType)), XsdSchemaContext::XSDError, location);
               return;
            }

            // 2.2.2.2
            if (simpleType->wxsSuperType()->derivationConstraints() & SchemaType::RestrictionConstraint) {
               m_context->error(QtXmlPatterns::tr("Base type of simple type %1 has defined derivation by restriction as final.").arg(
                                   formatType(m_namePool, simpleType)), XsdSchemaContext::XSDError, location);
               return;
            }

            // 2.2.2.3
            if (!XsdSchemaHelper::isSimpleDerivationOk(itemType, XsdSimpleType::Ptr(simpleType->wxsSuperType())->itemType(),
                  SchemaType::DerivationConstraints())) {
               m_context->error(QtXmlPatterns::tr("Item type of base type does not match item type of %1.").arg(formatType(m_namePool,
                                simpleType)), XsdSchemaContext::XSDError, location);
               return;
            }

            // 2.2.2.4
            const XsdFacet::Hash facets = simpleType->facets();
            XsdFacet::HashIterator it(facets);

            bool invalidFacetFound = false;
            XsdFacet::Type invalidFacetType = XsdFacet::None;
            while (it.hasNext()) {
               it.next();
               const XsdFacet::Type facetType = it.key();
               if (facetType != XsdFacet::Length &&
                     facetType != XsdFacet::MinimumLength &&
                     facetType != XsdFacet::MaximumLength &&
                     facetType != XsdFacet::WhiteSpace &&
                     facetType != XsdFacet::Pattern &&
                     facetType != XsdFacet::Enumeration) {
                  invalidFacetType = facetType;
                  invalidFacetFound = true;
                  break;
               }
            }

            if (invalidFacetFound) {
               m_context->error(QtXmlPatterns::tr("Simple type %1 contains not allowed facet type %2.")
                                .arg(formatType(m_namePool, simpleType))
                                .arg(formatKeyword(XsdFacet::typeName(invalidFacetType))),
                                XsdSchemaContext::XSDError, location);
               return;
            }

            // 2.2.2.5
            // TODO: check value constraints
         }


      } else if (simpleType->category() == XsdSimpleType::SimpleTypeUnion) {
         const AnySimpleType::List memberTypes = simpleType->memberTypes();

         if (simpleType->wxsSuperType()->name(m_namePool) == BuiltinTypes::xsAnySimpleType->name(m_namePool)) { // 3.1.1
            // 3.3.1.1
            for (int i = 0; i < memberTypes.count(); ++i) {
               const AnySimpleType::Ptr memberType = memberTypes.at(i);

               if (memberType->derivationConstraints() & XsdSimpleType::UnionConstraint) {
                  m_context->error(QtXmlPatterns::tr("%1 is not allowed to derive from %2 by union as the latter defines it as final.")
                                   .arg(formatType(m_namePool, simpleType))
                                   .arg(formatType(m_namePool, memberType)), XsdSchemaContext::XSDError, location);
                  return;
               }
            }

            // 3.3.1.2
            if (!simpleType->facets().isEmpty()) {
               m_context->error(QtXmlPatterns::tr("%1 is not allowed to have any facets.")
                                .arg(formatType(m_namePool, simpleType)),
                                XsdSchemaContext::XSDError, location);
               return;
            }
         } else {
            // 3.1.2.1
            if (simpleType->wxsSuperType()->category() != SchemaType::SimpleTypeUnion) {
               m_context->error(QtXmlPatterns::tr("Base type %1 of simple type %2 must have variety of union.")
                                .arg(formatType(m_namePool, simpleType->wxsSuperType()))
                                .arg(formatType(m_namePool, simpleType)),
                                XsdSchemaContext::XSDError, location);
               return;
            }

            // 3.1.2.2
            if (simpleType->wxsSuperType()->derivationConstraints() & SchemaType::DerivationRestriction) {
               m_context->error(QtXmlPatterns::tr("Base type %1 of simple type %2 is not allowed to have restriction in %3 attribute.")
                                .arg(formatType(m_namePool, simpleType->wxsSuperType()))
                                .arg(formatType(m_namePool, simpleType))
                                .arg(formatAttribute("final")),
                                XsdSchemaContext::XSDError, location);
               return;
            }

            //3.1.2.3
            if (simpleType->wxsSuperType()->isDefinedBySchema()) {
               const XsdSimpleType::Ptr simpleBaseType(simpleType->wxsSuperType());

               AnySimpleType::List baseMemberTypes = simpleBaseType->memberTypes();
               for (int i = 0; i < memberTypes.count(); ++i) {
                  const AnySimpleType::Ptr memberType = memberTypes.at(i);
                  const AnySimpleType::Ptr baseMemberType = baseMemberTypes.at(i);

                  if (!XsdSchemaHelper::isSimpleDerivationOk(memberType, baseMemberType, SchemaType::DerivationConstraints())) {
                     m_context->error(QtXmlPatterns::tr("Member type %1 cannot be derived from member type %2 of %3's base type %4.")
                                      .arg(formatType(m_namePool, memberType))
                                      .arg(formatType(m_namePool, baseMemberType))
                                      .arg(formatType(m_namePool, simpleType))
                                      .arg(formatType(m_namePool, simpleBaseType)),
                                      XsdSchemaContext::XSDError, location);
                  }
               }
            }

            // 3.1.2.4
            const XsdFacet::Hash facets = simpleType->facets();
            XsdFacet::HashIterator it(facets);

            bool invalidFacetFound = false;
            XsdFacet::Type invalidFacetType = XsdFacet::None;
            while (it.hasNext()) {
               it.next();
               const XsdFacet::Type facetType = it.key();
               if (facetType != XsdFacet::Pattern &&
                     facetType != XsdFacet::Enumeration) {
                  invalidFacetType = facetType;
                  invalidFacetFound = true;
                  break;
               }
            }

            if (invalidFacetFound) {
               m_context->error(QtXmlPatterns::tr("Simple type %1 contains not allowed facet type %2.")
                                .arg(formatType(m_namePool, simpleType))
                                .arg(formatKeyword(XsdFacet::typeName(invalidFacetType))),
                                XsdSchemaContext::XSDError, location);
               return;
            }

            // 3.1.2.5
            // TODO: check value constraints
         }
      }
   }
}

void XsdSchemaChecker::checkBasicComplexTypeConstraints()
{
   // check all global types...
   SchemaType::List types = m_schema->types();

   // .. and anonymous types
   types << m_schema->anonymousTypes();

   for (int i = 0; i < types.count(); ++i) {
      const SchemaType::Ptr type = types.at(i);

      if (!type->isComplexType() || !type->isDefinedBySchema()) {
         continue;
      }

      const XsdComplexType::Ptr complexType = type;

      const QSourceLocation location = sourceLocation(complexType);

      // check inheritance restrictions of complex type defined by schema constraints
      const SchemaType::Ptr baseType = complexType->wxsSuperType();

      // @see http://www.w3.org/TR/xmlschema11-1/#ct-props-correct 2)
      if (baseType->isSimpleType() && (complexType->derivationMethod() != XsdComplexType::DerivationExtension)) {
         m_context->error(
            QtXmlPatterns::tr("Derivation method of %1 must be extension because the base type %2 is a simple type.")
            .arg(formatType(m_namePool, complexType))
            .arg(formatType(m_namePool, baseType)),
            XsdSchemaContext::XSDError, location);
         return;
      }
   }
}

void XsdSchemaChecker::checkComplexTypeConstraints()
{
   // check all global types...
   SchemaType::List types = m_schema->types();

   // .. and anonymous types
   types << m_schema->anonymousTypes();

   for (int i = 0; i < types.count(); ++i) {
      const SchemaType::Ptr type = types.at(i);

      if (!type->isComplexType() || !type->isDefinedBySchema()) {
         continue;
      }

      const XsdComplexType::Ptr complexType = type;

      const QSourceLocation location = sourceLocation(complexType);

      if (complexType->contentType()->particle()) {
         XsdElement::Ptr duplicatedElement;
         if (XsdParticleChecker::hasDuplicatedElements(complexType->contentType()->particle(), m_namePool, duplicatedElement)) {
            m_context->error(QtXmlPatterns::tr("Complex type %1 has duplicated element %2 in its content model.")
                             .arg(formatType(m_namePool, complexType))
                             .arg(formatKeyword(duplicatedElement->displayName(m_namePool))),
                             XsdSchemaContext::XSDError, location);
            return;
         }

         if (!XsdParticleChecker::isUPAConform(complexType->contentType()->particle(), m_namePool)) {
            m_context->error(QtXmlPatterns::tr("Complex type %1 has non-deterministic content.")
                             .arg(formatType(m_namePool, complexType)),
                             XsdSchemaContext::XSDError, location);
            return;
         }
      }

      // check inheritance restrictions of complex type defined by schema constraints
      const SchemaType::Ptr baseType = complexType->wxsSuperType();

      // @see http://www.w3.org/TR/xmlschema11-1/#cos-ct-extends
      if (complexType->derivationMethod() == XsdComplexType::DerivationExtension) {
         if (baseType->isComplexType() && baseType->isDefinedBySchema()) {
            const XsdComplexType::Ptr complexBaseType = baseType;

            // we can skip 1.1 here, as it is tested in checkInheritanceRestrictions() already

            // 1.2 and 1.3
            QString errorMsg;
            if (!XsdSchemaHelper::isValidAttributeUsesExtension(complexType->attributeUses(), complexBaseType->attributeUses(),
                  complexType->attributeWildcard(), complexBaseType->attributeWildcard(), m_context, errorMsg)) {
               m_context->error(
                  QtXmlPatterns::tr("Attributes of complex type %1 are not a valid extension of the attributes of base type %2: %3.")
                  .arg(formatType(m_namePool, complexType))
                  .arg(formatType(m_namePool, baseType))
                  .arg(errorMsg),
                  XsdSchemaContext::XSDError, location);
               return;
            }

            // 1.4
            bool validContentType = false;
            if (complexType->contentType()->variety() == XsdComplexType::ContentType::Simple &&
                  complexBaseType->contentType()->variety() == XsdComplexType::ContentType::Simple) {
               if (complexType->contentType()->simpleType() == complexBaseType->contentType()->simpleType()) {
                  validContentType = true; // 1.4.1
               }
            } else if (complexType->contentType()->variety() == XsdComplexType::ContentType::Empty &&
                       complexBaseType->contentType()->variety() == XsdComplexType::ContentType::Empty) {
               validContentType = true; // 1.4.2
            } else { // 1.4.3
               if (complexType->contentType()->variety() == XsdComplexType::ContentType::ElementOnly ||
                     complexType->contentType()->variety() == XsdComplexType::ContentType::Mixed) { // 1.4.3.1
                  if (complexBaseType->contentType()->variety() == XsdComplexType::ContentType::Empty) {
                     validContentType = true; // 1.4.3.2.1
                  } else { // 1.4.3.2.2
                     if (complexType->contentType()->particle()) {  // our own check
                        if ((complexType->contentType()->variety() == XsdComplexType::ContentType::ElementOnly &&
                              complexBaseType->contentType()->variety() == XsdComplexType::ContentType::ElementOnly) ||
                              (complexType->contentType()->variety() == XsdComplexType::ContentType::Mixed &&
                               complexBaseType->contentType()->variety() == XsdComplexType::ContentType::Mixed)) {  // 1.4.3.2.2.1
                           if (isValidParticleExtension(complexType->contentType()->particle(), complexBaseType->contentType()->particle())) {
                              validContentType = true; // 1.4.3.2.2.2
                           }
                        }
                     }
                     // 1.4.3.2.2.3  and 1.4.3.2.2.4 handle 'open content' that we do not support yet
                  }
               }
            }

            // 1.5 WTF?!?

            if (!validContentType) {
               m_context->error(QtXmlPatterns::tr("Content model of complex type %1 is not a valid extension of content model of %2.")
                                .arg(formatType(m_namePool, complexType))
                                .arg(formatType(m_namePool, complexBaseType)),
                                XsdSchemaContext::XSDError, location);
               return;
            }

         } else if (baseType->isSimpleType()) {
            // 2.1
            if (complexType->contentType()->variety() != XsdComplexType::ContentType::Simple) {
               m_context->error(QtXmlPatterns::tr("Complex type %1 must have simple content.")
                                .arg(formatType(m_namePool, complexType)),
                                XsdSchemaContext::XSDError, location);
               return;
            }

            if (complexType->contentType()->simpleType() != baseType) {
               m_context->error(QtXmlPatterns::tr("Complex type %1 must have the same simple type as its base class %2.")
                                .arg(formatType(m_namePool, complexType))
                                .arg(formatType(m_namePool, baseType)),
                                XsdSchemaContext::XSDError, location);
               return;
            }

            // 2.2 tested in checkInheritanceRestrictions() already
         }
      } else if (complexType->derivationMethod() == XsdComplexType::DerivationRestriction) {
         // @see http://www.w3.org/TR/xmlschema11-1/#d0e21402
         const SchemaType::Ptr baseType(complexType->wxsSuperType());

         bool derivationOk = false;
         QString errorMsg;

         // we can partly skip 1 here, as it is tested in checkInheritanceRestrictions() already
         if (baseType->isComplexType()) {

            // 2.1
            if (baseType->name(m_namePool) == BuiltinTypes::xsAnyType->name(m_namePool)) {
               derivationOk = true;
            }

            if (baseType->isDefinedBySchema()) {
               const XsdComplexType::Ptr complexBaseType(baseType);

               // 2.2.1
               if (complexType->contentType()->variety() == XsdComplexType::ContentType::Simple) {
                  // 2.2.2.1
                  if (XsdSchemaHelper::isSimpleDerivationOk(complexType->contentType()->simpleType(),
                        complexBaseType->contentType()->simpleType(), SchemaType::DerivationConstraints())) {
                     derivationOk = true;
                  }

                  // 2.2.2.2
                  if (complexBaseType->contentType()->variety() == XsdComplexType::ContentType::Mixed) {
                     if (XsdSchemaHelper::isParticleEmptiable(complexBaseType->contentType()->particle())) {
                        derivationOk = true;
                     }
                  }
               }

               // 2.3.1
               if (complexType->contentType()->variety() == XsdComplexType::ContentType::Empty) {
                  // 2.3.2.1
                  if (complexBaseType->contentType()->variety() == XsdComplexType::ContentType::Empty) {
                     derivationOk = true;
                  }

                  // 2.3.2.2
                  if (complexBaseType->contentType()->variety() == XsdComplexType::ContentType::ElementOnly ||
                        complexBaseType->contentType()->variety() == XsdComplexType::ContentType::Mixed) {
                     if (XsdSchemaHelper::isParticleEmptiable(complexBaseType->contentType()->particle())) {
                        derivationOk = true;
                     }
                  }
               }

               // 2.4.1.1
               if (((complexType->contentType()->variety() == XsdComplexType::ContentType::ElementOnly) &&
                     (complexBaseType->contentType()->variety() == XsdComplexType::ContentType::ElementOnly ||
                      complexBaseType->contentType()->variety() == XsdComplexType::ContentType::Mixed)) ||
                     // 2.4.1.2
                     (complexType->contentType()->variety() == XsdComplexType::ContentType::Mixed &&
                      complexBaseType->contentType()->variety() == XsdComplexType::ContentType::Mixed)) {

                  // 2.4.2
                  if (XsdParticleChecker::subsumes(complexBaseType->contentType()->particle(), complexType->contentType()->particle(),
                                                   m_context, errorMsg)) {
                     derivationOk = true;
                  }
               }
            }
         }

         if (!derivationOk) {
            m_context->error(QtXmlPatterns::tr("Complex type %1 cannot be derived from base type %2%3.")
                             .arg(formatType(m_namePool, complexType))
                             .arg(formatType(m_namePool, baseType))
                             .arg(errorMsg.isEmpty() ? QString() : QLatin1String(": ") + errorMsg),
                             XsdSchemaContext::XSDError, location);
            return;
         }

         if (baseType->isDefinedBySchema()) {
            const XsdComplexType::Ptr complexBaseType(baseType);

            QString errorMsg;
            if (!XsdSchemaHelper::isValidAttributeUsesRestriction(complexType->attributeUses(), complexBaseType->attributeUses(),
                  complexType->attributeWildcard(), complexBaseType->attributeWildcard(), m_context, errorMsg)) {
               m_context->error(
                  QtXmlPatterns::tr("Attributes of complex type %1 are not a valid restriction from the attributes of base type %2: %3.")
                  .arg(formatType(m_namePool, complexType))
                  .arg(formatType(m_namePool, baseType))
                  .arg(errorMsg),
                  XsdSchemaContext::XSDError, location);
               return;
            }
         }
      }

      // check that complex type with simple content is not allowed to inherit from
      // built in complex type xs:AnyType
      if (complexType->contentType()->variety() == XsdComplexType::ContentType::Simple) {
         if (baseType->name(m_namePool) == BuiltinTypes::xsAnyType->name(m_namePool)) {
            m_context->error(QtXmlPatterns::tr("Complex type %1 with simple content cannot be derived from complex base type %2.")
                             .arg(formatType(m_namePool, complexType))
                             .arg(formatType(m_namePool, baseType)),
                             XsdSchemaContext::XSDError, location);
            return;
         }
      }
   }
}

void XsdSchemaChecker::checkSimpleDerivationRestrictions()
{
   // check all global types...
   SchemaType::List types = m_schema->types();

   // .. and anonymous types
   types << m_schema->anonymousTypes();

   for (int i = 0; i < types.count(); ++i) {
      const SchemaType::Ptr type = types.at(i);

      if (type->isComplexType()) {
         continue;
      }

      if (type->category() != SchemaType::SimpleTypeList && type->category() != SchemaType::SimpleTypeUnion) {
         continue;
      }

      const XsdSimpleType::Ptr simpleType = type;
      const QSourceLocation location = sourceLocation(simpleType);

      // check all simple types derived by list
      if (simpleType->category() == XsdSimpleType::SimpleTypeList) {
         const AnySimpleType::Ptr itemType = simpleType->itemType();

         if (itemType->isComplexType()) {
            m_context->error(QtXmlPatterns::tr("Item type of simple type %1 cannot be a complex type.")
                             .arg(formatType(m_namePool, simpleType)),
                             XsdSchemaContext::XSDError, location);
            return;
         }


         if (itemType->isSimpleType() && itemType->isDefinedBySchema()) {
            const XsdSimpleType::Ptr simpleItemType = itemType;
            if (simpleItemType->derivationConstraints() & XsdSimpleType::ListConstraint) {
               m_context->error(QtXmlPatterns::tr("%1 is not allowed to derive from %2 by list as the latter defines it as final.")
                                .arg(formatType(m_namePool, simpleType))
                                .arg(formatType(m_namePool, simpleItemType)),
                                XsdSchemaContext::XSDError, location);
               return;
            }
         }

         // @see http://www.w3.org/TR/xmlschema-2/#cos-list-of-atomic
         if (itemType->category() != SchemaType::SimpleTypeAtomic && itemType->category() != SchemaType::SimpleTypeUnion) {
            m_context->error(QtXmlPatterns::tr("Variety of item type of %1 must be either atomic or union.").arg(formatType(
                                m_namePool, simpleType)), XsdSchemaContext::XSDError, location);
            return;
         }

         if (itemType->category() == SchemaType::SimpleTypeUnion && itemType->isDefinedBySchema()) {
            const XsdSimpleType::Ptr simpleItemType = itemType;
            const AnySimpleType::List memberTypes = simpleItemType->memberTypes();
            for (int j = 0; j < memberTypes.count(); ++j) {
               if (memberTypes.at(j)->category() != SchemaType::SimpleTypeAtomic) {
                  m_context->error(QtXmlPatterns::tr("Variety of member types of %1 must be atomic.").arg(formatType(m_namePool,
                                   simpleItemType)), XsdSchemaContext::XSDError, location);
                  return;
               }
            }
         }
      }

      // check all simple types derived by union
      if (simpleType->category() == XsdSimpleType::SimpleTypeUnion) {
         const AnySimpleType::List memberTypes = simpleType->memberTypes();

         for (int i = 0; i < memberTypes.count(); ++i) {
            const AnySimpleType::Ptr memberType = memberTypes.at(i);

            if (memberType->isComplexType()) {
               m_context->error(QtXmlPatterns::tr("Member type of simple type %1 cannot be a complex type.")
                                .arg(formatType(m_namePool, simpleType)),
                                XsdSchemaContext::XSDError, location);
               return;
            }

            // @see http://www.w3.org/TR/xmlschema-2/#cos-no-circular-unions
            if (simpleType->name(m_namePool) == memberType->name(m_namePool)) {
               m_context->error(QtXmlPatterns::tr("%1 is not allowed to have a member type with the same name as itself.")
                                .arg(formatType(m_namePool, simpleType)),
                                XsdSchemaContext::XSDError, location);
               return;
            }

            if (memberType->isSimpleType() && memberType->isDefinedBySchema()) {
               const XsdSimpleType::Ptr simpleMemberType = memberType;
               if (simpleMemberType->derivationConstraints() & XsdSimpleType::UnionConstraint) {
                  m_context->error(QtXmlPatterns::tr("%1 is not allowed to derive from %2 by union as the latter defines it as final.")
                                   .arg(formatType(m_namePool, simpleType))
                                   .arg(formatType(m_namePool, simpleMemberType)),
                                   XsdSchemaContext::XSDError, location);
                  return;
               }
            }
         }
      }
   }
}

void XsdSchemaChecker::checkConstrainingFacets()
{
   // first the global simple types
   const SchemaType::List types = m_schema->types();
   for (int i = 0; i < types.count(); ++i) {
      if (!(types.at(i)->isSimpleType()) || !(types.at(i)->isDefinedBySchema())) {
         continue;
      }

      const XsdSimpleType::Ptr simpleType = types.at(i);
      checkConstrainingFacets(simpleType->facets(), simpleType);
   }

   // and afterwards all anonymous simple types
   const SchemaType::List anonymousTypes = m_schema->anonymousTypes();
   for (int i = 0; i < anonymousTypes.count(); ++i) {
      if (!(anonymousTypes.at(i)->isSimpleType()) || !(anonymousTypes.at(i)->isDefinedBySchema())) {
         continue;
      }

      const XsdSimpleType::Ptr simpleType = anonymousTypes.at(i);
      checkConstrainingFacets(simpleType->facets(), simpleType);
   }
}

void XsdSchemaChecker::checkConstrainingFacets(const XsdFacet::Hash &facets, const XsdSimpleType::Ptr &simpleType)
{
   if (facets.isEmpty()) {
      return;
   }

   SchemaType::Ptr comparableBaseType;
   if (!simpleType->wxsSuperType()->isDefinedBySchema()) {
      comparableBaseType = simpleType->wxsSuperType();
   } else {
      comparableBaseType = simpleType->primitiveType();
   }

   const XsdSchemaSourceLocationReflection reflection(sourceLocation(simpleType));

   // start checks
   if (facets.contains(XsdFacet::Length)) {
      const XsdFacet::Ptr lengthFacet = facets.value(XsdFacet::Length);
      const DerivedInteger<TypeNonNegativeInteger>::Ptr lengthValue = lengthFacet->value();

      // @see http://www.w3.org/TR/xmlschema-2/#length-minLength-maxLength
      if (facets.contains(XsdFacet::MinimumLength)) {
         const XsdFacet::Ptr minLengthFacet = facets.value(XsdFacet::MinimumLength);
         const DerivedInteger<TypeNonNegativeInteger>::Ptr minLengthValue = minLengthFacet->value();

         bool foundSuperMinimumLength = false;
         SchemaType::Ptr baseType = simpleType->wxsSuperType();
         while (baseType) {
            const XsdFacet::Hash baseFacets = m_context->facetsForType(baseType);
            if (baseFacets.contains(XsdFacet::MinimumLength) && !baseFacets.contains(XsdFacet::Length)) {
               const DerivedInteger<TypeNonNegativeInteger>::Ptr superValue(baseFacets.value(XsdFacet::MinimumLength)->value());
               if (minLengthValue->toInteger() == superValue->toInteger()) {
                  foundSuperMinimumLength = true;
                  break;
               }
            }

            baseType = baseType->wxsSuperType();
         }

         if ((minLengthValue->toInteger() > lengthValue->toInteger()) || !foundSuperMinimumLength) {
            m_context->error(QtXmlPatterns::tr("%1 facet collides with %2 facet.")
                             .arg(formatKeyword("length"))
                             .arg(formatKeyword("minLength")),
                             XsdSchemaContext::XSDError, sourceLocation(simpleType));
            return;
         }
      }

      // @see http://www.w3.org/TR/xmlschema-2/#length-minLength-maxLength
      if (facets.contains(XsdFacet::MaximumLength)) {
         const XsdFacet::Ptr maxLengthFacet = facets.value(XsdFacet::MaximumLength);
         const DerivedInteger<TypeNonNegativeInteger>::Ptr maxLengthValue = maxLengthFacet->value();

         bool foundSuperMaximumLength = false;
         SchemaType::Ptr baseType = simpleType->wxsSuperType();
         while (baseType) {
            const XsdFacet::Hash baseFacets = m_context->facetsForType(baseType);
            if (baseFacets.contains(XsdFacet::MaximumLength) && !baseFacets.contains(XsdFacet::Length)) {
               const DerivedInteger<TypeNonNegativeInteger>::Ptr superValue(baseFacets.value(XsdFacet::MaximumLength)->value());
               if (maxLengthValue->toInteger() == superValue->toInteger()) {
                  foundSuperMaximumLength = true;
                  break;
               }
            }

            baseType = baseType->wxsSuperType();
         }

         if ((maxLengthValue->toInteger() < lengthValue->toInteger()) || !foundSuperMaximumLength) {
            m_context->error(QtXmlPatterns::tr("%1 facet collides with %2 facet.")
                             .arg(formatKeyword("length"))
                             .arg(formatKeyword("maxLength")),
                             XsdSchemaContext::XSDError, sourceLocation(simpleType));
            return;
         }
      }

      // @see http://www.w3.org/TR/xmlschema-2/#length-valid-restriction
      if (simpleType->derivationMethod() == XsdSimpleType::DerivationRestriction) {
         const XsdFacet::Hash baseFacets = m_context->facetsForType(simpleType->wxsSuperType());
         if (baseFacets.contains(XsdFacet::Length)) {
            const DerivedInteger<TypeNonNegativeInteger>::Ptr baseValue = baseFacets.value(XsdFacet::Length)->value();
            if (lengthValue->toInteger() != baseValue->toInteger()) {
               m_context->error(QtXmlPatterns::tr("%1 facet must have the same value as %2 facet of base type.")
                                .arg(formatKeyword("length"))
                                .arg(formatKeyword("length")),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
               return;
            }
         }
      }
   }

   if (facets.contains(XsdFacet::MinimumLength)) {
      const XsdFacet::Ptr minLengthFacet = facets.value(XsdFacet::MinimumLength);
      const DerivedInteger<TypeNonNegativeInteger>::Ptr minLengthValue = minLengthFacet->value();

      if (facets.contains(XsdFacet::MaximumLength)) {
         const XsdFacet::Ptr maxLengthFacet = facets.value(XsdFacet::MaximumLength);
         const DerivedInteger<TypeNonNegativeInteger>::Ptr maxLengthValue = maxLengthFacet->value();

         // @see http://www.w3.org/TR/xmlschema-2/#minLength-less-than-equal-to-maxLength
         if (maxLengthValue->toInteger() < minLengthValue->toInteger()) {
            m_context->error(QtXmlPatterns::tr("%1 facet collides with %2 facet.")
                             .arg(formatKeyword("minLength"))
                             .arg(formatKeyword("maxLength")),
                             XsdSchemaContext::XSDError, sourceLocation(simpleType));
            return;
         }

         // @see http://www.w3.org/TR/xmlschema-2/#minLength-valid-restriction
         //TODO: check parent facets
      }

      // @see http://www.w3.org/TR/xmlschema-2/#minLength-valid-restriction
      if (simpleType->derivationMethod() == XsdSimpleType::DerivationRestriction) {
         const XsdFacet::Hash baseFacets = m_context->facetsForType(simpleType->wxsSuperType());
         if (baseFacets.contains(XsdFacet::MinimumLength)) {
            const DerivedInteger<TypeNonNegativeInteger>::Ptr baseValue = baseFacets.value(XsdFacet::MinimumLength)->value();
            if (minLengthValue->toInteger() < baseValue->toInteger()) {
               m_context->error(QtXmlPatterns::tr("%1 facet must be equal or greater than %2 facet of base type.")
                                .arg(formatKeyword("minLength"))
                                .arg(formatKeyword("minLength")),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
               return;
            }
         }
      }
   }
   if (facets.contains(XsdFacet::MaximumLength)) {
      const XsdFacet::Ptr maxLengthFacet = facets.value(XsdFacet::MaximumLength);
      const DerivedInteger<TypeNonNegativeInteger>::Ptr maxLengthValue = maxLengthFacet->value();

      // @see http://www.w3.org/TR/xmlschema-2/#maxLength-valid-restriction
      if (simpleType->derivationMethod() == XsdSimpleType::DerivationRestriction) {
         const XsdFacet::Hash baseFacets = m_context->facetsForType(simpleType->wxsSuperType());
         if (baseFacets.contains(XsdFacet::MaximumLength)) {
            const DerivedInteger<TypeNonNegativeInteger>::Ptr baseValue(baseFacets.value(XsdFacet::MaximumLength)->value());
            if (maxLengthValue->toInteger() > baseValue->toInteger()) {
               m_context->error(QtXmlPatterns::tr("%1 facet must be less than or equal to %2 facet of base type.")
                                .arg(formatKeyword("maxLength"))
                                .arg(formatKeyword("maxLength")),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
               return;
            }
         }
      }
   }
   if (facets.contains(XsdFacet::Pattern)) {
      //  we keep the patterns in separated facets
      // @see http://www.w3.org/TR/xmlschema-2/#src-multiple-patterns

      // @see http://www.w3.org/TR/xmlschema-2/#cvc-pattern-valid
      const XsdFacet::Ptr patternFacet = facets.value(XsdFacet::Pattern);
      const AtomicValue::List multiValue = patternFacet->multiValue();

      for (int i = 0; i < multiValue.count(); ++i) {
         const DerivedString<TypeString>::Ptr value = multiValue.at(i);
         const QRegExp exp = PatternPlatform::parsePattern(value->stringValue(), m_context, &reflection);
         if (!exp.isValid()) {
            m_context->error(QtXmlPatterns::tr("%1 facet contains invalid regular expression").arg(formatKeyword("pattern.")),
                             XsdSchemaContext::XSDError, sourceLocation(simpleType));
            return;
         }
      }
   }
   if (facets.contains(XsdFacet::Enumeration)) {
      // @see http://www.w3.org/TR/xmlschema-2/#src-multiple-enumerations

      const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);

      if (BuiltinTypes::xsNOTATION->wxsTypeMatches(simpleType)) {
         const AtomicValue::List notationNames = facet->multiValue();
         for (int k = 0; k < notationNames.count(); ++k) {
            const QNameValue::Ptr notationName = notationNames.at(k);
            if (!m_schema->notation(notationName->qName())) {
               m_context->error(QtXmlPatterns::tr("Unknown notation %1 used in %2 facet.")
                                .arg(formatKeyword(m_namePool, notationName->qName()))
                                .arg(formatKeyword("enumeration")),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
            }
         }
      } else if (BuiltinTypes::xsQName->wxsTypeMatches(simpleType)) {
      } else {
         const XsdTypeChecker checker(m_context, QVector<QXmlName>(), sourceLocation(simpleType));

         const AnySimpleType::Ptr baseType = simpleType->wxsSuperType();
         const XsdFacet::Hash baseFacets = XsdTypeChecker::mergedFacetsForType(baseType, m_context);

         const AtomicValue::List multiValue = facet->multiValue();
         for (int k = 0; k < multiValue.count(); ++k) {
            const QString stringValue = multiValue.at(k)->as<DerivedString<TypeString> >()->stringValue();
            const QString actualValue = XsdTypeChecker::normalizedValue(stringValue, baseFacets);

            QString errorMsg;
            if (!checker.isValidString(actualValue, baseType, errorMsg)) {
               m_context->error(QtXmlPatterns::tr("%1 facet contains invalid value %2: %3.")
                                .arg(formatKeyword("enumeration"))
                                .arg(formatData(stringValue))
                                .arg(errorMsg),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
               return;
            }
         }
      }
   }
   if (facets.contains(XsdFacet::WhiteSpace)) {
      const XsdFacet::Ptr whiteSpaceFacet = facets.value(XsdFacet::WhiteSpace);
      const DerivedString<TypeString>::Ptr whiteSpaceValue = whiteSpaceFacet->value();

      // @see http://www.w3.org/TR/xmlschema-2/#whiteSpace-valid-restriction
      if (simpleType->derivationMethod() == XsdSimpleType::DerivationRestriction) {
         const XsdFacet::Hash baseFacets = m_context->facetsForType(simpleType->wxsSuperType());
         if (baseFacets.contains(XsdFacet::WhiteSpace)) {
            const QString value = whiteSpaceValue->stringValue();
            const QString baseValue = DerivedString<TypeString>::Ptr(baseFacets.value(
                                         XsdFacet::WhiteSpace)->value())->stringValue();
            if (value == XsdSchemaToken::toString(XsdSchemaToken::Replace) ||
                  value == XsdSchemaToken::toString(XsdSchemaToken::Preserve)) {
               if (baseValue == XsdSchemaToken::toString(XsdSchemaToken::Collapse)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet cannot be %2 or %3 if %4 facet of base type is %5.")
                                   .arg(formatKeyword("whiteSpace"))
                                   .arg(formatData("replace"))
                                   .arg(formatData("preserve"))
                                   .arg(formatKeyword("whiteSpace"))
                                   .arg(formatData("collapse")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
            if (value == XsdSchemaToken::toString(XsdSchemaToken::Preserve) &&
                  baseValue == XsdSchemaToken::toString(XsdSchemaToken::Replace)) {
               m_context->error(QtXmlPatterns::tr("%1 facet cannot be %2 if %3 facet of base type is %4.")
                                .arg(formatKeyword("whiteSpace"))
                                .arg(formatData("preserve"))
                                .arg(formatKeyword("whiteSpace"))
                                .arg(formatData("replace")),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
               return;
            }
         }
      }
   }
   if (facets.contains(XsdFacet::MaximumInclusive)) {
      const XsdFacet::Ptr maxFacet = facets.value(XsdFacet::MaximumInclusive);

      // @see http://www.w3.org/TR/xmlschema-2/#minInclusive-less-than-equal-to-maxInclusive
      if (facets.contains(XsdFacet::MinimumInclusive)) {
         const XsdFacet::Ptr minFacet = facets.value(XsdFacet::MinimumInclusive);

         if (comparableBaseType) {
            if (XsdSchemaHelper::constructAndCompare(minFacet->value(), AtomicComparator::OperatorGreaterThan, maxFacet->value(),
                  comparableBaseType, m_context, &reflection)) {
               m_context->error(QtXmlPatterns::tr("%1 facet must be less than or equal to %2 facet.")
                                .arg(formatKeyword("minInclusive"))
                                .arg(formatKeyword("maxInclusive")),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
               return;
            }
         }
      }

      // @see http://www.w3.org/TR/xmlschema-2/#maxInclusive-valid-restriction
      if (simpleType->derivationMethod() == XsdSimpleType::DerivationRestriction) {
         const XsdFacet::Hash baseFacets = m_context->facetsForType(simpleType->wxsSuperType());
         if (baseFacets.contains(XsdFacet::MaximumInclusive)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::MaximumInclusive);
            if (comparableBaseType) {
               if (XsdSchemaHelper::constructAndCompare(maxFacet->value(), AtomicComparator::OperatorGreaterThan, baseFacet->value(),
                     comparableBaseType, m_context, &reflection)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet must be less than or equal to %2 facet of base type.")
                                   .arg(formatKeyword("maxInclusive"))
                                   .arg(formatKeyword("maxInclusive")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
         if (baseFacets.contains(XsdFacet::MaximumExclusive)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::MaximumExclusive);
            if (comparableBaseType) {
               if (XsdSchemaHelper::constructAndCompare(maxFacet->value(), AtomicComparator::OperatorGreaterOrEqual,
                     baseFacet->value(), comparableBaseType, m_context, &reflection)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet must be less than %2 facet of base type.")
                                   .arg(formatKeyword("maxInclusive"))
                                   .arg(formatKeyword("maxExclusive")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
      }
   }
   if (facets.contains(XsdFacet::MaximumExclusive)) {
      const XsdFacet::Ptr maxFacet = facets.value(XsdFacet::MaximumExclusive);

      // @see http://www.w3.org/TR/xmlschema-2/#maxInclusive-maxExclusive
      if (facets.contains(XsdFacet::MaximumInclusive)) {
         m_context->error(QtXmlPatterns::tr("%1 facet and %2 facet cannot appear together.")
                          .arg(formatKeyword("maxExclusive"))
                          .arg(formatKeyword("maxInclusive")),
                          XsdSchemaContext::XSDError, sourceLocation(simpleType));
         return;
      }

      // @see http://www.w3.org/TR/xmlschema-2/#minExclusive-less-than-equal-to-maxExclusive
      if (facets.contains(XsdFacet::MinimumExclusive)) {
         const XsdFacet::Ptr minFacet = facets.value(XsdFacet::MinimumExclusive);
         if (comparableBaseType) {
            if (XsdSchemaHelper::constructAndCompare(minFacet->value(), AtomicComparator::OperatorGreaterThan, maxFacet->value(),
                  comparableBaseType, m_context, &reflection)) {
               m_context->error(QtXmlPatterns::tr("%1 facet must be less than or equal to %2 facet.")
                                .arg(formatKeyword("minExclusive"))
                                .arg(formatKeyword("maxExclusive")),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
               return;
            }
         }
      }

      // @see http://www.w3.org/TR/xmlschema-2/#maxExclusive-valid-restriction
      if (simpleType->derivationMethod() == XsdSimpleType::DerivationRestriction) {
         const XsdFacet::Hash baseFacets = m_context->facetsForType(simpleType->wxsSuperType());
         if (baseFacets.contains(XsdFacet::MaximumExclusive)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::MaximumExclusive);
            if (comparableBaseType) {
               if (XsdSchemaHelper::constructAndCompare(maxFacet->value(), AtomicComparator::OperatorGreaterThan, baseFacet->value(),
                     comparableBaseType, m_context, &reflection)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet must be less than or equal to %2 facet of base type.")
                                   .arg(formatKeyword("maxExclusive"))
                                   .arg(formatKeyword("maxExclusive")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
         if (baseFacets.contains(XsdFacet::MaximumInclusive)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::MaximumInclusive);
            if (comparableBaseType) {
               if (XsdSchemaHelper::constructAndCompare(maxFacet->value(), AtomicComparator::OperatorGreaterThan, baseFacet->value(),
                     comparableBaseType, m_context, &reflection)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet must be less than or equal to %2 facet of base type.")
                                   .arg(formatKeyword("maxExclusive"))
                                   .arg(formatKeyword("maxInclusive")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
         if (baseFacets.contains(XsdFacet::MinimumInclusive)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::MinimumInclusive);
            if (comparableBaseType) {
               if (XsdSchemaHelper::constructAndCompare(maxFacet->value(), AtomicComparator::OperatorLessOrEqual, baseFacet->value(),
                     comparableBaseType, m_context, &reflection)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet must be greater than %2 facet of base type.")
                                   .arg(formatKeyword("maxExclusive"))
                                   .arg(formatKeyword("minInclusive")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
         if (baseFacets.contains(XsdFacet::MinimumExclusive)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::MinimumExclusive);
            if (comparableBaseType) {
               if (XsdSchemaHelper::constructAndCompare(maxFacet->value(), AtomicComparator::OperatorLessOrEqual, baseFacet->value(),
                     comparableBaseType, m_context, &reflection)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet must be greater than %2 facet of base type.")
                                   .arg(formatKeyword("maxExclusive"))
                                   .arg(formatKeyword("minExclusive")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
      }
   }
   if (facets.contains(XsdFacet::MinimumExclusive)) {
      const XsdFacet::Ptr minFacet = facets.value(XsdFacet::MinimumExclusive);

      // @see http://www.w3.org/TR/xmlschema-2/#minInclusive-minExclusive
      if (facets.contains(XsdFacet::MinimumInclusive)) {
         m_context->error(QtXmlPatterns::tr("%1 facet and %2 facet cannot appear together.")
                          .arg(formatKeyword("minExclusive"))
                          .arg(formatKeyword("minInclusive")),
                          XsdSchemaContext::XSDError, sourceLocation(simpleType));
         return;
      }

      // @see http://www.w3.org/TR/xmlschema-2/#minExclusive-less-than-maxInclusive
      if (facets.contains(XsdFacet::MaximumInclusive)) {
         const XsdFacet::Ptr maxFacet = facets.value(XsdFacet::MaximumInclusive);
         if (comparableBaseType) {
            if (XsdSchemaHelper::constructAndCompare(minFacet->value(), AtomicComparator::OperatorGreaterOrEqual, maxFacet->value(),
                  comparableBaseType, m_context, &reflection)) {
               m_context->error(QtXmlPatterns::tr("%1 facet must be less than %2 facet.")
                                .arg(formatKeyword("minExclusive"))
                                .arg(formatKeyword("maxInclusive")),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
               return;
            }
         }
      }

      // @see http://www.w3.org/TR/xmlschema-2/#minExclusive-valid-restriction
      if (simpleType->derivationMethod() == XsdSimpleType::DerivationRestriction) {
         const XsdFacet::Hash baseFacets = m_context->facetsForType(simpleType->wxsSuperType());
         if (baseFacets.contains(XsdFacet::MinimumExclusive)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::MinimumExclusive);
            if (comparableBaseType) {
               if (XsdSchemaHelper::constructAndCompare(minFacet->value(), AtomicComparator::OperatorLessThan, baseFacet->value(),
                     comparableBaseType, m_context, &reflection)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet must be greater than or equal to %2 facet of base type.")
                                   .arg(formatKeyword("minExclusive"))
                                   .arg(formatKeyword("minExclusive")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
         if (baseFacets.contains(XsdFacet::MaximumExclusive)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::MaximumExclusive);
            if (comparableBaseType) {
               if (XsdSchemaHelper::constructAndCompare(minFacet->value(), AtomicComparator::OperatorGreaterOrEqual,
                     baseFacet->value(), comparableBaseType, m_context, &reflection)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet must be less than %2 facet of base type.")
                                   .arg(formatKeyword("minExclusive"))
                                   .arg(formatKeyword("maxExclusive")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
         if (baseFacets.contains(XsdFacet::MaximumInclusive)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::MaximumInclusive);
            if (comparableBaseType) {
               if (XsdSchemaHelper::constructAndCompare(minFacet->value(), AtomicComparator::OperatorGreaterThan, baseFacet->value(),
                     comparableBaseType, m_context, &reflection)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet must be less than or equal to %2 facet of base type.")
                                   .arg(formatKeyword("minExclusive"))
                                   .arg(formatKeyword("maxInclusive")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
      }
   }
   if (facets.contains(XsdFacet::MinimumInclusive)) {
      const XsdFacet::Ptr minFacet = facets.value(XsdFacet::MinimumInclusive);

      // @see http://www.w3.org/TR/xmlschema-2/#minInclusive-less-than-maxExclusive
      if (facets.contains(XsdFacet::MaximumExclusive)) {
         const XsdFacet::Ptr maxFacet = facets.value(XsdFacet::MaximumExclusive);
         if (comparableBaseType) {
            if (XsdSchemaHelper::constructAndCompare(minFacet->value(), AtomicComparator::OperatorGreaterOrEqual, maxFacet->value(),
                  comparableBaseType, m_context, &reflection)) {
               m_context->error(QtXmlPatterns::tr("%1 facet must be less than %2 facet.")
                                .arg(formatKeyword("minInclusive"))
                                .arg(formatKeyword("maxExclusive")),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
               return;
            }
         }
      }

      // @see http://www.w3.org/TR/xmlschema-2/#minInclusive-valid-restriction
      if (simpleType->derivationMethod() == XsdSimpleType::DerivationRestriction) {
         const XsdFacet::Hash baseFacets = m_context->facetsForType(simpleType->wxsSuperType());
         if (baseFacets.contains(XsdFacet::MinimumInclusive)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::MinimumInclusive);
            if (comparableBaseType) {
               if (XsdSchemaHelper::constructAndCompare(minFacet->value(), AtomicComparator::OperatorLessThan, baseFacet->value(),
                     comparableBaseType, m_context, &reflection)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet must be greater than or equal to %2 facet of base type.")
                                   .arg(formatKeyword("minInclusive"))
                                   .arg(formatKeyword("minInclusive")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
         if (baseFacets.contains(XsdFacet::MinimumExclusive)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::MinimumExclusive);
            if (comparableBaseType) {
               if (XsdSchemaHelper::constructAndCompare(minFacet->value(), AtomicComparator::OperatorLessOrEqual, baseFacet->value(),
                     comparableBaseType, m_context, &reflection)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet must be greater than %2 facet of base type.")
                                   .arg(formatKeyword("minInclusive"))
                                   .arg(formatKeyword("minExclusive")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
         if (baseFacets.contains(XsdFacet::MaximumInclusive)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::MaximumInclusive);
            if (comparableBaseType) {
               if (XsdSchemaHelper::constructAndCompare(minFacet->value(), AtomicComparator::OperatorGreaterThan, baseFacet->value(),
                     comparableBaseType, m_context, &reflection)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet must be less than or equal to %2 facet of base type.")
                                   .arg(formatKeyword("minInclusive"))
                                   .arg(formatKeyword("maxInclusive")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
         if (baseFacets.contains(XsdFacet::MaximumExclusive)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::MaximumExclusive);
            if (comparableBaseType) {
               if (XsdSchemaHelper::constructAndCompare(minFacet->value(), AtomicComparator::OperatorGreaterOrEqual,
                     baseFacet->value(), comparableBaseType, m_context, &reflection)) {
                  m_context->error(QtXmlPatterns::tr("%1 facet must be less than %2 facet of base type.")
                                   .arg(formatKeyword("minInclusive"))
                                   .arg(formatKeyword("maxExclusive")),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
      }
   }
   if (facets.contains(XsdFacet::TotalDigits)) {
      const XsdFacet::Ptr totalDigitsFacet = facets.value(XsdFacet::TotalDigits);
      const DerivedInteger<TypeNonNegativeInteger>::Ptr totalDigitsValue = totalDigitsFacet->value();

      // @see http://www.w3.org/TR/xmlschema-2/#totalDigits-valid-restriction
      if (simpleType->derivationMethod() == XsdSimpleType::DerivationRestriction) {
         const XsdFacet::Hash baseFacets = m_context->facetsForType(simpleType->wxsSuperType());
         if (baseFacets.contains(XsdFacet::TotalDigits)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::TotalDigits);
            const DerivedInteger<TypeNonNegativeInteger>::Ptr baseValue = baseFacet->value();

            if (totalDigitsValue->toInteger() > baseValue->toInteger()) {
               m_context->error(QtXmlPatterns::tr("%1 facet must be less than or equal to %2 facet of base type.")
                                .arg(formatKeyword("totalDigits"))
                                .arg(formatKeyword("totalDigits")),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
               return;
            }
         }
      }
   }
   if (facets.contains(XsdFacet::FractionDigits)) {
      const XsdFacet::Ptr fractionDigitsFacet = facets.value(XsdFacet::FractionDigits);
      const DerivedInteger<TypeNonNegativeInteger>::Ptr fractionDigitsValue = fractionDigitsFacet->value();

      // http://www.w3.org/TR/xmlschema-2/#fractionDigits-totalDigits
      if (facets.contains(XsdFacet::TotalDigits)) {
         const XsdFacet::Ptr totalDigitsFacet = facets.value(XsdFacet::TotalDigits);
         const DerivedInteger<TypeNonNegativeInteger>::Ptr totalDigitsValue = totalDigitsFacet->value();

         if (fractionDigitsValue->toInteger() > totalDigitsValue->toInteger()) {
            m_context->error(QtXmlPatterns::tr("%1 facet must be less than or equal to %2 facet.")
                             .arg(formatKeyword("fractionDigits"))
                             .arg(formatKeyword("totalDigits")),
                             XsdSchemaContext::XSDError, sourceLocation(simpleType));
            return;
         }
      }

      // @see http://www.w3.org/TR/xmlschema-2/#fractionDigits-valid-restriction
      if (simpleType->derivationMethod() == XsdSimpleType::DerivationRestriction) {
         const XsdFacet::Hash baseFacets = m_context->facetsForType(simpleType->wxsSuperType());
         if (baseFacets.contains(XsdFacet::FractionDigits)) {
            const XsdFacet::Ptr baseFacet = baseFacets.value(XsdFacet::FractionDigits);
            const DerivedInteger<TypeNonNegativeInteger>::Ptr baseValue = baseFacet->value();

            if (fractionDigitsValue->toInteger() > baseValue->toInteger()) {
               m_context->error(QtXmlPatterns::tr("%1 facet must be less than or equal to %2 facet of base type.")
                                .arg(formatKeyword("fractionDigits"))
                                .arg(formatKeyword("fractionDigits")),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
               return;
            }
         }
      }
   }


   // check whether facets are allowed for simple types variety
   if (simpleType->wxsSuperType()->category() == SchemaType::SimpleTypeAtomic) {
      if (simpleType->primitiveType()) {
         const QXmlName primitiveTypeName = simpleType->primitiveType()->name(m_namePool);
         if (m_allowedAtomicFacets.contains(primitiveTypeName)) {
            const QSet<XsdFacet::Type> allowedFacets = m_allowedAtomicFacets.value(primitiveTypeName);
            QSet<XsdFacet::Type> availableFacets = facets.keys().toSet();

            if (!availableFacets.subtract(allowedFacets).isEmpty()) {
               m_context->error(QtXmlPatterns::tr("Simple type contains not allowed facet %1.")
                                .arg(formatKeyword(XsdFacet::typeName(availableFacets.toList().first()))),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
               return;
            }
         }
      }
   } else if (simpleType->wxsSuperType()->category() == SchemaType::SimpleTypeList) {
      if (facets.contains(XsdFacet::MaximumInclusive) || facets.contains(XsdFacet::MinimumInclusive) ||
            facets.contains(XsdFacet::MaximumExclusive) || facets.contains(XsdFacet::MinimumExclusive) ||
            facets.contains(XsdFacet::TotalDigits) || facets.contains(XsdFacet::FractionDigits)) {
         m_context->error(QtXmlPatterns::tr("%1, %2, %3, %4, %5 and %6 facets are not allowed when derived by list.")
                          .arg(formatKeyword("maxInclusive"))
                          .arg(formatKeyword("maxExclusive"))
                          .arg(formatKeyword("minInclusive"))
                          .arg(formatKeyword("minExclusive"))
                          .arg(formatKeyword("totalDigits"))
                          .arg(formatKeyword("fractionDigits")),
                          XsdSchemaContext::XSDError, sourceLocation(simpleType));
      }
   } else if (simpleType->wxsSuperType()->category() == SchemaType::SimpleTypeUnion) {
      if (facets.contains(XsdFacet::MaximumInclusive) || facets.contains(XsdFacet::MinimumInclusive) ||
            facets.contains(XsdFacet::MaximumExclusive) || facets.contains(XsdFacet::MinimumExclusive) ||
            facets.contains(XsdFacet::TotalDigits) || facets.contains(XsdFacet::FractionDigits) ||
            facets.contains(XsdFacet::MinimumLength) || facets.contains(XsdFacet::MaximumLength) ||
            facets.contains(XsdFacet::Length) || facets.contains(XsdFacet::WhiteSpace)) {
         m_context->error(QtXmlPatterns::tr("Only %1 and %2 facets are allowed when derived by union.")
                          .arg(formatKeyword("pattern"))
                          .arg(formatKeyword("enumeration")),
                          XsdSchemaContext::XSDError, sourceLocation(simpleType));
      }
   }

   // check whether value of facet matches the value space of the simple types base type
   const SchemaType::Ptr baseType = simpleType->wxsSuperType();
   if (!baseType->isDefinedBySchema()) {
      const XsdSchemaSourceLocationReflection reflection(sourceLocation(simpleType));

      XsdFacet::HashIterator it(facets);
      while (it.hasNext()) {
         it.next();
         const XsdFacet::Ptr facet = it.value();
         if (facet->type() == XsdFacet::MaximumInclusive ||
               facet->type() == XsdFacet::MaximumExclusive ||
               facet->type() == XsdFacet::MinimumInclusive ||
               facet->type() == XsdFacet::MinimumExclusive) {
            const DerivedString<TypeString>::Ptr stringValue = facet->value();
            const AtomicValue::Ptr value = ValueFactory::fromLexical(stringValue->stringValue(), baseType, m_context, &reflection);
            if (value->hasError()) {
               m_context->error(QtXmlPatterns::tr("%1 contains %2 facet with invalid data: %3.")
                                .arg(formatType(m_namePool, simpleType))
                                .arg(formatKeyword(XsdFacet::typeName(facet->type())))
                                .arg(formatData(stringValue->stringValue())),
                                XsdSchemaContext::XSDError, sourceLocation(simpleType));
               return;
            }
         }

         // @see http://www.w3.org/TR/xmlschema-2/#enumeration-valid-restriction
         if (facet->type() == XsdFacet::Enumeration && baseType != BuiltinTypes::xsNOTATION) {
            const AtomicValue::List multiValue = facet->multiValue();
            for (int j = 0; j < multiValue.count(); ++j) {
               const QString stringValue = DerivedString<TypeString>::Ptr(multiValue.at(j))->stringValue();
               const AtomicValue::Ptr value = ValueFactory::fromLexical(stringValue, baseType, m_context, &reflection);
               if (value->hasError()) {
                  m_context->error(QtXmlPatterns::tr("%1 contains %2 facet with invalid data: %3.")
                                   .arg(formatType(m_namePool, simpleType))
                                   .arg(formatKeyword(XsdFacet::typeName(XsdFacet::Enumeration)))
                                   .arg(formatData(stringValue)),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }
            }
         }
      }
   }
}

void XsdSchemaChecker::checkDuplicatedAttributeUses()
{
   // first all global attribute groups
   const XsdAttributeGroup::List attributeGroups = m_schema->attributeGroups();
   for (int i = 0; i < attributeGroups.count(); ++i) {
      const XsdAttributeGroup::Ptr attributeGroup = attributeGroups.at(i);
      const XsdAttributeUse::List uses = attributeGroup->attributeUses();

      // @see http://www.w3.org/TR/xmlschema11-1/#ct-props-correct 4)
      XsdAttribute::Ptr conflictingAttribute;
      if (hasDuplicatedAttributeUses(uses, conflictingAttribute)) {
         m_context->error(QtXmlPatterns::tr("Attribute group %1 contains attribute %2 twice.")
                          .arg(formatKeyword(attributeGroup->displayName(m_namePool)))
                          .arg(formatKeyword(conflictingAttribute->displayName(m_namePool))),
                          XsdSchemaContext::XSDError, sourceLocation(attributeGroup));
         return;
      }

      // @see http://www.w3.org/TR/xmlschema11-1/#ct-props-correct 5)
      if (hasMultipleIDAttributeUses(uses)) {
         m_context->error(
            QtXmlPatterns::tr("Attribute group %1 contains two different attributes that both have types derived from %2.")
            .arg(formatKeyword(attributeGroup->displayName(m_namePool)))
            .arg(formatType(m_namePool, BuiltinTypes::xsID)),
            XsdSchemaContext::XSDError, sourceLocation(attributeGroup));
         return;
      }

      if (hasConstraintIDAttributeUse(uses, conflictingAttribute)) {
         m_context->error(
            QtXmlPatterns::tr("Attribute group %1 contains attribute %2 that has value constraint but type that inherits from %3.")
            .arg(formatKeyword(attributeGroup->displayName(m_namePool)))
            .arg(formatKeyword(conflictingAttribute->displayName(m_namePool)))
            .arg(formatType(m_namePool, BuiltinTypes::xsID)),
            XsdSchemaContext::XSDError, sourceLocation(attributeGroup));
         return;
      }
   }

   // then the global and anonymous complex types
   SchemaType::List types = m_schema->types();
   types << m_schema->anonymousTypes();

   for (int i = 0; i < types.count(); ++i) {
      if (!(types.at(i)->isComplexType()) || !types.at(i)->isDefinedBySchema()) {
         continue;
      }

      const XsdComplexType::Ptr complexType = types.at(i);
      const XsdAttributeUse::List attributeUses = complexType->attributeUses();

      // @see http://www.w3.org/TR/xmlschema11-1/#ct-props-correct 4)
      XsdAttribute::Ptr conflictingAttribute;
      if (hasDuplicatedAttributeUses(attributeUses, conflictingAttribute)) {
         m_context->error(QtXmlPatterns::tr("Complex type %1 contains attribute %2 twice.")
                          .arg(formatType(m_namePool, complexType))
                          .arg(formatKeyword(conflictingAttribute->displayName(m_namePool))),
                          XsdSchemaContext::XSDError, sourceLocation(complexType));
         return;
      }

      // @see http://www.w3.org/TR/xmlschema11-1/#ct-props-correct 5)
      if (hasMultipleIDAttributeUses(attributeUses)) {
         m_context->error(
            QtXmlPatterns::tr("Complex type %1 contains two different attributes that both have types derived from %2.")
            .arg(formatType(m_namePool, complexType))
            .arg(formatType(m_namePool, BuiltinTypes::xsID)),
            XsdSchemaContext::XSDError, sourceLocation(complexType));
         return;
      }

      if (hasConstraintIDAttributeUse(attributeUses, conflictingAttribute)) {
         m_context->error(
            QtXmlPatterns::tr("Complex type %1 contains attribute %2 that has value constraint but type that inherits from %3.")
            .arg(formatType(m_namePool, complexType))
            .arg(formatKeyword(conflictingAttribute->displayName(m_namePool)))
            .arg(formatType(m_namePool, BuiltinTypes::xsID)),
            XsdSchemaContext::XSDError, sourceLocation(complexType));
         return;
      }
   }
}

void XsdSchemaChecker::checkElementConstraints()
{
   const QSet<XsdElement::Ptr> elements = collectAllElements(m_schema);
   QSetIterator<XsdElement::Ptr> it(elements);
   while (it.hasNext()) {
      const XsdElement::Ptr element = it.next();

      // @see http://www.w3.org/TR/xmlschema11-1/#e-props-correct

      // 2 and xs:ID check
      if (element->valueConstraint()) {
         const SchemaType::Ptr type = element->type();

         AnySimpleType::Ptr targetType;
         if (type->isSimpleType() && type->category() == SchemaType::SimpleTypeAtomic) {
            targetType = type;

            // if it is a XsdSimpleType, use its primitive type as target type
            if (type->isDefinedBySchema()) {
               targetType = XsdSimpleType::Ptr(type)->primitiveType();
            }

         } else if (type->isComplexType() && type->isDefinedBySchema()) {
            const XsdComplexType::Ptr complexType(type);

            if (complexType->contentType()->variety() == XsdComplexType::ContentType::Simple) {
               const AnySimpleType::Ptr simpleType = complexType->contentType()->simpleType();
               if (simpleType->category() == AnySimpleType::SimpleTypeAtomic) {
                  targetType = simpleType;

                  if (simpleType->isDefinedBySchema()) {
                     targetType = XsdSimpleType::Ptr(simpleType)->primitiveType();
                  }
               }
            } else if (complexType->contentType()->variety() != XsdComplexType::ContentType::Mixed) {
               m_context->error(QtXmlPatterns::tr("Element %1 is not allowed to have a value constraint if its base type is complex.")
                                .arg(formatKeyword(element->displayName(m_namePool))),
                                XsdSchemaContext::XSDError, sourceLocation(element));
               return;
            }
         }
         if ((targetType == BuiltinTypes::xsID) || BuiltinTypes::xsID->wxsTypeMatches(type)) {
            m_context->error(
               QtXmlPatterns::tr("Element %1 is not allowed to have a value constraint if its type is derived from %2.")
               .arg(formatKeyword(element->displayName(m_namePool)))
               .arg(formatType(m_namePool, BuiltinTypes::xsID)),
               XsdSchemaContext::XSDError, sourceLocation(element));
            return;
         }

         if (type->isSimpleType()) {
            QString errorMsg;
            if (!isValidValue(element->valueConstraint()->value(), type, errorMsg)) {
               m_context->error(QtXmlPatterns::tr("Value constraint of element %1 is not of elements type: %2.")
                                .arg(formatKeyword(element->displayName(m_namePool)))
                                .arg(errorMsg),
                                XsdSchemaContext::XSDError, sourceLocation(element));
               return;
            }
         } else if (type->isComplexType() && type->isDefinedBySchema()) {
            const XsdComplexType::Ptr complexType(type);
            if (complexType->contentType()->variety() == XsdComplexType::ContentType::Simple) {
               QString errorMsg;
               if (!isValidValue(element->valueConstraint()->value(), complexType->contentType()->simpleType(), errorMsg)) {
                  m_context->error(QtXmlPatterns::tr("Value constraint of element %1 is not of elements type: %2.")
                                   .arg(formatKeyword(element->displayName(m_namePool)))
                                   .arg(errorMsg),
                                   XsdSchemaContext::XSDError, sourceLocation(element));
                  return;
               }
            }
         }
      }

      if (!element->substitutionGroupAffiliations().isEmpty()) {
         // 3
         if (!element->scope() || element->scope()->variety() != XsdElement::Scope::Global) {
            m_context->error(
               QtXmlPatterns::tr("Element %1 is not allowed to have substitution group affiliation as it is no global element.").arg(
                  formatKeyword(element->displayName(m_namePool))),
               XsdSchemaContext::XSDError, sourceLocation(element));
            return;
         }

         // 4
         const XsdElement::List affiliations = element->substitutionGroupAffiliations();
         for (int i = 0; i < affiliations.count(); ++i) {
            const XsdElement::Ptr affiliation = affiliations.at(i);

            bool derivationOk = false;
            if (element->type()->isComplexType() && affiliation->type()->isComplexType()) {
               if (XsdSchemaHelper::isComplexDerivationOk(element->type(), affiliation->type(),
                     affiliation->substitutionGroupExclusions())) {
                  derivationOk = true;
               }
            }
            if (element->type()->isComplexType() && affiliation->type()->isSimpleType()) {
               if (XsdSchemaHelper::isComplexDerivationOk(element->type(), affiliation->type(),
                     affiliation->substitutionGroupExclusions())) {
                  derivationOk = true;
               }
            }
            if (element->type()->isSimpleType()) {
               if (XsdSchemaHelper::isSimpleDerivationOk(element->type(), affiliation->type(),
                     affiliation->substitutionGroupExclusions())) {
                  derivationOk = true;
               }
            }

            if (!derivationOk) {
               m_context->error(
                  QtXmlPatterns::tr("Type of element %1 cannot be derived from type of substitution group affiliation.").arg(
                     formatKeyword(element->displayName(m_namePool))),
                  XsdSchemaContext::XSDError, sourceLocation(element));
               return;
            }
         }

         // 5 was checked in XsdSchemaResolver::resolveSubstitutionGroupAffiliations() already
      }
   }
}

void XsdSchemaChecker::checkAttributeConstraints()
{
   // all global attributes
   XsdAttribute::List attributes = m_schema->attributes();

   // and all local attributes
   SchemaType::List types = m_schema->types();
   types << m_schema->anonymousTypes();

   for (int i = 0; i < types.count(); ++i) {
      if (!types.at(i)->isComplexType() || !types.at(i)->isDefinedBySchema()) {
         continue;
      }

      const XsdComplexType::Ptr complexType(types.at(i));
      const XsdAttributeUse::List uses = complexType->attributeUses();
      for (int j = 0; j < uses.count(); ++j) {
         attributes.append(uses.at(j)->attribute());
      }
   }

   for (int i = 0; i < attributes.count(); ++i) {
      const XsdAttribute::Ptr attribute = attributes.at(i);

      if (!attribute->valueConstraint()) {
         continue;
      }

      if (attribute->valueConstraint()->variety() == XsdAttribute::ValueConstraint::Default ||
            attribute->valueConstraint()->variety() == XsdAttribute::ValueConstraint::Fixed) {
         const SchemaType::Ptr type = attribute->type();

         QString errorMsg;
         if (!isValidValue(attribute->valueConstraint()->value(), attribute->type(), errorMsg)) {
            m_context->error(QtXmlPatterns::tr("Value constraint of attribute %1 is not of attributes type: %2.")
                             .arg(formatKeyword(attribute->displayName(m_namePool)))
                             .arg(errorMsg),
                             XsdSchemaContext::XSDError, sourceLocation(attribute));
            return;
         }
      }

      if (BuiltinTypes::xsID->wxsTypeMatches(attribute->type())) {
         m_context->error(QtXmlPatterns::tr("Attribute %1 has value constraint but has type derived from %2.")
                          .arg(formatKeyword(attribute->displayName(m_namePool)))
                          .arg(formatType(m_namePool, BuiltinTypes::xsID)),
                          XsdSchemaContext::XSDError, sourceLocation(attribute));
         return;
      }
   }
}

bool XsdSchemaChecker::isValidValue(const QString &stringValue, const AnySimpleType::Ptr &type, QString &errorMsg) const
{
   if (BuiltinTypes::xsAnySimpleType->name(m_namePool) == type->name(m_namePool)) {
      return true;   // no need to check xs:anyType content
   }

   const XsdFacet::Hash facets = XsdTypeChecker::mergedFacetsForType(type, m_context);
   const QString actualValue = XsdTypeChecker::normalizedValue(stringValue, facets);

   const XsdTypeChecker checker(m_context, QVector<QXmlName>(), QSourceLocation(QUrl(QLatin1String("http://dummy.org")), 1,
                                1));
   return checker.isValidString(actualValue, type, errorMsg);
}

void XsdSchemaChecker::checkAttributeUseConstraints()
{
   XsdComplexType::List complexTypes;

   SchemaType::List types = m_schema->types();
   types << m_schema->anonymousTypes();

   for (int i = 0; i < types.count(); ++i) {
      const SchemaType::Ptr type = types.at(i);
      if (type->isComplexType() && type->isDefinedBySchema()) {
         complexTypes.append(XsdComplexType::Ptr(type));
      }
   }

   for (int i = 0; i < complexTypes.count(); ++i) {
      const XsdComplexType::Ptr complexType(complexTypes.at(i));
      const SchemaType::Ptr baseType = complexType->wxsSuperType();
      if (!baseType || !baseType->isComplexType() || !baseType->isDefinedBySchema()) {
         continue;
      }

      const XsdComplexType::Ptr complexBaseType(baseType);

      const XsdAttributeUse::List attributeUses = complexType->attributeUses();
      QHash<QXmlName, XsdAttributeUse::Ptr> lookupHash;
      for (int j = 0; j < attributeUses.count(); ++j) {
         lookupHash.insert(attributeUses.at(j)->attribute()->name(m_namePool), attributeUses.at(j));
      }

      const XsdAttributeUse::List baseAttributeUses = complexBaseType->attributeUses();
      for (int j = 0; j < baseAttributeUses.count(); ++j) {
         const XsdAttributeUse::Ptr baseAttributeUse = baseAttributeUses.at(j);

         if (lookupHash.contains(baseAttributeUse->attribute()->name(m_namePool))) {
            const XsdAttributeUse::Ptr attributeUse = lookupHash.value(baseAttributeUse->attribute()->name(m_namePool));

            if (baseAttributeUse->useType() == XsdAttributeUse::RequiredUse) {
               if (attributeUse->useType() == XsdAttributeUse::OptionalUse ||
                     attributeUse->useType() == XsdAttributeUse::ProhibitedUse) {
                  m_context->error(QtXmlPatterns::tr("%1 attribute in derived complex type must be %2 like in base type.")
                                   .arg(formatAttribute("use"))
                                   .arg(formatData("required")),
                                   XsdSchemaContext::XSDError, sourceLocation(complexType));
                  return;
               }
            }

            if (baseAttributeUse->valueConstraint()) {
               if (baseAttributeUse->valueConstraint()->variety() == XsdAttributeUse::ValueConstraint::Fixed) {
                  if (!attributeUse->valueConstraint()) {
                     m_context->error(
                        QtXmlPatterns::tr("Attribute %1 in derived complex type must have %2 value constraint like in base type.")
                        .arg(formatKeyword(attributeUse->attribute()->displayName(m_namePool)))
                        .arg(formatData("fixed")),
                        XsdSchemaContext::XSDError, sourceLocation(complexType));
                     return;
                  } else {
                     if (attributeUse->valueConstraint()->variety() == XsdAttributeUse::ValueConstraint::Fixed) {
                        const XsdTypeChecker checker(m_context, QVector<QXmlName>(), sourceLocation(complexType));
                        if (!checker.valuesAreEqual(attributeUse->valueConstraint()->value(), baseAttributeUse->valueConstraint()->value(),
                                                    attributeUse->attribute()->type())) {
                           m_context->error(
                              QtXmlPatterns::tr("Attribute %1 in derived complex type must have the same %2 value constraint like in base type.")
                              .arg(formatKeyword(attributeUse->attribute()->displayName(m_namePool)))
                              .arg(formatData("fixed")),
                              XsdSchemaContext::XSDError, sourceLocation(complexType));
                           return;
                        }
                     } else {
                        m_context->error(QtXmlPatterns::tr("Attribute %1 in derived complex type must have %2 value constraint.")
                                         .arg(formatKeyword(attributeUse->attribute()->displayName(m_namePool)))
                                         .arg(formatData("fixed")),
                                         XsdSchemaContext::XSDError, sourceLocation(complexType));
                        return;
                     }
                  }
               }
            }
         }
      }

      // additional check that process content property of attribute wildcard in derived type is
      // not weaker than the wildcard in base type
      const XsdWildcard::Ptr baseWildcard(complexBaseType->attributeWildcard());
      const XsdWildcard::Ptr derivedWildcard(complexType->attributeWildcard());
      if (baseWildcard && derivedWildcard) {
         if (!XsdSchemaHelper::checkWildcardProcessContents(baseWildcard, derivedWildcard)) {
            m_context->error(QtXmlPatterns::tr("processContent of base wildcard must be weaker than derived wildcard."),
                             XsdSchemaContext::XSDError, sourceLocation(complexType));
            return;
         }
      }
   }
}

void XsdSchemaChecker::checkElementDuplicates()
{
   // check all global types...
   SchemaType::List types = m_schema->types();

   // .. and anonymous types
   types << m_schema->anonymousTypes();

   for (int i = 0; i < types.count(); ++i) {
      const SchemaType::Ptr type = types.at(i);

      if (!type->isComplexType() || !type->isDefinedBySchema()) {
         continue;
      }

      const XsdComplexType::Ptr complexType(type);

      if ((complexType->contentType()->variety() == XsdComplexType::ContentType::ElementOnly) ||
            (complexType->contentType()->variety() == XsdComplexType::ContentType::Mixed)) {
         DuplicatedElementMap elementMap;
         DuplicatedWildcardMap wildcardMap;

         checkElementDuplicates(complexType->contentType()->particle(), elementMap, wildcardMap);
      }
   }
}

void XsdSchemaChecker::checkElementDuplicates(const XsdParticle::Ptr &particle, DuplicatedElementMap &elementMap,
      DuplicatedWildcardMap &wildcardMap)
{
   if (particle->term()->isElement()) {
      const XsdElement::Ptr element(particle->term());

      if (elementMap.contains(element->name(m_namePool))) {
         if (element->type() != elementMap.value(element->name(m_namePool))) {
            m_context->error(QtXmlPatterns::tr("Element %1 exists twice with different types.")
                             .arg(formatKeyword(element->displayName(m_namePool))),
                             XsdSchemaContext::XSDError, sourceLocation(element));
            return;
         }
      } else {
         elementMap.insert(element->name(m_namePool), element->type());
      }

      // check substitution group affiliation
      const XsdElement::List substElements = element->substitutionGroupAffiliations();
      for (int i = 0; i < substElements.count(); ++i) {
         const XsdElement::Ptr substElement = substElements.at(i);
         if (elementMap.contains(substElement->name(m_namePool))) {
            if (substElement->type() != elementMap.value(substElement->name(m_namePool))) {
               m_context->error(QtXmlPatterns::tr("Element %1 exists twice with different types.")
                                .arg(formatKeyword(substElement->displayName(m_namePool))),
                                XsdSchemaContext::XSDError, sourceLocation(element));
               return;
            }
         } else {
            elementMap.insert(substElement->name(m_namePool), substElement->type());
         }
      }
   } else if (particle->term()->isModelGroup()) {
      const XsdModelGroup::Ptr group(particle->term());
      const XsdParticle::List particles = group->particles();
      for (int i = 0; i < particles.count(); ++i) {
         checkElementDuplicates(particles.at(i), elementMap, wildcardMap);
      }
   } else if (particle->term()->isWildcard()) {
      const XsdWildcard::Ptr wildcard(particle->term());

      bool error = false;
      if (!wildcardMap.contains(wildcard->namespaceConstraint()->variety())) {
         if (!wildcardMap.isEmpty()) {
            error = true;
         }
      } else {
         const XsdWildcard::Ptr otherWildcard = wildcardMap.value(wildcard->namespaceConstraint()->variety());
         if ((wildcard->processContents() != otherWildcard->processContents()) ||
               (wildcard->namespaceConstraint()->namespaces() != otherWildcard->namespaceConstraint()->namespaces())) {
            error = true;
         }
      }

      if (error) {
         m_context->error(QtXmlPatterns::tr("Particle contains non-deterministic wildcards."), XsdSchemaContext::XSDError,
                          sourceLocation(wildcard));
         return;
      } else {
         wildcardMap.insert(wildcard->namespaceConstraint()->variety(), wildcard);
      }
   }
}

QSourceLocation XsdSchemaChecker::sourceLocation(const NamedSchemaComponent::Ptr &component) const
{
   if (m_componentLocationHash.contains(component)) {
      return m_componentLocationHash.value(component);

   } else {
      QSourceLocation location;
      location.setLine(1);
      location.setColumn(1);
      location.setUri( QUrl("dummyUri") );

      return location;
   }
}

QSourceLocation XsdSchemaChecker::sourceLocationForType(const SchemaType::Ptr &type) const
{
   if (type->isSimpleType()) {
      return sourceLocation(XsdSimpleType::Ptr(type));
   } else {
      return sourceLocation(XsdComplexType::Ptr(type));
   }
}

QT_END_NAMESPACE
