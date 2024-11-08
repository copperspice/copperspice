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

#ifndef QXsdSchemaParser_P_H
#define QXsdSchemaParser_P_H

#include <qnamespacesupport_p.h>
#include <qxsdalternative_p.h>
#include <qxsdattribute_p.h>
#include <qxsdattributegroup_p.h>
#include <qxsdattributeterm_p.h>
#include <qxsdcomplextype_p.h>
#include <qxsdelement_p.h>
#include <qxsdidcache_p.h>
#include <qxsdmodelgroup_p.h>
#include <qxsdnotation_p.h>
#include <qxsdsimpletype_p.h>
#include <qxsdschemacontext_p.h>
#include <qxsdschemaparsercontext_p.h>
#include <qxsdstatemachine_p.h>

#include <QHash>
#include <QSet>
#include <QUrl>
#include <QXmlStreamReader>
#include <QXmlNamePool>

namespace QPatternist {
class XsdSchemaParser : public MaintainingReader<XsdSchemaToken, XsdTagScope::Type>
{
   friend class ElementNamespaceHandler;
   friend class TagValidationHandler;

 public:
   enum ParserType {
      TopLevelParser,
      IncludeParser,
      ImportParser,
      RedefineParser
   };

   XsdSchemaParser(const XsdSchemaContext::Ptr &context, const XsdSchemaParserContext::Ptr &parserContext, QIODevice *device);

   bool parse(ParserType parserType = TopLevelParser);

   typedef QSet<QUrl> NamespaceSet;

   void addIncludedSchemas(const NamespaceSet &schemas);
   void setIncludedSchemas(const NamespaceSet &schemas);

   void addImportedSchemas(const NamespaceSet &schemas);
   void setImportedSchemas(const NamespaceSet &schemas);

   void addRedefinedSchemas(const NamespaceSet &schemas);
   void setRedefinedSchemas(const NamespaceSet &schemas);

   void setTargetNamespace(const QString &targetNamespace);

   void setDocumentURI(const QUrl &uri);
   QUrl documentURI() const override;

   bool isAnyAttributeAllowed() const override;

 private:
   virtual void error(const QString &msg);

   void attributeContentError(const char *attributeName, const char *elementName, const QString &value,
         const SchemaType::Ptr &type = SchemaType::Ptr());

   void setTargetNamespaceExtended(const QString &targetNamespace);

   void parseSchema(ParserType parserType);
   void parseInclude();
   void parseImport();
   void parseRedefine();

   XsdAnnotation::Ptr parseAnnotation();
   XsdApplicationInformation::Ptr parseAppInfo();
   XsdDocumentation::Ptr parseDocumentation();
   void parseDefaultOpenContent();

   XsdSimpleType::Ptr parseGlobalSimpleType();
   XsdSimpleType::Ptr parseLocalSimpleType();

   void parseSimpleRestriction(const XsdSimpleType::Ptr &ptr);
   void parseList(const XsdSimpleType::Ptr &ptr);
   void parseUnion(const XsdSimpleType::Ptr &ptr);

   XsdFacet::Ptr parseMinExclusiveFacet();
   XsdFacet::Ptr parseMinInclusiveFacet();

   XsdFacet::Ptr parseMaxExclusiveFacet();
   XsdFacet::Ptr parseMaxInclusiveFacet();

   XsdFacet::Ptr parseTotalDigitsFacet();
   XsdFacet::Ptr parseFractionDigitsFacet();

   XsdFacet::Ptr parseLengthFacet();
   XsdFacet::Ptr parseMinLengthFacet();
   XsdFacet::Ptr parseMaxLengthFacet();

   XsdFacet::Ptr parseEnumerationFacet();
   XsdFacet::Ptr parseWhiteSpaceFacet();
   XsdFacet::Ptr parsePatternFacet();
   XsdFacet::Ptr parseAssertionFacet();

   XsdComplexType::Ptr parseGlobalComplexType();
   XsdComplexType::Ptr parseLocalComplexType();

   void resolveComplexContentType(const XsdComplexType::Ptr &complexType, bool effectiveMixed);

   void parseSimpleContent(const XsdComplexType::Ptr &complexType);
   void parseSimpleContentRestriction(const XsdComplexType::Ptr &complexType);
   void parseSimpleContentExtension(const XsdComplexType::Ptr &complexType);
   void parseComplexContent(const XsdComplexType::Ptr &complexType, bool *mixed);
   void parseComplexContentRestriction(const XsdComplexType::Ptr &complexType);
   void parseComplexContentExtension(const XsdComplexType::Ptr &complexType);

   XsdAssertion::Ptr parseAssertion(const XsdSchemaToken::NodeName &nodeName, const XsdTagScope::Type &tag);

   XsdComplexType::OpenContent::Ptr parseOpenContent();
   XsdModelGroup::Ptr parseNamedGroup();

   XsdTerm::Ptr parseReferredGroup(const XsdParticle::Ptr &particle);

