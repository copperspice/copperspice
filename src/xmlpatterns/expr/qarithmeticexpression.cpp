/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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
#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qemptysequence_p.h"
#include "qgenericsequencetype_p.h"
#include "qliteral_p.h"
#include "qpatternistlocale_p.h"
#include "qschemanumeric_p.h"
#include "quntypedatomicconverter_p.h"

#include "qarithmeticexpression_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

ArithmeticExpression::ArithmeticExpression(const Expression::Ptr &op1,
      const AtomicMathematician::Operator op,
      const Expression::Ptr &op2) : PairContainer(op1, op2)
   , m_op(op)
   , m_isCompat(false)
{
}

Item ArithmeticExpression::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item op1(m_operand1->evaluateSingleton(context));
   if (!op1) {
      return Item();
   }

   const Item op2(m_operand2->evaluateSingleton(context));
   if (!op2) {
      return Item();
   }

   return flexiblyCalculate(op1, m_op, op2, m_mather, context, this,
                            ReportContext::XPTY0004, m_isCompat);
}

class DelegatingReflectionExpression : public Literal
{
 public:
   DelegatingReflectionExpression(const Item &item,
                                  const SourceLocationReflection *const reflection) : Literal(item)
      , m_reflection(reflection) {
   }

   const SourceLocationReflection *actualReflection() const override {
      return m_reflection;
   }

 private:
   const SourceLocationReflection *const m_reflection;
};

Item ArithmeticExpression::flexiblyCalculate(const Item &op1,
      const AtomicMathematician::Operator op,
      const Item &op2,
      const AtomicMathematician::Ptr &mather,
      const DynamicContext::Ptr &context,
      const SourceLocationReflection *const reflection,
      const ReportContext::ErrorCode code,
      const bool isCompat)
{
   if (mather) {
      return mather->calculate(op1, op, op2, context);
   }

   /* This is a very heavy code path. */
   Expression::Ptr a1(new DelegatingReflectionExpression(op1, reflection));
   Expression::Ptr a2(new DelegatingReflectionExpression(op2, reflection));

   const AtomicMathematician::Ptr ingela(fetchMathematician(a1, a2, op, true, context, reflection, code, isCompat));

   return ingela->calculate(a1->evaluateSingleton(context), op,
                            a2->evaluateSingleton(context), context);
}

Expression::Ptr ArithmeticExpression::typeCheck(const StaticContext::Ptr &context,
      const SequenceType::Ptr &reqType)
{
   m_isCompat = context->compatModeEnabled();

   const Expression::Ptr me(PairContainer::typeCheck(context, reqType));
   const ItemType::Ptr t1(m_operand1->staticType()->itemType());
   const ItemType::Ptr t2(m_operand2->staticType()->itemType());

   if (*CommonSequenceTypes::Empty == *t1 ||
         *CommonSequenceTypes::Empty == *t2) {
      return EmptySequence::create(this, context);
   }

   if (*BuiltinTypes::xsAnyAtomicType == *t1    ||
         *BuiltinTypes::xsAnyAtomicType == *t2    ||
         *BuiltinTypes::numeric == *t1            ||
         *BuiltinTypes::numeric == *t2) {
      /* The static type of (at least) one of the operands could not
       * be narrowed further than xs:anyAtomicType, so we do the operator
       * lookup at runtime. */
      return me;
   }

   m_mather = fetchMathematician(m_operand1, m_operand2, m_op, true, context, this,
                                 ReportContext::XPTY0004, m_isCompat);

   return me;
}

AtomicMathematician::Ptr
ArithmeticExpression::fetchMathematician(Expression::Ptr &op1,
      Expression::Ptr &op2,
      const AtomicMathematician::Operator op,
      const bool issueError,
      const ReportContext::Ptr &context,
      const SourceLocationReflection *const reflection,
      const ReportContext::ErrorCode code,
      const bool isCompat)
{
   ItemType::Ptr t1(op1->staticType()->itemType());
   ItemType::Ptr t2(op2->staticType()->itemType());

   if (BuiltinTypes::xsUntypedAtomic->xdtTypeMatches(t1)
         || (isCompat && (BuiltinTypes::xsString->xdtTypeMatches(t1)
                          || BuiltinTypes::xsDecimal->xdtTypeMatches(t1)))) {
      op1 = Expression::Ptr(new UntypedAtomicConverter(op1, BuiltinTypes::xsDouble));
      /* The types might have changed, reload. */
      t1 = op1->staticType()->itemType();
   }

   if (BuiltinTypes::xsUntypedAtomic->xdtTypeMatches(t2)
         || (isCompat && (BuiltinTypes::xsString->xdtTypeMatches(t1)
                          || BuiltinTypes::xsDecimal->xdtTypeMatches(t1)))) {
      op2 = Expression::Ptr(new UntypedAtomicConverter(op2, BuiltinTypes::xsDouble));
      /* The types might have changed, reload. */
      t2 = op2->staticType()->itemType();
   }

   const AtomicMathematicianLocator::Ptr locator
   (static_cast<const AtomicType *>(t1.data())->mathematicianLocator());

   if (!locator) {
      if (!issueError) {
         return AtomicMathematician::Ptr();
      }

      context->error(QtXmlPatterns::tr("Operator %1 cannot be used on type %2.")
                     .formatArg(formatKeyword(AtomicMathematician::displayName(op)))
                     .formatArg(formatType(context->namePool(), t1)), code, reflection);

      return AtomicMathematician::Ptr();
   }

   const AtomicMathematician::Ptr comp
   (static_cast<const AtomicType *>(t2.data())->accept(locator, op, reflection));

   if (comp) {
      return comp;
   }

   if (!issueError) {
      return AtomicMathematician::Ptr();
   }

   context->error(QtXmlPatterns::tr("Operator %1 cannot be used on atomic values of type %2 and %3.")
                  .formatArg(formatKeyword(AtomicMathematician::displayName(op)))
                  .formatArg(formatType(context->namePool(), t1))
                  .formatArg(formatType(context->namePool(), t2)), code, reflection);

   return AtomicMathematician::Ptr();
}

