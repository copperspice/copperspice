/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qglobal.h>

#ifndef QT_NO_PRINTER

#include <qdebug.h>
#include <qpaintengine_alpha_p.h>
#include <qpainter_p.h>
#include <qpicture_p.h>
#include <qfont_p.h>
#include <qpicture.h>

QAlphaPaintEngine::QAlphaPaintEngine(QAlphaPaintEnginePrivate &data, PaintEngineFeatures devcaps)
   : QPaintEngine(data, devcaps)
{
}

QAlphaPaintEngine::~QAlphaPaintEngine()
{
}

bool QAlphaPaintEngine::begin(QPaintDevice *pdev)
{
   Q_D(QAlphaPaintEngine);

   d->m_continueCall = true;
   if (d->m_pass != 0) {
      return true;
   }

   d->m_savedcaps = gccaps;
   d->m_pdev = pdev;

   d->m_alphaPen = false;
   d->m_alphaBrush = false;
   d->m_alphaOpacity = false;
   d->m_hasalpha = false;
   d->m_advancedPen = false;
   d->m_advancedBrush = false;
   d->m_complexTransform = false;
   d->m_emulateProjectiveTransforms = false;

   // clear alpha region
   d->m_alphargn = QRegion();
   d->m_cliprgn = QRegion();
   d->m_pen = QPen();
   d->m_transform = QTransform();

   flushAndInit();

   return true;
}

bool QAlphaPaintEngine::end()
{
   Q_D(QAlphaPaintEngine);

   d->m_continueCall = true;
   if (d->m_pass != 0) {
      return true;
   }

   flushAndInit(false);
   return true;
}

void QAlphaPaintEngine::updateState(const QPaintEngineState &state)
{
   Q_D(QAlphaPaintEngine);

   DirtyFlags flags = state.state();
   if (flags & QPaintEngine::DirtyTransform) {
      d->m_transform = state.transform();
      d->m_complexTransform = (d->m_transform.type() > QTransform::TxScale);
      d->m_emulateProjectiveTransforms = !(d->m_savedcaps & QPaintEngine::PerspectiveTransform)
         && !(d->m_savedcaps & QPaintEngine::AlphaBlend)
         && (d->m_transform.type() >= QTransform::TxProject);
   }
   if (flags & QPaintEngine::DirtyPen) {
      d->m_pen = state.pen();
      if (d->m_pen.style() == Qt::NoPen) {
         d->m_advancedPen = false;
         d->m_alphaPen = false;
      } else {
         d->m_advancedPen = (d->m_pen.brush().style() != Qt::SolidPattern);
         d->m_alphaPen = !d->m_pen.brush().isOpaque();
      }
   }

   if (d->m_pass != 0) {
      d->m_continueCall = true;
      return;
   }
   d->m_continueCall = false;

   if (flags & QPaintEngine::DirtyOpacity) {
      d->m_alphaOpacity = (state.opacity() != 1.0f);
   }

   if (flags & QPaintEngine::DirtyBrush) {
      if (state.brush().style() == Qt::NoBrush) {
         d->m_advancedBrush = false;
         d->m_alphaBrush = false;
      } else {
         d->m_advancedBrush = (state.brush().style() != Qt::SolidPattern);
         d->m_alphaBrush = !state.brush().isOpaque();
      }
   }


   d->m_hasalpha = d->m_alphaOpacity || d->m_alphaBrush || d->m_alphaPen;

   if (d->m_picengine) {
      const QPainter *p = painter();
      d->m_picpainter->setPen(p->pen());
      d->m_picpainter->setBrush(p->brush());
      d->m_picpainter->setBrushOrigin(p->brushOrigin());
      d->m_picpainter->setFont(p->font());
      d->m_picpainter->setOpacity(p->opacity());
      d->m_picpainter->setTransform(p->combinedTransform());
      d->m_picengine->updateState(state);
   }
}

