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

#include "qxsdschemaresolver_p.h"

#include "qderivedinteger_p.h"
#include "qderivedstring_p.h"
#include "qqnamevalue_p.h"
#include "qxsdattributereference_p.h"
#include "qxsdparticlechecker_p.h"
#include "qxsdreference_p.h"
#include "qxsdschemacontext_p.h"
#include "qxsdschemahelper_p.h"
#include "qxsdschemaparsercontext_p.h"
#include "qxsdschematypesfactory_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

XsdSchemaResolver::XsdSchemaResolver(const QExplicitlySharedDataPointer<XsdSchemaContext> &context,
                                     const XsdSchemaParserContext *parserContext)
   : m_context(context)
   , m_checker(parserContext->checker())
   , m_namePool(parserContext->namePool())
   , m_schema(parserContext->schema())
{
   m_keyReferences.reserve(20);
   m_simpleRestrictionBases.reserve(20);
   m_simpleListTypes.reserve(20);
   m_simpleUnionTypes.reserve(20);
   m_elementTypes.reserve(20);
   m_complexBaseTypes.reserve(20);
   m_attributeTypes.reserve(20);
   m_alternativeTypes.reserve(20);
   m_alternativeTypeElements.reserve(20);
   m_substitutionGroupAffiliations.reserve(20);

   m_predefinedSchemaTypes = m_context->schemaTypeFactory()->types().values();
}

XsdSchemaResolver::~XsdSchemaResolver()
{
}

void XsdSchemaResolver::resolve()
{
   m_checker->addComponentLocationHash(m_componentLocationHash);

   // resolve the base types for all types
   resolveSimpleRestrictionBaseTypes();
   resolveComplexBaseTypes();

   // do the basic checks which depend on having a base type available
   m_checker->basicCheck();

   // resolve further types that only map a type name to a type object
   resolveSimpleListType();
   resolveSimpleUnionTypes();
   resolveElementTypes();
   resolveAttributeTypes();
   resolveAlternativeTypes();

   // resolve objects that do not need information about inheritance
   resolveKeyReferences();
   resolveSubstitutionGroupAffiliations();

   // resolve objects that do need information about inheritance
   resolveSimpleRestrictions();
   resolveSimpleContentComplexTypes();

   // resolve objects which replace place holders
   resolveTermReferences();
   resolveAttributeTermReferences();

   // resolve additional objects that do need information about inheritance
   resolveAttributeInheritance();
   resolveComplexContentComplexTypes();
   resolveSubstitutionGroups();

   resolveEnumerationFacetValues();

   checkRedefinedGroups();
   checkRedefinedAttributeGroups();

   // check the constraining facets before we resolve them
   m_checker->checkConstrainingFacets();

   // add it again, as we may have added new components in the meantime
   m_checker->addComponentLocationHash(m_componentLocationHash);

   m_checker->check();
}

void XsdSchemaResolver::addKeyReference(const XsdElement::Ptr &element, const XsdIdentityConstraint::Ptr &keyRef,
                                        const QXmlName &reference, const QSourceLocation &location)
{
   KeyReference item;
   item.element = element;
   item.keyRef = keyRef;
   item.reference = reference;
   item.location = location;

   m_keyReferences.append(item);
}

void XsdSchemaResolver::addSimpleRestrictionBase(const XsdSimpleType::Ptr &simpleType, const QXmlName &baseName,
      const QSourceLocation &location)
{
   SimpleRestrictionBase item;
   item.simpleType = simpleType;
   item.baseName = baseName;
   item.location = location;

   m_simpleRestrictionBases.append(item);
}

void XsdSchemaResolver::removeSimpleRestrictionBase(const XsdSimpleType::Ptr &type)
{
   for (int i = 0; i < m_simpleRestrictionBases.count(); ++i) {
      if (m_simpleRestrictionBases.at(i).simpleType == type) {
         m_simpleRestrictionBases.remove(i);
         break;
      }
   }
}

void XsdSchemaResolver::addSimpleListType(const XsdSimpleType::Ptr &simpleType, const QXmlName &typeName,
      const QSourceLocation &location)
{
   SimpleListType item;
   item.simpleType = simpleType;
   item.typeName = typeName;
   item.location = location;

   m_simpleListTypes.append(item);
}

void XsdSchemaResolver::addSimpleUnionTypes(const XsdSimpleType::Ptr &simpleType, const QList<QXmlName> &typeNames,
      const QSourceLocation &location)
{
   SimpleUnionType item;
   item.simpleType = simpleType;
   item.typeNames = typeNames;
   item.location = location;

   m_simpleUnionTypes.append(item);
}

void XsdSchemaResolver::addElementType(const XsdElement::Ptr &element, const QXmlName &typeName,
                                       const QSourceLocation &location)
{
   ElementType item;
   item.element = element;
   item.typeName = typeName;
   item.location = location;

   m_elementTypes.append(item);
}

void XsdSchemaResolver::addComplexBaseType(const XsdComplexType::Ptr &complexType, const QXmlName &baseName,
      const QSourceLocation &location, const XsdFacet::Hash &facets)
{
   ComplexBaseType item;
   item.complexType = complexType;
   item.baseName = baseName;
   item.location = location;
   item.facets = facets;

   m_complexBaseTypes.append(item);
}

void XsdSchemaResolver::removeComplexBaseType(const XsdComplexType::Ptr &type)
{
   for (int i = 0; i < m_complexBaseTypes.count(); ++i) {
      if (m_complexBaseTypes.at(i).complexType == type) {
         m_complexBaseTypes.remove(i);
         break;
      }
   }
}

void XsdSchemaResolver::addComplexContentType(const XsdComplexType::Ptr &complexType, const XsdParticle::Ptr &content,
      bool mixed)
{
   ComplexContentType item;
   item.complexType = complexType;
   item.explicitContent = content;
   item.effectiveMixed = mixed;
   m_complexContentTypes.append(item);
}

void XsdSchemaResolver::addAttributeType(const XsdAttribute::Ptr &attribute, const QXmlName &typeName,
      const QSourceLocation &location)
{
   AttributeType item;
   item.attribute = attribute;
   item.typeName = typeName;
   item.location = location;

   m_attributeTypes.append(item);
}

void XsdSchemaResolver::addAlternativeType(const XsdAlternative::Ptr &alternative, const QXmlName &typeName,
      const QSourceLocation &location)
{
   AlternativeType item;
   item.alternative = alternative;
   item.typeName = typeName;
   item.location = location;

   m_alternativeTypes.append(item);
}

void XsdSchemaResolver::addAlternativeType(const XsdAlternative::Ptr &alternative, const XsdElement::Ptr &element)
{
   AlternativeTypeElement item;
   item.alternative = alternative;
   item.element = element;

   m_alternativeTypeElements.append(item);
}

void XsdSchemaResolver::addSubstitutionGroupAffiliation(const XsdElement::Ptr &element,
      const QList<QXmlName> &elementNames, const QSourceLocation &location)
{
   SubstitutionGroupAffiliation item;
   item.element = element;
   item.elementNames = elementNames;
   item.location = location;

   m_substitutionGroupAffiliations.append(item);
}

void XsdSchemaResolver::addSubstitutionGroupType(const XsdElement::Ptr &element)
{
   m_substitutionGroupTypes.append(element);
}

void XsdSchemaResolver::addComponentLocationHash(const ComponentLocationHash &hash)
{
   m_componentLocationHash.unite(hash);
}

void XsdSchemaResolver::addEnumerationFacetValue(const AtomicValue::Ptr &facetValue,
      const NamespaceSupport &namespaceSupport)
{
   m_enumerationFacetValues.insert(facetValue, namespaceSupport);
}

void XsdSchemaResolver::addRedefinedGroups(const XsdModelGroup::Ptr &redefinedGroup, const XsdModelGroup::Ptr &group)
{
   RedefinedGroups item;
   item.redefinedGroup = redefinedGroup;
   item.group = group;

   m_redefinedGroups.append(item);
}

void XsdSchemaResolver::addRedefinedAttributeGroups(const XsdAttributeGroup::Ptr &redefinedGroup,
      const XsdAttributeGroup::Ptr &group)
{
   RedefinedAttributeGroups item;
   item.redefinedGroup = redefinedGroup;
   item.group = group;

   m_redefinedAttributeGroups.append(item);
}

void XsdSchemaResolver::addAllGroupCheck(const XsdReference::Ptr &reference)
{
   m_allGroups.insert(reference);
}

void XsdSchemaResolver::copyDataTo(const XsdSchemaResolver::Ptr &other) const
{
   other->m_keyReferences << m_keyReferences;
   other->m_simpleRestrictionBases << m_simpleRestrictionBases;
   other->m_simpleListTypes << m_simpleListTypes;
   other->m_simpleUnionTypes << m_simpleUnionTypes;
   other->m_elementTypes << m_elementTypes;
   other->m_complexBaseTypes << m_complexBaseTypes;
   other->m_complexContentTypes << m_complexContentTypes;
   other->m_attributeTypes << m_attributeTypes;
   other->m_alternativeTypes << m_alternativeTypes;
   other->m_alternativeTypeElements << m_alternativeTypeElements;
   other->m_substitutionGroupAffiliations << m_substitutionGroupAffiliations;
   other->m_substitutionGroupTypes << m_substitutionGroupTypes;
}

