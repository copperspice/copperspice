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

#ifndef QQuantifiedExpression_P_H
#define QQuantifiedExpression_P_H

#include <qpaircontainer_p.h>

namespace QPatternist {

class QuantifiedExpression : public PairContainer
{
 public:
   enum Operator {
      Some    = 1,
      Every
   };

   QuantifiedExpression(const VariableSlotID varSlot,
                        const Operator quantifier,
                        const Expression::Ptr &inClause,
                        const Expression::Ptr &testExpression);

   bool evaluateEBV(const DynamicContext::Ptr &context) const override;
   SequenceType::Ptr staticType() const override;
   SequenceType::List expectedOperandTypes() const override;

   Operator operatorID() const;

   /**
    * Determines the string representation for a quantification operator.
    *
    * @return "some" if @p quantifier is Some, or "every" if @p quantifier
    * is Every
    */
   static QString displayName(const Operator quantifier);

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   inline Item mapToItem(const Item &item, const DynamicContext::Ptr &context) const;

 private:
   typedef QExplicitlySharedDataPointer<const QuantifiedExpression> ConstPtr;
   const VariableSlotID m_varSlot;
   const Operator m_quantifier;
};
}

#endif
