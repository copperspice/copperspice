/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDelegatingStaticContext_P_H
#define QDelegatingStaticContext_P_H

#include <QUrl>
#include <qstaticcontext_p.h>
#include <qfunctionfactory_p.h>
#include <qschematypefactory_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class DelegatingStaticContext : public StaticContext
{
 public:
   virtual NamespaceResolver::Ptr namespaceBindings() const;
   virtual void setNamespaceBindings(const NamespaceResolver::Ptr &);

   virtual FunctionFactory::Ptr functionSignatures() const;
   virtual SchemaTypeFactory::Ptr schemaDefinitions() const;
   virtual DynamicContext::Ptr dynamicContext() const;

   virtual QUrl baseURI() const;
   virtual void setBaseURI(const QUrl &uri);

   virtual bool compatModeEnabled() const;
   virtual void setCompatModeEnabled(const bool newVal);

   virtual QUrl defaultCollation() const;

   virtual QAbstractMessageHandler *messageHandler() const;

   virtual void setDefaultCollation(const QUrl &uri);

   virtual BoundarySpacePolicy boundarySpacePolicy() const;
   virtual void setBoundarySpacePolicy(const BoundarySpacePolicy policy);

   virtual ConstructionMode constructionMode() const;
   virtual void setConstructionMode(const ConstructionMode mode);

   virtual OrderingMode orderingMode() const;
   virtual void setOrderingMode(const OrderingMode mode);
   virtual OrderingEmptySequence orderingEmptySequence() const;
   virtual void setOrderingEmptySequence(const OrderingEmptySequence ordering);

   virtual QString defaultFunctionNamespace() const;
   virtual void setDefaultFunctionNamespace(const QString &ns);

   virtual QString defaultElementNamespace() const;
   virtual void setDefaultElementNamespace(const QString &ns);

   virtual InheritMode inheritMode() const;
   virtual void setInheritMode(const InheritMode mode);

   virtual PreserveMode preserveMode() const;
   virtual void setPreserveMode(const PreserveMode mode);

   virtual ItemType::Ptr contextItemType() const;
   virtual ItemType::Ptr currentItemType() const;

   virtual StaticContext::Ptr copy() const;

   virtual ExternalVariableLoader::Ptr externalVariableLoader() const;
   virtual ResourceLoader::Ptr resourceLoader() const;
   virtual NamePool::Ptr namePool() const;
   virtual void addLocation(const SourceLocationReflection *const reflection,
                            const QSourceLocation &location);
   virtual LocationHash sourceLocations() const;
   virtual QSourceLocation locationFor(const SourceLocationReflection *const reflection) const;
   virtual const QAbstractUriResolver *uriResolver() const;

   virtual VariableSlotID currentRangeSlot() const;
   virtual VariableSlotID allocateRangeSlot();

 protected:
   DelegatingStaticContext(const StaticContext::Ptr &context);

 private:
   const StaticContext::Ptr    m_context;
};
}

QT_END_NAMESPACE

#endif
