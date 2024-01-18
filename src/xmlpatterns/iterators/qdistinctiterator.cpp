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

#include "qdistinctiterator_p.h"

using namespace QPatternist;

DistinctIterator::DistinctIterator(const Item::Iterator::Ptr &seq,
                                   const AtomicComparator::Ptr &comp,
                                   const Expression::ConstPtr &expression,
                                   const DynamicContext::Ptr &context)
   : m_seq(seq)
   , m_context(context)
   , m_expr(expression)
   , m_position(0)
{
   Q_ASSERT(seq);
   prepareComparison(comp);
}

Item DistinctIterator::next()
{
   if (m_position == -1) {
      return Item();
   }

   const Item nitem(m_seq->next());
   if (!nitem) {
      m_position = -1;
      m_current.reset();
      return Item();
   }

   const Item::List::const_iterator end(m_processed.constEnd());
   Item::List::const_iterator it(m_processed.constBegin());

   for (; it != end; ++it) {
      if (flexibleCompare(*it, nitem, m_context)) {
         return next();
      }
   }

   m_current = nitem;
   ++m_position;
   m_processed.append(nitem);
   return nitem;
}

Item DistinctIterator::current() const
{
   return m_current;
}

xsInteger DistinctIterator::position() const
{
   return m_position;
}

Item::Iterator::Ptr DistinctIterator::copy() const
{
   return Item::Iterator::Ptr(new DistinctIterator(m_seq->copy(), comparator(), m_expr, m_context));
}

const SourceLocationReflection *DistinctIterator::actualReflection() const
{
   return m_expr.data();
}
