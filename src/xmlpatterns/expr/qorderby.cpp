/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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
#include "qnodebuilder_p.h"
#include "qschemanumeric_p.h"
#include "qpatternistlocale_p.h"
#include "qreturnorderby_p.h"
#include "qsorttuple_p.h"
#include "qsequencemappingiterator_p.h"

#include "qorderby_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

OrderBy::OrderBy(const Stability stability,
                 const OrderSpec::Vector &aOrderSpecs,
                 const Expression::Ptr &op,
                 ReturnOrderBy *const returnOrderBy) : SingleContainer(op)
   , m_stability(stability)
   , m_orderSpecs(aOrderSpecs)
   , m_returnOrderBy(returnOrderBy)
{
   Q_ASSERT(m_returnOrderBy);
}

void OrderBy::OrderSpec::prepare(const Expression::Ptr &source, const StaticContext::Ptr &context)
{
   m_expr = source;
   const ItemType::Ptr t(source->staticType()->itemType());
   prepareComparison(fetchComparator(t, t, context));
}

namespace std {

template<>
class less<Item::List>
{
 private:

   static inline bool isNaN(const Item &i) {
      return BuiltinTypes::xsDouble->xdtTypeMatches(i.type()) &&
             i.as<Numeric>()->isNaN();
   }

 public:
   inline less(const OrderBy::OrderSpec::Vector &orderspecs, const DynamicContext::Ptr &context)
      : m_orderSpecs(orderspecs), m_context(context) {

      Q_ASSERT(!m_orderSpecs.isEmpty());
      Q_ASSERT(context);
   }

   inline bool operator()(const Item &item1, const Item &item2) const {
      const SortTuple *const s1 = item1.as<SortTuple>();
      const SortTuple *const s2 = item2.as<SortTuple>();

      const Item::Vector &sortKeys1 = s1->sortKeys();
      const Item::Vector &sortKeys2 = s2->sortKeys();
      const int len = sortKeys1.count();
      Q_ASSERT(sortKeys1.count() == sortKeys2.count());

      for (int i = 0; i < len; ++i) {
         const Item &i1 = sortKeys1.at(i);
         const Item &i2 = sortKeys2.at(i);
         const OrderBy::OrderSpec &orderSpec = m_orderSpecs.at(i);

         if (!i1) {
            if (i2 && !isNaN(i2)) {
               /* We got ((), item()). */
               return orderSpec.orderingEmptySequence == StaticContext::Least ? orderSpec.direction == OrderBy::OrderSpec::Ascending
                      : orderSpec.direction != OrderBy::OrderSpec::Ascending;
            } else {
               return false;
            }
         }

         if (!i2) {
            if (i1 && !isNaN(i1))
               /* We got (item(), ()). */
               return orderSpec.orderingEmptySequence == StaticContext::Greatest ? orderSpec.direction == OrderBy::OrderSpec::Ascending
                      : orderSpec.direction != OrderBy::OrderSpec::Ascending;
            else {
               return false;
            }
         }

         Q_ASSERT(orderSpec.direction == OrderBy::OrderSpec::Ascending ||
                  orderSpec.direction == OrderBy::OrderSpec::Descending);
         const AtomicComparator::ComparisonResult result = orderSpec.detailedFlexibleCompare(i1, i2, m_context);

         switch (result) {
            case AtomicComparator::LessThan:
               return orderSpec.direction == OrderBy::OrderSpec::Ascending;
            case AtomicComparator::GreaterThan:
               return orderSpec.direction != OrderBy::OrderSpec::Ascending;
            case AtomicComparator::Equal:
               continue;

            case AtomicComparator::Incomparable:
               Q_ASSERT_X(false, Q_FUNC_INFO, "These values must be comparable.");
         }
      }

      return false;
   }

 private:
   /* Yes, we store references here. */
   const OrderBy::OrderSpec::Vector &m_orderSpecs;
   const DynamicContext::Ptr &m_context;
};

}

Item::Iterator::Ptr OrderBy::mapToSequence(const Item &i,
      const DynamicContext::Ptr &) const
{
   return i.as<SortTuple>()->value();
}

Item::Iterator::Ptr OrderBy::evaluateSequence(const DynamicContext::Ptr &context) const
{
   Item::List tuples(m_operand->evaluateSequence(context)->toList());
   const std::less<Item::List> sorter(m_orderSpecs, context);

   Q_ASSERT(m_stability == StableOrder || m_stability == UnstableOrder);

   if (m_stability == StableOrder) {
      std::stable_sort(tuples.begin(), tuples.end(), sorter);

   } else {
      std::sort(tuples.begin(), tuples.end(), sorter);
   }

   return makeSequenceMappingIterator<Item>(ConstPtr(this), makeListIterator(tuples), context);
}

Expression::Ptr OrderBy::typeCheck(const StaticContext::Ptr &context,
                                   const SequenceType::Ptr &reqType)
{
   m_returnOrderBy->setStay(true);

   /* It's important we do the typeCheck() before calling OrderSpec::prepare(), since
    * atomizers must first be inserted. */
   const Expression::Ptr me(SingleContainer::typeCheck(context, reqType));

   const Expression::List ops(m_returnOrderBy->operands());
   const int len = ops.count();
   Q_ASSERT(ops.count() > 1);
   Q_ASSERT(m_orderSpecs.count() == ops.count() - 1);

   for (int i = 1; i < len; ++i) {
      m_orderSpecs[i - 1].prepare(ops.at(i), context);
   }

   return me;

   /* It's not meaningful to sort a single item or less, so rewrite ourselves
    * away if that is the case. This is an optimization. */
   /* TODO: How do we remove ReturnOrderBy?
   if(Cardinality::zeroOrOne().isMatch(m_operand->staticType()->cardinality()))
       return m_operand->typeCheck(context, reqType);
   else
       return SingleContainer::typeCheck(context, reqType);
    */
}

Expression::Properties OrderBy::properties() const
{
   return m_operand->properties() & DisableElimination;
}

Expression::Ptr OrderBy::compress(const StaticContext::Ptr &context)
{
   /* If we only will produce one item, there's no point in sorting. */
   if (m_operand->staticType()->cardinality().allowsMany()) {
      return SingleContainer::compress(context);
   } else {
      m_returnOrderBy->setStay(false);
      return m_operand->compress(context);
   }
}

SequenceType::Ptr OrderBy::staticType() const
{
   return m_operand->staticType();
}

SequenceType::List OrderBy::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

ExpressionVisitorResult::Ptr
OrderBy::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

QT_END_NAMESPACE
