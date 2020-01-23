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

#ifndef QRECT_H
#define QRECT_H

#include <qmargins.h>
#include <qsize.h>
#include <qpoint.h>

#ifdef topLeft
#error qrect.h must be included before any header file that defines topLeft
#endif

QT_BEGIN_NAMESPACE

class Q_CORE_EXPORT QRect
{
 public:
   QRect() {
      x1 = 0;
      y1 = 0;

      x2 = -1;
      y2 = -1;
   }

   QRect(const QPoint &topleft, const QPoint &bottomright);
   QRect(const QPoint &topleft, const QSize &size);
   QRect(int left, int top, int width, int height);

   inline bool isNull() const;
   inline bool isEmpty() const;
   inline bool isValid() const;

   inline int left() const;
   inline int top() const;
   inline int right() const;
   inline int bottom() const;

   QRect normalized() const;

   inline int x() const;
   inline int y() const;

   inline void setLeft(int pos);
   inline void setTop(int pos);
   inline void setRight(int pos);
   inline void setBottom(int pos);

   inline void setX(int x);
   inline void setY(int y);

   inline void setTopLeft(const QPoint &p);
   inline void setBottomRight(const QPoint &p);
   inline void setTopRight(const QPoint &p);
   inline void setBottomLeft(const QPoint &p);

   inline QPoint topLeft() const;
   inline QPoint bottomRight() const;
   inline QPoint topRight() const;
   inline QPoint bottomLeft() const;

   inline QPoint center() const;

   inline void moveLeft(int pos);
   inline void moveTop(int pos);
   inline void moveRight(int pos);
   inline void moveBottom(int pos);
   inline void moveTopLeft(const QPoint &p);
   inline void moveBottomRight(const QPoint &p);
   inline void moveTopRight(const QPoint &p);
   inline void moveBottomLeft(const QPoint &p);

   inline void moveCenter(const QPoint &p);

   inline void translate(int dx, int dy);
   inline void translate(const QPoint &p);
   inline QRect translated(int dx, int dy) const;
   inline QRect translated(const QPoint &p) const;

   inline void moveTo(int x, int t);
   inline void moveTo(const QPoint &p);

   inline void setRect(int x, int y, int w, int h);
   inline void getRect(int *x, int *y, int *w, int *h) const;

   inline void setCoords(int x1, int y1, int x2, int y2);
   inline void getCoords(int *x1, int *y1, int *x2, int *y2) const;

   inline void adjust(int x1, int y1, int x2, int y2);
   inline QRect adjusted(int x1, int y1, int x2, int y2) const;

   inline QSize size() const;
   inline int width() const;
   inline int height() const;
   inline void setWidth(int w);
   inline void setHeight(int h);
   inline void setSize(const QSize &s);

   QRect operator|(const QRect &r) const;
   QRect operator&(const QRect &r) const;
   inline QRect &operator|=(const QRect &r);
   inline QRect &operator&=(const QRect &r);

   bool contains(const QRect &r, bool proper = false) const;
   bool contains(const QPoint &p, bool proper = false) const;
   inline bool contains(int x, int y) const;
   inline bool contains(int x, int y, bool proper) const;

   inline QRect united(const QRect &other) const;
   inline QRect intersected(const QRect &other) const;
   bool intersects(const QRect &r) const;

   inline QRect marginsAdded(const QMargins &margins) const;
   inline QRect marginsRemoved(const QMargins &margins) const;
   inline QRect &operator+=(const QMargins &margins);
   inline QRect &operator-=(const QMargins &margins);

   friend inline bool operator==(const QRect &, const QRect &);
   friend inline bool operator!=(const QRect &, const QRect &);

 private:
   int x1;
   int y1;
   int x2;
   int y2;

};
Q_DECLARE_TYPEINFO(QRect, Q_MOVABLE_TYPE);

inline bool operator==(const QRect &, const QRect &);
inline bool operator!=(const QRect &, const QRect &);

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QRect &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QRect &);


inline QRect::QRect(int aleft, int atop, int awidth, int aheight)
{
   x1 = aleft;
   y1 = atop;
   x2 = (aleft + awidth - 1);
   y2 = (atop + aheight - 1);
}

inline QRect::QRect(const QPoint &atopLeft, const QPoint &abottomRight)
{
   x1 = atopLeft.x();
   y1 = atopLeft.y();
   x2 = abottomRight.x();
   y2 = abottomRight.y();
}