void QAlphaPaintEngine::drawPath(const QPainterPath &path)
{
   Q_D(QAlphaPaintEngine);

   QRectF tr = d->addPenWidth(path);

   if (d->m_pass == 0) {
      d->m_continueCall = false;
      if (d->canSeeTroughBackground(d->m_hasalpha, tr) || d->m_advancedPen || d->m_advancedBrush
         || d->m_emulateProjectiveTransforms) {
         d->addAlphaRect(tr);
      }

      d->addDirtyRect(tr);

      if (d->m_picengine) {
         d->m_picengine->drawPath(path);
      }
   } else {
      d->m_continueCall = !d->fullyContained(tr);
   }
}

void QAlphaPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
   Q_D(QAlphaPaintEngine);

   QPolygonF poly;
   for (int i = 0; i < pointCount; ++i) {
      poly.append(points[i]);
   }

   QPainterPath path;
   path.addPolygon(poly);
   QRectF tr = d->addPenWidth(path);

   if (d->m_pass == 0) {
      d->m_continueCall = false;

      if (d->canSeeTroughBackground(d->m_hasalpha, tr) || d->m_advancedPen || d->m_advancedBrush
         || d->m_emulateProjectiveTransforms) {
         d->addAlphaRect(tr);
      }

      d->addDirtyRect(tr);

      if (d->m_picengine) {
         d->m_picengine->drawPolygon(points, pointCount, mode);
      }

   } else {
      d->m_continueCall = !d->fullyContained(tr);
   }
}

void QAlphaPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
   Q_D(QAlphaPaintEngine);

   QRectF tr = d->m_transform.mapRect(r);
   if (d->m_pass == 0) {
      d->m_continueCall = false;
      if (d->canSeeTroughBackground(pm.hasAlpha() || d->m_alphaOpacity, tr) || d->m_complexTransform || pm.isQBitmap()) {
         d->addAlphaRect(tr);
      }

      d->addDirtyRect(tr);

      if (d->m_picengine) {
         d->m_picengine->drawPixmap(r, pm, sr);
      }

   } else {
      d->m_continueCall = !d->fullyContained(tr);
   }
}

void QAlphaPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
   Q_D(QAlphaPaintEngine);

   QRectF tr(p.x(), p.y() - textItem.ascent(), textItem.width() + 5, textItem.ascent() + textItem.descent() + 5);
   tr = d->m_transform.mapRect(tr);

   if (d->m_pass == 0) {
      d->m_continueCall = false;

      if (d->canSeeTroughBackground(d->m_alphaPen || d->m_alphaOpacity, tr) || d->m_advancedPen) {
         d->addAlphaRect(tr);
      }
      d->addDirtyRect(tr);
      if (d->m_picengine) {
         d->m_picengine->drawTextItem(p, textItem);
      }

   } else {
      d->m_continueCall = !d->fullyContained(tr);
   }
}

void QAlphaPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s)
{
   Q_D(QAlphaPaintEngine);

   QRectF brect = d->m_transform.mapRect(r);

   if (d->m_pass == 0) {
      d->m_continueCall = false;
      if (d->canSeeTroughBackground(pixmap.hasAlpha() || d->m_alphaOpacity, brect) || d->m_complexTransform || pixmap.isQBitmap()) {
         d->addAlphaRect(brect);
      }

      d->addDirtyRect(brect);

      if (d->m_picengine) {
         d->m_picengine->drawTiledPixmap(r, pixmap, s);
      }
   } else {
      d->m_continueCall = !d->fullyContained(brect);
   }
}

QRegion QAlphaPaintEngine::alphaClipping() const
{
   Q_D(const QAlphaPaintEngine);
   return d->m_cliprgn;
}

bool QAlphaPaintEngine::continueCall() const
{
   Q_D(const QAlphaPaintEngine);
   return d->m_continueCall;
}

