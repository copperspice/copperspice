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

#ifndef QRangeIterator_P_H
#define QRangeIterator_P_H

#include <qitem_p.h>

namespace QPatternist {

class RangeIterator : public Item::Iterator
{
 public:

   /**
    * RangeIterator can iterate in both directions.
    * This enumerator exist for identifying different directions.
    */
   enum Direction {
      /**
       * Signifies that the QAbstractXmlForwardIterator operates in a reverse direction, where the
       * first item returned by the next() function is from the beginning of the
       * source sequence.
       */
      Backward = 0,

      /**
       * Signifies the forward direction. Iterators do conceptually operate
       * in the forward direction by default.
       */
      Forward = 1
   };

   /**
    * Creates an QAbstractXmlForwardIterator that returns integer values from consecutive sequence
    * of integers between @p start and @p end, where the step taken
    * between each integer is 1 with polarity as specified in @p direction.
    *
    * @note @p start must be smaller than @p end, not larger
    * or equal. This is not checked.
    */
   RangeIterator(const xsInteger start, const Direction direction, const xsInteger end);

   Item next() override;
   Item current() const override;
   xsInteger position() const override;
   xsInteger count() override;
   Item::Iterator::Ptr toReversed() override;
   Item::Iterator::Ptr copy() const override;

 private:
   xsInteger m_start;
   xsInteger m_end;
   Item m_current;
   xsInteger m_position;
   xsInteger m_count;
   const Direction m_direction;

   /**
    * We only need to store -1 or 1, so save memory with a bit field.
    */
   const qint8 m_increment : 2;
};

}

#endif
