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

#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qgenericsequencetype_p.h"
#include "qliteralsequence_p.h"

using namespace QPatternist;

LiteralSequence::LiteralSequence(const Item::List &list) : m_list(list)
{
   Q_ASSERT(list.size() >= 2);
}

Item::Iterator::Ptr LiteralSequence::evaluateSequence(const DynamicContext::Ptr &) const
{
   return makeListIterator(m_list);
}

SequenceType::Ptr LiteralSequence::staticType() const
{
   const Item::List::const_iterator end(m_list.constEnd());
   Item::List::const_iterator it(m_list.constBegin());

   /* Load the first item. */
   ItemType::Ptr t((*it).type());
   ++it;

   for (; end != it; ++it) {
      t |= (*it).type();
   }

   return makeGenericSequenceType(t, Cardinality::fromCount(m_list.size()));
}

ExpressionVisitorResult::Ptr LiteralSequence::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::ID LiteralSequence::id() const
{
   return IDExpressionSequence;
}

Expression::Properties LiteralSequence::properties() const
{
   return IsEvaluated;
}
