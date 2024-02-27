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

#ifndef QAccelIterators_P_H
#define QAccelIterators_P_H

#include <qacceltree_p.h>
#include <qitem_p.h>

namespace QPatternist {

class AccelIterator : public QXmlNodeModelIndex::Iterator
{
 public:
   xsInteger position() const override;
   QXmlNodeModelIndex current() const override;

 protected:
   inline AccelIterator(const AccelTree *const doc,
                        const AccelTree::PreNumber pre,
                        const AccelTree::PreNumber currentPre) : m_document(doc)
      , m_preNumber(pre)
      , m_currentPre(currentPre)
      , m_position(0)

   {
      Q_ASSERT(m_document);
      Q_ASSERT(m_preNumber >= 0);
   }

   inline QXmlNodeModelIndex closedExit() {
      m_position = -1;
      m_current.reset();
      return QXmlNodeModelIndex();
   }

   /**
    * We do not own it.
    */
   const AccelTree *const      m_document;

   /**
    * The pre number of the node that should be navigated from.
    */
   const AccelTree::PreNumber  m_preNumber;
   AccelTree::PreNumber        m_currentPre;
   xsInteger                   m_position;
   QXmlNodeModelIndex          m_current;
};


template<const bool IncludeSelf>
class AncestorIterator : public AccelIterator
{
 public:
   /**
    * @p pre is the node from which iteration starts
    * from. In the @c ancestor axis it is excluded,
    * while in @c ancestor-or-self it is included. @p pre
    * must have at least one ancestor.
    */
   inline AncestorIterator(const AccelTree *const doc,
                           const AccelTree::PreNumber pre) : AccelIterator(doc, pre, IncludeSelf ? pre : doc->basicData.at(pre).parent()) {
      Q_ASSERT(IncludeSelf || m_document->hasParent(pre));
   }

   QXmlNodeModelIndex next() override {
      if (m_currentPre == -1) {
         return closedExit();
      } else {
         ++m_position;
         m_current = m_document->createIndex(m_currentPre);
         m_currentPre = m_document->basicData.at(m_currentPre).parent();

         return m_current;
      }
   }

   QXmlNodeModelIndex::Iterator::Ptr copy() const override {
      return QXmlNodeModelIndex::Iterator::Ptr(new AncestorIterator<IncludeSelf>(m_document, m_preNumber));
   }
};

class ChildIterator : public AccelIterator
{
 public:
   /**
    * @p pre must have at least one child.
    */
   inline ChildIterator(const AccelTree *const doc,
                        const AccelTree::PreNumber pre) : AccelIterator(doc, pre, pre + 1),
      m_depth(m_document->depth(m_currentPre)) {
      Q_ASSERT(m_document->hasChildren(pre));

      /* Skip the attributes, that are children in the pre/post plane, of
       * the node we're applying the child axis to. */
      while (m_document->kind(m_currentPre) == QXmlNodeModelIndex::Attribute) {
         ++m_currentPre;
         /* We check the depth here because we would otherwise include
          * following siblings. */
         if (m_currentPre > m_document->maximumPreNumber() || m_document->depth(m_currentPre) != m_depth) {
            m_currentPre = -1;
            break;
         }
      }
   }

   QXmlNodeModelIndex next() override;
   QXmlNodeModelIndex::Iterator::Ptr copy() const override;

 private:
   const AccelTree::Depth m_depth;
};

template<const bool IsFollowing>
class SiblingIterator : public AccelIterator
{
 public:
   inline SiblingIterator(const AccelTree *const doc,
                          const AccelTree::PreNumber pre) : AccelIterator(doc, pre, pre + (IsFollowing ? 0 : -1)),
      m_depth(doc->depth(pre)) {
      Q_ASSERT_X(IsFollowing || pre != 0, "",
                 "When being preceding-sibling, the context node cannot be the first node in the document.");
      Q_ASSERT_X(!IsFollowing || pre != m_document->maximumPreNumber(), "",
                 "When being following-sibling, the context node cannot be the last node in the document.");
   }