void QAlphaPaintEngine::flushAndInit(bool init)
{
   Q_D(QAlphaPaintEngine);
   Q_ASSERT(d->m_pass == 0);

   if (d->m_pic) {
      d->m_picpainter->end();

      // set clip region
      d->m_alphargn = d->m_alphargn.intersected(QRect(0, 0, d->m_pdev->width(), d->m_pdev->height()));

      // just use the bounding rect if it's a complex region..
      QVector<QRect> rects = d->m_alphargn.rects();
      if (rects.size() > 10) {
         QRect br = d->m_alphargn.boundingRect();
         d->m_alphargn = QRegion(br);
         rects.clear();
         rects.append(br);
      }

      d->m_cliprgn = d->m_alphargn;

      // now replay the QPicture
      ++d->m_pass; // we are now doing pass #2

      // reset states
      gccaps = d->m_savedcaps;

      painter()->save();
      d->resetState(painter());

      // make sure the output from QPicture is unscaled
      QTransform mtx;
      mtx.scale(1.0f / (qreal(d->m_pdev->logicalDpiX()) / qreal(qt_defaultDpiX())),
         1.0f / (qreal(d->m_pdev->logicalDpiY()) / qreal(qt_defaultDpiY())));
      painter()->setTransform(mtx);
      painter()->drawPicture(0, 0, *d->m_pic);

      d->m_cliprgn = QRegion();
      d->resetState(painter());

      // fill in the alpha images
      for (int i = 0; i < rects.size(); ++i) {
         d->drawAlphaImage(rects.at(i));
      }

      d->m_alphargn = QRegion();

      painter()->restore();

      --d->m_pass; // pass #2 finished

      cleanUp();
   }

   if (init) {
      gccaps = PaintEngineFeatures(AllFeatures & ~QPaintEngine::ObjectBoundingModeGradients);

      d->m_pic = new QPicture();
      d->m_pic->d_ptr->in_memory_only = true;
      d->m_picpainter = new QPainter(d->m_pic);
      d->m_picengine = d->m_picpainter->paintEngine();

      // When newPage() is called and the m_picpainter is recreated
      // we have to copy the current state of the original printer
      // painter back to the m_picpainter
      d->m_picpainter->setPen(painter()->pen());
      d->m_picpainter->setBrush(painter()->brush());
      d->m_picpainter->setBrushOrigin(painter()->brushOrigin());
      d->m_picpainter->setFont(painter()->font());
      d->m_picpainter->setOpacity(painter()->opacity());
      d->m_picpainter->setTransform(painter()->combinedTransform());
      d->m_picengine->syncState();

      QPainterState &state = *d->m_picpainter->d_func()->state;
      QPainter *oldPainter = state.painter;
      state = *painter()->d_func()->state;
      state.painter = oldPainter;
   }
}

void QAlphaPaintEngine::cleanUp()
{
   Q_D(QAlphaPaintEngine);

   delete d->m_picpainter;
   delete d->m_pic;

   d->m_picpainter = 0;
   d->m_pic = 0;
   d->m_picengine = 0;
}

QAlphaPaintEnginePrivate::QAlphaPaintEnginePrivate()
   :   m_pass(0),
       m_pic(0),
       m_picengine(0),
       m_picpainter(0),
       m_numberOfCachedRects(0),
       m_hasalpha(false),
       m_alphaPen(false),
       m_alphaBrush(false),
       m_alphaOpacity(false),
       m_advancedPen(false),
       m_advancedBrush(false),
       m_complexTransform(false)
{

}

QAlphaPaintEnginePrivate::~QAlphaPaintEnginePrivate()
{
   delete m_picpainter;
   delete m_pic;
}

