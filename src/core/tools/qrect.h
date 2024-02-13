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

#ifndef QRECT_H
#define QRECT_H

#include <qmargins.h>
#include <qpoint.h>
#include <qsize.h>

#ifdef topLeft
#error qrect.h must be included before any header file that defines topLeft
#endif

class Q_CORE_EXPORT QRect
{
 public:
   QRect()
   {
      m_x1 = 0;
      m_y1 = 0;
      m_x2 = -1;
      m_y2 = -1;
   }

   QRect(const QPoint &topLeft, const QPoint &bottomRight);
   QRect(const QPoint &topLeft, const QSize &size);
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

   inline void setX(int xPos);
   inline void setY(int yPos);

   inline void setTopLeft(const QPoint &point);
   inline void setBottomRight(const QPoint &point);
   inline void setTopRight(const QPoint &point);
   inline void setBottomLeft(const QPoint &point);

   inline QPoint topLeft() const;
   inline QPoint bottomRight() const;
   inline QPoint topRight() const;
   inline QPoint bottomLeft() const;

   inline QPoint center() const;

   inline void moveLeft(int pos);
   inline void moveTop(int pos);
   inline void moveRight(int pos);
   inline void moveBottom(int pos);
   inline void moveTopLeft(const QPoint &point);
   inline void moveBottomRight(const QPoint &point);
   inline void moveTopRight(const QPoint &point);
   inline void moveBottomLeft(const QPoint &point);

   inline void moveCenter(const QPoint &point);

   inline void translate(int x, int y);
   inline void translate(const QPoint &offset);
   inline QRect translated(int x, int y) const;
   inline QRect translated(const QPoint &offset) const;

   inline void moveTo(int x, int y);
   inline void moveTo(const QPoint &point);

   inline void setRect(int x, int y, int width, int height);
   inline void getRect(int *x, int *y, int *width, int *height) const;

   inline void setCoords(int x1, int y1, int x2, int y2);
   inline void getCoords(int *x1, int *y1, int *x2, int *y2) const;

   inline void adjust(int x1, int y1, int x2, int y2);
   inline QRect adjusted(int x1, int y1, int x2, int y2) const;

   inline QSize size() const;
   inline int width() const;
   inline int height() const;
   inline void setWidth(int w);
   inline void setHeight(int h);
   inline void setSize(const QSize &size);

   QRect operator|(const QRect &rect) const;
   QRect operator&(const QRect &rect) const;
   inline QRect &operator|=(const QRect &rect);
   inline QRect &operator&=(const QRect &rect);

   bool contains(const QRect &rect, bool proper = false) const;
   bool contains(const QPoint &point, bool proper = false) const;
   inline bool contains(int x, int y) const;
   inline bool contains(int x, int y, bool proper) const;

   inline QRect united(const QRect &rect) const;
   inline QRect intersected(const QRect &rect) const;
   bool intersects(const QRect &rect) const;

   inline QRect marginsAdded(const QMargins &margins) const;
   inline QRect marginsRemoved(const QMargins &margins) const;
   inline QRect &operator+=(const QMargins &margins);
   inline QRect &operator-=(const QMargins &margins);

   friend inline bool operator==(const QRect &rect1, const QRect &rect2);
   friend inline bool operator!=(const QRect &rect1, const QRect &rect2);

 private:
   int m_x1;
   int m_y1;
   int m_x2;
   int m_y2;
};

inline bool operator==(const QRect &rect1, const QRect &rect2);
inline bool operator!=(const QRect &rect1, const QRect &rect2);

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QRect &rect);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QRect &rect);

inline QRect::QRect(int left, int top, int width, int height)
{
   m_x1 = left;
   m_y1 = top;
   m_x2 = (left + width - 1);
   m_y2 = (top + height - 1);
}

inline QRect::QRect(const QPoint &topLeft, const QPoint &bottomRight)
{
   m_x1 = topLeft.x();
   m_y1 = topLeft.y();
   m_x2 = bottomRight.x();
   m_y2 = bottomRight.y();
}

inline QRect::QRect(const QPoint &topLeft, const QSize &size)
{
   m_x1 = topLeft.x();
   m_y1 = topLeft.y();
   m_x2 = (m_x1 + size.width() - 1);
   m_y2 = (m_y1 + size.height() - 1);
}

