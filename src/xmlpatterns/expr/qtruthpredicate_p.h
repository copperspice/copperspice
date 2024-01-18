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

#ifndef QTruthPredicate_P_H
#define QTruthPredicate_P_H

#include <qgenericpredicate_p.h>

namespace QPatternist {

class TruthPredicate : public GenericPredicate
{
 public:
   TruthPredicate(const Expression::Ptr &sourceExpression, const Expression::Ptr &predicate);

   Item mapToItem(const Item &item, const DynamicContext::Ptr &context) const {
      Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");

      if (m_operand2->evaluateEBV(context)) {
         return item;
      } else {
         return Item();
      }
   }

   Item::Iterator::Ptr map(const Item &item, const DynamicContext::Ptr &context) const {
      Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");

      if (m_operand2->evaluateEBV(context)) {
         return makeSingletonIterator(item);
      } else {
         return CommonValues::emptyIterator;
      }
   }

   SequenceType::List expectedOperandTypes() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
};
}

#endif
