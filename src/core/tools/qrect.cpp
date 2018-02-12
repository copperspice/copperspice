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

#include <qrect.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qmath.h>
#include <math.h>

QT_BEGIN_NAMESPACE

QRect QRect::normalized() const
{
   QRect r;
   if (x2 < x1 - 1) {                                // swap bad x values
      r.x1 = x2;
      r.x2 = x1;
   } else {
      r.x1 = x1;
      r.x2 = x2;
   }
   if (y2 < y1 - 1) {                                // swap bad y values
      r.y1 = y2;
      r.y2 = y1;
   } else {
      r.y1 = y1;
      r.y2 = y2;
   }
   return r;
}

void QRect::moveCenter(const QPoint &p)
{
   int w = x2 - x1;
   int h = y2 - y1;
   x1 = p.x() - w / 2;
   y1 = p.y() - h / 2;
   x2 = x1 + w;
   y2 = y1 + h;
}

bool QRect::contains(const QPoint &p, bool proper) const
{
   int l, r;
   if (x2 < x1 - 1) {
      l = x2;
      r = x1;
   } else {
      l = x1;
      r = x2;
   }
   if (proper) {
      if (p.x() <= l || p.x() >= r) {
         return false;
      }
   } else {
      if (p.x() < l || p.x() > r) {
         return false;
      }
   }
   int t, b;
   if (y2 < y1 - 1) {
      t = y2;
      b = y1;
   } else {
      t = y1;
      b = y2;
   }
   if (proper) {
      if (p.y() <= t || p.y() >= b) {
         return false;
      }
   } else {
      if (p.y() < t || p.y() > b) {
         return false;
      }
   }
   return true;
}

bool QRect::contains(const QRect &r, bool proper) const
{
   if (isNull() || r.isNull()) {
      return false;
   }

   int l1 = x1;
   int r1 = x1;
   if (x2 - x1 + 1 < 0) {
      l1 = x2;
   } else {
      r1 = x2;
   }

   int l2 = r.x1;
   int r2 = r.x1;
   if (r.x2 - r.x1 + 1 < 0) {
      l2 = r.x2;
   } else {
      r2 = r.x2;
   }

   if (proper) {
      if (l2 <= l1 || r2 >= r1) {
         return false;
      }
   } else {
      if (l2 < l1 || r2 > r1) {
         return false;
      }
   }

   int t1 = y1;
   int b1 = y1;
   if (y2 - y1 + 1 < 0) {
      t1 = y2;
   } else {
      b1 = y2;
   }

   int t2 = r.y1;
   int b2 = r.y1;
   if (r.y2 - r.y1 + 1 < 0) {
      t2 = r.y2;
   } else {
      b2 = r.y2;
   }

   if (proper) {
      if (t2 <= t1 || b2 >= b1) {
         return false;
      }
   } else {
      if (t2 < t1 || b2 > b1) {
         return false;
      }
   }

   return true;
}

QRect QRect::operator|(const QRect &r) const
{
   if (isNull()) {
      return r;
   }
   if (r.isNull()) {
      return *this;
   }

   int l1 = x1;
   int r1 = x1;
   if (x2 - x1 + 1 < 0) {
      l1 = x2;
   } else {
      r1 = x2;
   }

   int l2 = r.x1;
   int r2 = r.x1;
   if (r.x2 - r.x1 + 1 < 0) {
      l2 = r.x2;
   } else {
      r2 = r.x2;
   }

   int t1 = y1;
   int b1 = y1;
   if (y2 - y1 + 1 < 0) {
      t1 = y2;
   } else {
      b1 = y2;
   }

   int t2 = r.y1;
   int b2 = r.y1;
   if (r.y2 - r.y1 + 1 < 0) {
      t2 = r.y2;
   } else {
      b2 = r.y2;
   }

   QRect tmp;
   tmp.x1 = qMin(l1, l2);
   tmp.x2 = qMax(r1, r2);
   tmp.y1 = qMin(t1, t2);
   tmp.y2 = qMax(b1, b2);
   return tmp;
}

QRect QRect::operator&(const QRect &r) const
{
   if (isNull() || r.isNull()) {
      return QRect();
   }

   int l1 = x1;
   int r1 = x1;
   if (x2 - x1 + 1 < 0) {
      l1 = x2;
   } else {
      r1 = x2;
   }

   int l2 = r.x1;
   int r2 = r.x1;
   if (r.x2 - r.x1 + 1 < 0) {
      l2 = r.x2;
   } else {
      r2 = r.x2;
   }

   if (l1 > r2 || l2 > r1) {
      return QRect();
   }

   int t1 = y1;
   int b1 = y1;
   if (y2 - y1 + 1 < 0) {
      t1 = y2;
   } else {
      b1 = y2;
   }

   int t2 = r.y1;
   int b2 = r.y1;
   if (r.y2 - r.y1 + 1 < 0) {
      t2 = r.y2;
   } else {
      b2 = r.y2;
   }

   if (t1 > b2 || t2 > b1) {
      return QRect();
   }

   QRect tmp;
   tmp.x1 = qMax(l1, l2);
   tmp.x2 = qMin(r1, r2);
   tmp.y1 = qMax(t1, t2);
   tmp.y2 = qMin(b1, b2);
   return tmp;
}

