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

#ifndef QSequenceFNs_P_H
#define QSequenceFNs_P_H

#include <qatomiccomparator_p.h>
#include <qcomparisonplatform_p.h>
#include <qliteral_p.h>
#include <qfunctioncall_p.h>

namespace QPatternist {

class BooleanFN : public FunctionCall
{
 public:
   bool evaluateEBV(const DynamicContext::Ptr &context) const override;

   /**
    * If @p reqType is CommonSequenceTypes::EBV, the type check of
    * the operand is returned. Hence, this removes redundant calls
    * to <tt>fn:boolean()</tt>.
    */
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;
};


class IndexOfFN : public FunctionCall,
   public ComparisonPlatform<IndexOfFN, false>
{
 public:
   IndexOfFN() : ComparisonPlatform<IndexOfFN, false>() {
   }

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   AtomicComparator::Operator operatorID() const {
      return AtomicComparator::OperatorEqual;
   }
};


template<const Expression::ID Id>
class Existence : public FunctionCall
{
 public:
   bool evaluateEBV(const DynamicContext::Ptr &context) const override {
      if constexpr (Id == IDExistsFN) {
         return !m_operands.first()->evaluateSequence(context)->isEmpty();
      } else {
         return m_operands.first()->evaluateSequence(context)->isEmpty();
      }
   }

   /**
    * Attempts to rewrite to @c false or @c true by looking at the static
    * cardinality of its operand.
    */
   Expression::Ptr compress(const StaticContext::Ptr &context)  override {
      // RVCT doesn't like using template parameter in trinary operator when the trinary operator result is
      // passed directly into another constructor.
      Q_ASSERT(Id == IDExistsFN || Id == IDEmptyFN);

      const Expression::Ptr me(FunctionCall::compress(context));

      if (me != this) {
         return me;
      }

      // RVCT doesn't like using template parameter in trinary operator when the trinary operator result is
      // passed directly into another constructor.
      Expression::ID tempId = Id;
      const Cardinality myCard((tempId == IDExistsFN) ? Cardinality::oneOrMore() : Cardinality::empty());

      const Cardinality card(m_operands.first()->staticType()->cardinality());
      if (myCard.isMatch(card)) {
         /* Since the dynamic type always is narrower than the static type or equal, and that the
            static type is in scope, it means we will always be true. */
         return wrapLiteral(CommonValues::BooleanTrue, context, this);

      } else {
         /* Is it even possible to hit? */
         if (myCard.canMatch(card)) {
            return me;
         } else {
            /* We can never hit. */
            return wrapLiteral(CommonValues::BooleanFalse, context, this);
         }
      }
   }
};


class DistinctValuesFN : public FunctionCall,
   public ComparisonPlatform<IndexOfFN, false>
{
 public:
   inline DistinctValuesFN() : ComparisonPlatform<IndexOfFN, false>() {
   }

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   /**
    * Performs necessary type checks, but also implements the optimization
    * of rewriting to its operand if the operand's cardinality is zero-or-one
    * or exactly-one.
    */
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   /**
    * @returns a type whose item type is the type of the first operand, and
    * a cardinality which is non-empty if the first operand's type is non-empty
    * and allows exactly-one. The latter is needed for operands which has the
    * cardinality 2+, since distinct-values possibly removes items from the
    * source sequence.
    */
   SequenceType::Ptr staticType() const override;

 protected:
   AtomicComparator::Operator operatorID() const {
      return AtomicComparator::OperatorEqual;
   }
};


class InsertBeforeFN : public FunctionCall
{
 public:
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

   /**
    * Implements the static enferences rules. The function's static item type
    * is the union type of the first and third argument, and the cardinality is
    * the cardinalities of the two operands added together. For example,
    * insert-before((1, "str"), 1, xs:double(0)) has the static type xs:anyAtomicType+.
    *
    * @see <a href="http://www.w3.org/TR/xquery-semantics/#sec_fn_insert_before">XQuery 1.0
    * and XPath 2.0 Formal Semantics, 7.2.15 The fn:insert-before function</a>
    */
   SequenceType::Ptr staticType() const override;
};


class RemoveFN : public FunctionCall
{
 public:
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

   /**
    * Implements the static enferences rules, "Since one item may be removed
    * from the sequence, the resulting type is made optional:"
    *
    * <tt>statEnv |-  (FN-URI,"remove")(Type, Type1) : prime(Type) * quantifier(Type)?</tt>
    *
    * However, because Patternist's type system is more fine grained than Formal Semantics,
    * the sequence isn't made optional. Instead its minimum length is reduced with one.
    *
    * @see <a href="http://www.w3.org/TR/xquery-semantics/#sec_fn_remove">XQuery 1.0
    * and XPath 2.0 Formal Semantics, 7.2.11 The fn:remove function</a>
    */
   SequenceType::Ptr staticType() const override;
};


class ReverseFN : public FunctionCall
{
 public:

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   SequenceType::Ptr staticType() const override;
};

class SubsequenceFN : public FunctionCall
{
 public:
   SubsequenceFN();

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   /**
    * This function implements rewrites the SubsequenceFN instance into an
    * empty sequence if its third argument, the sequence length argument, is
    * evaluated and is effectively equal or less than zero.
    */
   Expression::Ptr compress(const StaticContext::Ptr &context) override;

   /**
    * Partially implements the static type inference rules.
    *
    * @see <a href="http://www.w3.org/TR/xquery-semantics/#sec_fn_subsequence">XQuery 1.0
    * and XPath 2.0 Formal Semantics, 7.2.13 The fn:subsequence function</a>
    */
   SequenceType::Ptr staticType() const override;

 private:
   bool m_hasTypeChecked;
};

}

#endif
