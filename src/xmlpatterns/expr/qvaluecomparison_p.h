/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef Patternist_ValueComparison_P_H
#define Patternist_ValueComparison_P_H

#include "qatomiccomparator_p.h"
#include "qpaircontainer_p.h"
#include "qcomparisonplatform_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {

class ValueComparison : public PairContainer,
   public ComparisonPlatform<ValueComparison, true>
{
 public:
   ValueComparison(const Expression::Ptr &op1,
                   const AtomicComparator::Operator op,
                   const Expression::Ptr &op2);

   virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);

   /**
    * @returns always CommonSequenceTypes::ExactlyOneBoolean
    */
   virtual SequenceType::Ptr staticType() const;

   virtual SequenceType::List expectedOperandTypes() const;

   /**
    * @returns IDValueComparison
    */
   virtual ID id() const;

   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
   virtual QList<QExplicitlySharedDataPointer<OptimizationPass> > optimizationPasses() const;

   /**
    * Overridden to optimize case-insensitive compares.
    */
   virtual Expression::Ptr compress(const StaticContext::Ptr &context);

   /**
    * @returns the operator that this ValueComparison is using.
    */
   inline AtomicComparator::Operator operatorID() const {
      return m_operator;
   }

   /**
    * It is considered that the string value from @p op1 will be compared against @p op2. This
    * function determines whether the user intends the comparison to be case insensitive. If
    * that is the case @c true is returned, and the operands are re-written appropriately.
    *
    * This is a helper function for Expression classes that compares strings.
    *
    * @see ComparisonPlatform::useCaseInsensitiveComparator()
    */
   static bool isCaseInsensitiveCompare(Expression::Ptr &op1, Expression::Ptr &op2);

 private:
   const AtomicComparator::Operator m_operator;
};
}

QT_END_NAMESPACE

#endif
