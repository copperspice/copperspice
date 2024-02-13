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

#ifndef QTCONCURRENTMEDIAN_H
#define QTCONCURRENTMEDIAN_H

#include <qglobal.h>
#include <qvector.h>

#include <algorithm>

namespace QtConcurrent {

template <typename T>
class Median
{
 public:
   Median(int _bufferSize)
      : currentMedian(), bufferSize(_bufferSize), currentIndex(0), valid(false), dirty(true)
   {
      values.resize(bufferSize);
   }

   void reset() {
      values.fill(0);
      currentIndex = 0;
      valid = false;
      dirty = true;
   }

   void addValue(T value) {
      currentIndex = ((currentIndex + 1) % bufferSize);

      if (valid == false && currentIndex % bufferSize == 0) {
         valid = true;
      }

      // Only update the cached median value when we have to, that
      // is when the new value is on then other side of the median
      // compared to the current value at the index.
      const T currentIndexValue = values[currentIndex];

      if ((currentIndexValue > currentMedian && currentMedian > value)
            || (currentMedian > currentIndexValue && value > currentMedian)) {
         dirty = true;
      }

      values[currentIndex] = value;
   }

   bool isMedianValid() const {
      return valid;
   }

   T median() {
      if (dirty) {
         dirty = false;

         QVector<T> sorted = values;
         std::sort(sorted.begin(), sorted.end());

         currentMedian = sorted.at(bufferSize / 2 + 1);
      }

      return currentMedian;
   }

 private:
   QVector<T> values;
   T currentMedian;
   int bufferSize;
   int currentIndex;
   bool valid;
   bool dirty;
};

} // namespace QtConcurrent

#endif
