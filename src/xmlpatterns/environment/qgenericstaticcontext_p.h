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

#ifndef QGenericStaticContext_P_H
#define QGenericStaticContext_P_H

#include <QUrl>
#include <QXmlQuery>
#include <qstaticcontext_p.h>
#include <qfunctionfactory_p.h>
#include <qschematypefactory_p.h>

namespace QPatternist {
class GenericStaticContext : public StaticContext
{
 public:
   typedef QExplicitlySharedDataPointer<GenericStaticContext> Ptr;
   /**
    * Constructs a GenericStaticContext. The components are initialized as per
    * the recommended default values in XQuery 1.0. <tt>Default order for empty sequences</tt>,
    * orderingEmptySequence(), is initialized to Greatest.
    *
    * @see <a href="http://www.w3.org/TR/xquery/#id-xq-static-context-components">XQuery
    * 1.0: An XML Query Language, C.1 Static Context Components</a>
    * @param errorHandler the error handler. May be null.
    * @param np the NamePool. May not be null.
    * @param aBaseURI the base URI in the static context. Must be absolute
    * and valid.
    */
   GenericStaticContext(const NamePool::Ptr &np, QAbstractMessageHandler *const errorHandler,
                  const QUrl &aBaseURI, const FunctionFactory::Ptr &factory, const QXmlQuery::QueryLanguage lang);

   NamespaceResolver::Ptr namespaceBindings() const override;
   void setNamespaceBindings(const NamespaceResolver::Ptr &) override;

   FunctionFactory::Ptr functionSignatures() const override;
   SchemaTypeFactory::Ptr schemaDefinitions() const override;

   /**
    * Returns a DynamicContext used for evaluation at compile time.
    *
    * @bug The DynamicContext isn't stable. It should be cached privately.
    */
   DynamicContext::Ptr dynamicContext() const override;

   QUrl baseURI() const override;
   void setBaseURI(const QUrl &uri) override;

   bool compatModeEnabled() const override;
   void setCompatModeEnabled(const bool newVal) override;

   /**
    * @returns always the Unicode codepoint collation URI
    */
   QUrl defaultCollation() const override;

   QAbstractMessageHandler *messageHandler() const override;

   void setDefaultCollation(const QUrl &uri) override;

   BoundarySpacePolicy boundarySpacePolicy() const override;
   void setBoundarySpacePolicy(const BoundarySpacePolicy policy) override;

   ConstructionMode constructionMode() const override;
   void setConstructionMode(const ConstructionMode mode) override;

   OrderingMode orderingMode() const override;
   void setOrderingMode(const OrderingMode mode) override;
   OrderingEmptySequence orderingEmptySequence() const override;
   void setOrderingEmptySequence(const OrderingEmptySequence ordering) override;

   QString defaultFunctionNamespace() const override;
   void setDefaultFunctionNamespace(const QString &ns) override;

   QString defaultElementNamespace() const override;
   void setDefaultElementNamespace(const QString &ns) override;

   InheritMode inheritMode() const override;
   void setInheritMode(const InheritMode mode) override;

   PreserveMode preserveMode() const override;
   void setPreserveMode(const PreserveMode mode) override;

   ItemType::Ptr contextItemType() const override;
   void setContextItemType(const ItemType::Ptr &type);

   ItemType::Ptr currentItemType() const override;

   StaticContext::Ptr copy() const override;

   ResourceLoader::Ptr resourceLoader() const override;
   void setResourceLoader(const ResourceLoader::Ptr &loader);

   ExternalVariableLoader::Ptr externalVariableLoader() const override;
   void setExternalVariableLoader(const ExternalVariableLoader::Ptr &loader);
   NamePool::Ptr namePool() const override;

   void addLocation(const SourceLocationReflection *const reflection, const QSourceLocation &location) override;
   QSourceLocation locationFor(const SourceLocationReflection *const reflection) const override;

   LocationHash sourceLocations() const override;
   QAbstractUriResolver *uriResolver() const override;

   VariableSlotID currentRangeSlot() const override;
   VariableSlotID allocateRangeSlot() override;

 private:
   BoundarySpacePolicy         m_boundarySpacePolicy;
   ConstructionMode            m_constructionMode;
   FunctionFactory::Ptr        m_functionFactory;
   QString                     m_defaultElementNamespace;
   QString                     m_defaultFunctionNamespace;
   OrderingEmptySequence       m_orderingEmptySequence;
   OrderingMode                m_orderingMode;
   QUrl                        m_defaultCollation;
   QUrl                        m_baseURI;
   QAbstractMessageHandler    *m_messageHandler;
   PreserveMode                m_preserveMode;
   InheritMode                 m_inheritMode;
   NamespaceResolver::Ptr      m_namespaceResolver;
   ExternalVariableLoader::Ptr m_externalVariableLoader;
   ResourceLoader::Ptr         m_resourceLoader;
   const NamePool::Ptr         m_namePool;
   ItemType::Ptr               m_contextItemType;
   LocationHash                m_locations;
   QAbstractUriResolver       *m_uriResolver;
   QXmlQuery::QueryLanguage    m_queryLanguage;
   VariableSlotID              m_rangeSlot;
   bool                        m_compatModeEnabled;
};
}

#endif
