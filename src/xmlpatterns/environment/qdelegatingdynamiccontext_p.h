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

#ifndef QDelegatingDynamicContext_P_H
#define QDelegatingDynamicContext_P_H

#include <qdynamiccontext_p.h>
#include <qexpression_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class DelegatingDynamicContext : public DynamicContext
{
 public:
   virtual xsInteger contextPosition() const;
   virtual Item contextItem() const;
   virtual xsInteger contextSize();

   virtual ItemCacheCell &itemCacheCell(const VariableSlotID slot);
   virtual ItemSequenceCacheCell::Vector &itemSequenceCacheCells(const VariableSlotID slot);

   virtual void setRangeVariable(const VariableSlotID slotNumber,
                                 const Item &newValue);
   virtual Item rangeVariable(const VariableSlotID slotNumber) const;

   virtual void setExpressionVariable(const VariableSlotID slotNumber,
                                      const Expression::Ptr &newValue);
   virtual Expression::Ptr expressionVariable(const VariableSlotID slotNumber) const;

   virtual void setFocusIterator(const Item::Iterator::Ptr &it);
   virtual Item::Iterator::Ptr focusIterator() const;

   virtual Item::Iterator::Ptr positionIterator(const VariableSlotID slot) const;
   virtual void setPositionIterator(const VariableSlotID slot,
                                    const Item::Iterator::Ptr &newValue);

   virtual QAbstractMessageHandler *messageHandler() const;
   virtual QExplicitlySharedDataPointer<DayTimeDuration> implicitTimezone() const;
   virtual QDateTime currentDateTime() const;
   virtual QAbstractXmlReceiver *outputReceiver() const;
   virtual NodeBuilder::Ptr nodeBuilder(const QUrl &baseURI) const;
   virtual ResourceLoader::Ptr resourceLoader() const;
   virtual ExternalVariableLoader::Ptr externalVariableLoader() const;
   virtual NamePool::Ptr namePool() const;
   virtual QSourceLocation locationFor(const SourceLocationReflection *const reflection) const;
   virtual void addNodeModel(const QAbstractXmlNodeModel::Ptr &nm);
   virtual const QAbstractUriResolver *uriResolver() const;
   virtual ItemCacheCell &globalItemCacheCell(const VariableSlotID slot);
   virtual ItemSequenceCacheCell::Vector &globalItemSequenceCacheCells(const VariableSlotID slot);
   virtual Item currentItem() const;
   virtual TemplateParameterHash &templateParameterStore();

   virtual DynamicContext::Ptr previousContext() const;
   virtual QExplicitlySharedDataPointer<TemplateMode> currentTemplateMode() const;

 protected:
   DelegatingDynamicContext(const DynamicContext::Ptr &prevContext);

   const DynamicContext::Ptr m_prevContext;
};
}

QT_END_NAMESPACE

#endif
