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

#include <qpainterpath.h>
#include <qpainterpath_p.h>

#include <qbitmap.h>
#include <qdebug.h>
#include <qiodevice.h>
#include <qlist.h>
#include <qmath.h>
#include <qmatrix.h>
#include <qpen.h>
#include <qpolygon.h>
#include <qtextlayout.h>
#include <qvarlengtharray.h>

#include <qbezier_p.h>
#include <qfontengine_p.h>
#include <qnumeric_p.h>
#include <qpathclipper_p.h>
#include <qstroker_p.h>
#include <qtextengine_p.h>

#include <limits.h>

#define PM_INIT
#define PM_MEASURE(x)
#define PM_DISPLAY

namespace cs_internal {
  void QPainterPathPrivateDeleter::operator()(QPainterPathPrivate *d) const {
      // need to up-cast to QPainterPathData since QPainterPathPrivate
      // has a non-virtual destructor

      if (d != nullptr && ! d->ref.deref()) {
         delete static_cast<QPainterPathData *>(d);
      }
   }
}

// This value is used to determine the length of control point vectors when approximating
// arc segments as curves. The factor is multiplied with the radius of the circle.

QPainterPath qt_stroke_dash(const QPainterPath &path, qreal *dashes, int dashCount);

void qt_find_ellipse_coords(const QRectF &r, qreal angle, qreal length,
   QPointF *startPoint, QPointF *endPoint)
{
   if (r.isNull()) {
      if (startPoint) {
         *startPoint = QPointF();
      }

      if (endPoint) {
         *endPoint = QPointF();
      }

      return;
   }

   qreal w2 = r.width() / 2;
   qreal h2 = r.height() / 2;

   qreal angles[2] = { angle, angle + length };
   QPointF *points[2] = { startPoint, endPoint };

   for (int i = 0; i < 2; ++i) {
      if (! points[i]) {
         continue;
      }

      qreal theta = angles[i] - 360 * qFloor(angles[i] / 360);
      qreal t = theta / 90;

      // truncate
      int quadrant = int(t);
      t -= quadrant;

      t = qt_t_for_arc_angle(90 * t);

      // swap x and y?
      if (quadrant & 1) {
         t = 1 - t;
      }

      qreal a, b, c, d;
      QBezier::coefficients(t, a, b, c, d);
      QPointF p(a + b + c * QT_PATH_KAPPA, d + c + b * QT_PATH_KAPPA);

      // left quadrants
      if (quadrant == 1 || quadrant == 2) {
         p.rx() = -p.x();
      }

      // top quadrants
      if (quadrant == 0 || quadrant == 1) {
         p.ry() = -p.y();
      }

      *points[i] = r.center() + QPointF(w2 * p.x(), h2 * p.y());
   }
}

int QPainterPath::elementCount() const
{
    return d_ptr ? d_ptr->elements.size() : 0;
}

QPainterPath::Element QPainterPath::elementAt(int i) const
{
    Q_ASSERT(d_ptr);
    Q_ASSERT(i >= 0 && i < elementCount());
    return d_ptr->elements.at(i);
}

void QPainterPath::setElementPositionAt(int i, qreal x, qreal y)
{
    Q_ASSERT(d_ptr);
    Q_ASSERT(i >= 0 && i < elementCount());
    detach();
    QPainterPath::Element &e = d_ptr->elements[i];
    e.x = x;
    e.y = y;
}

QPainterPath::QPainterPath()
   : d_ptr(nullptr)
{
}

QPainterPath::QPainterPath(const QPainterPath &other)
   : d_ptr(other.d_ptr.data())
{
   if (d_ptr) {
      d_ptr->ref.ref();
   }
}

QPainterPath::QPainterPath(const QPointF &startPoint)
   : d_ptr(new QPainterPathData)
{
   Element e = { startPoint.x(), startPoint.y(), MoveToElement };
   d_func()->elements << e;
}

void QPainterPath::detach()
{
    if (d_ptr->ref.load() != 1)
        detach_helper();
    setDirty(true);
}

// internal
void QPainterPath::detach_helper()
{
   QPainterPathPrivate *data = new QPainterPathData(*d_func());
   d_ptr.reset(data);
}

// internal
void QPainterPath::ensureData_helper()
{
   QPainterPathPrivate *data = new QPainterPathData;
   data->elements.reserve(16);

   QPainterPath::Element e = { 0, 0, QPainterPath::MoveToElement };
   data->elements << e;
   d_ptr.reset(data);

   Q_ASSERT(d_ptr != nullptr);
}

QPainterPath &QPainterPath::operator=(const QPainterPath &other)
{
   if (other.d_func() != d_func()) {
      QPainterPathPrivate *data = other.d_func();
      if (data) {
         data->ref.ref();
      }
      d_ptr.reset(data);
   }

   return *this;
}

QPainterPath::~QPainterPath()
{
}

void QPainterPath::closeSubpath()
{
   if (isEmpty()) {
      return;
   }
   detach();

   d_func()->close();
}

void QPainterPath::moveTo(const QPointF &p)
{
   if (! qt_is_finite(p.x()) || ! qt_is_finite(p.y())) {

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
      qDebug("QPainterPath::moveTo() Value for point x or y is invalid");
#endif

      return;
   }

   ensureData();
   detach();

   QPainterPathData *d = d_func();
   Q_ASSERT(!d->elements.isEmpty());

   d->require_moveTo = false;

   if (d->elements.last().type == MoveToElement) {
      d->elements.last().x = p.x();
      d->elements.last().y = p.y();
   } else {
      Element elm = { p.x(), p.y(), MoveToElement };
      d->elements.append(elm);
   }
   d->cStart = d->elements.size() - 1;
}

void QPainterPath::lineTo(const QPointF &p)
{
   if (! qt_is_finite(p.x()) || ! qt_is_finite(p.y())) {

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
      qDebug("QPainterPath::lineTo() Value for point x or y is invalid");
#endif

      return;
   }

   ensureData();
   detach();

   QPainterPathData *d = d_func();
   Q_ASSERT(!d->elements.isEmpty());
   d->maybeMoveTo();

   if (p == QPointF(d->elements.last())) {
      return;
   }
   Element elm = { p.x(), p.y(), LineToElement };
   d->elements.append(elm);

   d->convex = d->elements.size() == 3 || (d->elements.size() == 4 && d->isClosed());
}

void QPainterPath::cubicTo(const QPointF &c1, const QPointF &c2, const QPointF &e)
{
   if (! qt_is_finite(c1.x()) || ! qt_is_finite(c1.y()) || ! qt_is_finite(c2.x()) || ! qt_is_finite(c2.y())
      || !qt_is_finite(e.x()) || ! qt_is_finite(e.y())) {

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
      qDebug("QPainterPath::cubicTo() Value for point x or y is invalid");
#endif

      return;
   }

   ensureData();
   detach();

   QPainterPathData *d = d_func();
   Q_ASSERT(!d->elements.isEmpty());

   // Abort on empty curve as a stroker cannot handle this and the
   // curve is irrelevant anyway.
   if (d->elements.last() == c1 && c1 == c2 && c2 == e) {
      return;
   }

   d->maybeMoveTo();

   Element ce1 = { c1.x(), c1.y(), CurveToElement };
   Element ce2 = { c2.x(), c2.y(), CurveToDataElement };
   Element ee = { e.x(), e.y(), CurveToDataElement };
   d->elements << ce1 << ce2 << ee;
}

