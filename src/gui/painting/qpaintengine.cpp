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

#include <qpaintengine.h>
#include <qpaintengine_p.h>
#include <qpainter_p.h>
#include <qpolygon.h>
#include <qbitmap.h>

#include <qdebug.h>
#include <qmath.h>
#include <qguiapplication.h>

#include <qtextengine_p.h>
#include <qvarlengtharray.h>
#include <qfontengine_p.h>
#include <qpaintengineex_p.h>

qreal QTextItem::descent() const
{
   const QTextItemInt *ti = static_cast<const QTextItemInt *>(this);
   return ti->descent.toReal();
}

qreal QTextItem::ascent() const
{
   const QTextItemInt *ti = static_cast<const QTextItemInt *>(this);
   return ti->ascent.toReal();
}

qreal QTextItem::width() const
{
   const QTextItemInt *ti = static_cast<const QTextItemInt *>(this);
   return ti->width.toReal();
}

QTextItem::RenderFlags QTextItem::renderFlags() const
{
   const QTextItemInt *ti = static_cast<const QTextItemInt *>(this);
   return ti->flags;
}

QString QTextItem::text() const
{
   const QTextItemInt *ti = static_cast<const QTextItemInt *>(this);
   return QString(ti->m_iter, ti->m_end);
}

QFont QTextItem::font() const
{
   const QTextItemInt *ti = static_cast<const QTextItemInt *>(this);
   return ti->f ? *ti->f : QGuiApplication::font();
}

void QPaintEngine::syncState()
{
   Q_ASSERT(state);
   updateState(*state);

   if (isExtended()) {
      static_cast<QPaintEngineEx *>(this)->sync();
   }
}

static QPaintEngine *qt_polygon_recursion = nullptr;

struct QT_Point {
   int x;
   int y;
};

void QPaintEngine::drawPolygon(const QPointF *pointPtr, int pointCount, PolygonDrawMode mode)
{
   Q_ASSERT_X(qt_polygon_recursion != this, "QPaintEngine::drawPolygon",
      "At least one drawPolygon function must be implemented");

   qt_polygon_recursion = this;
   Q_ASSERT(sizeof(QT_Point) == sizeof(QPoint));
   QVarLengthArray<QT_Point> p(pointCount);

   for (int i = 0; i < pointCount; ++i) {
      p[i].x = qRound(pointPtr[i].x());
      p[i].y = qRound(pointPtr[i].y());
   }

   drawPolygon((QPoint *)p.data(), pointCount, mode);
   qt_polygon_recursion = nullptr;
}

struct QT_PointF {
   qreal x;
   qreal y;
};

void QPaintEngine::drawPolygon(const QPoint *pointPtr, int pointCount, PolygonDrawMode mode)
{
   Q_ASSERT_X(qt_polygon_recursion != this, "QPaintEngine::drawPolygon",
      "At least one drawPolygon function must be implemented");

   qt_polygon_recursion = this;

   Q_ASSERT(sizeof(QT_PointF) == sizeof(QPointF));
   QVarLengthArray<QT_PointF> p(pointCount);

   for (int i = 0; i < pointCount; ++i) {
      p[i].x = pointPtr[i].x();
      p[i].y = pointPtr[i].y();
   }

   drawPolygon((QPointF *)p.data(), pointCount, mode);
   qt_polygon_recursion = nullptr;
}

void QPaintEngine::drawPoints(const QPointF *pointPtr, int pointCount)
{
   QPainter *p = painter();

   if (! p) {
      return;
   }

   qreal penWidth = p->pen().widthF();
   if (penWidth == 0) {
      penWidth = 1;
   }

   bool ellipses = p->pen().capStyle() == Qt::RoundCap;

   p->save();

   QTransform transform;

   if (qt_pen_is_cosmetic(p->pen(), p->renderHints())) {
      transform = p->transform();
      p->setTransform(QTransform());
   }

   p->setBrush(p->pen().brush());
   p->setPen(Qt::NoPen);

   for (int i = 0; i < pointCount; ++i) {
      QPointF pos = transform.map(pointPtr[i]);
      QRectF rect(pos.x() - penWidth / 2, pos.y() - penWidth / 2, penWidth, penWidth);

      if (ellipses) {
         p->drawEllipse(rect);
      } else {
         p->drawRect(rect);
      }
   }

   p->restore();
}

