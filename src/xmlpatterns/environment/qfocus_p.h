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

#ifndef QFocus_P_H
#define QFocus_P_H

#include <qdelegatingdynamiccontext_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class Focus : public DelegatingDynamicContext
{
 public:
   Focus(const DynamicContext::Ptr &prevContext);

   virtual xsInteger contextPosition() const;
   virtual Item contextItem() const;
   virtual xsInteger contextSize();

   virtual void setFocusIterator(const Item::Iterator::Ptr &it);
   virtual Item::Iterator::Ptr focusIterator() const;

   /**
    * If there is no top level expression that sets the current item,
    * the focus should be used. This implementation ensures that.
    */
   virtual Item currentItem() const;

 private:
   Item::Iterator::Ptr m_focusIterator;
   xsInteger           m_contextSizeCached;
};
}

QT_END_NAMESPACE

#endif