/*!
    \fn QRect QRect::intersect(const QRect &rectangle) const
    \obsolete

    Use intersected(\a rectangle) instead.
*/

/*!
    \fn QRect QRect::intersected(const QRect &rectangle) const
    \since 4.2

    Returns the intersection of this rectangle and the given \a
    rectangle. Note that \c{r.intersected(s)} is equivalent to \c{r & s}.

    \image qrect-intersect.png

    \sa intersects(), united(), operator&=()
*/

/*!
    \fn bool QRect::intersects(const QRect &rectangle) const

    Returns true if this rectangle intersects with the given \a
    rectangle (i.e., there is at least one pixel that is within both
    rectangles), otherwise returns false.

    The intersection rectangle can be retrieved using the intersected()
    function.

    \sa  contains()
*/

bool QRect::intersects(const QRect &r) const
{
   if (isNull() || r.isNull()) {
      return false;
   }

   int l1 = x1;
   int r1 = x1;
   if (x2 - x1 + 1 < 0) {
      l1 = x2;
   } else {
      r1 = x2;
   }

   int l2 = r.x1;
   int r2 = r.x1;
   if (r.x2 - r.x1 + 1 < 0) {
      l2 = r.x2;
   } else {
      r2 = r.x2;
   }

   if (l1 > r2 || l2 > r1) {
      return false;
   }

   int t1 = y1;
   int b1 = y1;
   if (y2 - y1 + 1 < 0) {
      t1 = y2;
   } else {
      b1 = y2;
   }

   int t2 = r.y1;
   int b2 = r.y1;
   if (r.y2 - r.y1 + 1 < 0) {
      t2 = r.y2;
   } else {
      b2 = r.y2;
   }

   if (t1 > b2 || t2 > b1) {
      return false;
   }

   return true;
}

QDataStream &operator<<(QDataStream &s, const QRect &r)
{

   s << (qint32)r.left() << (qint32)r.top()
     << (qint32)r.right() << (qint32)r.bottom();

   return s;
}

QDataStream &operator>>(QDataStream &s, QRect &r)
{
   qint32 x1, y1, x2, y2;
   s >> x1;
   s >> y1;
   s >> x2;
   s >> y2;
   r.setCoords(x1, y1, x2, y2);

   return s;
}


QDebug operator<<(QDebug dbg, const QRect &r)
{
   dbg.nospace() << "QRect(" << r.x() << ',' << r.y() << ' '
                 << r.width() << 'x' << r.height() << ')';
   return dbg.space();
}

QRectF QRectF::normalized() const
{
   QRectF r = *this;
   if (r.w < 0) {
      r.xp += r.w;
      r.w = -r.w;
   }
   if (r.h < 0) {
      r.yp += r.h;
      r.h = -r.h;
   }
   return r;
}

bool QRectF::contains(const QPointF &p) const
{
   qreal l = xp;
   qreal r = xp;
   if (w < 0) {
      l += w;
   } else {
      r += w;
   }
   if (l == r) { // null rect
      return false;
   }

   if (p.x() < l || p.x() > r) {
      return false;
   }

   qreal t = yp;
   qreal b = yp;
   if (h < 0) {
      t += h;
   } else {
      b += h;
   }
   if (t == b) { // null rect
      return false;
   }

   if (p.y() < t || p.y() > b) {
      return false;
   }

   return true;
}


/*!
    \fn bool QRectF::contains(qreal x, qreal y) const
    \overload

    Returns true if the point (\a x, \a y) is inside or on the edge of
    the rectangle; otherwise returns false.
*/

/*!
    \fn bool QRectF::contains(const QRectF &rectangle) const
    \overload

    Returns true if the given \a rectangle is inside this rectangle;
    otherwise returns false.
*/

