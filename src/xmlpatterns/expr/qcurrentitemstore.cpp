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

#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qcurrentitemcontext_p.h"
#include "qstaticcurrentcontext_p.h"

#include "qcurrentitemstore_p.h"

using namespace QPatternist;

CurrentItemStore::CurrentItemStore(const Expression::Ptr &operand) : SingleContainer(operand)
{
}

DynamicContext::Ptr CurrentItemStore::createContext(const DynamicContext::Ptr &old) const
{
   return DynamicContext::Ptr(new CurrentItemContext(old->contextItem(), old));
}

bool CurrentItemStore::evaluateEBV(const DynamicContext::Ptr &context) const
{
   return m_operand->evaluateEBV(createContext(context));
}

Item::Iterator::Ptr CurrentItemStore::evaluateSequence(const DynamicContext::Ptr &context) const
{
   return m_operand->evaluateSequence(createContext(context));
}

Item CurrentItemStore::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return m_operand->evaluateSingleton(createContext(context));
}

SequenceType::Ptr CurrentItemStore::staticType() const
{
   return m_operand->staticType();
}

SequenceType::List CurrentItemStore::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

StaticContext::Ptr CurrentItemStore::newStaticContext(const StaticContext::Ptr &context)
{
   /* It might be we are generated despite there is no focus. In that case
    * an error will reported in case current() is used, but in any case we cannot
    * crash, so use item() in case we have no focus.
    *
    * Such a case is when we're inside a named template, and it's invoked
    * without focus. */
   const ItemType::Ptr t(context->contextItemType());
   return StaticContext::Ptr(new StaticCurrentContext(t ? t : BuiltinTypes::item, context));
}

Expression::Ptr CurrentItemStore::compress(const StaticContext::Ptr &context)
{
   const Expression::Ptr me(SingleContainer::compress(newStaticContext(context)));

   if (me != this) {
      return me;
   } else {
      /* If fn:current() isn't called, there's no point in us sticking
       * around. */
      if (m_operand->deepProperties().testFlag(RequiresCurrentItem)) {
         return me;
      } else {
         return m_operand;
      }
   }
}

Expression::Ptr CurrentItemStore::typeCheck(const StaticContext::Ptr &context,
      const SequenceType::Ptr &reqType)
{
   return SingleContainer::typeCheck(newStaticContext(context), reqType);
}

ExpressionVisitorResult::Ptr CurrentItemStore::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

const SourceLocationReflection *CurrentItemStore::actualReflection() const
{
   return m_operand->actualReflection();
}

Expression::Properties CurrentItemStore::properties() const
{
   return m_operand->properties() & (RequiresFocus | IsEvaluated | DisableElimination);
}
