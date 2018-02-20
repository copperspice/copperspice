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

#include "qxsdschemaparser_p.h"
#include "qxmlutils_p.h"
#include "qacceltreeresourceloader_p.h"
#include "qboolean_p.h"
#include "qcommonnamespaces_p.h"
#include "qderivedinteger_p.h"
#include "qderivedstring_p.h"
#include "qqnamevalue_p.h"
#include "qxmlquery_p.h"
#include "qxpathhelper_p.h"
#include "qxsdattributereference_p.h"
#include "qxsdreference_p.h"
#include "qxsdschematoken_p.h"
#include <QFile>
#include <QXmlQuery>

QT_BEGIN_NAMESPACE

/**
 * @page schema_overview Overview
 * @section structure_and_components Structure and Components
 *
 * The schema validator code consists of 4 major components
 *
 * <dl>
 *  <dt>The schema parser (QPatternist::XsdSchemaParser)</dt>
 *  <dd>This component parses a XML document that is supplied via a QIODevice. It creates
 *      a so called (incomplete) 'compiled schema', which is a representation of the XML Schema
 *      structure as C++ objects.
 *      As the parser is a streaming parser, it can't resolve references to types or elements/attributes
 *      in place, therefor it creates resolver tasks which are passed to the schema resolver component
 *      for resolving at a later point in time.
 *      The parser does furthermore the basic XML structure constraint checking, e.g. if all required
 *      attributes are available or the order of the elements is correct.</dd>
 *
 *  <dt>The schema resolver (QPatternist::XsdSchemaResolver)</dt>
 *  <dd>This component is activated after the schema parser component has been finished the parsing
 *      of all schemas. The resolver has been supplied with resolve tasks by the schema parser that
 *      it will resolve in this step now. Between working on the single resolver tasks, the resolver
 *      calls check methods from the schema checker component to make sure that some assertions are
 *      valid (e.g. no circular inheritance of types), so that the resolver can work without hassle.
 *      During resoving references to attribute or element groups it also checks for circular references
 *      of these groups.
 *      At the end of that phase we have a compiled schema that is fully resolved (not necessarily valid though).</dd>
 *
 *  <dt>The schema checker (QPatternist::XsdSchemaChecker)</dt>
 *  <dd>This component does all the schema constraint checking as given by the Schema specification.
 *      At the end of that phase we have fully resolved and valid compiled schema that can be used for validation
 *      of instance documents.</dd>
 *
 *  <dt>The validator (QPatternist::XsdValidatingInstanceReader)</dt>
 *  <dd>This component is responsible for validating a XML instance document, provided via a QIODevice, against
 *      a valid compiled schema.</dd>
 * </dl>
 *
 * @ingroup Patternist_schema
 */

using namespace QPatternist;

namespace QPatternist {

/**
 * @short A helper class for automatically handling namespace scopes of elements.
 *
 * This class should be instantiated at the beginning of each parse XYZ method.
 */
class ElementNamespaceHandler
{
 public:
   /**
    * Creates a new element namespace handler object.
    *
    * It checks whether the @p parser is on the right @p tag and it creates a new namespace
    * context that contains the inherited and local namespace declarations.
    */
   ElementNamespaceHandler(const XsdSchemaToken::NodeName &tag, XsdSchemaParser *parser)
      : m_parser(parser) {
      Q_ASSERT(m_parser->isStartElement() && (XsdSchemaToken::toToken(m_parser->name()) == tag) &&
               (XsdSchemaToken::toToken(m_parser->namespaceUri()) == XsdSchemaToken::XML_NS_SCHEMA_URI));
      Q_UNUSED(tag)
      m_parser->m_namespaceSupport.pushContext();
      m_parser->m_namespaceSupport.setPrefixes(m_parser->namespaceDeclarations());
   }

   /**
    * Destroys the element namespace handler object.
    *
    * It destroys the local namespace context.
    */
   ~ElementNamespaceHandler() {
      m_parser->m_namespaceSupport.popContext();
   }

 private:
   XsdSchemaParser *m_parser;
};

/**
 * A helper class that checks for the right occurrence of
 * xml tags with the help of a DFA.
 */
class TagValidationHandler
{
 public:
   TagValidationHandler(XsdTagScope::Type tag, XsdSchemaParser *parser, const NamePool::Ptr &namePool)
      : m_parser(parser), m_machine(namePool) {
      Q_ASSERT(m_parser->m_stateMachines.contains(tag));

      m_machine = m_parser->m_stateMachines.value(tag);
      m_machine.reset();
   }

   void validate(XsdSchemaToken::NodeName token) {
      if (token == XsdSchemaToken::NoKeyword) {
         const QList<XsdSchemaToken::NodeName> tokens = m_machine.possibleTransitions();

         QStringList elementNames;
         for (int i = 0; i < tokens.count(); ++i) {
            elementNames.append(formatElement(XsdSchemaToken::toString(tokens.at(i))));
         }

         m_parser->error(QtXmlPatterns::tr("Can not process unknown element %1, expected elements are: %2.")
                         .arg(formatElement(m_parser->name().toString()))
                         .arg(elementNames.join(QLatin1String(", "))));
         return;
      }

      if (!m_machine.proceed(token)) {
         const QList<XsdSchemaToken::NodeName> tokens = m_machine.possibleTransitions();

         QStringList elementNames;
         for (int i = 0; i < tokens.count(); ++i) {
            elementNames.append(formatElement(XsdSchemaToken::toString(tokens.at(i))));
         }

         m_parser->error(QtXmlPatterns::tr("Element %1 is not allowed in this scope, possible elements are: %2.")
                         .arg(formatElement(XsdSchemaToken::toString(token)))
                         .arg(elementNames.join(QLatin1String(", "))));
         return;
      }
   }

   void finalize() const {
      if (!m_machine.inEndState()) {
         const QList<XsdSchemaToken::NodeName> tokens = m_machine.possibleTransitions();

         QStringList elementNames;
         for (int i = 0; i < tokens.count(); ++i) {
            elementNames.append(formatElement(XsdSchemaToken::toString(tokens.at(i))));
         }

         m_parser->error(QtXmlPatterns::tr("Child element is missing in that scope, possible child elements are: %1.")
                         .arg(elementNames.join(QLatin1String(", "))));
      }
   }

 private:
   XsdSchemaParser *m_parser;
   XsdStateMachine<XsdSchemaToken::NodeName> m_machine;
};

}

/**
 * Returns a list of all particles with group references that appear at any level of
 * the given unresolved @p group.
 */
static XsdParticle::List collectGroupRef(const XsdModelGroup::Ptr &group)
{
   XsdParticle::List refParticles;

   XsdParticle::List particles = group->particles();
   for (int i = 0; i < particles.count(); ++i) {
      if (particles.at(i)->term()->isReference()) {
         const XsdReference::Ptr reference(particles.at(i)->term());
         if (reference->type() == XsdReference::ModelGroup) {
            refParticles.append(particles.at(i));
         }
      }
      if (particles.at(i)->term()->isModelGroup()) {
         refParticles << collectGroupRef(XsdModelGroup::Ptr(particles.at(i)->term()));
      }
   }

   return refParticles;
}

/**
 * Helper function that works around the limited facilities of
 * QUrl/AnyURI::fromLexical to detect invalid URIs
 */
inline static bool isValidUri(const QString &string)
{
   // an empty URI points to the current document as defined in RFC 2396 (4.2)
   if (string.isEmpty()) {
      return true;
   }

   // explicit check as that is not checked by the code below
   if (string.startsWith(QLatin1String("##"))) {
      return false;
   }

   const AnyURI::Ptr uri = AnyURI::fromLexical(string);
   return (!(uri->hasError()));
}

XsdSchemaParser::XsdSchemaParser(const XsdSchemaContext::Ptr &context, const XsdSchemaParserContext::Ptr &parserContext,
                                 QIODevice *device)
   : MaintainingReader<XsdSchemaToken, XsdTagScope::Type>(parserContext->elementDescriptions(),
         QSet<XsdSchemaToken::NodeName>(), context, device)
   , m_context(context.data())
   , m_parserContext(parserContext.data())
   , m_namePool(m_parserContext->namePool().data())
   , m_namespaceSupport(*m_namePool)
{
   m_schema = m_parserContext->schema().data();
   m_schemaResolver = m_parserContext->resolver().data();
   m_idCache = XsdIdCache::Ptr(new XsdIdCache());

   setupStateMachines();
   setupBuiltinTypeNames();
}

void XsdSchemaParser::addIncludedSchemas(const NamespaceSet &schemas)
{
   m_includedSchemas += schemas;
}

void XsdSchemaParser::setIncludedSchemas(const NamespaceSet &schemas)
{
   m_includedSchemas = schemas;
}

void XsdSchemaParser::addImportedSchemas(const NamespaceSet &schemas)
{
   m_importedSchemas += schemas;
}

void XsdSchemaParser::setImportedSchemas(const NamespaceSet &schemas)
{
   m_importedSchemas = schemas;
}

void XsdSchemaParser::addRedefinedSchemas(const NamespaceSet &schemas)
{
   m_redefinedSchemas += schemas;
}

void XsdSchemaParser::setRedefinedSchemas(const NamespaceSet &schemas)
{
   m_redefinedSchemas = schemas;
}

void XsdSchemaParser::setTargetNamespace(const QString &targetNamespace)
{
   m_targetNamespace = targetNamespace;
}

void XsdSchemaParser::setTargetNamespaceExtended(const QString &targetNamespace)
{
   m_targetNamespace = targetNamespace;
   m_namespaceSupport.setTargetNamespace(m_namePool->allocateNamespace(m_targetNamespace));
}

void XsdSchemaParser::setDocumentURI(const QUrl &uri)
{
   m_documentURI = uri;

   // prevent to get included/imported/redefined twice
   m_includedSchemas.insert(uri);
   m_importedSchemas.insert(uri);
   m_redefinedSchemas.insert(uri);
}

QUrl XsdSchemaParser::documentURI() const
{
   return m_documentURI;
}

bool XsdSchemaParser::isAnyAttributeAllowed() const
{
   return false;
}

bool XsdSchemaParser::parse(ParserType parserType)
{
   m_componentLocationHash.clear();

   while (!atEnd()) {
      readNext();

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         if (isSchemaTag(XsdSchemaToken::Schema, token, namespaceToken)) {
            parseSchema(parserType);
         } else {
            error(QtXmlPatterns::tr("Document is not a XML schema."));
         }
      }
   }

   m_schemaResolver->addComponentLocationHash(m_componentLocationHash);
   m_schemaResolver->setDefaultOpenContent(m_defaultOpenContent, m_defaultOpenContentAppliesToEmpty);

   if (QXmlStreamReader::error() != QXmlStreamReader::NoError) {
      error(errorString());
   }

   return true;
}

void XsdSchemaParser::error(const QString &msg)
{
   MaintainingReader<XsdSchemaToken, XsdTagScope::Type>::error(msg, XsdSchemaContext::XSDError);
}

void XsdSchemaParser::attributeContentError(const char *attributeName, const char *elementName, const QString &value,
      const SchemaType::Ptr &type)
{
   if (type) {
      error(QtXmlPatterns::tr("%1 attribute of %2 element contains invalid content: {%3} is not a value of type %4.")
            .arg(formatAttribute(attributeName))
            .arg(formatElement(elementName))
            .arg(formatData(value))
            .arg(formatType(NamePool::Ptr(m_namePool), type)));
   } else {
      error(QtXmlPatterns::tr("%1 attribute of %2 element contains invalid content: {%3}.")
            .arg(formatAttribute(attributeName))
            .arg(formatElement(elementName))
            .arg(formatData(value)));
   }
}

