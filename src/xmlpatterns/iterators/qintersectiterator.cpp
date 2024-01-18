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

#include "qintersectiterator_p.h"

using namespace QPatternist;

IntersectIterator::IntersectIterator(const Item::Iterator::Ptr &it1,
                                     const Item::Iterator::Ptr &it2) : m_it1(it1),
   m_it2(it2),
   m_position(0),
   m_node1(m_it1->next()),
   m_node2(m_it2->next())
{
   Q_ASSERT(m_it1);
   Q_ASSERT(m_it2);
}

Item IntersectIterator::next()
{
   if (!m_node1 || !m_node2) {
      return closedExit();
   }

   do {
      if (m_node1.asNode().model() == m_node2.asNode().model()) {
         switch (m_node1.asNode().compareOrder(m_node2.asNode())) {
            case QXmlNodeModelIndex::Precedes: {
               m_node1 = m_it1->next();
               break;
            }
            case QXmlNodeModelIndex::Follows: {
               m_node2 = m_it2->next();
               break;
            }
            default: {
               m_current = m_node2;
               m_node1 = m_it1->next();
               m_node2 = m_it2->next();
               ++m_position;
               return m_current;
            }
         }
      } else {
         m_node2 = m_it2->next();
      }
   } while (m_node1 && m_node2);

   return Item();
}

Item IntersectIterator::current() const
{
   return m_current;
}

xsInteger IntersectIterator::position() const
{
   return m_position;
}

Item::Iterator::Ptr IntersectIterator::copy() const
{
   return Item::Iterator::Ptr(new IntersectIterator(m_it1->copy(), m_it2->copy()));
}
