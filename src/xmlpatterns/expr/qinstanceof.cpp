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

#include "qboolean_p.h"
#include "qcommonsequencetypes_p.h"
#include "qcommonvalues_p.h"
#include "qliteral_p.h"
#include "qinstanceof_p.h"

using namespace QPatternist;

InstanceOf::InstanceOf(const Expression::Ptr &operand,
                       const SequenceType::Ptr &tType) : SingleContainer(operand)
   , m_targetType(tType)
{
   Q_ASSERT(m_targetType);
}

bool InstanceOf::evaluateEBV(const DynamicContext::Ptr &context) const
{
   const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));
   Item item(it->next());
   unsigned int count = 1;

   if (!item) {
      return m_targetType->cardinality().allowsEmpty();
   }

   do {
      if (!m_targetType->itemType()->itemMatches(item)) {
         return false;
      }

      if (count == 2 && !m_targetType->cardinality().allowsMany()) {
         return false;
      }

      item = it->next();
      ++count;
   } while (item);

   return true;
}

Expression::Ptr InstanceOf::compress(const StaticContext::Ptr &context)
{
   const Expression::Ptr me(SingleContainer::compress(context));

   if (me != this || m_operand->has(DisableTypingDeduction)) {
      return me;
   }

   const SequenceType::Ptr opType(m_operand->staticType());
   const ItemType::Ptr targetType(m_targetType->itemType());
   const ItemType::Ptr operandType(opType->itemType());

   if (m_targetType->cardinality().isMatch(opType->cardinality())) {
      if (*operandType == *CommonSequenceTypes::Empty || targetType->xdtTypeMatches(operandType)) {
         return wrapLiteral(CommonValues::BooleanTrue, context, this);
      } else if (!operandType->xdtTypeMatches(targetType)) {
         return wrapLiteral(CommonValues::BooleanFalse, context, this);
      }
   }
   /* Optimization: rule out the case where instance of will always fail. */

   return me;
}

SequenceType::Ptr InstanceOf::targetType() const
{
   return m_targetType;
}

SequenceType::Ptr InstanceOf::staticType() const
{
   return CommonSequenceTypes::ExactlyOneBoolean;
}

SequenceType::List InstanceOf::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

ExpressionVisitorResult::Ptr InstanceOf::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
