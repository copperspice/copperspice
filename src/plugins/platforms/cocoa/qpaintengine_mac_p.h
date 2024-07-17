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

#ifndef QPAINTENGINE_MAC_P_H
#define QPAINTENGINE_MAC_P_H

#include <qpaintengine.h>
#include <qhash.h>

#include <qpaintengine_p.h>
#include <qpolygonclipper_p.h>
#include <qfont_p.h>
#include <qt_mac_p.h>

typedef struct CGColorSpace *CGColorSpaceRef;

class QCoreGraphicsPaintEnginePrivate;

class QCoreGraphicsPaintEngine : public QPaintEngine
{
   Q_DECLARE_PRIVATE(QCoreGraphicsPaintEngine)

 public:
   QCoreGraphicsPaintEngine();

   QCoreGraphicsPaintEngine(const QCoreGraphicsPaintEngine &) = delete;
   QCoreGraphicsPaintEngine &operator=(const QCoreGraphicsPaintEngine &) = delete;

   ~QCoreGraphicsPaintEngine();

   bool begin(QPaintDevice *pdev);
   bool end();
   static CGColorSpaceRef macGenericColorSpace();
   static CGColorSpaceRef macDisplayColorSpace(const QWidget *widget = nullptr);

   void updateState(const QPaintEngineState &state);

   void updatePen(const QPen &pen);
   void updateBrush(const QBrush &brush, const QPointF &pt);
   void updateFont(const QFont &font);
   void updateOpacity(qreal opacity);
   void updateMatrix(const QTransform &matrix);
   void updateTransform(const QTransform &matrix);
   void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
   void updateClipPath(const QPainterPath &path, Qt::ClipOperation op);
   void updateCompositionMode(QPainter::CompositionMode mode);
   void updateRenderHints(QPainter::RenderHints hints);

   void drawLines(const QLineF *lines, int lineCount);
   void drawRects(const QRectF *rects, int rectCount);
   void drawPoints(const QPointF *p, int pointCount);
   void drawEllipse(const QRectF &r);
   void drawPath(const QPainterPath &path);

   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
   void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
   void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);

   void drawTextItem(const QPointF &pos, const QTextItem &item);
   void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
      Qt::ImageConversionFlags flags = Qt::AutoColor);

   Type type() const {
      return QPaintEngine::CoreGraphics;
   }

   CGContextRef handle() const;

   static void initialize();
   static void cleanup();

   QPainter::RenderHints supportedRenderHints() const;

   //avoid partial shadowed overload warnings...
   void drawLines(const QLine *lines, int lineCount) {
      QPaintEngine::drawLines(lines, lineCount);
   }
   void drawRects(const QRect *rects, int rectCount) {
      QPaintEngine::drawRects(rects, rectCount);
   }
   void drawPoints(const QPoint *p, int pointCount) {
      QPaintEngine::drawPoints(p, pointCount);
   }
   void drawEllipse(const QRect &r) {
      QPaintEngine::drawEllipse(r);
   }
   void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode) {
      QPaintEngine::drawPolygon(points, pointCount, mode);
   }

 protected:
   friend class QMacPrintEngine;
   friend class QMacPrintEnginePrivate;
   QCoreGraphicsPaintEngine(QPaintEnginePrivate &dptr);

 private:
   static bool m_postRoutineRegistered;
   static CGColorSpaceRef m_genericColorSpace;
   static QHash<CGDirectDisplayID, CGColorSpaceRef> m_displayColorSpaceHash;
   static void cleanUpMacColorSpaces();
};

class QCoreGraphicsPaintEnginePrivate : public QPaintEnginePrivate
{
   Q_DECLARE_PUBLIC(QCoreGraphicsPaintEngine)

 public:
   enum { CosmeticNone, CosmeticTransformPath, CosmeticSetPenWidth } cosmeticPen;

   static constexpr const int CGStroke = 0x01;
   static constexpr const int CGEOFill = 0x02;
   static constexpr const int CGFill   = 0x04;

   QCoreGraphicsPaintEnginePrivate()
      : hd(nullptr), shading(nullptr), stackCount(0), complexXForm(false), disabledSmoothFonts(false)
   { }

   struct {
      QPen pen;
      QBrush brush;
      uint clipEnabled : 1;
      QRegion clip;
      QTransform transform;
   } current;

   //state info (shared with QD)
   CGAffineTransform orig_xform;

   //cg structures
   CGContextRef hd;
   CGShadingRef shading;
   int stackCount;
   bool complexXForm;
   bool disabledSmoothFonts;

   // pixel and cosmetic pen size in user coordinates.
   QPointF pixelSize;
   float cosmeticPenSize;

   void drawPath(uchar ops, CGMutablePathRef path = nullptr);
   void setClip(const QRegion *rgn = nullptr);
   void resetClip();
   void setFillBrush(const QPointF &origin = QPoint());
   void setStrokePen(const QPen &pen);
   inline void saveGraphicsState();
   inline void restoreGraphicsState();
   float penOffset();
   QPointF devicePixelSize(CGContextRef context);
   float adjustPenWidth(float penWidth);

   void setTransform(const QTransform *matrix = nullptr) {
      CGContextConcatCTM(hd, CGAffineTransformInvert(CGContextGetCTM(hd)));
      CGAffineTransform xform = orig_xform;

      if (matrix) {
         extern CGAffineTransform qt_mac_convert_transform_to_cg(const QTransform &);
         xform = CGAffineTransformConcat(qt_mac_convert_transform_to_cg(*matrix), xform);
      }

      CGContextConcatCTM(hd, xform);
      CGContextSetTextMatrix(hd, xform);
   }
};

inline void QCoreGraphicsPaintEnginePrivate::saveGraphicsState()
{
   ++stackCount;
   CGContextSaveGState(hd);
}

inline void QCoreGraphicsPaintEnginePrivate::restoreGraphicsState()
{
   --stackCount;
   Q_ASSERT(stackCount >= 0);
   CGContextRestoreGState(hd);
}

#endif // QPAINTENGINE_MAC_P_H
