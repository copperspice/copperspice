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

#include <qtimezone.h>

#include "qcommonvalues_p.h"
#include "qfocus_p.h"
#include "qtemplatemode_p.h"

#include "qgenericdynamiccontext_p.h"

using namespace QPatternist;

GenericDynamicContext::GenericDynamicContext(const NamePool::Ptr &np,
      QAbstractMessageHandler *const errHandler, const LocationHash &locations)
   : m_messageHandler(errHandler), m_currentDateTime(QDateTime::currentDateTime().toTimeZone(QTimeZone::utc())),
      m_outputReceiver(nullptr), m_namePool(np), m_locations(locations), m_uriResolver(nullptr)
{
   Q_ASSERT(m_messageHandler);
   Q_ASSERT(m_namePool);
}

QExplicitlySharedDataPointer<DayTimeDuration> GenericDynamicContext::implicitTimezone() const
{
   return CommonValues::DayTimeDurationZero;
}

QAbstractMessageHandler *GenericDynamicContext::messageHandler() const
{
   return m_messageHandler;
}

QDateTime GenericDynamicContext::currentDateTime() const
{
   return m_currentDateTime;
}

xsInteger GenericDynamicContext::contextPosition() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return 0;
}

Item GenericDynamicContext::contextItem() const
{
   return Item();
}

xsInteger GenericDynamicContext::contextSize()
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return 0;
}

void GenericDynamicContext::setFocusIterator(const Item::Iterator::Ptr &)
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
}

Item::Iterator::Ptr GenericDynamicContext::focusIterator() const
{
   return Item::Iterator::Ptr();
}

QAbstractXmlReceiver *GenericDynamicContext::outputReceiver() const
{
   return m_outputReceiver;
}

void GenericDynamicContext::setOutputReceiver(QAbstractXmlReceiver *const receiver)
{
   m_outputReceiver = receiver;
}

void GenericDynamicContext::setNodeBuilder(NodeBuilder::Ptr &builder)
{
   m_nodeBuilder = std::move(builder);
}

NodeBuilder::Ptr GenericDynamicContext::nodeBuilder(const QUrl &baseURI) const
{
   return m_nodeBuilder->create(baseURI);
}

ResourceLoader::Ptr GenericDynamicContext::resourceLoader() const
{
   return m_resourceLoader;
}

void GenericDynamicContext::setResourceLoader(const ResourceLoader::Ptr &loader)
{
   m_resourceLoader = loader;
}

ExternalVariableLoader::Ptr GenericDynamicContext::externalVariableLoader() const
{
   return m_externalVariableLoader;
}

void GenericDynamicContext::setExternalVariableLoader(const ExternalVariableLoader::Ptr &loader)
{
   m_externalVariableLoader = loader;
}

NamePool::Ptr GenericDynamicContext::namePool() const
{
   return m_namePool;
}

QSourceLocation GenericDynamicContext::locationFor(const SourceLocationReflection *const reflection) const
{

   return m_locations.value(reflection->actualReflection());
}

void GenericDynamicContext::addNodeModel(const QAbstractXmlNodeModel::Ptr &nm)
{
   m_nodeModels.append(nm);
}

const QAbstractUriResolver *GenericDynamicContext::uriResolver() const
{
   return m_uriResolver;
}

ItemCacheCell &GenericDynamicContext::globalItemCacheCell(const VariableSlotID slot)
{
   if (slot >= m_globalItemCacheCells.size()) {
      m_globalItemCacheCells.resize(qMax(slot + 1, m_globalItemCacheCells.size()));
   }

   return m_globalItemCacheCells[slot];
}

ItemSequenceCacheCell::Vector &GenericDynamicContext::globalItemSequenceCacheCells(const VariableSlotID slot)
{
   if (slot >= m_globalItemSequenceCacheCells.size()) {
      m_globalItemSequenceCacheCells.resize(qMax(slot + 1, m_globalItemSequenceCacheCells.size()));
   }

   return m_globalItemSequenceCacheCells;
}

void GenericDynamicContext::setUriResolver(const QAbstractUriResolver *const resolver)
{
   m_uriResolver = resolver;
}

Item GenericDynamicContext::currentItem() const
{
   return Item();
}

DynamicContext::Ptr GenericDynamicContext::previousContext() const
{
   return DynamicContext::Ptr();
}

QExplicitlySharedDataPointer<TemplateMode> GenericDynamicContext::currentTemplateMode() const
{
   return QExplicitlySharedDataPointer<TemplateMode>();
}

