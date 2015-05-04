/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QArithmeticExpression_P_H
#define QArithmeticExpression_P_H

#include <qatomicmathematician_p.h>
#include <qpaircontainer_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
/**
 * @short Implements arithmetics, such as multiplication and subtraction.
 *
 *
 * Optimizations: there's some operator/value combos that are no ops. For
 * instance, 0 + <value>, which is the case of unary plus. We can't compile
 * those away early due to that type checks needs to be done but one can
 * check for them in compress().
 *
 * @see <a href="http://www.w3.org/TR/xpath20/#id-arithmetic">XML Path Language
 * (XPath) 2.0, 3.4 Arithmetic Expressions</a>
 * @author Frans Englich <frans.englich@nokia.com>
 * @ingroup Patternist_expressions
 */
class ArithmeticExpression : public PairContainer
{
 public:
   ArithmeticExpression(const Expression::Ptr &operand1,
                        const AtomicMathematician::Operator op,
                        const Expression::Ptr &operand2);

   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

   virtual SequenceType::Ptr staticType() const;
   virtual SequenceType::List expectedOperandTypes() const;
   AtomicMathematician::Operator operatorID() const;

   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);

   static Item flexiblyCalculate(const Item &op1,
                                 const AtomicMathematician::Operator op,
                                 const Item &op2,
                                 const AtomicMathematician::Ptr &mather,
                                 const DynamicContext::Ptr &context,
                                 const SourceLocationReflection *const reflection,
                                 const ReportContext::ErrorCode code = ReportContext::XPTY0004,
                                 const bool isCompat = false);

   static AtomicMathematician::Ptr
   fetchMathematician(Expression::Ptr &t1,
                      Expression::Ptr &t2,
                      const AtomicMathematician::Operator op,
                      const bool issueError,
                      const ReportContext::Ptr &context,
                      const SourceLocationReflection *const reflection,
                      const ReportContext::ErrorCode code = ReportContext::XPTY0004,
                      const bool isCompat = false);
   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

 protected:

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

QT_END_NAMESPACE

#endif