QXmlName XsdSchemaResolver::baseTypeNameOfType(const SchemaType::Ptr &type) const
{
   for (int i = 0; i < m_simpleRestrictionBases.count(); ++i) {
      if (m_simpleRestrictionBases.at(i).simpleType == type) {
         return m_simpleRestrictionBases.at(i).baseName;
      }
   }

   for (int i = 0; i < m_complexBaseTypes.count(); ++i) {
      if (m_complexBaseTypes.at(i).complexType == type) {
         return m_complexBaseTypes.at(i).baseName;
      }
   }

   return QXmlName();
}

QXmlName XsdSchemaResolver::typeNameOfAttribute(const XsdAttribute::Ptr &attribute) const
{
   for (int i = 0; i < m_attributeTypes.count(); ++i) {
      if (m_attributeTypes.at(i).attribute == attribute) {
         return m_attributeTypes.at(i).typeName;
      }
   }

   return QXmlName();
}

void XsdSchemaResolver::setDefaultOpenContent(const XsdComplexType::OpenContent::Ptr &openContent, bool appliesToEmpty)
{
   m_defaultOpenContent = openContent;
   m_defaultOpenContentAppliesToEmpty = appliesToEmpty;
}

void XsdSchemaResolver::resolveKeyReferences()
{
   for (int i = 0; i < m_keyReferences.count(); ++i) {
      const KeyReference ref = m_keyReferences.at(i);

      const XsdIdentityConstraint::Ptr constraint = m_schema->identityConstraint(ref.reference);
      if (!constraint) {
         m_context->error(QtXmlPatterns::tr("%1 references unknown %2 or %3 element %4.")
                          .arg(formatKeyword(ref.keyRef->displayName(m_namePool)))
                          .arg(formatElement("key"))
                          .arg(formatElement("unique"))
                          .arg(formatKeyword(m_namePool, ref.reference)),
                          XsdSchemaContext::XSDError, ref.location);
         return;
      }

      if (constraint->category() != XsdIdentityConstraint::Key &&
            constraint->category() != XsdIdentityConstraint::Unique) { // only key and unique can be referenced
         m_context->error(QtXmlPatterns::tr("%1 references identity constraint %2 that is no %3 or %4 element.")
                          .arg(formatKeyword(ref.keyRef->displayName(m_namePool)))
                          .arg(formatKeyword(m_namePool, ref.reference))
                          .arg(formatElement("key"))
                          .arg(formatElement("unique")),
                          XsdSchemaContext::XSDError, ref.location);
         return;
      }

      if (constraint->fields().count() != ref.keyRef->fields().count()) {
         m_context->error(
            QtXmlPatterns::tr("%1 has a different number of fields from the identity constraint %2 that it references.")
            .arg(formatKeyword(ref.keyRef->displayName(m_namePool)))
            .arg(formatKeyword(m_namePool, ref.reference)),
            XsdSchemaContext::XSDError, ref.location);
         return;
      }

      ref.keyRef->setReferencedKey(constraint);
   }
}

void XsdSchemaResolver::resolveSimpleRestrictionBaseTypes()
{
   // iterate over all simple types that are derived by restriction
   for (int i = 0; i < m_simpleRestrictionBases.count(); ++i) {
      const SimpleRestrictionBase item = m_simpleRestrictionBases.at(i);

      // find the base type
      SchemaType::Ptr type = m_schema->type(item.baseName);
      if (!type) {
         // maybe it's a basic type...
         type = m_context->schemaTypeFactory()->createSchemaType(item.baseName);
         if (!type) {
            m_context->error(QtXmlPatterns::tr("Base type %1 of %2 element cannot be resolved.")
                             .arg(formatType(m_namePool, item.baseName))
                             .arg(formatElement("restriction")),
                             XsdSchemaContext::XSDError, item.location);
            return;
         }
      }

      item.simpleType->setWxsSuperType(type);
   }
}

void XsdSchemaResolver::resolveSimpleRestrictions()
{
   XsdSimpleType::List simpleTypes;

   // first collect the global simple types
   const SchemaType::List types = m_schema->types();
   for (int i = 0; i < types.count(); ++i) {
      if (types.at(i)->isSimpleType() && (types.at(i)->derivationMethod() == SchemaType::DerivationRestriction)) {
         simpleTypes.append(types.at(i));
      }
   }

   // then collect all anonymous simple types
   const SchemaType::List anonymousTypes = m_schema->anonymousTypes();
   for (int i = 0; i < anonymousTypes.count(); ++i) {
      if (anonymousTypes.at(i)->isSimpleType() &&
            (anonymousTypes.at(i)->derivationMethod() == SchemaType::DerivationRestriction)) {
         simpleTypes.append(anonymousTypes.at(i));
      }
   }

   QSet<XsdSimpleType::Ptr> visitedTypes;
   for (int i = 0; i < simpleTypes.count(); ++i) {
      resolveSimpleRestrictions(simpleTypes.at(i), visitedTypes);
   }
}

void XsdSchemaResolver::resolveSimpleRestrictions(const XsdSimpleType::Ptr &simpleType,
      QSet<XsdSimpleType::Ptr> &visitedTypes)
{
   if (visitedTypes.contains(simpleType)) {
      return;
   } else {
      visitedTypes.insert(simpleType);
   }

   if (simpleType->derivationMethod() != XsdSimpleType::DerivationRestriction) {
      return;
   }

   // as xs:NMTOKENS, xs:ENTITIES and xs:IDREFS are provided by our XsdSchemaTypesFactory, they are
   // setup correctly already and shouldn't be handled here
   if (m_predefinedSchemaTypes.contains(simpleType)) {
      return;
   }

   const SchemaType::Ptr baseType = simpleType->wxsSuperType();
   Q_ASSERT(baseType);

   if (baseType->isDefinedBySchema()) {
      resolveSimpleRestrictions(XsdSimpleType::Ptr(baseType), visitedTypes);
   }

   simpleType->setCategory(baseType->category());

   if (simpleType->category() == XsdSimpleType::SimpleTypeAtomic) {
      QSet<AnySimpleType::Ptr> visitedPrimitiveTypes;
      const AnySimpleType::Ptr primitiveType = findPrimitiveType(baseType, visitedPrimitiveTypes);
      simpleType->setPrimitiveType(primitiveType);
   } else if (simpleType->category() == XsdSimpleType::SimpleTypeList) {
      const XsdSimpleType::Ptr simpleBaseType = baseType;
      simpleType->setItemType(simpleBaseType->itemType());
   } else if (simpleType->category() == XsdSimpleType::SimpleTypeUnion) {
      const XsdSimpleType::Ptr simpleBaseType = baseType;
      simpleType->setMemberTypes(simpleBaseType->memberTypes());
   }
}

void XsdSchemaResolver::resolveSimpleListType()
{
   // iterate over all simple types where the item type shall be resolved
   for (int i = 0; i < m_simpleListTypes.count(); ++i) {
      const SimpleListType item = m_simpleListTypes.at(i);

      // try to resolve the name
      SchemaType::Ptr type = m_schema->type(item.typeName);
      if (!type) {
         // maybe it's a basic type...
         type = m_context->schemaTypeFactory()->createSchemaType(item.typeName);
         if (!type) {
            m_context->error(QtXmlPatterns::tr("Item type %1 of %2 element cannot be resolved.")
                             .arg(formatType(m_namePool, item.typeName))
                             .arg(formatElement("list")),
                             XsdSchemaContext::XSDError, item.location);
            return;
         }
      }

      item.simpleType->setItemType(type);
   }
}

void XsdSchemaResolver::resolveSimpleUnionTypes()
{
   // iterate over all simple types where the union member types shall be resolved
   for (int i = 0; i < m_simpleUnionTypes.count(); ++i) {
      const SimpleUnionType item = m_simpleUnionTypes.at(i);

      AnySimpleType::List memberTypes;

      // iterate over all union member type names
      const QList<QXmlName> typeNames = item.typeNames;
      for (int j = 0; j < typeNames.count(); ++j) {
         const QXmlName typeName = typeNames.at(j);

         // try to resolve the name
         SchemaType::Ptr type = m_schema->type(typeName);
         if (!type) {
            // maybe it's a basic type...
            type = m_context->schemaTypeFactory()->createSchemaType(typeName);
            if (!type) {
               m_context->error(QtXmlPatterns::tr("Member type %1 of %2 element cannot be resolved.")
                                .arg(formatType(m_namePool, typeName))
                                .arg(formatElement("union")),
                                XsdSchemaContext::XSDError, item.location);
               return;
            }
         }

         memberTypes.append(type);
      }

      // append the types that have been defined as <simpleType> children
      memberTypes << item.simpleType->memberTypes();

      item.simpleType->setMemberTypes(memberTypes);
   }
}