inline QRect::QRect(const QPoint &atopLeft, const QSize &asize)
{
   x1 = atopLeft.x();
   y1 = atopLeft.y();
   x2 = (x1 + asize.width() - 1);
   y2 = (y1 + asize.height() - 1);
}

inline bool QRect::isNull() const
{
   return x2 == x1 - 1 && y2 == y1 - 1;
}

inline bool QRect::isEmpty() const
{
   return x1 > x2 || y1 > y2;
}

inline bool QRect::isValid() const
{
   return x1 <= x2 && y1 <= y2;
}

inline int QRect::left() const
{
   return x1;
}

inline int QRect::top() const
{
   return y1;
}

inline int QRect::right() const
{
   return x2;
}

inline int QRect::bottom() const
{
   return y2;
}

inline int QRect::x() const
{
   return x1;
}

inline int QRect::y() const
{
   return y1;
}

inline void QRect::setLeft(int pos)
{
   x1 = pos;
}

inline void QRect::setTop(int pos)
{
   y1 = pos;
}

inline void QRect::setRight(int pos)
{
   x2 = pos;
}

inline void QRect::setBottom(int pos)
{
   y2 = pos;
}

inline void QRect::setTopLeft(const QPoint &p)
{
   x1 = p.x();
   y1 = p.y();
}

inline void QRect::setBottomRight(const QPoint &p)
{
   x2 = p.x();
   y2 = p.y();
}

inline void QRect::setTopRight(const QPoint &p)
{
   x2 = p.x();
   y1 = p.y();
}

inline void QRect::setBottomLeft(const QPoint &p)
{
   x1 = p.x();
   y2 = p.y();
}

inline void QRect::setX(int ax)
{
   x1 = ax;
}

inline void QRect::setY(int ay)
{
   y1 = ay;
}

inline QPoint QRect::topLeft() const
{
   return QPoint(x1, y1);
}

inline QPoint QRect::bottomRight() const
{
   return QPoint(x2, y2);
}

inline QPoint QRect::topRight() const
{
   return QPoint(x2, y1);
}

inline QPoint QRect::bottomLeft() const
{
   return QPoint(x1, y2);
}

inline QPoint QRect::center() const
{
   return QPoint((x1 + x2) / 2, (y1 + y2) / 2);
}

inline int QRect::width() const
{
   return  x2 - x1 + 1;
}

inline int QRect::height() const
{
   return  y2 - y1 + 1;
}

inline QSize QRect::size() const
{
   return QSize(width(), height());
}

inline void QRect::translate(int dx, int dy)
{
   x1 += dx;
   y1 += dy;
   x2 += dx;
   y2 += dy;
}

inline void QRect::translate(const QPoint &p)
{
   x1 += p.x();
   y1 += p.y();
   x2 += p.x();
   y2 += p.y();
}

inline QRect QRect::translated(int dx, int dy) const
{
   return QRect(QPoint(x1 + dx, y1 + dy), QPoint(x2 + dx, y2 + dy));
}

inline QRect QRect::translated(const QPoint &p) const
{
   return QRect(QPoint(x1 + p.x(), y1 + p.y()), QPoint(x2 + p.x(), y2 + p.y()));
}

inline void QRect::moveTo(int ax, int ay)
{
   x2 += ax - x1;
   y2 += ay - y1;
   x1 = ax;
   y1 = ay;
}

inline void QRect::moveTo(const QPoint &p)
{
   x2 += p.x() - x1;
   y2 += p.y() - y1;
   x1 = p.x();
   y1 = p.y();
}

inline void QRect::moveLeft(int pos)
{
   x2 += (pos - x1);
   x1 = pos;
}

inline void QRect::moveTop(int pos)
{
   y2 += (pos - y1);
   y1 = pos;
}

inline void QRect::moveRight(int pos)
{
   x1 += (pos - x2);
   x2 = pos;
}

inline void QRect::moveBottom(int pos)
{
   y1 += (pos - y2);
   y2 = pos;
}

inline void QRect::moveTopLeft(const QPoint &p)
{
   moveLeft(p.x());
   moveTop(p.y());
}

inline void QRect::moveBottomRight(const QPoint &p)
{
   moveRight(p.x());
   moveBottom(p.y());
}

inline void QRect::moveTopRight(const QPoint &p)
{
   moveRight(p.x());
   moveTop(p.y());
}

inline void QRect::moveBottomLeft(const QPoint &p)
{
   moveLeft(p.x());
   moveBottom(p.y());
}

