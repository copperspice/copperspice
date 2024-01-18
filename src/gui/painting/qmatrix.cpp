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

#include <qmatrix.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qhashfunc.h>
#include <qmath.h>
#include <qregion.h>
#include <qpainterpath.h>
#include <qvariant.h>

#include <qpainterpath_p.h>

#include <limits.h>

// some defines to inline some code
#define MAPDOUBLE(x, y, nx, ny) \
{ \
    qreal fx = x; \
    qreal fy = y; \
    nx = _m11*fx + _m21*fy + _dx; \
    ny = _m12*fx + _m22*fy + _dy; \
}

#define MAPINT(x, y, nx, ny) \
{ \
    qreal fx = x; \
    qreal fy = y; \
    nx = qRound(_m11*fx + _m21*fy + _dx); \
    ny = qRound(_m12*fx + _m22*fy + _dy); \
}

QMatrix::QMatrix()
   : _m11(1.), _m12(0.), _m21(0.), _m22(1.), _dx(0.), _dy(0.)
{
}

QMatrix::QMatrix(qreal m11, qreal m12, qreal m21, qreal m22, qreal dx, qreal dy)
   : _m11(m11), _m12(m12), _m21(m21), _m22(m22), _dx(dx), _dy(dy)
{
}

QMatrix::QMatrix(const QMatrix &matrix)
   : _m11(matrix._m11), _m12(matrix._m12), _m21(matrix._m21), _m22(matrix._m22), _dx(matrix._dx), _dy(matrix._dy)
{
}

void QMatrix::setMatrix(qreal m11, qreal m12, qreal m21, qreal m22, qreal dx, qreal dy)
{
   _m11 = m11;
   _m12 = m12;
   _m21 = m21;
   _m22 = m22;
   _dx  = dx;
   _dy  = dy;
}

void QMatrix::map(qreal x, qreal y, qreal *tx, qreal *ty) const
{
   MAPDOUBLE(x, y, *tx, *ty);
}

void QMatrix::map(int x, int y, int *tx, int *ty) const
{
   MAPINT(x, y, *tx, *ty);
}

QRect QMatrix::mapRect(const QRect &rect) const
{
   QRect result;

   if (_m12 == 0.0F && _m21 == 0.0F) {
      int x = qRound(_m11 * rect.x() + _dx);
      int y = qRound(_m22 * rect.y() + _dy);
      int w = qRound(_m11 * rect.width());
      int h = qRound(_m22 * rect.height());

      if (w < 0) {
         w = -w;
         x -= w;
      }

      if (h < 0) {
         h = -h;
         y -= h;
      }

      result = QRect(x, y, w, h);

   } else {
      // see mapToPolygon for explanations of the algorithm.
      qreal x0, y0;
      qreal x, y;

      MAPDOUBLE(rect.left(), rect.top(), x0, y0);
      qreal xmin = x0;
      qreal ymin = y0;
      qreal xmax = x0;
      qreal ymax = y0;

      MAPDOUBLE(rect.right() + 1, rect.top(), x, y);
      xmin = qMin(xmin, x);
      ymin = qMin(ymin, y);
      xmax = qMax(xmax, x);
      ymax = qMax(ymax, y);

      MAPDOUBLE(rect.right() + 1, rect.bottom() + 1, x, y);
      xmin = qMin(xmin, x);
      ymin = qMin(ymin, y);
      xmax = qMax(xmax, x);
      ymax = qMax(ymax, y);

      MAPDOUBLE(rect.left(), rect.bottom() + 1, x, y);
      xmin = qMin(xmin, x);
      ymin = qMin(ymin, y);
      xmax = qMax(xmax, x);
      ymax = qMax(ymax, y);
      result = QRect(qRound(xmin), qRound(ymin), qRound(xmax) - qRound(xmin), qRound(ymax) - qRound(ymin));
   }

   return result;
}