void XsdSchemaResolver::resolveElementTypes()
{
   for (int i = 0; i < m_elementTypes.count(); ++i) {
      const ElementType item = m_elementTypes.at(i);

      SchemaType::Ptr type = m_schema->type(item.typeName);
      if (!type) {
         // maybe it's a basic type...
         type = m_context->schemaTypeFactory()->createSchemaType(item.typeName);
         if (!type) {
            m_context->error(QtXmlPatterns::tr("Type %1 of %2 element cannot be resolved.")
                             .arg(formatType(m_namePool, item.typeName))
                             .arg(formatElement("element")),
                             XsdSchemaContext::XSDError, item.location);
            return;
         }
      }

      item.element->setType(type);
   }
}

void XsdSchemaResolver::resolveComplexBaseTypes()
{
   for (int i = 0; i < m_complexBaseTypes.count(); ++i) {
      const ComplexBaseType item = m_complexBaseTypes.at(i);

      SchemaType::Ptr type = m_schema->type(item.baseName);
      if (!type) {
         // maybe it's a basic type...
         type = m_context->schemaTypeFactory()->createSchemaType(item.baseName);
         if (!type) {
            m_context->error(QtXmlPatterns::tr("Base type %1 of complex type cannot be resolved.").arg(formatType(m_namePool,
                             item.baseName)), XsdSchemaContext::XSDError, item.location);
            return;
         }
      }

      if (item.complexType->contentType()->variety() == XsdComplexType::ContentType::Simple) {
         if (type->isComplexType() && type->isDefinedBySchema()) {
            const XsdComplexType::Ptr baseType = type;
            if (baseType->contentType()->variety() != XsdComplexType::ContentType::Simple) {
               m_context->error(QtXmlPatterns::tr("%1 cannot have complex base type that has a %2.")
                                .arg(formatElement("simpleContent"))
                                .arg(formatElement("complexContent")),
                                XsdSchemaContext::XSDError, item.location);
               return;
            }
         }
      }

      item.complexType->setWxsSuperType(type);
   }
}

void XsdSchemaResolver::resolveSimpleContentComplexTypes()
{
   XsdComplexType::List complexTypes;

   // first collect the global complex types
   const SchemaType::List types = m_schema->types();
   for (int i = 0; i < types.count(); ++i) {
      if (types.at(i)->isComplexType() && types.at(i)->isDefinedBySchema()) {
         complexTypes.append(types.at(i));
      }
   }

   // then collect all anonymous simple types
   const SchemaType::List anonymousTypes = m_schema->anonymousTypes();
   for (int i = 0; i < anonymousTypes.count(); ++i) {
      if (anonymousTypes.at(i)->isComplexType() && anonymousTypes.at(i)->isDefinedBySchema()) {
         complexTypes.append(anonymousTypes.at(i));
      }
   }

   QSet<XsdComplexType::Ptr> visitedTypes;
   for (int i = 0; i < complexTypes.count(); ++i) {
      if (XsdComplexType::Ptr(complexTypes.at(i))->contentType()->variety() == XsdComplexType::ContentType::Simple) {
         resolveSimpleContentComplexTypes(complexTypes.at(i), visitedTypes);
      }
   }
}

void XsdSchemaResolver::resolveSimpleContentComplexTypes(const XsdComplexType::Ptr &complexType,
      QSet<XsdComplexType::Ptr> &visitedTypes)
{
   if (visitedTypes.contains(complexType)) {
      return;
   } else {
      visitedTypes.insert(complexType);
   }

   const SchemaType::Ptr baseType = complexType->wxsSuperType();

   // at this point simple types have been resolved already, so we care about
   // complex types here only

   // http://www.w3.org/TR/xmlschema11-1/#dcl.ctd.ctsc
   // 1
   if (baseType->isComplexType() && baseType->isDefinedBySchema()) {
      const XsdComplexType::Ptr complexBaseType = baseType;

      resolveSimpleContentComplexTypes(complexBaseType, visitedTypes);

      if (complexBaseType->contentType()->variety() == XsdComplexType::ContentType::Simple) {
         if (complexType->derivationMethod() == XsdComplexType::DerivationRestriction) {
            if (complexType->contentType()->simpleType()) {
               // 1.1 contains the content of the <simpleType> already
            } else {
               // 1.2
               const XsdSimpleType::Ptr anonType(new XsdSimpleType());
               XsdSimpleType::TypeCategory baseCategory = complexBaseType->contentType()->simpleType()->category();
               anonType->setCategory(baseCategory);

               if (baseCategory == XsdSimpleType::SimpleTypeList) {
                  const XsdSimpleType::Ptr baseSimpleType = complexBaseType->contentType()->simpleType();
                  anonType->setItemType(baseSimpleType->itemType());
               }

               anonType->setDerivationMethod(XsdSimpleType::DerivationRestriction);
               anonType->setWxsSuperType(complexBaseType->contentType()->simpleType());
               anonType->setFacets(complexTypeFacets(complexType));

               QSet<AnySimpleType::Ptr> visitedPrimitiveTypes;
               const AnySimpleType::Ptr primitiveType = findPrimitiveType(anonType->wxsSuperType(), visitedPrimitiveTypes);
               anonType->setPrimitiveType(primitiveType);

               complexType->contentType()->setSimpleType(anonType);

               m_schema->addAnonymousType(anonType);
               m_componentLocationHash.insert(anonType, m_componentLocationHash.value(complexType));
            }
         } else if (complexBaseType->derivationMethod() == XsdComplexType::DerivationExtension) { // 3
            complexType->contentType()->setSimpleType(complexBaseType->contentType()->simpleType());
         }
      } else if (complexBaseType->contentType()->variety() == XsdComplexType::ContentType::Mixed &&
                 complexType->derivationMethod() == XsdComplexType::DerivationRestriction &&
                 XsdSchemaHelper::isParticleEmptiable(complexBaseType->contentType()->particle())) { // 2
         // simple type was already set in parser

         const XsdSimpleType::Ptr anonType(new XsdSimpleType());
         anonType->setCategory(complexType->contentType()->simpleType()->category());
         anonType->setDerivationMethod(XsdSimpleType::DerivationRestriction);
         anonType->setWxsSuperType(complexType->contentType()->simpleType());
         anonType->setFacets(complexTypeFacets(complexType));

         QSet<AnySimpleType::Ptr> visitedPrimitiveTypes;
         const AnySimpleType::Ptr primitiveType = findPrimitiveType(anonType->wxsSuperType(), visitedPrimitiveTypes);
         anonType->setPrimitiveType(primitiveType);

         complexType->contentType()->setSimpleType(anonType);

         m_schema->addAnonymousType(anonType);
         m_componentLocationHash.insert(anonType, m_componentLocationHash.value(complexType));
      } else {
         complexType->contentType()->setSimpleType(BuiltinTypes::xsAnySimpleType);
      }
   } else if (baseType->isSimpleType()) { // 4
      complexType->contentType()->setSimpleType(baseType);
   } else { // 5
      complexType->contentType()->setSimpleType(BuiltinTypes::xsAnySimpleType);
   }
}

void XsdSchemaResolver::resolveComplexContentComplexTypes()
{
   XsdComplexType::List complexTypes;

   // first collect the global complex types
   const SchemaType::List types = m_schema->types();
   for (int i = 0; i < types.count(); ++i) {
      if (types.at(i)->isComplexType() && types.at(i)->isDefinedBySchema()) {
         complexTypes.append(types.at(i));
      }
   }

   // then collect all anonymous simple types
   const SchemaType::List anonymousTypes = m_schema->anonymousTypes();
   for (int i = 0; i < anonymousTypes.count(); ++i) {
      if (anonymousTypes.at(i)->isComplexType() && anonymousTypes.at(i)->isDefinedBySchema()) {
         complexTypes.append(anonymousTypes.at(i));
      }
   }

   QSet<XsdComplexType::Ptr> visitedTypes;
   for (int i = 0; i < complexTypes.count(); ++i) {
      if (XsdComplexType::Ptr(complexTypes.at(i))->contentType()->variety() != XsdComplexType::ContentType::Simple) {
         resolveComplexContentComplexTypes(complexTypes.at(i), visitedTypes);
      }
   }
}

