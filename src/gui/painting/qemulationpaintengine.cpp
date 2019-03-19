/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qemulationpaintengine_p.h>
#include <qpainter_p.h>
#include <qtextengine_p.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

QEmulationPaintEngine::QEmulationPaintEngine(QPaintEngineEx *engine)
   : real_engine(engine)
{
   QPaintEngine::state = real_engine->state();
}


QPaintEngine::Type QEmulationPaintEngine::type() const
{
   return real_engine->type();
}

bool QEmulationPaintEngine::begin(QPaintDevice *)
{
   return true;
}

bool QEmulationPaintEngine::end()
{
   return true;
}


QPainterState *QEmulationPaintEngine::createState(QPainterState *orig) const
{
   return real_engine->createState(orig);
}

void QEmulationPaintEngine::fill(const QVectorPath &path, const QBrush &brush)
{
   QPainterState *s = state();

   if (s->bgMode == Qt::OpaqueMode) {
      Qt::BrushStyle style = brush.style();
      if (style >= Qt::Dense1Pattern && style <= Qt::DiagCrossPattern) {
         real_engine->fill(path, s->bgBrush);
      }
   }

   Qt::BrushStyle style = qbrush_style(brush);
   if (style >= Qt::LinearGradientPattern && style <= Qt::ConicalGradientPattern) {
      const QGradient *g = brush.gradient();

      if (g->coordinateMode() > QGradient::LogicalMode) {
         if (g->coordinateMode() == QGradient::StretchToDeviceMode) {
            QBrush copy = brush;
            QTransform mat = copy.transform();
            mat.scale(real_engine->painter()->device()->width(), real_engine->painter()->device()->height());
            copy.setTransform(mat);
            real_engine->fill(path, copy);
            return;
         } else if (g->coordinateMode() == QGradient::ObjectBoundingMode) {
            QBrush copy = brush;
            QTransform mat = copy.transform();
            QRectF r = path.controlPointRect();
            mat.translate(r.x(), r.y());
            mat.scale(r.width(), r.height());
            copy.setTransform(mat);
            real_engine->fill(path, copy);
            return;
         }
      }
   }

   real_engine->fill(path, brush);
}

void QEmulationPaintEngine::stroke(const QVectorPath &path, const QPen &pen)
{
   QPainterState *s = state();

   if (s->bgMode == Qt::OpaqueMode && pen.style() > Qt::SolidLine) {
      QPen bgPen = pen;
      bgPen.setBrush(s->bgBrush);
      bgPen.setStyle(Qt::SolidLine);
      real_engine->stroke(path, bgPen);
   }

   QBrush brush = pen.brush();
   QPen copy = pen;
   Qt::BrushStyle style = qbrush_style(brush);
   if (style >= Qt::LinearGradientPattern && style <= Qt::ConicalGradientPattern) {
      const QGradient *g = brush.gradient();

      if (g->coordinateMode() > QGradient::LogicalMode) {
         if (g->coordinateMode() == QGradient::StretchToDeviceMode) {
            QTransform mat = brush.transform();
            mat.scale(real_engine->painter()->device()->width(), real_engine->painter()->device()->height());
            brush.setTransform(mat);
            copy.setBrush(brush);
            real_engine->stroke(path, copy);
            return;
         } else if (g->coordinateMode() == QGradient::ObjectBoundingMode) {
            QTransform mat = brush.transform();
            QRectF r = path.controlPointRect();
            mat.translate(r.x(), r.y());
            mat.scale(r.width(), r.height());
            brush.setTransform(mat);
            copy.setBrush(brush);
            real_engine->stroke(path, copy);
            return;
         }
      }
   }

   real_engine->stroke(path, pen);
}

void QEmulationPaintEngine::clip(const QVectorPath &path, Qt::ClipOperation op)
{
   real_engine->clip(path, op);
}

void QEmulationPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
   if (state()->bgMode == Qt::OpaqueMode && pm.isQBitmap()) {
      fillBGRect(r);
   }
   real_engine->drawPixmap(r, pm, sr);
}

void QEmulationPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
   if (state()->bgMode == Qt::OpaqueMode) {
      const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
      QRectF rect(p.x(), p.y() - ti.ascent.toReal(), ti.width.toReal(), (ti.ascent + ti.descent + 1).toReal());
      fillBGRect(rect);
   }

   QPainterState *s = state();
   Qt::BrushStyle style = qbrush_style(s->pen.brush());
   if (style >= Qt::LinearGradientPattern && style <= Qt::ConicalGradientPattern) {
      QPen savedPen = s->pen;
      QGradient g = *s->pen.brush().gradient();

      if (g.coordinateMode() > QGradient::LogicalMode) {
         QTransform mat = s->pen.brush().transform();
         if (g.coordinateMode() == QGradient::StretchToDeviceMode) {
            mat.scale(real_engine->painter()->device()->width(), real_engine->painter()->device()->height());
         } else if (g.coordinateMode() == QGradient::ObjectBoundingMode) {
            const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
            QRectF r(p.x(), p.y() - ti.ascent.toReal(), ti.width.toReal(), (ti.ascent + ti.descent + 1).toReal());
            mat.translate(r.x(), r.y());
            mat.scale(r.width(), r.height());
         }
         g.setCoordinateMode(QGradient::LogicalMode);
         QBrush brush(g);
         brush.setTransform(mat);
         s->pen.setBrush(brush);
         penChanged();
         real_engine->drawTextItem(p, textItem);
         s->pen = savedPen;
         penChanged();
         return;
      }
   }

   real_engine->drawTextItem(p, textItem);
}

void QEmulationPaintEngine::drawStaticTextItem(QStaticTextItem *item)
{
   real_engine->drawStaticTextItem(item);
}

void QEmulationPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s)
{
   if (state()->bgMode == Qt::OpaqueMode && pixmap.isQBitmap()) {
      fillBGRect(r);
   }
   real_engine->drawTiledPixmap(r, pixmap, s);
}

void QEmulationPaintEngine::drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                                      Qt::ImageConversionFlags flags)
{
   real_engine->drawImage(r, pm, sr, flags);
}

void QEmulationPaintEngine::drawPixmapFragments(const QPainter::PixmapFragment *fragments, int fragmentCount,
      const QPixmap &pixmap,
      QPainter::PixmapFragmentHints hints)
{
   if (state()->bgMode == Qt::OpaqueMode && pixmap.isQBitmap()) {
      qreal oldOpacity = real_engine->state()->opacity;
      QTransform oldTransform = real_engine->state()->matrix;

      for (int i = 0; i < fragmentCount; ++i) {
         QTransform transform = oldTransform;
         transform.translate(fragments[i].x, fragments[i].y);
         transform.rotate(fragments[i].rotation);
         real_engine->state()->opacity = oldOpacity * fragments[i].opacity;
         real_engine->state()->matrix = transform;
         real_engine->opacityChanged();
         real_engine->transformChanged();

         qreal w = fragments[i].scaleX * fragments[i].width;
         qreal h = fragments[i].scaleY * fragments[i].height;
         fillBGRect(QRectF(-0.5 * w, -0.5 * h, w, h));
      }

      real_engine->state()->opacity = oldOpacity;
      real_engine->state()->matrix = oldTransform;
      real_engine->opacityChanged();
      real_engine->transformChanged();
   }

   real_engine->drawPixmapFragments(fragments, fragmentCount, pixmap, hints);
}

void QEmulationPaintEngine::drawPixmapFragments(const QRectF *targetRects, const QRectF *sourceRects, int fragmentCount,
      const QPixmap &pixmap,
      QPainter::PixmapFragmentHints hints)
{
   if (state()->bgMode == Qt::OpaqueMode && pixmap.isQBitmap()) {
      for (int i = 0; i < fragmentCount; ++i) {
         fillBGRect(targetRects[i]);
      }
   }

   real_engine->drawPixmapFragments(targetRects, sourceRects, fragmentCount, pixmap, hints);
}

void QEmulationPaintEngine::clipEnabledChanged()
{
   real_engine->clipEnabledChanged();
}

void QEmulationPaintEngine::penChanged()
{
   real_engine->penChanged();
}

void QEmulationPaintEngine::brushChanged()
{
   real_engine->brushChanged();
}

void QEmulationPaintEngine::brushOriginChanged()
{
   real_engine->brushOriginChanged();
}

void QEmulationPaintEngine::opacityChanged()
{
   real_engine->opacityChanged();
}

void QEmulationPaintEngine::compositionModeChanged()
{
   real_engine->compositionModeChanged();
}

void QEmulationPaintEngine::renderHintsChanged()
{
   real_engine->renderHintsChanged();
}

void QEmulationPaintEngine::transformChanged()
{
   real_engine->transformChanged();
}

void QEmulationPaintEngine::setState(QPainterState *s)
{
   QPaintEngine::state = s;
   real_engine->setState(s);
}

void QEmulationPaintEngine::beginNativePainting()
{
   real_engine->beginNativePainting();
}

void QEmulationPaintEngine::endNativePainting()
{
   real_engine->endNativePainting();
}

void QEmulationPaintEngine::fillBGRect(const QRectF &r)
{
   qreal pts[] = { r.x(), r.y(), r.x() + r.width(), r.y(),
                   r.x() + r.width(), r.y() + r.height(), r.x(), r.y() + r.height()
                 };
   QVectorPath vp(pts, 4, 0, QVectorPath::RectangleHint);
   real_engine->fill(vp, state()->bgBrush);
}

QT_END_NAMESPACE
