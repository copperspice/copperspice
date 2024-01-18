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

#include "qexternalvariablereference_p.h"

using namespace QPatternist;

ExternalVariableReference::ExternalVariableReference(const QXmlName &name,
      const SequenceType::Ptr &type) : m_name(name),
   m_seqType(type)
{
   Q_ASSERT(!m_name.isNull());
   Q_ASSERT(m_seqType);
}

Item::Iterator::Ptr ExternalVariableReference::evaluateSequence(const DynamicContext::Ptr &context) const
{
   Q_ASSERT(context->externalVariableLoader());
   return context->externalVariableLoader()->evaluateSequence(m_name, context);
}

Item ExternalVariableReference::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return context->externalVariableLoader()->evaluateSingleton(m_name, context);
}

bool ExternalVariableReference::evaluateEBV(const DynamicContext::Ptr &context) const
{
   return context->externalVariableLoader()->evaluateEBV(m_name, context);
}

SequenceType::Ptr ExternalVariableReference::staticType() const
{
   return m_seqType;
}

Expression::Properties ExternalVariableReference::properties() const
{
   return DisableElimination;
}

ExpressionVisitorResult::Ptr ExternalVariableReference::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