inline void QRect::moveCenter(const QPoint &p)
{
    int w = x2 - x1;
    int h = y2 - y1;
    x1 = p.x() - w/2;
    y1 = p.y() - h/2;
    x2 = x1 + w;
    y2 = y1 + h;
}
inline void QRect::getRect(int *ax, int *ay, int *aw, int *ah) const
{
   *ax = x1;
   *ay = y1;
   *aw = x2 - x1 + 1;
   *ah = y2 - y1 + 1;
}

inline void QRect::setRect(int ax, int ay, int aw, int ah)
{
   x1 = ax;
   y1 = ay;
   x2 = (ax + aw - 1);
   y2 = (ay + ah - 1);
}

inline void QRect::getCoords(int *xp1, int *yp1, int *xp2, int *yp2) const
{
   *xp1 = x1;
   *yp1 = y1;
   *xp2 = x2;
   *yp2 = y2;
}

inline void QRect::setCoords(int xp1, int yp1, int xp2, int yp2)
{
   x1 = xp1;
   y1 = yp1;
   x2 = xp2;
   y2 = yp2;
}

inline QRect QRect::adjusted(int xp1, int yp1, int xp2, int yp2) const
{
   return QRect(QPoint(x1 + xp1, y1 + yp1), QPoint(x2 + xp2, y2 + yp2));
}

inline void QRect::adjust(int dx1, int dy1, int dx2, int dy2)
{
   x1 += dx1;
   y1 += dy1;
   x2 += dx2;
   y2 += dy2;
}

inline void QRect::setWidth(int w)
{
   x2 = (x1 + w - 1);
}

inline void QRect::setHeight(int h)
{
   y2 = (y1 + h - 1);
}

inline void QRect::setSize(const QSize &s)
{
   x2 = (s.width()  + x1 - 1);
   y2 = (s.height() + y1 - 1);
}

inline bool QRect::contains(int ax, int ay, bool aproper) const
{
   return contains(QPoint(ax, ay), aproper);
}

inline bool QRect::contains(int ax, int ay) const
{
   return contains(QPoint(ax, ay), false);
}

inline QRect &QRect::operator|=(const QRect &r)
{
   *this = *this | r;
   return *this;
}

inline QRect &QRect::operator&=(const QRect &r)
{
   *this = *this & r;
   return *this;
}


inline QRect QRect::intersected(const QRect &other) const
{
   return *this & other;
}

inline QRect QRect::united(const QRect &r) const
{
   return *this | r;
}
inline bool operator==(const QRect &r1, const QRect &r2)
{
   return r1.x1 == r2.x1 && r1.x2 == r2.x2 && r1.y1 == r2.y1 && r1.y2 == r2.y2;
}

inline bool operator!=(const QRect &r1, const QRect &r2)
{
   return r1.x1 != r2.x1 || r1.x2 != r2.x2 || r1.y1 != r2.y1 || r1.y2 != r2.y2;
}


inline QRect operator+(const QRect &rectangle, const QMargins &margins)
{
    return QRect(QPoint(rectangle.left() - margins.left(), rectangle.top() - margins.top()),
                 QPoint(rectangle.right() + margins.right(), rectangle.bottom() + margins.bottom()));
}

inline QRect operator+(const QMargins &margins, const QRect &rectangle)
{
    return QRect(QPoint(rectangle.left() - margins.left(), rectangle.top() - margins.top()),
                 QPoint(rectangle.right() + margins.right(), rectangle.bottom() + margins.bottom()));
}

inline QRect operator-(const QRect &lhs, const QMargins &rhs)
{
    return QRect(QPoint(lhs.left() + rhs.left(), lhs.top() + rhs.top()),
                 QPoint(lhs.right() - rhs.right(), lhs.bottom() - rhs.bottom()));
}

inline QRect QRect::marginsAdded(const QMargins &margins) const
{
    return QRect(QPoint(x1 - margins.left(), y1 - margins.top()),
                 QPoint(x2 + margins.right(), y2 + margins.bottom()));
}

inline QRect QRect::marginsRemoved(const QMargins &margins) const
{
    return QRect(QPoint(x1 + margins.left(), y1 + margins.top()),
                 QPoint(x2 - margins.right(), y2 - margins.bottom()));
}

inline QRect &QRect::operator+=(const QMargins &margins)
{
    *this = marginsAdded(margins);
    return *this;
}

inline QRect &QRect::operator-=(const QMargins &margins)
{
    *this = marginsRemoved(margins);
    return *this;
}

Q_CORE_EXPORT QDebug operator<<(QDebug, const QRect &);

