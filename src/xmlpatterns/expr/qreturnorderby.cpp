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

#include "qcommonsequencetypes_p.h"
#include "qexpressionsequence_p.h"
#include "qsorttuple_p.h"
#include "qreturnorderby_p.h"

using namespace QPatternist;

ReturnOrderBy::ReturnOrderBy(const OrderBy::Stability aStability,
                             const OrderBy::OrderSpec::Vector &oSpecs,
                             const Expression::List &ops) : UnlimitedContainer(ops)
   , m_stability(aStability)
   , m_orderSpecs(oSpecs)
   , m_flyAway(true)
{
   Q_ASSERT_X(m_operands.size() >= 2, Q_FUNC_INFO,
              "ReturnOrderBy must have the return expression, and at least one sort key.");
   Q_ASSERT(m_orderSpecs.size() == ops.size() - 1);
}

Item ReturnOrderBy::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   Q_ASSERT(m_operands.size() > 1);
   const Item::Iterator::Ptr value(makeListIterator(m_operands.first()->evaluateSequence(context)->toList()));
   Item::Vector sortKeys;

   /* We're skipping the first operand. */
   const int len = m_operands.size() - 1;
   sortKeys.resize(len);

   for (int i = 1; i <= len; ++i) {
      sortKeys[i - 1] = m_operands.at(i)->evaluateSingleton(context);
   }

   return Item(new SortTuple(value, sortKeys));
}

bool ReturnOrderBy::evaluateEBV(const DynamicContext::Ptr &context) const
{
   // TODO This is temporary code.
   return m_operands.first()->evaluateEBV(context);
}

Expression::Ptr ReturnOrderBy::compress(const StaticContext::Ptr &context)
{
   /* We first did this in typeCheck(), but that broke due to that type checks were
    * missed, which other pieces relied on. */
   if (m_flyAway) {
      /* We only want the return expression, not the sort keys. */
      return m_operands.first()->compress(context);
   } else {
      /* We don't need the members, so don't keep a reference to them. */
      m_orderSpecs.clear();

      return UnlimitedContainer::compress(context);
   }
}

Expression::Properties ReturnOrderBy::properties() const
{
   /* For some unknown reason this is necessary for XQTS test case orderBy18. */
   return DisableElimination;
}

ExpressionVisitorResult::Ptr ReturnOrderBy::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

SequenceType::Ptr ReturnOrderBy::staticType() const
{
   return m_operands.first()->staticType();
}

SequenceType::List ReturnOrderBy::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   result.append(CommonSequenceTypes::ZeroOrOneAtomicType);
   return result;
}

Expression::ID ReturnOrderBy::id() const
{
   return IDReturnOrderBy;
}