void XsdSchemaResolver::resolveComplexContentComplexTypes(const XsdComplexType::Ptr &complexType,
      QSet<XsdComplexType::Ptr> &visitedTypes)
{
   if (visitedTypes.contains(complexType)) {
      return;
   } else {
      visitedTypes.insert(complexType);
   }

   ComplexContentType item;
   bool foundCorrespondingItem = false;
   for (int i = 0; i < m_complexContentTypes.count(); ++i) {
      if (m_complexContentTypes.at(i).complexType == complexType) {
         item = m_complexContentTypes.at(i);
         foundCorrespondingItem = true;
         break;
      }
   }

   if (!foundCorrespondingItem) {
      return;
   }

   const SchemaType::Ptr baseType = complexType->wxsSuperType();

   // at this point simple types have been resolved already, so we care about
   // complex types here only
   if (baseType->isComplexType() && baseType->isDefinedBySchema()) {
      resolveComplexContentComplexTypes(XsdComplexType::Ptr(baseType), visitedTypes);
   }


   // @see http://www.w3.org/TR/xmlschema11-1/#dcl.ctd.ctcc.common

   // 3
   XsdParticle::Ptr effectiveContent;
   if (!item.explicitContent) { // 3.1
      if (item.effectiveMixed == true) { // 3.1.1
         const XsdParticle::Ptr particle(new XsdParticle());
         particle->setMinimumOccurs(1);
         particle->setMaximumOccurs(1);
         particle->setMaximumOccursUnbounded(false);

         const XsdModelGroup::Ptr sequence(new XsdModelGroup());
         sequence->setCompositor(XsdModelGroup::SequenceCompositor);
         particle->setTerm(sequence);

         effectiveContent = particle;
      } else { // 3.1.2
         effectiveContent = XsdParticle::Ptr();
      }
   } else { // 3.2
      effectiveContent = item.explicitContent;
   }

   // 4
   XsdComplexType::ContentType::Ptr explicitContentType(new XsdComplexType::ContentType());
   if (item.complexType->derivationMethod() == XsdComplexType::DerivationRestriction) { // 4.1
      if (!effectiveContent) { // 4.1.1
         explicitContentType->setVariety(XsdComplexType::ContentType::Empty);
      } else { // 4.1.2
         if (item.effectiveMixed == true) {
            explicitContentType->setVariety(XsdComplexType::ContentType::Mixed);
         } else {
            explicitContentType->setVariety(XsdComplexType::ContentType::ElementOnly);
         }

         explicitContentType->setParticle(effectiveContent);
      }
   } else if (item.complexType->derivationMethod() == XsdComplexType::DerivationExtension) { // 4.2
      const SchemaType::Ptr baseType = item.complexType->wxsSuperType();
      if (baseType->isSimpleType() || (baseType->isComplexType() && baseType->isDefinedBySchema() &&
                                       (XsdComplexType::Ptr(baseType)->contentType()->variety() == XsdComplexType::ContentType::Empty ||
                                        XsdComplexType::Ptr(baseType)->contentType()->variety() == XsdComplexType::ContentType::Simple))) { // 4.2.1
         if (!effectiveContent) {
            explicitContentType->setVariety(XsdComplexType::ContentType::Empty);
         } else {
            if (item.effectiveMixed == true) {
               explicitContentType->setVariety(XsdComplexType::ContentType::Mixed);
            } else {
               explicitContentType->setVariety(XsdComplexType::ContentType::ElementOnly);
            }

            explicitContentType->setParticle(effectiveContent);
         }
      } else if (baseType->isComplexType() && baseType->isDefinedBySchema() &&
                 (XsdComplexType::Ptr(baseType)->contentType()->variety() == XsdComplexType::ContentType::ElementOnly ||
                  XsdComplexType::Ptr(baseType)->contentType()->variety() == XsdComplexType::ContentType::Mixed) &&
                 !effectiveContent) { // 4.2.2
         const XsdComplexType::Ptr complexBaseType(baseType);

         explicitContentType = complexBaseType->contentType();
      } else { // 4.2.3
         explicitContentType->setVariety(item.effectiveMixed ? XsdComplexType::ContentType::Mixed :
                                         XsdComplexType::ContentType::ElementOnly);

         XsdParticle::Ptr baseParticle;
         if (baseType == BuiltinTypes::xsAnyType) {
            // we need a workaround here, since the xsAnyType is no real (aka XsdComplexType) complex type...

            baseParticle = XsdParticle::Ptr(new XsdParticle());
            baseParticle->setMinimumOccurs(1);
            baseParticle->setMaximumOccurs(1);
            baseParticle->setMaximumOccursUnbounded(false);

            const XsdModelGroup::Ptr group(new XsdModelGroup());
            group->setCompositor(XsdModelGroup::SequenceCompositor);

            const XsdParticle::Ptr particle(new XsdParticle());
            particle->setMinimumOccurs(0);
            particle->setMaximumOccursUnbounded(true);

            const XsdWildcard::Ptr wildcard(new XsdWildcard());
            wildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Any);
            wildcard->setProcessContents(XsdWildcard::Lax);

            particle->setTerm(wildcard);
            XsdParticle::List particles;
            particles.append(particle);
            group->setParticles(particles);
            baseParticle->setTerm(group);
         } else {
            const XsdComplexType::Ptr complexBaseType(baseType);
            baseParticle = complexBaseType->contentType()->particle();
         }
         if (baseParticle && baseParticle->term()->isModelGroup() &&
               (XsdModelGroup::Ptr(baseParticle->term())->compositor() == XsdModelGroup::AllCompositor) &&
               (!item.explicitContent)) { // 4.2.3.1

            explicitContentType->setParticle(baseParticle);
         } else if (baseParticle && baseParticle->term()->isModelGroup() &&
                    (XsdModelGroup::Ptr(baseParticle->term())->compositor() == XsdModelGroup::AllCompositor) &&
                    (effectiveContent->term()->isModelGroup() &&
                     (XsdModelGroup::Ptr(effectiveContent->term())->compositor() == XsdModelGroup::AllCompositor))) { // 4.2.3.2
            const XsdParticle::Ptr particle(new XsdParticle());
            particle->setMinimumOccurs(effectiveContent->minimumOccurs());
            particle->setMaximumOccurs(1);
            particle->setMaximumOccursUnbounded(false);

            const XsdModelGroup::Ptr group(new XsdModelGroup());
            group->setCompositor(XsdModelGroup::AllCompositor);
            XsdParticle::List particles = XsdModelGroup::Ptr(baseParticle->term())->particles();
            particles << XsdModelGroup::Ptr(effectiveContent->term())->particles();
            group->setParticles(particles);
            particle->setTerm(group);

            explicitContentType->setParticle(particle);
         } else { // 4.2.3.3
            const XsdParticle::Ptr particle(new XsdParticle());
            particle->setMinimumOccurs(1);
            particle->setMaximumOccurs(1);
            particle->setMaximumOccursUnbounded(false);

            const XsdModelGroup::Ptr group(new XsdModelGroup());
            group->setCompositor(XsdModelGroup::SequenceCompositor);

            if (effectiveContent && effectiveContent->term()->isModelGroup() &&
                  XsdModelGroup::Ptr(effectiveContent->term())->compositor() == XsdModelGroup::AllCompositor) {
               m_context->error(
                  QtXmlPatterns::tr("Content model of complex type %1 contains %2 element, so it cannot be derived by extension from a non-empty type.")
                  .arg(formatType(m_namePool, complexType)).arg(formatKeyword("all")), XsdSchemaContext::XSDError,
                  sourceLocation(complexType));
               return;
            }

            if (baseParticle && baseParticle->term()->isModelGroup() &&
                  XsdModelGroup::Ptr(baseParticle->term())->compositor() == XsdModelGroup::AllCompositor) {
               m_context->error(
                  QtXmlPatterns::tr("Complex type %1 cannot be derived by extension from %2 as the latter contains %3 element in its content model.")
                  .arg(formatType(m_namePool, complexType))
                  .arg(formatType(m_namePool, baseType))
                  .arg(formatKeyword("all")), XsdSchemaContext::XSDError, sourceLocation(complexType));
               return;
            }

            XsdParticle::List particles;
            if (baseParticle) {
               particles << baseParticle;
            }
            if (effectiveContent) {
               particles << effectiveContent;
            }
            group->setParticles(particles);
            particle->setTerm(group);

            explicitContentType->setParticle(particle);
         }

         if (baseType->isDefinedBySchema()) { // xs:anyType has no open content
            const XsdComplexType::Ptr complexBaseType(baseType);
            explicitContentType->setOpenContent(complexBaseType->contentType()->openContent());
         }
      }
   }

   // 5
   XsdComplexType::OpenContent::Ptr wildcardElement;
   if (item.complexType->contentType()->openContent()) { // 5.1
      wildcardElement = item.complexType->contentType()->openContent();
   } else {
      if (m_defaultOpenContent) { // 5.2
         if ((explicitContentType->variety() != XsdComplexType::ContentType::Empty) || // 5.2.1
               (explicitContentType->variety() == XsdComplexType::ContentType::Empty && m_defaultOpenContentAppliesToEmpty)) { // 5.2.2
            wildcardElement = m_defaultOpenContent;
         }
      }
   }

   // 6
   if (!wildcardElement) { // 6.1
      item.complexType->setContentType(explicitContentType);
   } else {
      if (wildcardElement->mode() == XsdComplexType::OpenContent::None) { // 6.2
         const XsdComplexType::ContentType::Ptr contentType(new XsdComplexType::ContentType());
         contentType->setVariety(explicitContentType->variety());
         contentType->setParticle(explicitContentType->particle());

         item.complexType->setContentType(contentType);
      } else { // 6.3
         const XsdComplexType::ContentType::Ptr contentType(new XsdComplexType::ContentType());

         if (explicitContentType->variety() == XsdComplexType::ContentType::Empty) {
            contentType->setVariety(XsdComplexType::ContentType::ElementOnly);
         } else {
            contentType->setVariety(explicitContentType->variety());
         }

         if (explicitContentType->variety() == XsdComplexType::ContentType::Empty) {
            const XsdParticle::Ptr particle(new XsdParticle());
            particle->setMinimumOccurs(1);
            particle->setMaximumOccurs(1);
            const XsdModelGroup::Ptr sequence(new XsdModelGroup());
            sequence->setCompositor(XsdModelGroup::SequenceCompositor);
            particle->setTerm(sequence);
            contentType->setParticle(particle);
         } else {
            contentType->setParticle(explicitContentType->particle());
         }

         const XsdComplexType::OpenContent::Ptr openContent(new XsdComplexType::OpenContent());
         if (wildcardElement) {
            openContent->setMode(wildcardElement->mode());
         } else {
            openContent->setMode(XsdComplexType::OpenContent::Interleave);
         }

         if (wildcardElement) {
            openContent->setWildcard(wildcardElement->wildcard());
         }

         item.complexType->setContentType(contentType);
      }
   }
}