void QPainterPath::quadTo(const QPointF &c, const QPointF &e)
{
   if (! qt_is_finite(c.x()) || !qt_is_finite(c.y()) || !qt_is_finite(e.x()) || !qt_is_finite(e.y())) {

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
      qDebug("QPainterPath::quadTo() Value for point x or y is invalid");
#endif

      return;
   }

   ensureData();
   detach();

   Q_D(QPainterPath);
   Q_ASSERT(!d->elements.isEmpty());
   const QPainterPath::Element &elm = d->elements.at(elementCount() - 1);
   QPointF prev(elm.x, elm.y);

   // Abort on empty curve as a stroker cannot handle this and the
   // curve is irrelevant anyway.
   if (prev == c && c == e) {
      return;
   }

   QPointF c1((prev.x() + 2 * c.x()) / 3, (prev.y() + 2 * c.y()) / 3);
   QPointF c2((e.x() + 2 * c.x()) / 3, (e.y() + 2 * c.y()) / 3);
   cubicTo(c1, c2, e);
}

void QPainterPath::arcTo(const QRectF &rect, qreal startAngle, qreal sweepLength)
{
   if ((! qt_is_finite(rect.x()) && ! qt_is_finite(rect.y())) || ! qt_is_finite(rect.width()) || ! qt_is_finite(rect.height())
      || ! qt_is_finite(startAngle) || ! qt_is_finite(sweepLength)) {

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
      qDebug("QPainterPath::arcTo() Value for point x or y is invalid");
#endif

      return;
   }

   if (rect.isNull()) {
      return;
   }

   ensureData();
   detach();

   int point_count;
   QPointF pts[15];
   QPointF curve_start = qt_curves_for_arc(rect, startAngle, sweepLength, pts, &point_count);

   lineTo(curve_start);

   for (int i = 0; i < point_count; i += 3) {
      cubicTo(pts[i].x(), pts[i].y(), pts[i + 1].x(), pts[i + 1].y(), pts[i + 2].x(), pts[i + 2].y());
   }
}

void QPainterPath::arcMoveTo(const QRectF &rect, qreal angle)
{
   if (rect.isNull()) {
      return;
   }

   QPointF pt;
   qt_find_ellipse_coords(rect, angle, 0, &pt, nullptr);
   moveTo(pt);
}

QPointF QPainterPath::currentPosition() const
{
   return !d_ptr || d_func()->elements.isEmpty()
      ? QPointF() : QPointF(d_func()->elements.last().x, d_func()->elements.last().y);
}

void QPainterPath::addRect(const QRectF &r)
{
   if (!qt_is_finite(r.x()) || !qt_is_finite(r.y()) || !qt_is_finite(r.width()) || !qt_is_finite(r.height())) {

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
      qDebug("QPainterPath::addRect() Value for point x or y is invalid");
#endif

      return;
   }

   if (r.isNull()) {
      return;
   }

   ensureData();
   detach();

   bool first = d_func()->elements.size() < 2;

   d_func()->elements.reserve(d_func()->elements.size() + 5);
   moveTo(r.x(), r.y());

   Element l1 = { r.x() + r.width(), r.y(), LineToElement };
   Element l2 = { r.x() + r.width(), r.y() + r.height(), LineToElement };
   Element l3 = { r.x(), r.y() + r.height(), LineToElement };
   Element l4 = { r.x(), r.y(), LineToElement };

   d_func()->elements << l1 << l2 << l3 << l4;
   d_func()->require_moveTo = true;
   d_func()->convex = first;
}

void QPainterPath::addPolygon(const QPolygonF &polygon)
{
   if (polygon.isEmpty()) {
      return;
   }

   ensureData();
   detach();

   d_func()->elements.reserve(d_func()->elements.size() + polygon.size());

   moveTo(polygon.first());
   for (int i = 1; i < polygon.size(); ++i) {
      Element elm = { polygon.at(i).x(), polygon.at(i).y(), LineToElement };
      d_func()->elements << elm;
   }
}

void QPainterPath::addEllipse(const QRectF &boundingRect)
{
   if (!qt_is_finite(boundingRect.x()) || !qt_is_finite(boundingRect.y())
      || !qt_is_finite(boundingRect.width()) || !qt_is_finite(boundingRect.height())) {

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
      qDebug("QPainterPath::addEllipse() Value for ellipse is invalid");
#endif

      return;
   }

   if (boundingRect.isNull()) {
      return;
   }

   ensureData();
   detach();

   Q_D(QPainterPath);
   bool first = d_func()->elements.size() < 2;
   d->elements.reserve(d->elements.size() + 13);

   QPointF pts[12];
   int point_count;
   QPointF start = qt_curves_for_arc(boundingRect, 0, -360, pts, &point_count);

   moveTo(start);
   cubicTo(pts[0], pts[1], pts[2]);           // 0 -> 270
   cubicTo(pts[3], pts[4], pts[5]);           // 270 -> 180
   cubicTo(pts[6], pts[7], pts[8]);           // 180 -> 90
   cubicTo(pts[9], pts[10], pts[11]);         // 90 - >0
   d_func()->require_moveTo = true;

   d_func()->convex = first;
}

void QPainterPath::addText(const QPointF &point, const QFont &f, const QString &text)
{
   if (text.isEmpty()) {
      return;
   }

   ensureData();
   detach();

   QTextLayout layout(text, f);
   layout.setCacheEnabled(true);
   QTextEngine *eng = layout.engine();
   layout.beginLayout();

   QTextLine line = layout.createLine();
   (void) line;

   layout.endLayout();
   const QScriptLine &sl = eng->lines[0];
   if (!sl.length || !eng->layoutData) {
      return;
   }

   int nItems = eng->layoutData->items.size();

   qreal x(point.x());
   qreal y(point.y());

   QVarLengthArray<int> visualOrder(nItems);
   QVarLengthArray<uchar> levels(nItems);
   for (int i = 0; i < nItems; ++i) {
      levels[i] = eng->layoutData->items[i].analysis.bidiLevel;
   }

   QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

   for (int i = 0; i < nItems; ++i) {
      int item = visualOrder[i];
      QScriptItem &si = eng->layoutData->items[item];

      if (si.analysis.flags < QScriptAnalysis::TabOrObject) {
         QGlyphLayout glyphs = eng->shapedGlyphs(&si);
         QFontEngine *fe = f.d->engineForScript(si.analysis.script);
         Q_ASSERT(fe);
         fe->addOutlineToPath(x, y, glyphs, this, si.analysis.bidiLevel % 2
            ? QTextItem::RenderFlags(QTextItem::RightToLeft) : QTextItem::RenderFlags(Qt::EmptyFlag));

         const qreal lw = fe->lineThickness().toReal();
         if (f.d->underline) {
            qreal pos = fe->underlinePosition().toReal();
            addRect(x, y + pos, si.width.toReal(), lw);
         }
         if (f.d->overline) {
            qreal pos = fe->ascent().toReal() + 1;
            addRect(x, y - pos, si.width.toReal(), lw);
         }
         if (f.d->strikeOut) {
            qreal pos = fe->ascent().toReal() / 3;
            addRect(x, y - pos, si.width.toReal(), lw);
         }
      }
      x += si.width.toReal();
   }
}

void QPainterPath::addPath(const QPainterPath &other)
{
   if (other.isEmpty()) {
      return;
   }

   ensureData();
   detach();

   QPainterPathData *d = reinterpret_cast<QPainterPathData *>(d_func());
   // Remove last moveto so we don't get multiple moveto's
   if (d->elements.last().type == MoveToElement) {
      d->elements.remove(d->elements.size() - 1);
   }

   // Locate where our own current subpath will start after the other path is added.
   int cStart = d->elements.size() + other.d_func()->cStart;
   d->elements += other.d_func()->elements;
   d->cStart = cStart;

   d->require_moveTo = other.d_func()->isClosed();
}

void QPainterPath::connectPath(const QPainterPath &other)
{
   if (other.isEmpty()) {
      return;
   }

   ensureData();
   detach();

   QPainterPathData *d = reinterpret_cast<QPainterPathData *>(d_func());
   // Remove last moveto so we don't get multiple moveto's
   if (d->elements.last().type == MoveToElement) {
      d->elements.remove(d->elements.size() - 1);
   }

   // Locate where our own current subpath will start after the other path is added.
   int cStart = d->elements.size() + other.d_func()->cStart;
   int first = d->elements.size();
   d->elements += other.d_func()->elements;

   if (first != 0) {
      d->elements[first].type = LineToElement;
   }

   // avoid duplicate points
   if (first > 0 && QPointF(d->elements[first]) == QPointF(d->elements[first - 1])) {
      d->elements.remove(first--);
      --cStart;
   }

   if (cStart != first) {
      d->cStart = cStart;
   }
}

