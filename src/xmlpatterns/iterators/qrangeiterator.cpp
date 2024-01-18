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

#include "qrangeiterator_p.h"

using namespace QPatternist;

RangeIterator::RangeIterator(const xsInteger start,
                             const Direction direction,
                             const xsInteger end)
   : m_start(start),
     m_end(end),
     m_position(0),
     m_count(start),
     m_direction(direction),
     m_increment(m_direction == Forward ? 1 : -1)
{
   Q_ASSERT(m_start < m_end);
   Q_ASSERT(m_direction == Backward || m_direction == Forward);

   if (m_direction == Backward) {
      qSwap(m_start, m_end);
      m_count = m_start;
   }
}

Item RangeIterator::next()
{
   if (m_position == -1) {
      return Item();
   } else if ((m_direction == Forward && m_count > m_end) ||
              (m_direction == Backward && m_count < m_end)) {
      m_position = -1;
      m_current.reset();
      return Item();
   } else {
      m_current = Integer::fromValue(m_count);
      m_count += m_increment;
      ++m_position;
      return m_current;
   }
}

xsInteger RangeIterator::count()
{
   /* This complication is for handling that m_start & m_end may be reversed. */
   xsInteger ret;

   if (m_start < m_end) {
      ret = m_end - m_start;
   } else {
      ret = m_start - m_end;
   }

   return ret + 1;
}

Item::Iterator::Ptr RangeIterator::toReversed()
{
   return Item::Iterator::Ptr(new RangeIterator(m_start, Backward, m_end));
}

Item RangeIterator::current() const
{
   return m_current;
}

xsInteger RangeIterator::position() const
{
   return m_position;
}

Item::Iterator::Ptr RangeIterator::copy() const
{
   if (m_direction == Backward) {
      return Item::Iterator::Ptr(new RangeIterator(m_end, Backward, m_start));
   } else {
      return Item::Iterator::Ptr(new RangeIterator(m_start, Forward, m_end));
   }
}

