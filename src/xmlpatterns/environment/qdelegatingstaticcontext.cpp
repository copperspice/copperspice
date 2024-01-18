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

/* Patternist */
#include "qbasictypesfactory_p.h"
#include "qfunctionfactorycollection_p.h"
#include "qgenericnamespaceresolver_p.h"
#include "qcommonnamespaces_p.h"
#include "qgenericdynamiccontext_p.h"

#include "qstaticfocuscontext_p.h"

using namespace QPatternist;

DelegatingStaticContext::DelegatingStaticContext(const StaticContext::Ptr &context) : m_context(context)
{
   Q_ASSERT(context);
}

NamespaceResolver::Ptr DelegatingStaticContext::namespaceBindings() const
{
   return m_context->namespaceBindings();
}

FunctionFactory::Ptr DelegatingStaticContext::functionSignatures() const
{
   return m_context->functionSignatures();
}

DynamicContext::Ptr DelegatingStaticContext::dynamicContext() const
{
   return m_context->dynamicContext();
}

SchemaTypeFactory::Ptr DelegatingStaticContext::schemaDefinitions() const
{
   return m_context->schemaDefinitions();
}

QUrl DelegatingStaticContext::baseURI() const
{
   return m_context->baseURI();
}

void DelegatingStaticContext::setBaseURI(const QUrl &uri)
{
   m_context->setBaseURI(uri);
}

bool DelegatingStaticContext::compatModeEnabled() const
{
   return m_context->compatModeEnabled();
}

QUrl DelegatingStaticContext::defaultCollation() const
{
   return m_context->defaultCollation();
}

QAbstractMessageHandler *DelegatingStaticContext::messageHandler() const
{
   return m_context->messageHandler();
}

void DelegatingStaticContext::setDefaultCollation(const QUrl &uri)
{
   m_context->setDefaultCollation(uri);
}

void DelegatingStaticContext::setNamespaceBindings(const NamespaceResolver::Ptr &resolver)
{
   m_context->setNamespaceBindings(resolver);
}

StaticContext::BoundarySpacePolicy DelegatingStaticContext::boundarySpacePolicy() const
{
   return m_context->boundarySpacePolicy();
}

void DelegatingStaticContext::setBoundarySpacePolicy(const BoundarySpacePolicy policy)
{
   m_context->setBoundarySpacePolicy(policy);
}

StaticContext::ConstructionMode DelegatingStaticContext::constructionMode() const
{
   return m_context->constructionMode();
}

void DelegatingStaticContext::setConstructionMode(const ConstructionMode mode)
{
   m_context->setConstructionMode(mode);
}

StaticContext::OrderingMode DelegatingStaticContext::orderingMode() const
{
   return m_context->orderingMode();
}

void DelegatingStaticContext::setOrderingMode(const OrderingMode mode)
{
   m_context->setOrderingMode(mode);
}

StaticContext::OrderingEmptySequence DelegatingStaticContext::orderingEmptySequence() const
{
   return m_context->orderingEmptySequence();
}

void DelegatingStaticContext::setOrderingEmptySequence(const OrderingEmptySequence ordering)
{
   m_context->setOrderingEmptySequence(ordering);
}

QString DelegatingStaticContext::defaultFunctionNamespace() const
{
   return m_context->defaultFunctionNamespace();
}

void DelegatingStaticContext::setDefaultFunctionNamespace(const QString &ns)
{
   m_context->setDefaultFunctionNamespace(ns);
}

QString DelegatingStaticContext::defaultElementNamespace() const
{
   return m_context->defaultElementNamespace();
}

void DelegatingStaticContext::setDefaultElementNamespace(const QString &ns)
{
   m_context->setDefaultElementNamespace(ns);
}

StaticContext::InheritMode DelegatingStaticContext::inheritMode() const
{
   return m_context->inheritMode();
}

void DelegatingStaticContext::setInheritMode(const InheritMode mode)
{
   m_context->setInheritMode(mode);
}

StaticContext::PreserveMode DelegatingStaticContext::preserveMode() const
{
   return m_context->preserveMode();
}

void DelegatingStaticContext::setPreserveMode(const PreserveMode mode)
{
   m_context->setPreserveMode(mode);
}

ItemType::Ptr DelegatingStaticContext::contextItemType() const
{
   return m_context->contextItemType();
}

ItemType::Ptr DelegatingStaticContext::currentItemType() const
{
   return m_context->currentItemType();
}

ExternalVariableLoader::Ptr DelegatingStaticContext::externalVariableLoader() const
{
   return m_context->externalVariableLoader();
}

StaticContext::Ptr DelegatingStaticContext::copy() const
{
   return StaticContext::Ptr(new DelegatingStaticContext(m_context->copy()));
}

ResourceLoader::Ptr DelegatingStaticContext::resourceLoader() const
{
   return m_context->resourceLoader();
}

NamePool::Ptr DelegatingStaticContext::namePool() const
{
   return m_context->namePool();
}

void DelegatingStaticContext::addLocation(const SourceLocationReflection *const reflection,
      const QSourceLocation &location)
{
   m_context->addLocation(reflection, location);
}

StaticContext::LocationHash DelegatingStaticContext::sourceLocations() const
{
   return m_context->sourceLocations();
}

QSourceLocation DelegatingStaticContext::locationFor(const SourceLocationReflection *const reflection) const
{
   return m_context->locationFor(reflection);
}

const QAbstractUriResolver *DelegatingStaticContext::uriResolver() const
{
   return m_context->uriResolver();
}

VariableSlotID DelegatingStaticContext::currentRangeSlot() const
{
   return m_context->currentRangeSlot();
}

VariableSlotID DelegatingStaticContext::allocateRangeSlot()
{
   return m_context->allocateRangeSlot();
}

void DelegatingStaticContext::setCompatModeEnabled(const bool newVal)
{
   m_context->setCompatModeEnabled(newVal);
}
