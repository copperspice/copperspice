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

#include "qexpressionvariablereference_p.h"

using namespace QPatternist;

ExpressionVariableReference::ExpressionVariableReference(const VariableSlotID slotP,
      const VariableDeclaration *varDecl) : VariableReference(slotP)
   , m_varDecl(varDecl)
{
}

bool ExpressionVariableReference::evaluateEBV(const DynamicContext::Ptr &context) const
{
   return context->expressionVariable(slot())->evaluateEBV(context);
}

Item ExpressionVariableReference::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return context->expressionVariable(slot())->evaluateSingleton(context);
}

Item::Iterator::Ptr ExpressionVariableReference::evaluateSequence(const DynamicContext::Ptr &context) const
{
   return context->expressionVariable(slot())->evaluateSequence(context);
}
Expression::Ptr ExpressionVariableReference::typeCheck(const StaticContext::Ptr &context,
      const SequenceType::Ptr &reqType)
{
   if (m_varDecl->canSourceRewrite) {
      return m_varDecl->expression()->typeCheck(context, reqType);
   } else {
      return VariableReference::typeCheck(context, reqType);
   }
}

Expression::ID ExpressionVariableReference::id() const
{
   return IDExpressionVariableReference;
}

ExpressionVisitorResult::Ptr ExpressionVariableReference::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

SequenceType::Ptr ExpressionVariableReference::staticType() const
{
   return m_varDecl->expression()->staticType();
}
