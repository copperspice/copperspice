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

#ifndef QCombineNodes_P_H
#define QCombineNodes_P_H

#include <qpaircontainer_p.h>

namespace QPatternist {

class CombineNodes : public PairContainer
{
 public:
   enum Operator {
      Union       = 1,
      Intersect   = 2,
      Except      = 4
   };

   CombineNodes(const Expression::Ptr &operand1, const Operator op, const Expression::Ptr &operand2);

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   bool evaluateEBV(const DynamicContext::Ptr &context) const override;
   SequenceType::Ptr staticType() const override;
   SequenceType::List expectedOperandTypes() const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   Operator operatorID() const;
   ID id() const override;

   /**
    * Determines the string representation for operator @p op.
    *
    * @return "union" if @p op is Union, "intersect" if @p op
    * is Intersect and "except" if @p op is Except.
    */
   static QString displayName(const Operator op);

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

 private:
   const Operator m_operator;
};

}

#endif
