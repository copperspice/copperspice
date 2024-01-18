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

#ifndef QXsdSchemaParserContext_P_H
#define QXsdSchemaParserContext_P_H

#include <qmaintainingreader_p.h>    // for definition of ElementDescription
#include <qxsdschematoken_p.h>
#include <qxsdschema_p.h>
#include <qxsdschemachecker_p.h>
#include <qxsdschemacontext_p.h>
#include <qxsdschemaresolver_p.h>
#include <QSharedData>

namespace QPatternist {
/**
 * @short A namespace class that contains identifiers for the different
 *        scopes a tag from the xml schema spec can appear in.
 */
class XsdTagScope
{
 public:
   enum Type {
      Schema,
      Include,
      Import,
      Redefine,
      Annotation,
      AppInfo,
      Documentation,
      GlobalSimpleType,
      LocalSimpleType,
      SimpleRestriction,
      List,
      Union,
      MinExclusiveFacet,
      MinInclusiveFacet,
      MaxExclusiveFacet,
      MaxInclusiveFacet,
      TotalDigitsFacet,
      FractionDigitsFacet,
      LengthFacet,
      MinLengthFacet,
      MaxLengthFacet,
      EnumerationFacet,
      WhiteSpaceFacet,
      PatternFacet,
      GlobalComplexType,
      LocalComplexType,
      SimpleContent,
      SimpleContentRestriction,
      SimpleContentExtension,
      ComplexContent,
      ComplexContentRestriction,
      ComplexContentExtension,
      NamedGroup,
      ReferredGroup,
      All,
      LocalAll,
      Choice,
      LocalChoice,
      Sequence,
      LocalSequence,
      GlobalAttribute,
      LocalAttribute,
      NamedAttributeGroup,
      ReferredAttributeGroup,
      GlobalElement,
      LocalElement,
      Unique,
      Key,
      KeyRef,
      Selector,
      Field,
      Notation,
      Any,
      AnyAttribute,
      Alternative,
      Assert,
      Assertion,
      OpenContent,
      DefaultOpenContent,
      Override
   };
};

/**
 * A hash that keeps the mapping between the single components that can appear
 * in a schema document (e.g. elements, attributes, type definitions) and their
 * source locations inside the document.
 */
typedef QHash<NamedSchemaComponent::Ptr, QSourceLocation> ComponentLocationHash;

class XsdSchemaParserContext : public QSharedData
{
 public:
   /**
    * A smart pointer wrapping XsdSchemaParserContext instances.
    */
   typedef QExplicitlySharedDataPointer<XsdSchemaParserContext> Ptr;

   /**
    * Creates a new schema parser context object.
    *
    * @param namePool The name pool where all names of the schema will be stored in.
    * @param context The schema context to use for error reporting etc.
    */
   XsdSchemaParserContext(const NamePool::Ptr &namePool, const XsdSchemaContext::Ptr &context);

   /**
    * Returns the name pool of the schema parser context.
    */
   NamePool::Ptr namePool() const;

   /**
    * Returns the schema resolver of the schema context.
    */
   XsdSchemaResolver::Ptr resolver() const;

   /**
    * Returns the schema resolver of the schema context.
    */
   XsdSchemaChecker::Ptr checker() const;

   /**
    * Returns the schema object of the schema context.
    */
   XsdSchema::Ptr schema() const;

   /**
    * Returns the element descriptions for the schema parser.
    *
    * The element descriptions are a fast lookup table for
    * verifying whether certain attributes are allowed for
    * a given element type.
    */
   ElementDescription<XsdSchemaToken, XsdTagScope::Type>::Hash elementDescriptions() const;

   /**
    * Returns an unique name that is used by the schema parser
    * for anonymous types.
    *
    * @param targetNamespace The namespace of the name.
    */
   QXmlName createAnonymousName(const QString &targetNamespace) const;

 private:
   /**
    * Fills the element description hash with the required and prohibited
    * attributes.
    */
   static ElementDescription<XsdSchemaToken, XsdTagScope::Type>::Hash setupElementDescriptions();

   NamePool::Ptr                                                     m_namePool;
   XsdSchema::Ptr                                                    m_schema;
   XsdSchemaChecker::Ptr                                             m_checker;
   XsdSchemaResolver::Ptr                                            m_resolver;
   const ElementDescription<XsdSchemaToken, XsdTagScope::Type>::Hash m_elementDescriptions;
   mutable QAtomicInt                                                m_anonymousNameCounter;
};

}

#endif
