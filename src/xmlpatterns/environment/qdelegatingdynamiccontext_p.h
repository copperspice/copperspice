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

#ifndef QDelegatingDynamicContext_P_H
#define QDelegatingDynamicContext_P_H

#include <qdynamiccontext_p.h>
#include <qexpression_p.h>

namespace QPatternist {

class DelegatingDynamicContext : public DynamicContext
{
 public:
   xsInteger contextPosition() const override;
   Item contextItem() const override;
   xsInteger contextSize() override;

   ItemCacheCell &itemCacheCell(const VariableSlotID slot) override;
   ItemSequenceCacheCell::Vector &itemSequenceCacheCells(const VariableSlotID slot) override;

   void setRangeVariable(const VariableSlotID slotNumber, const Item &newValue) override;
   Item rangeVariable(const VariableSlotID slotNumber) const override;

   void setExpressionVariable(const VariableSlotID slotNumber, const Expression::Ptr &newValue) override;
   Expression::Ptr expressionVariable(const VariableSlotID slotNumber) const override;

   void setFocusIterator(const Item::Iterator::Ptr &it) override;
   Item::Iterator::Ptr focusIterator() const override;

   Item::Iterator::Ptr positionIterator(const VariableSlotID slot) const override;
   void setPositionIterator(const VariableSlotID slot, const Item::Iterator::Ptr &newValue) override;

   QAbstractMessageHandler *messageHandler() const override;
   QExplicitlySharedDataPointer<DayTimeDuration> implicitTimezone() const override;
   QDateTime currentDateTime() const override;
   QAbstractXmlReceiver *outputReceiver() const override;
   NodeBuilder::Ptr nodeBuilder(const QUrl &baseURI) const override;
   ResourceLoader::Ptr resourceLoader() const override;
   ExternalVariableLoader::Ptr externalVariableLoader() const override;
   NamePool::Ptr namePool() const override;
   QSourceLocation locationFor(const SourceLocationReflection *const reflection) const override;
   void addNodeModel(const QAbstractXmlNodeModel::Ptr &nm) override;
   const QAbstractUriResolver *uriResolver() const override;
   ItemCacheCell &globalItemCacheCell(const VariableSlotID slot) override;
   ItemSequenceCacheCell::Vector &globalItemSequenceCacheCells(const VariableSlotID slot) override;
   Item currentItem() const override;
   TemplateParameterHash &templateParameterStore() override;

   DynamicContext::Ptr previousContext() const override;
   QExplicitlySharedDataPointer<TemplateMode> currentTemplateMode() const override;

 protected:
   DelegatingDynamicContext(const DynamicContext::Ptr &prevContext);

   const DynamicContext::Ptr m_prevContext;
};
}

#endif