void XsdSchemaResolver::resolveAttributeTypes()
{
   for (int i = 0; i < m_attributeTypes.count(); ++i) {
      const AttributeType item = m_attributeTypes.at(i);

      SchemaType::Ptr type = m_schema->type(item.typeName);
      if (!type) {
         // maybe it's a basic type...
         type = m_context->schemaTypeFactory()->createSchemaType(item.typeName);
         if (!type) {
            m_context->error(QtXmlPatterns::tr("Type %1 of %2 element cannot be resolved.")
                             .arg(formatType(m_namePool, item.typeName))
                             .arg(formatElement("attribute")),
                             XsdSchemaContext::XSDError, item.location);
            return;
         }
      }

      if (!type->isSimpleType() && type->category() != SchemaType::None) {
         m_context->error(QtXmlPatterns::tr("Type of %1 element must be a simple type, %2 is not.")
                          .arg(formatElement("attribute"))
                          .arg(formatType(m_namePool, item.typeName)),
                          XsdSchemaContext::XSDError, item.location);
         return;
      }

      item.attribute->setType(type);
   }
}

void XsdSchemaResolver::resolveAlternativeTypes()
{
   for (int i = 0; i < m_alternativeTypes.count(); ++i) {
      const AlternativeType item = m_alternativeTypes.at(i);

      SchemaType::Ptr type = m_schema->type(item.typeName);
      if (!type) {
         // maybe it's a basic type...
         type = m_context->schemaTypeFactory()->createSchemaType(item.typeName);
         if (!type) {
            m_context->error(QtXmlPatterns::tr("Type %1 of %2 element cannot be resolved.")
                             .arg(formatType(m_namePool, item.typeName))
                             .arg(formatElement("alternative")),
                             XsdSchemaContext::XSDError, item.location);
            return;
         }
      }

      item.alternative->setType(type);
   }

   for (int i = 0; i < m_alternativeTypeElements.count(); ++i) {
      const AlternativeTypeElement item = m_alternativeTypeElements.at(i);
      item.alternative->setType(item.element->type());
   }
}

bool hasCircularSubstitutionGroup(const XsdElement::Ptr &current, const XsdElement::Ptr &head,
                                  const NamePool::Ptr &namePool)
{
   if (current == head) {
      return true;
   } else {
      const XsdElement::List elements = current->substitutionGroupAffiliations();
      for (int i = 0; i < elements.count(); ++i) {
         if (hasCircularSubstitutionGroup(elements.at(i), head, namePool)) {
            return true;
         }
      }
   }

   return false;
}

void XsdSchemaResolver::resolveSubstitutionGroupAffiliations()
{
   for (int i = 0; i < m_substitutionGroupAffiliations.count(); ++i) {
      const SubstitutionGroupAffiliation item = m_substitutionGroupAffiliations.at(i);

      XsdElement::List affiliations;
      for (int j = 0; j < item.elementNames.count(); ++j) {
         const XsdElement::Ptr element = m_schema->element(item.elementNames.at(j));
         if (!element) {
            m_context->error(QtXmlPatterns::tr("Substitution group %1 of %2 element cannot be resolved.")
                             .arg(formatKeyword(m_namePool, item.elementNames.at(j)))
                             .arg(formatElement("element")),
                             XsdSchemaContext::XSDError, item.location);
            return;
         }

         // @see http://www.w3.org/TR/xmlschema11-1/#e-props-correct 5)
         if (hasCircularSubstitutionGroup(element, item.element, m_namePool)) {
            m_context->error(QtXmlPatterns::tr("Substitution group %1 has circular definition.").arg(formatKeyword(m_namePool,
                             item.elementNames.at(j))), XsdSchemaContext::XSDError, item.location);
            return;
         }

         affiliations.append(element);
      }

      item.element->setSubstitutionGroupAffiliations(affiliations);
   }

   for (int i = 0; i < m_substitutionGroupTypes.count(); ++i) {
      const XsdElement::Ptr element = m_substitutionGroupTypes.at(i);
      element->setType(element->substitutionGroupAffiliations().first()->type());
   }
}

bool isSubstGroupHeadOf(const XsdElement::Ptr &head, const XsdElement::Ptr &element, const NamePool::Ptr &namePool)
{
   if (head->name(namePool) == element->name(namePool)) {
      return true;
   }

   const XsdElement::List affiliations = element->substitutionGroupAffiliations();
   for (int i = 0; i < affiliations.count(); ++i) {
      if (isSubstGroupHeadOf(head, affiliations.at(i), namePool)) {
         return true;
      }
   }

   return false;
}

void XsdSchemaResolver::resolveSubstitutionGroups()
{
   const XsdElement::List elements = m_schema->elements();
   for (int i = 0; i < elements.count(); ++i) {
      const XsdElement::Ptr element = elements.at(i);

      // the element is always itself in the substitution group
      element->addSubstitutionGroup(element);

      for (int j = 0; j < elements.count(); ++j) {
         if (i == j) {
            continue;
         }

         if (isSubstGroupHeadOf(element, elements.at(j), m_namePool)) {
            element->addSubstitutionGroup(elements.at(j));
         }
      }
   }
}

void XsdSchemaResolver::resolveTermReferences()
{
   // first the global complex types
   const SchemaType::List types = m_schema->types();
   for (int i = 0; i < types.count(); ++i) {
      if (!(types.at(i)->isComplexType()) || !types.at(i)->isDefinedBySchema()) {
         continue;
      }

      const XsdComplexType::Ptr complexType = types.at(i);
      if (complexType->contentType()->variety() != XsdComplexType::ContentType::ElementOnly &&
            complexType->contentType()->variety() != XsdComplexType::ContentType::Mixed) {
         continue;
      }

      resolveTermReference(complexType->contentType()->particle(), QSet<QXmlName>());
   }

   // then all anonymous complex types
   const SchemaType::List anonymousTypes = m_schema->anonymousTypes();
   for (int i = 0; i < anonymousTypes.count(); ++i) {
      if (!(anonymousTypes.at(i)->isComplexType()) || !anonymousTypes.at(i)->isDefinedBySchema()) {
         continue;
      }

      const XsdComplexType::Ptr complexType = anonymousTypes.at(i);
      if (complexType->contentType()->variety() != XsdComplexType::ContentType::ElementOnly &&
            complexType->contentType()->variety() != XsdComplexType::ContentType::Mixed) {
         continue;
      }

      resolveTermReference(complexType->contentType()->particle(), QSet<QXmlName>());
   }

   const XsdModelGroup::List groups = m_schema->elementGroups();
   for (int i = 0; i < groups.count(); ++i) {
      const XsdParticle::Ptr particle(new XsdParticle());
      particle->setTerm(groups.at(i));
      resolveTermReference(particle, QSet<QXmlName>());
   }
}

