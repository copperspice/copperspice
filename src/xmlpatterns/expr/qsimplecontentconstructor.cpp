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
#include "qatomicstring_p.h"

#include "qsimplecontentconstructor_p.h"

using namespace QPatternist;

SimpleContentConstructor::SimpleContentConstructor(const Expression::Ptr &operand) : SingleContainer(operand)
{
}

Item SimpleContentConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));
   Item next(it->next());
   QString result;

   if (next) {
      result = next.stringValue();
      next = it->next();
   } else {
      return Item();
   }

   while (next) {
      result += QLatin1Char(' ');
      result += next.stringValue();
      next = it->next();
   }

   return AtomicString::fromValue(result);
}

Expression::Ptr SimpleContentConstructor::compress(const StaticContext::Ptr &context)
{
   const Expression::Ptr me(SingleContainer::compress(context));

   if (me.data() == this) {
      /* Optimization: if we will evaluate to a single string, we're not
       * necessary. */
      if (CommonSequenceTypes::ExactlyOneString->matches(m_operand->staticType())) {
         return m_operand;
      }
   }

   return me;
}

SequenceType::List SimpleContentConstructor::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreAtomicTypes);
   return result;
}

SequenceType::Ptr SimpleContentConstructor::staticType() const
{
   if (m_operand->staticType()->cardinality().allowsEmpty()) {
      return CommonSequenceTypes::ZeroOrOneString;
   } else {
      return CommonSequenceTypes::ExactlyOneString;
   }
}

ExpressionVisitorResult::Ptr SimpleContentConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