inline bool QRect::isNull() const
{
   return m_x2 == m_x1 - 1 && m_y2 == m_y1 - 1;
}

inline bool QRect::isEmpty() const
{
   return m_x1 > m_x2 || m_y1 > m_y2;
}

inline bool QRect::isValid() const
{
   return m_x1 <= m_x2 && m_y1 <= m_y2;
}

inline int QRect::left() const
{
   return m_x1;
}

inline int QRect::top() const
{
   return m_y1;
}

inline int QRect::right() const
{
   return m_x2;
}

inline int QRect::bottom() const
{
   return m_y2;
}

inline int QRect::x() const
{
   return m_x1;
}

inline int QRect::y() const
{
   return m_y1;
}

inline void QRect::setLeft(int pos)
{
   m_x1 = pos;
}

inline void QRect::setTop(int pos)
{
   m_y1 = pos;
}

inline void QRect::setRight(int pos)
{
   m_x2 = pos;
}

inline void QRect::setBottom(int pos)
{
   m_y2 = pos;
}

inline void QRect::setTopLeft(const QPoint &point)
{
   m_x1 = point.x();
   m_y1 = point.y();
}

inline void QRect::setBottomRight(const QPoint &point)
{
   m_x2 = point.x();
   m_y2 = point.y();
}

inline void QRect::setTopRight(const QPoint &point)
{
   m_x2 = point.x();
   m_y1 = point.y();
}

inline void QRect::setBottomLeft(const QPoint &point)
{
   m_x1 = point.x();
   m_y2 = point.y();
}

inline void QRect::setX(int xPos)
{
   m_x1 = xPos;
}

inline void QRect::setY(int yPos)
{
   m_y1 = yPos;
}

inline QPoint QRect::topLeft() const
{
   return QPoint(m_x1, m_y1);
}

inline QPoint QRect::bottomRight() const
{
   return QPoint(m_x2, m_y2);
}

inline QPoint QRect::topRight() const
{
   return QPoint(m_x2, m_y1);
}

inline QPoint QRect::bottomLeft() const
{
   return QPoint(m_x1, m_y2);
}

inline QPoint QRect::center() const
{
   return QPoint((m_x1 + m_x2) / 2, (m_y1 + m_y2) / 2);
}

inline int QRect::width() const
{
   return  m_x2 - m_x1 + 1;
}

inline int QRect::height() const
{
   return  m_y2 - m_y1 + 1;
}

inline QSize QRect::size() const
{
   return QSize(width(), height());
}

inline void QRect::translate(int x, int y)
{
   m_x1 += x;
   m_y1 += y;
   m_x2 += x;
   m_y2 += y;
}

inline void QRect::translate(const QPoint &offset)
{
   m_x1 += offset.x();
   m_y1 += offset.y();
   m_x2 += offset.x();
   m_y2 += offset.y();
}

inline QRect QRect::translated(int x, int y) const
{
   return QRect(QPoint(m_x1 + x, m_y1 + y), QPoint(m_x2 + x, m_y2 + y));
}

inline QRect QRect::translated(const QPoint &offset) const
{
   return QRect(QPoint(m_x1 + offset.x(), m_y1 + offset.y()), QPoint(m_x2 + offset.x(), m_y2 + offset.y()));
}

inline void QRect::moveTo(int x, int y)
{
   m_x2 += x - m_x1;
   m_y2 += y - m_y1;
   m_x1 =  x;
   m_y1 =  y;
}

inline void QRect::moveTo(const QPoint &point)
{
   m_x2 += point.x() - m_x1;
   m_y2 += point.y() - m_y1;
   m_x1 =  point.x();
   m_y1 =  point.y();
}

inline void QRect::moveLeft(int pos)
{
   m_x2 += (pos - m_x1);
   m_x1 = pos;
}

inline void QRect::moveTop(int pos)
{
   m_y2 += (pos - m_y1);
   m_y1 = pos;
}

inline void QRect::moveRight(int pos)
{
   m_x1 += (pos - m_x2);
   m_x2 = pos;
}

inline void QRect::moveBottom(int pos)
{
   m_y1 += (pos - m_y2);
   m_y2 = pos;
}

inline void QRect::moveTopLeft(const QPoint &point)
{
   moveLeft(point.x());
   moveTop(point.y());
}