void XsdSchemaParser::parseSchema(ParserType parserType)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Schema, this);

   validateElement(XsdTagScope::Schema);

   // parse attributes

   if (parserType == TopLevelParser) {
      if (hasAttribute(QString::fromLatin1("targetNamespace"))) {
         m_targetNamespace = readNamespaceAttribute(QString::fromLatin1("targetNamespace"), "schema");
      }
   } else if (parserType == IncludeParser) {
      // m_targetNamespace is set to the target namespace of the including schema at this point

      if (hasAttribute(QString::fromLatin1("targetNamespace"))) {
         const QString targetNamespace = readNamespaceAttribute(QString::fromLatin1("targetNamespace"), "schema");

         if (m_targetNamespace != targetNamespace) {
            error(QtXmlPatterns::tr("Target namespace %1 of included schema is different from the target namespace %2 as defined by the including schema.")
                  .arg(formatURI(targetNamespace)).arg(formatURI(m_targetNamespace)));
            return;
         }
      }
   } else if (parserType == ImportParser) {
      // m_targetNamespace is set to the target namespace from the namespace attribute of the <import> tag at this point

      QString targetNamespace;
      if (hasAttribute(QString::fromLatin1("targetNamespace"))) {
         targetNamespace = readNamespaceAttribute(QString::fromLatin1("targetNamespace"), "schema");
      }

      if (m_targetNamespace != targetNamespace) {
         error(QtXmlPatterns::tr("Target namespace %1 of imported schema is different from the target namespace %2 as defined by the importing schema.")
               .arg(formatURI(targetNamespace)).arg(formatURI(m_targetNamespace)));
         return;
      }
   } else if (parserType == RedefineParser) {
      // m_targetNamespace is set to the target namespace of the redefining schema at this point

      if (hasAttribute(QString::fromLatin1("targetNamespace"))) {
         const QString targetNamespace = readNamespaceAttribute(QString::fromLatin1("targetNamespace"), "schema");

         if (m_targetNamespace != targetNamespace) {
            error(QtXmlPatterns::tr("Target namespace %1 of imported schema is different from the target namespace %2 as defined by the importing schema.")
                  .arg(formatURI(targetNamespace)).arg(formatURI(m_targetNamespace)));
            return;
         }
      }
   }

   if (hasAttribute(QString::fromLatin1("attributeFormDefault"))) {
      const QString value = readAttribute(QString::fromLatin1("attributeFormDefault"));
      if (value != QString::fromLatin1("qualified") && value != QString::fromLatin1("unqualified")) {
         attributeContentError("attributeFormDefault", "schema", value);
         return;
      }

      m_attributeFormDefault = value;
   } else {
      m_attributeFormDefault = QString::fromLatin1("unqualified");
   }

   if (hasAttribute(QString::fromLatin1("elementFormDefault"))) {
      const QString value = readAttribute(QString::fromLatin1("elementFormDefault"));
      if (value != QString::fromLatin1("qualified") && value != QString::fromLatin1("unqualified")) {
         attributeContentError("elementFormDefault", "schema", value);
         return;
      }

      m_elementFormDefault = value;
   } else {
      m_elementFormDefault = QString::fromLatin1("unqualified");
   }

   if (hasAttribute(QString::fromLatin1("blockDefault"))) {
      const QString blockDefault = readAttribute(QString::fromLatin1("blockDefault"));
      const QStringList blockDefaultList = blockDefault.split(QLatin1Char(' '), QString::SkipEmptyParts);
      for (int i = 0; i < blockDefaultList.count(); ++i) {
         const QString value = blockDefaultList.at(i);
         if (value != QString::fromLatin1("#all") &&
               value != QString::fromLatin1("extension") &&
               value != QString::fromLatin1("restriction") &&
               value != QString::fromLatin1("substitution")) {
            attributeContentError("blockDefault", "schema", value);
            return;
         }
      }

      m_blockDefault = blockDefault;
   }

   if (hasAttribute(QString::fromLatin1("finalDefault"))) {
      const QString finalDefault = readAttribute(QString::fromLatin1("finalDefault"));
      const QStringList finalDefaultList = finalDefault.split(QLatin1Char(' '), QString::SkipEmptyParts);
      for (int i = 0; i < finalDefaultList.count(); ++i) {
         const QString value = finalDefaultList.at(i);
         if (value != QString::fromLatin1("#all") &&
               value != QString::fromLatin1("extension") &&
               value != QString::fromLatin1("restriction") &&
               value != QString::fromLatin1("list") &&
               value != QString::fromLatin1("union")) {
            attributeContentError("finalDefault", "schema", value);
            return;
         }
      }

      m_finalDefault = finalDefault;
   }

   if (hasAttribute(QString::fromLatin1("xpathDefaultNamespace"))) {
      const QString xpathDefaultNamespace = readAttribute(QString::fromLatin1("xpathDefaultNamespace"));
      if (xpathDefaultNamespace != QString::fromLatin1("##defaultNamespace") &&
            xpathDefaultNamespace != QString::fromLatin1("##targetNamespace") &&
            xpathDefaultNamespace != QString::fromLatin1("##local")) {
         if (!isValidUri(xpathDefaultNamespace)) {
            attributeContentError("xpathDefaultNamespace", "schema", xpathDefaultNamespace);
            return;
         }
      }
      m_xpathDefaultNamespace = xpathDefaultNamespace;
   } else {
      m_xpathDefaultNamespace = QString::fromLatin1("##local");
   }

   if (hasAttribute(QString::fromLatin1("defaultAttributes"))) {
      const QString attrGroupName = readQNameAttribute(QString::fromLatin1("defaultAttributes"), "schema");
      convertName(attrGroupName, NamespaceSupport::ElementName,
                  m_defaultAttributes); // translate qualified name into QXmlName
   }

   if (hasAttribute(QString::fromLatin1("version"))) {
      const QString version = readAttribute(QString::fromLatin1("version"));
   }

   if (hasAttribute(CommonNamespaces::XML, QString::fromLatin1("lang"))) {
      const QString value = readAttribute(QString::fromLatin1("lang"), CommonNamespaces::XML);

      const QRegExp exp(QString::fromLatin1("[a-zA-Z]{1,8}(-[a-zA-Z0-9]{1,8})*"));
      if (!exp.exactMatch(value)) {
         attributeContentError("xml:lang", "schema", value);
         return;
      }
   }

   validateIdAttribute("schema");

   TagValidationHandler tagValidator(XsdTagScope::Schema, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Include, token, namespaceToken)) {
            parseInclude();
         } else if (isSchemaTag(XsdSchemaToken::Import, token, namespaceToken)) {
            parseImport();
         } else if (isSchemaTag(XsdSchemaToken::Redefine, token, namespaceToken)) {
            parseRedefine();
         } else if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            m_schema->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::DefaultOpenContent, token, namespaceToken)) {
            parseDefaultOpenContent();
         } else if (isSchemaTag(XsdSchemaToken::SimpleType, token, namespaceToken)) {
            const XsdSimpleType::Ptr type = parseGlobalSimpleType();
            addType(type);
         } else if (isSchemaTag(XsdSchemaToken::ComplexType, token, namespaceToken)) {
            const XsdComplexType::Ptr type = parseGlobalComplexType();
            addType(type);
         } else if (isSchemaTag(XsdSchemaToken::Group, token, namespaceToken)) {
            const XsdModelGroup::Ptr group = parseNamedGroup();
            addElementGroup(group);
         } else if (isSchemaTag(XsdSchemaToken::AttributeGroup, token, namespaceToken)) {
            XsdAttributeGroup::Ptr attributeGroup = parseNamedAttributeGroup();
            addAttributeGroup(attributeGroup);
         } else if (isSchemaTag(XsdSchemaToken::Element, token, namespaceToken)) {
            const XsdElement::Ptr element = parseGlobalElement();
            addElement(element);
         } else if (isSchemaTag(XsdSchemaToken::Attribute, token, namespaceToken)) {
            const XsdAttribute::Ptr attribute = parseGlobalAttribute();
            addAttribute(attribute);
         } else if (isSchemaTag(XsdSchemaToken::Notation, token, namespaceToken)) {
            const XsdNotation::Ptr notation = parseNotation();
            addNotation(notation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   m_schema->setTargetNamespace(m_targetNamespace);
}

void XsdSchemaParser::parseInclude()
{
   Q_ASSERT(isStartElement() && XsdSchemaToken::toToken(name()) == XsdSchemaToken::Include &&
            XsdSchemaToken::toToken(namespaceUri()) == XsdSchemaToken::XML_NS_SCHEMA_URI);

   validateElement(XsdTagScope::Include);

   // parse attributes
   const QString schemaLocation = readAttribute(QString::fromLatin1("schemaLocation"));

   QUrl url(schemaLocation);
   if (url.isRelative()) {
      Q_ASSERT(m_documentURI.isValid());

      url = m_documentURI.resolved(url);
   }

   if (m_includedSchemas.contains(url)) {
      // we have included that file already, according to the schema spec we are
      // allowed to silently skip it.
   } else {
      m_includedSchemas.insert(url);

      const std::unique_ptr<QNetworkReply> reply(AccelTreeResourceLoader::load(url, m_context->networkAccessManager(),
                                         XsdSchemaContext::Ptr(m_context), AccelTreeResourceLoader::ContinueOnError));
      if (reply) {
         // parse the included schema by a different parser but with the same context
         XsdSchemaParser parser(XsdSchemaContext::Ptr(m_context), XsdSchemaParserContext::Ptr(m_parserContext), reply.get());
         parser.setDocumentURI(url);
         parser.setTargetNamespaceExtended(m_targetNamespace);
         parser.setIncludedSchemas(m_includedSchemas);
         parser.setImportedSchemas(m_importedSchemas);
         parser.setRedefinedSchemas(m_redefinedSchemas);
         if (!parser.parse(XsdSchemaParser::IncludeParser)) {
            return;
         } else {
            // add indirectly loaded schemas to the list of already loaded ones
            addIncludedSchemas(parser.m_includedSchemas);
            addImportedSchemas(parser.m_importedSchemas);
            addRedefinedSchemas(parser.m_redefinedSchemas);
         }
      }
   }

   validateIdAttribute("include");

   TagValidationHandler tagValidator(XsdTagScope::Include, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            m_schema->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();
}

void XsdSchemaParser::parseImport()
{
   Q_ASSERT(isStartElement() && XsdSchemaToken::toToken(name()) == XsdSchemaToken::Import &&
            XsdSchemaToken::toToken(namespaceUri()) == XsdSchemaToken::XML_NS_SCHEMA_URI);

   validateElement(XsdTagScope::Import);

   // parse attributes
   QString importNamespace;
   if (hasAttribute(QString::fromLatin1("namespace"))) {
      importNamespace = readAttribute(QString::fromLatin1("namespace"));
      if (importNamespace == m_targetNamespace) {
         error(QtXmlPatterns::tr("%1 element is not allowed to have the same %2 attribute value as the target namespace %3.")
               .arg(formatElement("import"))
               .arg(formatAttribute("namespace"))
               .arg(formatURI(m_targetNamespace)));
         return;
      }
   } else {
      if (m_targetNamespace.isEmpty()) {
         error(QtXmlPatterns::tr("%1 element without %2 attribute is not allowed inside schema without target namespace.")
               .arg(formatElement("import"))
               .arg(formatAttribute("namespace")));
         return;
      }
   }

   if (hasAttribute(QString::fromLatin1("schemaLocation"))) {
      const QString schemaLocation = readAttribute(QString::fromLatin1("schemaLocation"));

      QUrl url(schemaLocation);
      if (url.isRelative()) {
         Q_ASSERT(m_documentURI.isValid());

         url = m_documentURI.resolved(url);
      }

      if (m_importedSchemas.contains(url)) {
         // we have imported that file already, according to the schema spec we are
         // allowed to silently skip it.

      } else {
         m_importedSchemas.insert(url);

         // as it is possible that well known schemas (e.g. XSD for XML) are only referenced by
         // namespace we should add it as well
         m_importedSchemas.insert(QUrl(importNamespace));

         std::unique_ptr<QNetworkReply> reply(AccelTreeResourceLoader::load(url, m_context->networkAccessManager(),
                                      XsdSchemaContext::Ptr(m_context), AccelTreeResourceLoader::ContinueOnError));
         if (reply) {
            // parse the included schema by a different parser but with the same context
            XsdSchemaParser parser(XsdSchemaContext::Ptr(m_context),
                  XsdSchemaParserContext::Ptr(m_parserContext), reply.get());

            parser.setDocumentURI(url);
            parser.setTargetNamespace(importNamespace);
            parser.setIncludedSchemas(m_includedSchemas);
            parser.setImportedSchemas(m_importedSchemas);
            parser.setRedefinedSchemas(m_redefinedSchemas);

            if (!parser.parse(XsdSchemaParser::ImportParser)) {
               return;

            } else {
               // add indirectly loaded schemas to the list of already loaded ones
               addIncludedSchemas(parser.m_includedSchemas);
               addImportedSchemas(parser.m_importedSchemas);
               addRedefinedSchemas(parser.m_redefinedSchemas);
            }
         }
      }

   } else {
      // check whether it is a known namespace we have a builtin schema for
      if (! importNamespace.isEmpty()) {
         QUrl tmp(importNamespace);

         if (! m_importedSchemas.contains(tmp)) {
            m_importedSchemas.insert(tmp);

            QFile file(":" + importNamespace);

            if (file.open(QIODevice::ReadOnly)) {
               XsdSchemaParser parser(XsdSchemaContext::Ptr(m_context), XsdSchemaParserContext::Ptr(m_parserContext), &file);
               parser.setDocumentURI(tmp);
               parser.setTargetNamespace(importNamespace);
               parser.setIncludedSchemas(m_includedSchemas);
               parser.setImportedSchemas(m_importedSchemas);
               parser.setRedefinedSchemas(m_redefinedSchemas);

               if (!parser.parse(XsdSchemaParser::ImportParser)) {
                  return;

               } else {
                  // add indirectly loaded schemas to the list of already loaded ones
                  addIncludedSchemas(parser.m_includedSchemas);
                  addImportedSchemas(parser.m_importedSchemas);
                  addRedefinedSchemas(parser.m_redefinedSchemas);
               }
            }
         }

      } else {
         // we do not import anything... that is valid according to the schema
      }
   }

   validateIdAttribute("import");

   TagValidationHandler tagValidator(XsdTagScope::Import, this, NamePool::Ptr(m_namePool));

   while (! atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            m_schema->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();
}

void XsdSchemaParser::parseRedefine()
{
   Q_ASSERT(isStartElement() && XsdSchemaToken::toToken(name()) == XsdSchemaToken::Redefine &&
            XsdSchemaToken::toToken(namespaceUri()) == XsdSchemaToken::XML_NS_SCHEMA_URI);

   validateElement(XsdTagScope::Redefine);

   // parse attributes
   validateIdAttribute("redefine");

   const QString schemaLocation = readAttribute(QString::fromLatin1("schemaLocation"));

   TagValidationHandler tagValidator(XsdTagScope::Redefine, this, NamePool::Ptr(m_namePool));

   XsdSimpleType::List redefinedSimpleTypes;
   XsdComplexType::List redefinedComplexTypes;
   XsdModelGroup::List redefinedGroups;
   XsdAttributeGroup::List redefinedAttributeGroups;

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            m_schema->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::SimpleType, token, namespaceToken)) {
            const XsdSimpleType::Ptr type = parseGlobalSimpleType();
            redefinedSimpleTypes.append(type);

            const QXmlName baseTypeName = m_parserContext->resolver()->baseTypeNameOfType(type);
            if (baseTypeName != type->name(NamePool::Ptr(m_namePool))) {
               error(QString::fromLatin1("redefined simple type %1 must have itself as base type").arg(formatType(NamePool::Ptr(
                        m_namePool), type)));
               return;
            }
         } else if (isSchemaTag(XsdSchemaToken::ComplexType, token, namespaceToken)) {
            const XsdComplexType::Ptr type = parseGlobalComplexType();
            redefinedComplexTypes.append(type);

            // @see http://www.w3.org/TR/xmlschema11-1/#src-redefine

            // 5
            const QXmlName baseTypeName = m_parserContext->resolver()->baseTypeNameOfType(type);
            if (baseTypeName != type->name(NamePool::Ptr(m_namePool))) {
               error(QString::fromLatin1("redefined complex type %1 must have itself as base type").arg(formatType(NamePool::Ptr(
                        m_namePool), type)));
               return;
            }
         } else if (isSchemaTag(XsdSchemaToken::Group, token, namespaceToken)) {
            const XsdModelGroup::Ptr group = parseNamedGroup();
            redefinedGroups.append(group);
         } else if (isSchemaTag(XsdSchemaToken::AttributeGroup, token, namespaceToken)) {
            const XsdAttributeGroup::Ptr group = parseNamedAttributeGroup();
            redefinedAttributeGroups.append(group);

         } else {
            parseUnknown();
         }
      }
   }

   bool locationMustResolve = false;
   if (!redefinedSimpleTypes.isEmpty() || !redefinedComplexTypes.isEmpty() ||
         !redefinedGroups.isEmpty() || !redefinedAttributeGroups.isEmpty()) {
      locationMustResolve = true;
   }

   QUrl url(schemaLocation);
   if (url.isRelative()) {
      Q_ASSERT(m_documentURI.isValid());

      url = m_documentURI.resolved(url);
   }

   // we parse the schema given in the redefine tag into its own context
   const XsdSchemaParserContext::Ptr redefinedContext(new XsdSchemaParserContext(NamePool::Ptr(m_namePool),
         XsdSchemaContext::Ptr(m_context)));

   if (m_redefinedSchemas.contains(url)) {
      // we have redefined that file already, according to the schema spec we are
      // allowed to silently skip it.
   } else {
      m_redefinedSchemas.insert(url);
      QNetworkReply *reply = AccelTreeResourceLoader::load(url, m_context->networkAccessManager(),
                             XsdSchemaContext::Ptr(m_context),
                             (locationMustResolve ? AccelTreeResourceLoader::FailOnError : AccelTreeResourceLoader::ContinueOnError));
      if (reply) {
         // parse the included schema by a different parser but with the same context
         XsdSchemaParser parser(XsdSchemaContext::Ptr(m_context), redefinedContext, reply);
         parser.setDocumentURI(url);
         parser.setTargetNamespaceExtended(m_targetNamespace);
         parser.setIncludedSchemas(m_includedSchemas);
         parser.setImportedSchemas(m_importedSchemas);
         parser.setRedefinedSchemas(m_redefinedSchemas);
         if (!parser.parse(XsdSchemaParser::RedefineParser)) {
            return;
         } else {
            // add indirectly loaded schemas to the list of already loaded ones
            addIncludedSchemas(parser.m_includedSchemas);
            addImportedSchemas(parser.m_importedSchemas);
            addRedefinedSchemas(parser.m_redefinedSchemas);
         }

         delete reply;
      }
   }

   XsdSimpleType::List contextSimpleTypes = redefinedContext->schema()->simpleTypes();
   XsdComplexType::List contextComplexTypes = redefinedContext->schema()->complexTypes();
   XsdModelGroup::List contextGroups = redefinedContext->schema()->elementGroups();
   XsdAttributeGroup::List contextAttributeGroups = redefinedContext->schema()->attributeGroups();

   // now we do the actual redefinition:

   // iterate over all redefined simple types
   for (int i = 0; i < redefinedSimpleTypes.count(); ++i) {
      XsdSimpleType::Ptr redefinedType = redefinedSimpleTypes.at(i);

      //TODONEXT: validation

      // search the definition they override in the context types
      bool found = false;
      for (int j = 0; j < contextSimpleTypes.count(); ++j) {
         XsdSimpleType::Ptr contextType = contextSimpleTypes.at(j);

         if (redefinedType->name(NamePool::Ptr(m_namePool)) == contextType->name(NamePool::Ptr(
                  m_namePool))) { // we found the right type
            found = true;

            // 1) set name of context type to empty name
            contextType->setName(m_parserContext->createAnonymousName(QString()));

            // 2) set the context type as base type for the redefined type
            redefinedType->setWxsSuperType(contextType);

            // 3) remove the base type resolving job from the resolver as
            //    we have set the base type here explicitly
            m_parserContext->resolver()->removeSimpleRestrictionBase(redefinedType);

            // 4) add the redefined type to the schema
            addType(redefinedType);

            // 5) add the context type as anonymous type, so the resolver
            //    can resolve it further.
            addAnonymousType(contextType);

            // 6) remove the context type from the list
            contextSimpleTypes.removeAt(j);

            break;
         }
      }

      if (!found) {
         error(QString::fromLatin1("no matching type found to redefine simple type %1").arg(formatType(NamePool::Ptr(m_namePool),
               redefinedType)));
         return;
      }
   }

   // add all remaining context simple types to the schema
   for (int i = 0; i < contextSimpleTypes.count(); ++i) {
      addType(contextSimpleTypes.at(i));
   }

   // iterate over all redefined complex types
   for (int i = 0; i < redefinedComplexTypes.count(); ++i) {
      XsdComplexType::Ptr redefinedType = redefinedComplexTypes.at(i);

      //TODONEXT: validation

      // search the definition they override in the context types
      bool found = false;
      for (int j = 0; j < contextComplexTypes.count(); ++j) {
         XsdComplexType::Ptr contextType = contextComplexTypes.at(j);

         if (redefinedType->name(NamePool::Ptr(m_namePool)) == contextType->name(NamePool::Ptr(
                  m_namePool))) { // we found the right type
            found = true;

            // 1) set name of context type to empty name
            contextType->setName(m_parserContext->createAnonymousName(QString()));

            // 2) set the context type as base type for the redefined type
            redefinedType->setWxsSuperType(contextType);

            // 3) remove the base type resolving job from the resolver as
            //    we have set the base type here explicitly
            m_parserContext->resolver()->removeComplexBaseType(redefinedType);

            // 4) add the redefined type to the schema
            addType(redefinedType);

            // 5) add the context type as anonymous type, so the resolver
            //    can resolve its attribute uses etc.
            addAnonymousType(contextType);

            // 6) remove the context type from the list
            contextComplexTypes.removeAt(j);

            break;
         }
      }

      if (!found) {
         error(QString::fromLatin1("no matching type found to redefine complex type %1").arg(formatType(NamePool::Ptr(
                  m_namePool), redefinedType)));
         return;
      }
   }

   // iterate over all redefined element groups
   for (int i = 0; i < redefinedGroups.count(); ++i) {
      const XsdModelGroup::Ptr group(redefinedGroups.at(i));

      // @see http://www.w3.org/TR/xmlschema11-1/#src-redefine

      // 6
      const XsdParticle::List particles = collectGroupRef(group);
      XsdParticle::Ptr referencedParticle;
      int sameNameCounter = 0;
      for (int i = 0; i < particles.count(); ++i) {
         const XsdReference::Ptr ref(particles.at(i)->term());
         if (ref->referenceName() == group->name(NamePool::Ptr(m_namePool))) {
            referencedParticle = particles.at(i);

            if (referencedParticle->minimumOccurs() != 1 || referencedParticle->maximumOccurs() != 1 ||
                  referencedParticle->maximumOccursUnbounded()) { // 6.1.2
               error(QString::fromLatin1("redefined group %1 can not contain reference to itself with minOccurs or maxOccurs != 1").arg(
                        formatKeyword(group->displayName(NamePool::Ptr(m_namePool)))));
               return;
            }
            sameNameCounter++;
         }
      }

      // 6.1.1
      if (sameNameCounter > 1) {
         error(QString::fromLatin1("redefined group %1 can not contain multiple references to itself").arg(formatKeyword(
                  group->displayName(NamePool::Ptr(m_namePool)))));
         return;
      }

      // search the group definition in the included schema (S2)
      XsdModelGroup::Ptr contextGroup;
      for (int j = 0; j < contextGroups.count(); ++j) {
         if (group->name(NamePool::Ptr(m_namePool)) == contextGroups.at(j)->name(NamePool::Ptr(m_namePool))) {
            contextGroup = contextGroups.at(j);
            break;
         }
      }

      if (!contextGroup) { // 6.2.1
         error(QString::fromLatin1("redefined group %1 has no occurrence in included schema").arg(formatKeyword(
                  group->displayName(NamePool::Ptr(m_namePool)))));
         return;
      }

      if (sameNameCounter == 1) {
         // there was a self reference in the redefined group, so use the
         // group from the included schema

         // set a anonymous name to the group of the included schema
         contextGroup->setName(m_parserContext->createAnonymousName(m_namePool->stringForNamespace(contextGroup->name(
                                  NamePool::Ptr(m_namePool)).namespaceURI())));

         // replace the self-reference with the group from the included schema
         referencedParticle->setTerm(contextGroup);

         addElementGroup(group);

         addElementGroup(contextGroup);
         contextGroups.removeAll(contextGroup);
      } else {
         // there was no self reference in the redefined group

         // just add the redefined group...
         addElementGroup(group);

         // we have to add them, otherwise it is not resolved and we can't validate it later
         contextGroup->setName(m_parserContext->createAnonymousName(m_namePool->stringForNamespace(contextGroup->name(
                                  NamePool::Ptr(m_namePool)).namespaceURI())));
         addElementGroup(contextGroup);

         m_schemaResolver->addRedefinedGroups(group, contextGroup);

         // ...and forget about the group from the included schema
         contextGroups.removeAll(contextGroup);
      }
   }

   // iterate over all redefined attribute groups
   for (int i = 0; i < redefinedAttributeGroups.count(); ++i) {
      const XsdAttributeGroup::Ptr group(redefinedAttributeGroups.at(i));

      // @see http://www.w3.org/TR/xmlschema11-1/#src-redefine

      // 7

      // 7.1
      int sameNameCounter = 0;
      for (int j = 0; j < group->attributeUses().count(); ++j) {
         const XsdAttributeUse::Ptr attributeUse(group->attributeUses().at(j));
         if (attributeUse->isReference()) {
            const XsdAttributeReference::Ptr reference(attributeUse);
            if (reference->type() == XsdAttributeReference::AttributeGroup) {
               if (group->name(NamePool::Ptr(m_namePool)) == reference->referenceName()) {
                  sameNameCounter++;
               }
            }
         }
      }
      if (sameNameCounter > 1) {
         error(QString::fromLatin1("redefined attribute group %1 can not contain multiple references to itself").arg(
                  formatKeyword(group->displayName(NamePool::Ptr(m_namePool)))));
         return;
      }

      // search the attribute group definition in the included schema (S2)
      XsdAttributeGroup::Ptr baseGroup;
      for (int j = 0; j < contextAttributeGroups.count(); ++j) {
         const XsdAttributeGroup::Ptr contextGroup(contextAttributeGroups.at(j));
         if (group->name(NamePool::Ptr(m_namePool)) == contextGroup->name(NamePool::Ptr(m_namePool))) {
            baseGroup = contextGroup;
            break;
         }
      }

      if (!baseGroup) { // 7.2.1
         error(QString::fromLatin1("redefined attribute group %1 has no occurrence in included schema").arg(formatKeyword(
                  group->displayName(NamePool::Ptr(m_namePool)))));
         return;
      }

      if (sameNameCounter == 1) {

         // first set an anonymous name to the attribute group from the included
         // schema
         baseGroup->setName(m_parserContext->createAnonymousName(m_namePool->stringForNamespace(baseGroup->name(NamePool::Ptr(
                               m_namePool)).namespaceURI())));

         // iterate over the attribute uses of the redefined attribute group
         // and replace the self-reference with the attribute group from the
         // included schema
         for (int j = 0; j < group->attributeUses().count(); ++j) {
            const XsdAttributeUse::Ptr attributeUse(group->attributeUses().at(j));
            if (attributeUse->isReference()) {
               const XsdAttributeReference::Ptr reference(attributeUse);
               if (reference->type() == XsdAttributeReference::AttributeGroup) {
                  if (group->name(NamePool::Ptr(m_namePool)) == reference->referenceName()) {
                     reference->setReferenceName(baseGroup->name(NamePool::Ptr(m_namePool)));
                     break;
                  }
               }
            }
         }

         // add both groups to the target schema
         addAttributeGroup(baseGroup);
         addAttributeGroup(group);

         contextAttributeGroups.removeAll(baseGroup);
      }

      if (sameNameCounter == 0) { // 7.2

         // we have to add them, otherwise it is not resolved and we can't validate it later
         baseGroup->setName(m_parserContext->createAnonymousName(m_namePool->stringForNamespace(baseGroup->name(NamePool::Ptr(
                               m_namePool)).namespaceURI())));
         addAttributeGroup(baseGroup);

         m_schemaResolver->addRedefinedAttributeGroups(group, baseGroup);

         // just add the redefined attribute group to the target schema...
         addAttributeGroup(group);

         // ... and forget about the one from the included schema
         contextAttributeGroups.removeAll(baseGroup);
      }
   }

   // add all remaining context complex types to the schema
   for (int i = 0; i < contextComplexTypes.count(); ++i) {
      addType(contextComplexTypes.at(i));
   }

   // add all remaining context element groups to the schema
   for (int i = 0; i < contextGroups.count(); ++i) {
      addElementGroup(contextGroups.at(i));
   }

   // add all remaining context attribute groups to the schema
   for (int i = 0; i < contextAttributeGroups.count(); ++i) {
      addAttributeGroup(contextAttributeGroups.at(i));
   }

   // copy all elements, attributes and notations
   const XsdElement::List contextElements = redefinedContext->schema()->elements();
   for (int i = 0; i < contextElements.count(); ++i) {
      addElement(contextElements.at(i));
   }

   const XsdAttribute::List contextAttributes = redefinedContext->schema()->attributes();
   for (int i = 0; i < contextAttributes.count(); ++i) {
      addAttribute(contextAttributes.at(i));
   }

   const XsdNotation::List contextNotations = redefinedContext->schema()->notations();
   for (int i = 0; i < contextNotations.count(); ++i) {
      addNotation(contextNotations.at(i));
   }

   // push all data to resolve from the context resolver to our resolver
   redefinedContext->resolver()->copyDataTo(m_parserContext->resolver());

   tagValidator.finalize();
}

XsdAnnotation::Ptr XsdSchemaParser::parseAnnotation()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Annotation, this);

   validateElement(XsdTagScope::Annotation);

   // parse attributes
   validateIdAttribute("annotation");

   TagValidationHandler tagValidator(XsdTagScope::Annotation, this, NamePool::Ptr(m_namePool));

   const XsdAnnotation::Ptr annotation(new XsdAnnotation());

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Appinfo, token, namespaceToken)) {
            const XsdApplicationInformation::Ptr info = parseAppInfo();
            annotation->addApplicationInformation(info);
         } else if (isSchemaTag(XsdSchemaToken::Documentation, token, namespaceToken)) {
            const XsdDocumentation::Ptr documentation = parseDocumentation();
            annotation->addDocumentation(documentation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return annotation;
}

XsdApplicationInformation::Ptr XsdSchemaParser::parseAppInfo()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Appinfo, this);

   validateElement(XsdTagScope::AppInfo);

   const XsdApplicationInformation::Ptr info(new XsdApplicationInformation());

   // parse attributes
   if (hasAttribute(QString::fromLatin1("source"))) {
      const QString value = readAttribute(QString::fromLatin1("source"));

      if (!isValidUri(value)) {
         attributeContentError("source", "appinfo", value, BuiltinTypes::xsAnyURI);
         return info;
      }

      if (!value.isEmpty()) {
         const AnyURI::Ptr source = AnyURI::fromLexical(value);
         info->setSource(source);
      }
   }

   while (!atEnd()) { //EVAL: can be anything... what to do?
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         parseUnknownDocumentation();
      }
   }

   return info;
}

XsdDocumentation::Ptr XsdSchemaParser::parseDocumentation()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Documentation, this);

   validateElement(XsdTagScope::Documentation);

   const XsdDocumentation::Ptr documentation(new XsdDocumentation());

   // parse attributes
   if (hasAttribute(QString::fromLatin1("source"))) {
      const QString value = readAttribute(QString::fromLatin1("source"));

      if (!isValidUri(value)) {
         attributeContentError("source", "documentation", value, BuiltinTypes::xsAnyURI);
         return documentation;
      }

      if (!value.isEmpty()) {
         const AnyURI::Ptr source = AnyURI::fromLexical(value);
         documentation->setSource(source);
      }
   }

   if (hasAttribute(CommonNamespaces::XML, QString::fromLatin1("lang"))) {
      const QString value = readAttribute(QString::fromLatin1("lang"), CommonNamespaces::XML);

      const QRegExp exp(QString::fromLatin1("[a-zA-Z]{1,8}(-[a-zA-Z0-9]{1,8})*"));
      if (!exp.exactMatch(value)) {
         attributeContentError("xml:lang", "documentation", value);
         return documentation;
      }
   }

   while (!atEnd()) { //EVAL: can by any... what to do?
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         parseUnknownDocumentation();
      }
   }

   return documentation;
}

