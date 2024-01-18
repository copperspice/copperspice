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

#include <qacceliterators_p.h>

#include <qdebug.h>

using namespace QPatternist;

xsInteger AccelIterator::position() const
{
   return m_position;
}

QXmlNodeModelIndex AccelIterator::current() const
{
   return m_current;
}

QXmlNodeModelIndex FollowingIterator::next()
{
   /* "the following axis contains all nodes that are descendants
    *  of the root of the tree in which the context node is found,
    *  are not descendants of the context node, and occur after
    *  the context node in document order." */

   if (m_position == 0) {
      /* Skip the descendants. */
      m_currentPre += m_document->size(m_preNumber) + 1;
   }

   if (m_currentPre > m_document->maximumPreNumber()) {
      return closedExit();
   }

   while (m_document->kind(m_currentPre) == QXmlNodeModelIndex::Attribute) {
      ++m_currentPre;
      if (m_currentPre > m_document->maximumPreNumber()) {
         return closedExit();
      }
   }

   m_current = m_document->createIndex(m_currentPre);
   ++m_position;
   ++m_currentPre;
   return m_current;
}

QXmlNodeModelIndex PrecedingIterator::next()
{
   if (m_currentPre == -1) {
      return closedExit();
   }

   /* We skip ancestors and attributes and take into account that they can be intermixed. If one
    * skips them in two separate loops, one can end up with skipping all the attributes to then
    * be positioned at an ancestor(which will be accepted because the ancestor loop was before the
    * attributes loop).  */
   while (m_document->kind(m_currentPre) == QXmlNodeModelIndex::Attribute ||
          m_document->postNumber(m_currentPre) > m_postNumber) {
      --m_currentPre;
      if (m_currentPre == -1) {
         return closedExit();
      }
   }

   if (m_currentPre == -1) {
      m_currentPre = -1;
      return closedExit();
   }

   /* Phew, m_currentPre is now 1) not an ancestor; and
    * 2) not an attribute; and 3) preceds the context node. */

   m_current = m_document->createIndex(m_currentPre);
   ++m_position;
   --m_currentPre;

   return m_current;
}

QXmlNodeModelIndex::Iterator::Ptr PrecedingIterator::copy() const
{
   return QXmlNodeModelIndex::Iterator::Ptr(new PrecedingIterator(m_document, m_preNumber));
}

QXmlNodeModelIndex::Iterator::Ptr FollowingIterator::copy() const
{
   return QXmlNodeModelIndex::Iterator::Ptr(new FollowingIterator(m_document, m_preNumber));
}

QXmlNodeModelIndex ChildIterator::next()
{
   if (m_currentPre == -1) {
      return closedExit();
   }

   ++m_position;
   m_current = m_document->createIndex(m_currentPre);

   /* We get the count of the descendants, and increment m_currentPre. After
    * this, m_currentPre is the node after the descendants. */
   m_currentPre += m_document->size(m_currentPre);
   ++m_currentPre;

   if (m_currentPre > m_document->maximumPreNumber() || m_document->depth(m_currentPre) != m_depth) {
      m_currentPre = -1;
   }

   return m_current;
}

QXmlNodeModelIndex::Iterator::Ptr ChildIterator::copy() const
{
   return QXmlNodeModelIndex::Iterator::Ptr(new ChildIterator(m_document, m_preNumber));
}

QXmlNodeModelIndex AttributeIterator::next()
{
   if (m_currentPre == -1) {
      return closedExit();
   } else {
      m_current = m_document->createIndex(m_currentPre);
      ++m_position;

      ++m_currentPre;

      if (m_currentPre > m_document->maximumPreNumber() ||
            m_document->kind(m_currentPre) != QXmlNodeModelIndex::Attribute) {
         m_currentPre = -1;
      }

      return m_current;
   }
}

QXmlNodeModelIndex::Iterator::Ptr AttributeIterator::copy() const
{
   return QXmlNodeModelIndex::Iterator::Ptr(new AttributeIterator(m_document, m_preNumber));
}
