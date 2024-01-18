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

#include "qargumentreference_p.h"

using namespace QPatternist;

ArgumentReference::ArgumentReference(const SequenceType::Ptr &sourceType,
                                     const VariableSlotID slotP) : VariableReference(slotP),
   m_type(sourceType)
{
   Q_ASSERT(m_type);
}

bool ArgumentReference::evaluateEBV(const DynamicContext::Ptr &context) const
{
   return context->expressionVariable(slot())->evaluateEBV(context);
}

Item ArgumentReference::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return context->expressionVariable(slot())->evaluateSingleton(context);
}

Item::Iterator::Ptr ArgumentReference::evaluateSequence(const DynamicContext::Ptr &context) const
{
   return context->expressionVariable(slot())->evaluateSequence(context);
}

SequenceType::Ptr ArgumentReference::staticType() const
{
   return m_type;
}

ExpressionVisitorResult::Ptr ArgumentReference::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::ID ArgumentReference::id() const
{
   return IDArgumentReference;
}