void XsdSchemaResolver::resolveTermReference(const XsdParticle::Ptr &particle, QSet<QXmlName> visitedGroups)
{
   if (!particle) {
      return;
   }

   const XsdTerm::Ptr term = particle->term();

   // if it is a model group, we iterate over it recursive...
   if (term->isModelGroup()) {
      const XsdModelGroup::Ptr modelGroup = term;
      const XsdParticle::List particles = modelGroup->particles();

      for (int i = 0; i < particles.count(); ++i) {
         resolveTermReference(particles.at(i), visitedGroups);
      }

      // check for unique names of elements inside all compositor
      if (modelGroup->compositor() != XsdModelGroup::ChoiceCompositor) {
         for (int i = 0; i < particles.count(); ++i) {
            const XsdParticle::Ptr particle = particles.at(i);
            const XsdTerm::Ptr term = particle->term();

            if (!(term->isElement())) {
               continue;
            }

            for (int j = 0; j < particles.count(); ++j) {
               const XsdParticle::Ptr otherParticle = particles.at(j);
               const XsdTerm::Ptr otherTerm = otherParticle->term();

               if (otherTerm->isElement() && i != j) {
                  const XsdElement::Ptr element = term;
                  const XsdElement::Ptr otherElement = otherTerm;

                  if (element->name(m_namePool) == otherElement->name(m_namePool)) {
                     if (modelGroup->compositor() == XsdModelGroup::AllCompositor) {
                        m_context->error(QtXmlPatterns::tr("Duplicated element names %1 in %2 element.")
                                         .arg(formatKeyword(element->displayName(m_namePool)))
                                         .arg(formatElement("all")),
                                         XsdSchemaContext::XSDError, sourceLocation(modelGroup));
                        return;
                     } else if (modelGroup->compositor() == XsdModelGroup::SequenceCompositor) {
                        if (element->type() != otherElement->type()) {  // not same variety
                           m_context->error(QtXmlPatterns::tr("Duplicated element names %1 in %2 element.")
                                            .arg(formatKeyword(element->displayName(m_namePool)))
                                            .arg(formatElement("sequence")),
                                            XsdSchemaContext::XSDError, sourceLocation(modelGroup));
                           return;
                        }
                     }
                  }
               }
            }
         }
      }

      return;
   }

   // ...otherwise we have reached the end of recursion...
   if (!term->isReference()) {
      return;
   }

   // ...or we have reached a reference term that must be resolved
   const XsdReference::Ptr reference = term;
   switch (reference->type()) {
      case XsdReference::Element: {
         const XsdElement::Ptr element = m_schema->element(reference->referenceName());
         if (element) {
            particle->setTerm(element);
         } else {
            m_context->error(QtXmlPatterns::tr("Reference %1 of %2 element cannot be resolved.")
                             .arg(formatKeyword(m_namePool, reference->referenceName()))
                             .arg(formatElement("element")),
                             XsdSchemaContext::XSDError, reference->sourceLocation());
            return;
         }
      }
      break;
      case XsdReference::ModelGroup: {
         const XsdModelGroup::Ptr modelGroup = m_schema->elementGroup(reference->referenceName());
         if (modelGroup) {
            if (visitedGroups.contains(modelGroup->name(m_namePool))) {
               m_context->error(QtXmlPatterns::tr("Circular group reference for %1.").arg(formatKeyword(modelGroup->displayName(
                                   m_namePool))),
                                XsdSchemaContext::XSDError, reference->sourceLocation());
            } else {
               visitedGroups.insert(modelGroup->name(m_namePool));
            }

            particle->setTerm(modelGroup);

            // start recursive iteration here as well to get all references resolved
            const XsdParticle::List particles = modelGroup->particles();
            for (int i = 0; i < particles.count(); ++i) {
               resolveTermReference(particles.at(i), visitedGroups);
            }

            if (modelGroup->compositor() == XsdModelGroup::AllCompositor) {
               if (m_allGroups.contains(reference)) {
                  m_context->error(QtXmlPatterns::tr("%1 element is not allowed in this scope").arg(formatElement("all.")),
                                   XsdSchemaContext::XSDError, reference->sourceLocation());
                  return;
               }
               if (particle->maximumOccursUnbounded() || particle->maximumOccurs() != 1) {
                  m_context->error(QtXmlPatterns::tr("%1 element cannot have %2 attribute with value other than %3.")
                                   .arg(formatElement("all"))
                                   .arg(formatAttribute("maxOccurs"))
                                   .arg(formatData("1")),
                                   XsdSchemaContext::XSDError, reference->sourceLocation());
                  return;
               }
               if (particle->minimumOccurs() != 0 && particle->minimumOccurs() != 1) {
                  m_context->error(QtXmlPatterns::tr("%1 element cannot have %2 attribute with value other than %3 or %4.")
                                   .arg(formatElement("all"))
                                   .arg(formatAttribute("minOccurs"))
                                   .arg(formatData("0"))
                                   .arg(formatData("1")),
                                   XsdSchemaContext::XSDError, reference->sourceLocation());
                  return;
               }
            }
         } else {
            m_context->error(QtXmlPatterns::tr("Reference %1 of %2 element cannot be resolved.")
                             .arg(formatKeyword(m_namePool, reference->referenceName()))
                             .arg(formatElement("group")),
                             XsdSchemaContext::XSDError, reference->sourceLocation());
            return;
         }
      }
      break;
   }
}

void XsdSchemaResolver::resolveAttributeTermReferences()
{
   // first all global attribute groups
   const XsdAttributeGroup::List attributeGroups = m_schema->attributeGroups();
   for (int i = 0; i < attributeGroups.count(); ++i) {
      XsdWildcard::Ptr wildcard = attributeGroups.at(i)->wildcard();
      const XsdAttributeUse::List uses = resolveAttributeTermReferences(attributeGroups.at(i)->attributeUses(), wildcard,
                                         QSet<QXmlName>());
      attributeGroups.at(i)->setAttributeUses(uses);
      attributeGroups.at(i)->setWildcard(wildcard);
   }

   // then the global complex types
   const SchemaType::List types = m_schema->types();
   for (int i = 0; i < types.count(); ++i) {
      if (!(types.at(i)->isComplexType()) || !types.at(i)->isDefinedBySchema()) {
         continue;
      }

      const XsdComplexType::Ptr complexType = types.at(i);
      const XsdAttributeUse::List attributeUses = complexType->attributeUses();

      XsdWildcard::Ptr wildcard = complexType->attributeWildcard();
      const XsdAttributeUse::List uses = resolveAttributeTermReferences(attributeUses, wildcard, QSet<QXmlName>());
      complexType->setAttributeUses(uses);
      complexType->setAttributeWildcard(wildcard);
   }

   // and afterwards all anonymous complex types
   const SchemaType::List anonymousTypes = m_schema->anonymousTypes();
   for (int i = 0; i < anonymousTypes.count(); ++i) {
      if (!(anonymousTypes.at(i)->isComplexType()) || !anonymousTypes.at(i)->isDefinedBySchema()) {
         continue;
      }

      const XsdComplexType::Ptr complexType = anonymousTypes.at(i);
      const XsdAttributeUse::List attributeUses = complexType->attributeUses();

      XsdWildcard::Ptr wildcard = complexType->attributeWildcard();
      const XsdAttributeUse::List uses = resolveAttributeTermReferences(attributeUses, wildcard, QSet<QXmlName>());
      complexType->setAttributeUses(uses);
      complexType->setAttributeWildcard(wildcard);
   }
}

XsdAttributeUse::List XsdSchemaResolver::resolveAttributeTermReferences(const XsdAttributeUse::List &attributeUses,
      XsdWildcard::Ptr &wildcard, QSet<QXmlName> visitedAttributeGroups)
{
   XsdAttributeUse::List resolvedAttributeUses;

   for (int i = 0; i < attributeUses.count(); ++i) {
      const XsdAttributeUse::Ptr attributeUse = attributeUses.at(i);
      if (attributeUse->isAttributeUse()) {
         // it is a real attribute use, so no need to resolve it
         resolvedAttributeUses.append(attributeUse);
      } else if (attributeUse->isReference()) {
         // it is just a reference, so resolve it to the real attribute use

         const XsdAttributeReference::Ptr reference = attributeUse;
         if (reference->type() == XsdAttributeReference::AttributeUse) {

            // lookup the real attribute
            const XsdAttribute::Ptr attribute = m_schema->attribute(reference->referenceName());
            if (!attribute) {
               m_context->error(QtXmlPatterns::tr("Reference %1 of %2 element cannot be resolved.")
                                .arg(formatKeyword(m_namePool, reference->referenceName()))
                                .arg(formatElement("attribute")),
                                XsdSchemaContext::XSDError, reference->sourceLocation());
               return XsdAttributeUse::List();
            }

            // if both, reference and definition have a fixed or default value set, then they must be equal
            if (attribute->valueConstraint() && attributeUse->valueConstraint()) {
               if (attribute->valueConstraint()->value() != attributeUse->valueConstraint()->value()) {
                  m_context->error(
                     QtXmlPatterns::tr("%1 or %2 attribute of reference %3 does not match with the attribute declaration %4.")
                     .arg(formatAttribute("fixed"))
                     .arg(formatAttribute("default"))
                     .arg(formatKeyword(m_namePool, reference->referenceName()))
                     .arg(formatKeyword(attribute->displayName(m_namePool))),
                     XsdSchemaContext::XSDError, reference->sourceLocation());
                  return XsdAttributeUse::List();
               }
            }

            attributeUse->setAttribute(attribute);
            if (!attributeUse->valueConstraint() && attribute->valueConstraint()) {
               attributeUse->setValueConstraint(XsdAttributeUse::ValueConstraint::fromAttributeValueConstraint(
                                                   attribute->valueConstraint()));
            }

            resolvedAttributeUses.append(attributeUse);
         } else if (reference->type() == XsdAttributeReference::AttributeGroup) {
            const XsdAttributeGroup::Ptr attributeGroup = m_schema->attributeGroup(reference->referenceName());
            if (!attributeGroup) {
               m_context->error(QtXmlPatterns::tr("Reference %1 of %2 element cannot be resolved.")
                                .arg(formatKeyword(m_namePool, reference->referenceName()))
                                .arg(formatElement("attributeGroup")),
                                XsdSchemaContext::XSDError, reference->sourceLocation());
               return XsdAttributeUse::List();
            }
            if (visitedAttributeGroups.contains(attributeGroup->name(m_namePool))) {
               m_context->error(QtXmlPatterns::tr("Attribute group %1 has circular reference.").arg(formatKeyword(m_namePool,
                                reference->referenceName())),
                                XsdSchemaContext::XSDError, reference->sourceLocation());
               return XsdAttributeUse::List();
            } else {
               visitedAttributeGroups.insert(attributeGroup->name(m_namePool));
            }

            // resolve attribute wildcards as defined in http://www.w3.org/TR/xmlschema11-1/#declare-attributeGroup-wildcard
            XsdWildcard::Ptr childWildcard;
            resolvedAttributeUses << resolveAttributeTermReferences(attributeGroup->attributeUses(), childWildcard,
                                  visitedAttributeGroups);
            if (!childWildcard) {
               if (attributeGroup->wildcard()) {
                  if (wildcard) {
                     const XsdWildcard::ProcessContents contents = wildcard->processContents();
                     wildcard = XsdSchemaHelper::wildcardIntersection(wildcard, attributeGroup->wildcard());
                     wildcard->setProcessContents(contents);
                  } else {
                     wildcard = attributeGroup->wildcard();
                  }
               }
            } else {
               XsdWildcard::Ptr newWildcard;
               if (attributeGroup->wildcard()) {
                  const XsdWildcard::ProcessContents contents = attributeGroup->wildcard()->processContents();
                  newWildcard = XsdSchemaHelper::wildcardIntersection(attributeGroup->wildcard(), childWildcard);
                  newWildcard->setProcessContents(contents);
               } else {
                  newWildcard = childWildcard;
               }

               if (wildcard) {
                  const XsdWildcard::ProcessContents contents = wildcard->processContents();
                  wildcard = XsdSchemaHelper::wildcardIntersection(wildcard, newWildcard);
                  wildcard->setProcessContents(contents);
               } else {
                  wildcard = newWildcard;
               }
            }
         }
      }
   }

   return resolvedAttributeUses;
}