QRectF QMatrix::mapRect(const QRectF &rect) const
{
   QRectF result;

   if (_m12 == 0.0F && _m21 == 0.0F) {
      qreal x = _m11 * rect.x() + _dx;
      qreal y = _m22 * rect.y() + _dy;
      qreal w = _m11 * rect.width();
      qreal h = _m22 * rect.height();

      if (w < 0) {
         w = -w;
         x -= w;
      }

      if (h < 0) {
         h = -h;
         y -= h;
      }

      result = QRectF(x, y, w, h);

   } else {
      qreal x0, y0;
      qreal x, y;

      MAPDOUBLE(rect.x(), rect.y(), x0, y0);
      qreal xmin = x0;
      qreal ymin = y0;
      qreal xmax = x0;
      qreal ymax = y0;

      MAPDOUBLE(rect.x() + rect.width(), rect.y(), x, y);
      xmin = qMin(xmin, x);
      ymin = qMin(ymin, y);
      xmax = qMax(xmax, x);
      ymax = qMax(ymax, y);

      MAPDOUBLE(rect.x() + rect.width(), rect.y() + rect.height(), x, y);
      xmin = qMin(xmin, x);
      ymin = qMin(ymin, y);
      xmax = qMax(xmax, x);
      ymax = qMax(ymax, y);

      MAPDOUBLE(rect.x(), rect.y() + rect.height(), x, y);
      xmin = qMin(xmin, x);
      ymin = qMin(ymin, y);
      xmax = qMax(xmax, x);
      ymax = qMax(ymax, y);
      result = QRectF(xmin, ymin, xmax - xmin, ymax - ymin);
   }

   return result;
}

QPoint QMatrix::map(const QPoint &p) const
{
   qreal fx = p.x();
   qreal fy = p.y();

   return QPoint(qRound(_m11 * fx + _m21 * fy + _dx),
      qRound(_m12 * fx + _m22 * fy + _dy));
}

QPointF QMatrix::map(const QPointF &point) const
{
   qreal fx = point.x();
   qreal fy = point.y();
   return QPointF(_m11 * fx + _m21 * fy + _dx, _m12 * fx + _m22 * fy + _dy);
}

QLineF QMatrix::map(const QLineF &line) const
{
   return QLineF(map(line.p1()), map(line.p2()));
}

QLine QMatrix::map(const QLine &line) const
{
   return QLine(map(line.p1()), map(line.p2()));
}

QPolygon QMatrix::map(const QPolygon &a) const
{
   int size = a.size();
   int i;

   QPolygon p(size);
   const QPoint *da = a.constData();

   QPoint *dp = p.data();

   for (i = 0; i < size; i++) {
      MAPINT(da[i].x(), da[i].y(), dp[i].rx(), dp[i].ry());
   }

   return p;
}

QPolygonF QMatrix::map(const QPolygonF &a) const
{
   int size = a.size();
   int i;

   QPolygonF p(size);
   const QPointF *da = a.constData();

   QPointF *dp = p.data();

   for (i = 0; i < size; i++) {
      MAPDOUBLE(da[i].xp, da[i].yp, dp[i].xp, dp[i].yp);
   }

   return p;
}

extern QPainterPath qt_regionToPath(const QRegion &region);

QRegion QMatrix::map(const QRegion &r) const
{
   if (_m11 == 1.0 && _m22 == 1.0 && _m12 == 0.0 && _m21 == 0.0) { // translate or identity
      if (_dx == 0.0 && _dy == 0.0) {
         // Identity
         return r;
      }

      QRegion copy(r);
      copy.translate(qRound(_dx), qRound(_dy));
      return copy;
   }

   QPainterPath p = map(qt_regionToPath(r));
   return p.toFillPolygon().toPolygon();
}

QPainterPath QMatrix::map(const QPainterPath &path) const
{
   if (path.isEmpty()) {
      return QPainterPath();
   }

   QPainterPath copy = path;

   // Translate or identity
   if (_m11 == 1.0 && _m22 == 1.0 && _m12 == 0.0 && _m21 == 0.0) {

      // Translate
      if (_dx != 0.0 || _dy != 0.0) {
         copy.detach();
         for (int i = 0; i < path.elementCount(); ++i) {
            QPainterPath::Element &e = copy.d_ptr->elements[i];
            e.x += _dx;
            e.y += _dy;
         }
      }

      // Full xform
   } else {
      copy.detach();
      for (int i = 0; i < path.elementCount(); ++i) {
         QPainterPath::Element &e = copy.d_ptr->elements[i];
         qreal fx = e.x, fy = e.y;
         e.x = _m11 * fx + _m21 * fy + _dx;
         e.y =  _m12 * fx + _m22 * fy + _dy;
      }
   }

   return copy;
}