class Q_CORE_EXPORT QRectF
{
 public:
   QRectF() : xp(0.), yp(0.), w(0.), h(0.)
   {}

   QRectF(const QPointF &topleft, const QSizeF &size);
   QRectF(const QPointF &topleft, const QPointF &bottomRight);
   QRectF(qreal left, qreal top, qreal width, qreal height);
   QRectF(const QRect &rect);

   inline bool isNull() const;
   inline bool isEmpty() const;
   inline bool isValid() const;
   QRectF normalized() const;

   inline qreal left() const {
      return xp;
   }

   inline qreal top() const {
      return yp;
   }

   inline qreal right() const {
      return xp + w;
   }

   inline qreal bottom() const {
      return yp + h;
   }

   inline qreal x() const;
   inline qreal y() const;

   inline void setLeft(qreal pos);
   inline void setTop(qreal pos);
   inline void setRight(qreal pos);
   inline void setBottom(qreal pos);

   inline void setX(qreal pos) {
      setLeft(pos);
   }

   inline void setY(qreal pos) {
      setTop(pos);
   }

   inline QPointF topLeft() const {
      return QPointF(xp, yp);
   }
   inline QPointF bottomRight() const {
      return QPointF(xp + w, yp + h);
   }
   inline QPointF topRight() const {
      return QPointF(xp + w, yp);
   }
   inline QPointF bottomLeft() const {
      return QPointF(xp, yp + h);
   }
   inline QPointF center() const;

   inline void setTopLeft(const QPointF &p);
   inline void setBottomRight(const QPointF &p);
   inline void setTopRight(const QPointF &p);
   inline void setBottomLeft(const QPointF &p);

   inline void moveLeft(qreal pos);
   inline void moveTop(qreal pos);
   inline void moveRight(qreal pos);
   inline void moveBottom(qreal pos);
   inline void moveTopLeft(const QPointF &p);
   inline void moveBottomRight(const QPointF &p);
   inline void moveTopRight(const QPointF &p);
   inline void moveBottomLeft(const QPointF &p);
   inline void moveCenter(const QPointF &p);

   inline void translate(qreal dx, qreal dy);
   inline void translate(const QPointF &p);

   inline QRectF translated(qreal dx, qreal dy) const;
   inline QRectF translated(const QPointF &p) const;

   inline void moveTo(qreal x, qreal y);
   inline void moveTo(const QPointF &p);

   inline void setRect(qreal x, qreal y, qreal w, qreal h);
   inline void getRect(qreal *x, qreal *y, qreal *w, qreal *h) const;

   inline void setCoords(qreal x1, qreal y1, qreal x2, qreal y2);
   inline void getCoords(qreal *x1, qreal *y1, qreal *x2, qreal *y2) const;

   inline void adjust(qreal x1, qreal y1, qreal x2, qreal y2);
   inline QRectF adjusted(qreal x1, qreal y1, qreal x2, qreal y2) const;

   inline QSizeF size() const;
   inline qreal width() const;
   inline qreal height() const;
   inline void setWidth(qreal w);
   inline void setHeight(qreal h);
   inline void setSize(const QSizeF &s);

   QRectF operator|(const QRectF &r) const;
   QRectF operator&(const QRectF &r) const;
   inline QRectF &operator|=(const QRectF &r);
   inline QRectF &operator&=(const QRectF &r);

   bool contains(const QRectF &r) const;
   bool contains(const QPointF &p) const;
   inline bool contains(qreal x, qreal y) const;

   inline QRectF united(const QRectF &other) const;
   inline QRectF intersected(const QRectF &other) const;
   bool intersects(const QRectF &r) const;
   inline QRectF marginsAdded(const QMarginsF &margins) const;
   inline QRectF marginsRemoved(const QMarginsF &margins) const;
   inline QRectF &operator+=(const QMarginsF &margins);
   inline QRectF &operator-=(const QMarginsF &margins);

   friend inline bool operator==(const QRectF &, const QRectF &);
   friend inline bool operator!=(const QRectF &, const QRectF &);

   inline QRect toRect() const;
   QRect toAlignedRect() const;

 private:
   qreal xp;
   qreal yp;
   qreal w;
   qreal h;
};
Q_DECLARE_TYPEINFO(QRectF, Q_MOVABLE_TYPE);

inline bool operator==(const QRectF &, const QRectF &);
inline bool operator!=(const QRectF &, const QRectF &);

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QRectF &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QRectF &);

