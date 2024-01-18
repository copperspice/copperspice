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

#include "qcontextitem_p.h"
#include "qcommonsequencetypes_p.h"
#include "qemptysequence_p.h"
#include "qfunctionsignature_p.h"
#include "qgenericsequencetype_p.h"
#include "qcollationchecker_p.h"
#include "qcommonnamespaces_p.h"

#include "qfunctioncall_p.h"

using namespace QPatternist;

SequenceType::List FunctionCall::expectedOperandTypes() const
{
   const FunctionArgument::List args(signature()->arguments());
   FunctionArgument::List::const_iterator it(args.constBegin());
   const FunctionArgument::List::const_iterator end(args.constEnd());
   // TODO reserve/resize()
   SequenceType::List result;

   for (; it != end; ++it) {
      result.append((*it)->type());
   }

   return result;
}

Expression::Ptr FunctionCall::typeCheck(const StaticContext::Ptr &context,
                                        const SequenceType::Ptr &reqType)
{
   /* We don't cache properties() at some stages because it can be invalidated
    * by the typeCheck(). */

   const FunctionSignature::Arity maxArgs = signature()->maximumArguments();
   /* We do this before the typeCheck() such that the appropriate conversions
    * are applied to the ContextItem. */
   if (m_operands.count() < maxArgs &&
         has(UseContextItem)) {
      m_operands.append(Expression::Ptr(new ContextItem()));
      context->wrapExpressionWith(this, m_operands.last());
   }

   const Expression::Ptr me(UnlimitedContainer::typeCheck(context, reqType));
   if (me != this) {
      return me;
   }

   const Properties props(properties());

   if (props.testFlag(RewriteToEmptyOnEmpty) &&
         *CommonSequenceTypes::Empty == *m_operands.first()->staticType()->itemType()) {
      return EmptySequence::create(this, context);
   }

   if (props.testFlag(LastOperandIsCollation) &&
         m_operands.count() == maxArgs) {
      m_operands.last() = Expression::Ptr(new CollationChecker(m_operands.last()));
      context->wrapExpressionWith(this, m_operands.last());
   }

   return me;
}

void FunctionCall::setSignature(const FunctionSignature::Ptr &sign)
{
   m_signature = sign;
}

FunctionSignature::Ptr FunctionCall::signature() const
{
   Q_ASSERT(m_signature); /* It really should be set. */
   return m_signature;
}

SequenceType::Ptr FunctionCall::staticType() const
{
   Q_ASSERT(m_signature);
   if (has(EmptynessFollowsChild)) {
      if (m_operands.isEmpty()) {
         /* This is a function which uses the context item when having no arguments. */
         return signature()->returnType();
      }
      const Cardinality card(m_operands.first()->staticType()->cardinality());
      if (card.allowsEmpty()) {
         return signature()->returnType();
      } else {
         /* Remove empty. */
         return makeGenericSequenceType(signature()->returnType()->itemType(),
                                        card & Cardinality::oneOrMore());
      }
   }
   return signature()->returnType();
}

Expression::Properties FunctionCall::properties() const
{
   Q_ASSERT(m_signature);
   return signature()->properties();
}

ExpressionVisitorResult::Ptr FunctionCall::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::ID FunctionCall::id() const
{
   Q_ASSERT(m_signature);
   return m_signature->id();
}

