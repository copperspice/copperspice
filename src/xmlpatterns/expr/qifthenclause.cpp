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
#include "qgenericsequencetype_p.h"
#include "qoptimizationpasses_p.h"
#include "qifthenclause_p.h"

using namespace QPatternist;

IfThenClause::IfThenClause(const Expression::Ptr &test,
                           const Expression::Ptr &then,
                           const Expression::Ptr &el) : TripleContainer(test, then, el)
{
}

Item::Iterator::Ptr IfThenClause::evaluateSequence(const DynamicContext::Ptr &context) const
{
   return m_operand1->evaluateEBV(context)
          ? m_operand2->evaluateSequence(context)
          : m_operand3->evaluateSequence(context);
}

Item IfThenClause::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return m_operand1->evaluateEBV(context)
          ? m_operand2->evaluateSingleton(context)
          : m_operand3->evaluateSingleton(context);
}

bool IfThenClause::evaluateEBV(const DynamicContext::Ptr &context) const
{
   return m_operand1->evaluateEBV(context)
          ? m_operand2->evaluateEBV(context)
          : m_operand3->evaluateEBV(context);
}

void IfThenClause::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
   if (m_operand1->evaluateEBV(context)) {
      m_operand2->evaluateToSequenceReceiver(context);
   } else {
      m_operand3->evaluateToSequenceReceiver(context);
   }
}

Expression::Ptr IfThenClause::compress(const StaticContext::Ptr &context)
{
   const Expression::Ptr me(TripleContainer::compress(context));

   if (me != this) {
      return me;
   }

   /* All operands mustn't be evaluated in order for const folding to
    * be possible. Let's see how far we get. */

   if (m_operand1->isEvaluated()) {
      if (m_operand1->evaluateEBV(context->dynamicContext())) {
         return m_operand2;
      } else {
         return m_operand3;
      }
   } else {
      return me;
   }
}

QList<QExplicitlySharedDataPointer<OptimizationPass> > IfThenClause::optimizationPasses() const
{
   return OptimizationPasses::ifThenPasses;
}

SequenceType::List IfThenClause::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::EBV);
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

SequenceType::Ptr IfThenClause::staticType() const
{
   const SequenceType::Ptr t1(m_operand2->staticType());
   const SequenceType::Ptr t2(m_operand3->staticType());

   return makeGenericSequenceType(t1->itemType() | t2->itemType(),
                                  t1->cardinality() | t2->cardinality());
}

ExpressionVisitorResult::Ptr IfThenClause::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::ID IfThenClause::id() const
{
   return IDIfThenClause;
}

/*
Expression::Properties IfThenClause::properties() const
{
    return   m_operand1->properties()
           | m_operand2->properties()
           | m_operand3->properties()
           & (  Expression::RequiresFocus
              | Expression::IsEvaluated
              | Expression::DisableElimination);
}
*/