void XsdSchemaParser::parseDefaultOpenContent()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::DefaultOpenContent, this);

   validateElement(XsdTagScope::DefaultOpenContent);

   m_defaultOpenContent = XsdComplexType::OpenContent::Ptr(new XsdComplexType::OpenContent());

   if (hasAttribute(QString::fromLatin1("appliesToEmpty"))) {
      const QString value = readAttribute(QString::fromLatin1("appliesToEmpty"));
      const Boolean::Ptr appliesToEmpty = Boolean::fromLexical(value);
      if (appliesToEmpty->hasError()) {
         attributeContentError("appliesToEmpty", "defaultOpenContent", value, BuiltinTypes::xsBoolean);
         return;
      }

      m_defaultOpenContentAppliesToEmpty = appliesToEmpty->as<Boolean>()->value();
   } else {
      m_defaultOpenContentAppliesToEmpty = false;
   }

   if (hasAttribute(QString::fromLatin1("mode"))) {
      const QString mode = readAttribute(QString::fromLatin1("mode"));

      if (mode == QString::fromLatin1("interleave")) {
         m_defaultOpenContent->setMode(XsdComplexType::OpenContent::Interleave);
      } else if (mode == QString::fromLatin1("suffix")) {
         m_defaultOpenContent->setMode(XsdComplexType::OpenContent::Suffix);
      } else {
         attributeContentError("mode", "defaultOpenContent", mode);
         return;
      }
   } else {
      m_defaultOpenContent->setMode(XsdComplexType::OpenContent::Interleave);
   }

   validateIdAttribute("defaultOpenContent");

   TagValidationHandler tagValidator(XsdTagScope::DefaultOpenContent, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            m_defaultOpenContent->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Any, token, namespaceToken)) {
            const XsdParticle::Ptr particle;
            const XsdWildcard::Ptr wildcard = parseAny(particle);
            m_defaultOpenContent->setWildcard(wildcard);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();
}

XsdSimpleType::Ptr XsdSchemaParser::parseGlobalSimpleType()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::SimpleType, this);

   validateElement(XsdTagScope::GlobalSimpleType);

   const XsdSimpleType::Ptr simpleType(new XsdSimpleType());
   simpleType->setCategory(XsdSimpleType::SimpleTypeAtomic); // just to make sure it's not invalid

   // parse attributes
   const SchemaType::DerivationConstraints allowedConstraints(SchemaType::ExtensionConstraint |
         SchemaType::RestrictionConstraint | SchemaType::ListConstraint | SchemaType::UnionConstraint);
   simpleType->setDerivationConstraints(readDerivationConstraintAttribute(allowedConstraints, "simpleType"));

   const QXmlName objectName = m_namePool->allocateQName(m_targetNamespace, readNameAttribute("simpleType"));
   simpleType->setName(objectName);

   validateIdAttribute("simpleType");

   TagValidationHandler tagValidator(XsdTagScope::GlobalSimpleType, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            simpleType->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Restriction, token, namespaceToken)) {
            parseSimpleRestriction(simpleType);
         } else if (isSchemaTag(XsdSchemaToken::List, token, namespaceToken)) {
            parseList(simpleType);
         } else if (isSchemaTag(XsdSchemaToken::Union, token, namespaceToken)) {
            parseUnion(simpleType);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return simpleType;
}

XsdSimpleType::Ptr XsdSchemaParser::parseLocalSimpleType()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::SimpleType, this);

   validateElement(XsdTagScope::LocalSimpleType);

   const XsdSimpleType::Ptr simpleType(new XsdSimpleType());
   simpleType->setCategory(XsdSimpleType::SimpleTypeAtomic); // just to make sure it's not invalid
   simpleType->setName(m_parserContext->createAnonymousName(m_targetNamespace));

   validateIdAttribute("simpleType");

   TagValidationHandler tagValidator(XsdTagScope::LocalSimpleType, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            simpleType->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Restriction, token, namespaceToken)) {
            parseSimpleRestriction(simpleType);
         } else if (isSchemaTag(XsdSchemaToken::List, token, namespaceToken)) {
            parseList(simpleType);
         } else if (isSchemaTag(XsdSchemaToken::Union, token, namespaceToken)) {
            parseUnion(simpleType);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return simpleType;
}

void XsdSchemaParser::parseSimpleRestriction(const XsdSimpleType::Ptr &ptr)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Restriction, this);

   validateElement(XsdTagScope::SimpleRestriction);

   ptr->setDerivationMethod(XsdSimpleType::DerivationRestriction);

   // The base attribute and simpleType member are mutually exclusive,
   // so we keep track of that
   bool hasBaseAttribute = false;
   bool hasBaseTypeSpecified = false;

   QXmlName baseName;
   if (hasAttribute(QString::fromLatin1("base"))) {
      const QString base = readQNameAttribute(QString::fromLatin1("base"), "restriction");
      convertName(base, NamespaceSupport::ElementName, baseName); // translate qualified name into QXmlName
      m_schemaResolver->addSimpleRestrictionBase(ptr, baseName, currentSourceLocation()); // add to resolver

      hasBaseAttribute = true;
      hasBaseTypeSpecified = true;
   }
   validateIdAttribute("restriction");

   XsdFacet::Hash facets;
   QList<XsdFacet::Ptr> patternFacets;
   QList<XsdFacet::Ptr> enumerationFacets;
   QList<XsdFacet::Ptr> assertionFacets;

   TagValidationHandler tagValidator(XsdTagScope::SimpleRestriction, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            ptr->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::SimpleType, token, namespaceToken)) {
            if (hasBaseAttribute) {
               error(QtXmlPatterns::tr("%1 element is not allowed inside %2 element if %3 attribute is present.")
                     .arg(formatElement("simpleType"))
                     .arg(formatElement("restriction"))
                     .arg(formatAttribute("base")));
               return;
            }

            const XsdSimpleType::Ptr type = parseLocalSimpleType();
            type->setContext(ptr);
            ptr->setWxsSuperType(type);
            ptr->setCategory(type->category());
            hasBaseTypeSpecified = true;

            // add it to list of anonymous types as well
            addAnonymousType(type);
         } else if (isSchemaTag(XsdSchemaToken::MinExclusive, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseMinExclusiveFacet();
            addFacet(facet, facets, ptr);
         } else if (isSchemaTag(XsdSchemaToken::MinInclusive, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseMinInclusiveFacet();
            addFacet(facet, facets, ptr);
         } else if (isSchemaTag(XsdSchemaToken::MaxExclusive, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseMaxExclusiveFacet();
            addFacet(facet, facets, ptr);
         } else if (isSchemaTag(XsdSchemaToken::MaxInclusive, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseMaxInclusiveFacet();
            addFacet(facet, facets, ptr);
         } else if (isSchemaTag(XsdSchemaToken::TotalDigits, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseTotalDigitsFacet();
            addFacet(facet, facets, ptr);
         } else if (isSchemaTag(XsdSchemaToken::FractionDigits, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseFractionDigitsFacet();
            addFacet(facet, facets, ptr);
         } else if (isSchemaTag(XsdSchemaToken::Length, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseLengthFacet();
            addFacet(facet, facets, ptr);
         } else if (isSchemaTag(XsdSchemaToken::MinLength, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseMinLengthFacet();
            addFacet(facet, facets, ptr);
         } else if (isSchemaTag(XsdSchemaToken::MaxLength, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseMaxLengthFacet();
            addFacet(facet, facets, ptr);
         } else if (isSchemaTag(XsdSchemaToken::Enumeration, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseEnumerationFacet();
            enumerationFacets.append(facet);
         } else if (isSchemaTag(XsdSchemaToken::WhiteSpace, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseWhiteSpaceFacet();
            addFacet(facet, facets, ptr);
         } else if (isSchemaTag(XsdSchemaToken::Pattern, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parsePatternFacet();
            patternFacets.append(facet);
         } else if (isSchemaTag(XsdSchemaToken::Assertion, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseAssertionFacet();
            assertionFacets.append(facet);
         } else {
            parseUnknown();
         }
      }
   }

   if (!hasBaseTypeSpecified) {
      error(QtXmlPatterns::tr("%1 element has neither %2 attribute nor %3 child element.")
            .arg(formatElement("restriction"))
            .arg(formatAttribute("base"))
            .arg(formatElement("simpleType")));
      return;
   }

   // merge all pattern facets into one multi value facet
   if (!patternFacets.isEmpty()) {
      const XsdFacet::Ptr patternFacet(new XsdFacet());
      patternFacet->setType(XsdFacet::Pattern);

      AtomicValue::List multiValue;
      for (int i = 0; i < patternFacets.count(); ++i) {
         multiValue << patternFacets.at(i)->multiValue();
      }

      patternFacet->setMultiValue(multiValue);
      addFacet(patternFacet, facets, ptr);
   }

   // merge all enumeration facets into one multi value facet
   if (!enumerationFacets.isEmpty()) {
      const XsdFacet::Ptr enumerationFacet(new XsdFacet());
      enumerationFacet->setType(XsdFacet::Enumeration);

      AtomicValue::List multiValue;
      for (int i = 0; i < enumerationFacets.count(); ++i) {
         multiValue << enumerationFacets.at(i)->multiValue();
      }

      enumerationFacet->setMultiValue(multiValue);
      addFacet(enumerationFacet, facets, ptr);
   }

   // merge all assertion facets into one facet
   if (!assertionFacets.isEmpty()) {
      const XsdFacet::Ptr assertionFacet(new XsdFacet());
      assertionFacet->setType(XsdFacet::Assertion);

      XsdAssertion::List assertions;
      for (int i = 0; i < assertionFacets.count(); ++i) {
         assertions << assertionFacets.at(i)->assertions();
      }

      assertionFacet->setAssertions(assertions);
      addFacet(assertionFacet, facets, ptr);
   }

   ptr->setFacets(facets);

   tagValidator.finalize();
}

void XsdSchemaParser::parseList(const XsdSimpleType::Ptr &ptr)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::List, this);

   validateElement(XsdTagScope::List);

   ptr->setCategory(XsdSimpleType::SimpleTypeList);
   ptr->setDerivationMethod(XsdSimpleType::DerivationList);
   ptr->setWxsSuperType(BuiltinTypes::xsAnySimpleType);

   // The itemType attribute and simpleType member are mutually exclusive,
   // so we keep track of that
   bool hasItemTypeAttribute = false;
   bool hasItemTypeSpecified = false;

   if (hasAttribute(QString::fromLatin1("itemType"))) {
      const QString itemType = readQNameAttribute(QString::fromLatin1("itemType"), "list");
      QXmlName typeName;
      convertName(itemType, NamespaceSupport::ElementName, typeName); // translate qualified name into QXmlName
      m_schemaResolver->addSimpleListType(ptr, typeName, currentSourceLocation()); // add to resolver

      hasItemTypeAttribute = true;
      hasItemTypeSpecified = true;
   }

   validateIdAttribute("list");

   TagValidationHandler tagValidator(XsdTagScope::List, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            ptr->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::SimpleType, token, namespaceToken)) {
            if (hasItemTypeAttribute) {
               error(QtXmlPatterns::tr("%1 element is not allowed inside %2 element if %3 attribute is present.")
                     .arg(formatElement("simpleType"))
                     .arg(formatElement("list"))
                     .arg(formatAttribute("itemType")));
               return;
            }

            const XsdSimpleType::Ptr type = parseLocalSimpleType();
            type->setContext(ptr);
            ptr->setItemType(type);

            hasItemTypeSpecified = true;

            // add it to list of anonymous types as well
            addAnonymousType(type);
         } else {
            parseUnknown();
         }
      }
   }

   if (!hasItemTypeSpecified) {
      error(QtXmlPatterns::tr("%1 element has neither %2 attribute nor %3 child element.")
            .arg(formatElement("list"))
            .arg(formatAttribute("itemType"))
            .arg(formatElement("simpleType")));
      return;
   }

   tagValidator.finalize();

   // add the default white space facet that every simple type with list derivation has
   const XsdFacet::Ptr defaultFacet(new XsdFacet());
   defaultFacet->setType(XsdFacet::WhiteSpace);
   defaultFacet->setFixed(true);
   defaultFacet->setValue(DerivedString<TypeString>::fromLexical(NamePool::Ptr(m_namePool),
                          XsdSchemaToken::toString(XsdSchemaToken::Collapse)));
   XsdFacet::Hash facets;
   facets.insert(defaultFacet->type(), defaultFacet);
   ptr->setFacets(facets);
}

void XsdSchemaParser::parseUnion(const XsdSimpleType::Ptr &ptr)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Union, this);

   validateElement(XsdTagScope::Union);

   ptr->setCategory(XsdSimpleType::SimpleTypeUnion);
   ptr->setDerivationMethod(XsdSimpleType::DerivationUnion);
   ptr->setWxsSuperType(BuiltinTypes::xsAnySimpleType);

   // The memberTypes attribute is not allowed to be empty,
   // so we keep track of that
   bool hasMemberTypesSpecified = false;

   if (hasAttribute(QString::fromLatin1("memberTypes"))) {
      const QStringList memberTypes = readAttribute(QString::fromLatin1("memberTypes")).split(QLatin1Char(' '),
                                      QString::SkipEmptyParts);
      QList<QXmlName> typeNames;

      for (int i = 0; i < memberTypes.count(); ++i) {
         QXmlName typeName;
         convertName(memberTypes.at(i), NamespaceSupport::ElementName, typeName); // translate qualified name into QXmlName
         typeNames.append(typeName);
      }

      if (!typeNames.isEmpty()) {
         m_schemaResolver->addSimpleUnionTypes(ptr, typeNames, currentSourceLocation()); // add to resolver
         hasMemberTypesSpecified = true;
      }
   }

   validateIdAttribute("union");

   AnySimpleType::List memberTypes;

   TagValidationHandler tagValidator(XsdTagScope::Union, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            ptr->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::SimpleType, token, namespaceToken)) {
            const XsdSimpleType::Ptr type = parseLocalSimpleType();
            type->setContext(ptr);
            memberTypes.append(type);

            // add it to list of anonymous types as well
            addAnonymousType(type);
         } else {
            parseUnknown();
         }
      }
   }

   if (!memberTypes.isEmpty()) {
      ptr->setMemberTypes(memberTypes);
      hasMemberTypesSpecified = true;
   }

   if (!hasMemberTypesSpecified) {
      error(QtXmlPatterns::tr("%1 element has neither %2 attribute nor %3 child element.")
            .arg(formatElement("union"))
            .arg(formatAttribute("memberTypes"))
            .arg(formatElement("simpleType")));
      return;
   }

   tagValidator.finalize();
}

XsdFacet::Ptr XsdSchemaParser::parseMinExclusiveFacet()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::MinExclusive, this);

   validateElement(XsdTagScope::MinExclusiveFacet);

   const XsdFacet::Ptr facet = XsdFacet::Ptr(new XsdFacet());
   facet->setType(XsdFacet::MinimumExclusive);

   // parse attributes
   if (hasAttribute(QString::fromLatin1("fixed"))) {
      const QString value = readAttribute(QString::fromLatin1("fixed"));
      const Boolean::Ptr fixed = Boolean::fromLexical(value);
      if (fixed->hasError()) {
         attributeContentError("fixed", "minExclusive", value, BuiltinTypes::xsBoolean);
         return facet;
      }

      facet->setFixed(fixed->as<Boolean>()->value());
   } else {
      facet->setFixed(false); // the default value
   }

   // as minExclusive can have a value of type anySimpleType, we just read
   // the string here and store it for later intepretation
   const QString value = readAttribute(QString::fromLatin1("value"));
   DerivedString<TypeString>::Ptr string = DerivedString<TypeString>::fromLexical(NamePool::Ptr(m_namePool), value);
   if (string->hasError()) {
      attributeContentError("value", "minExclusive", value, BuiltinTypes::xsAnySimpleType);
      return facet;
   } else {
      facet->setValue(string);
   }

   validateIdAttribute("minExclusive");

   TagValidationHandler tagValidator(XsdTagScope::MinExclusiveFacet, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            facet->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return facet;
}

XsdFacet::Ptr XsdSchemaParser::parseMinInclusiveFacet()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::MinInclusive, this);

   validateElement(XsdTagScope::MinInclusiveFacet);

   const XsdFacet::Ptr facet = XsdFacet::Ptr(new XsdFacet());
   facet->setType(XsdFacet::MinimumInclusive);

   // parse attributes
   if (hasAttribute(QString::fromLatin1("fixed"))) {
      const QString value = readAttribute(QString::fromLatin1("fixed"));
      const Boolean::Ptr fixed = Boolean::fromLexical(value);
      if (fixed->hasError()) {
         attributeContentError("fixed", "minInclusive", value, BuiltinTypes::xsBoolean);
         return facet;
      }

      facet->setFixed(fixed->as<Boolean>()->value());
   } else {
      facet->setFixed(false); // the default value
   }

   // as minInclusive can have a value of type anySimpleType, we just read
   // the string here and store it for later intepretation
   const QString value = readAttribute(QString::fromLatin1("value"));
   DerivedString<TypeString>::Ptr string = DerivedString<TypeString>::fromLexical(NamePool::Ptr(m_namePool), value);
   if (string->hasError()) {
      attributeContentError("value", "minInclusive", value, BuiltinTypes::xsAnySimpleType);
      return facet;
   } else {
      facet->setValue(string);
   }

   validateIdAttribute("minInclusive");

   TagValidationHandler tagValidator(XsdTagScope::MinInclusiveFacet, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            facet->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return facet;
}

XsdFacet::Ptr XsdSchemaParser::parseMaxExclusiveFacet()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::MaxExclusive, this);

   validateElement(XsdTagScope::MaxExclusiveFacet);

   const XsdFacet::Ptr facet = XsdFacet::Ptr(new XsdFacet());
   facet->setType(XsdFacet::MaximumExclusive);

   // parse attributes
   if (hasAttribute(QString::fromLatin1("fixed"))) {
      const QString value = readAttribute(QString::fromLatin1("fixed"));
      const Boolean::Ptr fixed = Boolean::fromLexical(value);
      if (fixed->hasError()) {
         attributeContentError("fixed", "maxExclusive", value, BuiltinTypes::xsBoolean);
         return facet;
      }

      facet->setFixed(fixed->as<Boolean>()->value());
   } else {
      facet->setFixed(false); // the default value
   }

   // as maxExclusive can have a value of type anySimpleType, we just read
   // the string here and store it for later intepretation
   const QString value = readAttribute(QString::fromLatin1("value"));
   DerivedString<TypeString>::Ptr string = DerivedString<TypeString>::fromLexical(NamePool::Ptr(m_namePool), value);
   if (string->hasError()) {
      attributeContentError("value", "maxExclusive", value, BuiltinTypes::xsAnySimpleType);
      return facet;
   } else {
      facet->setValue(string);
   }

   validateIdAttribute("maxExclusive");

   TagValidationHandler tagValidator(XsdTagScope::MaxExclusiveFacet, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            facet->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return facet;
}

XsdFacet::Ptr XsdSchemaParser::parseMaxInclusiveFacet()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::MaxInclusive, this);

   validateElement(XsdTagScope::MaxInclusiveFacet);

   const XsdFacet::Ptr facet = XsdFacet::Ptr(new XsdFacet());
   facet->setType(XsdFacet::MaximumInclusive);

   // parse attributes
   if (hasAttribute(QString::fromLatin1("fixed"))) {
      const QString value = readAttribute(QString::fromLatin1("fixed"));
      const Boolean::Ptr fixed = Boolean::fromLexical(value);
      if (fixed->hasError()) {
         attributeContentError("fixed", "maxInclusive", value, BuiltinTypes::xsBoolean);
         return facet;
      }

      facet->setFixed(fixed->as<Boolean>()->value());
   } else {
      facet->setFixed(false); // the default value
   }

   // as maxInclusive can have a value of type anySimpleType, we just read
   // the string here and store it for later intepretation
   const QString value = readAttribute(QString::fromLatin1("value"));
   DerivedString<TypeString>::Ptr string = DerivedString<TypeString>::fromLexical(NamePool::Ptr(m_namePool), value);
   if (string->hasError()) {
      attributeContentError("value", "maxInclusive", value, BuiltinTypes::xsAnySimpleType);
      return facet;
   } else {
      facet->setValue(string);
   }

   validateIdAttribute("maxInclusive");

   TagValidationHandler tagValidator(XsdTagScope::MaxInclusiveFacet, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            facet->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return facet;
}

XsdFacet::Ptr XsdSchemaParser::parseTotalDigitsFacet()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::TotalDigits, this);

   validateElement(XsdTagScope::TotalDigitsFacet);

   const XsdFacet::Ptr facet = XsdFacet::Ptr(new XsdFacet());
   facet->setType(XsdFacet::TotalDigits);

   // parse attributes
   if (hasAttribute(QString::fromLatin1("fixed"))) {
      const QString value = readAttribute(QString::fromLatin1("fixed"));
      const Boolean::Ptr fixed = Boolean::fromLexical(value);
      if (fixed->hasError()) {
         attributeContentError("fixed", "totalDigits", value, BuiltinTypes::xsBoolean);
         return facet;
      }

      facet->setFixed(fixed->as<Boolean>()->value());
   } else {
      facet->setFixed(false); // the default value
   }

   const QString value = readAttribute(QString::fromLatin1("value"));
   DerivedInteger<TypePositiveInteger>::Ptr integer = DerivedInteger<TypePositiveInteger>::fromLexical(NamePool::Ptr(
            m_namePool), value);
   if (integer->hasError()) {
      attributeContentError("value", "totalDigits", value, BuiltinTypes::xsPositiveInteger);
      return facet;
   } else {
      facet->setValue(integer);
   }

   validateIdAttribute("totalDigits");

   TagValidationHandler tagValidator(XsdTagScope::TotalDigitsFacet, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            facet->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return facet;
}

