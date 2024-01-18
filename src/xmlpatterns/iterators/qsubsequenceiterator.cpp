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

#include "qxpathhelper_p.h"

#include "qsubsequenceiterator_p.h"

using namespace QPatternist;

SubsequenceIterator::SubsequenceIterator(const Item::Iterator::Ptr &iterator,
      const xsInteger start,
      const xsInteger len)
   : m_position(0),
     m_it(iterator),
     m_counter(start),
     m_start(start),
     m_len(len),
     m_stop(m_start + m_len)
{
   Q_ASSERT(iterator);
   Q_ASSERT(start >= 1);
   Q_ASSERT(len == -1 || len >= 1);

   /* Note, "The first item of a sequence is located at position 1, not position 0." */
   for (xsInteger i = 1; i != m_start; ++i) {
      m_it->next();
   }
}

Item SubsequenceIterator::next()
{
   if (m_position == -1) {
      return Item();
   }

   m_current = m_it->next();
   ++m_position;

   if (m_len == -1) {
      if (!m_current) {
         m_position = -1;
      }

      return m_current;
   }

   ++m_counter;

   if (!(m_counter > m_stop) && m_current) {
      return m_current;
   }

   m_position = -1;
   m_current.reset();
   return Item();
}

Item SubsequenceIterator::current() const
{
   return m_current;
}

xsInteger SubsequenceIterator::position() const
{
   return m_position;
}

Item::Iterator::Ptr SubsequenceIterator::copy() const
{
   return Item::Iterator::Ptr(new SubsequenceIterator(m_it->copy(), m_start, m_len));
}
