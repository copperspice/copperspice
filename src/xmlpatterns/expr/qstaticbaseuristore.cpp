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
#include "qstaticbaseuricontext_p.h"

#include "qstaticbaseuristore_p.h"

using namespace QPatternist;

StaticBaseURIStore::StaticBaseURIStore(const QUrl &baseURI,
                                       const Expression::Ptr &operand) : SingleContainer(operand)
   , m_baseURI(baseURI)
{
}

Expression::Ptr StaticBaseURIStore::typeCheck(const StaticContext::Ptr &context,
      const SequenceType::Ptr &reqType)
{
   const StaticContext::Ptr newContext(new StaticBaseURIContext(context->baseURI().resolved(m_baseURI),
                                       context));
   return m_operand->typeCheck(newContext, reqType);
}

SequenceType::Ptr StaticBaseURIStore::staticType() const
{
   return m_operand->staticType();
}

SequenceType::List StaticBaseURIStore::expectedOperandTypes() const
{
   SequenceType::List ops;
   ops.append(CommonSequenceTypes::ZeroOrMoreItems);
   return ops;
}

ExpressionVisitorResult::Ptr StaticBaseURIStore::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
