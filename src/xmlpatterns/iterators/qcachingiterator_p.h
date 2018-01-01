/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QCachingIterator_P_H
#define QCachingIterator_P_H

#include <QList>
#include <QVector>
#include <qdynamiccontext_p.h>
#include <qitem_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class CachingIterator : public Item::Iterator
{
 public:
   /**
    * We always use the same cache cell so why don't we use it directly,
    * instead of passing the slot and ItemSequenceCacheCell::Vector to
    * this class? Because the GenericDynamicContext might decide to resize
    * the vector and that would invalidate the reference.
    *
    * We intentionally pass in a non-const reference here.
    */
   CachingIterator(ItemSequenceCacheCell::Vector &cacheCells, const VariableSlotID slot, const DynamicContext::Ptr &context);

   Item next() override;
   Item current() const override;
   xsInteger position() const override;
   Item::Iterator::Ptr copy() const override;

 private:
   Item      m_current;
   xsInteger m_position;

   /**
    * This variable cannot be called m_slot, because
    * /usr/include/sys/sysmacros.h on hpuxi-acc defines it.
    */
   const VariableSlotID        m_varSlot;

   /**
    * We don't use the context. We only keep a reference such that it
    * doesn't get deleted, and m_cacheCells starts to dangle.
    */
   const DynamicContext::Ptr   m_context;

   /**
    * We intentionally store a reference here such that we are able to
    * modify the item.
    */
   ItemSequenceCacheCell::Vector &m_cacheCells;

   /**
    * Whether this CachingIterator is delivering items from
    * m_cacheCell.cacheItems or from m_cacheCell.sourceIterator.
    */
   bool m_usingCache;
};
}

QT_END_NAMESPACE

#endif