QRectF QAlphaPaintEnginePrivate::addPenWidth(const QPainterPath &path)
{
   Q_Q(QAlphaPaintEngine);
   QPainterPath tmp = path;

   if (m_pen.style() == Qt::NoPen) {
      return (path.controlPointRect() * m_transform).boundingRect();
   }

   bool cosmetic = qt_pen_is_cosmetic(m_pen, q->state->renderHints());

   if (cosmetic) {
      tmp = path * m_transform;
   }

   QPainterPathStroker stroker;
   if (m_pen.widthF() == 0.0f) {
      stroker.setWidth(1.0);
   } else {
      stroker.setWidth(m_pen.widthF());
   }

   stroker.setJoinStyle(m_pen.joinStyle());
   stroker.setCapStyle(m_pen.capStyle());
   tmp = stroker.createStroke(tmp);

   if (cosmetic) {
      return tmp.controlPointRect();
   }

   return (tmp.controlPointRect() * m_transform).boundingRect();
}

void QAlphaPaintEnginePrivate::addAlphaRect(const QRectF &rect)
{
   m_alphargn |= rect.toAlignedRect();
}
bool QAlphaPaintEnginePrivate::canSeeTroughBackground(bool somethingInRectHasAlpha, const QRectF &rect) const
{
   if (somethingInRectHasAlpha) {
      if (m_dirtyRects.count() != m_numberOfCachedRects) {
         m_cachedDirtyRgn.setRects(m_dirtyRects.constData(), m_dirtyRects.count());
         m_numberOfCachedRects = m_dirtyRects.count();
      }
      return m_cachedDirtyRgn.intersects(rect.toAlignedRect());
   }
   return false;
}

void QAlphaPaintEnginePrivate::drawAlphaImage(const QRectF &rect)
{
   Q_Q(QAlphaPaintEngine);

   qreal dpiX = qMax(m_pdev->logicalDpiX(), 300);
   qreal dpiY = qMax(m_pdev->logicalDpiY(), 300);
   qreal xscale = (dpiX / m_pdev->logicalDpiX());
   qreal yscale = (dpiY / m_pdev->logicalDpiY());

   QTransform picscale;
   picscale.scale(xscale, yscale);

   const int tileSize = 2048;
   QSize size((int(rect.width() * xscale)), int(rect.height() * yscale));
   int divw = (size.width() / tileSize);
   int divh = (size.height() / tileSize);
   divw += 1;
   divh += 1;

   int incx = int(rect.width() / divw);
   int incy = int(rect.height() / divh);

   for (int y = 0; y < divh; ++y) {
      int ypos = int((incy * y) + rect.y());
      int height = int((y == (divh - 1)) ? (rect.height() - (incy * y)) : incy) + 1;

      for (int x = 0; x < divw; ++x) {
         int xpos = int((incx * x) + rect.x());
         int width = int((x == (divw - 1)) ? (rect.width() - (incx * x)) : incx) + 1;

         QSize imgsize((int)(width * xscale), (int)(height * yscale));
         QImage img(imgsize, QImage::Format_RGB32);
         img.fill(0xffffffff);

         QPainter imgpainter(&img);
         imgpainter.setTransform(picscale);
         QPointF picpos(qreal(-xpos), qreal(-ypos));
         imgpainter.drawPicture(picpos, *m_pic);
         imgpainter.end();

         q->painter()->setTransform(QTransform());
         QRect r(xpos, ypos, width, height);
         q->painter()->drawImage(r, img);
      }
   }
}

bool QAlphaPaintEnginePrivate::fullyContained(const QRectF &rect) const
{
   QRegion r(rect.toAlignedRect());
   return (m_cliprgn.intersected(r) == r);
}

void QAlphaPaintEnginePrivate::resetState(QPainter *p)
{
   p->setPen(QPen());
   p->setBrush(QBrush());
   p->setBrushOrigin(0, 0);
   p->setBackground(QBrush());
   p->setFont(QFont());
   p->setTransform(QTransform());
   // The view transform is already recorded and included in the
   // picture we're about to replay. If we don't turn if off,
   // the view matrix will be applied twice.
   p->setViewTransformEnabled(false);
   p->setClipRegion(QRegion(), Qt::NoClip);
   p->setClipPath(QPainterPath(), Qt::NoClip);
   p->setClipping(false);
   p->setOpacity(1.0f);
}

#endif // QT_NO_PRINTER
