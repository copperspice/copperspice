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

#include "qdaytimeduration_p.h"

#include "qfocus_p.h"

using namespace QPatternist;

Focus::Focus(const DynamicContext::Ptr &prevContext) : DelegatingDynamicContext(prevContext),
   m_contextSizeCached(-1)
{
   Q_ASSERT(prevContext);
   Q_ASSERT(prevContext != this);
}

xsInteger Focus::contextPosition() const
{
   Q_ASSERT(m_focusIterator);
   return m_focusIterator->position();
}

Item Focus::contextItem() const
{
   Q_ASSERT(m_focusIterator);
   return m_focusIterator->current();
}

xsInteger Focus::contextSize()
{
   Q_ASSERT(m_focusIterator);
   if (m_contextSizeCached == -1) {
      m_contextSizeCached = m_focusIterator->copy()->count();
   }

   Q_ASSERT_X(m_contextSizeCached == m_focusIterator->copy()->count(), Q_FUNC_INFO,
              "If our cache is not the same as the real count, something is wrong.");

   return m_contextSizeCached;
}

void Focus::setFocusIterator(const Item::Iterator::Ptr &it)
{
   Q_ASSERT(it);
   m_focusIterator = it;
}

Item::Iterator::Ptr Focus::focusIterator() const
{
   return m_focusIterator;
}

Item Focus::currentItem() const
{
   /* In the case that there is no top level expression that creates a focus,
    * fn:current() should return the focus. This logic achieves this.
    * Effectively we traverse up our "context stack" through recursion, and
    * start returning when we've found the top most focus. */

   const Item current(m_prevContext->currentItem());

   if (current.isNull()) {
      return m_focusIterator->current();
   } else {
      return current;
   }
}

