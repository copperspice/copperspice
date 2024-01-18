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

#ifndef QSubsequenceIterator_P_H
#define QSubsequenceIterator_P_H

#include <qitem_p.h>

namespace QPatternist {

class SubsequenceIterator : public Item::Iterator
{
 public:
   /**
    * Creates a SubsequenceIterator that extracts a subsequence from the sequence
    * in @p iterator, as specified by the @p start position and @p length parameter.
    *
    * @param iterator the iterator which the subsequence should
    * be extracted from
    * @param start the start position of extraction. Must be 1 or larger.
    * @param length the length of the subsequence to extract. If it is
    * -1, to the end is returned. The value must be -1 or 1 or larger.
    */
   SubsequenceIterator(const Item::Iterator::Ptr &iterator,
                       const xsInteger start,
                       const xsInteger length);

   Item next() override;
   Item current() const override;
   xsInteger position() const override;
   Item::Iterator::Ptr copy() const override;

 private:
   xsInteger m_position;
   Item m_current;
   const Item::Iterator::Ptr m_it;
   xsInteger m_counter;
   const xsInteger m_start;
   const xsInteger m_len;
   const xsInteger m_stop;
};

}

#endif
