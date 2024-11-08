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

#ifndef QStaticContext_P_H
#define QStaticContext_P_H

#include <qexternalvariableloader_p.h>
#include <qitemtype_p.h>
#include <qnamepool_p.h>
#include <qnamespaceresolver_p.h>
#include <qreportcontext_p.h>
#include <qresourceloader_p.h>
#include <qcontainerfwd.h>

class QUrl;

namespace QPatternist {

class DynamicContext;
class Expression;
class FunctionFactory;
class SchemaTypeFactory;

class StaticContext : public ReportContext
{
 public:
   typedef QExplicitlySharedDataPointer<StaticContext> Ptr;

   enum BoundarySpacePolicy {
      BSPPreserve,
      BSPStrip
   };

   enum ConstructionMode {
      CMPreserve,
      CMStrip
   };

   enum OrderingMode {
      Ordered,
      Unordered
   };

   enum OrderingEmptySequence {
      Greatest,
      Least
   };

   enum InheritMode {
      Inherit,
      NoInherit
   };

   enum PreserveMode {
      Preserve,
      NoPreserve
   };

   StaticContext() {
   }

   virtual ~StaticContext();

   virtual NamespaceResolver::Ptr namespaceBindings() const = 0;
   virtual void setNamespaceBindings(const NamespaceResolver::Ptr &) = 0;
   virtual QExplicitlySharedDataPointer<FunctionFactory> functionSignatures() const = 0;
   virtual QExplicitlySharedDataPointer<SchemaTypeFactory> schemaDefinitions() const = 0;

   virtual QUrl baseURI() const = 0;

   virtual void setBaseURI(const QUrl &uri) = 0;

   virtual QString defaultFunctionNamespace() const = 0;
   virtual void setDefaultFunctionNamespace(const QString &ns) = 0;

   virtual QString defaultElementNamespace() const = 0;
   virtual void setDefaultElementNamespace(const QString &ns) = 0;

   virtual QUrl defaultCollation() const = 0;

   virtual void setDefaultCollation(const QUrl &uri) = 0;

   virtual bool compatModeEnabled() const = 0;

   virtual void setCompatModeEnabled(const bool newVal) = 0;

   virtual QExplicitlySharedDataPointer<DynamicContext> dynamicContext() const = 0;

   virtual BoundarySpacePolicy boundarySpacePolicy() const = 0;
   virtual void setBoundarySpacePolicy(const BoundarySpacePolicy policy) = 0;

   virtual ConstructionMode constructionMode() const = 0;
   virtual void setConstructionMode(const ConstructionMode mode) = 0;

   virtual OrderingMode orderingMode() const = 0;
   virtual void setOrderingMode(const OrderingMode mode) = 0;
   virtual OrderingEmptySequence orderingEmptySequence() const = 0;
   virtual void setOrderingEmptySequence(const OrderingEmptySequence ordering) = 0;

   virtual InheritMode inheritMode() const = 0;
   virtual void setInheritMode(const InheritMode mode) = 0;

   virtual PreserveMode preserveMode() const = 0;
   virtual void setPreserveMode(const PreserveMode mode) = 0;

   virtual ItemType::Ptr contextItemType() const = 0;

   virtual ItemType::Ptr currentItemType() const = 0;

   virtual StaticContext::Ptr copy() const = 0;

   virtual ExternalVariableLoader::Ptr externalVariableLoader() const = 0;
   virtual ResourceLoader::Ptr resourceLoader() const = 0;

   virtual void addLocation(const SourceLocationReflection *const reflection, const QSourceLocation &location) = 0;

   virtual LocationHash sourceLocations() const = 0;

   virtual VariableSlotID currentRangeSlot() const = 0;
   virtual VariableSlotID allocateRangeSlot() = 0;

   void wrapExpressionWith(const SourceLocationReflection *const existingNode,
         const QExplicitlySharedDataPointer<Expression> &newNode);
};

}

#endif
