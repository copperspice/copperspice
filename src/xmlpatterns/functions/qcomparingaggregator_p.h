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

#ifndef QComparingAggregator_P_H
#define QComparingAggregator_P_H

#include <qabstractfloat_p.h>
#include <qdecimal_p.h>
#include <qcastingplatform_p.h>
#include <qcomparisonplatform_p.h>
#include <qliteral_p.h>
#include <qaggregator_p.h>
#include <quntypedatomicconverter_p.h>
#include <qpatternistlocale_p.h>

namespace QPatternist {

template <AtomicComparator::Operator oper, AtomicComparator::ComparisonResult result>
class ComparingAggregator : public Aggregator,
   public ComparisonPlatform<ComparingAggregator<oper, result>,
   true, AtomicComparator::AsValueComparison, ReportContext::FORG0006>,
   public CastingPlatform<ComparingAggregator<oper, result>, true>
{
 public:
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   AtomicComparator::Operator operatorID() const {
      return oper;
   }

   ItemType::Ptr targetType() const {
      return BuiltinTypes::xsDouble;
   }

 private:
   inline Item applyNumericPromotion(const Item &old, const Item &nev, const Item &newVal) const;

   using ComparisonPlatform<ComparingAggregator<oper, result>,
         true,
         AtomicComparator::AsValueComparison,
         ReportContext::FORG0006>::comparator;

   using ComparisonPlatform<ComparingAggregator<oper, result>,
         true,
         AtomicComparator::AsValueComparison,
         ReportContext::FORG0006>::fetchComparator;
   using CastingPlatform<ComparingAggregator<oper, result>, true>::cast;
};

#include "qcomparingaggregator.cpp"

typedef ComparingAggregator<AtomicComparator::OperatorGreaterThan, AtomicComparator::GreaterThan> MaxFN;
typedef ComparingAggregator<AtomicComparator::OperatorLessThan, AtomicComparator::LessThan> MinFN;
}

#endif