void XsdSchemaResolver::resolveAttributeInheritance()
{
   // collect the global and anonymous complex types
   SchemaType::List types = m_schema->types();
   types << m_schema->anonymousTypes();

   QSet<XsdComplexType::Ptr> visitedTypes;
   for (int i = 0; i < types.count(); ++i) {
      if (!(types.at(i)->isComplexType()) || !types.at(i)->isDefinedBySchema()) {
         continue;
      }

      const XsdComplexType::Ptr complexType = types.at(i);

      resolveAttributeInheritance(complexType, visitedTypes);
   }
}

bool isValidWildcardRestriction(const XsdWildcard::Ptr &wildcard, const XsdWildcard::Ptr &baseWildcard)
{
   if (wildcard->namespaceConstraint()->variety() == baseWildcard->namespaceConstraint()->variety()) {
      if (!XsdSchemaHelper::checkWildcardProcessContents(baseWildcard, wildcard)) {
         return false;
      }
   }

   if (wildcard->namespaceConstraint()->variety() == XsdWildcard::NamespaceConstraint::Any &&
         baseWildcard->namespaceConstraint()->variety() != XsdWildcard::NamespaceConstraint::Any ) {
      return false;
   }
   if (baseWildcard->namespaceConstraint()->variety() == XsdWildcard::NamespaceConstraint::Not &&
         wildcard->namespaceConstraint()->variety() == XsdWildcard::NamespaceConstraint::Enumeration) {
      if (!baseWildcard->namespaceConstraint()->namespaces().intersect(
               wildcard->namespaceConstraint()->namespaces()).isEmpty()) {
         return false;
      }
   }
   if (baseWildcard->namespaceConstraint()->variety() == XsdWildcard::NamespaceConstraint::Enumeration &&
         wildcard->namespaceConstraint()->variety() == XsdWildcard::NamespaceConstraint::Enumeration) {
      if (!wildcard->namespaceConstraint()->namespaces().subtract(
               baseWildcard->namespaceConstraint()->namespaces()).isEmpty()) {
         return false;
      }
   }

   return true;
}

/*
 * Since we inherit the attributes from our base class we have to walk up in the
 * inheritance hierarchy first and resolve the attribute inheritance top-down.
 */
void XsdSchemaResolver::resolveAttributeInheritance(const XsdComplexType::Ptr &complexType,
      QSet<XsdComplexType::Ptr> &visitedTypes)
{
   if (visitedTypes.contains(complexType)) {
      return;
   } else {
      visitedTypes.insert(complexType);
   }

   const SchemaType::Ptr baseType = complexType->wxsSuperType();
   Q_ASSERT(baseType);

   if (!(baseType->isComplexType()) || !baseType->isDefinedBySchema()) {
      return;
   }

   const XsdComplexType::Ptr complexBaseType = baseType;

   resolveAttributeInheritance(complexBaseType, visitedTypes);

   // @see http://www.w3.org/TR/xmlschema11-1/#dcl.ctd.attuses

   // 1 and 2 (the attribute groups have been resolved here already)
   const XsdAttributeUse::List uses = complexBaseType->attributeUses();

   if (complexType->derivationMethod() == XsdComplexType::DerivationRestriction) { // 3.2
      const XsdAttributeUse::List currentUses = complexType->attributeUses();

      // 3.2.1 and 3.2.2 As we also keep the prohibited attributes as objects, the algorithm below
      // handles both the same way

      // add only these attribute uses of the base type that match one of the following criteria:
      //   1: there is no attribute use with the same name in type
      //   2: there is no attribute with the same name marked as prohibited in type
      for (int j = 0; j < uses.count(); ++j) {
         const XsdAttributeUse::Ptr use = uses.at(j);
         bool found = false;
         for (int k = 0; k < currentUses.count(); ++k) {
            if (use->attribute()->name(m_namePool) == currentUses.at(k)->attribute()->name(m_namePool)) {
               found = true;

               // check if prohibited usage is violated
               if ((use->useType() == XsdAttributeUse::ProhibitedUse) &&
                     (currentUses.at(k)->useType() != XsdAttributeUse::ProhibitedUse)) {
                  m_context->error(QtXmlPatterns::tr("%1 attribute in %2 must have %3 use like in base type %4.")
                                   .arg(formatAttribute(use->attribute()->displayName(m_namePool)))
                                   .arg(formatType(m_namePool, complexType))
                                   .arg(formatData("prohibited"))
                                   .arg(formatType(m_namePool, complexBaseType)),
                                   XsdSchemaContext::XSDError, sourceLocation(complexType));
                  return;
               }

               break;
            }
         }

         if (!found && uses.at(j)->useType() != XsdAttributeUse::ProhibitedUse) {
            complexType->addAttributeUse(uses.at(j));
         }
      }
   } else if (complexType->derivationMethod() == XsdComplexType::DerivationExtension) { // 3.1
      QHash<QXmlName, XsdAttributeUse::Ptr> availableUses;

      // fill hash with attribute uses of current type for faster lookup
      {
         const XsdAttributeUse::List attributeUses = complexType->attributeUses();

         for (int i = 0; i < attributeUses.count(); ++i) {
            availableUses.insert(attributeUses.at(i)->attribute()->name(m_namePool), attributeUses.at(i));
         }
      }

      // just add the attribute uses of the base type
      for (int i = 0; i < uses.count(); ++i) {
         const XsdAttributeUse::Ptr currentAttributeUse = uses.at(i);

         // if the base type defines the attribute as prohibited but we override it in current type, then don't copy the prohibited attribute use
         if ((currentAttributeUse->useType() == XsdAttributeUse::ProhibitedUse) &&
               availableUses.contains(currentAttributeUse->attribute()->name(m_namePool))) {
            continue;
         }

         complexType->addAttributeUse(uses.at(i));
      }
   }

   // handle attribute wildcards: @see http://www.w3.org/TR/xmlschema11-1/#dcl.ctd.anyatt

   // 1
   const XsdWildcard::Ptr completeWildcard(complexType->attributeWildcard());

   if (complexType->derivationMethod() == XsdComplexType::DerivationRestriction) {
      if (complexType->wxsSuperType()->isComplexType() && complexType->wxsSuperType()->isDefinedBySchema()) {
         const XsdComplexType::Ptr complexBaseType(complexType->wxsSuperType());
         if (complexType->attributeWildcard()) {
            if (complexBaseType->attributeWildcard()) {
               if (!isValidWildcardRestriction(complexType->attributeWildcard(), complexBaseType->attributeWildcard())) {
                  m_context->error(
                     QtXmlPatterns::tr("Attribute wildcard of %1 is not a valid restriction of attribute wildcard of base type %2.")
                     .arg(formatType(m_namePool, complexType))
                     .arg(formatType(m_namePool, complexBaseType)),
                     XsdSchemaContext::XSDError, sourceLocation(complexType));
                  return;
               }
            } else {
               m_context->error(QtXmlPatterns::tr("%1 has attribute wildcard but its base type %2 has not.")
                                .arg(formatType(m_namePool, complexType))
                                .arg(formatType(m_namePool, complexBaseType)),
                                XsdSchemaContext::XSDError, sourceLocation(complexType));
               return;
            }
         }
      }
      complexType->setAttributeWildcard(completeWildcard); // 2.1
   } else if (complexType->derivationMethod() == XsdComplexType::DerivationExtension) {
      XsdWildcard::Ptr baseWildcard; // 2.2.1
      if (complexType->wxsSuperType()->isComplexType() && complexType->wxsSuperType()->isDefinedBySchema()) {
         baseWildcard = XsdComplexType::Ptr(complexType->wxsSuperType())->attributeWildcard();   // 2.2.1.1
      } else {
         baseWildcard = XsdWildcard::Ptr();   // 2.2.1.2
      }

      if (!baseWildcard) {
         complexType->setAttributeWildcard(completeWildcard); // 2.2.2.1
      } else if (!completeWildcard) {
         complexType->setAttributeWildcard(baseWildcard); // 2.2.2.2
      } else {
         XsdWildcard::Ptr unionWildcard = XsdSchemaHelper::wildcardUnion(completeWildcard, baseWildcard);
         if (unionWildcard) {
            unionWildcard->setProcessContents(completeWildcard->processContents());
            complexType->setAttributeWildcard(unionWildcard); // 2.2.2.3
         } else {
            m_context->error(
               QtXmlPatterns::tr("Union of attribute wildcard of type %1 and attribute wildcard of its base type %2 is not expressible.")
               .arg(formatType(m_namePool, complexType))
               .arg(formatType(m_namePool, complexBaseType)),
               XsdSchemaContext::XSDError, sourceLocation(complexType));
            return;
         }
      }
   }
}