   QXmlNodeModelIndex next() override {
      if (m_currentPre == -1) {
         return QXmlNodeModelIndex();
      }

      if (IsFollowing) {
         /* Skip the descendants, and jump to the next node. */
         m_currentPre += m_document->size(m_currentPre) + 1;

         if (m_currentPre > m_document->maximumPreNumber() || m_document->depth(m_currentPre) != m_depth) {
            return closedExit();
         } else {
            ++m_position;
            m_current = m_document->createIndex(m_currentPre);
            return m_current;
         }
      } else {
         while (m_document->depth(m_currentPre) > m_depth) {
            --m_currentPre;
         }

         while (m_document->kind(m_currentPre) == QXmlNodeModelIndex::Attribute) {
            --m_currentPre;
         }

         if (m_document->depth(m_currentPre) == m_depth &&
               m_document->kind(m_currentPre) != QXmlNodeModelIndex::Attribute) {
            m_current = m_document->createIndex(m_currentPre);
            ++m_position;
            --m_currentPre;
            return m_current;
         } else {
            m_currentPre = -1;
            return closedExit();
         }
      }
   }

   QXmlNodeModelIndex::Iterator::Ptr copy() const override {
      return QXmlNodeModelIndex::Iterator::Ptr(new SiblingIterator<IsFollowing>(m_document, m_preNumber));
   }

 private:
   const AccelTree::Depth m_depth;
};

template<const bool IncludeSelf>
class DescendantIterator : public AccelIterator
{
 public:
   /**
    * @p pre must have at least one child.
    */
   inline DescendantIterator(const AccelTree *const doc,
                             const AccelTree::PreNumber pre) : AccelIterator(doc, pre, pre + (IncludeSelf ? 0 : 1)),
      m_postNumber(doc->postNumber(pre)) {
      Q_ASSERT(IncludeSelf || m_document->hasChildren(pre));

      /* Make sure that m_currentPre is the first node part of this axis.
       * Since we're not including ourself, advance to the node after our
       * attributes, if any. */
      if (!IncludeSelf) {
         while (m_document->kind(m_currentPre) == QXmlNodeModelIndex::Attribute) {
            ++m_currentPre;
            /* We check the depth here because we would otherwise include
             * following siblings. */
            if (m_currentPre > m_document->maximumPreNumber() || m_document->postNumber(m_currentPre) > m_postNumber) {
               m_currentPre = -1;
               break;
            }
         }
      }
   }

   QXmlNodeModelIndex next() override {
      if (m_currentPre == -1) {
         return closedExit();
      }

      ++m_position;
      m_current = m_document->createIndex(m_currentPre);

      ++m_currentPre;

      if (m_currentPre > m_document->maximumPreNumber()) {
         m_currentPre = -1;
         return m_current;
      }

      if (m_document->postNumber(m_currentPre) < m_postNumber) {
         while (m_document->kind(m_currentPre) == QXmlNodeModelIndex::Attribute) {
            ++m_currentPre;
            if (m_currentPre > m_document->maximumPreNumber()) {
               m_currentPre = -1;
               break;
            }
         }
      } else {
         m_currentPre = -1;
      }

      return m_current;
   }

   QXmlNodeModelIndex::Iterator::Ptr copy() const override {
      return QXmlNodeModelIndex::Iterator::Ptr(new DescendantIterator<IncludeSelf>(m_document, m_preNumber));
   }

 private:
   const AccelTree::PreNumber m_postNumber;
};

class FollowingIterator : public AccelIterator
{
 public:
   /**
    * @ pre must have at least one child.
    */
   inline FollowingIterator(const AccelTree *const doc,
                            const AccelTree::PreNumber pre) : AccelIterator(doc, pre, pre) {
   }

   QXmlNodeModelIndex next() override;
   QXmlNodeModelIndex::Iterator::Ptr copy() const override;
};

class PrecedingIterator : public AccelIterator
{
 public:
   /**
    * @ pre must have at least one child.
    */
   PrecedingIterator(const AccelTree *const doc, const AccelTree::PreNumber pre)
                  : AccelIterator(doc, pre, pre - 1 /* currentPre */),
                  m_postNumber(m_document->postNumber(m_preNumber)) {
   }

   QXmlNodeModelIndex next() override;
   QXmlNodeModelIndex::Iterator::Ptr copy() const override;

 private:
   const AccelTree::PreNumber  m_postNumber;
};

class AttributeIterator : public AccelIterator
{
 public:
   /**
    * @p pre must have at least one child.
    */
   inline AttributeIterator(const AccelTree *const doc, const AccelTree::PreNumber pre)
                  : AccelIterator(doc, pre, pre + 1) {

      Q_ASSERT(m_document->hasChildren(pre));
      Q_ASSERT(m_document->kind(m_currentPre) == QXmlNodeModelIndex::Attribute);
   }

   QXmlNodeModelIndex next() override;
   QXmlNodeModelIndex::Iterator::Ptr copy() const override;
};

}

#endif