XsdFacet::Ptr XsdSchemaParser::parseFractionDigitsFacet()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::FractionDigits, this);

   validateElement(XsdTagScope::FractionDigitsFacet);

   const XsdFacet::Ptr facet = XsdFacet::Ptr(new XsdFacet());
   facet->setType(XsdFacet::FractionDigits);

   // parse attributes
   if (hasAttribute(QString::fromLatin1("fixed"))) {
      const QString value = readAttribute(QString::fromLatin1("fixed"));
      const Boolean::Ptr fixed = Boolean::fromLexical(value);
      if (fixed->hasError()) {
         attributeContentError("fixed", "fractionDigits", value, BuiltinTypes::xsBoolean);
         return facet;
      }

      facet->setFixed(fixed->as<Boolean>()->value());
   } else {
      facet->setFixed(false); // the default value
   }

   const QString value = readAttribute(QString::fromLatin1("value"));
   DerivedInteger<TypeNonNegativeInteger>::Ptr integer = DerivedInteger<TypeNonNegativeInteger>::fromLexical(NamePool::Ptr(
            m_namePool), value);
   if (integer->hasError()) {
      attributeContentError("value", "fractionDigits", value, BuiltinTypes::xsNonNegativeInteger);
      return facet;
   } else {
      facet->setValue(integer);
   }

   validateIdAttribute("fractionDigits");

   TagValidationHandler tagValidator(XsdTagScope::FractionDigitsFacet, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            facet->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return facet;
}

XsdFacet::Ptr XsdSchemaParser::parseLengthFacet()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Length, this);

   validateElement(XsdTagScope::LengthFacet);

   const XsdFacet::Ptr facet = XsdFacet::Ptr(new XsdFacet());
   facet->setType(XsdFacet::Length);

   // parse attributes
   if (hasAttribute(QString::fromLatin1("fixed"))) {
      const QString value = readAttribute(QString::fromLatin1("fixed"));
      const Boolean::Ptr fixed = Boolean::fromLexical(value);
      if (fixed->hasError()) {
         attributeContentError("fixed", "length", value, BuiltinTypes::xsBoolean);
         return facet;
      }

      facet->setFixed(fixed->as<Boolean>()->value());
   } else {
      facet->setFixed(false); // the default value
   }

   const QString value = readAttribute(QString::fromLatin1("value"));
   DerivedInteger<TypeNonNegativeInteger>::Ptr integer = DerivedInteger<TypeNonNegativeInteger>::fromLexical(NamePool::Ptr(
            m_namePool), value);
   if (integer->hasError()) {
      attributeContentError("value", "length", value, BuiltinTypes::xsNonNegativeInteger);
      return facet;
   } else {
      facet->setValue(integer);
   }

   validateIdAttribute("length");

   TagValidationHandler tagValidator(XsdTagScope::LengthFacet, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            facet->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return facet;
}

XsdFacet::Ptr XsdSchemaParser::parseMinLengthFacet()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::MinLength, this);

   validateElement(XsdTagScope::MinLengthFacet);

   const XsdFacet::Ptr facet = XsdFacet::Ptr(new XsdFacet());
   facet->setType(XsdFacet::MinimumLength);

   // parse attributes
   if (hasAttribute(QString::fromLatin1("fixed"))) {
      const QString value = readAttribute(QString::fromLatin1("fixed"));
      const Boolean::Ptr fixed = Boolean::fromLexical(value);
      if (fixed->hasError()) {
         attributeContentError("fixed", "minLength", value, BuiltinTypes::xsBoolean);
         return facet;
      }

      facet->setFixed(fixed->as<Boolean>()->value());
   } else {
      facet->setFixed(false); // the default value
   }

   const QString value = readAttribute(QString::fromLatin1("value"));
   DerivedInteger<TypeNonNegativeInteger>::Ptr integer = DerivedInteger<TypeNonNegativeInteger>::fromLexical(NamePool::Ptr(
            m_namePool), value);
   if (integer->hasError()) {
      attributeContentError("value", "minLength", value, BuiltinTypes::xsNonNegativeInteger);
      return facet;
   } else {
      facet->setValue(integer);
   }

   validateIdAttribute("minLength");

   TagValidationHandler tagValidator(XsdTagScope::MinLengthFacet, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            facet->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return facet;
}

XsdFacet::Ptr XsdSchemaParser::parseMaxLengthFacet()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::MaxLength, this);

   validateElement(XsdTagScope::MaxLengthFacet);

   const XsdFacet::Ptr facet = XsdFacet::Ptr(new XsdFacet());
   facet->setType(XsdFacet::MaximumLength);

   // parse attributes
   if (hasAttribute(QString::fromLatin1("fixed"))) {
      const QString value = readAttribute(QString::fromLatin1("fixed"));
      const Boolean::Ptr fixed = Boolean::fromLexical(value);
      if (fixed->hasError()) {
         attributeContentError("fixed", "maxLength", value, BuiltinTypes::xsBoolean);
         return facet;
      }

      facet->setFixed(fixed->as<Boolean>()->value());
   } else {
      facet->setFixed(false); // the default value
   }

   const QString value = readAttribute(QString::fromLatin1("value"));
   DerivedInteger<TypeNonNegativeInteger>::Ptr integer = DerivedInteger<TypeNonNegativeInteger>::fromLexical(NamePool::Ptr(
            m_namePool), value);
   if (integer->hasError()) {
      attributeContentError("value", "maxLength", value, BuiltinTypes::xsNonNegativeInteger);
      return facet;
   } else {
      facet->setValue(integer);
   }

   validateIdAttribute("maxLength");

   TagValidationHandler tagValidator(XsdTagScope::MaxLengthFacet, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            facet->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return facet;
}

XsdFacet::Ptr XsdSchemaParser::parseEnumerationFacet()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Enumeration, this);

   validateElement(XsdTagScope::EnumerationFacet);

   const XsdFacet::Ptr facet = XsdFacet::Ptr(new XsdFacet());
   facet->setType(XsdFacet::Enumeration);

   // parse attributes
   facet->setFixed(false); // not defined in schema, but can't hurt

   const QString value = readAttribute(QString::fromLatin1("value"));

   // as enumeration can have a value of type anySimpleType, we just read
   // the string here and store it for later intepretation
   DerivedString<TypeString>::Ptr string = DerivedString<TypeString>::fromLexical(NamePool::Ptr(m_namePool), value);
   if (string->hasError()) {
      attributeContentError("value", "enumeration", value);
      return facet;
   } else {
      AtomicValue::List multiValue;
      multiValue << string;
      facet->setMultiValue(multiValue);
   }
   m_schemaResolver->addEnumerationFacetValue(string, m_namespaceSupport);

   validateIdAttribute("enumeration");

   TagValidationHandler tagValidator(XsdTagScope::EnumerationFacet, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            facet->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return facet;
}

XsdFacet::Ptr XsdSchemaParser::parseWhiteSpaceFacet()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::WhiteSpace, this);

   validateElement(XsdTagScope::WhiteSpaceFacet);

   const XsdFacet::Ptr facet = XsdFacet::Ptr(new XsdFacet());
   facet->setType(XsdFacet::WhiteSpace);

   // parse attributes
   if (hasAttribute(QString::fromLatin1("fixed"))) {
      const QString value = readAttribute(QString::fromLatin1("fixed"));
      const Boolean::Ptr fixed = Boolean::fromLexical(value);
      if (fixed->hasError()) {
         attributeContentError("fixed", "whiteSpace", value, BuiltinTypes::xsBoolean);
         return facet;
      }

      facet->setFixed(fixed->as<Boolean>()->value());
   } else {
      facet->setFixed(false); // the default value
   }

   const QString value = readAttribute(QString::fromLatin1("value"));
   if (value != XsdSchemaToken::toString(XsdSchemaToken::Collapse) &&
         value != XsdSchemaToken::toString(XsdSchemaToken::Preserve) &&
         value != XsdSchemaToken::toString(XsdSchemaToken::Replace)) {
      attributeContentError("value", "whiteSpace", value);
      return facet;
   } else {
      DerivedString<TypeString>::Ptr string = DerivedString<TypeString>::fromLexical(NamePool::Ptr(m_namePool), value);
      if (string->hasError()) {
         attributeContentError("value", "whiteSpace", value);
         return facet;
      } else {
         facet->setValue(string);
      }
   }

   validateIdAttribute("whiteSpace");

   TagValidationHandler tagValidator(XsdTagScope::WhiteSpaceFacet, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            facet->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return facet;
}

XsdFacet::Ptr XsdSchemaParser::parsePatternFacet()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Pattern, this);

   validateElement(XsdTagScope::PatternFacet);

   const XsdFacet::Ptr facet = XsdFacet::Ptr(new XsdFacet());
   facet->setType(XsdFacet::Pattern);

   // parse attributes

   // as pattern can have a value of type anySimpleType, we just read
   // the string here and store it for later intepretation
   const QString value = readAttribute(QString::fromLatin1("value"));
   DerivedString<TypeString>::Ptr string = DerivedString<TypeString>::fromLexical(NamePool::Ptr(m_namePool), value);
   if (string->hasError()) {
      attributeContentError("value", "pattern", value);
      return facet;
   } else {
      AtomicValue::List multiValue;
      multiValue << string;
      facet->setMultiValue(multiValue);
   }

   validateIdAttribute("pattern");

   TagValidationHandler tagValidator(XsdTagScope::PatternFacet, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            facet->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return facet;
}

XsdFacet::Ptr XsdSchemaParser::parseAssertionFacet()
{
   // this is just a wrapper function around the parseAssertion() method

   const XsdAssertion::Ptr assertion = parseAssertion(XsdSchemaToken::Assertion, XsdTagScope::Assertion);

   const XsdFacet::Ptr facet = XsdFacet::Ptr(new XsdFacet());
   facet->setType(XsdFacet::Assertion);
   facet->setAssertions(XsdAssertion::List() << assertion);

   return facet;
}

XsdComplexType::Ptr XsdSchemaParser::parseGlobalComplexType()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::ComplexType, this);

   validateElement(XsdTagScope::GlobalComplexType);

   bool hasTypeSpecified = false;
   bool hasComplexContent = false;

   const XsdComplexType::Ptr complexType(new XsdComplexType());

   // parse attributes
   if (hasAttribute(QString::fromLatin1("abstract"))) {
      const QString abstract = readAttribute(QString::fromLatin1("abstract"));

      const Boolean::Ptr value = Boolean::fromLexical(abstract);
      if (value->hasError()) {
         attributeContentError("abstract", "complexType", abstract, BuiltinTypes::xsBoolean);
         return complexType;
      }

      complexType->setIsAbstract(value->as<Boolean>()->value());
   } else {
      complexType->setIsAbstract(false);  // default value
   }

   complexType->setProhibitedSubstitutions(readBlockingConstraintAttribute(NamedSchemaComponent::ExtensionConstraint |
                                           NamedSchemaComponent::RestrictionConstraint, "complexType"));
   complexType->setDerivationConstraints(readDerivationConstraintAttribute(SchemaType::ExtensionConstraint |
                                         SchemaType::RestrictionConstraint, "complexType"));

   const QXmlName objectName = m_namePool->allocateQName(m_targetNamespace, readNameAttribute("complexType"));
   complexType->setName(objectName);

   bool effectiveMixed = false;
   if (hasAttribute(QString::fromLatin1("mixed"))) {
      const QString mixed = readAttribute(QString::fromLatin1("mixed"));

      const Boolean::Ptr value = Boolean::fromLexical(mixed);
      if (value->hasError()) {
         attributeContentError("mixed", "complexType", mixed, BuiltinTypes::xsBoolean);
         return complexType;
      }

      effectiveMixed = value->as<Boolean>()->value();
   }

   validateIdAttribute("complexType");

   TagValidationHandler tagValidator(XsdTagScope::GlobalComplexType, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            complexType->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::SimpleContent, token, namespaceToken)) {
            if (effectiveMixed) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("complexType"))
                     .arg(formatElement("simpleContent"))
                     .arg(formatAttribute("mixed")));
               return complexType;
            }

            parseSimpleContent(complexType);
            hasTypeSpecified = true;
         } else if (isSchemaTag(XsdSchemaToken::ComplexContent, token, namespaceToken)) {
            bool mixed;
            parseComplexContent(complexType, &mixed);
            hasTypeSpecified = true;

            effectiveMixed = (effectiveMixed || mixed);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::OpenContent, token, namespaceToken)) {
            const XsdComplexType::OpenContent::Ptr openContent = parseOpenContent();
            complexType->contentType()->setOpenContent(openContent);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Group, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseReferredGroup(particle);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::All, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalAll(particle, complexType);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Choice, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalChoice(particle, complexType);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Sequence, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalSequence(particle, complexType);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Attribute, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseLocalAttribute(complexType);
            complexType->addAttributeUse(attributeUse);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::AttributeGroup, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseReferredAttributeGroup();
            complexType->addAttributeUse(attributeUse);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::AnyAttribute, token, namespaceToken)) {
            const XsdWildcard::Ptr wildcard = parseAnyAttribute();
            complexType->setAttributeWildcard(wildcard);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Assert, token, namespaceToken)) {
            const XsdAssertion::Ptr assertion = parseAssertion(XsdSchemaToken::Assert, XsdTagScope::Assert);
            complexType->addAssertion(assertion);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   if (!hasTypeSpecified) {
      complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
      complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
      hasComplexContent = true;
   }

   if (hasComplexContent == true) {
      resolveComplexContentType(complexType, effectiveMixed);
   }

   return complexType;
}

XsdComplexType::Ptr XsdSchemaParser::parseLocalComplexType()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::ComplexType, this);

   validateElement(XsdTagScope::LocalComplexType);

   bool hasTypeSpecified = false;
   bool hasComplexContent = true;

   const XsdComplexType::Ptr complexType(new XsdComplexType());
   complexType->setName(m_parserContext->createAnonymousName(m_targetNamespace));

   // parse attributes
   bool effectiveMixed = false;
   if (hasAttribute(QString::fromLatin1("mixed"))) {
      const QString mixed = readAttribute(QString::fromLatin1("mixed"));

      const Boolean::Ptr value = Boolean::fromLexical(mixed);
      if (value->hasError()) {
         attributeContentError("mixed", "complexType", mixed, BuiltinTypes::xsBoolean);
         return complexType;
      }

      effectiveMixed = value->as<Boolean>()->value();
   }

   validateIdAttribute("complexType");

   TagValidationHandler tagValidator(XsdTagScope::LocalComplexType, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            complexType->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::SimpleContent, token, namespaceToken)) {
            parseSimpleContent(complexType);
            hasTypeSpecified = true;
         } else if (isSchemaTag(XsdSchemaToken::ComplexContent, token, namespaceToken)) {
            bool mixed;
            parseComplexContent(complexType, &mixed);
            hasTypeSpecified = true;

            effectiveMixed = (effectiveMixed || mixed);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::OpenContent, token, namespaceToken)) {
            const XsdComplexType::OpenContent::Ptr openContent = parseOpenContent();
            complexType->contentType()->setOpenContent(openContent);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Group, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseReferredGroup(particle);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::All, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalAll(particle, complexType);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Choice, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalChoice(particle, complexType);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Sequence, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalSequence(particle, complexType);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Attribute, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseLocalAttribute(complexType);
            complexType->addAttributeUse(attributeUse);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::AttributeGroup, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseReferredAttributeGroup();
            complexType->addAttributeUse(attributeUse);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::AnyAttribute, token, namespaceToken)) {
            const XsdWildcard::Ptr wildcard = parseAnyAttribute();
            complexType->setAttributeWildcard(wildcard);

            complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
            complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);
            complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
            hasComplexContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Assert, token, namespaceToken)) {
            const XsdAssertion::Ptr assertion = parseAssertion(XsdSchemaToken::Assert, XsdTagScope::Assert);
            complexType->addAssertion(assertion);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   if (!hasTypeSpecified) {
      complexType->setWxsSuperType(BuiltinTypes::xsAnyType);
      complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);
      hasComplexContent = true;
   }

   if (hasComplexContent == true) {
      resolveComplexContentType(complexType, effectiveMixed);
   }

   return complexType;
}

void XsdSchemaParser::resolveComplexContentType(const XsdComplexType::Ptr &complexType, bool effectiveMixed)
{
   // @see http://www.w3.org/TR/xmlschema11-1/#dcl.ctd.ctcc.common

   // 1
   // the effectiveMixed contains the effective mixed value

   // 2
   bool hasEmptyContent = false;
   if (!complexType->contentType()->particle()) {
      hasEmptyContent = true; // 2.1.1
   } else {
      if (complexType->contentType()->particle()->term()->isModelGroup()) {
         const XsdModelGroup::Ptr group = complexType->contentType()->particle()->term();
         if (group->compositor() == XsdModelGroup::SequenceCompositor || group->compositor() == XsdModelGroup::AllCompositor) {
            if (group->particles().isEmpty()) {
               hasEmptyContent = true;   // 2.1.2
            }
         } else if (group->compositor() == XsdModelGroup::ChoiceCompositor) {
            if ((complexType->contentType()->particle()->minimumOccurs() == 0) && group->particles().isEmpty()) {
               hasEmptyContent = true;   // 2.1.3
            }
         }

         if ((complexType->contentType()->particle()->maximumOccursUnbounded() == false) &&
               (complexType->contentType()->particle()->maximumOccurs() == 0)) {
            hasEmptyContent = true;   // 2.1.4
         }
      }
   }

   const XsdParticle::Ptr explicitContent = (hasEmptyContent ? XsdParticle::Ptr() :
         complexType->contentType()->particle());

   // do all the other work (3, 4, 5 and 6) in the resolver, as they need access to the base type object
   m_schemaResolver->addComplexContentType(complexType, explicitContent, effectiveMixed);
}

void XsdSchemaParser::parseSimpleContent(const XsdComplexType::Ptr &complexType)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::SimpleContent, this);

   validateElement(XsdTagScope::SimpleContent);

   complexType->contentType()->setVariety(XsdComplexType::ContentType::Simple);

   // parse attributes
   validateIdAttribute("simpleContent");

   TagValidationHandler tagValidator(XsdTagScope::SimpleContent, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            complexType->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Restriction, token, namespaceToken)) {
            parseSimpleContentRestriction(complexType);
         } else if (isSchemaTag(XsdSchemaToken::Extension, token, namespaceToken)) {
            parseSimpleContentExtension(complexType);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();
}

