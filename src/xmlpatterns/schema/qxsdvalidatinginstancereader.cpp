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

#include <qxsdvalidatinginstancereader_p.h>

#include <qfile.h>
#include <qxmlquery.h>
#include <qxmlresultitems.h>

#include "qabstractdatetime_p.h"
#include "qacceltreeresourceloader_p.h"
#include "qbase64binary_p.h"
#include "qboolean_p.h"
#include "qcommonnamespaces_p.h"
#include "qderivedinteger_p.h"
#include "qduration_p.h"
#include "qgenericstaticcontext_p.h"
#include "qhexbinary_p.h"
#include "qnamespaceresolver_p.h"
#include "qpatternplatform_p.h"
#include "qqnamevalue_p.h"
#include "qsourcelocationreflection_p.h"
#include "qvaluefactory_p.h"
#include "qxmlnamepool.h"
#include "qxmlquery_p.h"
#include "qxmlschema_p.h"
#include "qxsdschemahelper_p.h"
#include "qxsdschemamerger_p.h"
#include "qxsdstatemachine_p.h"
#include "qxsdstatemachinebuilder_p.h"
#include "qxsdtypechecker_p.h"
#include "qxsdschemadebugger_p.h"

using namespace QPatternist;

namespace QPatternist {

template <>
template <>
bool XsdStateMachine<XsdTerm::Ptr>::inputEqualsTransition<QXmlName>(QXmlName name, XsdTerm::Ptr term) const
{
   if (term->isElement()) {
      return (XsdElement::Ptr(term)->name(m_namePool) == name);

   } else if (term->isWildcard()) {
      // wildcards using XsdWildcard::absentNamespace, so we have to fix that here
      if (name.namespaceURI() == StandardNamespaces::empty) {
         name.setNamespaceURI(m_namePool->allocateNamespace(XsdWildcard::absentNamespace()));
      }

      return XsdSchemaHelper::wildcardAllowsExpandedName(name, XsdWildcard::Ptr(term), m_namePool);
   }

   return false;
}
}

XsdValidatingInstanceReader::XsdValidatingInstanceReader(XsdValidatedXmlNodeModel *model, const QUrl &documentUri,
      const XsdSchemaContext::Ptr &context)
   : XsdInstanceReader(model, context)
   , m_model(model)
   , m_namePool(m_context->namePool())
   , m_xsiNilName(m_namePool->allocateQName(CommonNamespaces::XSI, QLatin1String("nil")))
   , m_xsiTypeName(m_namePool->allocateQName(CommonNamespaces::XSI, QLatin1String("type")))
   , m_xsiSchemaLocationName(m_namePool->allocateQName(CommonNamespaces::XSI, QLatin1String("schemaLocation")))
   , m_xsiNoNamespaceSchemaLocationName(m_namePool->allocateQName(CommonNamespaces::XSI,
                                        QLatin1String("noNamespaceSchemaLocation")))
   , m_documentUri(documentUri)
{
   m_idRefsType = m_context->schemaTypeFactory()->createSchemaType(m_namePool->allocateQName(CommonNamespaces::WXS,
                  QLatin1String("IDREFS")));
}

void XsdValidatingInstanceReader::addSchema(const XsdSchema::Ptr &schema, const QUrl &locationUrl)
{
   if (!m_mergedSchemas.contains(locationUrl)) {
      m_mergedSchemas.insert(locationUrl, QStringList() << schema->targetNamespace());
   } else {
      QStringList &targetNamespaces = m_mergedSchemas[locationUrl];
      if (targetNamespaces.contains(schema->targetNamespace())) {
         return;
      }

      targetNamespaces.append(schema->targetNamespace());
   }

   const XsdSchemaMerger merger(m_schema, schema);
   m_schema = merger.mergedSchema();
   /*
       XsdSchemaDebugger dbg(m_namePool);
       dbg.dumpSchema(m_schema);
   */
}

bool XsdValidatingInstanceReader::read()
{
   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         return true;
      }

      if (isStartElement()) {
         const QXmlName elementName = name();
         const QXmlItem currentItem = item();
         bool hasStateMachine = false;
         XsdElement::Ptr processedElement;

         if (!validate(hasStateMachine, processedElement)) {
            return false;
         }

         read();

         if (processedElement) { // for wildcard with 'skip' we have no element
            m_model->setAssignedElement(currentItem.toNodeModelIndex(), processedElement);

            // check identity constraints after all child nodes have been
            // validated, so that we know there assigned types
            validateIdentityConstraint(processedElement, currentItem);
         }

         if (!m_stateMachines.isEmpty() && hasStateMachine) {
            if (!m_stateMachines.top().inEndState()) {
               error(QtXmlPatterns::tr("Element %1 is missing child element.").formatArg(formatKeyword(m_namePool->displayName(
                        elementName))));
               return false;
            }
            m_stateMachines.pop();
         }
      }
   }

   // final validations

   // check IDREF occurrences
   const QStringList ids = m_model->idIdRefBindingIds();
   QSetIterator<QString> it(m_idRefs);
   while (it.hasNext()) {
      const QString id = it.next();
      if (!ids.contains(id)) {
         error(QtXmlPatterns::tr("There is one IDREF value with no corresponding ID: %1.").formatArg(formatKeyword(id)));
         return false;
      }
   }

   return true;
}

void XsdValidatingInstanceReader::error(const QString &msg) const
{
   m_context.data()->error(msg, XsdSchemaContext::XSDError, sourceLocation());
}

bool XsdValidatingInstanceReader::loadSchema(const QString &targetNamespace, const QUrl &location)
{
   const std::unique_ptr<QNetworkReply> reply(AccelTreeResourceLoader::load(location, m_context->networkAccessManager(),
                                      m_context, AccelTreeResourceLoader::ContinueOnError));
   if (! reply) {
      return true;
   }

   // we have to create a separated schema context here, that however shares the type factory
   XsdSchemaContext::Ptr context(new XsdSchemaContext(m_namePool));
   context->m_schemaTypeFactory = m_context->m_schemaTypeFactory;

   QXmlSchemaPrivate schema(context);
   schema.load(reply.get(), location, targetNamespace);

   if (!schema.isValid()) {
      error(QtXmlPatterns::tr("Loaded schema file is invalid."));
      return false;
   }

   addSchema(schema.m_schemaParserContext->schema(), location);

   return true;
}