inline QRectF::QRectF(qreal aleft, qreal atop, qreal awidth, qreal aheight)
   : xp(aleft), yp(atop), w(awidth), h(aheight)
{
}

inline QRectF::QRectF(const QPointF &atopLeft, const QSizeF &asize)
{
   xp = atopLeft.x();
   yp = atopLeft.y();
   w = asize.width();
   h = asize.height();
}

inline QRectF::QRectF(const QPointF &atopLeft, const QPointF &abottomRight)
{
   xp = atopLeft.x();
   yp = atopLeft.y();
   w = abottomRight.x() - xp;
   h = abottomRight.y() - yp;
}

inline QRectF::QRectF(const QRect &r)
   : xp(r.x()), yp(r.y()), w(r.width()), h(r.height())
{
}

inline bool QRectF::isNull() const
{
   return w == 0. && h == 0.;
}

inline bool QRectF::isEmpty() const
{
   return w <= 0. || h <= 0.;
}

inline bool QRectF::isValid() const
{
   return w > 0. && h > 0.;
}

inline qreal QRectF::x() const
{
   return xp;
}

inline qreal QRectF::y() const
{
   return yp;
}

inline void QRectF::setLeft(qreal pos)
{
   qreal diff = pos - xp;
   xp += diff;
   w -= diff;
}

inline void QRectF::setRight(qreal pos)
{
   w = pos - xp;
}

inline void QRectF::setTop(qreal pos)
{
   qreal diff = pos - yp;
   yp += diff;
   h -= diff;
}

inline void QRectF::setBottom(qreal pos)
{
   h = pos - yp;
}

inline void QRectF::setTopLeft(const QPointF &p)
{
   setLeft(p.x());
   setTop(p.y());
}

inline void QRectF::setTopRight(const QPointF &p)
{
   setRight(p.x());
   setTop(p.y());
}

inline void QRectF::setBottomLeft(const QPointF &p)
{
   setLeft(p.x());
   setBottom(p.y());
}

inline void QRectF::setBottomRight(const QPointF &p)
{
   setRight(p.x());
   setBottom(p.y());
}

inline QPointF QRectF::center() const
{
   return QPointF(xp + w / 2, yp + h / 2);
}

inline void QRectF::moveLeft(qreal pos)
{
   xp = pos;
}

inline void QRectF::moveTop(qreal pos)
{
   yp = pos;
}

inline void QRectF::moveRight(qreal pos)
{
   xp = pos - w;
}

inline void QRectF::moveBottom(qreal pos)
{
   yp = pos - h;
}

inline void QRectF::moveTopLeft(const QPointF &p)
{
   moveLeft(p.x());
   moveTop(p.y());
}

inline void QRectF::moveTopRight(const QPointF &p)
{
   moveRight(p.x());
   moveTop(p.y());
}

inline void QRectF::moveBottomLeft(const QPointF &p)
{
   moveLeft(p.x());
   moveBottom(p.y());
}

inline void QRectF::moveBottomRight(const QPointF &p)
{
   moveRight(p.x());
   moveBottom(p.y());
}

inline void QRectF::moveCenter(const QPointF &p)
{
   xp = p.x() - w / 2;
   yp = p.y() - h / 2;
}

inline qreal QRectF::width() const
{
   return w;
}

inline qreal QRectF::height() const
{
   return h;
}

inline QSizeF QRectF::size() const
{
   return QSizeF(w, h);
}

inline void QRectF::translate(qreal dx, qreal dy)
{
   xp += dx;
   yp += dy;
}

inline void QRectF::translate(const QPointF &p)
{
   xp += p.x();
   yp += p.y();
}

inline void QRectF::moveTo(qreal ax, qreal ay)
{
   xp = ax;
   yp = ay;
}

inline void QRectF::moveTo(const QPointF &p)
{
   xp = p.x();
   yp = p.y();
}

inline QRectF QRectF::translated(qreal dx, qreal dy) const
{
   return QRectF(xp + dx, yp + dy, w, h);
}

inline QRectF QRectF::translated(const QPointF &p) const
{
   return QRectF(xp + p.x(), yp + p.y(), w, h);
}

inline void QRectF::getRect(qreal *ax, qreal *ay, qreal *aaw, qreal *aah) const
{
   *ax = this->xp;
   *ay = this->yp;
   *aaw = this->w;
   *aah = this->h;
}

inline void QRectF::setRect(qreal ax, qreal ay, qreal aaw, qreal aah)
{
   this->xp = ax;
   this->yp = ay;
   this->w = aaw;
   this->h = aah;
}

