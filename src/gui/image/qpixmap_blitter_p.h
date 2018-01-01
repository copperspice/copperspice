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

#ifndef QPIXMAP_BLITTER_P_H
#define QPIXMAP_BLITTER_P_H

#include <qpixmapdata_p.h>
#include <qpaintengine_blitter_p.h>

#ifndef QT_NO_BLITTABLE
QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT  QBlittablePixmapData : public QPixmapData
{
   //     Q_DECLARE_PRIVATE(QBlittablePixmapData);

 public:
   QBlittablePixmapData();
   ~QBlittablePixmapData();

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

#ifdef QT_BLITTER_RASTEROVERLAY
   QImage *m_rasterOverlay;
   QImage *m_unmergedCopy;
   QColor m_overlayColor;

   void markRasterOverlayImpl(const QRectF &);
   void unmarkRasterOverlayImpl(const QRectF &);
   QRectF clipAndTransformRect(const QRectF &) const;
#endif 

};

inline void QBlittablePixmapData::markRasterOverlay(const QRectF &rect)
{
#ifdef QT_BLITTER_RASTEROVERLAY
   markRasterOverlayImpl(rect);
#else
   Q_UNUSED(rect)
#endif
}

inline void QBlittablePixmapData::markRasterOverlay(const QVectorPath &path)
{
#ifdef QT_BLITTER_RASTEROVERLAY
   markRasterOverlayImpl(path.convertToPainterPath().boundingRect());
#else
   Q_UNUSED(path)
#endif
}

inline void QBlittablePixmapData::markRasterOverlay(const QPointF &pos, const QTextItem &ti)
{
#ifdef QT_BLITTER_RASTEROVERLAY
   QFontMetricsF fm(ti.font());
   QRectF rect = fm.tightBoundingRect(ti.text());
   rect.moveBottomLeft(pos);
   markRasterOverlay(rect);
#else
   Q_UNUSED(pos)
   Q_UNUSED(ti)
#endif
}

inline void QBlittablePixmapData::markRasterOverlay(const QRect *rects, int rectCount)
{
#ifdef QT_BLITTER_RASTEROVERLAY
   for (int i = 0; i < rectCount; i++) {
      markRasterOverlay(rects[i]);
   }
#else
   Q_UNUSED(rects)
   Q_UNUSED(rectCount)
#endif
}
inline void QBlittablePixmapData::markRasterOverlay(const QRectF *rects, int rectCount)
{
#ifdef QT_BLITTER_RASTEROVERLAY
   for (int i = 0; i < rectCount; i++) {
      markRasterOverlay(rects[i]);
   }
#else
   Q_UNUSED(rects)
   Q_UNUSED(rectCount)
#endif
}

inline void QBlittablePixmapData::markRasterOverlay(const QPointF *points, int pointCount)
{
#ifdef QT_BLITTER_RASTEROVERLAY
#error "not ported yet"
#else
   Q_UNUSED(points);
   Q_UNUSED(pointCount);
#endif
}

inline void QBlittablePixmapData::markRasterOverlay(const QPoint *points, int pointCount)
{
#ifdef QT_BLITTER_RASTEROVERLAY
#error "not ported yet"
#else
   Q_UNUSED(points);
   Q_UNUSED(pointCount);
#endif
}

inline void QBlittablePixmapData::markRasterOverlay(const QPainterPath &path)
{
#ifdef QT_BLITTER_RASTEROVERLAY
#error "not ported yet"
#else
   Q_UNUSED(path);
#endif
}

inline void QBlittablePixmapData::unmarkRasterOverlay(const QRectF &rect)
{
#ifdef QT_BLITTER_RASTEROVERLAY
   unmarkRasterOverlayImpl(rect);
#else
   Q_UNUSED(rect)
#endif
}

QT_END_NAMESPACE
#endif // QT_NO_BLITTABLE
#endif // QPIXMAP_BLITTER_P_H