bool QRectF::contains(const QRectF &r) const
{
   qreal l1 = xp;
   qreal r1 = xp;
   if (w < 0) {
      l1 += w;
   } else {
      r1 += w;
   }
   if (l1 == r1) { // null rect
      return false;
   }

   qreal l2 = r.xp;
   qreal r2 = r.xp;
   if (r.w < 0) {
      l2 += r.w;
   } else {
      r2 += r.w;
   }
   if (l2 == r2) { // null rect
      return false;
   }

   if (l2 < l1 || r2 > r1) {
      return false;
   }

   qreal t1 = yp;
   qreal b1 = yp;
   if (h < 0) {
      t1 += h;
   } else {
      b1 += h;
   }
   if (t1 == b1) { // null rect
      return false;
   }

   qreal t2 = r.yp;
   qreal b2 = r.yp;
   if (r.h < 0) {
      t2 += r.h;
   } else {
      b2 += r.h;
   }
   if (t2 == b2) { // null rect
      return false;
   }

   if (t2 < t1 || b2 > b1) {
      return false;
   }

   return true;
}

/*!
    \fn qreal QRectF::left() const

    Returns the x-coordinate of the rectangle's left edge. Equivalent
    to x().

    \sa setLeft(), topLeft(), bottomLeft()
*/

/*!
    \fn qreal QRectF::top() const

    Returns the y-coordinate of the rectangle's top edge. Equivalent
    to y().

    \sa setTop(), topLeft(), topRight()
*/

/*!
    \fn qreal QRectF::right() const

    Returns the x-coordinate of the rectangle's right edge.

    \sa setRight(), topRight(), bottomRight()
*/

/*!
    \fn qreal QRectF::bottom() const

    Returns the y-coordinate of the rectangle's bottom edge.

    \sa setBottom(), bottomLeft(), bottomRight()
*/

/*!
    \fn QPointF QRectF::topLeft() const

    Returns the position of the rectangle's top-left corner.

    \sa setTopLeft(), top(), left()
*/

/*!
    \fn QPointF QRectF::bottomRight() const

    Returns the position of the rectangle's  bottom-right corner.

    \sa setBottomRight(), bottom(), right()
*/

/*!
    \fn QPointF QRectF::topRight() const

    Returns the position of the rectangle's top-right corner.

    \sa setTopRight(), top(), right()
*/

/*!
    \fn QPointF QRectF::bottomLeft() const

    Returns the position of the rectangle's  bottom-left corner.

    \sa setBottomLeft(),  bottom(), left()
*/

/*!
    \fn QRectF& QRectF::operator|=(const QRectF &rectangle)

    Unites this rectangle with the given \a rectangle.

    \sa united(), operator|()
*/

/*!
    \fn QRectF& QRectF::operator&=(const QRectF &rectangle)

    Intersects this rectangle with the given \a rectangle.

    \sa intersected(), operator|=()
*/


/*!
    \fn QRectF QRectF::operator|(const QRectF &rectangle) const

    Returns the bounding rectangle of this rectangle and the given \a rectangle.

    \sa united(), operator|=()
*/

QRectF QRectF::operator|(const QRectF &r) const
{
   if (isNull()) {
      return r;
   }
   if (r.isNull()) {
      return *this;
   }

   qreal left = xp;
   qreal right = xp;
   if (w < 0) {
      left += w;
   } else {
      right += w;
   }

   if (r.w < 0) {
      left = qMin(left, r.xp + r.w);
      right = qMax(right, r.xp);
   } else {
      left = qMin(left, r.xp);
      right = qMax(right, r.xp + r.w);
   }

   qreal top = yp;
   qreal bottom = yp;
   if (h < 0) {
      top += h;
   } else {
      bottom += h;
   }

   if (r.h < 0) {
      top = qMin(top, r.yp + r.h);
      bottom = qMax(bottom, r.yp);
   } else {
      top = qMin(top, r.yp);
      bottom = qMax(bottom, r.yp + r.h);
   }

   return QRectF(left, top, right - left, bottom - top);
}

/*!
    \fn QRectF QRectF::unite(const QRectF &rectangle) const
    \obsolete

    Use united(\a rectangle) instead.
*/

/*!
    \fn QRectF QRectF::united(const QRectF &rectangle) const
    \since 4.2

    Returns the bounding rectangle of this rectangle and the given \a
    rectangle.

    \image qrect-unite.png

    \sa intersected()
*/


/*!
    \fn QRectF QRectF::operator &(const QRectF &rectangle) const

    Returns the intersection of this rectangle and the given \a
    rectangle. Returns an empty rectangle if there is no intersection.

    \sa operator&=(), intersected()
*/

