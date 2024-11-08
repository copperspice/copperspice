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

#ifndef QDynamicContext_P_H
#define QDynamicContext_P_H

#include <qcachecells_p.h>
#include <qexternalvariableloader_p.h>
#include <qitem_p.h>
#include <qnamepool_p.h>
#include <qnodebuilder_p.h>
#include <qprimitives_p.h>
#include <qreportcontext_p.h>
#include <qresourceloader_p.h>

class QDateTime;

template<typename T>
class QVector;

namespace QPatternist {

class DayTimeDuration;
class Expression;
class TemplateMode;

class DynamicContext : public ReportContext
{
 public:
   typedef QHash<QXmlName, QExplicitlySharedDataPointer<Expression> > TemplateParameterHash;
   typedef QExplicitlySharedDataPointer<DynamicContext> Ptr;

   virtual ~DynamicContext()
   { }

   virtual ItemCacheCell &itemCacheCell(const VariableSlotID slot) = 0;
   virtual ItemSequenceCacheCell::Vector &itemSequenceCacheCells(const VariableSlotID slot) = 0;

   virtual xsInteger contextPosition() const = 0;
   virtual Item contextItem() const = 0;
   virtual xsInteger contextSize() = 0;

   virtual void setRangeVariable(const VariableSlotID slot, const Item &newValue) = 0;
   virtual Item rangeVariable(const VariableSlotID slot) const = 0;

   virtual void setExpressionVariable(const VariableSlotID slot,
                  const QExplicitlySharedDataPointer<Expression> &newValue) = 0;

   virtual QExplicitlySharedDataPointer<Expression>
   expressionVariable(const VariableSlotID slot) const = 0;

   virtual Item::Iterator::Ptr positionIterator(const VariableSlotID slot) const = 0;
   virtual void setPositionIterator(const VariableSlotID slot, const Item::Iterator::Ptr &newValue) = 0;

   virtual void setFocusIterator(const Item::Iterator::Ptr &it) = 0;
   virtual Item::Iterator::Ptr focusIterator() const = 0;

   virtual QExplicitlySharedDataPointer<DayTimeDuration> implicitTimezone() const = 0;
   virtual QDateTime currentDateTime() const = 0;

   virtual QAbstractXmlReceiver *outputReceiver() const = 0;
   virtual NodeBuilder::Ptr nodeBuilder(const QUrl &baseURI) const = 0;
   virtual ResourceLoader::Ptr resourceLoader() const = 0;
   virtual ExternalVariableLoader::Ptr externalVariableLoader() const = 0;

   virtual Item currentItem() const = 0;

   DynamicContext::Ptr createFocus();
   DynamicContext::Ptr createStack();
   DynamicContext::Ptr createReceiverContext(QAbstractXmlReceiver *const receiver);

   virtual void addNodeModel(const QAbstractXmlNodeModel::Ptr &nm) = 0;
   virtual ItemCacheCell &globalItemCacheCell(const VariableSlotID slot) = 0;

   virtual TemplateParameterHash &templateParameterStore() = 0;
   virtual ItemSequenceCacheCell::Vector &globalItemSequenceCacheCells(const VariableSlotID slot) = 0;
   virtual DynamicContext::Ptr previousContext() const = 0;
   virtual QExplicitlySharedDataPointer<TemplateMode> currentTemplateMode() const = 0;
};

}

#endif