bool XsdValidatingInstanceReader::validate(bool &hasStateMachine, XsdElement::Ptr &processedElement)
{
   // first check if a custom schema is defined
   if (hasAttribute(m_xsiSchemaLocationName)) {
      const QString schemaLocation = attribute(m_xsiSchemaLocationName);
      const QStringList parts = schemaLocation.split(QLatin1Char(' '), QStringParser::SkipEmptyParts);
      if ((parts.count() % 2) == 1) {
         error(QtXmlPatterns::tr("%1 contains invalid data.").formatArg(formatKeyword(m_namePool, m_xsiSchemaLocationName)));
         return false;
      }

      for (int i = 0; i < parts.count(); i += 2) {
         const QString identifier = QString::fromLatin1("%1 %2").formatArg(parts.at(i)).formatArg(parts.at(i + 1));
         if (m_processedSchemaLocations.contains(identifier)) {
            continue;
         } else {
            m_processedSchemaLocations.insert(identifier);
         }

         // check constraint 4) from http://www.w3.org/TR/xmlschema-1/#schema-loc (only valid for XML Schema 1.0?)
         if (m_processedNamespaces.contains(parts.at(i))) {
            error(QtXmlPatterns::tr("xsi:schemaLocation namespace %1 has already appeared earlier in the instance document.").formatArg(
                     formatKeyword(parts.at(i))));
            return false;
         }

         QUrl url(parts.at(i + 1));
         if (url.isRelative()) {
            Q_ASSERT(m_documentUri.isValid());

            url = m_documentUri.resolved(url);
         }

         loadSchema(parts.at(i), url);
      }
   }

   if (hasAttribute(m_xsiNoNamespaceSchemaLocationName)) {
      const QString schemaLocation = attribute(m_xsiNoNamespaceSchemaLocationName);

      if (!m_processedSchemaLocations.contains(schemaLocation)) {
         m_processedSchemaLocations.insert(schemaLocation);

         if (m_processedNamespaces.contains(QString())) {
            error(QtXmlPatterns::tr("xsi:noNamespaceSchemaLocation cannot appear after the first no-namespace element or attribute."));
            return false;
         }

         QUrl url(schemaLocation);
         if (url.isRelative()) {
            Q_ASSERT(m_documentUri.isValid());

            url = m_documentUri.resolved(url);
         }

         loadSchema(QString(), url);
      }
   }

   m_processedNamespaces.insert(m_namePool->stringForNamespace(name().namespaceURI()));

   if (!m_schema) {
      error(QtXmlPatterns::tr("No schema defined for validation."));
      return false;
   }

   // check if we are 'inside' a type definition
   if (m_stateMachines.isEmpty()) {
      // find out the type of the top-level element
      XsdElement::Ptr element = elementByName(name());
      if (!element) {
         if (!hasAttribute(m_xsiTypeName)) {
            error(QtXmlPatterns::tr("No definition for element %1 available.").formatArg(formatKeyword(m_namePool, name())));
            return false;
         }

         // This instance document has an element with no definition in the schema
         // but an explicitly given type, that is fine according to the spec.
         // We will create an element definition manually here and continue the
         // normal validation process
         element = XsdElement::Ptr(new XsdElement());
         element->setName(name());
         element->setIsAbstract(false);
         element->setIsNillable(hasAttribute(m_xsiNilName));

         const QString type = qNameAttribute(m_xsiTypeName);
         const QXmlName typeName = convertToQName(type);

         const SchemaType::Ptr elementType = typeByName(typeName);
         if (!elementType) {
            error(QtXmlPatterns::tr("Specified type %1 is not known to the schema.").formatArg(formatType(m_namePool, typeName)));
            return false;
         }
         element->setType(elementType);
      }

      // rememeber the element we process
      processedElement = element;

      if (!validateElement(element, hasStateMachine)) {
         return false;
      }

   } else {
      if (!m_stateMachines.top().proceed<QXmlName>(name())) {
         error(QtXmlPatterns::tr("Element %1 is not defined in this scope.").formatArg(formatKeyword(m_namePool, name())));
         return false;
      }

      const XsdTerm::Ptr term = m_stateMachines.top().lastTransition();
      if (term->isElement()) {
         const XsdElement::Ptr element(term);

         // rememeber the element we process
         processedElement = element;

         if (!validateElement(element, hasStateMachine)) {
            return false;
         }

      } else {
         const XsdWildcard::Ptr wildcard(term);
         if (wildcard->processContents() != XsdWildcard::Skip) {
            XsdElement::Ptr elementDeclaration = elementByName(name());
            if (!elementDeclaration) {
               if (hasAttribute(m_xsiTypeName)) {
                  // This instance document has an element with no definition in the schema
                  // but an explicitly given type, that is fine according to the spec.
                  // We will create an element definition manually here and continue the
                  // normal validation process
                  elementDeclaration = XsdElement::Ptr(new XsdElement());
                  elementDeclaration->setName(name());
                  elementDeclaration->setIsAbstract(false);
                  elementDeclaration->setIsNillable(hasAttribute(m_xsiNilName));

                  const QString type = qNameAttribute(m_xsiTypeName);
                  const QXmlName typeName = convertToQName(type);

                  const SchemaType::Ptr elementType = typeByName(typeName);
                  if (!elementType) {
                     error(QtXmlPatterns::tr("Specified type %1 is not known to the schema.").formatArg(formatType(m_namePool, typeName)));
                     return false;
                  }
                  elementDeclaration->setType(elementType);
               }
            }

            if (!elementDeclaration) {
               if (wildcard->processContents() == XsdWildcard::Strict) {
                  error(QtXmlPatterns::tr("Declaration for element %1 does not exist.").formatArg(formatKeyword(m_namePool->displayName(
                           name()))));
                  return false;
               } else {
                  // in this case we put a state machine for the xs:anyType on the statemachine stack,
                  // so we accept every content of this element

                  createAndPushStateMachine(anyType()->contentType()->particle());
                  hasStateMachine = true;
               }
            } else {
               if (!validateElement(elementDeclaration, hasStateMachine)) {
                  if (wildcard->processContents() == XsdWildcard::Strict) {
                     error(QtXmlPatterns::tr("Element %1 contains invalid content.").formatArg(formatKeyword(m_namePool->displayName(name()))));
                     return false;
                  }
               }

               // rememeber the type of that element node
               m_model->setAssignedType(item().toNodeModelIndex(), elementDeclaration->type());
            }
         } else { // wildcard process contents type is Skip
            // in this case we put a state machine for the xs:anyType on the statemachine stack,
            // so we accept every content of this element

            const XsdWildcard::Ptr wildcard(new XsdWildcard());
            wildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Any);
            wildcard->setProcessContents(XsdWildcard::Skip);

            const XsdParticle::Ptr outerParticle(new XsdParticle());
            outerParticle->setMinimumOccurs(1);
            outerParticle->setMaximumOccurs(1);

            const XsdParticle::Ptr innerParticle(new XsdParticle());
            innerParticle->setMinimumOccurs(0);
            innerParticle->setMaximumOccursUnbounded(true);
            innerParticle->setTerm(wildcard);

            const XsdModelGroup::Ptr outerModelGroup(new XsdModelGroup());
            outerModelGroup->setCompositor(XsdModelGroup::SequenceCompositor);
            outerModelGroup->setParticles(XsdParticle::List() << innerParticle);
            outerParticle->setTerm(outerModelGroup);

            createAndPushStateMachine(outerParticle);
            hasStateMachine = true;
         }
      }
   }

   return true;
}

