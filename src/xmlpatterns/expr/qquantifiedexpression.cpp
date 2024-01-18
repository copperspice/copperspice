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

#include "qcommonsequencetypes_p.h"
#include "qgenericsequencetype_p.h"
#include "qitemmappingiterator_p.h"

#include "qquantifiedexpression_p.h"

using namespace QPatternist;

QuantifiedExpression::QuantifiedExpression(const VariableSlotID varSlot,
      const Operator quantifier,
      const Expression::Ptr &inClause,
      const Expression::Ptr &testExpression)
   : PairContainer(inClause, testExpression),
     m_varSlot(varSlot),
     m_quantifier(quantifier)
{
   Q_ASSERT(quantifier == Some || quantifier == Every);
}

Item QuantifiedExpression::mapToItem(const Item &item,
                                     const DynamicContext::Ptr &context) const
{
   context->setRangeVariable(m_varSlot, item);
   return item;
}

bool QuantifiedExpression::evaluateEBV(const DynamicContext::Ptr &context) const
{
   const Item::Iterator::Ptr it(makeItemMappingIterator<Item>(ConstPtr(this),
                                m_operand1->evaluateSequence(context),
                                context));

   Item item(it->next());

   if (m_quantifier == Some) {
      while (item) {
         if (m_operand2->evaluateEBV(context)) {
            return true;
         } else {
            item = it->next();
         }
      };

      return false;
   } else {
      Q_ASSERT(m_quantifier == Every);

      while (item) {
         if (m_operand2->evaluateEBV(context)) {
            item = it->next();
         } else {
            return false;
         }
      }

      return true;
   }
}

QString QuantifiedExpression::displayName(const Operator quantifier)
{
   if (quantifier == Some) {
      return QLatin1String("some");
   } else {
      Q_ASSERT(quantifier == Every);
      return QLatin1String("every");
   }
}

SequenceType::Ptr QuantifiedExpression::staticType() const
{
   return CommonSequenceTypes::ExactlyOneBoolean;
}

SequenceType::List QuantifiedExpression::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   result.append(CommonSequenceTypes::EBV);
   return result;
}

QuantifiedExpression::Operator QuantifiedExpression::operatorID() const
{
   return m_quantifier;
}

ExpressionVisitorResult::Ptr QuantifiedExpression::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
