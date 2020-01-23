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

#include <qpolygon.h>
#include <qrect.h>
#include <qdatastream.h>
#include <qmatrix.h>
#include <qdebug.h>
#include <qpainterpath.h>
#include <qvariant.h>
#include <qpainterpath_p.h>
#include <qbezier_p.h>

#include <stdarg.h>

//same as qt_painterpath_isect_line in qpainterpath.cpp
static void qt_polygon_isect_line(const QPointF &p1, const QPointF &p2, const QPointF &pos, int *winding)
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



QPolygon::QPolygon(const QRect &r, bool closed)
{
   reserve(closed ? 5 : 4);
   *this << QPoint(r.x(), r.y())
      << QPoint(r.x() + r.width(), r.y())
      << QPoint(r.x() + r.width(), r.y() + r.height())
      << QPoint(r.x(), r.y() + r.height());
   if (closed) {
      *this << QPoint(r.left(), r.top());
   }
}

/*!
    \internal
    Constructs a point array with \a nPoints points, taken from the
    \a points array.

    Equivalent to setPoints(nPoints, points).
*/

QPolygon::QPolygon(int nPoints, const int *points)
{
   setPoints(nPoints, points);
}


/*!
    \fn QPolygon::~QPolygon()

    Destroys the polygon.
*/


/*!
    Translates all points in the polygon by (\a{dx}, \a{dy}).

    \sa translated()
*/

void QPolygon::translate(int dx, int dy)
{
   if (dx == 0 && dy == 0) {
      return;
   }

   QPoint *p = data();
   int i = size();
   QPoint pt(dx, dy);
   while (i--) {
      *p += pt;
      ++p;
   }
}


QPolygon QPolygon::translated(int dx, int dy) const
{
   QPolygon copy(*this);
   copy.translate(dx, dy);
   return copy;
}


void QPolygon::point(int index, int *x, int *y) const
{
   QPoint p = at(index);
   if (x) {
      *x = (int)p.x();
   }
   if (y) {
      *y = (int)p.y();
   }
}


void QPolygon::setPoints(int nPoints, const int *points)
{
   resize(nPoints);
   int i = 0;
   while (nPoints--) {
      setPoint(i++, *points, *(points + 1));
      points += 2;
   }
}



void QPolygon::setPoints(int nPoints, int firstx, int firsty, ...)
{
   va_list ap;
   resize(nPoints);
   setPoint(0, firstx, firsty);
   int i = 0, x, y;
   va_start(ap, firsty);
   while (--nPoints) {
      x = va_arg(ap, int);
      y = va_arg(ap, int);
      setPoint(++i, x, y);
   }
   va_end(ap);
}

/*!
    \overload
    \internal

    Copies \a nPoints points from the \a points coord array into this
    point array, and resizes the point array if \c{index+nPoints}
    exceeds the size of the array.

    \sa setPoint()
*/

void QPolygon::putPoints(int index, int nPoints, const int *points)
{
   if (index + nPoints > size()) {
      resize(index + nPoints);
   }
   int i = index;
   while (nPoints--) {
      setPoint(i++, *points, *(points + 1));
      points += 2;
   }
}


void QPolygon::putPoints(int index, int nPoints, int firstx, int firsty, ...)
{
   va_list ap;
   if (index + nPoints > size()) {
      resize(index + nPoints);
   }
   if (nPoints <= 0) {
      return;
   }
   setPoint(index, firstx, firsty);
   int i = index, x, y;
   va_start(ap, firsty);
   while (--nPoints) {
      x = va_arg(ap, int);
      y = va_arg(ap, int);
      setPoint(++i, x, y);
   }
   va_end(ap);
}


/*!
    \fn void QPolygon::putPoints(int index, int nPoints, const QPolygon &fromPolygon, int fromIndex)
    \overload

    Copies \a nPoints points from the given \a fromIndex ( 0 by
    default) in \a fromPolygon into this polygon, starting at the
    specified \a index. For example:

    \snippet doc/src/snippets/polygon/polygon.cpp 6
*/

