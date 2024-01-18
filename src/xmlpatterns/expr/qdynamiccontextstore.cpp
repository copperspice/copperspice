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

#include "qdynamiccontextstore_p.h"

using namespace QPatternist;

DynamicContextStore::DynamicContextStore(const Expression::Ptr &operand,
      const DynamicContext::Ptr &context) : SingleContainer(operand),
   m_context(context.data())
{
   Q_ASSERT(context);
}

bool DynamicContextStore::evaluateEBV(const DynamicContext::Ptr &) const
{
   return m_operand->evaluateEBV(DynamicContext::Ptr(m_context));
}

Item::Iterator::Ptr DynamicContextStore::evaluateSequence(const DynamicContext::Ptr &) const
{
   return m_operand->evaluateSequence(DynamicContext::Ptr(m_context));
}

Item DynamicContextStore::evaluateSingleton(const DynamicContext::Ptr &) const
{
   return m_operand->evaluateSingleton(DynamicContext::Ptr(m_context));
}

SequenceType::Ptr DynamicContextStore::staticType() const
{
   return m_operand->staticType();
}

SequenceType::List DynamicContextStore::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

ExpressionVisitorResult::Ptr DynamicContextStore::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

const SourceLocationReflection *DynamicContextStore::actualReflection() const
{
   return m_operand->actualReflection();
}

