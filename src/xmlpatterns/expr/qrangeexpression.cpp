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

#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qcommonvalues_p.h"
#include "qgenericsequencetype_p.h"
#include "qinteger_p.h"
#include "qliteral_p.h"
#include "qrangeiterator_p.h"

#include "qrangeexpression_p.h"

using namespace QPatternist;

RangeExpression::RangeExpression(const Expression::Ptr &operand1,
                                 const Expression::Ptr &operand2) : PairContainer(operand1, operand2)
{
}

Item::Iterator::Ptr RangeExpression::evaluateSequence(const DynamicContext::Ptr &context) const
{
   const Item s(m_operand1->evaluateSingleton(context));

   if (!s) {
      return CommonValues::emptyIterator;
   }

   const Item e(m_operand2->evaluateSingleton(context));
   if (!e) {
      return CommonValues::emptyIterator;
   }

   const xsInteger start = s.as<Numeric>()->toInteger();
   const xsInteger end = e.as<Numeric>()->toInteger();

   if (start > end) {
      return CommonValues::emptyIterator;
   } else if (start == end) {
      return makeSingletonIterator(s);
   } else {
      return Item::Iterator::Ptr(new RangeIterator(start, RangeIterator::Forward, end));
   }
}

Item RangeExpression::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return m_operand1->evaluateSingleton(context);
}

SequenceType::List RangeExpression::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrOneInteger);
   result.append(CommonSequenceTypes::ZeroOrOneInteger);
   return result;
}

SequenceType::Ptr RangeExpression::staticType() const
{
   /* This logic makes the Cardinality more specific. */
   Cardinality::Count from;
   bool hasFrom;

   if (m_operand1->is(IDIntegerValue)) {
      from = m_operand1->as<Literal>()->item().as<Integer>()->toInteger();
      hasFrom = true;
   } else {
      hasFrom = false;
      from = 0;
   }

   /* We can't check whether to is -1 since maybe the user wrote -1. Hence
    * hasTo is required. */
   bool hasTo;
   Cardinality::Count to;

   if (m_operand2->is(IDIntegerValue)) {
      const xsInteger asInt = m_operand2->as<Literal>()->item().as<Integer>()->toInteger();
      to = asInt;

      if (to == asInt) {
         hasTo = true;
      } else {
         /* Cardinality::Count is not the same as type xsInteger. We had overflow. */
         to = -1;
         hasTo = false;
      }
   } else {
      to = -1;
      hasTo = false;
   }

   if (hasTo && hasFrom) {
      if (from > to) {
         /* The query is incorrectly written, we'll evaluate to the empty sequence.
          * Just return what's correct. */
         return CommonSequenceTypes::ZeroOrMoreIntegers;
      } else {
         Cardinality::Count count = (to - from) + 1; /* + 1, since it's inclusive. */
         return makeGenericSequenceType(BuiltinTypes::xsInteger, Cardinality::fromExact(count));
      }
   } else {
      /* We can't do fromExact(from, -1) since the latter can evaluate to a value that actually is
       * lower than from, although that unfortunately is very unlikely. */
      return CommonSequenceTypes::ZeroOrMoreIntegers;
   }
}

Expression::Properties RangeExpression::properties() const
{
   return Expression::DisableElimination;
}

ExpressionVisitorResult::Ptr RangeExpression::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
