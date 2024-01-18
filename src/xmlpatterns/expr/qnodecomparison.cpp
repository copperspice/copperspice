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
#include "qcommonvalues_p.h"
#include "qemptysequence_p.h"
#include "qinteger_p.h"
#include "qschemanumeric_p.h"
#include "qrangeiterator_p.h"

#include "qnodecomparison_p.h"

using namespace QPatternist;

NodeComparison::NodeComparison(const Expression::Ptr &operand1,
                               const QXmlNodeModelIndex::DocumentOrder op,
                               const Expression::Ptr &operand2)
   : PairContainer(operand1, operand2)
   , m_op(op)
{
   Q_ASSERT(op == QXmlNodeModelIndex::Precedes   ||
            op == QXmlNodeModelIndex::Follows    ||
            op == QXmlNodeModelIndex::Is);
}

Item NodeComparison::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   switch (evaluate(context)) {
      case True:
         return CommonValues::BooleanTrue;
      case False:
         return CommonValues::BooleanFalse;
      default:
         return Item();
   }
}

bool NodeComparison::evaluateEBV(const DynamicContext::Ptr &context) const
{
   switch (evaluate(context)) {
      case True:
         return true;
      default:
         /* We include the empty sequence here. */
         return false;
   }
}

NodeComparison::Result NodeComparison::evaluate(const DynamicContext::Ptr &context) const
{
   const Item op1(m_operand1->evaluateSingleton(context));
   if (!op1) {
      return Empty;
   }

   const Item op2(m_operand2->evaluateSingleton(context));
   if (!op2) {
      return Empty;
   }

   /* We just returns an arbitrary value, since there's no order defined for nodes from different
    * models, except for that the return value must be stable. */
   if (op1.asNode().model() != op2.asNode().model()) {
      return False;
   }

   switch (m_op) {
      case QXmlNodeModelIndex::Is:
         return op1.asNode().is(op2.asNode()) ? True : False;
      case QXmlNodeModelIndex::Precedes:
         return op1.asNode().compareOrder(op2.asNode()) == QXmlNodeModelIndex::Precedes ? True : False;
      default: {
         Q_ASSERT(m_op == QXmlNodeModelIndex::Follows);
         return op1.asNode().compareOrder(op2.asNode()) == QXmlNodeModelIndex::Follows ? True : False;
      }
   }
}


SequenceType::List NodeComparison::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrOneNode);
   result.append(CommonSequenceTypes::ZeroOrOneNode);
   return result;
}

Expression::Ptr NodeComparison::compress(const StaticContext::Ptr &context)
{
   const Expression::Ptr me(PairContainer::compress(context));

   if (me != this)
      /* We're already rewritten. */
   {
      return me;
   }

   if (m_operand1->staticType()->cardinality().isEmpty() ||
         m_operand2->staticType()->cardinality().isEmpty()) {
      // TODO issue a warning in the @p context saying that one of the operands
      // were empty, and that the expression always result in the empty sequence
      // (which never is the intent, right?).
      return EmptySequence::create(this, context);
   }

   return Expression::Ptr(this);
}

QString NodeComparison::displayName(const QXmlNodeModelIndex::DocumentOrder op)
{
   switch (op) {
      case QXmlNodeModelIndex::Is:
         return QLatin1String("is");
      case QXmlNodeModelIndex::Precedes:
         return QLatin1String("<<");
      default: {
         Q_ASSERT(op == QXmlNodeModelIndex::Follows);
         return QLatin1String(">>");
      }
   }
}

SequenceType::Ptr NodeComparison::staticType() const
{
   if (m_operand1->staticType()->cardinality().allowsEmpty() ||
         m_operand2->staticType()->cardinality().allowsEmpty()) {
      return CommonSequenceTypes::ZeroOrOneBoolean;
   } else {
      return CommonSequenceTypes::ExactlyOneBoolean;
   }
}

QXmlNodeModelIndex::DocumentOrder NodeComparison::operatorID() const
{
   return m_op;
}

ExpressionVisitorResult::Ptr NodeComparison::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
