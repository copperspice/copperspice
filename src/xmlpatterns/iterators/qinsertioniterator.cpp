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

#include "qinsertioniterator_p.h"

using namespace QPatternist;

InsertionIterator::InsertionIterator(const Item::Iterator::Ptr &target,
                                     const xsInteger pos,
                                     const Item::Iterator::Ptr &inserts)
   : m_target(target),
     m_insertPos(pos),
     m_inserts(inserts),
     m_position(0),
     m_isInserting(pos == 1)
{
   Q_ASSERT(target);
   Q_ASSERT(inserts);
   Q_ASSERT(m_insertPos >= 1);
}

Item InsertionIterator::next()
{
   if (m_isInserting) {
      m_current = m_inserts->next();

      if (m_current) {
         ++m_position;
         return m_current;
      }
   } else if (m_position == (m_insertPos - 1) && !m_isInserting) {
      /* Entered only the first time insertion starts. */
      m_isInserting = true;
      return next();
   }

   ++m_position;
   m_current = m_target->next();

   if (m_current) {
      return m_current;
   } else if (m_inserts->position() == -1) { /* We're at the end of the both iterators. */
      m_position = -1;
      m_current.reset();
      return Item();
   }

   /* Insert the insertion iterator, since it's still left. */
   Q_ASSERT(m_target->position() < m_insertPos);
   m_isInserting = true;
   m_current = m_inserts->next();

   if (m_current) {
      return m_current;
   } else {
      /* m_current is already null, so no need to reset it. */
      m_position = -1;
      return Item();
   }
}

xsInteger InsertionIterator::count()
{
   return m_target->count() + m_inserts->count();
}

Item InsertionIterator::current() const
{
   return m_current;
}

xsInteger InsertionIterator::position() const
{
   return m_position;
}

Item::Iterator::Ptr InsertionIterator::copy() const
{
   return Item::Iterator::Ptr(new InsertionIterator(m_target->copy(), m_insertPos, m_inserts->copy()));
}