void XsdValidatingInstanceReader::createAndPushStateMachine(const XsdParticle::Ptr &particle)
{
   XsdStateMachine<XsdTerm::Ptr> stateMachine(m_namePool);

   XsdStateMachineBuilder builder(&stateMachine, m_namePool, XsdStateMachineBuilder::ValidatingMode);
   const XsdStateMachine<XsdTerm::Ptr>::StateId endState = builder.reset();
   const XsdStateMachine<XsdTerm::Ptr>::StateId startState = builder.buildParticle(particle, endState);
   builder.addStartState(startState);

   /*
       QString fileName = QString("/tmp/foo_%1.dot").formatArg(m_namePool->displayName(complexType->name(m_namePool)));
       QString pngFileName = QString("/tmp/foo_%1.png").formatArg(m_namePool->displayName(complexType->name(m_namePool)));
       QFile file(fileName);
       file.open(QIODevice::WriteOnly);
       stateMachine.outputGraph(&file, "Hello");
       file.close();
       ::system(QString("dot -Tpng %1 -o%2").formatArg(fileName).formatArg(pngFileName).toLatin1().data());
   */

   stateMachine = stateMachine.toDFA();

   m_stateMachines.push(stateMachine);
}

bool XsdValidatingInstanceReader::validateElement(const XsdElement::Ptr &declaration, bool &hasStateMachine)
{
   // http://www.w3.org/TR/xmlschema11-1/#d0e10998

   bool isNilled = false;

   // 1 tested already, 'declaration' corresponds D

   // 2
   if (declaration->isAbstract()) {
      error(QtXmlPatterns::tr("Element %1 is declared as abstract.").formatArg(formatKeyword(declaration->displayName(
               m_namePool))));
      return false;
   }

   // 3
   if (!declaration->isNillable()) {
      if (hasAttribute(m_xsiNilName)) {
         error(QtXmlPatterns::tr("Element %1 is not nillable.").formatArg(formatKeyword(declaration->displayName(m_namePool))));
         return false; // 3.1
      }
   } else {
      if (hasAttribute(m_xsiNilName)) {
         const QString value = attribute(m_xsiNilName);
         const Boolean::Ptr nil = Boolean::fromLexical(value);
         if (nil->hasError()) {
            error(QtXmlPatterns::tr("Attribute %1 contains invalid data: %2").formatArg(formatKeyword(QLatin1String("nil."))).formatArg(
                     formatData(value)));
            return false;
         }

         // 3.2.3
         if (nil->as<Boolean>()->value() == true) {
            // 3.2.3.1
            if (hasChildElement() || hasChildText()) {
               error(QtXmlPatterns::tr("Element contains content although it is nillable."));
               return false;
            }

            // 3.2.3.2
            if (declaration->valueConstraint() && declaration->valueConstraint()->variety() == XsdElement::ValueConstraint::Fixed) {
               error(QtXmlPatterns::tr("Fixed value constraint not allowed if element is nillable."));
               return false;
            }
         }

         isNilled = nil->as<Boolean>()->value();
      }
   }

   SchemaType::Ptr finalElementType = declaration->type();

   // 4
   if (hasAttribute(m_xsiTypeName)) {
      const QString type = qNameAttribute(m_xsiTypeName);
      const QXmlName typeName = convertToQName(type);

      const SchemaType::Ptr elementType = typeByName(typeName);
      // 4.1
      if (!elementType) {
         error(QtXmlPatterns::tr("Specified type %1 is not known to the schema.").formatArg(formatType(m_namePool, typeName)));
         return false;
      }

      // 4.2
      SchemaType::DerivationConstraints constraints = Qt::EmptyFlag;

      if (declaration->disallowedSubstitutions() & NamedSchemaComponent::ExtensionConstraint) {
         constraints |= SchemaType::ExtensionConstraint;
      }
      if (declaration->disallowedSubstitutions() & NamedSchemaComponent::RestrictionConstraint) {
         constraints |= SchemaType::RestrictionConstraint;
      }

      if (!XsdSchemaHelper::isValidlySubstitutable(elementType, declaration->type(), constraints)) {
         if (declaration->type()->name(m_namePool) != BuiltinTypes::xsAnyType->name(
                  m_namePool)) { // xs:anyType is a valid substitutable type here
            error(QtXmlPatterns::tr("Specified type %1 is not validly substitutable with element type %2.").formatArg(formatType(
                     m_namePool, elementType)).formatArg(formatType(m_namePool, declaration->type())));
            return false;
         }
      }

      finalElementType = elementType;
   }

   if (!validateElementType(declaration, finalElementType, isNilled, hasStateMachine)) {
      return false;
   }

   return true;
}

