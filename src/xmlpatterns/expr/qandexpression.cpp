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

#include "qboolean_p.h"
#include "qcommonsequencetypes_p.h"
#include "qcommonvalues_p.h"
#include "qliteral_p.h"

#include "qandexpression_p.h"

using namespace QPatternist;

AndExpression::AndExpression(const Expression::Ptr &operand1,
                             const Expression::Ptr &operand2) : PairContainer(operand1, operand2)
{
}

bool AndExpression::evaluateEBV(const DynamicContext::Ptr &context) const
{
   return m_operand1->evaluateEBV(context) && m_operand2->evaluateEBV(context);
}

Expression::Ptr AndExpression::compress(const StaticContext::Ptr &context)
{
   const Expression::Ptr newMe(PairContainer::compress(context));

   if (newMe != this) {
      return newMe;
   }

   /* Both operands mustn't be evaluated in order to be able to compress. */
   if (m_operand1->isEvaluated() && !m_operand1->evaluateEBV(context->dynamicContext())) {
      return wrapLiteral(CommonValues::BooleanFalse, context, this);
   } else if (m_operand2->isEvaluated() && !m_operand2->evaluateEBV(context->dynamicContext())) {
      return wrapLiteral(CommonValues::BooleanFalse, context, this);
   } else {
      return Expression::Ptr(this);
   }
}

SequenceType::List AndExpression::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::EBV);
   result.append(CommonSequenceTypes::EBV);
   return result;
}

SequenceType::Ptr AndExpression::staticType() const
{
   return CommonSequenceTypes::ExactlyOneBoolean;
}

ExpressionVisitorResult::Ptr AndExpression::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
