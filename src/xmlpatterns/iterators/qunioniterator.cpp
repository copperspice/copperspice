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

#include "qitem_p.h"

#include "qunioniterator_p.h"

using namespace QPatternist;

UnionIterator::UnionIterator(const Item::Iterator::Ptr &it1,
                             const Item::Iterator::Ptr &it2) : m_it1(it1),
   m_it2(it2),
   m_position(0),
   m_node1(m_it1->next()),
   m_node2(m_it2->next())
{
   Q_ASSERT(m_it1);
   Q_ASSERT(m_it2);
}

Item UnionIterator::next()
{
   ++m_position;
   if (m_node1 && m_node2) {
      if (m_node1.asNode().model() != m_node2.asNode().model()) {
         m_current = m_node1;
         m_node1 = m_it1->next();
         return m_current;
      }

      switch (m_node1.asNode().compareOrder(m_node2.asNode())) {
         case QXmlNodeModelIndex::Precedes: {
            m_current = m_node1;
            m_node1 = m_it1->next();
            return m_current;
         }
         case QXmlNodeModelIndex::Follows: {
            m_current = m_node2;
            m_node2 = m_it2->next();
            return m_current;
         }
         default: {
            m_current = m_node2;
            m_node1 = m_it1->next();
            m_node2 = m_it2->next();
            return m_current;
         }
      }
   }

   if (m_node1) {
      m_current = m_node1;
      m_node1 = m_it1->next();
      return m_current;
   }

   if (m_node2) {
      m_current = m_node2;
      m_node2 = m_it2->next();
      return m_current;
   }

   m_current.reset();
   m_position = -1;
   return Item();
}

Item UnionIterator::current() const
{
   return m_current;
}

xsInteger UnionIterator::position() const
{
   return m_position;
}

Item::Iterator::Ptr UnionIterator::copy() const
{
   return Item::Iterator::Ptr(new UnionIterator(m_it1->copy(), m_it2->copy()));
}
