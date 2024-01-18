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
#include "qdynamiccontextstore_p.h"
#include "qliteral_p.h"
#include "qletclause_p.h"

using namespace QPatternist;

LetClause::LetClause(const Expression::Ptr &operand1,
                     const Expression::Ptr &operand2,
                     const VariableDeclaration::Ptr &decl) : PairContainer(operand1, operand2)
   , m_varDecl(decl)
{
   Q_ASSERT(m_varDecl);
}

DynamicContext::Ptr LetClause::bindVariable(const DynamicContext::Ptr &context) const
{
   context->setExpressionVariable(m_varDecl->slot, m_operand1);
   return context;
}

Item::Iterator::Ptr LetClause::evaluateSequence(const DynamicContext::Ptr &context) const
{
   return m_operand2->evaluateSequence(bindVariable(context));
}

Item LetClause::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return m_operand2->evaluateSingleton(bindVariable(context));
}

bool LetClause::evaluateEBV(const DynamicContext::Ptr &context) const
{
   return m_operand2->evaluateEBV(bindVariable(context));
}

void LetClause::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
   m_operand2->evaluateToSequenceReceiver(bindVariable(context));
}

Expression::Ptr LetClause::typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType)
{
   /* Consider the following query:
    *
    * <tt>let $d := \<child type=""/>
    * return $d//\*[let $i := @type
    *              return $d//\*[$i]]</tt>
    *
    * The node test <tt>@type</tt> is referenced from two different places,
    * where each reference have a different focus. So, in the case of that the source
    * uses the focus, we need to use a DynamicContextStore to ensure the variable
    * is always evaluated with the correct focus, regardless of where it is referenced
    * from.
    *
    * We miss out a lot of false positives. For instance, the case of where the focus
    * is identical for everyone. One reason we cannot check this, is that Expression
    * doesn't know about its parent.
    */
   m_varDecl->canSourceRewrite = !m_operand1->deepProperties().testFlag(RequiresFocus);

   if (m_varDecl->canSourceRewrite) {
      return m_operand2->typeCheck(context, reqType);
   } else {
      return PairContainer::typeCheck(context, reqType);
   }
}

Expression::Properties LetClause::properties() const
{
   return m_varDecl->expression()->properties() & (Expression::RequiresFocus | Expression::IsEvaluated |
          Expression::DisableElimination);
}

SequenceType::List LetClause::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

SequenceType::Ptr LetClause::staticType() const
{
   return m_operand2->staticType();
}

ExpressionVisitorResult::Ptr LetClause::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::ID LetClause::id() const
{
   return IDLetClause;
}
