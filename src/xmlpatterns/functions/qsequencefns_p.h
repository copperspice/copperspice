/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef QSequenceFNs_P_H
#define QSequenceFNs_P_H

#include <qatomiccomparator_p.h>
#include <qcomparisonplatform_p.h>
#include <qliteral_p.h>
#include <qfunctioncall_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class BooleanFN : public FunctionCall
{
 public:
   virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;

   /**
    * If @p reqType is CommonSequenceTypes::EBV, the type check of
    * the operand is returned. Hence, this removes redundant calls
    * to <tt>fn:boolean()</tt>.
    */
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);
};


class IndexOfFN : public FunctionCall,
   public ComparisonPlatform<IndexOfFN, false>
{
 public:
   inline IndexOfFN() : ComparisonPlatform<IndexOfFN, false>() {
   }

   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);

   inline AtomicComparator::Operator operatorID() const {
      return AtomicComparator::OperatorEqual;
   }
};


template<const Expression::ID Id>
class Existence : public FunctionCall
{
 public:
   virtual bool evaluateEBV(const DynamicContext::Ptr &context) const {
      if (Id == IDExistsFN) {
         return !m_operands.first()->evaluateSequence(context)->isEmpty();
      } else {
         return m_operands.first()->evaluateSequence(context)->isEmpty();
      }
   }

   /**
    * Attempts to rewrite to @c false or @c true by looking at the static
    * cardinality of its operand.
    */
   virtual Expression::Ptr compress(const StaticContext::Ptr &context) {
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

   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
   /**
    * Performs necessary type checks, but also implements the optimization
    * of rewriting to its operand if the operand's cardinality is zero-or-one
    * or exactly-one.
    */
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);
   /**
    * @returns a type whose item type is the type of the first operand, and
    * a cardinality which is non-empty if the first operand's type is non-empty
    * and allows exactly-one. The latter is needed for operands which has the
    * cardinality 2+, since distinct-values possibly removes items from the
    * source sequence.
    */
   virtual SequenceType::Ptr staticType() const;

 protected:
   inline AtomicComparator::Operator operatorID() const {
      return AtomicComparator::OperatorEqual;
   }
};


class InsertBeforeFN : public FunctionCall
{
 public:
   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

   /**
    * Implements the static enferences rules. The function's static item type
    * is the union type of the first and third argument, and the cardinality is
    * the cardinalities of the two operands added together. For example,
    * insert-before((1, "str"), 1, xs:double(0)) has the static type xs:anyAtomicType+.
    *
    * @see <a href="http://www.w3.org/TR/xquery-semantics/#sec_fn_insert_before">XQuery 1.0
    * and XPath 2.0 Formal Semantics, 7.2.15 The fn:insert-before function</a>
    */
   virtual SequenceType::Ptr staticType() const;
};


class RemoveFN : public FunctionCall
{
 public:
   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

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
   virtual SequenceType::Ptr staticType() const;
};


class ReverseFN : public FunctionCall
{
 public:

   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);

   virtual SequenceType::Ptr staticType() const;
};

class SubsequenceFN : public FunctionCall
{
 public:
   SubsequenceFN();
   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);

   /**
    * This function implements rewrites the SubsequenceFN instance into an
    * empty sequence if its third argument, the sequence length argument, is
    * evaluated and is effectively equal or less than zero.
    */
   virtual Expression::Ptr compress(const StaticContext::Ptr &context);

   /**
    * Partially implements the static type inference rules.
    *
    * @see <a href="http://www.w3.org/TR/xquery-semantics/#sec_fn_subsequence">XQuery 1.0
    * and XPath 2.0 Formal Semantics, 7.2.13 The fn:subsequence function</a>
    */
   virtual SequenceType::Ptr staticType() const;

 private:
   bool m_hasTypeChecked;
};
}

QT_END_NAMESPACE

#endif