void QPolygon::putPoints(int index, int nPoints, const QPolygon &from, int fromIndex)
{
   if (index + nPoints > size()) {
      resize(index + nPoints);
   }
   if (nPoints <= 0) {
      return;
   }
   int n = 0;
   while (n < nPoints) {
      setPoint(index + n, from[fromIndex + n]);
      ++n;
   }
}


/*!
    Returns the bounding rectangle of the polygon, or QRect(0, 0, 0,
    0) if the polygon is empty.

    \sa QVector::isEmpty()
*/

QRect QPolygon::boundingRect() const
{
   if (isEmpty()) {
      return QRect(0, 0, 0, 0);
   }
   const QPoint *pd = constData();
   int minx, maxx, miny, maxy;
   minx = maxx = pd->x();
   miny = maxy = pd->y();
   ++pd;
   for (int i = 1; i < size(); ++i) {
      if (pd->x() < minx) {
         minx = pd->x();
      } else if (pd->x() > maxx) {
         maxx = pd->x();
      }
      if (pd->y() < miny) {
         miny = pd->y();
      } else if (pd->y() > maxy) {
         maxy = pd->y();
      }
      ++pd;
   }
   return QRect(QPoint(minx, miny), QPoint(maxx, maxy));
}

QDebug operator<<(QDebug dbg, const QPolygon &a)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QPolygon(";

   for (int i = 0; i < a.count(); ++i) {
      dbg.nospace() << a.at(i);
   }

   dbg.nospace() << ')';
   return dbg;
}

QPolygonF::QPolygonF(const QRectF &r)
{
   reserve(5);
   append(QPointF(r.x(), r.y()));
   append(QPointF(r.x() + r.width(), r.y()));
   append(QPointF(r.x() + r.width(), r.y() + r.height()));
   append(QPointF(r.x(), r.y() + r.height()));
   append(QPointF(r.x(), r.y()));
}


QPolygonF::QPolygonF(const QPolygon &a)
{
   reserve(a.size());
   for (int i = 0; i < a.size(); ++i) {
      append(a.at(i));
   }
}


void QPolygonF::translate(const QPointF &offset)
{
   if (offset.isNull()) {
      return;
   }

   QPointF *p = data();
   int i = size();
   while (i--) {
      *p += offset;
      ++p;
   }
}


QPolygonF QPolygonF::translated(const QPointF &offset) const
{
   QPolygonF copy(*this);
   copy.translate(offset);
   return copy;
}


QRectF QPolygonF::boundingRect() const
{
   if (isEmpty()) {
      return QRectF(0, 0, 0, 0);
   }

   const QPointF *pd = constData();
   qreal minx, maxx, miny, maxy;
   minx = maxx = pd->x();
   miny = maxy = pd->y();
   ++pd;

   for (int i = 1; i < size(); ++i) {
      if (pd->x() < minx) {
         minx = pd->x();
      } else if (pd->x() > maxx) {
         maxx = pd->x();
      }
      if (pd->y() < miny) {
         miny = pd->y();
      } else if (pd->y() > maxy) {
         maxy = pd->y();
      }
      ++pd;
   }
   return QRectF(minx, miny, maxx - minx, maxy - miny);
}

/*!
    Creates and returns a QPolygon by converting each QPointF to a
    QPoint.

    \sa QPointF::toPoint()
*/

QPolygon QPolygonF::toPolygon() const
{
   QPolygon a;
   a.reserve(size());
   for (int i = 0; i < size(); ++i) {
      a.append(at(i).toPoint());
   }
   return a;
}

QPolygon::operator QVariant() const
{
   return QVariant(QVariant::Polygon, this);
}

QDataStream &operator<<(QDataStream &s, const QPolygon &a)
{
   const QVector<QPoint> &v = a;
   return s << v;
}


QDataStream &operator>>(QDataStream &s, QPolygon &a)
{
   QVector<QPoint> &v = a;
   return s >> v;
}

QDataStream &operator<<(QDataStream &s, const QPolygonF &a)
{
   quint32 len = a.size();
   uint i;

   s << len;
   for (i = 0; i < len; ++i) {
      s << a.at(i);
   }
   return s;
}

