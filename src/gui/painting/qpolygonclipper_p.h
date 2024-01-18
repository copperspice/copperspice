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

#ifndef QPOLYGONCLIPPER_P_H
#define QPOLYGONCLIPPER_P_H

#include <qvector.h>

/* based on sutherland-hodgman line-by-line clipping, as described in
   Computer Graphics and Principles */
template <typename InType, typename OutType, typename CastType> class QPolygonClipper
{
 public:
   QPolygonClipper()
   {
      x1 = y1 = x2 = y2 = 0;
   }

   ~QPolygonClipper() {
   }

   void setBoundingRect(const QRect bounds) {
      x1 = bounds.x();
      x2 = bounds.x() + bounds.width();
      y1 = bounds.y();
      y2 = bounds.y() + bounds.height();
   }

   QRect boundingRect() {
      return QRect(QPoint(x1, y1), QPoint(x2, y2));
   }

   inline OutType intersectLeft(const OutType &p1, const OutType &p2) {
      OutType t;
      qreal dy = (p1.y - p2.y) / qreal(p1.x - p2.x);
      t.x = x1;
      t.y = static_cast<CastType>(p2.y + (x1 - p2.x) * dy);
      return t;
   }

   inline OutType intersectRight(const OutType &p1, const OutType &p2) {
      OutType t;
      qreal dy = (p1.y - p2.y) / qreal(p1.x - p2.x);
      t.x = x2;
      t.y = static_cast<CastType>(p2.y + (x2 - p2.x) * dy);
      return t;
   }

   inline OutType intersectTop(const OutType &p1, const OutType &p2) {
      OutType t;
      qreal dx = (p1.x - p2.x) / qreal(p1.y - p2.y);
      t.x = static_cast<CastType>(p2.x + (y1 - p2.y) * dx);
      t.y = y1;
      return t;
   }

   inline OutType intersectBottom(const OutType &p1, const OutType &p2) {
      OutType t;
      qreal dx = (p1.x - p2.x) / qreal(p1.y - p2.y);
      t.x = static_cast<CastType>(p2.x + (y2 - p2.y) * dx);
      t.y = y2;
      return t;
   }

   void clipPolygon(const InType *inPoints, int inCount, OutType **outPoints, int *outCount, bool closePolygon = true) {
      Q_ASSERT(outPoints);
      Q_ASSERT(outCount);

      if (inCount < 2) {
         *outCount = 0;
         return;
      }

      buffer1.reset();
      buffer2.reset();

      QVector<OutType> *source  = &buffer1;
      QVector<OutType> *clipped = &buffer2;

      // Gather some info since we are iterating through the points anyway..
      bool doLeft = false, doRight = false, doTop = false, doBottom = false;
      OutType ot;

      for (int i = 0; i < inCount; ++i) {
         ot = inPoints[i];
         clipped->add(ot);

         if (ot.x < x1) {
            doLeft = true;
         } else if (ot.x > x2) {
            doRight = true;
         }
         if (ot.y < y1) {
            doTop = true;
         } else if (ot.y > y2) {
            doBottom = true;
         }
      }

      if (doLeft && clipped->size() > 1) {
         QVector<OutType> *tmp = source;

         source = clipped;
         clipped = tmp;
         clipped->reset();
         int lastPos, start;
         if (closePolygon) {
            lastPos = source->size() - 1;
            start = 0;
         } else {
            lastPos = 0;
            start = 1;
            if (source->at(0).x >= x1) {
               clipped->add(source->at(0));
            }
         }
         for (int i = start; i < inCount; ++i) {
            const OutType &cpt = source->at(i);
            const OutType &ppt = source->at(lastPos);

            if (cpt.x >= x1) {
               if (ppt.x >= x1) {
                  clipped->add(cpt);
               } else {
                  clipped->add(intersectLeft(cpt, ppt));
                  clipped->add(cpt);
               }
            } else if (ppt.x >= x1) {
               clipped->add(intersectLeft(cpt, ppt));
            }
            lastPos = i;
         }
      }

      if (doRight && clipped->size() > 1) {
         QVector<OutType> *tmp = source;

         source = clipped;
         clipped = tmp;
         clipped->reset();
         int lastPos, start;
         if (closePolygon) {
            lastPos = source->size() - 1;
            start = 0;
         } else {
            lastPos = 0;
            start = 1;
            if (source->at(0).x <= x2) {
               clipped->add(source->at(0));
            }
         }
         for (int i = start; i < source->size(); ++i) {
            const OutType &cpt = source->at(i);
            const OutType &ppt = source->at(lastPos);

            if (cpt.x <= x2) {
               if (ppt.x <= x2) {
                  clipped->add(cpt);
               } else {
                  clipped->add(intersectRight(cpt, ppt));
                  clipped->add(cpt);
               }
            } else if (ppt.x <= x2) {
               clipped->add(intersectRight(cpt, ppt));
            }

            lastPos = i;
         }

      }

      if (doTop && clipped->size() > 1) {
         QVector<OutType> *tmp = source;

         source = clipped;
         clipped = tmp;
         clipped->reset();
         int lastPos, start;
         if (closePolygon) {
            lastPos = source->size() - 1;
            start = 0;
         } else {
            lastPos = 0;
            start = 1;
            if (source->at(0).y >= y1) {
               clipped->add(source->at(0));
            }
         }
         for (int i = start; i < source->size(); ++i) {
            const OutType &cpt = source->at(i);
            const OutType &ppt = source->at(lastPos);

            if (cpt.y >= y1) {
               if (ppt.y >= y1) {
                  clipped->add(cpt);
               } else {
                  clipped->add(intersectTop(cpt, ppt));
                  clipped->add(cpt);
               }
            } else if (ppt.y >= y1) {
               clipped->add(intersectTop(cpt, ppt));
            }

            lastPos = i;
         }
      }

      if (doBottom && clipped->size() > 1) {
         QVector<OutType> *tmp = source;

         source = clipped;
         clipped = tmp;
         clipped->reset();
         int lastPos, start;
         if (closePolygon) {
            lastPos = source->size() - 1;
            start = 0;
         } else {
            lastPos = 0;
            start = 1;
            if (source->at(0).y <= y2) {
               clipped->add(source->at(0));
            }
         }
         for (int i = start; i < source->size(); ++i) {
            const OutType &cpt = source->at(i);
            const OutType &ppt = source->at(lastPos);

            if (cpt.y <= y2) {
               if (ppt.y <= y2) {
                  clipped->add(cpt);
               } else {
                  clipped->add(intersectBottom(cpt, ppt));
                  clipped->add(cpt);
               }
            } else if (ppt.y <= y2) {
               clipped->add(intersectBottom(cpt, ppt));
            }
            lastPos = i;
         }
      }

      if (closePolygon && clipped->size() > 0) {
         // close clipped polygon
         if (clipped->at(0).x != clipped->at(clipped->size() - 1).x ||
            clipped->at(0).y != clipped->at(clipped->size() - 1).y) {
            OutType ot = clipped->at(0);
            clipped->add(ot);
         }
      }
      *outCount = clipped->size();
      *outPoints = clipped->data();
   }

 private:
   int x1;
   int x2;
   int y1;
   int y2;

   QVector<OutType> buffer1;
   QVector<OutType> buffer2;
};

#endif // QPOLYGONCLIPPER_P_H
