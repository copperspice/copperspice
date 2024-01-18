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
#include "qgenericsequencetype_p.h"

#include "qrangevariablereference_p.h"

using namespace QPatternist;

RangeVariableReference::RangeVariableReference(const Expression::Ptr &source,
      const VariableSlotID slotP) : VariableReference(slotP),
   m_sourceExpression(source)
{
   Q_ASSERT(source);
}

bool RangeVariableReference::evaluateEBV(const DynamicContext::Ptr &context) const
{
   Q_ASSERT_X(context->rangeVariable(slot()), Q_FUNC_INFO, "The range variable must be set.");
   return Boolean::evaluateEBV(context->rangeVariable(slot()), context);
}

Item RangeVariableReference::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   Q_ASSERT_X(context->rangeVariable(slot()), Q_FUNC_INFO, "The range variable must be set.");
   return context->rangeVariable(slot());
}

SequenceType::Ptr RangeVariableReference::staticType() const
{
   return makeGenericSequenceType(m_sourceExpression->staticType()->itemType(),
                                  Cardinality::exactlyOne());
}

Expression::ID RangeVariableReference::id() const
{
   return IDRangeVariableReference;
}

ExpressionVisitorResult::Ptr
RangeVariableReference::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::Properties RangeVariableReference::properties() const
{
   return DependsOnLocalVariable;
}
