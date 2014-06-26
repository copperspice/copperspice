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

#ifndef QSubsequenceIterator_P_H
#define QSubsequenceIterator_P_H

#include <qitem_p.h>

QT_BEGIN_NAMESPACE

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

   virtual Item next();
   virtual Item current() const;
   virtual xsInteger position() const;
   virtual Item::Iterator::Ptr copy() const;

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

QT_END_NAMESPACE

#endif