inline void QRect::moveBottomRight(const QPoint &point)
{
   moveRight(point.x());
   moveBottom(point.y());
}

inline void QRect::moveTopRight(const QPoint &point)
{
   moveRight(point.x());
   moveTop(point.y());
}

inline void QRect::moveBottomLeft(const QPoint &point)
{
   moveLeft(point.x());
   moveBottom(point.y());
}

inline void QRect::moveCenter(const QPoint &point)
{
   int w = m_x2 - m_x1;
   int h = m_y2 - m_y1;

   m_x1 = point.x() - w / 2;
   m_y1 = point.y() - h / 2;
   m_x2 = m_x1 + w;
   m_y2 = m_y1 + h;
}

inline void QRect::getRect(int *x, int *y, int *width, int *height) const
{
   *x = m_x1;
   *y = m_y1;
   *width  = m_x2 - m_x1 + 1;
   *height = m_y2 - m_y1 + 1;
}

inline void QRect::setRect(int x, int y, int width, int height)
{
   m_x1 = x;
   m_y1 = y;
   m_x2 = (x + width - 1);
   m_y2 = (y + height - 1);
}

inline void QRect::getCoords(int *x1, int *y1, int *x2, int *y2) const
{
   *x1 = m_x1;
   *y1 = m_y1;
   *x2 = m_x2;
   *y2 = m_y2;
}

inline void QRect::setCoords(int x1, int y1, int x2, int y2)
{
   m_x1 = x1;
   m_y1 = y1;
   m_x2 = x2;
   m_y2 = y2;
}

inline QRect QRect::adjusted(int x1, int y1, int x2, int y2) const
{
   return QRect(QPoint(m_x1 + x1, m_y1 + y1), QPoint(m_x2 + x2, m_y2 + y2));
}

inline void QRect::adjust(int x1, int y1, int x2, int y2)
{
   m_x1 += x1;
   m_y1 += y1;
   m_x2 += x2;
   m_y2 += y2;
}

inline void QRect::setWidth(int w)
{
   m_x2 = (m_x1 + w - 1);
}

inline void QRect::setHeight(int h)
{
   m_y2 = (m_y1 + h - 1);
}

inline void QRect::setSize(const QSize &size)
{
   m_x2 = (size.width()  + m_x1 - 1);
   m_y2 = (size.height() + m_y1 - 1);
}

inline bool QRect::contains(int x, int y, bool proper) const
{
   return contains(QPoint(x, y), proper);
}

inline bool QRect::contains(int x, int y) const
{
   return contains(QPoint(x, y), false);
}

inline QRect &QRect::operator|=(const QRect &rect)
{
   *this = *this | rect;
   return *this;
}

inline QRect &QRect::operator&=(const QRect &rect)
{
   *this = *this & rect;
   return *this;
}

inline QRect QRect::intersected(const QRect &rect) const
{
   return *this & rect;
}

inline QRect QRect::united(const QRect &rect) const
{
   return *this | rect;
}

inline bool operator==(const QRect &rect1, const QRect &rect2)
{
   return rect1.m_x1 == rect2.m_x1 && rect1.m_x2 == rect2.m_x2 && rect1.m_y1 == rect2.m_y1 && rect1.m_y2 == rect2.m_y2;
}

inline bool operator!=(const QRect &rect1, const QRect &rect2)
{
   return rect1.m_x1 != rect2.m_x1 || rect1.m_x2 != rect2.m_x2 || rect1.m_y1 != rect2.m_y1 || rect1.m_y2 != rect2.m_y2;
}

inline QRect operator+(const QRect &rect, const QMargins &margins)
{
   return QRect(QPoint(rect.left() - margins.left(), rect.top() - margins.top()),
         QPoint(rect.right() + margins.right(), rect.bottom() + margins.bottom()));
}

inline QRect operator+(const QMargins &margins, const QRect &rect)
{
   return QRect(QPoint(rect.left() - margins.left(), rect.top() - margins.top()),
         QPoint(rect.right() + margins.right(), rect.bottom() + margins.bottom()));
}

inline QRect operator-(const QRect &lhs, const QMargins &rhs)
{
   return QRect(QPoint(lhs.left() + rhs.left(), lhs.top() + rhs.top()),
         QPoint(lhs.right() - rhs.right(), lhs.bottom() - rhs.bottom()));
}