void QPaintEngine::drawPoints(const QPoint *pointPtr, int pointCount)
{
   Q_ASSERT(sizeof(QT_PointF) == sizeof(QPointF));
   QT_PointF fp[256];

   while (pointCount) {
      int i = 0;

      while (i < pointCount && i < 256) {
         fp[i].x = pointPtr[i].x();
         fp[i].y = pointPtr[i].y();
         ++i;
      }

      drawPoints((QPointF *)(void *)fp, i);
      pointPtr   += i;
      pointCount -= i;
   }
}

void QPaintEngine::drawEllipse(const QRectF &rect)
{
   QPainterPath path;
   path.addEllipse(rect);

   if (hasFeature(PainterPaths)) {
      drawPath(path);
   } else {
      QPolygonF polygon = path.toFillPolygon();
      drawPolygon(polygon.data(), polygon.size(), ConvexMode);
   }
}

void QPaintEngine::drawEllipse(const QRect &rect)
{
   drawEllipse(QRectF(rect));
}

void qt_fill_tile(QPixmap *tile, const QPixmap &pixmap)
{
   QPainter p(tile);
   p.drawPixmap(0, 0, pixmap);
   int x = pixmap.width();

   while (x < tile->width()) {
      p.drawPixmap(x, 0, *tile, 0, 0, x, pixmap.height());
      x *= 2;
   }

   int y = pixmap.height();
   while (y < tile->height()) {
      p.drawPixmap(0, y, *tile, 0, 0, tile->width(), y);
      y *= 2;
   }
}

void qt_draw_tile(QPaintEngine *gc, qreal x, qreal y, qreal w, qreal h,
   const QPixmap &pixmap, qreal xOffset, qreal yOffset)
{
   qreal yPos, xPos, drawH, drawW, yOff, xOff;
   yPos = y;
   yOff = yOffset;

   while (yPos < y + h) {
      drawH = pixmap.height() - yOff;        // Cropping first row
      if (yPos + drawH > y + h) {            // Cropping last row
         drawH = y + h - yPos;
      }

      xPos = x;
      xOff = xOffset;

      while (xPos < x + w) {
         drawW = pixmap.width() - xOff;      // Cropping first column
         if (xPos + drawW > x + w) {         // Cropping last column
            drawW = x + w - xPos;
         }
         if (drawW > 0 && drawH > 0) {
            gc->drawPixmap(QRectF(xPos, yPos, drawW, drawH), pixmap, QRectF(xOff, yOff, drawW, drawH));
         }
         xPos += drawW;
         xOff = 0;
      }
      yPos += drawH;
      yOff = 0;
   }
}

void QPaintEngine::drawTiledPixmap(const QRectF &rect, const QPixmap &pixmap, const QPointF &p)
{
   int sw = pixmap.width();
   int sh = pixmap.height();

   if (sw * sh < 8192 && sw * sh < 16 * rect.width()*rect.height()) {
      int tw = sw, th = sh;
      while (tw * th < 32678 && tw < rect.width() / 2) {
         tw *= 2;
      }
      while (tw * th < 32678 && th < rect.height() / 2) {
         th *= 2;
      }
      QPixmap tile;
      if (pixmap.depth() == 1) {
         tile = QBitmap(tw, th);
      } else {
         tile = QPixmap(tw, th);
         if (pixmap.hasAlphaChannel()) {
            tile.fill(Qt::transparent);
         }
      }

      qt_fill_tile(&tile, pixmap);
      qt_draw_tile(this, rect.x(), rect.y(), rect.width(), rect.height(), tile, p.x(), p.y());

   } else {
      qt_draw_tile(this, rect.x(), rect.y(), rect.width(), rect.height(), pixmap, p.x(), p.y());
   }
}