QPolygon QMatrix::mapToPolygon(const QRect &rect) const
{
   QPolygon a(4);
   qreal x[4], y[4];

   if (_m12 == 0.0F && _m21 == 0.0F) {
      x[0] = _m11 * rect.x() + _dx;
      y[0] = _m22 * rect.y() + _dy;

      qreal w = _m11 * rect.width();
      qreal h = _m22 * rect.height();

      if (w < 0) {
         w = -w;
         x[0] -= w;
      }

      if (h < 0) {
         h = -h;
         y[0] -= h;
      }
      x[1] = x[0] + w;
      x[2] = x[1];
      x[3] = x[0];
      y[1] = y[0];
      y[2] = y[0] + h;
      y[3] = y[2];

   } else {
      qreal right = rect.x() + rect.width();
      qreal bottom = rect.y() + rect.height();

      MAPDOUBLE(rect.x(), rect.y(), x[0], y[0]);
      MAPDOUBLE(right, rect.y(), x[1], y[1]);
      MAPDOUBLE(right, bottom, x[2], y[2]);
      MAPDOUBLE(rect.x(), bottom, x[3], y[3]);
   }

   // all coordinates are correctly, tranform to a pointarray
   // (rounding to the next integer)
   a.setPoints(4, qRound(x[0]), qRound(y[0]),
      qRound(x[1]), qRound(y[1]),
      qRound(x[2]), qRound(y[2]),
      qRound(x[3]), qRound(y[3]));
   return a;
}

void QMatrix::reset()
{
   _m11 = _m22 = 1.0;
   _m12 = _m21 = _dx = _dy = 0.0;
}

QMatrix &QMatrix::translate(qreal dx, qreal dy)
{
   _dx += dx * _m11 + dy * _m21;
   _dy += dy * _m22 + dx * _m12;
   return *this;
}

QMatrix &QMatrix::scale(qreal sx, qreal sy)
{
   _m11 *= sx;
   _m12 *= sx;
   _m21 *= sy;
   _m22 *= sy;

   return *this;
}

QMatrix &QMatrix::shear(qreal sh, qreal sv)
{
   qreal tm11 = sv * _m21;
   qreal tm12 = sv * _m22;
   qreal tm21 = sh * _m11;
   qreal tm22 = sh * _m12;

   _m11 += tm11;
   _m12 += tm12;
   _m21 += tm21;
   _m22 += tm22;

   return *this;
}

const qreal deg2rad = qreal(0.017453292519943295769);        // pi/180

QMatrix &QMatrix::rotate(qreal a)
{
   qreal sina = 0;
   qreal cosa = 0;

   if (a == 90. || a == -270.) {
      sina = 1.;

   } else if (a == 270. || a == -90.) {
      sina = -1.;

   } else if (a == 180.) {
      cosa = -1.;

   } else {
      qreal b = deg2rad * a;                      // convert to radians
      sina = qSin(b);               // fast and convenient
      cosa = qCos(b);
   }

   qreal tm11 = cosa * _m11 + sina * _m21;
   qreal tm12 = cosa * _m12 + sina * _m22;
   qreal tm21 = -sina * _m11 + cosa * _m21;
   qreal tm22 = -sina * _m12 + cosa * _m22;

   _m11 = tm11;
   _m12 = tm12;
   _m21 = tm21;
   _m22 = tm22;

   return *this;
}