inline QRect QRect::marginsAdded(const QMargins &margins) const
{
   return QRect(QPoint(m_x1 - margins.left(), m_y1 - margins.top()),
         QPoint(m_x2 + margins.right(), m_y2 + margins.bottom()));
}

inline QRect QRect::marginsRemoved(const QMargins &margins) const
{
   return QRect(QPoint(m_x1 + margins.left(), m_y1 + margins.top()),
         QPoint(m_x2 - margins.right(), m_y2 - margins.bottom()));
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
   QRectF()
      : m_x(0.0), m_y(0.0), m_w(0.0), m_h(0.0)
   {
   }

   QRectF(const QPointF &topLeft, const QSizeF &size);
   QRectF(const QPointF &topLeft, const QPointF &bottomRight);
   QRectF(qreal left, qreal top, qreal width, qreal height);
   QRectF(const QRect &rect);

   inline bool isNull() const;
   inline bool isEmpty() const;
   inline bool isValid() const;
   QRectF normalized() const;

   qreal left() const {
      return m_x;
   }

   qreal top() const {
      return m_y;
   }

   qreal right() const {
      return m_x + m_w;
   }

   qreal bottom() const {
      return m_y + m_h;
   }

   inline qreal x() const;
   inline qreal y() const;

   inline void setLeft(qreal pos);
   inline void setTop(qreal pos);
   inline void setRight(qreal pos);
   inline void setBottom(qreal pos);

   void setX(qreal xPos) {
      setLeft(xPos);
   }

   void setY(qreal yPos) {
      setTop(yPos);
   }

   QPointF topLeft() const {
      return QPointF(m_x, m_y);
   }

   QPointF bottomRight() const {
      return QPointF(m_x + m_w, m_y + m_h);
   }

   QPointF topRight() const {
      return QPointF(m_x + m_w, m_y);
   }

   QPointF bottomLeft() const {
      return QPointF(m_x, m_y + m_h);
   }

   inline QPointF center() const;

   inline void setTopLeft(const QPointF &point);
   inline void setBottomRight(const QPointF &point);
   inline void setTopRight(const QPointF &point);
   inline void setBottomLeft(const QPointF &point);

   inline void moveLeft(qreal pos);
   inline void moveTop(qreal pos);
   inline void moveRight(qreal pos);
   inline void moveBottom(qreal pos);

   inline void moveTopLeft(const QPointF &point);
   inline void moveBottomRight(const QPointF &point);
   inline void moveTopRight(const QPointF &point);
   inline void moveBottomLeft(const QPointF &point);
   inline void moveCenter(const QPointF &point);

   inline void translate(qreal x, qreal y);
   inline void translate(const QPointF &offset);
   inline QRectF translated(qreal x, qreal y) const;
   inline QRectF translated(const QPointF &offset) const;

   inline void moveTo(qreal x, qreal y);
   inline void moveTo(const QPointF &point);

   inline void setRect(qreal x, qreal y, qreal width, qreal height);
   inline void getRect(qreal *x, qreal *y, qreal *width, qreal *height) const;

   inline void setCoords(qreal x1, qreal y1, qreal x2, qreal y2);
   inline void getCoords(qreal *x1, qreal *y1, qreal *x2, qreal *y2) const;

   inline void adjust(qreal x1, qreal y1, qreal x2, qreal y2);
   inline QRectF adjusted(qreal x1, qreal y1, qreal x2, qreal y2) const;

   inline QSizeF size() const;
   inline qreal width() const;
   inline qreal height() const;
   inline void setWidth(qreal w);
   inline void setHeight(qreal h);
   inline void setSize(const QSizeF &size);

   QRectF operator|(const QRectF &rect) const;
   QRectF operator&(const QRectF &rect) const;
   inline QRectF &operator|=(const QRectF &rect);
   inline QRectF &operator&=(const QRectF &rect);

   bool contains(const QRectF &rect) const;
   bool contains(const QPointF &point) const;
   inline bool contains(qreal x, qreal y) const;

   inline QRectF united(const QRectF &rect) const;
   inline QRectF intersected(const QRectF &rect) const;
   bool intersects(const QRectF &rect) const;
   inline QRectF marginsAdded(const QMarginsF &margins) const;
   inline QRectF marginsRemoved(const QMarginsF &margins) const;
   inline QRectF &operator+=(const QMarginsF &margins);
   inline QRectF &operator-=(const QMarginsF &margins);

   friend inline bool operator==(const QRectF &rect1, const QRectF &rect2);
   friend inline bool operator!=(const QRectF &rect1, const QRectF &rect2);

   inline QRect toRect() const;
   QRect toAlignedRect() const;

 private:
   qreal m_x;
   qreal m_y;
   qreal m_w;
   qreal m_h;
};