bool XsdValidatingInstanceReader::validateElementType(const XsdElement::Ptr &declaration, const SchemaType::Ptr &type,
      bool isNilled, bool &hasStateMachine)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#d0e11749

   // 1 checked already

   // 2
   if (type->isComplexType() && type->isDefinedBySchema()) {
      if (XsdComplexType::Ptr(type)->isAbstract()) {
         error(QtXmlPatterns::tr("Complex type %1 is not allowed to be abstract.").formatArg(formatType(m_namePool, type)));
         return false;
      }
   }

   // 3
   if (type->isSimpleType()) {
      return validateElementSimpleType(declaration, type, isNilled);   // 3.1
   } else {
      return validateElementComplexType(declaration, type, isNilled, hasStateMachine);   // 3.2
   }
}

bool XsdValidatingInstanceReader::validateElementSimpleType(const XsdElement::Ptr &declaration,
      const SchemaType::Ptr &type, bool isNilled)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#d0e11749

   // 3.1.1
   const QSet<QXmlName> allowedAttributes(QSet<QXmlName>() << m_xsiNilName << m_xsiTypeName << m_xsiSchemaLocationName <<
                                          m_xsiNoNamespaceSchemaLocationName);
   QSet<QXmlName> elementAttributes = attributeNames();
   elementAttributes.subtract(allowedAttributes);
   if (!elementAttributes.isEmpty()) {
      error(QtXmlPatterns::tr("Element %1 contains not allowed attributes.").formatArg(formatKeyword(declaration->displayName(
               m_namePool))));
      return false;
   }

   // 3.1.2
   if (hasChildElement()) {
      error(QtXmlPatterns::tr("Element %1 contains not allowed child element.").formatArg(formatKeyword(declaration->displayName(
               m_namePool))));
      return false;
   }

   // 3.1.3
   if (!isNilled) {
      const XsdFacet::Hash facets = XsdTypeChecker::mergedFacetsForType(type, m_context);

      QString actualValue;
      if (hasChildText()) {
         actualValue = XsdTypeChecker::normalizedValue(text(), facets);
      } else {
         if (declaration->valueConstraint()) {
            actualValue = XsdTypeChecker::normalizedValue(declaration->valueConstraint()->value(), facets);
         }
      }

      QString errorMsg;
      AnySimpleType::Ptr boundType;

      const XsdTypeChecker checker(m_context, namespaceBindings(item().toNodeModelIndex()), sourceLocation());
      if (!checker.isValidString(actualValue, type, errorMsg, &boundType)) {
         error(QtXmlPatterns::tr("Content of element %1 does not match its type definition: %2.").formatArg(formatKeyword(
                  declaration->displayName(m_namePool))).formatArg(errorMsg));
         return false;
      }

      // additional check
      if (declaration->valueConstraint() && declaration->valueConstraint()->variety() == XsdElement::ValueConstraint::Fixed) {
         const QString actualConstraintValue = XsdTypeChecker::normalizedValue(declaration->valueConstraint()->value(), facets);
         if (!text().isEmpty() && !checker.valuesAreEqual(actualValue, actualConstraintValue, type)) {
            error(QtXmlPatterns::tr("Content of element %1 does not match defined value constraint.").formatArg(formatKeyword(
                     declaration->displayName(m_namePool))));
            return false;
         }
      }
   }

   // 4  checked in validateElement already

   // rememeber the type of that element node
   m_model->setAssignedType(item().toNodeModelIndex(), type);

   const XsdFacet::Hash facets = XsdTypeChecker::mergedFacetsForType(type, m_context);
   const QString actualValue = XsdTypeChecker::normalizedValue(text(), facets);

   if (BuiltinTypes::xsID->wxsTypeMatches(type)) {
      addIdIdRefBinding(actualValue, declaration);
   }

   if (m_idRefsType->wxsTypeMatches(type)) {
      const QStringList idRefs = actualValue.split(QLatin1Char(' '), QStringParser::SkipEmptyParts);
      for (int i = 0; i < idRefs.count(); ++i) {
         m_idRefs.insert(idRefs.at(i));
      }
   } else if (BuiltinTypes::xsIDREF->wxsTypeMatches(type)) {
      m_idRefs.insert(actualValue);
   }

   return true;
}

static bool hasIDAttributeUse(const XsdAttributeUse::List &uses)
{
   const int count = uses.count();
   for (int i = 0; i < count; ++i) {
      if (BuiltinTypes::xsID->wxsTypeMatches(uses.at(i)->attribute()->type())) {
         return true;
      }
   }

   return false;
}

