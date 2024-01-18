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

#ifndef QDistinctIterator_P_H
#define QDistinctIterator_P_H

#include <qlist.h>

#include <qexpression_p.h>
#include <qitem_p.h>
#include <qatomiccomparator_p.h>
#include <qcomparisonplatform_p.h>
#include <qsourcelocationreflection_p.h>

namespace QPatternist {

class DistinctIterator : public Item::Iterator, public ComparisonPlatform<DistinctIterator, false>,
      public SourceLocationReflection
{
 public:
   /**
    * Creates a DistinctIterator.
    * @param comp the AtomicComparator to be used for comparing values. This may be @c null,
    * meaning the IndexOfIterator iterator will dynamically determine what comparator to use
    * @param seq the sequence whose duplicates should be filtered out
    * @param context the usual context, used for error reporting and by AtomicComparators.
    * @param expression the Expression that this DistinctIterator is
    * evaluating for. It is used for error reporting, via
    * actualReflection().
    */
   DistinctIterator(const Item::Iterator::Ptr &seq, const AtomicComparator::Ptr &comp,
                    const Expression::ConstPtr &expression, const DynamicContext::Ptr &context);

   Item next() override;
   Item current() const override;
   xsInteger position() const override;
   Item::Iterator::Ptr copy() const override;
   const SourceLocationReflection *actualReflection() const override;

   AtomicComparator::Operator operatorID() const {
      return AtomicComparator::OperatorEqual;
   }

 private:
   const Item::Iterator::Ptr   m_seq;
   const DynamicContext::Ptr   m_context;
   const Expression::ConstPtr  m_expr;
   Item                        m_current;
   xsInteger                   m_position;
   Item::List                  m_processed;
};
}

#endif