inline bool operator==(const QRectF &rect1, const QRectF &rect2);
inline bool operator!=(const QRectF &rect1, const QRectF &rect2);

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QRectF &rectF);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QRectF &rectF);

inline QRectF::QRectF(qreal left, qreal top, qreal width, qreal height)
   : m_x(left), m_y(top), m_w(width), m_h(height)
{
}

inline QRectF::QRectF(const QPointF &topLeft, const QSizeF &size)
{
   m_x = topLeft.x();
   m_y = topLeft.y();
   m_w  = size.width();
   m_h  = size.height();
}

inline QRectF::QRectF(const QPointF &topLeft, const QPointF &bottomRight)
{
   m_x = topLeft.x();
   m_y = topLeft.y();
   m_w = bottomRight.x() - m_x;
   m_h = bottomRight.y() - m_y;
}

inline QRectF::QRectF(const QRect &rect)
   : m_x(rect.x()), m_y(rect.y()), m_w(rect.width()), m_h(rect.height())
{
}

inline bool QRectF::isNull() const
{
   return m_w == 0.0 && m_h == 0.0;
}

inline bool QRectF::isEmpty() const
{
   return m_w <= 0.0 || m_h <= 0.0;
}

inline bool QRectF::isValid() const
{
   return m_w > 0.0 && m_h > 0.0;
}

inline qreal QRectF::x() const
{
   return m_x;
}

inline qreal QRectF::y() const
{
   return m_y;
}

inline void QRectF::setLeft(qreal pos)
{
   qreal diff = pos - m_x;
   m_x += diff;
   m_w -= diff;
}

inline void QRectF::setRight(qreal pos)
{
   m_w = pos - m_x;
}

inline void QRectF::setTop(qreal pos)
{
   qreal diff = pos - m_y;
   m_y += diff;
   m_h -= diff;
}

inline void QRectF::setBottom(qreal pos)
{
   m_h = pos - m_y;
}

inline void QRectF::setTopLeft(const QPointF &point)
{
   setLeft(point.x());
   setTop(point.y());
}

inline void QRectF::setTopRight(const QPointF &point)
{
   setRight(point.x());
   setTop(point.y());
}

inline void QRectF::setBottomLeft(const QPointF &point)
{
   setLeft(point.x());
   setBottom(point.y());
}

inline void QRectF::setBottomRight(const QPointF &point)
{
   setRight(point.x());
   setBottom(point.y());
}

inline QPointF QRectF::center() const
{
   return QPointF(m_x + m_w / 2, m_y + m_h / 2);
}

inline void QRectF::moveLeft(qreal pos)
{
   m_x = pos;
}

inline void QRectF::moveTop(qreal pos)
{
   m_y = pos;
}

inline void QRectF::moveRight(qreal pos)
{
   m_x = pos - m_w;
}

inline void QRectF::moveBottom(qreal pos)
{
   m_y = pos - m_h;
}

inline void QRectF::moveTopLeft(const QPointF &point)
{
   moveLeft(point.x());
   moveTop(point.y());
}

inline void QRectF::moveTopRight(const QPointF &point)
{
   moveRight(point.x());
   moveTop(point.y());
}

inline void QRectF::moveBottomLeft(const QPointF &point)
{
   moveLeft(point.x());
   moveBottom(point.y());
}

inline void QRectF::moveBottomRight(const QPointF &point)
{
   moveRight(point.x());
   moveBottom(point.y());
}

inline void QRectF::moveCenter(const QPointF &point)
{
   m_x = point.x() - m_w / 2;
   m_y = point.y() - m_h / 2;
}

inline qreal QRectF::width() const
{
   return m_w;
}

inline qreal QRectF::height() const
{
   return m_h;
}

inline QSizeF QRectF::size() const
{
   return QSizeF(m_w, m_h);
}

