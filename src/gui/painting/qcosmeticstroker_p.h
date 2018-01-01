/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QCOSMETICSTROKER_P_H
#define QCOSMETICSTROKER_P_H

#include <qdrawhelper_p.h>
#include <qvectorpath_p.h>
#include <qpaintengine_raster_p.h>
#include <qpen.h>

QT_BEGIN_NAMESPACE

class QCosmeticStroker;

typedef bool (*StrokeLine)(QCosmeticStroker *stroker, qreal x1, qreal y1, qreal x2, qreal y2, int caps);

class QCosmeticStroker
{
 public:
   struct Point {
      int x;
      int y;
   };
   struct PointF {
      qreal x;
      qreal y;
   };

   enum Caps {
      NoCaps = 0,
      CapBegin = 0x1,
      CapEnd = 0x2,
   };

   // used to avoid drop outs or duplicated points
   enum Direction {
      TopToBottom = 0x1,
      BottomToTop = 0x2,
      LeftToRight = 0x4,
      RightToLeft = 0x8,
      VerticalMask = 0x3,
      HorizontalMask = 0xc
   };

   QCosmeticStroker(QRasterPaintEngineState *s, const QRect &dr, const QRect &dr_unclipped)
      : state(s),
        deviceRect(dr_unclipped),
        clip(dr),
        pattern(0),
        reversePattern(0),
        patternSize(0),
        patternLength(0),
        patternOffset(0),
        current_span(0),
        lastDir(LeftToRight),
        lastAxisAligned(false) {
      setup();
   }

   ~QCosmeticStroker() {
      free(pattern);
      free(reversePattern);
   }

   void drawLine(const QPointF &p1, const QPointF &p2);
   void drawPath(const QVectorPath &path);
   void drawPoints(const QPoint *points, int num);
   void drawPoints(const QPointF *points, int num);


   QRasterPaintEngineState *state;
   QRect deviceRect;
   QRect clip;
   // clip bounds in real
   qreal xmin, xmax;
   qreal ymin, ymax;

   StrokeLine stroke;
   bool drawCaps;

   int *pattern;
   int *reversePattern;
   int patternSize;
   int patternLength;
   int patternOffset;

   enum { NSPANS = 255 };
   QT_FT_Span spans[NSPANS];
   int current_span;
   ProcessSpans blend;

   int opacity;

   uint color;
   uint *pixels;
   int ppl;

   Direction lastDir;
   Point lastPixel;
   bool lastAxisAligned;

   bool clipLine(qreal &x1, qreal &y1, qreal &x2, qreal &y2);

 private:
   void setup();

   void renderCubic(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4, int caps);
   void renderCubicSubdivision(PointF *points, int level, int caps);
   // used for closed subpaths
   void calculateLastPoint(qreal rx1, qreal ry1, qreal rx2, qreal ry2);
   
};

QT_END_NAMESPACE

#endif // QCOSMETICLINE_H
