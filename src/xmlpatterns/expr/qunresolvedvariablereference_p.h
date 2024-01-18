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

#ifndef QUnresolvedVariableReference_P_H
#define QUnresolvedVariableReference_P_H

#include <qemptycontainer_p.h>

namespace QPatternist {

class UnresolvedVariableReference : public EmptyContainer
{
 public:
   UnresolvedVariableReference(const QXmlName &name);

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;
   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   ID id() const override;

   inline void bindTo(const Expression::Ptr &body);
   inline Expression::Ptr replacement() const;

 private:
   const QXmlName  m_name;
   Expression::Ptr m_replacement;
};

void UnresolvedVariableReference::bindTo(const Expression::Ptr &body)
{
   Q_ASSERT(body);
   m_replacement = body;
}

Expression::Ptr UnresolvedVariableReference::replacement() const
{
   Q_ASSERT(m_replacement);
   return m_replacement;
}

}

#endif