inline void QRectF::translate(qreal x, qreal y)
{
   m_x += x;
   m_y += y;
}

inline void QRectF::translate(const QPointF &offset)
{
   m_x += offset.x();
   m_y += offset.y();
}

inline void QRectF::moveTo(qreal x, qreal y)
{
   m_x = x;
   m_y = y;
}

inline void QRectF::moveTo(const QPointF &point)
{
   m_x = point.x();
   m_y = point.y();
}

inline QRectF QRectF::translated(qreal x, qreal y) const
{
   return QRectF(m_x + x, m_y + y, m_w, m_h);
}

inline QRectF QRectF::translated(const QPointF &offset) const
{
   return QRectF(m_x + offset.x(), m_y + offset.y(), m_w, m_h);
}

inline void QRectF::getRect(qreal *x, qreal *y, qreal *w, qreal *h) const
{
   *x = m_x;
   *y = m_y;
   *w = m_w;
   *h = m_h;
}

inline void QRectF::setRect(qreal x, qreal y, qreal width, qreal height)
{
   m_x = x;
   m_y = y;
   m_w = width;
   m_h = height;
}

inline void QRectF::getCoords(qreal *x1, qreal *y1, qreal *x2, qreal *y2) const
{
   *x1 = m_x;
   *y1 = m_y;
   *x2 = m_x + m_w;
   *y2 = m_y + m_h;
}

inline void QRectF::setCoords(qreal x1, qreal y1, qreal x2, qreal y2)
{
   m_x = x1;
   m_y = y1;
   m_w = x2 - x1;
   m_h = y2 - y1;
}

inline void QRectF::adjust(qreal x1, qreal y1, qreal x2, qreal y2)
{
   m_x += x1;
   m_y += y1;
   m_w += x2 - x1;
   m_h += y2 - y1;
}

inline QRectF QRectF::adjusted(qreal x1, qreal y1, qreal x2, qreal y2) const
{
   return QRectF(m_x + x1, m_y + y1, m_w + x2 - x1, m_h + y2 - y1);
}

inline void QRectF::setWidth(qreal w)
{
   m_w = w;
}

inline void QRectF::setHeight(qreal h)
{
   m_h = h;
}

inline void QRectF::setSize(const QSizeF &size)
{
   m_w = size.width();
   m_h = size.height();
}

inline bool QRectF::contains(qreal x, qreal y) const
{
   return contains(QPointF(x, y));
}

inline QRectF &QRectF::operator|=(const QRectF &rect)
{
   *this = *this | rect;
   return *this;
}

inline QRectF &QRectF::operator&=(const QRectF &rect)
{
   *this = *this & rect;
   return *this;
}

inline QRectF QRectF::intersected(const QRectF &rect) const
{
   return *this & rect;
}

inline QRectF QRectF::united(const QRectF &rect) const
{
   return *this | rect;
}

inline bool operator==(const QRectF &rect1, const QRectF &rect2)
{
   return qFuzzyCompare(rect1.m_x, rect2.m_x) && qFuzzyCompare(rect1.m_y, rect2.m_y)
         && qFuzzyCompare(rect1.m_w, rect2.m_w) && qFuzzyCompare(rect1.m_h, rect2.m_h);
}

inline bool operator!=(const QRectF &rect1, const QRectF &rect2)
{
   return ! qFuzzyCompare(rect1.m_x, rect2.m_x) || ! qFuzzyCompare(rect1.m_y, rect2.m_y)
         || ! qFuzzyCompare(rect1.m_w, rect2.m_w) || ! qFuzzyCompare(rect1.m_h, rect2.m_h);
}

inline QRect QRectF::toRect() const
{
   return QRect(qRound(m_x), qRound(m_y), qRound(m_w), qRound(m_h));
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
   return QRectF(QPointF(m_x - margins.left(), m_y - margins.top()),
         QSizeF(m_w + margins.left() + margins.right(), m_h + margins.top() + margins.bottom()));
}

inline QRectF QRectF::marginsRemoved(const QMarginsF &margins) const
{
   return QRectF(QPointF(m_x + margins.left(), m_y + margins.top()),
         QSizeF(m_w - margins.left() - margins.right(), m_h - margins.top() - margins.bottom()));
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

Q_CORE_EXPORT QDebug operator<<(QDebug, const QRectF &rect);

#endif // QRECT_H