bool XsdValidatingInstanceReader::validateElementComplexType(const XsdElement::Ptr &declaration,
      const SchemaType::Ptr &type, bool isNilled, bool &hasStateMachine)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#cvc-complex-type

   // 1
   if (!isNilled) {
      XsdComplexType::Ptr complexType;

      if (type->isDefinedBySchema()) {
         complexType = XsdComplexType::Ptr(type);
      } else {
         if (type->name(m_namePool) == BuiltinTypes::xsAnyType->name(m_namePool)) {
            complexType = anyType();
         }
      }

      if (complexType) {
         // 1.1
         if (complexType->contentType()->variety() == XsdComplexType::ContentType::Empty) {
            if (hasChildText() || hasChildElement()) {
               error(QtXmlPatterns::tr("Element %1 contains not allowed child content.").formatArg(formatKeyword(declaration->displayName(
                        m_namePool))));
               return false;
            }
         }

         // 1.2
         if (complexType->contentType()->variety() == XsdComplexType::ContentType::Simple) {
            if (hasChildElement()) {
               error(QtXmlPatterns::tr("Element %1 contains not allowed child element.").formatArg(formatKeyword(declaration->displayName(
                        m_namePool))));
               return false;
            }

            const XsdFacet::Hash facets = XsdTypeChecker::mergedFacetsForType(complexType->contentType()->simpleType(), m_context);
            QString actualValue;
            if (hasChildText()) {
               actualValue = XsdTypeChecker::normalizedValue(text(), facets);
            } else {
               if (declaration->valueConstraint()) {
                  actualValue = XsdTypeChecker::normalizedValue(declaration->valueConstraint()->value(), facets);
               }
            }

            QString errorMsg;
            AnySimpleType::Ptr boundType;
            const XsdTypeChecker checker(m_context, namespaceBindings(item().toNodeModelIndex()), sourceLocation());
            if (!checker.isValidString(actualValue, complexType->contentType()->simpleType(), errorMsg, &boundType)) {
               error(QtXmlPatterns::tr("Content of element %1 does not match its type definition: %2.").formatArg(formatKeyword(
                        declaration->displayName(m_namePool))).formatArg(errorMsg));
               return false;
            }

            // additional check
            if (declaration->valueConstraint() && declaration->valueConstraint()->variety() == XsdElement::ValueConstraint::Fixed) {
               if (!checker.valuesAreEqual(actualValue, declaration->valueConstraint()->value(), boundType)) {
                  error(QtXmlPatterns::tr("Content of element %1 does not match defined value constraint.").formatArg(formatKeyword(
                           declaration->displayName(m_namePool))));
                  return false;
               }
            }
         }

         // 1.3
         if (complexType->contentType()->variety() == XsdComplexType::ContentType::ElementOnly) {
            if (!text().simplified().isEmpty()) {
               error(QtXmlPatterns::tr("Element %1 contains not allowed text content.").formatArg(formatKeyword(declaration->displayName(
                        m_namePool))));
               return false;
            }
         }

         // 1.4
         if (complexType->contentType()->variety() == XsdComplexType::ContentType::ElementOnly ||
               complexType->contentType()->variety() == XsdComplexType::ContentType::Mixed) {

            if (complexType->contentType()->particle()) {
               createAndPushStateMachine(complexType->contentType()->particle());
               hasStateMachine = true;
            }

            // additional check
            if (complexType->contentType()->variety() == XsdComplexType::ContentType::Mixed) {
               if (declaration->valueConstraint() && declaration->valueConstraint()->variety() == XsdElement::ValueConstraint::Fixed) {
                  if (hasChildElement()) {
                     error(QtXmlPatterns::tr("Element %1 cannot contain other elements, as it has fixed content.").formatArg(formatKeyword(
                              declaration->displayName(m_namePool))));
                     return false;
                  }

                  const XsdFacet::Hash facets = XsdTypeChecker::mergedFacetsForType(complexType->contentType()->simpleType(), m_context);
                  QString actualValue;
                  if (hasChildText()) {
                     actualValue = XsdTypeChecker::normalizedValue(text(), facets);
                  } else {
                     if (declaration->valueConstraint()) {
                        actualValue = XsdTypeChecker::normalizedValue(declaration->valueConstraint()->value(), facets);
                     }
                  }

                  if (actualValue != declaration->valueConstraint()->value()) {
                     error(QtXmlPatterns::tr("Content of element %1 does not match defined value constraint.").formatArg(formatKeyword(
                              declaration->displayName(m_namePool))));
                     return false;
                  }
               }
            }
         }
      }
   }

   if (type->isDefinedBySchema()) {
      const XsdComplexType::Ptr complexType(type);

      // create a lookup hash for faster access
      QHash<QXmlName, XsdAttributeUse::Ptr> attributeUseHash;
      {
         const XsdAttributeUse::List attributeUses = complexType->attributeUses();
         for (int i = 0; i < attributeUses.count(); ++i) {
            attributeUseHash.insert(attributeUses.at(i)->attribute()->name(m_namePool), attributeUses.at(i));
         }
      }

      const QSet<QXmlName> attributes(attributeNames());

      // 3
      QHashIterator<QXmlName, XsdAttributeUse::Ptr> usesIt(attributeUseHash);
      while (usesIt.hasNext()) {
         usesIt.next();

         if (usesIt.value()->isRequired()) {
            if (!attributes.contains(usesIt.key())) {
               error(QtXmlPatterns::tr("Element %1 is missing required attribute %2.").formatArg(formatKeyword(declaration->displayName(
                        m_namePool)))
                     .formatArg(formatKeyword(m_namePool->displayName(usesIt.key()))));
               return false;
            }
         }
      }

      bool hasIDAttribute = hasIDAttributeUse(complexType->attributeUses());

      // 2
      QSetIterator<QXmlName> it(attributes);
      while (it.hasNext()) {
         const QXmlName attributeName = it.next();

         // skip builtin attributes
         if (attributeName == m_xsiNilName ||
               attributeName == m_xsiTypeName ||
               attributeName == m_xsiSchemaLocationName ||
               attributeName == m_xsiNoNamespaceSchemaLocationName) {
            continue;
         }

         // 2.1
         if (attributeUseHash.contains(attributeName) &&
               (attributeUseHash.value(attributeName)->useType() != XsdAttributeUse::ProhibitedUse)) {
            if (!validateAttribute(attributeUseHash.value(attributeName), attribute(attributeName))) {
               return false;
            }
         } else { // 2.2
            if (complexType->attributeWildcard()) {
               const XsdWildcard::Ptr wildcard(complexType->attributeWildcard());
               if (!validateAttributeWildcard(attributeName, wildcard)) {
                  error(QtXmlPatterns::tr("Attribute %1 does not match the attribute wildcard.").formatArg(formatKeyword(
                           m_namePool->displayName(attributeName))));
                  return false;
               }

               if (wildcard->processContents() != XsdWildcard::Skip) {
                  const XsdAttribute::Ptr attributeDeclaration = attributeByName(attributeName);

                  if (!attributeDeclaration) {
                     if (wildcard->processContents() == XsdWildcard::Strict) {
                        error(QtXmlPatterns::tr("Declaration for attribute %1 does not exist.").formatArg(formatKeyword(m_namePool->displayName(
                                 attributeName))));
                        return false;
                     }
                  } else {
                     if (BuiltinTypes::xsID->wxsTypeMatches(attributeDeclaration->type())) {
                        if (hasIDAttribute) {
                           error(QtXmlPatterns::tr("Element %1 contains two attributes of type %2.")
                                 .formatArg(formatKeyword(declaration->displayName(m_namePool)))
                                 .formatArg(formatKeyword("ID")));
                           return false;
                        }

                        hasIDAttribute = true;
                     }

                     if (!validateAttribute(attributeDeclaration, attribute(attributeName))) {
                        if (wildcard->processContents() == XsdWildcard::Strict) {
                           error(QtXmlPatterns::tr("Attribute %1 contains invalid content.").formatArg(formatKeyword(m_namePool->displayName(
                                    attributeName))));
                           return false;
                        }
                     }
                  }
               }
            } else {
               error(QtXmlPatterns::tr("Element %1 contains unknown attribute %2.").formatArg(formatKeyword(declaration->displayName(
                        m_namePool)))
                     .formatArg(formatKeyword(m_namePool->displayName(attributeName))));
               return false;
            }
         }
      }
   }

   // 4
   // so what?...

   // 5
   // hmm...

   // 6
   // TODO: check assertions

   // 7
   // TODO: check type table restrictions

   // rememeber the type of that element node
   m_model->setAssignedType(item().toNodeModelIndex(), type);

   return true;
}

