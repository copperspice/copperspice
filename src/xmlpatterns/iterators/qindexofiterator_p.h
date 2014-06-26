/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QIndexOfIterator_P_H
#define QIndexOfIterator_P_H

#include <qitem_p.h>
#include <qatomiccomparator_p.h>
#include <qcomparisonplatform_p.h>
#include <qdynamiccontext_p.h>
#include <qexpression_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class IndexOfIterator : public Item::Iterator
   , public ComparisonPlatform<IndexOfIterator, false>
   , public SourceLocationReflection
{
 public:

   /**
    * Creates an IndexOfIterator, whose next() function returns integers being
    * the index positions of where @p searchParam was found in @p inputSequence.
    *
    * @param comp the AtomicComparator to be used for comparing values. This may be @c null,
    * meaning the IndexOfIterator iterator will dynamically determine what comparator to use
    * on an item per item basis, which is slower.
    * @param searchParam the item which should be compared to the items in @p inputSequence.
    * @param inputSequence the input sequence which indexes of the @p searchParam should
    * be returned for.
    * @param context the usual DynamicContext
    * @param expr the Expression that this IndexOfIterator is evaluating
    * for. It is used for error reporting, via actualReflection().
    */
   IndexOfIterator(const Item::Iterator::Ptr &inputSequence,
                   const Item &searchParam,
                   const AtomicComparator::Ptr &comp,
                   const DynamicContext::Ptr &context,
                   const Expression::ConstPtr &expr);

   virtual Item next();
   virtual Item current() const;
   virtual xsInteger position() const;
   virtual Item::Iterator::Ptr copy() const;

   inline AtomicComparator::Operator operatorID() const {
      return AtomicComparator::OperatorEqual;
   }

   virtual const SourceLocationReflection *actualReflection() const;

 private:
   const Item::Iterator::Ptr   m_seq;
   const Item                  m_searchParam;
   const DynamicContext::Ptr   m_context;
   const Expression::ConstPtr  m_expr;
   Item                        m_current;
   xsInteger                   m_position;
   xsInteger                   m_seqPos;
};
}

QT_END_NAMESPACE

#endif
