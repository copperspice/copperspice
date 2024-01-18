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

#ifndef QDeduplicateIterator_P_H
#define QDeduplicateIterator_P_H

#include <qlist.h>

#include <qexpression_p.h>
#include <qitem_p.h>
#include <qatomiccomparator_p.h>
#include <qcomparisonplatform_p.h>
#include <qsourcelocationreflection_p.h>

namespace QPatternist {

class DeduplicateIterator : public ListIterator<Item>
{
 public:
   DeduplicateIterator(const Item::List &source);

   Item next() override;
   Item::Iterator::Ptr copy() const override;
   xsInteger count() override;

 private:
   /**
    * m_position in ListIteratorPlatform is the position that we
    * show to the outside through position) but do not correspond
    * to the position in m_list, since we skip entries in that one.
    *
    * However, this guy, m_listPos, is the position into m_list.
    */
   int m_listPos;
};

}

#endif