bool XsdValidatingInstanceReader::validateAttribute(const XsdAttributeUse::Ptr &declaration, const QString &value)
{
   const AnySimpleType::Ptr attributeType = declaration->attribute()->type();
   const XsdFacet::Hash facets = XsdTypeChecker::mergedFacetsForType(attributeType, m_context);

   const QString actualValue = XsdTypeChecker::normalizedValue(value, facets);

   QString errorMsg;
   AnySimpleType::Ptr boundType;

   const QXmlNodeModelIndex index = attributeItem(declaration->attribute()->name(m_namePool)).toNodeModelIndex();

   const XsdTypeChecker checker(m_context, namespaceBindings(index), sourceLocation());
   if (!checker.isValidString(actualValue, attributeType, errorMsg, &boundType)) {
      error(QtXmlPatterns::tr("Content of attribute %1 does not match its type definition: %2.").formatArg(formatKeyword(
               declaration->attribute()->displayName(m_namePool))).formatArg(errorMsg));
      return false;
   }

   // @see http://www.w3.org/TR/xmlschema11-1/#cvc-au
   if (declaration->valueConstraint() &&
         declaration->valueConstraint()->variety() == XsdAttributeUse::ValueConstraint::Fixed) {
      const QString actualConstraintValue = XsdTypeChecker::normalizedValue(declaration->valueConstraint()->value(), facets);
      if (!checker.valuesAreEqual(actualValue, actualConstraintValue, attributeType)) {
         error(QtXmlPatterns::tr("Content of attribute %1 does not match defined value constraint.").formatArg(formatKeyword(
                  declaration->attribute()->displayName(m_namePool))));
         return false;
      }
   }

   if (BuiltinTypes::xsID->wxsTypeMatches(declaration->attribute()->type())) {
      addIdIdRefBinding(actualValue, declaration->attribute());
   }

   if (m_idRefsType->wxsTypeMatches(declaration->attribute()->type())) {
      const QStringList idRefs = actualValue.split(QLatin1Char(' '), QStringParser::SkipEmptyParts);
      for (int i = 0; i < idRefs.count(); ++i) {
         m_idRefs.insert(idRefs.at(i));
      }
   } else if (BuiltinTypes::xsIDREF->wxsTypeMatches(declaration->attribute()->type())) {
      m_idRefs.insert(actualValue);
   }

   m_model->setAssignedType(index, declaration->attribute()->type());
   m_model->setAssignedAttribute(index, declaration->attribute());

   return true;
}

//TODO: merge that with the method above
bool XsdValidatingInstanceReader::validateAttribute(const XsdAttribute::Ptr &declaration, const QString &value)
{
   const AnySimpleType::Ptr attributeType = declaration->type();
   const XsdFacet::Hash facets = XsdTypeChecker::mergedFacetsForType(attributeType, m_context);

   const QString actualValue = XsdTypeChecker::normalizedValue(value, facets);

   QString errorMsg;
   AnySimpleType::Ptr boundType;

   const QXmlNodeModelIndex index = attributeItem(declaration->name(m_namePool)).toNodeModelIndex();

   const XsdTypeChecker checker(m_context, namespaceBindings(index), sourceLocation());
   if (!checker.isValidString(actualValue, attributeType, errorMsg, &boundType)) {
      error(QtXmlPatterns::tr("Content of attribute %1 does not match its type definition: %2.").formatArg(formatKeyword(
               declaration->displayName(m_namePool))).formatArg(errorMsg));
      return false;
   }

   // @see http://www.w3.org/TR/xmlschema11-1/#cvc-au
   if (declaration->valueConstraint() &&
         declaration->valueConstraint()->variety() == XsdAttribute::ValueConstraint::Fixed) {
      const QString actualConstraintValue = XsdTypeChecker::normalizedValue(declaration->valueConstraint()->value(), facets);
      if (!checker.valuesAreEqual(actualValue, actualConstraintValue, attributeType)) {
         error(QtXmlPatterns::tr("Content of attribute %1 does not match defined value constraint.").formatArg(formatKeyword(
                  declaration->displayName(m_namePool))));
         return false;
      }
   }

   if (BuiltinTypes::xsID->wxsTypeMatches(declaration->type())) {
      addIdIdRefBinding(actualValue, declaration);
   }

   if (m_idRefsType->wxsTypeMatches(declaration->type())) {
      const QStringList idRefs = actualValue.split(QLatin1Char(' '), QStringParser::SkipEmptyParts);
      for (int i = 0; i < idRefs.count(); ++i) {
         m_idRefs.insert(idRefs.at(i));
      }
   } else if (BuiltinTypes::xsIDREF->wxsTypeMatches(declaration->type())) {
      m_idRefs.insert(actualValue);
   }

   m_model->setAssignedType(index, declaration->type());
   m_model->setAssignedAttribute(index, declaration);

   return true;
}

bool XsdValidatingInstanceReader::validateAttributeWildcard(const QXmlName &attributeName,
      const XsdWildcard::Ptr &wildcard)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#cvc-wildcard

   // wildcards using XsdWildcard::absentNamespace, so we have to fix that here
   QXmlName name(attributeName);
   if (name.namespaceURI() == StandardNamespaces::empty) {
      name.setNamespaceURI(m_namePool->allocateNamespace(XsdWildcard::absentNamespace()));
   }

   return XsdSchemaHelper::wildcardAllowsExpandedName(name, wildcard, m_namePool);
}