void XsdSchemaParser::parseSimpleContentRestriction(const XsdComplexType::Ptr &complexType)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Restriction, this);

   validateElement(XsdTagScope::SimpleContentRestriction);

   complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);

   // parse attributes
   const QString baseType = readQNameAttribute(QString::fromLatin1("base"), "restriction");
   QXmlName typeName;
   convertName(baseType, NamespaceSupport::ElementName, typeName); // translate qualified name into QXmlName

   validateIdAttribute("restriction");

   XsdFacet::Hash facets;
   QList<XsdFacet::Ptr> patternFacets;
   QList<XsdFacet::Ptr> enumerationFacets;
   QList<XsdFacet::Ptr> assertionFacets;

   TagValidationHandler tagValidator(XsdTagScope::SimpleContentRestriction, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            complexType->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::SimpleType, token, namespaceToken)) {
            const XsdSimpleType::Ptr type = parseLocalSimpleType();
            type->setContext(complexType); //TODO: investigate what the schema spec really wants here?!?
            complexType->contentType()->setSimpleType(type);

            // add it to list of anonymous types as well
            addAnonymousType(type);
         } else if (isSchemaTag(XsdSchemaToken::MinExclusive, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseMinExclusiveFacet();
            addFacet(facet, facets, complexType);
         } else if (isSchemaTag(XsdSchemaToken::MinInclusive, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseMinInclusiveFacet();
            addFacet(facet, facets, complexType);
         } else if (isSchemaTag(XsdSchemaToken::MaxExclusive, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseMaxExclusiveFacet();
            addFacet(facet, facets, complexType);
         } else if (isSchemaTag(XsdSchemaToken::MaxInclusive, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseMaxInclusiveFacet();
            addFacet(facet, facets, complexType);
         } else if (isSchemaTag(XsdSchemaToken::TotalDigits, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseTotalDigitsFacet();
            addFacet(facet, facets, complexType);
         } else if (isSchemaTag(XsdSchemaToken::FractionDigits, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseFractionDigitsFacet();
            addFacet(facet, facets, complexType);
         } else if (isSchemaTag(XsdSchemaToken::Length, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseLengthFacet();
            addFacet(facet, facets, complexType);
         } else if (isSchemaTag(XsdSchemaToken::MinLength, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseMinLengthFacet();
            addFacet(facet, facets, complexType);
         } else if (isSchemaTag(XsdSchemaToken::MaxLength, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseMaxLengthFacet();
            addFacet(facet, facets, complexType);
         } else if (isSchemaTag(XsdSchemaToken::Enumeration, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseEnumerationFacet();
            enumerationFacets.append(facet);
         } else if (isSchemaTag(XsdSchemaToken::WhiteSpace, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseWhiteSpaceFacet();
            addFacet(facet, facets, complexType);
         } else if (isSchemaTag(XsdSchemaToken::Pattern, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parsePatternFacet();
            patternFacets.append(facet);
         } else if (isSchemaTag(XsdSchemaToken::Assertion, token, namespaceToken)) {
            const XsdFacet::Ptr facet = parseAssertionFacet();
            assertionFacets.append(facet);
         } else if (isSchemaTag(XsdSchemaToken::Attribute, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseLocalAttribute(complexType);
            complexType->addAttributeUse(attributeUse);
         } else if (isSchemaTag(XsdSchemaToken::AttributeGroup, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseReferredAttributeGroup();
            complexType->addAttributeUse(attributeUse);
         } else if (isSchemaTag(XsdSchemaToken::AnyAttribute, token, namespaceToken)) {
            const XsdWildcard::Ptr wildcard = parseAnyAttribute();
            complexType->setAttributeWildcard(wildcard);
         } else if (isSchemaTag(XsdSchemaToken::Assert, token, namespaceToken)) {
            const XsdAssertion::Ptr assertion = parseAssertion(XsdSchemaToken::Assert, XsdTagScope::Assert);
            complexType->addAssertion(assertion);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   // merge all pattern facets into one multi value facet
   if (!patternFacets.isEmpty()) {
      const XsdFacet::Ptr patternFacet(new XsdFacet());
      patternFacet->setType(XsdFacet::Pattern);

      AtomicValue::List multiValue;
      for (int i = 0; i < patternFacets.count(); ++i) {
         multiValue << patternFacets.at(i)->multiValue();
      }

      patternFacet->setMultiValue(multiValue);
      addFacet(patternFacet, facets, complexType);
   }

   // merge all enumeration facets into one multi value facet
   if (!enumerationFacets.isEmpty()) {
      const XsdFacet::Ptr enumerationFacet(new XsdFacet());
      enumerationFacet->setType(XsdFacet::Enumeration);

      AtomicValue::List multiValue;
      for (int i = 0; i < enumerationFacets.count(); ++i) {
         multiValue << enumerationFacets.at(i)->multiValue();
      }

      enumerationFacet->setMultiValue(multiValue);
      addFacet(enumerationFacet, facets, complexType);
   }

   // merge all assertion facets into one facet
   if (!assertionFacets.isEmpty()) {
      const XsdFacet::Ptr assertionFacet(new XsdFacet());
      assertionFacet->setType(XsdFacet::Assertion);

      XsdAssertion::List assertions;
      for (int i = 0; i < assertionFacets.count(); ++i) {
         assertions << assertionFacets.at(i)->assertions();
      }

      assertionFacet->setAssertions(assertions);
      addFacet(assertionFacet, facets, complexType);
   }

   m_schemaResolver->addComplexBaseType(complexType, typeName, currentSourceLocation(), facets); // add to resolver
}

void XsdSchemaParser::parseSimpleContentExtension(const XsdComplexType::Ptr &complexType)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Extension, this);

   validateElement(XsdTagScope::SimpleContentExtension);

   complexType->setDerivationMethod(XsdComplexType::DerivationExtension);

   // parse attributes
   const QString baseType = readQNameAttribute(QString::fromLatin1("base"), "extension");
   QXmlName typeName;
   convertName(baseType, NamespaceSupport::ElementName, typeName); // translate qualified name into QXmlName
   m_schemaResolver->addComplexBaseType(complexType, typeName, currentSourceLocation()); // add to resolver

   validateIdAttribute("extension");

   TagValidationHandler tagValidator(XsdTagScope::SimpleContentExtension, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            complexType->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Attribute, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseLocalAttribute(complexType);
            complexType->addAttributeUse(attributeUse);
         } else if (isSchemaTag(XsdSchemaToken::AttributeGroup, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseReferredAttributeGroup();
            complexType->addAttributeUse(attributeUse);
         } else if (isSchemaTag(XsdSchemaToken::AnyAttribute, token, namespaceToken)) {
            const XsdWildcard::Ptr wildcard = parseAnyAttribute();
            complexType->setAttributeWildcard(wildcard);
         } else if (isSchemaTag(XsdSchemaToken::Assert, token, namespaceToken)) {
            const XsdAssertion::Ptr assertion = parseAssertion(XsdSchemaToken::Assert, XsdTagScope::Assert);
            complexType->addAssertion(assertion);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();
}

void XsdSchemaParser::parseComplexContent(const XsdComplexType::Ptr &complexType, bool *mixed)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::ComplexContent, this);

   validateElement(XsdTagScope::ComplexContent);

   complexType->contentType()->setVariety(XsdComplexType::ContentType::ElementOnly);

   // parse attributes
   if (hasAttribute(QString::fromLatin1("mixed"))) {
      const QString mixedStr = readAttribute(QString::fromLatin1("mixed"));

      const Boolean::Ptr value = Boolean::fromLexical(mixedStr);
      if (value->hasError()) {
         attributeContentError("mixed", "complexType", mixedStr, BuiltinTypes::xsBoolean);
         return;
      }

      *mixed = value->as<Boolean>()->value();
   } else {
      *mixed = false;
   }

   validateIdAttribute("complexContent");

   TagValidationHandler tagValidator(XsdTagScope::ComplexContent, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            complexType->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Restriction, token, namespaceToken)) {
            parseComplexContentRestriction(complexType);
         } else if (isSchemaTag(XsdSchemaToken::Extension, token, namespaceToken)) {
            parseComplexContentExtension(complexType);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();
}

void XsdSchemaParser::parseComplexContentRestriction(const XsdComplexType::Ptr &complexType)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Restriction, this);

   validateElement(XsdTagScope::ComplexContentRestriction);

   complexType->setDerivationMethod(XsdComplexType::DerivationRestriction);

   // parse attributes
   const QString baseType = readQNameAttribute(QString::fromLatin1("base"), "restriction");
   QXmlName typeName;
   convertName(baseType, NamespaceSupport::ElementName, typeName); // translate qualified name into QXmlName
   m_schemaResolver->addComplexBaseType(complexType, typeName, currentSourceLocation()); // add to resolver

   validateIdAttribute("restriction");

   TagValidationHandler tagValidator(XsdTagScope::ComplexContentRestriction, this, NamePool::Ptr(m_namePool));

   bool hasContent = false;
   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            complexType->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::OpenContent, token, namespaceToken)) {
            const XsdComplexType::OpenContent::Ptr openContent = parseOpenContent();
            complexType->contentType()->setOpenContent(openContent);
            hasContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Group, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseReferredGroup(particle);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);
            hasContent = true;
         } else if (isSchemaTag(XsdSchemaToken::All, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalAll(particle, complexType);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);
            hasContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Choice, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalChoice(particle, complexType);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);
            hasContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Sequence, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalSequence(particle, complexType);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);
            hasContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Attribute, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseLocalAttribute(complexType);
            complexType->addAttributeUse(attributeUse);
         } else if (isSchemaTag(XsdSchemaToken::AttributeGroup, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseReferredAttributeGroup();
            complexType->addAttributeUse(attributeUse);
         } else if (isSchemaTag(XsdSchemaToken::AnyAttribute, token, namespaceToken)) {
            const XsdWildcard::Ptr wildcard = parseAnyAttribute();
            complexType->setAttributeWildcard(wildcard);
         } else if (isSchemaTag(XsdSchemaToken::Assert, token, namespaceToken)) {
            const XsdAssertion::Ptr assertion = parseAssertion(XsdSchemaToken::Assert, XsdTagScope::Assert);
            complexType->addAssertion(assertion);
         } else {
            parseUnknown();
         }
      }
   }

   if (!hasContent) {
      complexType->contentType()->setVariety(XsdComplexType::ContentType::Empty);
   }

   tagValidator.finalize();
}

void XsdSchemaParser::parseComplexContentExtension(const XsdComplexType::Ptr &complexType)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Extension, this);

   validateElement(XsdTagScope::ComplexContentExtension);

   complexType->setDerivationMethod(XsdComplexType::DerivationExtension);

   // parse attributes
   const QString baseType = readQNameAttribute(QString::fromLatin1("base"), "extension");
   QXmlName typeName;
   convertName(baseType, NamespaceSupport::ElementName, typeName); // translate qualified name into QXmlName
   m_schemaResolver->addComplexBaseType(complexType, typeName, currentSourceLocation()); // add to resolver

   validateIdAttribute("extension");

   TagValidationHandler tagValidator(XsdTagScope::ComplexContentExtension, this, NamePool::Ptr(m_namePool));

   bool hasContent = false;
   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            complexType->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::OpenContent, token, namespaceToken)) {
            const XsdComplexType::OpenContent::Ptr openContent = parseOpenContent();
            complexType->contentType()->setOpenContent(openContent);
            hasContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Group, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseReferredGroup(particle);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);
            hasContent = true;
         } else if (isSchemaTag(XsdSchemaToken::All, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalAll(particle, complexType);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);
            hasContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Choice, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalChoice(particle, complexType);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);
            hasContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Sequence, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalSequence(particle, complexType);
            particle->setTerm(term);
            complexType->contentType()->setParticle(particle);
            hasContent = true;
         } else if (isSchemaTag(XsdSchemaToken::Attribute, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseLocalAttribute(complexType);
            complexType->addAttributeUse(attributeUse);
         } else if (isSchemaTag(XsdSchemaToken::AttributeGroup, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseReferredAttributeGroup();
            complexType->addAttributeUse(attributeUse);
         } else if (isSchemaTag(XsdSchemaToken::AnyAttribute, token, namespaceToken)) {
            const XsdWildcard::Ptr wildcard = parseAnyAttribute();
            complexType->setAttributeWildcard(wildcard);
         } else if (isSchemaTag(XsdSchemaToken::Assert, token, namespaceToken)) {
            const XsdAssertion::Ptr assertion = parseAssertion(XsdSchemaToken::Assert, XsdTagScope::Assert);
            complexType->addAssertion(assertion);
         } else {
            parseUnknown();
         }
      }
   }

   if (!hasContent) {
      complexType->contentType()->setVariety(XsdComplexType::ContentType::Empty);
   }

   tagValidator.finalize();
}


XsdAssertion::Ptr XsdSchemaParser::parseAssertion(const XsdSchemaToken::NodeName &nodeName,
      const XsdTagScope::Type &tag)
{
   const ElementNamespaceHandler namespaceHandler(nodeName, this);

   validateElement(tag);

   const XsdAssertion::Ptr assertion(new XsdAssertion());

   // parse attributes

   const XsdXPathExpression::Ptr expression = readXPathExpression("assertion");
   assertion->setTest(expression);

   const QString test = readXPathAttribute(QString::fromLatin1("test"), XPath20, "assertion");
   expression->setExpression(test);

   validateIdAttribute("assertion");

   TagValidationHandler tagValidator(tag, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            assertion->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return assertion;
}

XsdComplexType::OpenContent::Ptr XsdSchemaParser::parseOpenContent()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::OpenContent, this);

   validateElement(XsdTagScope::OpenContent);

   const XsdComplexType::OpenContent::Ptr openContent(new XsdComplexType::OpenContent());

   if (hasAttribute(QString::fromLatin1("mode"))) {
      const QString mode = readAttribute(QString::fromLatin1("mode"));

      if (mode == QString::fromLatin1("none")) {
         m_defaultOpenContent->setMode(XsdComplexType::OpenContent::None);
      } else if (mode == QString::fromLatin1("interleave")) {
         m_defaultOpenContent->setMode(XsdComplexType::OpenContent::Interleave);
      } else if (mode == QString::fromLatin1("suffix")) {
         m_defaultOpenContent->setMode(XsdComplexType::OpenContent::Suffix);
      } else {
         attributeContentError("mode", "openContent", mode);
         return openContent;
      }
   } else {
      openContent->setMode(XsdComplexType::OpenContent::Interleave);
   }

   validateIdAttribute("openContent");

   TagValidationHandler tagValidator(XsdTagScope::OpenContent, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            openContent->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Any, token, namespaceToken)) {
            const XsdParticle::Ptr particle;
            const XsdWildcard::Ptr wildcard = parseAny(particle);
            openContent->setWildcard(wildcard);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return openContent;
}

XsdModelGroup::Ptr XsdSchemaParser::parseNamedGroup()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Group, this);

   validateElement(XsdTagScope::NamedGroup);

   const XsdModelGroup::Ptr modelGroup(new XsdModelGroup());
   XsdModelGroup::Ptr group;

   QXmlName objectName;
   if (hasAttribute(QString::fromLatin1("name"))) {
      objectName = m_namePool->allocateQName(m_targetNamespace, readNameAttribute("group"));
   }

   validateIdAttribute("group");

   TagValidationHandler tagValidator(XsdTagScope::NamedGroup, this, NamePool::Ptr(m_namePool));

   XsdAnnotation::Ptr annotation;

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            annotation = parseAnnotation();
         } else if (isSchemaTag(XsdSchemaToken::All, token, namespaceToken)) {
            group = parseAll(modelGroup);
         } else if (isSchemaTag(XsdSchemaToken::Choice, token, namespaceToken)) {
            group = parseChoice(modelGroup);
         } else if (isSchemaTag(XsdSchemaToken::Sequence, token, namespaceToken)) {
            group = parseSequence(modelGroup);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   group->setName(objectName);

   if (annotation) {
      group->addAnnotation(annotation);
   }

   return group;
}

XsdTerm::Ptr XsdSchemaParser::parseReferredGroup(const XsdParticle::Ptr &particle)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Group, this);

   validateElement(XsdTagScope::ReferredGroup);

   const XsdReference::Ptr reference(new XsdReference());
   reference->setType(XsdReference::ModelGroup);
   reference->setSourceLocation(currentSourceLocation());

   // parse attributes
   if (!parseMinMaxConstraint(particle, "group")) {
      return reference;
   }

   const QString value = readQNameAttribute(QString::fromLatin1("ref"), "group");
   QXmlName referenceName;
   convertName(value, NamespaceSupport::ElementName, referenceName); // translate qualified name into QXmlName
   reference->setReferenceName(referenceName);

   validateIdAttribute("group");

   TagValidationHandler tagValidator(XsdTagScope::ReferredGroup, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            reference->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return reference;
}

XsdModelGroup::Ptr XsdSchemaParser::parseAll(const NamedSchemaComponent::Ptr &parent)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::All, this);

   validateElement(XsdTagScope::All);

   const XsdModelGroup::Ptr modelGroup(new XsdModelGroup());
   modelGroup->setCompositor(XsdModelGroup::AllCompositor);

   validateIdAttribute("all");

   TagValidationHandler tagValidator(XsdTagScope::All, this, NamePool::Ptr(m_namePool));

   XsdParticle::List particles;
   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            modelGroup->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Element, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalElement(particle, parent);
            particle->setTerm(term);

            if (particle->maximumOccursUnbounded() || particle->maximumOccurs() > 1) {
               error(QtXmlPatterns::tr("%1 attribute of %2 element must be %3 or %4.")
                     .arg(formatAttribute("maxOccurs"))
                     .arg(formatElement("all"))
                     .arg(formatData("0"))
                     .arg(formatData("1")));
               return modelGroup;
            }

            particles.append(particle);
         } else {
            parseUnknown();
         }
      }
   }

   modelGroup->setParticles(particles);

   tagValidator.finalize();

   return modelGroup;
}

XsdModelGroup::Ptr XsdSchemaParser::parseLocalAll(const XsdParticle::Ptr &particle,
      const NamedSchemaComponent::Ptr &parent)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::All, this);

   validateElement(XsdTagScope::LocalAll);

   const XsdModelGroup::Ptr modelGroup(new XsdModelGroup());
   modelGroup->setCompositor(XsdModelGroup::AllCompositor);

   // parse attributes
   if (!parseMinMaxConstraint(particle, "all")) {
      return modelGroup;
   }
   if (particle->maximumOccursUnbounded() || particle->maximumOccurs() != 1) {
      error(QtXmlPatterns::tr("%1 attribute of %2 element must have a value of %3.")
            .arg(formatAttribute("maxOccurs"))
            .arg(formatElement("all"))
            .arg(formatData("1")));
      return modelGroup;
   }
   if (particle->minimumOccurs() != 0 && particle->minimumOccurs() != 1) {
      error(QtXmlPatterns::tr("%1 attribute of %2 element must have a value of %3 or %4.")
            .arg(formatAttribute("minOccurs"))
            .arg(formatElement("all"))
            .arg(formatData("0"))
            .arg(formatData("1")));
      return modelGroup;
   }

   validateIdAttribute("all");

   TagValidationHandler tagValidator(XsdTagScope::LocalAll, this, NamePool::Ptr(m_namePool));

   XsdParticle::List particles;
   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            modelGroup->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Element, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalElement(particle, parent);
            particle->setTerm(term);

            if (particle->maximumOccursUnbounded() || particle->maximumOccurs() > 1) {
               error(QtXmlPatterns::tr("%1 attribute of %2 element must have a value of %3 or %4.")
                     .arg(formatAttribute("maxOccurs"))
                     .arg(formatElement("all"))
                     .arg(formatData("0"))
                     .arg(formatData("1")));
               return modelGroup;
            }

            particles.append(particle);
         } else {
            parseUnknown();
         }
      }
   }

   modelGroup->setParticles(particles);

   tagValidator.finalize();

   return modelGroup;
}

XsdModelGroup::Ptr XsdSchemaParser::parseChoice(const NamedSchemaComponent::Ptr &parent)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Choice, this);

   validateElement(XsdTagScope::Choice);

   const XsdModelGroup::Ptr modelGroup(new XsdModelGroup());
   modelGroup->setCompositor(XsdModelGroup::ChoiceCompositor);

   validateIdAttribute("choice");

   XsdParticle::List particles;

   TagValidationHandler tagValidator(XsdTagScope::Choice, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            modelGroup->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Element, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalElement(particle, parent);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Group, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseReferredGroup(particle);
            m_schemaResolver->addAllGroupCheck(term);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Choice, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalChoice(particle, parent);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Sequence, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalSequence(particle, parent);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Any, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseAny(particle);
            particle->setTerm(term);
            particles.append(particle);
         } else {
            parseUnknown();
         }
      }
   }

   modelGroup->setParticles(particles);

   tagValidator.finalize();

   return modelGroup;
}

XsdModelGroup::Ptr XsdSchemaParser::parseLocalChoice(const XsdParticle::Ptr &particle,
      const NamedSchemaComponent::Ptr &parent)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Choice, this);

   validateElement(XsdTagScope::LocalChoice);

   const XsdModelGroup::Ptr modelGroup(new XsdModelGroup());
   modelGroup->setCompositor(XsdModelGroup::ChoiceCompositor);

   // parse attributes
   if (!parseMinMaxConstraint(particle, "choice")) {
      return modelGroup;
   }

   validateIdAttribute("choice");

   XsdParticle::List particles;

   TagValidationHandler tagValidator(XsdTagScope::LocalChoice, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            modelGroup->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Element, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalElement(particle, parent);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Group, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseReferredGroup(particle);
            m_schemaResolver->addAllGroupCheck(term);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Choice, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalChoice(particle, parent);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Sequence, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalSequence(particle, parent);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Any, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseAny(particle);
            particle->setTerm(term);
            particles.append(particle);
         } else {
            parseUnknown();
         }
      }
   }

   modelGroup->setParticles(particles);

   tagValidator.finalize();

   return modelGroup;
}

XsdModelGroup::Ptr XsdSchemaParser::parseSequence(const NamedSchemaComponent::Ptr &parent)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Sequence, this);

   validateElement(XsdTagScope::Sequence);

   const XsdModelGroup::Ptr modelGroup(new XsdModelGroup());
   modelGroup->setCompositor(XsdModelGroup::SequenceCompositor);

   validateIdAttribute("sequence");

   XsdParticle::List particles;

   TagValidationHandler tagValidator(XsdTagScope::Sequence, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            modelGroup->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Element, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalElement(particle, parent);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Group, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseReferredGroup(particle);
            m_schemaResolver->addAllGroupCheck(term);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Choice, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalChoice(particle, parent);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Sequence, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalSequence(particle, parent);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Any, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseAny(particle);
            particle->setTerm(term);
            particles.append(particle);
         } else {
            parseUnknown();
         }
      }
   }

   modelGroup->setParticles(particles);

   tagValidator.finalize();

   return modelGroup;
}

XsdModelGroup::Ptr XsdSchemaParser::parseLocalSequence(const XsdParticle::Ptr &particle,
      const NamedSchemaComponent::Ptr &parent)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Sequence, this);

   validateElement(XsdTagScope::LocalSequence);

   const XsdModelGroup::Ptr modelGroup(new XsdModelGroup());
   modelGroup->setCompositor(XsdModelGroup::SequenceCompositor);

   // parse attributes
   if (!parseMinMaxConstraint(particle, "sequence")) {
      return modelGroup;
   }

   validateIdAttribute("sequence");

   XsdParticle::List particles;

   TagValidationHandler tagValidator(XsdTagScope::LocalSequence, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            modelGroup->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Element, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalElement(particle, parent);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Group, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseReferredGroup(particle);
            m_schemaResolver->addAllGroupCheck(term);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Choice, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalChoice(particle, parent);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Sequence, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseLocalSequence(particle, parent);
            particle->setTerm(term);
            particles.append(particle);
         } else if (isSchemaTag(XsdSchemaToken::Any, token, namespaceToken)) {
            const XsdParticle::Ptr particle(new XsdParticle());
            const XsdTerm::Ptr term = parseAny(particle);
            particle->setTerm(term);
            particles.append(particle);
         } else {
            parseUnknown();
         }
      }
   }

   modelGroup->setParticles(particles);

   tagValidator.finalize();

   return modelGroup;
}

XsdAttribute::Ptr XsdSchemaParser::parseGlobalAttribute()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Attribute, this);

   validateElement(XsdTagScope::GlobalAttribute);

   const XsdAttribute::Ptr attribute(new XsdAttribute());
   attribute->setScope(XsdAttribute::Scope::Ptr(new XsdAttribute::Scope()));
   attribute->scope()->setVariety(XsdAttribute::Scope::Global);

   if (hasAttribute(QString::fromLatin1("default")) && hasAttribute(QString::fromLatin1("fixed"))) {
      error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
            .arg(formatElement("attribute"))
            .arg(formatAttribute("default"))
            .arg(formatAttribute("fixed")));
      return attribute;
   }

   // parse attributes
   if (hasAttribute(QString::fromLatin1("default"))) {
      const QString value = readAttribute(QString::fromLatin1("default"));
      attribute->setValueConstraint(XsdAttribute::ValueConstraint::Ptr(new XsdAttribute::ValueConstraint()));
      attribute->valueConstraint()->setVariety(XsdAttribute::ValueConstraint::Default);
      attribute->valueConstraint()->setValue(value);
   } else if (hasAttribute(QString::fromLatin1("fixed"))) {
      const QString value = readAttribute(QString::fromLatin1("fixed"));
      attribute->setValueConstraint(XsdAttribute::ValueConstraint::Ptr(new XsdAttribute::ValueConstraint()));
      attribute->valueConstraint()->setVariety(XsdAttribute::ValueConstraint::Fixed);
      attribute->valueConstraint()->setValue(value);
   }

   const QXmlName objectName = m_namePool->allocateQName(m_targetNamespace, readNameAttribute("attribute"));
   if ((objectName.namespaceURI() == StandardNamespaces::xsi) &&
         (m_namePool->stringForLocalName(objectName.localName()) != QString::fromLatin1("type")) &&
         (m_namePool->stringForLocalName(objectName.localName()) != QString::fromLatin1("nil")) &&
         (m_namePool->stringForLocalName(objectName.localName()) != QString::fromLatin1("schemaLocation")) &&
         (m_namePool->stringForLocalName(objectName.localName()) != QString::fromLatin1("noNamespaceSchemaLocation"))) {

      error(QtXmlPatterns::tr("Content of %1 attribute of %2 element must not be from namespace %3.")
            .arg(formatAttribute("name"))
            .arg(formatElement("attribute"))
            .arg(formatURI(CommonNamespaces::XSI)));
      return attribute;
   }
   if (m_namePool->stringForLocalName(objectName.localName()) == QString::fromLatin1("xmlns")) {
      error(QtXmlPatterns::tr("%1 attribute of %2 element must not be %3.")
            .arg(formatAttribute("name"))
            .arg(formatElement("attribute"))
            .arg(formatData("xmlns")));
      return attribute;
   }
   attribute->setName(objectName);

   bool hasTypeAttribute = false;
   bool hasTypeSpecified = false;

   if (hasAttribute(QString::fromLatin1("type"))) {
      hasTypeAttribute = true;

      const QString type = readQNameAttribute(QString::fromLatin1("type"), "attribute");
      QXmlName typeName;
      convertName(type, NamespaceSupport::ElementName, typeName); // translate qualified name into QXmlName
      m_schemaResolver->addAttributeType(attribute, typeName, currentSourceLocation()); // add to resolver
      hasTypeSpecified = true;
   }

   validateIdAttribute("attribute");

   TagValidationHandler tagValidator(XsdTagScope::GlobalAttribute, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            attribute->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::SimpleType, token, namespaceToken)) {
            if (hasTypeAttribute) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("attribute"))
                     .arg(formatElement("simpleType"))
                     .arg(formatAttribute("type")));
               break;
            }

            const XsdSimpleType::Ptr type = parseLocalSimpleType();
            type->setContext(attribute);
            attribute->setType(type);
            hasTypeSpecified = true;

            // add it to list of anonymous types as well
            addAnonymousType(type);
         } else {
            parseUnknown();
         }
      }
   }

   if (!hasTypeSpecified) {
      attribute->setType(BuiltinTypes::xsAnySimpleType); // default value
      return attribute;
   }

   tagValidator.finalize();

   return attribute;
}

