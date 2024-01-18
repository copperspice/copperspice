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

#ifndef QRangeExpression_P_H
#define QRangeExpression_P_H

#include <qpaircontainer_p.h>

namespace QPatternist {

class RangeExpression : public PairContainer
{
 public:
   RangeExpression(const Expression::Ptr &operand1, const Expression::Ptr &operand2);

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const override;
   /**
    * It's likely that this function gets called if staticType() inferred
    * the cardinality to an exact number. In that case, we know that the
    * first arguments is the same as the second argument.
    */
   Item evaluateSingleton(const DynamicContext::Ptr &) const override;

   SequenceType::List expectedOperandTypes() const override;

   /**
    * @returns always CommonSequenceTypes::ZeroOrMoreIntegers
    */
   SequenceType::Ptr staticType() const override;

   /**
    * Disables compression for optimization reasons. For example, the
    * expression "1 to 1000" would consume thousand allocated instances
    * of Integer, and RangeIterator is well suited for dynamic evaluation.
    *
    * @returns Expression::DisableElimination
    */
   Expression::Properties properties() const override;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
};

}

#endif