   XsdModelGroup::Ptr parseAll(const NamedSchemaComponent::Ptr &parent);
   XsdModelGroup::Ptr parseLocalAll(const XsdParticle::Ptr &particle, const NamedSchemaComponent::Ptr &parent);

   XsdModelGroup::Ptr parseChoice(const NamedSchemaComponent::Ptr &parent);
   XsdModelGroup::Ptr parseLocalChoice(const XsdParticle::Ptr &particle, const NamedSchemaComponent::Ptr &parent);
   XsdModelGroup::Ptr parseSequence(const NamedSchemaComponent::Ptr &parent);
   XsdModelGroup::Ptr parseLocalSequence(const XsdParticle::Ptr &particle, const NamedSchemaComponent::Ptr &parent);

   bool parseMinMaxConstraint(const XsdParticle::Ptr &particle, const char *tagName);

   XsdAttribute::Ptr parseGlobalAttribute();
   XsdAttributeUse::Ptr parseLocalAttribute(const NamedSchemaComponent::Ptr &parent);
   XsdAttributeGroup::Ptr parseNamedAttributeGroup();
   XsdAttributeUse::Ptr parseReferredAttributeGroup();
   XsdElement::Ptr parseGlobalElement();
   XsdTerm::Ptr parseLocalElement(const XsdParticle::Ptr &particle, const NamedSchemaComponent::Ptr &parent);
   XsdIdentityConstraint::Ptr parseUnique();
   XsdIdentityConstraint::Ptr parseKey();
   XsdIdentityConstraint::Ptr parseKeyRef(const XsdElement::Ptr &element);

   void parseSelector(const XsdIdentityConstraint::Ptr &ptr);

   void parseField(const XsdIdentityConstraint::Ptr &ptr);
   XsdAlternative::Ptr parseAlternative();
   XsdNotation::Ptr parseNotation();
   XsdWildcard::Ptr parseAny(const XsdParticle::Ptr &particle);
   XsdWildcard::Ptr parseAnyAttribute();

   void parseUnknownDocumentation();
   void parseUnknown();

   QSourceLocation currentSourceLocation() const;
   void convertName(const QString &qualified, NamespaceSupport::NameType type, QXmlName &name);

   inline QString readNameAttribute(const char *elementName);
   inline QString readQNameAttribute(const QString &typeAttribute, const char *elementName);
   inline QString readNamespaceAttribute(const QString &attributeName, const char *elementName);

   inline SchemaType::DerivationConstraints readDerivationConstraintAttribute(const SchemaType::DerivationConstraints
         &allowedConstraints, const char *elementName);

   inline NamedSchemaComponent::BlockingConstraints readBlockingConstraintAttribute(const
         NamedSchemaComponent::BlockingConstraints &allowedConstraints, const char *elementName);

   XsdXPathExpression::Ptr readXPathExpression(const char *elementName);

   enum XPathType {
      XPath20,
      XPathSelector,
      XPathField
   };

   QString readXPathAttribute(const QString &attributeName, XPathType type, const char *elementName);

   inline void validateIdAttribute(const char *elementName);

   void addElement(const XsdElement::Ptr &element);
   void addAttribute(const XsdAttribute::Ptr &attribute);
   void addType(const SchemaType::Ptr &type);
   void addAnonymousType(const SchemaType::Ptr &type);
   void addAttributeGroup(const XsdAttributeGroup::Ptr &group);
   void addElementGroup(const XsdModelGroup::Ptr &group);
   void addNotation(const XsdNotation::Ptr &notation);
   void addIdentityConstraint(const XsdIdentityConstraint::Ptr &constraint);
   void addFacet(const XsdFacet::Ptr &facet, XsdFacet::Hash &facets, const SchemaType::Ptr &type);

   void setupStateMachines();
   void setupBuiltinTypeNames();

   inline bool isSchemaTag(XsdSchemaToken::NodeName tag, XsdSchemaToken::NodeName token,
         XsdSchemaToken::NodeName namespaceToken) const;

   XsdSchemaContext *m_context;
   XsdSchemaParserContext *m_parserContext;
   NamePool *m_namePool;
   NamespaceSupport  m_namespaceSupport;
   XsdSchemaResolver *m_schemaResolver;
   XsdSchema *m_schema;

   QString m_targetNamespace;
   QString m_attributeFormDefault;
   QString m_elementFormDefault;
   QString m_blockDefault;
   QString m_finalDefault;
   QString m_xpathDefaultNamespace;

   QXmlName m_defaultAttributes;
   XsdComplexType::OpenContent::Ptr m_defaultOpenContent;
   bool m_defaultOpenContentAppliesToEmpty;

   NamespaceSet m_includedSchemas;
   NamespaceSet m_importedSchemas;
   NamespaceSet m_redefinedSchemas;

   QUrl m_documentURI;
   XsdIdCache::Ptr m_idCache;
   QHash<XsdTagScope::Type, XsdStateMachine<XsdSchemaToken::NodeName> > m_stateMachines;
   ComponentLocationHash m_componentLocationHash;
   QSet<QXmlName> m_builtinTypeNames;
};

}

#endif