XsdAttributeUse::Ptr XsdSchemaParser::parseLocalAttribute(const NamedSchemaComponent::Ptr &parent)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Attribute, this);

   validateElement(XsdTagScope::LocalAttribute);

   bool hasRefAttribute = false;
   bool hasTypeAttribute = false;
   bool hasTypeSpecified = false;

   XsdAttributeUse::Ptr attributeUse;
   if (hasAttribute(QString::fromLatin1("ref"))) {
      const XsdAttributeReference::Ptr reference = XsdAttributeReference::Ptr(new XsdAttributeReference());
      reference->setType(XsdAttributeReference::AttributeUse);
      reference->setSourceLocation(currentSourceLocation());

      attributeUse = reference;
      hasRefAttribute = true;
   } else {
      attributeUse = XsdAttributeUse::Ptr(new XsdAttributeUse());
   }

   if (hasAttribute(QString::fromLatin1("default")) && hasAttribute(QString::fromLatin1("fixed"))) {
      error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
            .arg(formatElement("attribute"))
            .arg(formatAttribute("default"))
            .arg(formatAttribute("fixed")));
      return attributeUse;
   }

   if (hasRefAttribute) {
      if (hasAttribute(QString::fromLatin1("form"))) {
         error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
               .arg(formatElement("attribute"))
               .arg(formatAttribute("ref"))
               .arg(formatAttribute("form")));
         return attributeUse;
      }
      if (hasAttribute(QString::fromLatin1("name"))) {
         error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
               .arg(formatElement("attribute"))
               .arg(formatAttribute("ref"))
               .arg(formatAttribute("name")));
         return attributeUse;
      }
      if (hasAttribute(QString::fromLatin1("type"))) {
         error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
               .arg(formatElement("attribute"))
               .arg(formatAttribute("ref"))
               .arg(formatAttribute("type")));
         return attributeUse;
      }
   }

   // parse attributes

   // default, fixed and use are handled by both, attribute use and attribute reference
   if (hasAttribute(QString::fromLatin1("default"))) {
      const QString value = readAttribute(QString::fromLatin1("default"));
      attributeUse->setValueConstraint(XsdAttributeUse::ValueConstraint::Ptr(new XsdAttributeUse::ValueConstraint()));
      attributeUse->valueConstraint()->setVariety(XsdAttributeUse::ValueConstraint::Default);
      attributeUse->valueConstraint()->setValue(value);
   } else if (hasAttribute(QString::fromLatin1("fixed"))) {
      const QString value = readAttribute(QString::fromLatin1("fixed"));
      attributeUse->setValueConstraint(XsdAttributeUse::ValueConstraint::Ptr(new XsdAttributeUse::ValueConstraint()));
      attributeUse->valueConstraint()->setVariety(XsdAttributeUse::ValueConstraint::Fixed);
      attributeUse->valueConstraint()->setValue(value);
   }

   if (hasAttribute(QString::fromLatin1("use"))) {
      const QString value = readAttribute(QString::fromLatin1("use"));
      if (value != QString::fromLatin1("optional") &&
            value != QString::fromLatin1("prohibited") &&
            value != QString::fromLatin1("required")) {
         attributeContentError("use", "attribute", value);
         return attributeUse;
      }

      if (value == QString::fromLatin1("optional")) {
         attributeUse->setUseType(XsdAttributeUse::OptionalUse);
      } else if (value == QString::fromLatin1("prohibited")) {
         attributeUse->setUseType(XsdAttributeUse::ProhibitedUse);
      } else if (value == QString::fromLatin1("required")) {
         attributeUse->setUseType(XsdAttributeUse::RequiredUse);
      }

      if (attributeUse->valueConstraint() &&
            attributeUse->valueConstraint()->variety() == XsdAttributeUse::ValueConstraint::Default &&
            value != QString::fromLatin1("optional")) {
         error(QtXmlPatterns::tr("%1 attribute of %2 element must have the value %3 because the %4 attribute is set.")
               .arg(formatAttribute("use"))
               .arg(formatElement("attribute"))
               .arg(formatData("optional"))
               .arg(formatElement("default")));
         return attributeUse;
      }
   }

   const XsdAttribute::Ptr attribute(new XsdAttribute());

   attributeUse->setAttribute(attribute);
   m_componentLocationHash.insert(attribute, currentSourceLocation());

   attribute->setScope(XsdAttribute::Scope::Ptr(new XsdAttribute::Scope()));
   attribute->scope()->setVariety(XsdAttribute::Scope::Local);
   attribute->scope()->setParent(parent);

   // now make a difference between attribute reference and attribute use
   if (hasRefAttribute) {
      const QString reference = readQNameAttribute(QString::fromLatin1("ref"), "attribute");
      QXmlName referenceName;
      convertName(reference, NamespaceSupport::ElementName, referenceName);   // translate qualified name into QXmlName

      const XsdAttributeReference::Ptr attributeReference = attributeUse;
      attributeReference->setReferenceName(referenceName);
   } else {
      if (hasAttribute(QString::fromLatin1("name"))) {
         const QString attributeName = readNameAttribute("attribute");

         QXmlName objectName;
         if (hasAttribute(QString::fromLatin1("form"))) {
            const QString value = readAttribute(QString::fromLatin1("form"));
            if (value != QString::fromLatin1("qualified") && value != QString::fromLatin1("unqualified")) {
               attributeContentError("form", "attribute", value);
               return attributeUse;
            }

            if (value == QString::fromLatin1("qualified")) {
               objectName = m_namePool->allocateQName(m_targetNamespace, attributeName);
            } else {
               objectName = m_namePool->allocateQName(QString(), attributeName);
            }
         } else {
            if (m_attributeFormDefault == QString::fromLatin1("qualified")) {
               objectName = m_namePool->allocateQName(m_targetNamespace, attributeName);
            } else {
               objectName = m_namePool->allocateQName(QString(), attributeName);
            }
         }

         if ((objectName.namespaceURI() == StandardNamespaces::xsi) &&
               (m_namePool->stringForLocalName(objectName.localName()) != QString::fromLatin1("type")) &&
               (m_namePool->stringForLocalName(objectName.localName()) != QString::fromLatin1("nil")) &&
               (m_namePool->stringForLocalName(objectName.localName()) != QString::fromLatin1("schemaLocation")) &&
               (m_namePool->stringForLocalName(objectName.localName()) != QString::fromLatin1("noNamespaceSchemaLocation"))) {

            error(QtXmlPatterns::tr("Content of %1 attribute of %2 element must not be from namespace %3.")
                  .arg(formatAttribute("name"))
                  .arg(formatElement("attribute"))
                  .arg(formatURI(CommonNamespaces::XSI)));
            return attributeUse;
         }
         if (m_namePool->stringForLocalName(objectName.localName()) == QString::fromLatin1("xmlns")) {
            error(QtXmlPatterns::tr("%1 attribute of %2 element must not be %3.")
                  .arg(formatAttribute("name"))
                  .arg(formatElement("attribute"))
                  .arg(formatData("xmlns")));
            return attributeUse;
         }

         attribute->setName(objectName);
      }

      if (hasAttribute(QString::fromLatin1("type"))) {
         hasTypeAttribute = true;

         const QString type = readQNameAttribute(QString::fromLatin1("type"), "attribute");
         QXmlName typeName;
         convertName(type, NamespaceSupport::ElementName, typeName); // translate qualified name into QXmlName
         m_schemaResolver->addAttributeType(attribute, typeName, currentSourceLocation()); // add to resolver
         hasTypeSpecified = true;
      }

      if (attributeUse->valueConstraint()) {
         //TODO: check whether assigning the value constraint of the attribute use to the attribute is correct
         if (!attribute->valueConstraint()) {
            attribute->setValueConstraint(XsdAttribute::ValueConstraint::Ptr(new XsdAttribute::ValueConstraint()));
         }

         attribute->valueConstraint()->setVariety((XsdAttribute::ValueConstraint::Variety)
               attributeUse->valueConstraint()->variety());
         attribute->valueConstraint()->setValue(attributeUse->valueConstraint()->value());
         attribute->valueConstraint()->setLexicalForm(attributeUse->valueConstraint()->lexicalForm());
      }
   }

   validateIdAttribute("attribute");

   TagValidationHandler tagValidator(XsdTagScope::LocalAttribute, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            attribute->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::SimpleType, token, namespaceToken)) {
            if (hasTypeAttribute) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("attribute"))
                     .arg(formatElement("simpleType"))
                     .arg(formatAttribute("type")));
               break;
            }
            if (hasRefAttribute) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("attribute"))
                     .arg(formatElement("simpleType"))
                     .arg(formatAttribute("ref")));
               break;
            }

            const XsdSimpleType::Ptr type = parseLocalSimpleType();
            type->setContext(attribute);
            attribute->setType(type);
            hasTypeSpecified = true;

            // add it to list of anonymous types as well
            addAnonymousType(type);
         } else {
            parseUnknown();
         }
      }
   }

   if (!hasTypeSpecified) {
      attribute->setType(BuiltinTypes::xsAnySimpleType); // default value
   }

   tagValidator.finalize();

   return attributeUse;
}

XsdAttributeGroup::Ptr XsdSchemaParser::parseNamedAttributeGroup()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::AttributeGroup, this);

   validateElement(XsdTagScope::NamedAttributeGroup);

   const XsdAttributeGroup::Ptr attributeGroup(new XsdAttributeGroup());

   // parse attributes
   const QXmlName objectName = m_namePool->allocateQName(m_targetNamespace, readNameAttribute("attributeGroup"));
   attributeGroup->setName(objectName);

   validateIdAttribute("attributeGroup");

   TagValidationHandler tagValidator(XsdTagScope::NamedAttributeGroup, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            attributeGroup->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Attribute, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseLocalAttribute(attributeGroup);

            if (attributeUse->useType() == XsdAttributeUse::ProhibitedUse) {
               warning(QtXmlPatterns::tr("Specifying use='prohibited' inside an attribute group has no effect."));
            } else {
               attributeGroup->addAttributeUse(attributeUse);
            }
         } else if (isSchemaTag(XsdSchemaToken::AttributeGroup, token, namespaceToken)) {
            const XsdAttributeUse::Ptr attributeUse = parseReferredAttributeGroup();
            attributeGroup->addAttributeUse(attributeUse);
         } else if (isSchemaTag(XsdSchemaToken::AnyAttribute, token, namespaceToken)) {
            const XsdWildcard::Ptr wildcard = parseAnyAttribute();
            attributeGroup->setWildcard(wildcard);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return attributeGroup;
}

XsdAttributeUse::Ptr XsdSchemaParser::parseReferredAttributeGroup()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::AttributeGroup, this);

   validateElement(XsdTagScope::ReferredAttributeGroup);

   const XsdAttributeReference::Ptr attributeReference(new XsdAttributeReference());
   attributeReference->setType(XsdAttributeReference::AttributeGroup);
   attributeReference->setSourceLocation(currentSourceLocation());

   // parse attributes
   const QString reference = readQNameAttribute(QString::fromLatin1("ref"), "attributeGroup");
   QXmlName referenceName;
   convertName(reference, NamespaceSupport::ElementName, referenceName); // translate qualified name into QXmlName
   attributeReference->setReferenceName(referenceName);

   validateIdAttribute("attributeGroup");

   TagValidationHandler tagValidator(XsdTagScope::ReferredAttributeGroup, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            attributeReference->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return attributeReference;
}

XsdElement::Ptr XsdSchemaParser::parseGlobalElement()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Element, this);

   validateElement(XsdTagScope::GlobalElement);

   const XsdElement::Ptr element(new XsdElement());
   element->setScope(XsdElement::Scope::Ptr(new XsdElement::Scope()));
   element->scope()->setVariety(XsdElement::Scope::Global);

   bool hasTypeAttribute = false;
   bool hasTypeSpecified = false;
   bool hasSubstitutionGroup = false;

   // parse attributes
   const QXmlName objectName = m_namePool->allocateQName(m_targetNamespace, readNameAttribute("element"));
   element->setName(objectName);

   if (hasAttribute(QString::fromLatin1("abstract"))) {
      const QString abstract = readAttribute(QString::fromLatin1("abstract"));

      const Boolean::Ptr value = Boolean::fromLexical(abstract);
      if (value->hasError()) {
         attributeContentError("abstract", "element", abstract, BuiltinTypes::xsBoolean);
         return element;
      }

      element->setIsAbstract(value->as<Boolean>()->value());
   } else {
      element->setIsAbstract(false); // the default value
   }

   if (hasAttribute(QString::fromLatin1("default")) && hasAttribute(QString::fromLatin1("fixed"))) {
      error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
            .arg(formatElement("element"))
            .arg(formatAttribute("default"))
            .arg(formatAttribute("fixed")));
      return element;
   }

   if (hasAttribute(QString::fromLatin1("default"))) {
      const QString value = readAttribute(QString::fromLatin1("default"));
      element->setValueConstraint(XsdElement::ValueConstraint::Ptr(new XsdElement::ValueConstraint()));
      element->valueConstraint()->setVariety(XsdElement::ValueConstraint::Default);
      element->valueConstraint()->setValue(value);
   } else if (hasAttribute(QString::fromLatin1("fixed"))) {
      const QString value = readAttribute(QString::fromLatin1("fixed"));
      element->setValueConstraint(XsdElement::ValueConstraint::Ptr(new XsdElement::ValueConstraint()));
      element->valueConstraint()->setVariety(XsdElement::ValueConstraint::Fixed);
      element->valueConstraint()->setValue(value);
   }

   element->setDisallowedSubstitutions(readBlockingConstraintAttribute(NamedSchemaComponent::ExtensionConstraint |
                                       NamedSchemaComponent::RestrictionConstraint | NamedSchemaComponent::SubstitutionConstraint, "element"));
   element->setSubstitutionGroupExclusions(readDerivationConstraintAttribute(SchemaType::ExtensionConstraint |
                                           SchemaType::RestrictionConstraint, "element"));

   if (hasAttribute(QString::fromLatin1("nillable"))) {
      const QString nillable = readAttribute(QString::fromLatin1("nillable"));

      const Boolean::Ptr value = Boolean::fromLexical(nillable);
      if (value->hasError()) {
         attributeContentError("nillable", "element", nillable, BuiltinTypes::xsBoolean);
         return element;
      }

      element->setIsNillable(value->as<Boolean>()->value());
   } else {
      element->setIsNillable(false); // the default value
   }

   if (hasAttribute(QString::fromLatin1("type"))) {
      const QString type = readQNameAttribute(QString::fromLatin1("type"), "element");
      QXmlName typeName;
      convertName(type, NamespaceSupport::ElementName, typeName); // translate qualified name into QXmlName
      m_schemaResolver->addElementType(element, typeName, currentSourceLocation()); // add to resolver

      hasTypeAttribute = true;
      hasTypeSpecified = true;
   }

   if (hasAttribute(QString::fromLatin1("substitutionGroup"))) {
      QList<QXmlName> elementNames;

      const QString value = readAttribute(QString::fromLatin1("substitutionGroup"));
      const QStringList substitutionGroups = value.split(QLatin1Char(' '), QString::SkipEmptyParts);
      if (substitutionGroups.isEmpty()) {
         attributeContentError("substitutionGroup", "element", value, BuiltinTypes::xsQName);
         return element;
      }

      for (int i = 0; i < substitutionGroups.count(); ++i) {
         const QString value = substitutionGroups.at(i).simplified();
         if (!XPathHelper::isQName(value)) {
            attributeContentError("substitutionGroup", "element", value, BuiltinTypes::xsQName);
            return element;
         }

         QXmlName elementName;
         convertName(value, NamespaceSupport::ElementName, elementName); // translate qualified name into QXmlName
         elementNames.append(elementName);
      }

      m_schemaResolver->addSubstitutionGroupAffiliation(element, elementNames, currentSourceLocation()); // add to resolver

      hasSubstitutionGroup = true;
   }

   validateIdAttribute("element");

   XsdAlternative::List alternatives;

   TagValidationHandler tagValidator(XsdTagScope::GlobalElement, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            element->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::SimpleType, token, namespaceToken)) {
            if (hasTypeAttribute) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("element"))
                     .arg(formatElement("simpleType"))
                     .arg(formatAttribute("type")));
               return element;
            }

            const XsdSimpleType::Ptr type = parseLocalSimpleType();
            type->setContext(element);
            element->setType(type);

            // add it to list of anonymous types as well
            addAnonymousType(type);

            hasTypeSpecified = true;
         } else if (isSchemaTag(XsdSchemaToken::ComplexType, token, namespaceToken)) {
            if (hasTypeAttribute) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("element"))
                     .arg(formatElement("complexType"))
                     .arg(formatAttribute("type")));
               return element;
            }

            const XsdComplexType::Ptr type = parseLocalComplexType();
            type->setContext(element);
            element->setType(type);

            // add it to list of anonymous types as well
            addAnonymousType(type);

            hasTypeSpecified = true;
         } else if (isSchemaTag(XsdSchemaToken::Alternative, token, namespaceToken)) {
            const XsdAlternative::Ptr alternative = parseAlternative();
            alternatives.append(alternative);
         } else if (isSchemaTag(XsdSchemaToken::Unique, token, namespaceToken)) {
            const XsdIdentityConstraint::Ptr constraint = parseUnique();
            element->addIdentityConstraint(constraint);
         } else if (isSchemaTag(XsdSchemaToken::Key, token, namespaceToken)) {
            const XsdIdentityConstraint::Ptr constraint = parseKey();
            element->addIdentityConstraint(constraint);
         } else if (isSchemaTag(XsdSchemaToken::Keyref, token, namespaceToken)) {
            const XsdIdentityConstraint::Ptr constraint = parseKeyRef(element);
            element->addIdentityConstraint(constraint);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   if (!hasTypeSpecified) {
      if (hasSubstitutionGroup) {
         m_schemaResolver->addSubstitutionGroupType(element);
      } else {
         element->setType(BuiltinTypes::xsAnyType);
      }
   }

   if (!alternatives.isEmpty()) {
      element->setTypeTable(XsdElement::TypeTable::Ptr(new XsdElement::TypeTable()));

      for (int i = 0; i < alternatives.count(); ++i) {
         if (alternatives.at(i)->test()) {
            element->typeTable()->addAlternative(alternatives.at(i));
         }

         if (i == (alternatives.count() - 1)) { // the final one
            if (!alternatives.at(i)->test()) {
               element->typeTable()->setDefaultTypeDefinition(alternatives.at(i));
            } else {
               const XsdAlternative::Ptr alternative(new XsdAlternative());
               if (element->type()) {
                  alternative->setType(element->type());
               } else {
                  m_schemaResolver->addAlternativeType(alternative, element);   // add to resolver
               }

               element->typeTable()->setDefaultTypeDefinition(alternative);
            }
         }
      }
   }

   return element;
}

