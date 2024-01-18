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
#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qgenericsequencetype_p.h"

#include "qliteral_p.h"

using namespace QPatternist;

Literal::Literal(const Item &i) : m_item(i)
{
   Q_ASSERT(m_item);
   Q_ASSERT(m_item.isAtomicValue());
}

Item Literal::evaluateSingleton(const DynamicContext::Ptr &) const
{
   return m_item;
}

bool Literal::evaluateEBV(const DynamicContext::Ptr &context) const
{
   return Boolean::evaluateEBV(m_item, context);
}

void Literal::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
   context->outputReceiver()->item(m_item);
}

SequenceType::Ptr Literal::staticType() const
{
   return makeGenericSequenceType(m_item.type(), Cardinality::exactlyOne());
}

ExpressionVisitorResult::Ptr Literal::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::ID Literal::id() const
{
   Q_ASSERT(m_item);
   Q_ASSERT(m_item.isAtomicValue());
   const ItemType::Ptr t(m_item.type());

   if (BuiltinTypes::xsBoolean->xdtTypeMatches(t)) {
      return IDBooleanValue;
   } else if (BuiltinTypes::xsString->xdtTypeMatches(t) ||
              BuiltinTypes::xsAnyURI->xdtTypeMatches(t) ||
              BuiltinTypes::xsUntypedAtomic->xdtTypeMatches(t)) {
      return IDStringValue;
   } else if (BuiltinTypes::xsInteger->xdtTypeMatches(t)) {
      return IDIntegerValue;
   } else {
      return IDFloat;
   }
}

Expression::Properties Literal::properties() const
{
   return IsEvaluated;
}

QString Literal::description() const
{
   return m_item.stringValue();
}
