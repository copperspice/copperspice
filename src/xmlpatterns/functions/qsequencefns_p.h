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
         // Since the dynamic type always is narrower than the static type or equal, and that the
         // static type is in scope, it means we will always be true.
         return wrapLiteral(CommonValues::BooleanTrue, context, this);

      } else {
         if (myCard.canMatch(card)) {
            return me;
         } else {
            return wrapLiteral(CommonValues::BooleanFalse, context, this);
         }
      }
   }
};


class DistinctValuesFN : public FunctionCall,
   public ComparisonPlatform<IndexOfFN, false>
{
 public:
   DistinctValuesFN()
      : ComparisonPlatform<IndexOfFN, false>()
   {
   }

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

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

   SequenceType::Ptr staticType() const override;
};


class RemoveFN : public FunctionCall
{
 public:
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

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

   Expression::Ptr compress(const StaticContext::Ptr &context) override;

   SequenceType::Ptr staticType() const override;

 private:
   bool m_hasTypeChecked;
};

}

#endif
