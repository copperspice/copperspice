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

#ifndef QGenericDynamicContext_P_H
#define QGenericDynamicContext_P_H

#include <qdatetime.h>
#include <qvector.h>

#include <qdaytimeduration_p.h>
#include <qstackcontextbase_p.h>
#include <qexpression_p.h>

namespace QPatternist {

class GenericDynamicContext : public StackContextBase<DynamicContext>
{
 public:
   typedef QExplicitlySharedDataPointer<GenericDynamicContext> Ptr;

   GenericDynamicContext(const NamePool::Ptr &np, QAbstractMessageHandler *const messageHandler, const LocationHash &locations);

   xsInteger contextPosition() const override;
   /**
    * @returns always @c null, the focus is always undefined when an GenericDynamicContext
    * is used.
    */
   Item contextItem() const override;
   xsInteger contextSize() override;

   void setFocusIterator(const Item::Iterator::Ptr &it) override;
   Item::Iterator::Ptr focusIterator() const override;

   QAbstractMessageHandler *messageHandler() const override;
   QExplicitlySharedDataPointer<DayTimeDuration> implicitTimezone() const override;
   QDateTime currentDateTime() const override;

   QAbstractXmlReceiver *outputReceiver() const override;
   void setOutputReceiver(QAbstractXmlReceiver *const receiver);

   NodeBuilder::Ptr nodeBuilder(const QUrl &baseURI) const override;
   void setNodeBuilder(NodeBuilder::Ptr &builder);

   ResourceLoader::Ptr resourceLoader() const override;
   void setResourceLoader(const ResourceLoader::Ptr &loader);

   ExternalVariableLoader::Ptr externalVariableLoader() const override;
   void setExternalVariableLoader(const ExternalVariableLoader::Ptr &loader);
   NamePool::Ptr namePool() const override;
   QSourceLocation locationFor(const SourceLocationReflection *const reflection) const override;
   void addNodeModel(const QAbstractXmlNodeModel::Ptr &nm) override;
   const QAbstractUriResolver *uriResolver() const override;
   ItemCacheCell &globalItemCacheCell(const VariableSlotID slot) override;
   ItemSequenceCacheCell::Vector &globalItemSequenceCacheCells(const VariableSlotID slot) override;

   void setUriResolver(const QAbstractUriResolver *const resolver);

   /**
    * We return a null item, we have no focus.
    */
   Item currentItem() const override;

   /**
    * @short Returns always @c null, since we are always a top-level context.
    */
   DynamicContext::Ptr previousContext() const override;

   QExplicitlySharedDataPointer<TemplateMode> currentTemplateMode() const override;

 private:
   QAbstractMessageHandler        *m_messageHandler;
   const QDateTime                 m_currentDateTime;
   const DayTimeDuration::Ptr      m_zoneOffset;
   QAbstractXmlReceiver           *m_outputReceiver;
   mutable NodeBuilder::Ptr        m_nodeBuilder;
   ExternalVariableLoader::Ptr     m_externalVariableLoader;
   ResourceLoader::Ptr             m_resourceLoader;
   NamePool::Ptr                   m_namePool;
   const LocationHash              m_locations;
   QAbstractXmlNodeModel::List     m_nodeModels;
   const QAbstractUriResolver     *m_uriResolver;
   ItemCacheCell::Vector           m_globalItemCacheCells;
   ItemSequenceCacheCell::Vector   m_globalItemSequenceCacheCells;
};

}

#endif
