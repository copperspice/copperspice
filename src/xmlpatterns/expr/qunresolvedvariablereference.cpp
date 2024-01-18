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

#include "qunresolvedvariablereference_p.h"

using namespace QPatternist;

UnresolvedVariableReference::UnresolvedVariableReference(const QXmlName &name) : m_name(name)
{
   Q_ASSERT(!m_name.isNull());
}

Expression::Ptr UnresolvedVariableReference::typeCheck(const StaticContext::Ptr &context,
      const SequenceType::Ptr &reqType)
{
   /* We may be called before m_replacement is called, when we're part of a
    * function body whose type checking is performed for. See
    * UserFunctionCallsite::typeCheck(). */
   if (m_replacement) {
      return m_replacement->typeCheck(context, reqType);
   } else {
      return EmptyContainer::typeCheck(context, reqType);
   }
}

SequenceType::Ptr UnresolvedVariableReference::staticType() const
{
   /* We may be called by xmlpatternsview before the typeCheck() stage. */
   if (m_replacement) {
      return m_replacement->staticType();
   } else {
      return CommonSequenceTypes::ZeroOrMoreItems;
   }
}

SequenceType::List UnresolvedVariableReference::expectedOperandTypes() const
{
   return SequenceType::List();
}

ExpressionVisitorResult::Ptr UnresolvedVariableReference::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::ID UnresolvedVariableReference::id() const
{
   return IDUnresolvedVariableReference;
}