QRectF QRectF::operator&(const QRectF &r) const
{
   qreal l1 = xp;
   qreal r1 = xp;
   if (w < 0) {
      l1 += w;
   } else {
      r1 += w;
   }
   if (l1 == r1) { // null rect
      return QRectF();
   }

   qreal l2 = r.xp;
   qreal r2 = r.xp;
   if (r.w < 0) {
      l2 += r.w;
   } else {
      r2 += r.w;
   }
   if (l2 == r2) { // null rect
      return QRectF();
   }

   if (l1 >= r2 || l2 >= r1) {
      return QRectF();
   }

   qreal t1 = yp;
   qreal b1 = yp;
   if (h < 0) {
      t1 += h;
   } else {
      b1 += h;
   }
   if (t1 == b1) { // null rect
      return QRectF();
   }

   qreal t2 = r.yp;
   qreal b2 = r.yp;
   if (r.h < 0) {
      t2 += r.h;
   } else {
      b2 += r.h;
   }
   if (t2 == b2) { // null rect
      return QRectF();
   }

   if (t1 >= b2 || t2 >= b1) {
      return QRectF();
   }

   QRectF tmp;
   tmp.xp = qMax(l1, l2);
   tmp.yp = qMax(t1, t2);
   tmp.w = qMin(r1, r2) - tmp.xp;
   tmp.h = qMin(b1, b2) - tmp.yp;
   return tmp;
}

/*!
    \fn QRectF QRectF::intersect(const QRectF &rectangle) const
    \obsolete

    Use intersected(\a rectangle) instead.
*/

/*!
    \fn QRectF QRectF::intersected(const QRectF &rectangle) const
    \since 4.2

    Returns the intersection of this rectangle and the given \a
    rectangle. Note that \c {r.intersected(s)} is equivalent to \c
    {r & s}.

    \image qrect-intersect.png

    \sa intersects(), united(), operator&=()
*/

/*!
    \fn bool QRectF::intersects(const QRectF &rectangle) const

    Returns true if this rectangle intersects with the given \a
    rectangle (i.e. there is a non-empty area of overlap between
    them), otherwise returns false.

    The intersection rectangle can be retrieved using the intersected()
    function.

    \sa contains()
*/

bool QRectF::intersects(const QRectF &r) const
{
   qreal l1 = xp;
   qreal r1 = xp;
   if (w < 0) {
      l1 += w;
   } else {
      r1 += w;
   }
   if (l1 == r1) { // null rect
      return false;
   }

   qreal l2 = r.xp;
   qreal r2 = r.xp;
   if (r.w < 0) {
      l2 += r.w;
   } else {
      r2 += r.w;
   }
   if (l2 == r2) { // null rect
      return false;
   }

   if (l1 >= r2 || l2 >= r1) {
      return false;
   }

   qreal t1 = yp;
   qreal b1 = yp;
   if (h < 0) {
      t1 += h;
   } else {
      b1 += h;
   }
   if (t1 == b1) { // null rect
      return false;
   }

   qreal t2 = r.yp;
   qreal b2 = r.yp;
   if (r.h < 0) {
      t2 += r.h;
   } else {
      b2 += r.h;
   }
   if (t2 == b2) { // null rect
      return false;
   }

   if (t1 >= b2 || t2 >= b1) {
      return false;
   }

   return true;
}

/*!
    \fn QRect QRectF::toRect() const

    Returns a QRect based on the values of this rectangle.  Note that the
    coordinates in the returned rectangle are rounded to the nearest integer.

    \sa QRectF(), toAlignedRect()
*/

/*!
    \fn QRect QRectF::toAlignedRect() const
    \since 4.3

    Returns a QRect based on the values of this rectangle that is the
    smallest possible integer rectangle that completely contains this
    rectangle.

    \sa toRect()
*/

QRect QRectF::toAlignedRect() const
{
   int xmin = int(qFloor(xp));
   int xmax = int(qCeil(xp + w));
   int ymin = int(qFloor(yp));
   int ymax = int(qCeil(yp + h));
   return QRect(xmin, ymin, xmax - xmin, ymax - ymin);
}


/*****************************************************************************
  QRectF stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QRectF &rectangle)

    \relates QRectF

    Writes the \a rectangle to the \a stream, and returns a reference to the
    stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &s, const QRectF &r)
{
   s << double(r.x()) << double(r.y()) << double(r.width()) << double(r.height());
   return s;
}

QDataStream &operator>>(QDataStream &s, QRectF &r)
{
   double x, y, w, h;
   s >> x;
   s >> y;
   s >> w;
   s >> h;
   r.setRect(qreal(x), qreal(y), qreal(w), qreal(h));
   return s;
}

#endif // QT_NO_DATASTREAM


QDebug operator<<(QDebug dbg, const QRectF &r)
{
   dbg.nospace() << "QRectF(" << r.x() << ',' << r.y() << ' '
                 << r.width() << 'x' << r.height() << ')';
   return dbg.space();
}

QT_END_NAMESPACE