void QPaintEngine::drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
   Qt::ImageConversionFlags flags)
{
   QRectF baseSize(0, 0, image.width(), image.height());
   QImage im = image;

   if (baseSize != sr) {
      im = im.copy(qFloor(sr.x()), qFloor(sr.y()), qCeil(sr.width()), qCeil(sr.height()));
   }

   QPixmap pm = QPixmap::fromImage(im, flags);
   drawPixmap(r, pm, QRectF(QPointF(0, 0), pm.size()));
}

QPaintEngine::QPaintEngine(PaintEngineFeatures caps)
   : state(nullptr), gccaps(caps), active(0), selfDestruct(false), extended(false),
     d_ptr(new QPaintEnginePrivate)
{
   d_ptr->q_ptr = this;
}

// internal
QPaintEngine::QPaintEngine(QPaintEnginePrivate &dptr, PaintEngineFeatures caps)
   : state(nullptr), gccaps(caps), active(0), selfDestruct(false), extended(false), d_ptr(&dptr)
{
   d_ptr->q_ptr = this;
}

QPaintEngine::~QPaintEngine()
{
}

QPainter *QPaintEngine::painter() const
{
   return state ? state->painter() : nullptr;
}

void QPaintEngine::drawPath(const QPainterPath &)
{
   if (hasFeature(PainterPaths)) {
      qWarning("QPaintEngine::drawPath() Implement when PainterPaths flag is enabled");
   }
}

void QPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
   const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

   QPainterPath path;
   path.setFillRule(Qt::WindingFill);

   if (ti.glyphs.numGlyphs) {
      ti.fontEngine->addOutlineToPath(0, 0, ti.glyphs, &path, ti.flags);
   }

   if (! path.isEmpty()) {
      painter()->save();

      painter()->setRenderHint(QPainter::Antialiasing,
         bool((painter()->renderHints() & QPainter::TextAntialiasing)
            && ! (painter()->font().styleStrategy() & QFont::NoAntialias)));

      painter()->translate(p.x(), p.y());
      painter()->fillPath(path, painter()->pen().brush());
      painter()->restore();
   }
}

void QPaintEngine::drawLines(const QLineF *linePtr, int lineCount)
{
   for (int i = 0; i < lineCount; ++i) {
      QPointF pts[2] = { linePtr[i].p1(), linePtr[i].p2() };

      if (pts[0] == pts[1]) {
         if (state->pen().capStyle() != Qt::FlatCap) {
            drawPoints(pts, 1);
         }
         continue;
      }

      drawPolygon(pts, 2, PolylineMode);
   }
}

void QPaintEngine::drawLines(const QLine *linePtr, int lineCount)
{
   struct PointF {
      qreal x;
      qreal y;
   };

   struct LineF {
      PointF p1;
      PointF p2;
   };

   Q_ASSERT(sizeof(PointF) == sizeof(QPointF));
   Q_ASSERT(sizeof(LineF)  == sizeof(QLineF));

   LineF fl[256];

   while (lineCount) {
      int i = 0;
      while (i < lineCount && i < 256) {
         fl[i].p1.x = linePtr[i].x1();
         fl[i].p1.y = linePtr[i].y1();
         fl[i].p2.x = linePtr[i].x2();
         fl[i].p2.y = linePtr[i].y2();
         ++i;
      }

      drawLines((QLineF *)(void *)fl, i);
      linePtr   += i;
      lineCount -= i;
   }
}

void QPaintEngine::drawRects(const QRect *rectPtr, int rectCount)
{
   struct RectF {
      qreal x;
      qreal y;
      qreal w;
      qreal h;
   };
   Q_ASSERT(sizeof(RectF) == sizeof(QRectF));

   RectF fr[256];
   while (rectCount) {
      int i = 0;

      while (i < rectCount && i < 256) {
         fr[i].x = rectPtr[i].x();
         fr[i].y = rectPtr[i].y();
         fr[i].w = rectPtr[i].width();
         fr[i].h = rectPtr[i].height();
         ++i;
      }

      drawRects((QRectF *)(void *)fr, i);
      rectPtr   += i;
      rectCount -= i;
   }
}

