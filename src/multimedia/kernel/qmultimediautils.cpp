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

#include <qmultimediautils_p.h>

void qt_real_to_fraction(qreal value, int *numerator, int *denominator)
{
   if (!numerator || !denominator) {
      return;
   }

   const int dMax = 1000;
   int n1 = 0, d1 = 1, n2 = 1, d2 = 1;
   qreal mid = 0.;
   while (d1 <= dMax && d2 <= dMax) {
      mid = qreal(n1 + n2) / (d1 + d2);

      if (qAbs(value - mid) < 0.000001) {
         if (d1 + d2 <= dMax) {
            *numerator = n1 + n2;
            *denominator = d1 + d2;
            return;
         } else if (d2 > d1) {
            *numerator = n2;
            *denominator = d2;
            return;
         } else {
            *numerator = n1;
            *denominator = d1;
            return;
         }
      } else if (value > mid) {
         n1 = n1 + n2;
         d1 = d1 + d2;
      } else {
         n2 = n1 + n2;
         d2 = d1 + d2;
      }
   }

   if (d1 > dMax) {
      *numerator = n2;
      *denominator = d2;
   } else {
      *numerator = n1;
      *denominator = d1;
   }
}

