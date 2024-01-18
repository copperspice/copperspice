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

#ifndef QFocus_P_H
#define QFocus_P_H

#include <qdelegatingdynamiccontext_p.h>

namespace QPatternist {

class Focus : public DelegatingDynamicContext
{
 public:
   Focus(const DynamicContext::Ptr &prevContext);

   xsInteger contextPosition() const override;
   Item contextItem() const override;
   xsInteger contextSize() override;

   void setFocusIterator(const Item::Iterator::Ptr &it) override;
   Item::Iterator::Ptr focusIterator() const override;

   /**
    * If there is no top level expression that sets the current item,
    * the focus should be used. This implementation ensures that.
    */
   Item currentItem() const override;

 private:
   Item::Iterator::Ptr m_focusIterator;
   xsInteger           m_contextSizeCached;
};
}

#endif