void QPaintEngine::drawRects(const QRectF *rectPtr, int rectCount)
{
   if (hasFeature(PainterPaths) && ! state->penNeedsResolving() && ! state->brushNeedsResolving()) {
      for (int i = 0; i < rectCount; ++i) {
         QPainterPath path;
         path.addRect(rectPtr[i]);

         if (path.isEmpty()) {
            continue;
         }

         drawPath(path);
      }

   } else {
      for (int i = 0; i < rectCount; ++i) {
         QRectF rf = rectPtr[i];

         QPointF pts[4] = { QPointF(rf.x(), rf.y()),
               QPointF(rf.x() + rf.width(), rf.y()),
               QPointF(rf.x() + rf.width(), rf.y() + rf.height()),
               QPointF(rf.x(), rf.y() + rf.height())
            };
         drawPolygon(pts, 4, ConvexMode);
      }
   }
}

/*!
    \internal
    Sets the paintdevice that this engine operates on to \a device
*/
void QPaintEngine::setPaintDevice(QPaintDevice *device)
{
   d_func()->pdev = device;
}

/*!
    Returns the device that this engine is painting on, if painting is
    active; otherwise returns 0.
*/
QPaintDevice *QPaintEngine::paintDevice() const
{
   return d_func()->pdev;
}


QPoint QPaintEngine::coordinateOffset() const
{
   return QPoint();
}

void QPaintEngine::setSystemClip(const QRegion &region)
{
   Q_D(QPaintEngine);
   d->systemClip = region;
   // Be backward compatible and only call d->systemStateChanged()
   // if we currently have a system transform/viewport set.
   if (d->hasSystemTransform || d->hasSystemViewport) {
      d->transformSystemClip();
      d->systemStateChanged();
   }
}

/*!
    \internal

    Returns the system clip. The system clip is read only while the
    painter is active. An empty region indicates that system clip
    is not in use.
*/

QRegion QPaintEngine::systemClip() const
{
   return d_func()->systemClip;
}

/*!
    \internal

    Sets the target rect for drawing within the backing store. This
    function should ONLY be used by the backing store.
*/
void QPaintEngine::setSystemRect(const QRect &rect)
{
   if (isActive()) {
      qWarning("QPaintEngine::setSystemRect() Unable to change while engine is active");
      return;
   }
   d_func()->systemRect = rect;
}

/*!
    \internal

    Retrieves the rect for drawing within the backing store. This
    function should ONLY be used by the backing store.
 */
QRect QPaintEngine::systemRect() const
{
   return d_func()->systemRect;
}

void QPaintEnginePrivate::drawBoxTextItem(const QPointF &p, const QTextItemInt &ti)
{
   if (!ti.glyphs.numGlyphs) {
      return;
   }

   // any fixes here should probably also be done in QFontEngineBox::draw
   const int size = qRound(ti.fontEngine->ascent());
   QVarLengthArray<QFixedPoint> positions;
   QVarLengthArray<glyph_t> glyphs;
   QTransform matrix = QTransform::fromTranslate(p.x(), p.y() - size);
   ti.fontEngine->getGlyphPositions(ti.glyphs, matrix, ti.flags, glyphs, positions);

   if (glyphs.size() == 0) {
      return;
   }

   QSize s(size - 3, size - 3);

   QPainter *painter = q_func()->state->painter();
   painter->save();
   painter->setBrush(Qt::NoBrush);
   QPen pen = painter->pen();
   pen.setWidthF(ti.fontEngine->lineThickness().toReal());
   painter->setPen(pen);

   for (int k = 0; k < positions.size(); k++) {
      painter->drawRect(QRectF(positions[k].toPointF(), s));
   }
   painter->restore();
}


