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
   virtual ~DynamicContext() { }

   /**
    * @short Carries template parameters at runtime.
    *
    * The key is the name of the parameter, and the value the Expression
    * which supplies the value.
    */
   typedef QHash<QXmlName, QExplicitlySharedDataPointer<Expression> > TemplateParameterHash;
   typedef QExplicitlySharedDataPointer<DynamicContext> Ptr;


   /**
    * This function intentionally returns by reference.
    *
    * @see globalItemCacheCell()
    */
   virtual ItemCacheCell &itemCacheCell(const VariableSlotID slot) = 0;

   /**
    * This function intentionally returns by reference.
    *
    * @see globalItemSequenceCacheCells
    */
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

   /**
    * @short Returns the item that @c fn:current() returns.
    *
    * Hence, this is not the focus, and very different from the focus.
    *
    * @see CurrentItemStore
    * @see CurrentFN
    */
   virtual Item currentItem() const = 0;

   DynamicContext::Ptr createFocus();
   DynamicContext::Ptr createStack();
   DynamicContext::Ptr createReceiverContext(QAbstractXmlReceiver *const receiver);

   /**
    * Whenever a tree gets built, this function is called. DynamicContext
    * has the responsibility of keeping a copy of @p nm, such that it
    * doesn't go out of scope, since no one else will reference @p nm.
    *
    * I think this is currently only used for temporary node trees. In
    * other cases they are stored in the ExternalResourceLoader.
    *
    * The caller guarantees that @p nm is not @c null.
    */
   virtual void addNodeModel(const QAbstractXmlNodeModel::Ptr &nm) = 0;

   /**
    * Same as itemCacheCell(), but is only used for global varibles. This
    * is needed because sometimes stack frames needs to be created for
    * other kinds of variables(such as in the case of user function
    * calls), while the global variable(s) needs to continue to use the
    * same cache, instead of one for each new stack frame, typically an
    * instance of StackContextBase.
    *
    * This has two effects:
    *
    * - It's an optimization. Instead of that a global variable gets evaluated each
    * time a user function is called, think recursive functions, it's done
    * only once.
    * - Query stability, hence affects things like node identity and
    * therefore conformance. Hence affects for instance what nodes a query
    * returns, since node identity affect node deduplication.
    */
   virtual ItemCacheCell &globalItemCacheCell(const VariableSlotID slot) = 0;

   /**
    * @short When a template is called, this member carries the template
    * parameters.
    *
    * Hence this is similar to the other variable stack functions such as
    * rangeVariable() and expressionVariable(), the difference being that
    * the order of template parameters as well as its arguments can appear
    * in arbitrary order. Hence the name is used to make the order
    * insignificant.
    */
   virtual TemplateParameterHash &templateParameterStore() = 0;

   /**
    * Same as itemSequenceCacheCells() but applies only for global
    * variables.
    *
    * @see globalItemCacheCell()
    */
   virtual ItemSequenceCacheCell::Vector &globalItemSequenceCacheCells(const VariableSlotID slot) = 0;

   /**
    * @short Returns the previous DynamicContext. If this context is the
    * top-level one, @c null is returned.
    */
   virtual DynamicContext::Ptr previousContext() const = 0;

   /**
    * @short Returns the current template mode that is in effect.
    *
    * If @c null is returned, it means that the default mode should be
    * used as the current mode.
    */
   virtual QExplicitlySharedDataPointer<TemplateMode> currentTemplateMode() const = 0;
};
}

#endif
