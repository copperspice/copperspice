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

#ifndef QPAINTENGINE_BLITTER_P_H
#define QPAINTENGINE_BLITTER_P_H

#include <qpaintengine_raster_p.h>

#ifndef QT_NO_BLITTABLE


class QBlitterPaintEnginePrivate;
class QBlittablePlatformPixmap;
class QBlittable;

class Q_GUI_EXPORT QBlitterPaintEngine : public QRasterPaintEngine
{
   Q_DECLARE_PRIVATE(QBlitterPaintEngine)

 public:
   QBlitterPaintEngine(QBlittablePlatformPixmap *p);

   QPaintEngine::Type type() const  override {
      return Blitter;
   }

   bool begin(QPaintDevice *pdev) override;
   bool end() override;

   // Call down into QBlittable
   void fill(const QVectorPath &path, const QBrush &brush) override;
   void fillRect(const QRectF &rect, const QBrush &brush) override;
   void fillRect(const QRectF &rect, const QColor &color) override;

   void drawRects(const QRect *rects, int rectCount) override;
   void drawRects(const QRectF *rects, int rectCount) override;

   void drawPixmap(const QPointF &point, const QPixmap &pixmap) override;
   void drawPixmap(const QRectF &rect, const QPixmap &pixmap, const QRectF &srcRect) override;

   // State tracking
   void setState(QPainterState *s) override;
   void clipEnabledChanged() override;
   void penChanged() override;
   void brushChanged() override;
   void opacityChanged() override;
   void compositionModeChanged() override;
   void renderHintsChanged() override;
   void transformChanged() override;

   // Override to lock the QBlittable before using raster
   void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode) override;
   void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode) override;

   void fillPath(const QPainterPath &path, QSpanData *fillData);
   void fillPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
   void drawEllipse(const QRectF &rect) override;

   void drawImage(const QPointF &point, const QImage &image) override;
   void drawImage(const QRectF &rect, const QImage &pixmap, const QRectF &srcRect,
      Qt::ImageConversionFlags flags = Qt::AutoColor) override;

   void drawTiledPixmap(const QRectF &rect, const QPixmap &pixmap, const QPointF &srcPoint) override;
   void drawTextItem(const QPointF &point, const QTextItem &textItem) override;

   void drawPoints(const QPointF *points, int pointCount) override;
   void drawPoints(const QPoint *points, int pointCount) override;

   void stroke(const QVectorPath &path, const QPen &pen) override;
   void drawStaticTextItem(QStaticTextItem *) override;

   bool drawCachedGlyphs(int numGlyphs, const glyph_t *glyphs, const QFixedPoint *positions, QFontEngine *fontEngine) override;
};


#endif // QT_NO_BLITTABLE
#endif // QPAINTENGINE_BLITTER_P_H

