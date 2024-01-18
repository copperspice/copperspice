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

#ifndef QArithmeticExpression_P_H
#define QArithmeticExpression_P_H

#include <qatomicmathematician_p.h>
#include <qpaircontainer_p.h>

namespace QPatternist {

class ArithmeticExpression : public PairContainer
{
 public:
   ArithmeticExpression(const Expression::Ptr &operand1, const AtomicMathematician::Operator op,
                  const Expression::Ptr &operand2);

   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

   SequenceType::Ptr staticType() const override;
   SequenceType::List expectedOperandTypes() const override;
   AtomicMathematician::Operator operatorID() const;

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   static Item flexiblyCalculate(const Item &op1, const AtomicMathematician::Operator op,
                                 const Item &op2, const AtomicMathematician::Ptr &mather,
                                 const DynamicContext::Ptr &context,
                                 const SourceLocationReflection *const reflection,
                                 const ReportContext::ErrorCode code = ReportContext::XPTY0004,
                                 const bool isCompat = false);

   static AtomicMathematician::Ptr fetchMathematician(Expression::Ptr &t1,
                      Expression::Ptr &t2, const AtomicMathematician::Operator op,
                      const bool issueError, const ReportContext::Ptr &context,
                      const SourceLocationReflection *const reflection,
                      const ReportContext::ErrorCode code = ReportContext::XPTY0004,
                      const bool isCompat = false);

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

 private:
   const AtomicMathematician::Operator m_op;
   AtomicMathematician::Ptr m_mather;
   bool m_isCompat;
};

inline AtomicMathematician::Operator ArithmeticExpression::operatorID() const
{
   return m_op;
}

}

#endif
