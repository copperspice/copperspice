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

#ifndef QOrderBy_P_H
#define QOrderBy_P_H

#include <qatomiccomparator_p.h>
#include <qcomparisonplatform_p.h>
#include <qsinglecontainer_p.h>

namespace QPatternist {

class ReturnOrderBy;

class OrderBy : public SingleContainer
{
 public:
   enum Stability {
      StableOrder,
      UnstableOrder
   };

   /**
    * This class is value based.
    */
   class OrderSpec : public ComparisonPlatform<OrderBy::OrderSpec,
      true, /* Yes, issue errors. */
      AtomicComparator::AsValueComparison>
   {
    public:
      /**
       * We want this guy to be public.
       */
      using ComparisonPlatform<OrderBy::OrderSpec, true, AtomicComparator::AsValueComparison>::detailedFlexibleCompare;

      typedef QVector<OrderSpec> Vector;

      enum Direction {
         Ascending,
         Descending
      };

      /**
       * @short Default constructor, which is needed by QVector.
       */
      inline OrderSpec() {
      }

      inline OrderSpec(const Direction dir,
                       const StaticContext::OrderingEmptySequence orderingEmpty) : direction(dir),
         orderingEmptySequence(orderingEmpty) {
      }

      void prepare(const Expression::Ptr &source,
                   const StaticContext::Ptr &context);

      const SourceLocationReflection *actualReflection() const {
         return m_expr.data();
      }

    private:
      Expression::Ptr m_expr;

    public:
      /**
       * We place these afterwards, such that m_expr gets aligned at the
       * start of the address.
       */
      Direction direction;

      StaticContext::OrderingEmptySequence orderingEmptySequence;

      inline AtomicComparator::Operator operatorID() const {
         return orderingEmptySequence == StaticContext::Least ? AtomicComparator::OperatorLessThanNaNLeast
                : AtomicComparator::OperatorLessThanNaNGreatest;
      }

   };

   OrderBy(const Stability stability,
           const OrderSpec::Vector &orderSpecs,
           const Expression::Ptr &operand,
           ReturnOrderBy *const returnOrderBy);

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   SequenceType::Ptr staticType() const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;
   Expression::Ptr compress(const StaticContext::Ptr &context) override;
   SequenceType::List expectedOperandTypes() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   inline Item::Iterator::Ptr mapToSequence(const Item &i, const DynamicContext::Ptr &context) const;
   Properties properties() const override;

 private:
   /**
    * Needed when calling makeSequenceMappingIterator().
    */
   typedef QExplicitlySharedDataPointer<const OrderBy> ConstPtr;

   const Stability             m_stability;
   OrderSpec::Vector           m_orderSpecs;
   ReturnOrderBy *const        m_returnOrderBy;
};

}

#endif