XsdTerm::Ptr XsdSchemaParser::parseLocalElement(const XsdParticle::Ptr &particle,
      const NamedSchemaComponent::Ptr &parent)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Element, this);

   validateElement(XsdTagScope::LocalElement);

   bool hasRefAttribute = false;
   bool hasTypeAttribute = false;
   bool hasTypeSpecified = false;

   XsdTerm::Ptr term;
   XsdElement::Ptr element;
   if (hasAttribute(QString::fromLatin1("ref"))) {
      term = XsdReference::Ptr(new XsdReference());
      hasRefAttribute = true;
   } else {
      term = XsdElement::Ptr(new XsdElement());
      element = term;
   }

   if (hasRefAttribute) {
      if (hasAttribute(QString::fromLatin1("name"))) {
         error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
               .arg(formatElement("element"))
               .arg(formatAttribute("ref"))
               .arg(formatAttribute("name")));
         return term;
      } else if (hasAttribute(QString::fromLatin1("block"))) {
         error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
               .arg(formatElement("element"))
               .arg(formatAttribute("ref"))
               .arg(formatAttribute("block")));
         return term;
      } else if (hasAttribute(QString::fromLatin1("nillable"))) {
         error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
               .arg(formatElement("element"))
               .arg(formatAttribute("ref"))
               .arg(formatAttribute("nillable")));
         return term;
      } else if (hasAttribute(QString::fromLatin1("default"))) {
         error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
               .arg(formatElement("element"))
               .arg(formatAttribute("ref"))
               .arg(formatAttribute("default")));
         return term;
      } else if (hasAttribute(QString::fromLatin1("fixed"))) {
         error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
               .arg(formatElement("element"))
               .arg(formatAttribute("ref"))
               .arg(formatAttribute("fixed")));
         return term;
      } else if (hasAttribute(QString::fromLatin1("form"))) {
         error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
               .arg(formatElement("element"))
               .arg(formatAttribute("ref"))
               .arg(formatAttribute("form")));
         return term;
      } else if (hasAttribute(QString::fromLatin1("type"))) {
         error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
               .arg(formatElement("element"))
               .arg(formatAttribute("ref"))
               .arg(formatAttribute("type")));
         return term;
      }
   }

   // parse attributes
   if (!parseMinMaxConstraint(particle, "element")) {
      return element;
   }

   if (!hasAttribute(QString::fromLatin1("name")) && !hasAttribute(QString::fromLatin1("ref"))) {
      error(QtXmlPatterns::tr("%1 element must have either %2 or %3 attribute.")
            .arg(formatElement("element"))
            .arg(formatAttribute("name"))
            .arg(formatAttribute("ref")));
      return element;
   }

   if (hasRefAttribute) {
      const QString ref = readQNameAttribute(QString::fromLatin1("ref"), "element");
      QXmlName referenceName;
      convertName(ref, NamespaceSupport::ElementName, referenceName); // translate qualified name into QXmlName

      const XsdReference::Ptr reference = term;
      reference->setReferenceName(referenceName);
      reference->setType(XsdReference::Element);
      reference->setSourceLocation(currentSourceLocation());
   } else {
      element->setScope(XsdElement::Scope::Ptr(new XsdElement::Scope()));
      element->scope()->setVariety(XsdElement::Scope::Local);
      element->scope()->setParent(parent);

      if (hasAttribute(QString::fromLatin1("name"))) {
         const QString elementName = readNameAttribute("element");

         QXmlName objectName;
         if (hasAttribute(QString::fromLatin1("form"))) {
            const QString value = readAttribute(QString::fromLatin1("form"));
            if (value != QString::fromLatin1("qualified") && value != QString::fromLatin1("unqualified")) {
               attributeContentError("form", "element", value);
               return element;
            }

            if (value == QString::fromLatin1("qualified")) {
               objectName = m_namePool->allocateQName(m_targetNamespace, elementName);
            } else {
               objectName = m_namePool->allocateQName(QString(), elementName);
            }
         } else {
            if (m_elementFormDefault == QString::fromLatin1("qualified")) {
               objectName = m_namePool->allocateQName(m_targetNamespace, elementName);
            } else {
               objectName = m_namePool->allocateQName(QString(), elementName);
            }
         }

         element->setName(objectName);
      }

      if (hasAttribute(QString::fromLatin1("nillable"))) {
         const QString nillable = readAttribute(QString::fromLatin1("nillable"));

         const Boolean::Ptr value = Boolean::fromLexical(nillable);
         if (value->hasError()) {
            attributeContentError("nillable", "element", nillable, BuiltinTypes::xsBoolean);
            return term;
         }

         element->setIsNillable(value->as<Boolean>()->value());
      } else {
         element->setIsNillable(false); // the default value
      }

      if (hasAttribute(QString::fromLatin1("default")) && hasAttribute(QString::fromLatin1("fixed"))) {
         error(QtXmlPatterns::tr("%1 element must not have %2 and %3 attribute together.")
               .arg(formatElement("element"))
               .arg(formatAttribute("default"))
               .arg(formatAttribute("fixed")));
         return element;
      }

      if (hasAttribute(QString::fromLatin1("default"))) {
         const QString value = readAttribute(QString::fromLatin1("default"));
         element->setValueConstraint(XsdElement::ValueConstraint::Ptr(new XsdElement::ValueConstraint()));
         element->valueConstraint()->setVariety(XsdElement::ValueConstraint::Default);
         element->valueConstraint()->setValue(value);
      } else if (hasAttribute(QString::fromLatin1("fixed"))) {
         const QString value = readAttribute(QString::fromLatin1("fixed"));
         element->setValueConstraint(XsdElement::ValueConstraint::Ptr(new XsdElement::ValueConstraint()));
         element->valueConstraint()->setVariety(XsdElement::ValueConstraint::Fixed);
         element->valueConstraint()->setValue(value);
      }

      if (hasAttribute(QString::fromLatin1("type"))) {
         const QString type = readQNameAttribute(QString::fromLatin1("type"), "element");
         QXmlName typeName;
         convertName(type, NamespaceSupport::ElementName, typeName); // translate qualified name into QXmlName
         m_schemaResolver->addElementType(element, typeName, currentSourceLocation()); // add to resolver

         hasTypeAttribute = true;
         hasTypeSpecified = true;
      }

      element->setDisallowedSubstitutions(readBlockingConstraintAttribute(NamedSchemaComponent::ExtensionConstraint |
                                          NamedSchemaComponent::RestrictionConstraint | NamedSchemaComponent::SubstitutionConstraint, "element"));
   }

   validateIdAttribute("element");

   XsdAlternative::List alternatives;

   TagValidationHandler tagValidator(XsdTagScope::LocalElement, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            term->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::SimpleType, token, namespaceToken)) {
            if (hasRefAttribute) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("element"))
                     .arg(formatElement("simpleType"))
                     .arg(formatAttribute("ref")));
               return term;
            } else if (hasTypeAttribute) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("element"))
                     .arg(formatElement("simpleType"))
                     .arg(formatAttribute("type")));
               return term;
            }

            const XsdSimpleType::Ptr type = parseLocalSimpleType();
            type->setContext(element);
            element->setType(type);

            // add it to list of anonymous types as well
            addAnonymousType(type);

            hasTypeSpecified = true;
         } else if (isSchemaTag(XsdSchemaToken::ComplexType, token, namespaceToken)) {
            if (hasRefAttribute) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("element"))
                     .arg(formatElement("complexType"))
                     .arg(formatAttribute("ref")));
               return term;
            } else if (hasTypeAttribute) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("element"))
                     .arg(formatElement("complexType"))
                     .arg(formatAttribute("type")));
               return term;
            }

            const XsdComplexType::Ptr type = parseLocalComplexType();
            type->setContext(element);
            element->setType(type);

            // add it to list of anonymous types as well
            addAnonymousType(type);

            hasTypeSpecified = true;
         } else if (isSchemaTag(XsdSchemaToken::Alternative, token, namespaceToken)) {
            if (hasRefAttribute) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("element"))
                     .arg(formatElement("alternative"))
                     .arg(formatAttribute("ref")));
               return term;
            }

            const XsdAlternative::Ptr alternative = parseAlternative();
            alternatives.append(alternative);
         } else if (isSchemaTag(XsdSchemaToken::Unique, token, namespaceToken)) {
            if (hasRefAttribute) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("element"))
                     .arg(formatElement("unique"))
                     .arg(formatAttribute("ref")));
               return term;
            }

            const XsdIdentityConstraint::Ptr constraint = parseUnique();
            element->addIdentityConstraint(constraint);
         } else if (isSchemaTag(XsdSchemaToken::Key, token, namespaceToken)) {
            if (hasRefAttribute) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("element"))
                     .arg(formatElement("key"))
                     .arg(formatAttribute("ref")));
               return term;
            }

            const XsdIdentityConstraint::Ptr constraint = parseKey();
            element->addIdentityConstraint(constraint);
         } else if (isSchemaTag(XsdSchemaToken::Keyref, token, namespaceToken)) {
            if (hasRefAttribute) {
               error(QtXmlPatterns::tr("%1 element with %2 child element must not have a %3 attribute.")
                     .arg(formatElement("element"))
                     .arg(formatElement("keyref"))
                     .arg(formatAttribute("ref")));
               return term;
            }

            const XsdIdentityConstraint::Ptr constraint = parseKeyRef(element);
            element->addIdentityConstraint(constraint);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   if (!hasTypeSpecified && !hasRefAttribute) {
      element->setType(BuiltinTypes::xsAnyType);
   }

   if (!hasRefAttribute && !alternatives.isEmpty()) {
      element->setTypeTable(XsdElement::TypeTable::Ptr(new XsdElement::TypeTable()));

      for (int i = 0; i < alternatives.count(); ++i) {
         if (alternatives.at(i)->test()) {
            element->typeTable()->addAlternative(alternatives.at(i));
         }

         if (i == (alternatives.count() - 1)) { // the final one
            if (!alternatives.at(i)->test()) {
               element->typeTable()->setDefaultTypeDefinition(alternatives.at(i));
            } else {
               const XsdAlternative::Ptr alternative(new XsdAlternative());
               if (element->type()) {
                  alternative->setType(element->type());
               } else {
                  m_schemaResolver->addAlternativeType(alternative, element);   // add to resolver
               }

               element->typeTable()->setDefaultTypeDefinition(alternative);
            }
         }
      }
   }

   return term;
}

XsdIdentityConstraint::Ptr XsdSchemaParser::parseUnique()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Unique, this);

   validateElement(XsdTagScope::Unique);

   const XsdIdentityConstraint::Ptr constraint(new XsdIdentityConstraint());
   constraint->setCategory(XsdIdentityConstraint::Unique);

   // parse attributes
   const QXmlName objectName = m_namePool->allocateQName(m_targetNamespace, readNameAttribute("unique"));
   constraint->setName(objectName);

   validateIdAttribute("unique");

   TagValidationHandler tagValidator(XsdTagScope::Unique, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            constraint->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Selector, token, namespaceToken)) {
            parseSelector(constraint);
         } else if (isSchemaTag(XsdSchemaToken::Field, token, namespaceToken)) {
            parseField(constraint);
         } else {
            parseUnknown();
         }
      }
   }

   // add constraint to schema for further checking
   addIdentityConstraint(constraint);

   tagValidator.finalize();

   return constraint;
}

XsdIdentityConstraint::Ptr XsdSchemaParser::parseKey()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Key, this);

   validateElement(XsdTagScope::Key);

   const XsdIdentityConstraint::Ptr constraint(new XsdIdentityConstraint());
   constraint->setCategory(XsdIdentityConstraint::Key);

   // parse attributes
   const QXmlName objectName = m_namePool->allocateQName(m_targetNamespace, readNameAttribute("key"));
   constraint->setName(objectName);

   validateIdAttribute("key");

   TagValidationHandler tagValidator(XsdTagScope::Key, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            constraint->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Selector, token, namespaceToken)) {
            parseSelector(constraint);
         } else if (isSchemaTag(XsdSchemaToken::Field, token, namespaceToken)) {
            parseField(constraint);
         } else {
            parseUnknown();
         }
      }
   }

   // add constraint to schema for further checking
   addIdentityConstraint(constraint);

   tagValidator.finalize();

   return constraint;
}

XsdIdentityConstraint::Ptr XsdSchemaParser::parseKeyRef(const XsdElement::Ptr &element)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Keyref, this);

   validateElement(XsdTagScope::KeyRef);

   const XsdIdentityConstraint::Ptr constraint(new XsdIdentityConstraint());
   constraint->setCategory(XsdIdentityConstraint::KeyReference);

   // parse attributes
   const QXmlName objectName = m_namePool->allocateQName(m_targetNamespace, readNameAttribute("keyref"));
   constraint->setName(objectName);

   const QString refer = readQNameAttribute(QString::fromLatin1("refer"), "keyref");
   QXmlName referenceName;
   convertName(refer, NamespaceSupport::ElementName, referenceName); // translate qualified name into QXmlName
   m_schemaResolver->addKeyReference(element, constraint, referenceName, currentSourceLocation()); // add to resolver

   validateIdAttribute("keyref");

   TagValidationHandler tagValidator(XsdTagScope::KeyRef, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            constraint->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::Selector, token, namespaceToken)) {
            parseSelector(constraint);
         } else if (isSchemaTag(XsdSchemaToken::Field, token, namespaceToken)) {
            parseField(constraint);
         } else {
            parseUnknown();
         }
      }
   }

   // add constraint to schema for further checking
   addIdentityConstraint(constraint);

   tagValidator.finalize();

   return constraint;
}

void XsdSchemaParser::parseSelector(const XsdIdentityConstraint::Ptr &ptr)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Selector, this);

   validateElement(XsdTagScope::Selector);

   // parse attributes
   const XsdXPathExpression::Ptr expression = readXPathExpression("selector");

   const QString xpath = readXPathAttribute(QString::fromLatin1("xpath"), XPathSelector, "selector");
   expression->setExpression(xpath);

   ptr->setSelector(expression);

   validateIdAttribute("selector");

   TagValidationHandler tagValidator(XsdTagScope::Selector, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            expression->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();
}

void XsdSchemaParser::parseField(const XsdIdentityConstraint::Ptr &ptr)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Field, this);

   validateElement(XsdTagScope::Field);

   // parse attributes
   const XsdXPathExpression::Ptr expression = readXPathExpression("field");

   const QString xpath = readXPathAttribute(QString::fromLatin1("xpath"), XPathField, "field");
   expression->setExpression(xpath);

   ptr->addField(expression);

   validateIdAttribute("field");

   TagValidationHandler tagValidator(XsdTagScope::Field, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            expression->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();
}

XsdAlternative::Ptr XsdSchemaParser::parseAlternative()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Alternative, this);

   validateElement(XsdTagScope::Alternative);

   const XsdAlternative::Ptr alternative(new XsdAlternative());

   bool hasTypeSpecified = false;

   if (hasAttribute(QString::fromLatin1("test"))) {
      const XsdXPathExpression::Ptr expression = readXPathExpression("alternative");

      const QString test = readXPathAttribute(QString::fromLatin1("test"), XPath20, "alternative");
      expression->setExpression(test);

      alternative->setTest(expression);
   }

   if (hasAttribute(QString::fromLatin1("type"))) {
      const QString type = readQNameAttribute(QString::fromLatin1("type"), "alternative");
      QXmlName typeName;
      convertName(type, NamespaceSupport::ElementName, typeName); // translate qualified name into QXmlName
      m_schemaResolver->addAlternativeType(alternative, typeName, currentSourceLocation()); // add to resolver

      hasTypeSpecified = true;
   }

   validateIdAttribute("alternative");

   TagValidationHandler tagValidator(XsdTagScope::Alternative, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            alternative->addAnnotation(annotation);
         } else if (isSchemaTag(XsdSchemaToken::SimpleType, token, namespaceToken)) {
            const XsdSimpleType::Ptr type = parseLocalSimpleType();
            alternative->setType(type);

            // add it to list of anonymous types as well
            addAnonymousType(type);

            hasTypeSpecified = true;
         } else if (isSchemaTag(XsdSchemaToken::ComplexType, token, namespaceToken)) {
            const XsdComplexType::Ptr type = parseLocalComplexType();
            alternative->setType(type);

            // add it to list of anonymous types as well
            addAnonymousType(type);

            hasTypeSpecified = true;
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   if (!hasTypeSpecified) {
      error(QtXmlPatterns::tr("%1 element must have either %2 attribute or %3 or %4 as child element.")
            .arg(formatElement("alternative"))
            .arg(formatAttribute("type"))
            .arg(formatElement("simpleType"))
            .arg(formatElement("complexType")));
      return alternative;
   }

   return alternative;
}

XsdNotation::Ptr XsdSchemaParser::parseNotation()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Notation, this);

   validateElement(XsdTagScope::Notation);

   const XsdNotation::Ptr notation(new XsdNotation());

   // parse attributes
   const QXmlName objectName = m_namePool->allocateQName(m_targetNamespace, readNameAttribute("notation"));
   notation->setName(objectName);

   bool hasOptionalAttribute = false;

   if (hasAttribute(QString::fromLatin1("public"))) {
      const QString value = readAttribute(QString::fromLatin1("public"));
      if (!value.isEmpty()) {
         const DerivedString<TypeToken>::Ptr publicId = DerivedString<TypeToken>::fromLexical(NamePool::Ptr(m_namePool), value);
         if (publicId->hasError()) {
            attributeContentError("public", "notation", value, BuiltinTypes::xsToken);
            return notation;
         }
         notation->setPublicId(publicId);
      }

      hasOptionalAttribute = true;
   }

   if (hasAttribute(QString::fromLatin1("system"))) {
      const QString value = readAttribute(QString::fromLatin1("system"));
      if (!isValidUri(value)) {
         attributeContentError("system", "notation", value, BuiltinTypes::xsAnyURI);
         return notation;
      }

      if (!value.isEmpty()) {
         const AnyURI::Ptr systemId = AnyURI::fromLexical(value);
         notation->setSystemId(systemId);
      }

      hasOptionalAttribute = true;
   }

   if (!hasOptionalAttribute) {
      error(QtXmlPatterns::tr("%1 element requires either %2 or %3 attribute.")
            .arg(formatElement("notation"))
            .arg(formatAttribute("public"))
            .arg(formatAttribute("system")));
      return notation;
   }

   validateIdAttribute("notation");

   TagValidationHandler tagValidator(XsdTagScope::Notation, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isCharacters() || isEntityReference()) {
         if (!text().toString().trimmed().isEmpty()) {
            error(QtXmlPatterns::tr("Text or entity references not allowed inside %1 element").arg(formatElement("notation.")));
            return notation;
         }
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            notation->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return notation;
}

XsdWildcard::Ptr XsdSchemaParser::parseAny(const XsdParticle::Ptr &particle)
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::Any, this);

   validateElement(XsdTagScope::Any);

   const XsdWildcard::Ptr wildcard(new XsdWildcard());

   // parse attributes
   if (!parseMinMaxConstraint(particle, "any")) {
      return wildcard;
   }

   if (hasAttribute(QString::fromLatin1("namespace"))) {
      const QSet<QString> values = readAttribute(QString::fromLatin1("namespace")).split(QLatin1Char(' '),
                                   QString::SkipEmptyParts).toSet();
      if ((values.contains(QString::fromLatin1("##any")) || values.contains(QString::fromLatin1("##other"))) &&
            values.count() != 1) {
         error(QtXmlPatterns::tr("%1 attribute of %2 element must contain %3, %4 or a list of URIs.")
               .arg(formatAttribute("namespace"))
               .arg(formatElement("any"))
               .arg(formatData("##any"))
               .arg(formatData("##other")));
         return wildcard;
      }

      if (values.contains(QString::fromLatin1("##any"))) {
         wildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Any);
      } else if (values.contains(QString::fromLatin1("##other"))) {
         wildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Not);
         if (!m_targetNamespace.isEmpty()) {
            wildcard->namespaceConstraint()->setNamespaces(QSet<QString>() << m_targetNamespace);
         } else {
            wildcard->namespaceConstraint()->setNamespaces(QSet<QString>() << XsdWildcard::absentNamespace());
         }
      } else {
         wildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Enumeration);
         QStringList newValues = values.toList();

         // replace the ##targetNamespace entry
         for (int i = 0; i < newValues.count(); ++i) {
            if (newValues.at(i) == QString::fromLatin1("##targetNamespace")) {
               if (!m_targetNamespace.isEmpty()) {
                  newValues[i] = m_targetNamespace;
               } else {
                  newValues[i] = XsdWildcard::absentNamespace();
               }
            } else if (newValues.at(i) == QString::fromLatin1("##local")) {
               newValues[i] = XsdWildcard::absentNamespace();
            }
         }

         // check for invalid URIs
         for (int i = 0; i < newValues.count(); ++i) {
            const QString stringValue = newValues.at(i);
            if (stringValue == XsdWildcard::absentNamespace()) {
               continue;
            }

            if (!isValidUri(stringValue)) {
               attributeContentError("namespace", "any", stringValue, BuiltinTypes::xsAnyURI);
               return wildcard;
            }
         }

         wildcard->namespaceConstraint()->setNamespaces(newValues.toSet());
      }
   } else {
      wildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Any);
   }

   if (hasAttribute(QString::fromLatin1("processContents"))) {
      const QString value = readAttribute(QString::fromLatin1("processContents"));
      if (value != QString::fromLatin1("lax") &&
            value != QString::fromLatin1("skip") &&
            value != QString::fromLatin1("strict")) {
         attributeContentError("processContents", "any", value);
         return wildcard;
      }

      if (value == QString::fromLatin1("lax")) {
         wildcard->setProcessContents(XsdWildcard::Lax);
      } else if (value == QString::fromLatin1("skip")) {
         wildcard->setProcessContents(XsdWildcard::Skip);
      } else if (value == QString::fromLatin1("strict")) {
         wildcard->setProcessContents(XsdWildcard::Strict);
      }
   } else {
      wildcard->setProcessContents(XsdWildcard::Strict);
   }

   validateIdAttribute("any");

   TagValidationHandler tagValidator(XsdTagScope::Any, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            wildcard->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return wildcard;
}

