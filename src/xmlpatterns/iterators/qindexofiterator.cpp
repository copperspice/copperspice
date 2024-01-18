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

#include "qinteger_p.h"

#include "qindexofiterator_p.h"

using namespace QPatternist;

IndexOfIterator::IndexOfIterator(const Item::Iterator::Ptr &seq,
                                 const Item &searchParam,
                                 const AtomicComparator::Ptr &comp,
                                 const DynamicContext::Ptr &context,
                                 const Expression::ConstPtr &expr)
   : m_seq(seq)
   , m_searchParam(searchParam)
   , m_context(context)
   , m_expr(expr)
   , m_position(0)
   , m_seqPos(0)
{
   Q_ASSERT(seq);
   Q_ASSERT(searchParam);
   prepareComparison(comp);
}

Item IndexOfIterator::next()
{
   if (m_position == -1) {
      return Item();
   }

   const Item item(m_seq->next());
   ++m_seqPos;

   if (!item) {
      m_current.reset();
      m_position = -1;
      return Item();
   }

   if (flexibleCompare(item, m_searchParam, m_context)) {
      ++m_position;
      return Integer::fromValue(m_seqPos);
   }

   return next();
}

Item IndexOfIterator::current() const
{
   return m_current;
}

xsInteger IndexOfIterator::position() const
{
   return m_position;
}

Item::Iterator::Ptr IndexOfIterator::copy() const
{
   return Item::Iterator::Ptr(new IndexOfIterator(m_seq->copy(),
                              m_searchParam,
                              comparator(),
                              m_context,
                              m_expr));
}

const SourceLocationReflection *IndexOfIterator::actualReflection() const
{
   return m_expr.data();
}
