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

#ifndef QNodeComparison_P_H
#define QNodeComparison_P_H

#include <qpaircontainer_p.h>

namespace QPatternist {

class NodeComparison : public PairContainer
{
 public:
   NodeComparison(const Expression::Ptr &operand1, const QXmlNodeModelIndex::DocumentOrder op, const Expression::Ptr &operand2);

   Item evaluateSingleton(const DynamicContext::Ptr &) const override;
   bool evaluateEBV(const DynamicContext::Ptr &) const override;

   SequenceType::List expectedOperandTypes() const override;

   virtual QXmlNodeModelIndex::DocumentOrder operatorID() const;
   /**
    * If any operator is the empty sequence, the NodeComparison rewrites
    * into that, since the empty sequence is always the result in that case.
    */
   Expression::Ptr compress(const StaticContext::Ptr &context) override;

   /**
    * @returns either CommonSequenceTypes::ZeroOrOneBoolean or
    * CommonSequenceTypes::ExactlyOneBoolean depending on the static
    * cardinality of its operands.
    */
   SequenceType::Ptr staticType() const override;

   /**
    * Determines the string representation for a node comparison operator.
    *
    * @returns
    * - "<<" if @p op is Precedes
    * - ">>" if @p op is Follows
    * - "is" if @p op is Is
    */
   static QString displayName(const QXmlNodeModelIndex::DocumentOrder op);

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

 private:
   enum Result {
      Empty,
      True,
      False
   };
   inline Result evaluate(const DynamicContext::Ptr &context) const;

   const QXmlNodeModelIndex::DocumentOrder m_op;
};
}

#endif
