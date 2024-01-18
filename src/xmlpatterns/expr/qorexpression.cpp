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
#include "qcommonvalues_p.h"
#include "qliteral_p.h"

#include "qorexpression_p.h"

using namespace QPatternist;

OrExpression::OrExpression(const Expression::Ptr &operand1,
                           const Expression::Ptr &operand2) : AndExpression(operand1, operand2)
{
}

bool OrExpression::evaluateEBV(const DynamicContext::Ptr &context) const
{
   return m_operand1->evaluateEBV(context) || m_operand2->evaluateEBV(context);
}

Expression::Ptr OrExpression::compress(const StaticContext::Ptr &context)
{
   const Expression::Ptr newMe(PairContainer::compress(context));

   if (newMe != this) {
      return newMe;
   }

   /* Both operands mustn't be evaluated in order to be able to compress. */
   if (m_operand1->isEvaluated() && m_operand1->evaluateEBV(context->dynamicContext())) {
      return wrapLiteral(CommonValues::BooleanTrue, context, this);
   } else if (m_operand2->isEvaluated() && m_operand2->evaluateEBV(context->dynamicContext())) {
      return wrapLiteral(CommonValues::BooleanTrue, context, this);
   } else {
      return Expression::Ptr(this);
   }
}

ExpressionVisitorResult::Ptr OrExpression::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