bool XsdValidatingInstanceReader::validateIdentityConstraint(const XsdElement::Ptr &element,
      const QXmlItem &currentItem)
{
   const XsdIdentityConstraint::List constraints = element->identityConstraints();

   for (int i = 0; i < constraints.count(); ++i) {
      const XsdIdentityConstraint::Ptr constraint = constraints.at(i);

      TargetNode::Set targetNodeSet, qualifiedNodeSet;
      selectNodeSets(element, currentItem, constraint, targetNodeSet, qualifiedNodeSet);

      if (constraint->category() == XsdIdentityConstraint::Unique) {
         if (!validateUniqueIdentityConstraint(element, constraint, qualifiedNodeSet)) {
            return false;
         }
      } else if (constraint->category() == XsdIdentityConstraint::Key) {
         if (!validateKeyIdentityConstraint(element, constraint, targetNodeSet, qualifiedNodeSet)) {
            return false;
         }
      }
   }

   // we do the keyref check in a separated run to make sure that all keys are available
   for (int i = 0; i < constraints.count(); ++i) {
      const XsdIdentityConstraint::Ptr constraint = constraints.at(i);
      if (constraint->category() == XsdIdentityConstraint::KeyReference) {
         TargetNode::Set targetNodeSet, qualifiedNodeSet;
         selectNodeSets(element, currentItem, constraint, targetNodeSet, qualifiedNodeSet);

         if (!validateKeyRefIdentityConstraint(element, constraint, qualifiedNodeSet)) {
            return false;
         }
      }
   }

   return true;
}

bool XsdValidatingInstanceReader::validateUniqueIdentityConstraint(const XsdElement::Ptr &,
      const XsdIdentityConstraint::Ptr &constraint, const TargetNode::Set &qualifiedNodeSet)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#d0e32243

   // 4.1
   const XsdSchemaSourceLocationReflection reflection(sourceLocation());

   QSetIterator<TargetNode> it(qualifiedNodeSet);
   while (it.hasNext()) {
      const TargetNode node = it.next();
      QSetIterator<TargetNode> innerIt(qualifiedNodeSet);
      while (innerIt.hasNext()) {
         const TargetNode innerNode = innerIt.next();

         if (node == innerNode) { // do not compare with ourself
            continue;
         }

         if (node.fieldsAreEqual(innerNode, m_namePool, m_context, &reflection)) {
            error(QtXmlPatterns::tr("Non-unique value found for constraint %1.").formatArg(formatKeyword(constraint->displayName(
                     m_namePool))));
            return false;
         }
      }
   }

   m_idcKeys.insert(constraint->name(m_namePool), qualifiedNodeSet);

   return true;
}

bool XsdValidatingInstanceReader::validateKeyIdentityConstraint(const XsdElement::Ptr &element,
      const XsdIdentityConstraint::Ptr &constraint, const TargetNode::Set &targetNodeSet,
      const TargetNode::Set &qualifiedNodeSet)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#d0e32243

   // 4.2
   const XsdSchemaSourceLocationReflection reflection(sourceLocation());

   // 4.2.1
   if (targetNodeSet.count() != qualifiedNodeSet.count()) {
      error(QtXmlPatterns::tr("Key constraint %1 contains absent fields.").formatArg(formatKeyword(constraint->displayName(
               m_namePool))));
      return false;
   }

   // 4.2.2
   if (!validateUniqueIdentityConstraint(element, constraint, qualifiedNodeSet)) {
      return false;
   }

   // 4.2.3
   QSetIterator<TargetNode> it(qualifiedNodeSet);
   while (it.hasNext()) {
      const TargetNode node = it.next();
      const QVector<QXmlItem> fieldItems = node.fieldItems();
      for (int i = 0; i < fieldItems.count(); ++i) {
         const QXmlNodeModelIndex index = fieldItems.at(i).toNodeModelIndex();
         if (m_model->kind(index) == QXmlNodeModelIndex::Element) {
            const XsdElement::Ptr declaration = m_model->assignedElement(index);
            if (declaration && declaration->isNillable()) {
               error(QtXmlPatterns::tr("Key constraint %1 contains references nillable element %2.")
                     .formatArg(formatKeyword(constraint->displayName(m_namePool)))
                     .formatArg(formatKeyword(declaration->displayName(m_namePool))));
               return false;
            }
         }
      }
   }

   m_idcKeys.insert(constraint->name(m_namePool), qualifiedNodeSet);

   return true;
}

bool XsdValidatingInstanceReader::validateKeyRefIdentityConstraint(const XsdElement::Ptr &,
      const XsdIdentityConstraint::Ptr &constraint, const TargetNode::Set &qualifiedNodeSet)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#d0e32243

   // 4.3
   const XsdSchemaSourceLocationReflection reflection(sourceLocation());

   const TargetNode::Set keySet = m_idcKeys.value(constraint->referencedKey()->name(m_namePool));

   QSetIterator<TargetNode> it(qualifiedNodeSet);
   while (it.hasNext()) {
      const TargetNode node = it.next();

      bool foundMatching = false;

      QSetIterator<TargetNode> keyIt(keySet);
      while (keyIt.hasNext()) {
         const TargetNode keyNode = keyIt.next();

         if (node.fieldsAreEqual(keyNode, m_namePool, m_context, &reflection)) {
            foundMatching = true;
            break;
         }
      }

      if (!foundMatching) {
         error(QtXmlPatterns::tr("No referenced value found for key reference %1.").formatArg(formatKeyword(constraint->displayName(
                  m_namePool))));
         return false;
      }
   }

   return true;
}

QXmlQuery XsdValidatingInstanceReader::createXQuery(const QList<QXmlName> &namespaceBindings,
      const QXmlItem &contextNode, const QString &queryString) const
{
   // create a public name pool from our name pool
   QXmlNamePool namePool(m_namePool.data());

   // the QXmlQuery shall work with the same name pool as we do
   QXmlQuery query(namePool);

   // add additional namespace bindings
   QXmlQueryPrivate *queryPrivate = query.d;

   for (int i = 0; i < namespaceBindings.count(); ++i) {
      if (namespaceBindings.at(i).prefix() != StandardPrefixes::empty) {
         queryPrivate->addAdditionalNamespaceBinding(namespaceBindings.at(i));
      }
   }

   // set the context node for that query and the query string
   query.setFocus(contextNode);
   query.setQuery(queryString, m_documentUri);

   return query;
}

