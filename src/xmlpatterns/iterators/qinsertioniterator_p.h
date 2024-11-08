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

#ifndef QInsertionIterator_P_H
#define QInsertionIterator_P_H

#include <qabstractxmlforwarditerator_p.h>
#include <qitem_p.h>

namespace QPatternist {

class InsertionIterator : public Item::Iterator
{
 public:
   InsertionIterator(const Item::Iterator::Ptr &target,
                     const xsInteger position,
                     const Item::Iterator::Ptr &insertIterator);

   Item next() override;
   Item current() const override;
   xsInteger position() const override;
   xsInteger count() override;
   Item::Iterator::Ptr copy() const override;

 private:
   const Item::Iterator::Ptr m_target;
   const xsInteger m_insertPos;
   const Item::Iterator::Ptr m_inserts;
   Item m_current;
   xsInteger m_position;
   bool m_isInserting;
};

}

#endif