SequenceType::Ptr ArithmeticExpression::staticType() const
{
   Cardinality card;

   /* These variables are important because they ensure staticType() only
    * gets called once from this function. Before, this lead to strange
    * semi-infinite recursion involving many arithmetic expressions. */
   const SequenceType::Ptr st1(m_operand1->staticType());
   const SequenceType::Ptr st2(m_operand2->staticType());

   if (st1->cardinality().allowsEmpty() ||
         st2->cardinality().allowsEmpty()) {
      card = Cardinality::zeroOrOne();
   } else {
      card = Cardinality::exactlyOne();
   }

   if (m_op == AtomicMathematician::IDiv) {
      return makeGenericSequenceType(BuiltinTypes::xsInteger, card);
   }

   const ItemType::Ptr t1(st1->itemType());
   const ItemType::Ptr t2(st2->itemType());
   ItemType::Ptr returnType;

   /* Please, make this beautiful? */
   if (BuiltinTypes::xsTime->xdtTypeMatches(t1) ||
         BuiltinTypes::xsDate->xdtTypeMatches(t1) ||
         BuiltinTypes::xsDateTime->xdtTypeMatches(t1)) {
      if (BuiltinTypes::xsDuration->xdtTypeMatches(t2)) {
         returnType = t1;
      } else {
         returnType = BuiltinTypes::xsDayTimeDuration;
      }
   } else if (BuiltinTypes::xsYearMonthDuration->xdtTypeMatches(t1)) {
      if (m_op == AtomicMathematician::Div &&
            BuiltinTypes::xsYearMonthDuration->xdtTypeMatches(t2)) {
         returnType = BuiltinTypes::xsDecimal;
      } else if (BuiltinTypes::numeric->xdtTypeMatches(t2)) {
         returnType = BuiltinTypes::xsYearMonthDuration;
      } else {
         returnType = t2;
      }
   } else if (BuiltinTypes::xsYearMonthDuration->xdtTypeMatches(t2)) {
      returnType = BuiltinTypes::xsYearMonthDuration;
   } else if (BuiltinTypes::xsDayTimeDuration->xdtTypeMatches(t1)) {
      if (m_op == AtomicMathematician::Div &&
            BuiltinTypes::xsDayTimeDuration->xdtTypeMatches(t2)) {
         returnType = BuiltinTypes::xsDecimal;
      } else if (BuiltinTypes::numeric->xdtTypeMatches(t2)) {
         returnType = BuiltinTypes::xsDayTimeDuration;
      } else {
         returnType = t2;
      }
   } else if (BuiltinTypes::xsDayTimeDuration->xdtTypeMatches(t2)) {
      returnType = BuiltinTypes::xsDayTimeDuration;
   } else if (BuiltinTypes::xsDouble->xdtTypeMatches(t1) ||
              BuiltinTypes::xsDouble->xdtTypeMatches(t2)) {
      returnType = BuiltinTypes::xsDouble;
   } else if (BuiltinTypes::xsFloat->xdtTypeMatches(t1) ||
              BuiltinTypes::xsFloat->xdtTypeMatches(t2)) {
      if (m_isCompat) {
         returnType = BuiltinTypes::xsFloat;
      } else {
         returnType = BuiltinTypes::xsDouble;
      }
   } else if (BuiltinTypes::xsInteger->xdtTypeMatches(t1) &&
              BuiltinTypes::xsInteger->xdtTypeMatches(t2)) {
      if (m_isCompat) {
         returnType = BuiltinTypes::xsDouble;
      } else {
         /* "A div B  numeric  numeric  op:numeric-divide(A, B)
          * numeric; but xs:decimal if both operands are xs:integer" */
         if (m_op == AtomicMathematician::Div) {
            returnType = BuiltinTypes::xsDecimal;
         } else {
            returnType = BuiltinTypes::xsInteger;
         }
      }
   } else if (m_isCompat && (BuiltinTypes::xsInteger->xdtTypeMatches(t1) &&
                             BuiltinTypes::xsInteger->xdtTypeMatches(t2))) {
      returnType = BuiltinTypes::xsDouble;
   } else {
      /* If typeCheck() has been called, our operands conform to expectedOperandTypes(), and
       * the types are hence either xs:decimals, or xs:anyAtomicType(meaning the static type could
       * not be inferred), or empty-sequence(). So we use the union of the two types. The combinations
       * could also be wrong.*/
      returnType = t1 | t2;

      /* However, if we're called before typeCheck(), we could potentially have nodes, so we need to make
       * sure that the type is at least atomic. */
      if (!BuiltinTypes::xsAnyAtomicType->xdtTypeMatches(returnType)) {
         returnType = BuiltinTypes::xsAnyAtomicType;
      }
   }

   return makeGenericSequenceType(returnType, card);
}

SequenceType::List ArithmeticExpression::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrOneAtomicType);
   result.append(CommonSequenceTypes::ZeroOrOneAtomicType);
   return result;
}

ExpressionVisitorResult::Ptr ArithmeticExpression::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

QT_END_NAMESPACE