bool XsdValidatingInstanceReader::selectNodeSets(const XsdElement::Ptr &, const QXmlItem &currentItem,
      const XsdIdentityConstraint::Ptr &constraint, TargetNode::Set &targetNodeSet, TargetNode::Set &qualifiedNodeSet)
{
   // at first select all target nodes
   const XsdXPathExpression::Ptr selector = constraint->selector();
   const XsdXPathExpression::List fields = constraint->fields();

   QXmlQuery query = createXQuery(selector->namespaceBindings(), currentItem, selector->expression());

   QXmlResultItems resultItems;
   query.evaluateTo(&resultItems);

   // now we iterate over all target nodes and select the fields for each node
   QXmlItem item(resultItems.next());

   while (!item.isNull()) {

      TargetNode targetNode(item);

      for (int i = 0; i < fields.count(); ++i) {
         const XsdXPathExpression::Ptr field = fields.at(i);
         QXmlQuery fieldQuery = createXQuery(field->namespaceBindings(), item, field->expression());

         QXmlResultItems fieldResultItems;
         fieldQuery.evaluateTo(&fieldResultItems);

         // copy result into vetor for better testing...
         QVector<QXmlItem> fieldVector;
         QXmlItem fieldItem(fieldResultItems.next());
         while (!fieldItem.isNull()) {
            fieldVector.append(fieldItem);
            fieldItem = fieldResultItems.next();
         }

         if (fieldVector.count() > 1) {
            error(QtXmlPatterns::tr("More than one value found for field %1.").formatArg(formatData(field->expression())));
            return false;
         }

         if (fieldVector.count() == 1) {
            fieldItem = fieldVector.first();

            const QXmlNodeModelIndex index = fieldItem.toNodeModelIndex();
            const SchemaType::Ptr type = m_model->assignedType(index);

            bool typeOk = true;
            if (type->isComplexType()) {
               if (type->isDefinedBySchema()) {
                  if (XsdComplexType::Ptr(type)->contentType()->variety() != XsdComplexType::ContentType::Simple) {
                     typeOk = false;
                  }
               } else {
                  typeOk = false;
               }
            }
            if (!typeOk) {
               error(QtXmlPatterns::tr("Field %1 has no simple type.").formatArg(formatData(field->expression())));
               return false;
            }

            SchemaType::Ptr targetType = type;
            QString value = m_model->stringValue(fieldItem.toNodeModelIndex());

            if (type->isDefinedBySchema()) {
               if (type->isSimpleType()) {
                  targetType = XsdSimpleType::Ptr(type)->primitiveType();
               } else {
                  targetType = XsdComplexType::Ptr(type)->contentType()->simpleType();
               }
            } else {
               if (BuiltinTypes::xsAnySimpleType->name(m_namePool) == type->name(m_namePool)) {
                  targetType = BuiltinTypes::xsString;
                  value = QLatin1String("___anySimpleType_value");
               }
            }

            // if it is xs:QName derived type, we normalize the name content
            // and do a string comparison
            if (BuiltinTypes::xsQName->wxsTypeMatches(type)) {
               targetType = BuiltinTypes::xsString;

               const QXmlName qName = convertToQName(value.trimmed());
               value = QString::fromLatin1("%1:%2").formatArg(m_namePool->stringForNamespace(qName.namespaceURI())).formatArg(
                          m_namePool->stringForLocalName(qName.localName()));
            }

            targetNode.addField(fieldItem, value, targetType);
         } else {
            // we add an empty entry here, that makes comparison easier later on
            targetNode.addField(QXmlItem(), QString(), SchemaType::Ptr());
         }
      }

      targetNodeSet.insert(targetNode);

      item = resultItems.next();
   }

   // copy all items from target node set to qualified node set, that have no empty fields
   QSetIterator<TargetNode> it(targetNodeSet);
   while (it.hasNext()) {
      const TargetNode node = it.next();
      if (node.emptyFieldsCount() == 0) {
         qualifiedNodeSet.insert(node);
      }
   }

   return true;
}

XsdElement::Ptr XsdValidatingInstanceReader::elementByName(const QXmlName &name) const
{
   return m_schema->element(name);
}

XsdAttribute::Ptr XsdValidatingInstanceReader::attributeByName(const QXmlName &name) const
{
   return m_schema->attribute(name);
}

SchemaType::Ptr XsdValidatingInstanceReader::typeByName(const QXmlName &name) const
{
   const SchemaType::Ptr type = m_schema->type(name);
   if (type) {
      return type;
   }

   return m_context->schemaTypeFactory()->createSchemaType(name);
}

void XsdValidatingInstanceReader::addIdIdRefBinding(const QString &id, const NamedSchemaComponent::Ptr &binding)
{
   if (!m_model->idIdRefBindings(id).isEmpty()) {
      error(QtXmlPatterns::tr("ID value '%1' is not unique.").formatArg(formatKeyword(id)));
      return;
   }

   m_model->addIdIdRefBinding(id, binding);
}

QString XsdValidatingInstanceReader::qNameAttribute(const QXmlName &attributeName)
{
   const QString value = attribute(attributeName).simplified();
   if (!XPathHelper::isQName(value)) {
      error(QtXmlPatterns::tr("'%1' attribute contains invalid QName content: %2.").formatArg(m_namePool->displayName(
               attributeName)).formatArg(formatData(value)));
      return QString();
   } else {
      return value;
   }
}

XsdComplexType::Ptr XsdValidatingInstanceReader::anyType()
{
   if (m_anyType) {
      return m_anyType;
   }

   const XsdWildcard::Ptr wildcard(new XsdWildcard());
   wildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Any);
   wildcard->setProcessContents(XsdWildcard::Lax);

   const XsdParticle::Ptr outerParticle(new XsdParticle());
   outerParticle->setMinimumOccurs(1);
   outerParticle->setMaximumOccurs(1);

   const XsdParticle::Ptr innerParticle(new XsdParticle());
   innerParticle->setMinimumOccurs(0);
   innerParticle->setMaximumOccursUnbounded(true);
   innerParticle->setTerm(wildcard);

   const XsdModelGroup::Ptr outerModelGroup(new XsdModelGroup());
   outerModelGroup->setCompositor(XsdModelGroup::SequenceCompositor);
   outerModelGroup->setParticles(XsdParticle::List() << innerParticle);
   outerParticle->setTerm(outerModelGroup);

   m_anyType = XsdComplexType::Ptr(new XsdComplexType());
   m_anyType->setName(BuiltinTypes::xsAnyType->name(m_namePool));
   m_anyType->setDerivationMethod(XsdComplexType::DerivationRestriction);
   m_anyType->contentType()->setVariety(XsdComplexType::ContentType::Mixed);
   m_anyType->contentType()->setParticle(outerParticle);
   m_anyType->setAttributeWildcard(wildcard);
   m_anyType->setIsAbstract(false);

   return m_anyType;
}