void QPainterPath::addRegion(const QRegion &region)
{
   ensureData();
   detach();

   QVector<QRect> rects = region.rects();
   d_func()->elements.reserve(rects.size() * 5);
   for (int i = 0; i < rects.size(); ++i) {
      addRect(rects.at(i));
   }
}

Qt::FillRule QPainterPath::fillRule() const
{
   return isEmpty() ? Qt::OddEvenFill : d_func()->fillRule;
}

void QPainterPath::setFillRule(Qt::FillRule fillRule)
{
   ensureData();
   if (d_func()->fillRule == fillRule) {
      return;
   }
   detach();

   d_func()->fillRule = fillRule;
}

#define QT_BEZIER_A(bezier, coord) 3 * (-bezier.coord##1 \
                                        + 3*bezier.coord##2 \
                                        - 3*bezier.coord##3 \
                                        +bezier.coord##4)

#define QT_BEZIER_B(bezier, coord) 6 * (bezier.coord##1 \
                                        - 2*bezier.coord##2 \
                                        + bezier.coord##3)

#define QT_BEZIER_C(bezier, coord) 3 * (- bezier.coord##1 \
                                        + bezier.coord##2)

#define QT_BEZIER_CHECK_T(bezier, t) \
    if (t >= 0 && t <= 1) { \
        QPointF p(b.pointAt(t)); \
        if (p.x() < minx) minx = p.x(); \
        else if (p.x() > maxx) maxx = p.x(); \
        if (p.y() < miny) miny = p.y(); \
        else if (p.y() > maxy) maxy = p.y(); \
    }


static QRectF qt_painterpath_bezier_extrema(const QBezier &b)
{
   qreal minx, miny, maxx, maxy;

   // initialize with end points
   if (b.x1 < b.x4) {
      minx = b.x1;
      maxx = b.x4;
   } else {
      minx = b.x4;
      maxx = b.x1;
   }
   if (b.y1 < b.y4) {
      miny = b.y1;
      maxy = b.y4;
   } else {
      miny = b.y4;
      maxy = b.y1;
   }

   // Update for the X extrema
   {
      qreal ax = QT_BEZIER_A(b, x);
      qreal bx = QT_BEZIER_B(b, x);
      qreal cx = QT_BEZIER_C(b, x);
      // specialcase quadratic curves to avoid div by zero
      if (qFuzzyIsNull(ax)) {

         // linear curves are covered by initialization.
         if (!qFuzzyIsNull(bx)) {
            qreal t = -cx / bx;
            QT_BEZIER_CHECK_T(b, t);
         }

      } else {
         const qreal tx = bx * bx - 4 * ax * cx;

         if (tx >= 0) {
            qreal temp = qSqrt(tx);
            qreal rcp = 1 / (2 * ax);
            qreal t1 = (-bx + temp) * rcp;
            QT_BEZIER_CHECK_T(b, t1);

            qreal t2 = (-bx - temp) * rcp;
            QT_BEZIER_CHECK_T(b, t2);
         }
      }
   }

   // Update for the Y extrema
   {
      qreal ay = QT_BEZIER_A(b, y);
      qreal by = QT_BEZIER_B(b, y);
      qreal cy = QT_BEZIER_C(b, y);

      // specialcase quadratic curves to avoid div by zero
      if (qFuzzyIsNull(ay)) {

         // linear curves are covered by initialization.
         if (!qFuzzyIsNull(by)) {
            qreal t = -cy / by;
            QT_BEZIER_CHECK_T(b, t);
         }

      } else {
         const qreal ty = by * by - 4 * ay * cy;

         if (ty > 0) {
            qreal temp = qSqrt(ty);
            qreal rcp = 1 / (2 * ay);
            qreal t1 = (-by + temp) * rcp;
            QT_BEZIER_CHECK_T(b, t1);

            qreal t2 = (-by - temp) * rcp;
            QT_BEZIER_CHECK_T(b, t2);
         }
      }
   }
   return QRectF(minx, miny, maxx - minx, maxy - miny);
}

QRectF QPainterPath::boundingRect() const
{
   if (!d_ptr) {
      return QRectF();
   }
   QPainterPathData *d = d_func();

   if (d->dirtyBounds) {
      computeBoundingRect();
   }
   return d->bounds;
}

QRectF QPainterPath::controlPointRect() const
{
   if (!d_ptr) {
      return QRectF();
   }
   QPainterPathData *d = d_func();

   if (d->dirtyControlBounds) {
      computeControlPointRect();
   }
   return d->controlBounds;
}

bool QPainterPath::isEmpty() const
{
    return !d_ptr || (d_ptr->elements.size() == 1 && d_ptr->elements.first().type == MoveToElement);
}

QPainterPath QPainterPath::toReversed() const
{
   Q_D(const QPainterPath);
   QPainterPath rev;

   if (isEmpty()) {
      rev = *this;
      return rev;
   }

   rev.moveTo(d->elements.at(d->elements.size() - 1).x, d->elements.at(d->elements.size() - 1).y);

   for (int i = d->elements.size() - 1; i >= 1; --i) {
      const QPainterPath::Element &elm = d->elements.at(i);
      const QPainterPath::Element &prev = d->elements.at(i - 1);
      switch (elm.type) {
         case LineToElement:
            rev.lineTo(prev.x, prev.y);
            break;
         case MoveToElement:
            rev.moveTo(prev.x, prev.y);
            break;
         case CurveToDataElement: {
            Q_ASSERT(i >= 3);
            const QPainterPath::Element &cp1 = d->elements.at(i - 2);
            const QPainterPath::Element &sp = d->elements.at(i - 3);
            Q_ASSERT(prev.type == CurveToDataElement);
            Q_ASSERT(cp1.type == CurveToElement);
            rev.cubicTo(prev.x, prev.y, cp1.x, cp1.y, sp.x, sp.y);
            i -= 2;
            break;
         }
         default:
            Q_ASSERT(!"qt_reversed_path");
            break;
      }
   }

   return rev;
}

