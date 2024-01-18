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

#ifndef QGeneralComparison_P_H
#define QGeneralComparison_P_H

#include <qatomiccomparator_p.h>
#include <qpaircontainer_p.h>
#include <qcomparisonplatform_p.h>

namespace QPatternist {

class GeneralComparison : public PairContainer, public ComparisonPlatform<GeneralComparison,
   true /* We want to report errors. */, AtomicComparator::AsGeneralComparison>
{
 public:
   GeneralComparison(const Expression::Ptr &op1,
                     const AtomicComparator::Operator op,
                     const Expression::Ptr &op2,
                     const bool isBackwardsCompat = false);

   bool evaluateEBV(const DynamicContext::Ptr &) const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   /**
    * @returns always IDGeneralComparison
    */
   ID id() const override;

   QList<QExplicitlySharedDataPointer<OptimizationPass> > optimizationPasses() const override;

   /**
    * @returns the operator that this GeneralComparison is using.
    */
   AtomicComparator::Operator operatorID() const {
      return m_operator;
   }

   /**
    * Overridden to optimize case-insensitive compares.
    */
   Expression::Ptr compress(const StaticContext::Ptr &context) override;

 private:
   static inline void updateType(ItemType::Ptr &type, const Expression::Ptr &source);

   AtomicComparator::Ptr fetchGeneralComparator(Expression::Ptr &op1,
         Expression::Ptr &op2, const ReportContext::Ptr &context) const;

   bool generalCompare(const Item &op1, const Item &op2, const DynamicContext::Ptr &context) const;

   const AtomicComparator::Operator m_operator;
   const bool m_isBackwardsCompat;
};
}

#endif