QDataStream &operator>>(QDataStream &s, QPolygonF &a)
{
   quint32 len;
   uint i;

   s >> len;
   a.reserve(a.size() + (int)len);
   QPointF p;
   for (i = 0; i < len; ++i) {
      s >> p;
      a.insert(i, p);
   }
   return s;
}

QDebug operator<<(QDebug dbg, const QPolygonF &a)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QPolygonF(";

   for (int i = 0; i < a.count(); ++i) {
      dbg.nospace() << a.at(i);
   }
   dbg.nospace() << ')';

   return dbg;
}

bool QPolygonF::containsPoint(const QPointF &pt, Qt::FillRule fillRule) const
{
   if (isEmpty()) {
      return false;
   }

   int winding_number = 0;

   QPointF last_pt = at(0);
   QPointF last_start = at(0);
   for (int i = 1; i < size(); ++i) {
      const QPointF &e = at(i);
      qt_polygon_isect_line(last_pt, e, pt, &winding_number);
      last_pt = e;
   }

   // implicitly close last subpath
   if (last_pt != last_start) {
      qt_polygon_isect_line(last_pt, last_start, pt, &winding_number);
   }

   return (fillRule == Qt::WindingFill
         ? (winding_number != 0)
         : ((winding_number % 2) != 0));
}

/*!
    \since 4.3

    \fn bool QPolygon::containsPoint(const QPoint &point, Qt::FillRule fillRule) const
    Returns true if the given \a point is inside the polygon according to
    the specified \a fillRule; otherwise returns false.
*/
bool QPolygon::containsPoint(const QPoint &pt, Qt::FillRule fillRule) const
{
   if (isEmpty()) {
      return false;
   }

   int winding_number = 0;

   QPoint last_pt = at(0);
   QPoint last_start = at(0);
   for (int i = 1; i < size(); ++i) {
      const QPoint &e = at(i);
      qt_polygon_isect_line(last_pt, e, pt, &winding_number);
      last_pt = e;
   }

   // implicitly close last subpath
   if (last_pt != last_start) {
      qt_polygon_isect_line(last_pt, last_start, pt, &winding_number);
   }

   return (fillRule == Qt::WindingFill
         ? (winding_number != 0)
         : ((winding_number % 2) != 0));
}

/*!
    \since 4.3

    Returns a polygon which is the union of this polygon and \a r.

    Set operations on polygons, will treat the polygons as areas, and
    implicitly close the polygon.

    \sa intersected(), subtracted()
*/

QPolygon QPolygon::united(const QPolygon &r) const
{
   QPainterPath subject;
   subject.addPolygon(*this);

   QPainterPath clip;
   clip.addPolygon(r);

   return subject.united(clip).toFillPolygon().toPolygon();
}




QPolygon QPolygon::intersected(const QPolygon &r) const
{
   QPainterPath subject;
   subject.addPolygon(*this);

   QPainterPath clip;
   clip.addPolygon(r);

   return subject.intersected(clip).toFillPolygon().toPolygon();
}

QPolygon QPolygon::subtracted(const QPolygon &r) const
{
   QPainterPath subject;
   subject.addPolygon(*this);

   QPainterPath clip;
   clip.addPolygon(r);

   return subject.subtracted(clip).toFillPolygon().toPolygon();
}

QPolygonF QPolygonF::united(const QPolygonF &r) const
{
   QPainterPath subject;
   subject.addPolygon(*this);

   QPainterPath clip;
   clip.addPolygon(r);

   return subject.united(clip).toFillPolygon();
}

QPolygonF QPolygonF::intersected(const QPolygonF &r) const
{
   QPainterPath subject;
   subject.addPolygon(*this);

   QPainterPath clip;
   clip.addPolygon(r);

   return subject.intersected(clip).toFillPolygon();
}

QPolygonF QPolygonF::subtracted(const QPolygonF &r) const
{
   QPainterPath subject;
   subject.addPolygon(*this);

   QPainterPath clip;
   clip.addPolygon(r);
   return subject.subtracted(clip).toFillPolygon();
}

QPolygonF::operator QVariant() const
{
   return QVariant(QMetaType::QPolygonF, this);
}