XsdWildcard::Ptr XsdSchemaParser::parseAnyAttribute()
{
   const ElementNamespaceHandler namespaceHandler(XsdSchemaToken::AnyAttribute, this);

   validateElement(XsdTagScope::AnyAttribute);

   const XsdWildcard::Ptr wildcard(new XsdWildcard());

   // parse attributes
   if (hasAttribute(QString::fromLatin1("namespace"))) {
      const QSet<QString> values = readAttribute(QString::fromLatin1("namespace")).split(QLatin1Char(' '),
                                   QString::SkipEmptyParts).toSet();
      if ((values.contains(QString::fromLatin1("##any")) || values.contains(QString::fromLatin1("##other"))) &&
            values.count() != 1) {
         error(QtXmlPatterns::tr("%1 attribute of %2 element must contain %3, %4 or a list of URIs.")
               .arg(formatAttribute("namespace"))
               .arg(formatElement("anyAttribute"))
               .arg(formatData("##any"))
               .arg(formatData("##other")));
         return wildcard;
      }

      if (values.contains(QString::fromLatin1("##any"))) {
         wildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Any);
      } else if (values.contains(QString::fromLatin1("##other"))) {
         wildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Not);
         if (!m_targetNamespace.isEmpty()) {
            wildcard->namespaceConstraint()->setNamespaces(QSet<QString>() << m_targetNamespace);
         } else {
            wildcard->namespaceConstraint()->setNamespaces(QSet<QString>() << XsdWildcard::absentNamespace());
         }
      } else {
         wildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Enumeration);
         QStringList newValues = values.toList();

         // replace the ##targetNamespace entry
         for (int i = 0; i < newValues.count(); ++i) {
            if (newValues.at(i) == QString::fromLatin1("##targetNamespace")) {
               if (!m_targetNamespace.isEmpty()) {
                  newValues[i] = m_targetNamespace;
               } else {
                  newValues[i] = XsdWildcard::absentNamespace();
               }
            } else if (newValues.at(i) == QString::fromLatin1("##local")) {
               newValues[i] = XsdWildcard::absentNamespace();
            }
         }

         // check for invalid URIs
         for (int i = 0; i < newValues.count(); ++i) {
            const QString stringValue = newValues.at(i);
            if (stringValue == XsdWildcard::absentNamespace()) {
               continue;
            }

            if (!isValidUri(stringValue)) {
               attributeContentError("namespace", "anyAttribute", stringValue, BuiltinTypes::xsAnyURI);
               return wildcard;
            }
         }

         wildcard->namespaceConstraint()->setNamespaces(newValues.toSet());
      }
   } else {
      wildcard->namespaceConstraint()->setVariety(XsdWildcard::NamespaceConstraint::Any);
   }

   if (hasAttribute(QString::fromLatin1("processContents"))) {
      const QString value = readAttribute(QString::fromLatin1("processContents"));
      if (value != QString::fromLatin1("lax") &&
            value != QString::fromLatin1("skip") &&
            value != QString::fromLatin1("strict")) {
         attributeContentError("processContents", "anyAttribute", value);
         return wildcard;
      }

      if (value == QString::fromLatin1("lax")) {
         wildcard->setProcessContents(XsdWildcard::Lax);
      } else if (value == QString::fromLatin1("skip")) {
         wildcard->setProcessContents(XsdWildcard::Skip);
      } else if (value == QString::fromLatin1("strict")) {
         wildcard->setProcessContents(XsdWildcard::Strict);
      }
   } else {
      wildcard->setProcessContents(XsdWildcard::Strict);
   }

   validateIdAttribute("anyAttribute");

   TagValidationHandler tagValidator(XsdTagScope::AnyAttribute, this, NamePool::Ptr(m_namePool));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         const XsdSchemaToken::NodeName token = XsdSchemaToken::toToken(name());
         const XsdSchemaToken::NodeName namespaceToken = XsdSchemaToken::toToken(namespaceUri());

         tagValidator.validate(token);

         if (isSchemaTag(XsdSchemaToken::Annotation, token, namespaceToken)) {
            const XsdAnnotation::Ptr annotation = parseAnnotation();
            wildcard->addAnnotation(annotation);
         } else {
            parseUnknown();
         }
      }
   }

   tagValidator.finalize();

   return wildcard;
}


void XsdSchemaParser::parseUnknownDocumentation()
{
   Q_ASSERT(isStartElement());
   m_namespaceSupport.pushContext();
   m_namespaceSupport.setPrefixes(namespaceDeclarations());

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         parseUnknownDocumentation();
      }
   }

   m_namespaceSupport.popContext();
}

void XsdSchemaParser::parseUnknown()
{
   Q_ASSERT(isStartElement());
   m_namespaceSupport.pushContext();
   m_namespaceSupport.setPrefixes(namespaceDeclarations());

   error(QtXmlPatterns::tr("%1 element is not allowed in this context.").arg(formatElement(name().toString())));

   while (!atEnd()) {
      readNext();

      if (isEndElement()) {
         break;
      }

      if (isStartElement()) {
         parseUnknown();
      }
   }

   m_namespaceSupport.popContext();
}

bool XsdSchemaParser::parseMinMaxConstraint(const XsdParticle::Ptr &particle, const char *elementName)
{
   if (hasAttribute(QString::fromLatin1("minOccurs"))) {
      const QString value = readAttribute(QString::fromLatin1("minOccurs"));

      DerivedInteger<TypeNonNegativeInteger>::Ptr integer = DerivedInteger<TypeNonNegativeInteger>::fromLexical(NamePool::Ptr(
               m_namePool), value);
      if (integer->hasError()) {
         attributeContentError("minOccurs", elementName, value, BuiltinTypes::xsNonNegativeInteger);
         return false;
      } else {
         particle->setMinimumOccurs(integer->as< DerivedInteger<TypeNonNegativeInteger> >()->storedValue());
      }
   } else {
      particle->setMinimumOccurs(1);
   }

   if (hasAttribute(QString::fromLatin1("maxOccurs"))) {
      const QString value = readAttribute(QString::fromLatin1("maxOccurs"));

      if (value == QString::fromLatin1("unbounded")) {
         particle->setMaximumOccursUnbounded(true);
      } else {
         particle->setMaximumOccursUnbounded(false);
         DerivedInteger<TypeNonNegativeInteger>::Ptr integer = DerivedInteger<TypeNonNegativeInteger>::fromLexical(NamePool::Ptr(
                  m_namePool), value);
         if (integer->hasError()) {
            attributeContentError("maxOccurs", elementName, value, BuiltinTypes::xsNonNegativeInteger);
            return false;
         } else {
            particle->setMaximumOccurs(integer->as< DerivedInteger<TypeNonNegativeInteger> >()->storedValue());
         }
      }
   } else {
      particle->setMaximumOccursUnbounded(false);
      particle->setMaximumOccurs(1);
   }

   if (!particle->maximumOccursUnbounded()) {
      if (particle->maximumOccurs() < particle->minimumOccurs()) {
         error(QtXmlPatterns::tr("%1 attribute of %2 element has larger value than %3 attribute.")
               .arg(formatAttribute("minOccurs"))
               .arg(formatElement(elementName))
               .arg(formatAttribute("maxOccurs")));
         return false;
      }
   }

   return true;
}

QSourceLocation XsdSchemaParser::currentSourceLocation() const
{
   QSourceLocation location;
   location.setLine(lineNumber());
   location.setColumn(columnNumber());
   location.setUri(m_documentURI);

   return location;
}

void XsdSchemaParser::convertName(const QString &qualifiedName, NamespaceSupport::NameType type, QXmlName &name)
{
   bool result = m_namespaceSupport.processName(qualifiedName, type, name);
   if (!result) {
      error(QtXmlPatterns::tr("Prefix of qualified name %1 is not defined.").arg(formatKeyword(qualifiedName)));
   }
}

QString XsdSchemaParser::readNameAttribute(const char *elementName)
{
   const QString value = readAttribute(QString::fromLatin1("name")).simplified();
   if (!QXmlUtils::isNCName(value)) {
      attributeContentError("name", elementName, value, BuiltinTypes::xsNCName);
      return QString();
   } else {
      return value;
   }
}

QString XsdSchemaParser::readQNameAttribute(const QString &typeAttribute, const char *elementName)
{
   const QString value = readAttribute(typeAttribute).simplified();
   if (! XPathHelper::isQName(value)) {
      attributeContentError(csPrintable(typeAttribute), elementName, value, BuiltinTypes::xsQName);
      return QString();
   } else {
      return value;
   }
}

QString XsdSchemaParser::readNamespaceAttribute(const QString &attributeName, const char *elementName)
{
   const QString value = readAttribute(attributeName);
   if (value.isEmpty()) {
      attributeContentError(csPrintable(attributeName), elementName, value, BuiltinTypes::xsAnyURI);
      return QString();
   }

   return value;
}

SchemaType::DerivationConstraints XsdSchemaParser::readDerivationConstraintAttribute(
   const SchemaType::DerivationConstraints &allowedConstraints, const char *elementName)
{
   // first convert the flags into strings for easier comparison
   QSet<QString> allowedContent;
   if (allowedConstraints & SchemaType::RestrictionConstraint) {
      allowedContent.insert(QString::fromLatin1("restriction"));
   }
   if (allowedConstraints & SchemaType::ExtensionConstraint) {
      allowedContent.insert(QString::fromLatin1("extension"));
   }
   if (allowedConstraints & SchemaType::ListConstraint) {
      allowedContent.insert(QString::fromLatin1("list"));
   }
   if (allowedConstraints & SchemaType::UnionConstraint) {
      allowedContent.insert(QString::fromLatin1("union"));
   }

   // read content from the attribute if available, otherwise use the default definitions from the schema tag
   QString content;
   if (hasAttribute(QString::fromLatin1("final"))) {
      content = readAttribute(QString::fromLatin1("final"));

      // split string into list to validate the content of the attribute
      const QStringList values = content.split(QLatin1Char(' '), QString::SkipEmptyParts);
      for (int i = 0; i < values.count(); i++) {
         const QString value = values.at(i);
         if (!allowedContent.contains(value) && (value != QString::fromLatin1("#all"))) {
            attributeContentError("final", elementName, value);
            return SchemaType::DerivationConstraints();
         }

         if ((value == QString::fromLatin1("#all")) && values.count() != 1) {
            error(QtXmlPatterns::tr("%1 attribute of %2 element must either contain %3 or the other values.")
                  .arg(formatAttribute("final"))
                  .arg(formatElement(elementName))
                  .arg(formatData("#all")));
            return SchemaType::DerivationConstraints();
         }
      }
   } else {
      // content of the default value has been validated in parseSchema already
      content = m_finalDefault;
   }

   QSet<QString> contentSet = content.split(QLatin1Char(' '), QString::SkipEmptyParts).toSet();

   // if the '#all' tag is defined, we return all allowed values
   if (contentSet.contains(QString::fromLatin1("#all"))) {
      return allowedConstraints;
   } else { // return the values from content set that intersects with the allowed values
      contentSet.intersect(allowedContent);

      SchemaType::DerivationConstraints constraints;

      if (contentSet.contains(QString::fromLatin1("restriction"))) {
         constraints |= SchemaType::RestrictionConstraint;
      }
      if (contentSet.contains(QString::fromLatin1("extension"))) {
         constraints |= SchemaType::ExtensionConstraint;
      }
      if (contentSet.contains(QString::fromLatin1("list"))) {
         constraints |= SchemaType::ListConstraint;
      }
      if (contentSet.contains(QString::fromLatin1("union"))) {
         constraints |= SchemaType::UnionConstraint;
      }

      return constraints;
   }
}

NamedSchemaComponent::BlockingConstraints XsdSchemaParser::readBlockingConstraintAttribute(
   const NamedSchemaComponent::BlockingConstraints &allowedConstraints, const char *elementName)
{
   // first convert the flags into strings for easier comparison
   QSet<QString> allowedContent;
   if (allowedConstraints & NamedSchemaComponent::RestrictionConstraint) {
      allowedContent.insert(QString::fromLatin1("restriction"));
   }
   if (allowedConstraints & NamedSchemaComponent::ExtensionConstraint) {
      allowedContent.insert(QString::fromLatin1("extension"));
   }
   if (allowedConstraints & NamedSchemaComponent::SubstitutionConstraint) {
      allowedContent.insert(QString::fromLatin1("substitution"));
   }

   // read content from the attribute if available, otherwise use the default definitions from the schema tag
   QString content;
   if (hasAttribute(QString::fromLatin1("block"))) {
      content = readAttribute(QString::fromLatin1("block"));

      // split string into list to validate the content of the attribute
      const QStringList values = content.split(QLatin1Char(' '), QString::SkipEmptyParts);
      for (int i = 0; i < values.count(); i++) {
         const QString value = values.at(i);
         if (!allowedContent.contains(value) && (value != QString::fromLatin1("#all"))) {
            attributeContentError("block", elementName, value);
            return NamedSchemaComponent::BlockingConstraints();
         }

         if ((value == QString::fromLatin1("#all")) && values.count() != 1) {
            error(QtXmlPatterns::tr("%1 attribute of %2 element must either contain %3 or the other values.")
                  .arg(formatAttribute("block"))
                  .arg(formatElement(elementName))
                  .arg(formatData("#all")));
            return NamedSchemaComponent::BlockingConstraints();
         }
      }
   } else {
      // content of the default value has been validated in parseSchema already
      content = m_blockDefault;
   }

   QSet<QString> contentSet = content.split(QLatin1Char(' '), QString::SkipEmptyParts).toSet();

   // if the '#all' tag is defined, we return all allowed values
   if (contentSet.contains(QString::fromLatin1("#all"))) {
      return allowedConstraints;
   } else { // return the values from content set that intersects with the allowed values
      contentSet.intersect(allowedContent);

      NamedSchemaComponent::BlockingConstraints constraints;

      if (contentSet.contains(QString::fromLatin1("restriction"))) {
         constraints |= NamedSchemaComponent::RestrictionConstraint;
      }
      if (contentSet.contains(QString::fromLatin1("extension"))) {
         constraints |= NamedSchemaComponent::ExtensionConstraint;
      }
      if (contentSet.contains(QString::fromLatin1("substitution"))) {
         constraints |= NamedSchemaComponent::SubstitutionConstraint;
      }

      return constraints;
   }
}

XsdXPathExpression::Ptr XsdSchemaParser::readXPathExpression(const char *elementName)
{
   const XsdXPathExpression::Ptr expression(new XsdXPathExpression());

   const QList<QXmlName> namespaceBindings = m_namespaceSupport.namespaceBindings();
   QXmlName emptyName;
   for (int i = 0; i < namespaceBindings.count(); ++i) {
      if (namespaceBindings.at(i).prefix() == StandardPrefixes::empty) {
         emptyName = namespaceBindings.at(i);
      }
   }

   expression->setNamespaceBindings(namespaceBindings);

   QString xpathDefaultNamespace;
   if (hasAttribute(QString::fromLatin1("xpathDefaultNamespace"))) {
      xpathDefaultNamespace = readAttribute(QString::fromLatin1("xpathDefaultNamespace"));
      if (xpathDefaultNamespace != QString::fromLatin1("##defaultNamespace") &&
            xpathDefaultNamespace != QString::fromLatin1("##targetNamespace") &&
            xpathDefaultNamespace != QString::fromLatin1("##local")) {
         if (!isValidUri(xpathDefaultNamespace)) {
            attributeContentError("xpathDefaultNamespace", elementName, xpathDefaultNamespace, BuiltinTypes::xsAnyURI);
            return expression;
         }
      }
   } else {
      xpathDefaultNamespace = m_xpathDefaultNamespace;
   }

   AnyURI::Ptr namespaceURI;
   if (xpathDefaultNamespace == QString::fromLatin1("##defaultNamespace")) {
      if (!emptyName.isNull()) {
         namespaceURI = AnyURI::fromLexical(m_namePool->stringForNamespace(emptyName.namespaceURI()));
      }
   } else if (xpathDefaultNamespace == QString::fromLatin1("##targetNamespace")) {
      if (!m_targetNamespace.isEmpty()) {
         namespaceURI = AnyURI::fromLexical(m_targetNamespace);
      }

   } else if (xpathDefaultNamespace == QString::fromLatin1("##local")) {
      // it is absent
   } else {
      namespaceURI = AnyURI::fromLexical(xpathDefaultNamespace);
   }
   if (namespaceURI) {
      if (namespaceURI->hasError()) {
         attributeContentError("xpathDefaultNamespace", elementName, xpathDefaultNamespace, BuiltinTypes::xsAnyURI);
         return expression;
      }

      expression->setDefaultNamespace(namespaceURI);
   }

   //TODO: read the base uri if qmaintaining reader support it

   return expression;
}

QString XsdSchemaParser::readXPathAttribute(const QString &attributeName, XPathType type,  const char *elementName)
{
   const QString value = readAttribute(attributeName);

   if (value.isEmpty() || value.startsWith('/')) {
      attributeContentError(csPrintable(attributeName), elementName, value);
      return QString();
   }

   QXmlNamePool namePool(NamePool::Ptr(m_namePool).data());

   QXmlQuery::QueryLanguage language = QXmlQuery::XPath20;
   switch (type) {
      case XPath20:
         language = QXmlQuery::XPath20;
         break;
      case XPathSelector:
         language = QXmlQuery::XmlSchema11IdentityConstraintSelector;
         break;
      case XPathField:
         language = QXmlQuery::XmlSchema11IdentityConstraintField;
         break;
   };

   QXmlQuery query(language, namePool);
   QXmlQueryPrivate *queryPrivate = query.d;

   const QList<QXmlName> namespaceBindings = m_namespaceSupport.namespaceBindings();
   for (int i = 0; i < namespaceBindings.count(); ++i) {
      if (namespaceBindings.at(i).prefix() != StandardPrefixes::empty) {
         queryPrivate->addAdditionalNamespaceBinding(namespaceBindings.at(i));
      }
   }

   query.setQuery(value, m_documentURI);
   if (!query.isValid()) {
      attributeContentError(csPrintable(attributeName), elementName, value);
      return QString();
   }

   return value;
}

void XsdSchemaParser::validateIdAttribute(const char *elementName)
{
   if (hasAttribute(QString::fromLatin1("id"))) {
      const QString value = readAttribute(QString::fromLatin1("id"));
      DerivedString<TypeID>::Ptr id = DerivedString<TypeID>::fromLexical(NamePool::Ptr(m_namePool), value);

      if (id->hasError()) {
         attributeContentError("id", elementName, value, BuiltinTypes::xsID);
      } else {
         if (m_idCache->hasId(value)) {
            error(QtXmlPatterns::tr("Component with ID %1 has been defined previously.").arg(formatData(value)));
         } else {
            m_idCache->addId(value);
         }
      }
   }
}

bool XsdSchemaParser::isSchemaTag(XsdSchemaToken::NodeName tag, XsdSchemaToken::NodeName token,
                                  XsdSchemaToken::NodeName namespaceToken) const
{
   return ((tag == token) && (namespaceToken == XsdSchemaToken::XML_NS_SCHEMA_URI));
}

void XsdSchemaParser::addElement(const XsdElement::Ptr &element)
{
   const QXmlName objectName = element->name(NamePool::Ptr(m_namePool));
   if (m_schema->element(objectName)) {
      error(QtXmlPatterns::tr("Element %1 already defined.").arg(formatElement(m_namePool->displayName(objectName))));
   } else {
      m_schema->addElement(element);
      m_componentLocationHash.insert(element, currentSourceLocation());
   }
}

void XsdSchemaParser::addAttribute(const XsdAttribute::Ptr &attribute)
{
   const QXmlName objectName = attribute->name(NamePool::Ptr(m_namePool));
   if (m_schema->attribute(objectName)) {
      error(QtXmlPatterns::tr("Attribute %1 already defined.").arg(formatAttribute(m_namePool->displayName(objectName))));
   } else {
      m_schema->addAttribute(attribute);
      m_componentLocationHash.insert(attribute, currentSourceLocation());
   }
}

void XsdSchemaParser::addType(const SchemaType::Ptr &type)
{
   // we don't import redefinitions of builtin types, that just causes problems
   if (m_builtinTypeNames.contains(type->name(NamePool::Ptr(m_namePool)))) {
      return;
   }

   const QXmlName objectName = type->name(NamePool::Ptr(m_namePool));
   if (m_schema->type(objectName)) {
      error(QtXmlPatterns::tr("Type %1 already defined.").arg(formatType(NamePool::Ptr(m_namePool), objectName)));
   } else {
      m_schema->addType(type);
      if (type->isSimpleType()) {
         m_componentLocationHash.insert(XsdSimpleType::Ptr(type), currentSourceLocation());
      } else {
         m_componentLocationHash.insert(XsdComplexType::Ptr(type), currentSourceLocation());
      }
   }
}

void XsdSchemaParser::addAnonymousType(const SchemaType::Ptr &type)
{
   m_schema->addAnonymousType(type);
   if (type->isSimpleType()) {
      m_componentLocationHash.insert(XsdSimpleType::Ptr(type), currentSourceLocation());
   } else {
      m_componentLocationHash.insert(XsdComplexType::Ptr(type), currentSourceLocation());
   }
}

void XsdSchemaParser::addAttributeGroup(const XsdAttributeGroup::Ptr &group)
{
   const QXmlName objectName = group->name(NamePool::Ptr(m_namePool));
   if (m_schema->attributeGroup(objectName)) {
      error(QtXmlPatterns::tr("Attribute group %1 already defined.").arg(formatKeyword(NamePool::Ptr(m_namePool),
            objectName)));
   } else {
      m_schema->addAttributeGroup(group);
      m_componentLocationHash.insert(group, currentSourceLocation());
   }
}

void XsdSchemaParser::addElementGroup(const XsdModelGroup::Ptr &group)
{
   const QXmlName objectName = group->name(NamePool::Ptr(m_namePool));
   if (m_schema->elementGroup(objectName)) {
      error(QtXmlPatterns::tr("Element group %1 already defined.").arg(formatKeyword(NamePool::Ptr(m_namePool), objectName)));
   } else {
      m_schema->addElementGroup(group);
      m_componentLocationHash.insert(group, currentSourceLocation());
   }
}

void XsdSchemaParser::addNotation(const XsdNotation::Ptr &notation)
{
   const QXmlName objectName = notation->name(NamePool::Ptr(m_namePool));
   if (m_schema->notation(objectName)) {
      error(QtXmlPatterns::tr("Notation %1 already defined.").arg(formatKeyword(NamePool::Ptr(m_namePool), objectName)));
   } else {
      m_schema->addNotation(notation);
      m_componentLocationHash.insert(notation, currentSourceLocation());
   }
}

void XsdSchemaParser::addIdentityConstraint(const XsdIdentityConstraint::Ptr &constraint)
{
   const QXmlName objectName = constraint->name(NamePool::Ptr(m_namePool));
   if (m_schema->identityConstraint(objectName)) {
      error(QtXmlPatterns::tr("Identity constraint %1 already defined.").arg(formatKeyword(NamePool::Ptr(m_namePool),
            objectName)));
   } else {
      m_schema->addIdentityConstraint(constraint);
      m_componentLocationHash.insert(constraint, currentSourceLocation());
   }
}

void XsdSchemaParser::addFacet(const XsdFacet::Ptr &facet, XsdFacet::Hash &facets, const SchemaType::Ptr &type)
{
   // @see http://www.w3.org/TR/xmlschema-2/#src-single-facet-value
   if (facets.contains(facet->type())) {
      error(QtXmlPatterns::tr("Duplicated facets in simple type %1.").arg(formatType(NamePool::Ptr(m_namePool), type)));
      return;
   }

   facets.insert(facet->type(), facet);
}

QT_END_NAMESPACE