QMatrix QMatrix::inverted(bool *invertible) const
{
   qreal dtr = determinant();
   if (dtr == 0.0) {
      if (invertible) {
         *invertible = false;          // singular matrix
      }
      return QMatrix(true);

   } else {                            // invertible matrix
      if (invertible) {
         *invertible = true;
      }

      qreal dinv = 1.0 / dtr;
      return QMatrix((_m22 * dinv),        (-_m12 * dinv),
            (-_m21 * dinv), (_m11 * dinv),
            ((_m21 * _dy - _m22 * _dx) * dinv),
            ((_m12 * _dx - _m11 * _dy) * dinv),
            true);
   }
}

bool QMatrix::operator==(const QMatrix &m) const
{
   return _m11 == m._m11 &&
      _m12 == m._m12 &&
      _m21 == m._m21 &&
      _m22 == m._m22 &&
      _dx == m._dx &&
      _dy == m._dy;
}

uint qHash(const QMatrix &key, uint seed)
{
   seed = qHash(key.m11(), seed);
   seed = qHash(key.m12(), seed);
   seed = qHash(key.m21(), seed);
   seed = qHash(key.m22(), seed);
   seed = qHash(key.dx(),  seed);
   seed = qHash(key.dy(),  seed);

   return seed;
}

bool QMatrix::operator!=(const QMatrix &m) const
{
   return _m11 != m._m11 ||
      _m12 != m._m12 ||
      _m21 != m._m21 ||
      _m22 != m._m22 ||
      _dx != m._dx ||
      _dy != m._dy;
}

QMatrix &QMatrix::operator *=(const QMatrix &m)
{
   qreal tm11 = _m11 * m._m11 + _m12 * m._m21;
   qreal tm12 = _m11 * m._m12 + _m12 * m._m22;
   qreal tm21 = _m21 * m._m11 + _m22 * m._m21;
   qreal tm22 = _m21 * m._m12 + _m22 * m._m22;

   qreal tdx  = _dx * m._m11  + _dy * m._m21 + m._dx;
   qreal tdy =  _dx * m._m12  + _dy * m._m22 + m._dy;

   _m11 = tm11;
   _m12 = tm12;
   _m21 = tm21;
   _m22 = tm22;
   _dx = tdx;
   _dy = tdy;

   return *this;
}

QMatrix QMatrix::operator *(const QMatrix &m) const
{
   qreal tm11 = _m11 * m._m11 + _m12 * m._m21;
   qreal tm12 = _m11 * m._m12 + _m12 * m._m22;
   qreal tm21 = _m21 * m._m11 + _m22 * m._m21;
   qreal tm22 = _m21 * m._m12 + _m22 * m._m22;

   qreal tdx  = _dx * m._m11  + _dy * m._m21 + m._dx;
   qreal tdy =  _dx * m._m12  + _dy * m._m22 + m._dy;
   return QMatrix(tm11, tm12, tm21, tm22, tdx, tdy, true);
}

QMatrix &QMatrix::operator=(const QMatrix &matrix)
{
   _m11 = matrix._m11;
   _m12 = matrix._m12;
   _m21 = matrix._m21;
   _m22 = matrix._m22;
   _dx  = matrix._dx;
   _dy  = matrix._dy;
   return *this;
}

QMatrix::operator QVariant() const
{
   return QVariant(QVariant::Matrix, this);
}

Q_GUI_EXPORT QPainterPath operator *(const QPainterPath &p, const QMatrix &m)
{
   return m.map(p);
}

QDataStream &operator<<(QDataStream &s, const QMatrix &m)
{
   s << double(m.m11())
      << double(m.m12())
      << double(m.m21())
      << double(m.m22())
      << double(m.dx())
      << double(m.dy());

   return s;
}

QDataStream &operator>>(QDataStream &s, QMatrix &m)
{
   double m11, m12, m21, m22, dx, dy;
   s >> m11;
   s >> m12;
   s >> m21;
   s >> m22;
   s >> dx;
   s >> dy;
   m.setMatrix(m11, m12, m21, m22, dx, dy);

   return s;
}

QDebug operator<<(QDebug dbg, const QMatrix &m)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QMatrix("
      << "11=" << m.m11()
      << " 12=" << m.m12()
      << " 21=" << m.m21()
      << " 22=" << m.m22()
      << " dx=" << m.dx()
      << " dy=" << m.dy()
      << ')';
   return dbg;
}
