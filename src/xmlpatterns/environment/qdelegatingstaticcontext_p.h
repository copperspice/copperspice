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

#ifndef QDelegatingStaticContext_P_H
#define QDelegatingStaticContext_P_H

#include <QUrl>
#include <qstaticcontext_p.h>
#include <qfunctionfactory_p.h>
#include <qschematypefactory_p.h>

namespace QPatternist {

class DelegatingStaticContext : public StaticContext
{
 public:
   NamespaceResolver::Ptr namespaceBindings() const override;
   void setNamespaceBindings(const NamespaceResolver::Ptr &) override;

   FunctionFactory::Ptr functionSignatures() const override;
   SchemaTypeFactory::Ptr schemaDefinitions() const override;
   DynamicContext::Ptr dynamicContext() const override;

   QUrl baseURI() const override;
   void setBaseURI(const QUrl &uri) override;

   bool compatModeEnabled() const override;
   void setCompatModeEnabled(const bool newVal) override;

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
   ItemType::Ptr currentItemType() const override;

   StaticContext::Ptr copy() const override;

   ExternalVariableLoader::Ptr externalVariableLoader() const override;
   ResourceLoader::Ptr resourceLoader() const override;
   NamePool::Ptr namePool() const override;
   void addLocation(const SourceLocationReflection *const reflection, const QSourceLocation &location) override;
   LocationHash sourceLocations() const override;
   QSourceLocation locationFor(const SourceLocationReflection *const reflection) const override;
   const QAbstractUriResolver *uriResolver() const override;

   VariableSlotID currentRangeSlot() const override;
   VariableSlotID allocateRangeSlot() override;

 protected:
   DelegatingStaticContext(const StaticContext::Ptr &context);

 private:
   const StaticContext::Ptr m_context;
};
}

#endif
