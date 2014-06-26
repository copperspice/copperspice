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

QT_BEGIN_NAMESPACE

namespace QPatternist {

template <AtomicComparator::Operator oper, AtomicComparator::ComparisonResult result>
class ComparingAggregator : public Aggregator,
   public ComparisonPlatform<ComparingAggregator<oper, result>,
   true, AtomicComparator::AsValueComparison, ReportContext::FORG0006>,
   public CastingPlatform<ComparingAggregator<oper, result>, true>
{
 public:
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);

   inline AtomicComparator::Operator operatorID() const {
      return oper;
   }

   inline ItemType::Ptr targetType() const {
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

QT_END_NAMESPACE

#endif
