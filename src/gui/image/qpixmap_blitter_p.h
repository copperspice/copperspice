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

#ifndef QPIXMAP_BLITTER_P_H
#define QPIXMAP_BLITTER_P_H

#include <qplatform_pixmap.h>
#include <qpaintengine_blitter_p.h>

#ifndef QT_NO_BLITTABLE

class Q_GUI_EXPORT  QBlittablePlatformPixmap : public QPlatformPixmap
{
   //  Q_DECLARE_PRIVATE(QBlittablePixmapData);

 public:
   QBlittablePlatformPixmap();
   ~QBlittablePlatformPixmap();

   virtual QBlittable *createBlittable(const QSize &size, bool alpha) const = 0;
   QBlittable *blittable() const;
   void setBlittable(QBlittable *blittable);

   void resize(int width, int height) override;
   int metric(QPaintDevice::PaintDeviceMetric metric) const override;
   void fill(const QColor &color) override;
   QImage *buffer() override;
   QImage toImage() const override;
   bool hasAlphaChannel() const override;
   void fromImage(const QImage &image, Qt::ImageConversionFlags flags) override;
   qreal devicePixelRatio() const override;
   void setDevicePixelRatio(qreal scaleFactor) override;

   QPaintEngine *paintEngine() const override;

   void markRasterOverlay(const QRectF &);
   void markRasterOverlay(const QPointF &, const QTextItem &);
   void markRasterOverlay(const QVectorPath &);
   void markRasterOverlay(const QPainterPath &);
   void markRasterOverlay(const QRect *rects, int rectCount);
   void markRasterOverlay(const QRectF *rects, int rectCount);
   void markRasterOverlay(const QPointF *points, int pointCount);
   void markRasterOverlay(const QPoint *points, int pointCount);
   void unmarkRasterOverlay(const QRectF &);

#ifdef QT_BLITTER_RASTEROVERLAY
   void mergeOverlay();
   void unmergeOverlay();
   QImage *overlay();
#endif

 protected:
   QScopedPointer<QBlitterPaintEngine> m_engine;
   QScopedPointer<QBlittable> m_blittable;
   bool m_alpha;
   qreal m_devicePixelRatio;

#ifdef QT_BLITTER_RASTEROVERLAY
   QImage *m_rasterOverlay;
   QImage *m_unmergedCopy;
   QColor m_overlayColor;

   void markRasterOverlayImpl(const QRectF &);
   void unmarkRasterOverlayImpl(const QRectF &);
   QRectF clipAndTransformRect(const QRectF &) const;
#endif

};

inline void QBlittablePlatformPixmap::markRasterOverlay(const QRectF &rect)
{
#ifdef QT_BLITTER_RASTEROVERLAY
   markRasterOverlayImpl(rect);
#else
   (void) rect;
#endif
}

inline void QBlittablePlatformPixmap::markRasterOverlay(const QVectorPath &path)
{
#ifdef QT_BLITTER_RASTEROVERLAY
   markRasterOverlayImpl(path.convertToPainterPath().boundingRect());
#else
   (void) path;
#endif
}

inline void QBlittablePlatformPixmap::markRasterOverlay(const QPointF &pos, const QTextItem &ti)
{
#ifdef QT_BLITTER_RASTEROVERLAY
   QFontMetricsF fm(ti.font());
   QRectF rect = fm.tightBoundingRect(ti.text());
   rect.moveBottomLeft(pos);
   markRasterOverlay(rect);
#else
   (void) pos;
   (void) ti;
#endif
}

inline void QBlittablePlatformPixmap::markRasterOverlay(const QRect *rects, int rectCount)
{
#ifdef QT_BLITTER_RASTEROVERLAY
   for (int i = 0; i < rectCount; i++) {
      markRasterOverlay(rects[i]);
   }
#else
   (void) rects;
   (void) rectCount;
#endif
}

inline void QBlittablePlatformPixmap::markRasterOverlay(const QRectF *rects, int rectCount)
{
#ifdef QT_BLITTER_RASTEROVERLAY
   for (int i = 0; i < rectCount; i++) {
      markRasterOverlay(rects[i]);
   }
#else
   (void) rects;
   (void) rectCount;
#endif
}

inline void QBlittablePlatformPixmap::markRasterOverlay(const QPointF *points, int pointCount)
{
#ifdef QT_BLITTER_RASTEROVERLAY
#error "not ported yet"
#else
   (void) points;
   (void) pointCount;
#endif
}

inline void QBlittablePlatformPixmap::markRasterOverlay(const QPoint *points, int pointCount)
{
#ifdef QT_BLITTER_RASTEROVERLAY
#error "not ported yet"
#else
   (void) points;
   (void) pointCount;
#endif
}

inline void QBlittablePlatformPixmap::markRasterOverlay(const QPainterPath &path)
{
#ifdef QT_BLITTER_RASTEROVERLAY
#error "not ported yet"
#else
   (void) path;
#endif
}

inline void QBlittablePlatformPixmap::unmarkRasterOverlay(const QRectF &rect)
{
#ifdef QT_BLITTER_RASTEROVERLAY
   unmarkRasterOverlayImpl(rect);
#else
   (void) rect;
#endif
}

#endif // QT_NO_BLITTABLE
#endif // QPIXMAP_BLITTER_P_H