inline void QRectF::getCoords(qreal *xp1, qreal *yp1, qreal *xp2, qreal *yp2) const
{
   *xp1 = xp;
   *yp1 = yp;
   *xp2 = xp + w;
   *yp2 = yp + h;
}

inline void QRectF::setCoords(qreal xp1, qreal yp1, qreal xp2, qreal yp2)
{
   xp = xp1;
   yp = yp1;
   w = xp2 - xp1;
   h = yp2 - yp1;
}

inline void QRectF::adjust(qreal xp1, qreal yp1, qreal xp2, qreal yp2)
{
   xp += xp1;
   yp += yp1;
   w += xp2 - xp1;
   h += yp2 - yp1;
}

inline QRectF QRectF::adjusted(qreal xp1, qreal yp1, qreal xp2, qreal yp2) const
{
   return QRectF(xp + xp1, yp + yp1, w + xp2 - xp1, h + yp2 - yp1);
}

inline void QRectF::setWidth(qreal aw)
{
   this->w = aw;
}

inline void QRectF::setHeight(qreal ah)
{
   this->h = ah;
}

inline void QRectF::setSize(const QSizeF &s)
{
   w = s.width();
   h = s.height();
}

inline bool QRectF::contains(qreal ax, qreal ay) const
{
   return contains(QPointF(ax, ay));
}

inline QRectF &QRectF::operator|=(const QRectF &r)
{
   *this = *this | r;
   return *this;
}

inline QRectF &QRectF::operator&=(const QRectF &r)
{
   *this = *this & r;
   return *this;
}

inline QRectF QRectF::intersected(const QRectF &r) const
{
   return *this & r;
}

inline QRectF QRectF::united(const QRectF &r) const
{
   return *this | r;
}

inline bool operator==(const QRectF &r1, const QRectF &r2)
{
   return qFuzzyCompare(r1.xp, r2.xp) && qFuzzyCompare(r1.yp, r2.yp)
          && qFuzzyCompare(r1.w, r2.w) && qFuzzyCompare(r1.h, r2.h);
}

inline bool operator!=(const QRectF &r1, const QRectF &r2)
{
   return !qFuzzyCompare(r1.xp, r2.xp) || !qFuzzyCompare(r1.yp, r2.yp)
          || !qFuzzyCompare(r1.w, r2.w) || !qFuzzyCompare(r1.h, r2.h);
}

inline QRect QRectF::toRect() const
{
   return QRect(qRound(xp), qRound(yp), qRound(w), qRound(h));
}

inline QRectF operator+(const QRectF &lhs, const QMarginsF &rhs)
{
    return QRectF(QPointF(lhs.left() - rhs.left(), lhs.top() - rhs.top()),
                  QSizeF(lhs.width() + rhs.left() + rhs.right(), lhs.height() + rhs.top() + rhs.bottom()));
}
inline QRectF operator+(const QMarginsF &lhs, const QRectF &rhs)
{
    return QRectF(QPointF(rhs.left() - lhs.left(), rhs.top() - lhs.top()),
                  QSizeF(rhs.width() + lhs.left() + lhs.right(), rhs.height() + lhs.top() + lhs.bottom()));
}
inline QRectF operator-(const QRectF &lhs, const QMarginsF &rhs)
{
    return QRectF(QPointF(lhs.left() + rhs.left(), lhs.top() + rhs.top()),
                  QSizeF(lhs.width() - rhs.left() - rhs.right(), lhs.height() - rhs.top() - rhs.bottom()));
}
inline QRectF QRectF::marginsAdded(const QMarginsF &margins) const
{
    return QRectF(QPointF(xp - margins.left(), yp - margins.top()),
                  QSizeF(w + margins.left() + margins.right(), h + margins.top() + margins.bottom()));
}
inline QRectF QRectF::marginsRemoved(const QMarginsF &margins) const
{
    return QRectF(QPointF(xp + margins.left(), yp + margins.top()),
                  QSizeF(w - margins.left() - margins.right(), h - margins.top() - margins.bottom()));
}
inline QRectF &QRectF::operator+=(const QMarginsF &margins)
{
    *this = marginsAdded(margins);
    return *this;
}
inline QRectF &QRectF::operator-=(const QMarginsF &margins)
{
    *this = marginsRemoved(margins);
    return *this;
}
Q_CORE_EXPORT QDebug operator<<(QDebug, const QRectF &);


#endif // QRECT_H
