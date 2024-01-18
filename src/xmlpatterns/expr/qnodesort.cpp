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

#include <algorithm>

#include "qcommonsequencetypes_p.h"
#include "qdeduplicateiterator_p.h"
#include "qnodesort_p.h"

using namespace QPatternist;

NodeSortExpression::NodeSortExpression(const Expression::Ptr &op) : SingleContainer(op)
{
}

bool NodeSortExpression::lessThanUsingNodeModel(const Item &n1,
      const Item &n2)
{
   Q_ASSERT(n1.isNode());
   Q_ASSERT(n2.isNode());

   if (n1.asNode().model() == n2.asNode().model()) {
      return n1.asNode().compareOrder(n2.asNode()) == QXmlNodeModelIndex::Precedes;
   } else {
      /* The two nodes are from different trees. The sort order is implementation
       * defined, but it must be stable.
       *
       * We do this by looking at the pointer difference. The value means nothing,
       * but it is stable, and that's what we're looking for. */
      return n1.asNode().model() - n2.asNode().model() < 0;
   }
}

Item::Iterator::Ptr NodeSortExpression::evaluateSequence(const DynamicContext::Ptr &context) const
{
   Q_ASSERT_X(m_operand->staticType()->cardinality().allowsMany(), Q_FUNC_INFO,
              "It makes no sense to sort a single node.");

   Item::List nodes(m_operand->evaluateSequence(context)->toList());

   if (nodes.isEmpty()) {
      return CommonValues::emptyIterator;
   } else if (nodes.first().isAtomicValue()) {
      return makeListIterator(nodes);
   } else {
      std::sort(nodes.begin(), nodes.end(), lessThanUsingNodeModel);

      return Item::Iterator::Ptr(new DeduplicateIterator(nodes));
   }
}

Expression::Ptr NodeSortExpression::wrapAround(const Expression::Ptr &operand,
      const StaticContext::Ptr &context)
{
   Q_ASSERT(operand);
   Q_ASSERT(context);

   const Expression::Ptr sort(new NodeSortExpression(operand));
   context->wrapExpressionWith(operand.data(), sort);
   return sort;
}

Expression::Ptr NodeSortExpression::compress(const StaticContext::Ptr &context)
{
   const Expression::Ptr me(SingleContainer::compress(context));

   /* It make no sense to sort & deduplicate a single node. */
   if (m_operand->staticType()->cardinality().allowsMany()) {
      return me;
   } else {
      return m_operand;
   }
}

SequenceType::Ptr NodeSortExpression::staticType() const
{
   return m_operand->staticType();
}

SequenceType::List NodeSortExpression::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

ExpressionVisitorResult::Ptr
NodeSortExpression::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::Properties NodeSortExpression::properties() const
{
   /* The reason we disable elimination is that the assert for sorting a
    * single node in evaluateSequence() triggers unless our compress() routine
    * has been run. Anyhow, it's not that we would manage to write away anyway,
    * since the node source in most(all?) cases prevents it.
    */
   return AffectsOrderOnly | DisableElimination;
}
