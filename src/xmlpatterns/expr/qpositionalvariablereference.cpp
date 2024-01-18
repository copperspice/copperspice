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
#include "qinteger_p.h"

#include "qpositionalvariablereference_p.h"

using namespace QPatternist;

PositionalVariableReference::PositionalVariableReference(const VariableSlotID s) : VariableReference(s)
{
}

Item PositionalVariableReference::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   Q_ASSERT(context);
   Q_ASSERT(context->positionIterator(slot()));
   return Integer::fromValue(context->positionIterator(slot())->position());
}

bool PositionalVariableReference::evaluateEBV(const DynamicContext::Ptr &) const
{
   return true;
}

SequenceType::Ptr PositionalVariableReference::staticType() const
{
   return CommonSequenceTypes::ExactlyOneInteger;
}

Expression::Properties PositionalVariableReference::properties() const
{
   return DependsOnLocalVariable;
}

ExpressionVisitorResult::Ptr

PositionalVariableReference::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
