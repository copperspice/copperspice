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

#ifndef Patternist_NodeComparison_H
#define Patternist_NodeComparison_H

#include "qpaircontainer_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {

class NodeComparison : public PairContainer
{
 public:
   NodeComparison(const Expression::Ptr &operand1,
                  const QXmlNodeModelIndex::DocumentOrder op,
                  const Expression::Ptr &operand2);

   virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;
   virtual bool evaluateEBV(const DynamicContext::Ptr &) const;

   virtual SequenceType::List expectedOperandTypes() const;

   virtual QXmlNodeModelIndex::DocumentOrder operatorID() const;
   /**
    * If any operator is the empty sequence, the NodeComparison rewrites
    * into that, since the empty sequence is always the result in that case.
    */
   virtual Expression::Ptr compress(const StaticContext::Ptr &context);

   /**
    * @returns either CommonSequenceTypes::ZeroOrOneBoolean or
    * CommonSequenceTypes::ExactlyOneBoolean depending on the static
    * cardinality of its operands.
    */
   virtual SequenceType::Ptr staticType() const;

   /**
    * Determines the string representation for a node comparison operator.
    *
    * @returns
    * - "<<" if @p op is Precedes
    * - ">>" if @p op is Follows
    * - "is" if @p op is Is
    */
   static QString displayName(const QXmlNodeModelIndex::DocumentOrder op);

   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
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

QT_END_NAMESPACE

#endif
