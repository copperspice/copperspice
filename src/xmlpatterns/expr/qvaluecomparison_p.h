/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QValueComparison_P_H
#define QValueComparison_P_H

#include <qatomiccomparator_p.h>
#include <qpaircontainer_p.h>
#include <qcomparisonplatform_p.h>

namespace QPatternist {

class ValueComparison : public PairContainer,
   public ComparisonPlatform<ValueComparison, true>
{
 public:
   ValueComparison(const Expression::Ptr &op1, const AtomicComparator::Operator op, const Expression::Ptr &op2);

   Item evaluateSingleton(const DynamicContext::Ptr &) const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   SequenceType::Ptr staticType() const override;
   SequenceType::List expectedOperandTypes() const override;

   ID id() const override;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   QList<QExplicitlySharedDataPointer<OptimizationPass> > optimizationPasses() const override;

   Expression::Ptr compress(const StaticContext::Ptr &context) override;

   AtomicComparator::Operator operatorID() const {
      return m_operator;
   }

   static bool isCaseInsensitiveCompare(Expression::Ptr &op1, Expression::Ptr &op2);

 private:
   const AtomicComparator::Operator m_operator;
};

}

#endif
