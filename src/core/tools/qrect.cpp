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

#include <qrect.h>

#include <qdatastream.h>
#include <qdebug.h>
#include <qmath.h>

#include <qdebug_p.h>

QRect QRect::normalized() const
{
   QRect r;

   if (m_x2 < m_x1 - 1) {                                // swap bad x values
      r.m_x1 = m_x2;
      r.m_x2 = m_x1;

   } else {
      r.m_x1 = m_x1;
      r.m_x2 = m_x2;
   }

   if (m_y2 < m_y1 - 1) {                                // swap bad y values
      r.m_y1 = m_y2;
      r.m_y2 = m_y1;

   } else {
      r.m_y1 = m_y1;
      r.m_y2 = m_y2;
   }

   return r;
}

bool QRect::contains(const QPoint &p, bool proper) const
{
   int l;
   int r;

   if (m_x2 < m_x1 - 1) {
      l = m_x2;
      r = m_x1;
   } else {
      l = m_x1;
      r = m_x2;
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

   int t;
   int b;

   if (m_y2 < m_y1 - 1) {
      t = m_y2;
      b = m_y1;
   } else {
      t = m_y1;
      b = m_y2;
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

   int l1 = m_x1;
   int r1 = m_x1;

   if (m_x2 - m_x1 + 1 < 0) {
      l1 = m_x2;

   } else {
      r1 = m_x2;
   }

   int l2 = r.m_x1;
   int r2 = r.m_x1;

   if (r.m_x2 - r.m_x1 + 1 < 0) {
      l2 = r.m_x2;
   } else {
      r2 = r.m_x2;
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

   int t1 = m_y1;
   int b1 = m_y1;

   if (m_y2 - m_y1 + 1 < 0) {
      t1 = m_y2;
   } else {
      b1 = m_y2;
   }

   int t2 = r.m_y1;
   int b2 = r.m_y1;

   if (r.m_y2 - r.m_y1 + 1 < 0) {
      t2 = r.m_y2;
   } else {
      b2 = r.m_y2;
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

   int l1 = m_x1;
   int r1 = m_x1;

   if (m_x2 - m_x1 + 1 < 0) {
      l1 = m_x2;
   } else {
      r1 = m_x2;
   }

   int l2 = r.m_x1;
   int r2 = r.m_x1;

   if (r.m_x2 - r.m_x1 + 1 < 0) {
      l2 = r.m_x2;
   } else {
      r2 = r.m_x2;
   }

   int t1 = m_y1;
   int b1 = m_y1;

   if (m_y2 - m_y1 + 1 < 0) {
      t1 = m_y2;
   } else {
      b1 = m_y2;
   }

   int t2 = r.m_y1;
   int b2 = r.m_y1;

   if (r.m_y2 - r.m_y1 + 1 < 0) {
      t2 = r.m_y2;
   } else {
      b2 = r.m_y2;
   }

   QRect tmp;
   tmp.m_x1 = qMin(l1, l2);
   tmp.m_x2 = qMax(r1, r2);
   tmp.m_y1 = qMin(t1, t2);
   tmp.m_y2 = qMax(b1, b2);

   return tmp;
}

QRect QRect::operator&(const QRect &r) const
{
   if (isNull() || r.isNull()) {
      return QRect();
   }

   int l1 = m_x1;
   int r1 = m_x1;

   if (m_x2 - m_x1 + 1 < 0) {
      l1 = m_x2;
   } else {
      r1 = m_x2;
   }

   int l2 = r.m_x1;
   int r2 = r.m_x1;

   if (r.m_x2 - r.m_x1 + 1 < 0) {
      l2 = r.m_x2;
   } else {
      r2 = r.m_x2;
   }

   if (l1 > r2 || l2 > r1) {
      return QRect();
   }

   int t1 = m_y1;
   int b1 = m_y1;

   if (m_y2 - m_y1 + 1 < 0) {
      t1 = m_y2;
   } else {
      b1 = m_y2;
   }

   int t2 = r.m_y1;
   int b2 = r.m_y1;

   if (r.m_y2 - r.m_y1 + 1 < 0) {
      t2 = r.m_y2;
   } else {
      b2 = r.m_y2;
   }

   if (t1 > b2 || t2 > b1) {
      return QRect();
   }

   QRect tmp;
   tmp.m_x1 = qMax(l1, l2);
   tmp.m_x2 = qMin(r1, r2);
   tmp.m_y1 = qMax(t1, t2);
   tmp.m_y2 = qMin(b1, b2);

   return tmp;
}

bool QRect::intersects(const QRect &r) const
{
   if (isNull() || r.isNull()) {
      return false;
   }

   int l1 = m_x1;
   int r1 = m_x1;

   if (m_x2 - m_x1 + 1 < 0) {
      l1 = m_x2;
   } else {
      r1 = m_x2;
   }

   int l2 = r.m_x1;
   int r2 = r.m_x1;

   if (r.m_x2 - r.m_x1 + 1 < 0) {
      l2 = r.m_x2;
   } else {
      r2 = r.m_x2;
   }

   if (l1 > r2 || l2 > r1) {
      return false;
   }

   int t1 = m_y1;
   int b1 = m_y1;

   if (m_y2 - m_y1 + 1 < 0) {
      t1 = m_y2;
   } else {
      b1 = m_y2;
   }

   int t2 = r.m_y1;
   int b2 = r.m_y1;

   if (r.m_y2 - r.m_y1 + 1 < 0) {
      t2 = r.m_y2;
   } else {
      b2 = r.m_y2;
   }

   if (t1 > b2 || t2 > b1) {
      return false;
   }

   return true;
}

QDataStream &operator<<(QDataStream &stream, const QRect &rect)
{
   stream << (qint32)rect.left()  << (qint32)rect.top()
         << (qint32)rect.right() << (qint32)rect.bottom();

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QRect &rect)
{
   qint32 x1, y1, x2, y2;
   stream >> x1;
   stream >> y1;
   stream >> x2;
   stream >> y2;

   rect.setCoords(x1, y1, x2, y2);

   return stream;
}

QDebug operator<<(QDebug dbg, const QRect &r)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   dbg << "QRect" << '(';
   QtDebugUtils::formatQRect(dbg, r);
   dbg << ')';

   return dbg;
}

QRectF QRectF::normalized() const
{
   QRectF r = *this;

   if (r.m_w < 0) {
      r.m_x += r.m_w;
      r.m_w = -r.m_w;
   }

   if (r.m_h < 0) {
      r.m_y += r.m_h;
      r.m_h = -r.m_h;
   }

   return r;
}

bool QRectF::contains(const QPointF &p) const
{
   qreal l = m_x;
   qreal r = m_x;

   if (m_w < 0) {
      l += m_w;
   } else {
      r += m_w;
   }

   if (l == r) {
      // null rect
      return false;
   }

   if (p.x() < l || p.x() > r) {
      return false;
   }

   qreal t = m_y;
   qreal b = m_y;

   if (m_h < 0) {
      t += m_h;
   } else {
      b += m_h;
   }

   if (t == b) {
      // null rect
      return false;
   }

   if (p.y() < t || p.y() > b) {
      return false;
   }

   return true;
}

bool QRectF::contains(const QRectF &r) const
{
   qreal l1 = m_x;
   qreal r1 = m_x;

   if (m_w < 0) {
      l1 += m_w;
   } else {
      r1 += m_w;
   }

   if (l1 == r1) {
      // null rect
      return false;
   }

   qreal l2 = r.m_x;
   qreal r2 = r.m_x;

   if (r.m_w < 0) {
      l2 += r.m_w;
   } else {
      r2 += r.m_w;
   }

   if (l2 == r2) {
      // null rect
      return false;
   }

   if (l2 < l1 || r2 > r1) {
      return false;
   }

   qreal t1 = m_y;
   qreal b1 = m_y;

   if (m_h < 0) {
      t1 += m_h;
   } else {
      b1 += m_h;
   }

   if (t1 == b1) {
      // null rect
      return false;
   }

   qreal t2 = r.m_y;
   qreal b2 = r.m_y;

   if (r.m_h < 0) {
      t2 += r.m_h;
   } else {
      b2 += r.m_h;
   }

   if (t2 == b2) {
      // null rect
      return false;
   }

   if (t2 < t1 || b2 > b1) {
      return false;
   }

   return true;
}

QRectF QRectF::operator|(const QRectF &r) const
{
   if (isNull()) {
      return r;
   }

   if (r.isNull()) {
      return *this;
   }

   qreal left  = m_x;
   qreal right = m_x;

   if (m_w < 0) {
      left += m_w;
   } else {
      right += m_w;
   }

   if (r.m_w < 0) {
      left  = qMin(left, r.m_x + r.m_w);
      right = qMax(right, r.m_x);
   } else {
      left  = qMin(left, r.m_x);
      right = qMax(right, r.m_x + r.m_w);
   }

   qreal top    = m_y;
   qreal bottom = m_y;

   if (m_h < 0) {
      top += m_h;
   } else {
      bottom += m_h;
   }

   if (r.m_h < 0) {
      top = qMin(top, r.m_y + r.m_h);
      bottom = qMax(bottom, r.m_y);
   } else {
      top = qMin(top, r.m_y);
      bottom = qMax(bottom, r.m_y + r.m_h);
   }

   return QRectF(left, top, right - left, bottom - top);
}

QRectF QRectF::operator&(const QRectF &r) const
{
   qreal l1 = m_x;
   qreal r1 = m_x;

   if (m_w < 0) {
      l1 += m_w;
   } else {
      r1 += m_w;
   }

   if (l1 == r1) {
      // null rect
      return QRectF();
   }

   qreal l2 = r.m_x;
   qreal r2 = r.m_x;

   if (r.m_w < 0) {
      l2 += r.m_w;
   } else {
      r2 += r.m_w;
   }

   if (l2 == r2) {
      // null rect
      return QRectF();
   }

   if (l1 >= r2 || l2 >= r1) {
      return QRectF();
   }

   qreal t1 = m_y;
   qreal b1 = m_y;

   if (m_h < 0) {
      t1 += m_h;
   } else {
      b1 += m_h;
   }

   if (t1 == b1) {
      // null rect
      return QRectF();
   }

   qreal t2 = r.m_y;
   qreal b2 = r.m_y;

   if (r.m_h < 0) {
      t2 += r.m_h;
   } else {
      b2 += r.m_h;
   }

   if (t2 == b2) {
      // null rect
      return QRectF();
   }

   if (t1 >= b2 || t2 >= b1) {
      return QRectF();
   }

   QRectF tmp;
   tmp.m_x = qMax(l1, l2);
   tmp.m_y = qMax(t1, t2);
   tmp.m_w = qMin(r1, r2) - tmp.m_x;
   tmp.m_h = qMin(b1, b2) - tmp.m_y;

   return tmp;
}

bool QRectF::intersects(const QRectF &r) const
{
   qreal l1 = m_x;
   qreal r1 = m_x;

   if (m_w < 0) {
      l1 += m_w;
   } else {
      r1 += m_w;
   }

   if (l1 == r1) {
      // null rect
      return false;
   }

   qreal l2 = r.m_x;
   qreal r2 = r.m_x;

   if (r.m_w < 0) {
      l2 += r.m_w;
   } else {
      r2 += r.m_w;
   }

   if (l2 == r2) {
      // null rect
      return false;
   }

   if (l1 >= r2 || l2 >= r1) {
      return false;
   }

   qreal t1 = m_y;
   qreal b1 = m_y;

   if (m_h < 0) {
      t1 += m_h;
   } else {
      b1 += m_h;
   }

   if (t1 == b1) {
      // null rect
      return false;
   }

   qreal t2 = r.m_y;
   qreal b2 = r.m_y;

   if (r.m_h < 0) {
      t2 += r.m_h;
   } else {
      b2 += r.m_h;
   }

   if (t2 == b2) {
      // null rect
      return false;
   }

   if (t1 >= b2 || t2 >= b1) {
      return false;
   }

   return true;
}

QRect QRectF::toAlignedRect() const
{
   int xmin = int(qFloor(m_x));
   int xmax = int(qCeil(m_x + m_w));
   int ymin = int(qFloor(m_y));
   int ymax = int(qCeil(m_y + m_h));

   return QRect(xmin, ymin, xmax - xmin, ymax - ymin);
}

QDataStream &operator<<(QDataStream &stream, const QRectF &rectF)
{
   stream << double(rectF.x()) << double(rectF.y()) << double(rectF.width()) << double(rectF.height());

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QRectF &rectF)
{
   double x, y, w, h;
   stream >> x;
   stream >> y;
   stream >> w;
   stream >> h;

   rectF.setRect(qreal(x), qreal(y), qreal(w), qreal(h));

   return stream;
}

QDebug operator<<(QDebug dbg, const QRectF &r)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   dbg << "QRectF" << '(';
   QtDebugUtils::formatQRect(dbg, r);
   dbg << ')';

   return dbg;
}