QList<QPolygonF> QPainterPath::toSubpathPolygons(const QTransform &matrix) const
{

   Q_D(const QPainterPath);
   QList<QPolygonF> flatCurves;

   if (isEmpty()) {
      return flatCurves;
   }

   QPolygonF current;
   for (int i = 0; i < elementCount(); ++i) {
      const QPainterPath::Element &e = d->elements.at(i);
      switch (e.type) {
         case QPainterPath::MoveToElement:
            if (current.size() > 1) {
               flatCurves += current;
            }
            current.clear();
            current.reserve(16);
            current += QPointF(e.x, e.y) * matrix;
            break;

         case QPainterPath::LineToElement:
            current += QPointF(e.x, e.y) * matrix;
            break;

         case QPainterPath::CurveToElement: {
            Q_ASSERT(d->elements.at(i + 1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(d->elements.at(i + 2).type == QPainterPath::CurveToDataElement);
            QBezier bezier = QBezier::fromPoints(QPointF(d->elements.at(i - 1).x, d->elements.at(i - 1).y) * matrix,
                  QPointF(e.x, e.y) * matrix,
                  QPointF(d->elements.at(i + 1).x, d->elements.at(i + 1).y) * matrix,
                  QPointF(d->elements.at(i + 2).x, d->elements.at(i + 2).y) * matrix);
            bezier.addToPolygon(&current);
            i += 2;
            break;
         }

         case QPainterPath::CurveToDataElement:
            Q_ASSERT(!"QPainterPath::toSubpathPolygons(), bad element type");
            break;
      }
   }

   if (current.size() > 1) {
      flatCurves += current;
   }

   return flatCurves;
}

QList<QPolygonF> QPainterPath::toSubpathPolygons(const QMatrix &matrix) const
{
   return toSubpathPolygons(QTransform(matrix));
}

QList<QPolygonF> QPainterPath::toFillPolygons(const QTransform &matrix) const
{
   QList<QPolygonF> polys;

   QList<QPolygonF> subpaths = toSubpathPolygons(matrix);
   int count = subpaths.size();

   if (count == 0) {
      return polys;
   }

   QVector<QRectF> bounds;
   for (int i = 0; i < count; ++i) {
      bounds += subpaths.at(i).boundingRect();
   }

   QVector< QList<int>> isects;
   isects.resize(count);

   // find all intersections
   for (int j = 0; j < count; ++j) {
      if (subpaths.at(j).size() <= 2) {
         continue;
      }

      QRectF cbounds = bounds.at(j);
      for (int i = 0; i < count; ++i) {
         if (cbounds.intersects(bounds.at(i))) {
            isects[j] << i;
         }
      }
   }

   // flatten the sets of intersections
   for (int i = 0; i < count; ++i) {
      const QList<int> &current_isects = isects.at(i);
      for (int j = 0; j < current_isects.size(); ++j) {
         int isect_j = current_isects.at(j);

         if (isect_j == i) {
            continue;
         }

         for (int k = 0; k < isects[isect_j].size(); ++k) {
            int isect_k = isects[isect_j][k];
            if (isect_k != i && !isects.at(i).contains(isect_k)) {
               isects[i] += isect_k;
            }
         }
         isects[isect_j].clear();
      }
   }

   // Join the intersected subpaths as rewinded polygons
   for (int i = 0; i < count; ++i) {
      const QList<int> &subpath_list = isects[i];

      if (!subpath_list.isEmpty()) {
         QPolygonF buildUp;

         for (int j = 0; j < subpath_list.size(); ++j) {
            const QPolygonF &subpath = subpaths.at(subpath_list.at(j));
            buildUp += subpath;
            if (!subpath.isClosed()) {
               buildUp += subpath.first();
            }
            if (!buildUp.isClosed()) {
               buildUp += buildUp.first();
            }
         }
         polys += buildUp;
      }
   }

   return polys;
}

QList<QPolygonF> QPainterPath::toFillPolygons(const QMatrix &matrix) const
{
   return toFillPolygons(QTransform(matrix));
}

//same as qt_polygon_isect_line in qpolygon.cpp
static void qt_painterpath_isect_line(const QPointF &p1, const QPointF &p2, const QPointF &pos, int *winding)
{
   qreal x1 = p1.x();
   qreal y1 = p1.y();
   qreal x2 = p2.x();
   qreal y2 = p2.y();
   qreal y = pos.y();

   int dir = 1;

   if (qFuzzyCompare(y1, y2)) {
      // ignore horizontal lines according to scan conversion rule
      return;
   } else if (y2 < y1) {
      qreal x_tmp = x2;
      x2 = x1;
      x1 = x_tmp;

      qreal y_tmp = y2;
      y2 = y1;
      y1 = y_tmp;
      dir = -1;
   }

   if (y >= y1 && y < y2) {
      qreal x = x1 + ((x2 - x1) / (y2 - y1)) * (y - y1);

      // count up the winding number if we're
      if (x <= pos.x()) {
         (*winding) += dir;
      }
   }
}

static void qt_painterpath_isect_curve(const QBezier &bezier, const QPointF &pt,
   int *winding, int depth = 0)
{
   qreal y = pt.y();
   qreal x = pt.x();
   QRectF bounds = bezier.bounds();

   // potential intersection, divide and try again...
   // Please note that a sideeffect of the bottom exclusion is that
   // horizontal lines are dropped, but this is correct according to
   // scan conversion rules.
   if (y >= bounds.y() && y < bounds.y() + bounds.height()) {

      // hit lower limit... This is a rough threshold, but its a
      // tradeoff between speed and precision.
      const qreal lower_bound = qreal(.001);
      if (depth == 32 || (bounds.width() < lower_bound && bounds.height() < lower_bound)) {
         // We make the assumption here that the curve starts to
         // approximate a line after while (i.e. that it doesn't
         // change direction drastically during its slope)
         if (bezier.pt1().x() <= x) {
            (*winding) += (bezier.pt4().y() > bezier.pt1().y() ? 1 : -1);
         }
         return;
      }

      // split curve and try again...
      QBezier first_half, second_half;
      bezier.split(&first_half, &second_half);
      qt_painterpath_isect_curve(first_half, pt, winding, depth + 1);
      qt_painterpath_isect_curve(second_half, pt, winding, depth + 1);
   }
}

bool QPainterPath::contains(const QPointF &pt) const
{
   if (isEmpty() || !controlPointRect().contains(pt)) {
      return false;
   }

   QPainterPathData *d = d_func();

   int winding_number = 0;

   QPointF last_pt;
   QPointF last_start;
   for (int i = 0; i < d->elements.size(); ++i) {
      const Element &e = d->elements.at(i);

      switch (e.type) {

         case MoveToElement:
            if (i > 0) {
               // implicitly close all paths.
               qt_painterpath_isect_line(last_pt, last_start, pt, &winding_number);
            }

            last_start = last_pt = e;
            break;

         case LineToElement:
            qt_painterpath_isect_line(last_pt, e, pt, &winding_number);
            last_pt = e;
            break;

         case CurveToElement: {
            const QPainterPath::Element &cp2 = d->elements.at(++i);
            const QPainterPath::Element &ep = d->elements.at(++i);
            qt_painterpath_isect_curve(QBezier::fromPoints(last_pt, e, cp2, ep),
               pt, &winding_number);
            last_pt = ep;

         }
         break;

         default:
            break;
      }
   }

   // implicitly close last subpath
   if (last_pt != last_start) {
      qt_painterpath_isect_line(last_pt, last_start, pt, &winding_number);
   }

   return (d->fillRule == Qt::WindingFill
         ? (winding_number != 0)
         : ((winding_number % 2) != 0));
}

static bool qt_painterpath_isect_line_rect(qreal x1, qreal y1, qreal x2, qreal y2, const QRectF &rect)
{
   enum PainterDirection {
      Left,
      Right,
      Top,
      Bottom
   };

   qreal left   = rect.left();
   qreal right  = rect.right();
   qreal top    = rect.top();
   qreal bottom = rect.bottom();

   // clip the lines, after cohen-sutherland, see e.g. http://www.nondot.org/~sabre/graphpro/line6.html
   int p1 = ((x1 < left) << Left)
      | ((x1 > right) << Right)
      | ((y1 < top) << Top)
      | ((y1 > bottom) << Bottom);

   int p2 = ((x2 < left) << Left)
      | ((x2 > right) << Right)
      | ((y2 < top) << Top)
      | ((y2 > bottom) << Bottom);

   if (p1 & p2) {
      // completely inside
      return false;
   }

   if (p1 | p2) {
      qreal dx = x2 - x1;
      qreal dy = y2 - y1;

      // clip x coordinates
      if (x1 < left) {
         y1 += dy / dx * (left - x1);
         x1 = left;
      } else if (x1 > right) {
         y1 -= dy / dx * (x1 - right);
         x1 = right;
      }
      if (x2 < left) {
         y2 += dy / dx * (left - x2);
         x2 = left;
      } else if (x2 > right) {
         y2 -= dy / dx * (x2 - right);
         x2 = right;
      }

      p1 = ((y1 < top) << Top)
         | ((y1 > bottom) << Bottom);
      p2 = ((y2 < top) << Top)
         | ((y2 > bottom) << Bottom);

      if (p1 & p2) {
         return false;
      }

      // clip y coordinates
      if (y1 < top) {
         x1 += dx / dy * (top - y1);
         y1 = top;
      } else if (y1 > bottom) {
         x1 -= dx / dy * (y1 - bottom);
         y1 = bottom;
      }
      if (y2 < top) {
         x2 += dx / dy * (top - y2);
         y2 = top;
      } else if (y2 > bottom) {
         x2 -= dx / dy * (y2 - bottom);
         y2 = bottom;
      }

      p1 = ((x1 < left) << Left)
         | ((x1 > right) << Right);
      p2 = ((x2 < left) << Left)
         | ((x2 > right) << Right);

      if (p1 & p2) {
         return false;
      }

      return true;
   }
   return false;
}

static bool qt_isect_curve_horizontal(const QBezier &bezier, qreal y, qreal x1, qreal x2, int depth = 0)
{
   QRectF bounds = bezier.bounds();

   if (y >= bounds.top() && y < bounds.bottom()
      && bounds.right() >= x1 && bounds.left() < x2) {
      const qreal lower_bound = qreal(.01);
      if (depth == 32 || (bounds.width() < lower_bound && bounds.height() < lower_bound)) {
         return true;
      }

      QBezier first_half, second_half;
      bezier.split(&first_half, &second_half);
      if (qt_isect_curve_horizontal(first_half, y, x1, x2, depth + 1)
         || qt_isect_curve_horizontal(second_half, y, x1, x2, depth + 1)) {
         return true;
      }
   }
   return false;
}

static bool qt_isect_curve_vertical(const QBezier &bezier, qreal x, qreal y1, qreal y2, int depth = 0)
{
   QRectF bounds = bezier.bounds();

   if (x >= bounds.left() && x < bounds.right()
      && bounds.bottom() >= y1 && bounds.top() < y2) {
      const qreal lower_bound = qreal(.01);
      if (depth == 32 || (bounds.width() < lower_bound && bounds.height() < lower_bound)) {
         return true;
      }

      QBezier first_half, second_half;
      bezier.split(&first_half, &second_half);
      if (qt_isect_curve_vertical(first_half, x, y1, y2, depth + 1)
         || qt_isect_curve_vertical(second_half, x, y1, y2, depth + 1)) {
         return true;
      }
   }
   return false;
}

static bool qt_painterpath_check_crossing(const QPainterPath *path, const QRectF &rect)
{
   QPointF last_pt;
   QPointF last_start;
   for (int i = 0; i < path->elementCount(); ++i) {
      const QPainterPath::Element &e = path->elementAt(i);

      switch (e.type) {

         case QPainterPath::MoveToElement:
            if (i > 0
               && qFuzzyCompare(last_pt.x(), last_start.x())
               && qFuzzyCompare(last_pt.y(), last_start.y())
               && qt_painterpath_isect_line_rect(last_pt.x(), last_pt.y(),
                  last_start.x(), last_start.y(), rect)) {
               return true;
            }
            last_start = last_pt = e;
            break;

         case QPainterPath::LineToElement:
            if (qt_painterpath_isect_line_rect(last_pt.x(), last_pt.y(), e.x, e.y, rect)) {
               return true;
            }
            last_pt = e;
            break;

         case QPainterPath::CurveToElement: {
            QPointF cp2 = path->elementAt(++i);
            QPointF ep = path->elementAt(++i);
            QBezier bezier = QBezier::fromPoints(last_pt, e, cp2, ep);
            if (qt_isect_curve_horizontal(bezier, rect.top(), rect.left(), rect.right())
               || qt_isect_curve_horizontal(bezier, rect.bottom(), rect.left(), rect.right())
               || qt_isect_curve_vertical(bezier, rect.left(), rect.top(), rect.bottom())
               || qt_isect_curve_vertical(bezier, rect.right(), rect.top(), rect.bottom())) {
               return true;
            }
            last_pt = ep;
         }
         break;

         default:
            break;
      }
   }

   // implicitly close last subpath
   if (last_pt != last_start
      && qt_painterpath_isect_line_rect(last_pt.x(), last_pt.y(),
         last_start.x(), last_start.y(), rect)) {
      return true;
   }

   return false;
}

bool QPainterPath::intersects(const QRectF &rect) const
{
   if (elementCount() == 1 && rect.contains(elementAt(0))) {
      return true;
   }

   if (isEmpty()) {
      return false;
   }

   QRectF cp = controlPointRect();
   QRectF rn = rect.normalized();

   // QRectF::intersects returns false if one of the rects is a null rect
   // which would happen for a painter path consisting of a vertical or
   // horizontal line
   if (qMax(rn.left(), cp.left()) > qMin(rn.right(), cp.right())
      || qMax(rn.top(), cp.top()) > qMin(rn.bottom(), cp.bottom())) {
      return false;
   }

   // If any path element cross the rect its bound to be an intersection
   if (qt_painterpath_check_crossing(this, rect)) {
      return true;
   }

   if (contains(rect.center())) {
      return true;
   }

   Q_D(QPainterPath);

   // Check if the rectangle surounds any subpath...
   for (int i = 0; i < d->elements.size(); ++i) {
      const Element &e = d->elements.at(i);
      if (e.type == QPainterPath::MoveToElement && rect.contains(e)) {
         return true;
      }
   }

   return false;
}

void QPainterPath::translate(qreal dx, qreal dy)
{
   if (!d_ptr || (dx == 0 && dy == 0)) {
      return;
   }

   int elementsLeft = d_ptr->elements.size();
   if (elementsLeft <= 0) {
      return;
   }

   detach();
   QPainterPath::Element *element = d_func()->elements.data();
   Q_ASSERT(element);
   while (elementsLeft--) {
      element->x += dx;
      element->y += dy;
      ++element;
   }
}

QPainterPath QPainterPath::translated(qreal dx, qreal dy) const
{
   QPainterPath copy(*this);
   copy.translate(dx, dy);
   return copy;
}

bool QPainterPath::contains(const QRectF &rect) const
{
   Q_D(QPainterPath);

   // the path is empty or the control point rect doesn't completely
   // cover the rectangle we abort stratight away.
   if (isEmpty() || !controlPointRect().contains(rect)) {
      return false;
   }

   // if there are intersections, chances are that the rect is not
   // contained, except if we have winding rule, in which case it
   // still might.
   if (qt_painterpath_check_crossing(this, rect)) {
      if (fillRule() == Qt::OddEvenFill) {
         return false;
      } else {
         // Do some wague sampling in the winding case. This is not
         // precise but it should mostly be good enough.
         if (!contains(rect.topLeft()) ||
            !contains(rect.topRight()) ||
            !contains(rect.bottomRight()) ||
            !contains(rect.bottomLeft())) {
            return false;
         }
      }
   }

   // If there exists a point inside that is not part of the path its
   // because: rectangle lies completely outside path or a subpath
   // excludes parts of the rectangle. Both cases mean that the rect
   // is not contained
   if (!contains(rect.center())) {
      return false;
   }

   // If there are any subpaths inside this rectangle we need to
   // check if they are still contained as a result of the fill
   // rule. This can only be the case for WindingFill though. For
   // OddEvenFill the rect will never be contained if it surrounds a
   // subpath. (the case where two subpaths are completely identical
   // can be argued but we choose to neglect it).
   for (int i = 0; i < d->elements.size(); ++i) {
      const Element &e = d->elements.at(i);
      if (e.type == QPainterPath::MoveToElement && rect.contains(e)) {
         if (fillRule() == Qt::OddEvenFill) {
            return false;
         }

         bool stop = false;
         for (; !stop && i < d->elements.size(); ++i) {
            const Element &el = d->elements.at(i);
            switch (el.type) {
               case MoveToElement:
                  stop = true;
                  break;
               case LineToElement:
                  if (!contains(el)) {
                     return false;
                  }
                  break;
               case CurveToElement:
                  if (!contains(d->elements.at(i + 2))) {
                     return false;
                  }
                  i += 2;
                  break;
               default:
                  break;
            }
         }

         // compensate for the last ++i in the inner for
         --i;
      }
   }

   return true;
}

static inline bool epsilonCompare(const QPointF &a, const QPointF &b, const QSizeF &epsilon)
{
   return qAbs(a.x() - b.x()) <= epsilon.width()
      && qAbs(a.y() - b.y()) <= epsilon.height();
}

bool QPainterPath::operator==(const QPainterPath &path) const
{
   QPainterPathData *d = reinterpret_cast<QPainterPathData *>(d_func());
   if (path.d_func() == d) {
      return true;
   } else if (!d || !path.d_func()) {
      return false;
   } else if (d->fillRule != path.d_func()->fillRule) {
      return false;
   } else if (d->elements.size() != path.d_func()->elements.size()) {
      return false;
   }

   const qreal qt_epsilon = sizeof(qreal) == sizeof(double) ? 1e-12 : qreal(1e-5);

   QSizeF epsilon = boundingRect().size();
   epsilon.rwidth() *= qt_epsilon;
   epsilon.rheight() *= qt_epsilon;

   for (int i = 0; i < d->elements.size(); ++i)
      if (d->elements.at(i).type != path.d_func()->elements.at(i).type
         || !epsilonCompare(d->elements.at(i), path.d_func()->elements.at(i), epsilon)) {
         return false;
      }

   return true;
}

bool QPainterPath::operator!=(const QPainterPath &path) const
{
   return !(*this == path);
}

QPainterPath QPainterPath::operator&(const QPainterPath &other) const
{
   return intersected(other);
}

QPainterPath QPainterPath::operator|(const QPainterPath &other) const
{
   return united(other);
}

QPainterPath QPainterPath::operator+(const QPainterPath &other) const
{
   return united(other);
}

QPainterPath QPainterPath::operator-(const QPainterPath &other) const
{
   return subtracted(other);
}

QPainterPath &QPainterPath::operator&=(const QPainterPath &other)
{
   return *this = (*this & other);
}

QPainterPath &QPainterPath::operator|=(const QPainterPath &other)
{
   return *this = (*this | other);
}

QPainterPath &QPainterPath::operator+=(const QPainterPath &other)
{
   return *this = (*this + other);
}

QPainterPath &QPainterPath::operator-=(const QPainterPath &other)
{
   return *this = (*this - other);
}

QDataStream &operator<<(QDataStream &s, const QPainterPath &p)
{
   if (p.isEmpty()) {
      s << 0;
      return s;
   }

   s << p.elementCount();
   for (int i = 0; i < p.d_func()->elements.size(); ++i) {
      const QPainterPath::Element &e = p.d_func()->elements.at(i);
      s << int(e.type);
      s << double(e.x) << double(e.y);
   }
   s << p.d_func()->cStart;
   s << int(p.d_func()->fillRule);
   return s;
}

QDataStream &operator>>(QDataStream &s, QPainterPath &p)
{
   int size;
   s >> size;

   if (size == 0) {
      return s;
   }

   p.ensureData(); // in case if p.d_func() == 0

   if (p.d_func()->elements.size() == 1) {
      Q_ASSERT(p.d_func()->elements.at(0).type == QPainterPath::MoveToElement);
      p.d_func()->elements.clear();
   }

   p.d_func()->elements.reserve(p.d_func()->elements.size() + size);

   for (int i = 0; i < size; ++i) {
      int type;
      double x, y;
      s >> type;
      s >> x;
      s >> y;
      Q_ASSERT(type >= 0 && type <= 3);

      if (!qt_is_finite(x) || !qt_is_finite(y)) {

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
         qDebug("QDataStream::operator>>() Element in QPainterPath is invalid");
#endif

         continue;
      }

      QPainterPath::Element elm = { qreal(x), qreal(y), QPainterPath::ElementType(type) };
      p.d_func()->elements.append(elm);
   }

   s >> p.d_func()->cStart;
   int fillRule;
   s >> fillRule;
   Q_ASSERT(fillRule == Qt::OddEvenFill || Qt::WindingFill);
   p.d_func()->fillRule = Qt::FillRule(fillRule);
   p.d_func()->dirtyBounds = true;
   p.d_func()->dirtyControlBounds = true;

   return s;
}

void qt_path_stroke_move_to(qfixed x, qfixed y, void *data)
{
   ((QPainterPath *) data)->moveTo(qt_fixed_to_real(x), qt_fixed_to_real(y));
}

void qt_path_stroke_line_to(qfixed x, qfixed y, void *data)
{
   ((QPainterPath *) data)->lineTo(qt_fixed_to_real(x), qt_fixed_to_real(y));
}

void qt_path_stroke_cubic_to(qfixed c1x, qfixed c1y,
   qfixed c2x, qfixed c2y,
   qfixed ex, qfixed ey,
   void *data)
{
   ((QPainterPath *) data)->cubicTo(qt_fixed_to_real(c1x), qt_fixed_to_real(c1y),
      qt_fixed_to_real(c2x), qt_fixed_to_real(c2y),
      qt_fixed_to_real(ex), qt_fixed_to_real(ey));
}

QPainterPathStrokerPrivate::QPainterPathStrokerPrivate()
   : dashOffset(0)
{
   stroker.setMoveToHook(qt_path_stroke_move_to);
   stroker.setLineToHook(qt_path_stroke_line_to);
   stroker.setCubicToHook(qt_path_stroke_cubic_to);
}


QPainterPathStroker::QPainterPathStroker()
   : d_ptr(new QPainterPathStrokerPrivate)
{
}

QPainterPathStroker::QPainterPathStroker(const QPen &pen)
    : d_ptr(new QPainterPathStrokerPrivate)
{
    setWidth(pen.widthF());
    setCapStyle(pen.capStyle());
    setJoinStyle(pen.joinStyle());
    setMiterLimit(pen.miterLimit());
    setDashOffset(pen.dashOffset());
    if (pen.style() == Qt::CustomDashLine)
        setDashPattern(pen.dashPattern());
    else
        setDashPattern(pen.style());
}

QPainterPathStroker::~QPainterPathStroker()
{
}

QPainterPath QPainterPathStroker::createStroke(const QPainterPath &path) const
{
   QPainterPathStrokerPrivate *d = const_cast<QPainterPathStrokerPrivate *>(d_func());
   QPainterPath stroke;
   if (path.isEmpty()) {
      return path;
   }
   if (d->dashPattern.isEmpty()) {
      d->stroker.strokePath(path, &stroke, QTransform());
   } else {
      QDashStroker dashStroker(&d->stroker);
      dashStroker.setDashPattern(d->dashPattern);
      dashStroker.setDashOffset(d->dashOffset);
      dashStroker.setClipRect(d->stroker.clipRect());
      dashStroker.strokePath(path, &stroke, QTransform());
   }
   stroke.setFillRule(Qt::WindingFill);
   return stroke;
}

void QPainterPathStroker::setWidth(qreal width)
{
   Q_D(QPainterPathStroker);
   if (width <= 0) {
      width = 1;
   }
   d->stroker.setStrokeWidth(qt_real_to_fixed(width));
}

qreal QPainterPathStroker::width() const
{
   return qt_fixed_to_real(d_func()->stroker.strokeWidth());
}

void QPainterPathStroker::setCapStyle(Qt::PenCapStyle style)
{
   d_func()->stroker.setCapStyle(style);
}

Qt::PenCapStyle QPainterPathStroker::capStyle() const
{
   return d_func()->stroker.capStyle();
}

void QPainterPathStroker::setJoinStyle(Qt::PenJoinStyle style)
{
   d_func()->stroker.setJoinStyle(style);
}

Qt::PenJoinStyle QPainterPathStroker::joinStyle() const
{
   return d_func()->stroker.joinStyle();
}

void QPainterPathStroker::setMiterLimit(qreal limit)
{
   d_func()->stroker.setMiterLimit(qt_real_to_fixed(limit));
}

qreal QPainterPathStroker::miterLimit() const
{
   return qt_fixed_to_real(d_func()->stroker.miterLimit());
}

void QPainterPathStroker::setCurveThreshold(qreal threshold)
{
   d_func()->stroker.setCurveThreshold(qt_real_to_fixed(threshold));
}

qreal QPainterPathStroker::curveThreshold() const
{
   return qt_fixed_to_real(d_func()->stroker.curveThreshold());
}

void QPainterPathStroker::setDashPattern(Qt::PenStyle style)
{
   d_func()->dashPattern = QDashStroker::patternForStyle(style);
}

void QPainterPathStroker::setDashPattern(const QVector<qreal> &dashPattern)
{
   d_func()->dashPattern.clear();
   for (int i = 0; i < dashPattern.size(); ++i) {
      d_func()->dashPattern << qt_real_to_fixed(dashPattern.at(i));
   }
}

QVector<qreal> QPainterPathStroker::dashPattern() const
{
   return d_func()->dashPattern;
}

qreal QPainterPathStroker::dashOffset() const
{
   return d_func()->dashOffset;
}

void QPainterPathStroker::setDashOffset(qreal offset)
{
   d_func()->dashOffset = offset;
}

QPolygonF QPainterPath::toFillPolygon(const QTransform &matrix) const
{

   QList<QPolygonF> flats = toSubpathPolygons(matrix);
   QPolygonF polygon;
   if (flats.isEmpty()) {
      return polygon;
   }
   QPointF first = flats.first().first();
   for (int i = 0; i < flats.size(); ++i) {
      polygon += flats.at(i);
      if (!flats.at(i).isClosed()) {
         polygon += flats.at(i).first();
      }
      if (i > 0) {
         polygon += first;
      }
   }

   return polygon;
}

QPolygonF QPainterPath::toFillPolygon(const QMatrix &matrix) const
{
   return toFillPolygon(QTransform(matrix));
}

// derivative of the equation
static inline qreal slopeAt(qreal t, qreal a, qreal b, qreal c, qreal d)
{
   return 3 * t * t * (d - 3 * c + 3 * b - a) + 6 * t * (c - 2 * b + a) + 3 * (b - a);
}

qreal QPainterPath::length() const
{
   Q_D(QPainterPath);
   if (isEmpty()) {
      return 0;
   }

   qreal len = 0;
   for (int i = 1; i < d->elements.size(); ++i) {
      const Element &e = d->elements.at(i);

      switch (e.type) {
         case MoveToElement:
            break;
         case LineToElement: {
            len += QLineF(d->elements.at(i - 1), e).length();
            break;
         }
         case CurveToElement: {
            QBezier b = QBezier::fromPoints(d->elements.at(i - 1),
                  e,
                  d->elements.at(i + 1),
                  d->elements.at(i + 2));
            len += b.length();
            i += 2;
            break;
         }
         default:
            break;
      }
   }

   return len;
}

qreal QPainterPath::percentAtLength(qreal len) const
{
   Q_D(QPainterPath);
   if (isEmpty() || len <= 0) {
      return 0;
   }

   qreal totalLength = length();
   if (len > totalLength) {
      return 1;
   }

   qreal curLen = 0;
   for (int i = 1; i < d->elements.size(); ++i) {
      const Element &e = d->elements.at(i);

      switch (e.type) {
         case MoveToElement:
            break;
         case LineToElement: {
            QLineF line(d->elements.at(i - 1), e);
            qreal llen = line.length();
            curLen += llen;
            if (curLen >= len) {
               return len / totalLength ;
            }

            break;
         }
         case CurveToElement: {
            QBezier b = QBezier::fromPoints(d->elements.at(i - 1),
                  e,
                  d->elements.at(i + 1),
                  d->elements.at(i + 2));
            qreal blen = b.length();
            qreal prevLen = curLen;
            curLen += blen;

            if (curLen >= len) {
               qreal res = b.tAtLength(len - prevLen);
               return (res * blen + prevLen) / totalLength;
            }

            i += 2;
            break;
         }
         default:
            break;
      }
   }

   return 0;
}

static inline QBezier bezierAtT(const QPainterPath &path, qreal t, qreal *startingLength, qreal *bezierLength)
{
   *startingLength = 0;
   if (t > 1) {
      return QBezier();
   }

   qreal curLen = 0;
   qreal totalLength = path.length();

   const int lastElement = path.elementCount() - 1;
   for (int i = 0; i <= lastElement; ++i) {
      const QPainterPath::Element &e = path.elementAt(i);

      switch (e.type) {
         case QPainterPath::MoveToElement:
            break;
         case QPainterPath::LineToElement: {
            QLineF line(path.elementAt(i - 1), e);
            qreal llen = line.length();
            curLen += llen;
            if (i == lastElement || curLen / totalLength >= t) {
               *bezierLength = llen;
               QPointF a = path.elementAt(i - 1);
               QPointF delta = e - a;
               return QBezier::fromPoints(a, a + delta / 3, a + 2 * delta / 3, e);
            }
            break;
         }
         case QPainterPath::CurveToElement: {
            QBezier b = QBezier::fromPoints(path.elementAt(i - 1),
                  e,
                  path.elementAt(i + 1),
                  path.elementAt(i + 2));
            qreal blen = b.length();
            curLen += blen;

            if (i + 2 == lastElement || curLen / totalLength >= t) {
               *bezierLength = blen;
               return b;
            }

            i += 2;
            break;
         }
         default:
            break;
      }
      *startingLength = curLen;
   }
   return QBezier();
}
QPointF QPainterPath::pointAtPercent(qreal t) const
{
   if (t < 0 || t > 1) {
      qWarning("QPainterPath::pointAtPercent() Only values between 0 and 1 are valid");
      return QPointF();
   }

   if (!d_ptr || d_ptr->elements.size() == 0) {
      return QPointF();
   }

   if (d_ptr->elements.size() == 1) {
      return d_ptr->elements.at(0);
   }

   qreal totalLength = length();
   qreal curLen = 0;
   qreal bezierLen = 0;
   QBezier b = bezierAtT(*this, t, &curLen, &bezierLen);
   qreal realT = (totalLength * t - curLen) / bezierLen;

   return b.pointAt(qBound(qreal(0), realT, qreal(1)));
}

qreal QPainterPath::angleAtPercent(qreal t) const
{
   if (t < 0 || t > 1) {
      qWarning("QPainterPath::angleAtPercent() Only values between 0 and 1 are valid");
      return 0;
   }

   qreal totalLength = length();
   qreal curLen = 0;
   qreal bezierLen = 0;
   QBezier bez = bezierAtT(*this, t, &curLen, &bezierLen);
   qreal realT = (totalLength * t - curLen) / bezierLen;

   qreal m1 = slopeAt(realT, bez.x1, bez.x2, bez.x3, bez.x4);
   qreal m2 = slopeAt(realT, bez.y1, bez.y2, bez.y3, bez.y4);

   return QLineF(0, 0, m1, m2).angle();
}

qreal QPainterPath::slopeAtPercent(qreal t) const
{
   if (t < 0 || t > 1) {
      qWarning("QPainterPath::slopeAtPercent() Only values between 0 and 1 are valid");
      return 0;
   }

   qreal totalLength = length();
   qreal curLen = 0;
   qreal bezierLen = 0;
   QBezier bez = bezierAtT(*this, t, &curLen, &bezierLen);
   qreal realT = (totalLength * t - curLen) / bezierLen;

   qreal m1 = slopeAt(realT, bez.x1, bez.x2, bez.x3, bez.x4);
   qreal m2 = slopeAt(realT, bez.y1, bez.y2, bez.y3, bez.y4);
   //tangent line
   qreal slope = 0;

    if (m1)
        slope = m2/m1;
    else {
        if (std::numeric_limits<qreal>::has_infinity) {
            slope = (m2  < 0) ? -std::numeric_limits<qreal>::infinity()
                              : std::numeric_limits<qreal>::infinity();
        } else {
            if (sizeof(qreal) == sizeof(double)) {
                return 1.79769313486231570e+308;
            } else {
                return ((qreal)3.40282346638528860e+38);
            }
        }
   }

   return slope;
}

void QPainterPath::addRoundedRect(const QRectF &rect, qreal xRadius, qreal yRadius,
   Qt::SizeMode mode)
{
   QRectF r = rect.normalized();

   if (r.isNull()) {
      return;
   }

   if (mode == Qt::AbsoluteSize) {
      qreal w = r.width() / 2;
      qreal h = r.height() / 2;

      if (w == 0) {
         xRadius = 0;
      } else {
         xRadius = 100 * qMin(xRadius, w) / w;
      }
      if (h == 0) {
         yRadius = 0;
      } else {
         yRadius = 100 * qMin(yRadius, h) / h;
      }
   } else {
      if (xRadius > 100) {                        // fix ranges
         xRadius = 100;
      }

      if (yRadius > 100) {
         yRadius = 100;
      }
   }

   if (xRadius <= 0 || yRadius <= 0) {             // add normal rectangle
      addRect(r);
      return;
   }

   qreal x = r.x();
   qreal y = r.y();
   qreal w = r.width();
   qreal h = r.height();
   qreal rxx2 = w * xRadius / 100;
   qreal ryy2 = h * yRadius / 100;

   ensureData();
   detach();

   bool first = d_func()->elements.size() < 2;

   arcMoveTo(x, y, rxx2, ryy2, 180);
   arcTo(x, y, rxx2, ryy2, 180, -90);
   arcTo(x + w - rxx2, y, rxx2, ryy2, 90, -90);
   arcTo(x + w - rxx2, y + h - ryy2, rxx2, ryy2, 0, -90);
   arcTo(x, y + h - ryy2, rxx2, ryy2, 270, -90);
   closeSubpath();

   d_func()->require_moveTo = true;
   d_func()->convex = first;
}

void QPainterPath::addRoundRect(const QRectF &r, int xRnd, int yRnd)
{
   if (xRnd >= 100) {                       // fix ranges
      xRnd = 99;
   }
   if (yRnd >= 100) {
      yRnd = 99;
   }
   if (xRnd <= 0 || yRnd <= 0) {            // add normal rectangle
      addRect(r);
      return;
   }

   QRectF rect = r.normalized();

   if (rect.isNull()) {
      return;
   }

   qreal x = rect.x();
   qreal y = rect.y();
   qreal w = rect.width();
   qreal h = rect.height();
   qreal rxx2 = w * xRnd / 100;
   qreal ryy2 = h * yRnd / 100;

   ensureData();
   detach();

   bool first = d_func()->elements.size() < 2;

   arcMoveTo(x, y, rxx2, ryy2, 180);
   arcTo(x, y, rxx2, ryy2, 180, -90);
   arcTo(x + w - rxx2, y, rxx2, ryy2, 90, -90);
   arcTo(x + w - rxx2, y + h - ryy2, rxx2, ryy2, 0, -90);
   arcTo(x, y + h - ryy2, rxx2, ryy2, 270, -90);
   closeSubpath();

   d_func()->require_moveTo = true;
   d_func()->convex = first;
}

QPainterPath QPainterPath::united(const QPainterPath &p) const
{
   if (isEmpty() || p.isEmpty()) {
      return isEmpty() ? p : *this;
   }
   QPathClipper clipper(*this, p);
   return clipper.clip(QPathClipper::BoolOr);
}

QPainterPath QPainterPath::intersected(const QPainterPath &p) const
{
   if (isEmpty() || p.isEmpty()) {
      return QPainterPath();
   }
   QPathClipper clipper(*this, p);
   return clipper.clip(QPathClipper::BoolAnd);
}

QPainterPath QPainterPath::subtracted(const QPainterPath &p) const
{
   if (isEmpty() || p.isEmpty()) {
      return *this;
   }
   QPathClipper clipper(*this, p);
   return clipper.clip(QPathClipper::BoolSub);
}

QPainterPath QPainterPath::subtractedInverted(const QPainterPath &p) const
{
   return p.subtracted(*this);
}

QPainterPath QPainterPath::simplified() const
{
   if (isEmpty()) {
      return *this;
   }
   QPathClipper clipper(*this, QPainterPath());
   return clipper.clip(QPathClipper::Simplify);
}

bool QPainterPath::intersects(const QPainterPath &p) const
{
   if (p.elementCount() == 1) {
      return contains(p.elementAt(0));
   }
   if (isEmpty() || p.isEmpty()) {
      return false;
   }
   QPathClipper clipper(*this, p);
   return clipper.intersect();
}

bool QPainterPath::contains(const QPainterPath &p) const
{
   if (p.elementCount() == 1) {
      return contains(p.elementAt(0));
   }
   if (isEmpty() || p.isEmpty()) {
      return false;
   }
   QPathClipper clipper(*this, p);
   return clipper.contains();
}

void QPainterPath::setDirty(bool dirty)
{
   d_func()->dirtyBounds        = dirty;
   d_func()->dirtyControlBounds = dirty;

   delete d_func()->pathConverter;
   d_func()->pathConverter = nullptr;
   d_func()->convex        = false;
}

void QPainterPath::computeBoundingRect() const
{
   QPainterPathData *d = d_func();
   d->dirtyBounds = false;
   if (!d_ptr) {
      d->bounds = QRect();
      return;
   }

   qreal minx, maxx, miny, maxy;
   minx = maxx = d->elements.at(0).x;
   miny = maxy = d->elements.at(0).y;
   for (int i = 1; i < d->elements.size(); ++i) {
      const Element &e = d->elements.at(i);

      switch (e.type) {
         case MoveToElement:
         case LineToElement:
            if (e.x > maxx) {
               maxx = e.x;
            } else if (e.x < minx) {
               minx = e.x;
            }
            if (e.y > maxy) {
               maxy = e.y;
            } else if (e.y < miny) {
               miny = e.y;
            }
            break;
         case CurveToElement: {
            QBezier b = QBezier::fromPoints(d->elements.at(i - 1),
                  e,
                  d->elements.at(i + 1),
                  d->elements.at(i + 2));
            QRectF r = qt_painterpath_bezier_extrema(b);
            qreal right = r.right();
            qreal bottom = r.bottom();
            if (r.x() < minx) {
               minx = r.x();
            }
            if (right > maxx) {
               maxx = right;
            }
            if (r.y() < miny) {
               miny = r.y();
            }
            if (bottom > maxy) {
               maxy = bottom;
            }
            i += 2;
         }
         break;
         default:
            break;
      }
   }
   d->bounds = QRectF(minx, miny, maxx - minx, maxy - miny);
}

void QPainterPath::computeControlPointRect() const
{
   QPainterPathData *d = d_func();
   d->dirtyControlBounds = false;

   if (!d_ptr) {
      d->controlBounds = QRect();
      return;
   }

   qreal minx, maxx, miny, maxy;
   minx = maxx = d->elements.at(0).x;
   miny = maxy = d->elements.at(0).y;

   for (int i = 1; i < d->elements.size(); ++i) {
      const Element &e = d->elements.at(i);
      if (e.x > maxx) {
         maxx = e.x;
      } else if (e.x < minx) {
         minx = e.x;
      }
      if (e.y > maxy) {
         maxy = e.y;
      } else if (e.y < miny) {
         miny = e.y;
      }
   }
   d->controlBounds = QRectF(minx, miny, maxx - minx, maxy - miny);
}

QDebug operator<<(QDebug debug, const QPainterPath &p)
{
   debug.nospace() << "QPainterPath() Element count =" << p.elementCount() << endl;
   const char *types[] = {"MoveTo", "LineTo", "CurveTo", "CurveToData"};

   for (int i = 0; i < p.elementCount(); ++i) {
      debug.nospace() << " -> " << types[p.elementAt(i).type]
            << "(x =" << p.elementAt(i).x << ", y =" << p.elementAt(i).y << ")\n";
   }

   return debug;
}
