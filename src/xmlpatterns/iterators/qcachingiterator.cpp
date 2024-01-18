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

#include "qcachingiterator_p.h"

using namespace QPatternist;

CachingIterator::CachingIterator(ItemSequenceCacheCell::Vector &cacheCells,
                                 const VariableSlotID slot,
                                 const DynamicContext::Ptr &context) : m_position(0),
   m_varSlot(slot),
   m_context(context),
   m_cacheCells(cacheCells),
   m_usingCache(true)
{
   Q_ASSERT(m_varSlot > -1);
   Q_ASSERT(m_context);
   Q_ASSERT(m_cacheCells.at(m_varSlot).sourceIterator);
   Q_ASSERT_X((m_cacheCells.at(m_varSlot).cachedItems.isEmpty() &&
               m_cacheCells.at(m_varSlot).cacheState == ItemSequenceCacheCell::Empty) ||
              m_cacheCells.at(m_varSlot).cacheState == ItemSequenceCacheCell::PartiallyPopulated,
              Q_FUNC_INFO,
              "It makes no sense to construct a CachingIterator for a cache that is ItemSequenceCacheCell::Full.");
}

Item CachingIterator::next()
{
   ItemSequenceCacheCell &cell = m_cacheCells[m_varSlot];
   if (m_position == -1) {
      return Item();
   }

   if (m_usingCache) {
      ++m_position;

      /* QAbstractXmlForwardIterator::position() starts at 1, while Qt's container classes
       * starts at 0. */
      if (m_position - 1 < cell.cachedItems.count()) {
         m_current = cell.cachedItems.at(m_position - 1);
         return m_current;
      } else {
         cell.cacheState = ItemSequenceCacheCell::PartiallyPopulated;
         m_usingCache = false;
         /* We decrement here so we don't have to add a branch for this
          * when using the source QAbstractXmlForwardIterator below. */
         --m_position;
      }
   }

   m_current = cell.sourceIterator->next();

   if (m_current) {
      cell.cachedItems.append(m_current);
      Q_ASSERT(cell.cacheState == ItemSequenceCacheCell::PartiallyPopulated);
      ++m_position;
      return m_current;
   } else {
      m_position = -1;
      cell.cacheState = ItemSequenceCacheCell::Full;
      return Item();
   }
}

Item CachingIterator::current() const
{
   return m_current;
}

xsInteger CachingIterator::position() const
{
   return m_position;
}

Item::Iterator::Ptr CachingIterator::copy() const
{
   const ItemSequenceCacheCell &cell = m_cacheCells.at(m_varSlot);
   if (cell.cacheState == ItemSequenceCacheCell::Full) {
      return makeListIterator(cell.cachedItems);
   } else {
      return Item::Iterator::Ptr(new CachingIterator(m_cacheCells, m_varSlot, m_context));
   }
}