void XsdSchemaResolver::resolveEnumerationFacetValues()
{
   XsdSimpleType::List simpleTypes;

   // first collect the global simple types
   const SchemaType::List types = m_schema->types();
   for (int i = 0; i < types.count(); ++i) {
      if (types.at(i)->isSimpleType()) {
         simpleTypes.append(types.at(i));
      }
   }

   // then collect all anonymous simple types
   const SchemaType::List anonymousTypes = m_schema->anonymousTypes();
   for (int i = 0; i < anonymousTypes.count(); ++i) {
      if (anonymousTypes.at(i)->isSimpleType()) {
         simpleTypes.append(anonymousTypes.at(i));
      }
   }
   // process all simple types
   for (int i = 0; i < simpleTypes.count(); ++i) {
      const XsdSimpleType::Ptr simpleType = simpleTypes.at(i);

      // we resolve the enumeration values only for xs:QName and xs:NOTATION based types
      if (BuiltinTypes::xsQName->wxsTypeMatches(simpleType) ||
            BuiltinTypes::xsNOTATION->wxsTypeMatches(simpleType)) {
         const XsdFacet::Hash facets = simpleType->facets();
         if (facets.contains(XsdFacet::Enumeration)) {
            AtomicValue::List newValues;

            const XsdFacet::Ptr facet = facets.value(XsdFacet::Enumeration);
            const AtomicValue::List values = facet->multiValue();
            for (int j = 0; j < values.count(); ++j) {
               const AtomicValue::Ptr value = values.at(j);

               Q_ASSERT(m_enumerationFacetValues.contains(value));
               const NamespaceSupport support( m_enumerationFacetValues.value(value) );

               const QString qualifiedName = value->as<DerivedString<TypeString> >()->stringValue();
               if (!XPathHelper::isQName(qualifiedName)) {
                  m_context->error(QtXmlPatterns::tr("Enumeration facet contains invalid content: {%1} is not a value of type %2.")
                                   .arg(formatData(qualifiedName))
                                   .arg(formatType(m_namePool, BuiltinTypes::xsQName)),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }

               QXmlName qNameValue;
               bool result = support.processName(qualifiedName, NamespaceSupport::ElementName, qNameValue);
               if (!result) {
                  m_context->error(QtXmlPatterns::tr("Namespace prefix of qualified name %1 is not defined.").arg(formatData(
                                      qualifiedName)),
                                   XsdSchemaContext::XSDError, sourceLocation(simpleType));
                  return;
               }

               newValues.append(QNameValue::fromValue(m_namePool, qNameValue));
            }
            facet->setMultiValue(newValues);
         }
      }
   }
}

QSourceLocation XsdSchemaResolver::sourceLocation(const NamedSchemaComponent::Ptr component) const
{
   if (m_componentLocationHash.contains(component)) {
      return m_componentLocationHash.value(component);
   } else {
      QSourceLocation location;
      location.setLine(1);
      location.setColumn(1);
      location.setUri( QUrl("dummyUri"));

      return location;
   }
}

XsdFacet::Hash XsdSchemaResolver::complexTypeFacets(const XsdComplexType::Ptr &complexType) const
{
   for (int i = 0; i < m_complexBaseTypes.count(); ++i) {
      if (m_complexBaseTypes.at(i).complexType == complexType) {
         return m_complexBaseTypes.at(i).facets;
      }
   }

   return XsdFacet::Hash();
}

void XsdSchemaResolver::checkRedefinedGroups()
{
   for (int i = 0; i < m_redefinedGroups.count(); ++i) {
      const RedefinedGroups item = m_redefinedGroups.at(i);

      // create dummy particles...
      const XsdParticle::Ptr redefinedParticle(new XsdParticle());
      redefinedParticle->setTerm(item.redefinedGroup);
      const XsdParticle::Ptr particle(new XsdParticle());
      particle->setTerm(item.group);

      // so that we can pass them to XsdParticleChecker::subsumes()
      QString errorMsg;
      if (!XsdParticleChecker::subsumes(particle, redefinedParticle, m_context, errorMsg)) {
         m_context->error(QtXmlPatterns::tr("%1 element %2 is not a valid restriction of the %3 element it redefines: %4.")
                          .arg(formatElement("group"))
                          .arg(formatData(item.redefinedGroup->displayName(m_namePool)))
                          .arg(formatElement("group"))
                          .arg(errorMsg),
                          XsdSchemaContext::XSDError, sourceLocation(item.redefinedGroup));
         return;
      }
   }
}

void XsdSchemaResolver::checkRedefinedAttributeGroups()
{
   for (int i = 0; i < m_redefinedAttributeGroups.count(); ++i) {
      const RedefinedAttributeGroups item = m_redefinedAttributeGroups.at(i);

      QString errorMsg;
      if (!XsdSchemaHelper::isValidAttributeGroupRestriction(item.redefinedGroup, item.group, m_context, errorMsg)) {
         m_context->error(QtXmlPatterns::tr("%1 element %2 is not a valid restriction of the %3 element it redefines: %4.")
                          .arg(formatElement("attributeGroup"))
                          .arg(formatData(item.redefinedGroup->displayName(m_namePool)))
                          .arg(formatElement("attributeGroup"))
                          .arg(errorMsg),
                          XsdSchemaContext::XSDError, sourceLocation(item.redefinedGroup));
         return;
      }
   }
}

AnySimpleType::Ptr XsdSchemaResolver::findPrimitiveType(const AnySimpleType::Ptr &type,
      QSet<AnySimpleType::Ptr> &visitedTypes)
{
   if (visitedTypes.contains(type)) {
      // found invalid circular reference...
      return AnySimpleType::Ptr();
   } else {
      visitedTypes.insert(type);
   }

   const QXmlName typeName = type->name(m_namePool);
   if (typeName == BuiltinTypes::xsString->name(m_namePool) ||
         typeName == BuiltinTypes::xsBoolean->name(m_namePool) ||
         typeName == BuiltinTypes::xsFloat->name(m_namePool) ||
         typeName == BuiltinTypes::xsDouble->name(m_namePool) ||
         typeName == BuiltinTypes::xsDecimal->name(m_namePool) ||
         typeName == BuiltinTypes::xsDuration->name(m_namePool) ||
         typeName == BuiltinTypes::xsDateTime->name(m_namePool) ||
         typeName == BuiltinTypes::xsTime->name(m_namePool) ||
         typeName == BuiltinTypes::xsDate->name(m_namePool) ||
         typeName == BuiltinTypes::xsGYearMonth->name(m_namePool) ||
         typeName == BuiltinTypes::xsGYear->name(m_namePool) ||
         typeName == BuiltinTypes::xsGMonthDay->name(m_namePool) ||
         typeName == BuiltinTypes::xsGDay->name(m_namePool) ||
         typeName == BuiltinTypes::xsGMonth->name(m_namePool) ||
         typeName == BuiltinTypes::xsHexBinary->name(m_namePool) ||
         typeName == BuiltinTypes::xsBase64Binary->name(m_namePool) ||
         typeName == BuiltinTypes::xsAnyURI->name(m_namePool) ||
         typeName == BuiltinTypes::xsQName->name(m_namePool) ||
         typeName == BuiltinTypes::xsNOTATION->name(m_namePool) ||
         typeName == BuiltinTypes::xsAnySimpleType->name(m_namePool)) {
      return type;
   } else {
      if (type->wxsSuperType()) {
         return findPrimitiveType(type->wxsSuperType(), visitedTypes);
      } else {
         return AnySimpleType::Ptr();
      }
   }
}

QT_END_NAMESPACE
