/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qexceptiterator_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

ExceptIterator::ExceptIterator(const Item::Iterator::Ptr &it1,
                               const Item::Iterator::Ptr &it2) : m_it1(it1)
   , m_it2(it2)
   , m_position(0)
   , m_node1(m_it1->next())
   , m_node2(m_it2->next())
{
   Q_ASSERT(m_it1);
   Q_ASSERT(m_it2);
}

Item ExceptIterator::fromFirstOperand()
{
   ++m_position;
   m_current = m_node1;
   m_node1 = m_it1->next();

   return m_current;
}

Item ExceptIterator::next()
{
   while (true) {
      if (!m_node1) {
         m_position = -1;
         m_current = Item();
         return Item();
      } else if (!m_node2) {
         return fromFirstOperand();
      }

      if (m_node1.asNode().model() != m_node2.asNode().model()) {
         return fromFirstOperand();
      }

      switch (m_node1.asNode().compareOrder(m_node2.asNode())) {
         case QXmlNodeModelIndex::Precedes:
            return fromFirstOperand();
         case QXmlNodeModelIndex::Follows: {
            m_node2 = m_it2->next();
            if (m_node2) {
               continue;
            } else {
               return fromFirstOperand();
            }
         }
         default: {
            m_node1 = m_it1->next();
            m_node2 = m_it2->next();
         }
      }
   }
}

Item ExceptIterator::current() const
{
   return m_current;
}

xsInteger ExceptIterator::position() const
{
   return m_position;
}

Item::Iterator::Ptr ExceptIterator::copy() const
{
   return Item::Iterator::Ptr(new ExceptIterator(m_it1->copy(), m_it2->copy()));
}

QT_END_NAMESPACE
