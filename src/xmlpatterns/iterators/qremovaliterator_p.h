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

#ifndef QRemovalIterator_P_H
#define QRemovalIterator_P_H

#include <qitem_p.h>

namespace QPatternist {

class RemovalIterator : public Item::Iterator
{
 public:

   /**
    * Creates an RemovalIterator.
    *
    * @param target the QAbstractXmlForwardIterator containing the sequence of items
    * which the item at position @p position should be removed from.
    * @param position the position of the item to remove. Must be
    * 1 or larger.
    */
   RemovalIterator(const Item::Iterator::Ptr &target,
                   const xsInteger position);

   Item next() override;
   Item current() const override;
   xsInteger position() const override;

   /**
    * The QAbstractXmlForwardIterator's count is computed by subtracting one from the source
    * QAbstractXmlForwardIterator's count.
    */
   xsInteger count() override;

   Item::Iterator::Ptr copy() const override;

 private:
   const Item::Iterator::Ptr m_target;
   const xsInteger m_removalPos;
   Item m_current;
   xsInteger m_position;
};

}

#endif
