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

#include "qfirstitempredicate_p.h"

using namespace QPatternist;

FirstItemPredicate::FirstItemPredicate(const Expression::Ptr &source) : SingleContainer(source)
{
}

Item FirstItemPredicate::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   /* If our operand allows the empty sequence, this function can return Item(), otherwise
    * it returns the first item. As simple as that. */
   return m_operand->evaluateSequence(context)->next();
}

SequenceType::Ptr FirstItemPredicate::staticType() const
{
   const SequenceType::Ptr t(m_operand->staticType());
   return makeGenericSequenceType(t->itemType(), t->cardinality().toWithoutMany());
}

Expression::Ptr FirstItemPredicate::compress(const StaticContext::Ptr &context)
{
   const Expression::Ptr me(SingleContainer::compress(context));
   if (me != this) {
      return me;
   }

   if (m_operand->is(IDFirstItemPredicate)) {
      m_operand = m_operand->operands().first();
   }

   return me;
}

SequenceType::List FirstItemPredicate::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

ExpressionVisitorResult::Ptr FirstItemPredicate::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::ID FirstItemPredicate::id() const
{
   return IDFirstItemPredicate;
}
