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

#ifndef QLINE_H
#define QLINE_H

#include <qpoint.h>

class QDataStream;
class QDebug;

class Q_CORE_EXPORT QLine
{
 public:
   QLine() = default;

   QLine(const QPoint &point1, const QPoint &point2)
      : pt1(point1), pt2(point2)
   { }

   QLine(int x1, int y1, int x2, int y2)
      : pt1(QPoint(x1, y1)), pt2(QPoint(x2, y2))
   { }

   inline bool isNull() const;

   inline QPoint p1() const;
   inline QPoint p2() const;

   inline int x1() const;
   inline int y1() const;

   inline int x2() const;
   inline int y2() const;

   inline int dx() const;
   inline int dy() const;

   inline void translate(const QPoint &offset);
   inline void translate(int dx, int dy);

   inline QLine translated(const QPoint &offset) const;
   inline QLine translated(int dx, int dy) const;

   inline void setP1(const QPoint &point);
   inline void setP2(const QPoint &point);
   inline void setPoints(const QPoint &point1, const QPoint &point2);
   inline void setLine(int x1, int y1, int x2, int y2);

   inline bool operator==(const QLine &line) const;
   bool operator!=(const QLine &line) const {
      return !(*this == line);
   }

 private:
   QPoint pt1;
   QPoint pt2;
};

inline bool QLine::isNull() const
{
   return pt1 == pt2;
}

inline int QLine::x1() const
{
   return pt1.x();
}

inline int QLine::y1() const
{
   return pt1.y();
}

inline int QLine::x2() const
{
   return pt2.x();
}

inline int QLine::y2() const
{
   return pt2.y();
}

inline QPoint QLine::p1() const
{
   return pt1;
}

inline QPoint QLine::p2() const
{
   return pt2;
}

inline int QLine::dx() const
{
   return pt2.x() - pt1.x();
}

inline int QLine::dy() const
{
   return pt2.y() - pt1.y();
}

inline void QLine::translate(const QPoint &offset)
{
   pt1 += offset;
   pt2 += offset;
}

inline void QLine::translate(int dx, int dy)
{
   this->translate(QPoint(dx, dy));
}

inline QLine QLine::translated(const QPoint &offset) const
{
   return QLine(pt1 + offset, pt2 + offset);
}

inline QLine QLine::translated(int dx, int dy) const
{
   return translated(QPoint(dx, dy));
}

inline void QLine::setP1(const QPoint &point)
{
   pt1 = point;
}

inline void QLine::setP2(const QPoint &point)
{
   pt2 = point;
}

inline void QLine::setPoints(const QPoint &point1, const QPoint &point2)
{
   pt1 = point1;
   pt2 = point2;
}

inline void QLine::setLine(int x1, int y1, int x2, int y2)
{
   pt1 = QPoint(x1, y1);
   pt2 = QPoint(x2, y2);
}

inline bool QLine::operator==(const QLine &line) const
{
   return pt1 == line.pt1 && pt2 == line.pt2;
}

Q_CORE_EXPORT QDebug operator<<(QDebug d, const QLine &line);

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QLine &line);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QLine &line);

class Q_CORE_EXPORT QLineF
{
 public:
   enum IntersectType {
      NoIntersection,
      BoundedIntersection,
      UnboundedIntersection,
   };

   QLineF() = default;

   QLineF(const QPointF &point1, const QPointF &point2)
      : pt1(point1), pt2(point2)
   {
   }

   QLineF(qreal x1, qreal y1, qreal x2, qreal y2)
      : pt1(x1, y1), pt2(x2, y2)
   {
   }

   QLineF(const QLine &line)
      : pt1(line.p1()), pt2(line.p2())
   {
   }

   static QLineF fromPolar(qreal length, qreal angle);

   bool isNull() const;

   inline QPointF p1() const;
   inline QPointF p2() const;

   inline qreal x1() const;
   inline qreal y1() const;

   inline qreal x2() const;
   inline qreal y2() const;

   inline qreal dx() const;
   inline qreal dy() const;

   qreal length() const;
   inline void setLength(qreal length);

   qreal angle() const;
   qreal angle(const QLineF &lineF) const;

   qreal angleTo(const QLineF &lineF) const;

   void setAngle(qreal angle);

   QLineF unitVector() const;
   inline QLineF normalVector() const;

   // TODO: rename intersects() or intersection() and rename IntersectType IntersectionType
   IntersectType intersect(const QLineF &lineF, QPointF *intersectionPoint) const;

   inline QPointF pointAt(qreal pos) const;
   inline void translate(const QPointF &offset);
   inline void translate(qreal dx, qreal dy);

   inline QLineF translated(const QPointF &offset) const;
   inline QLineF translated(qreal dx, qreal dy) const;

   inline void setP1(const QPointF &point);
   inline void setP2(const QPointF &point);
   inline void setPoints(const QPointF &point1, const QPointF &point2);
   inline void setLine(qreal x1, qreal y1, qreal x2, qreal y2);

   inline bool operator==(const QLineF &lineF) const;
   bool operator!=(const QLineF &lineF) const {
      return !(*this == lineF);
   }

   inline QLine toLine() const;

 private:
   QPointF pt1, pt2;
};

inline qreal QLineF::x1() const
{
   return pt1.x();
}

inline qreal QLineF::y1() const
{
   return pt1.y();
}

inline qreal QLineF::x2() const
{
   return pt2.x();
}

inline qreal QLineF::y2() const
{
   return pt2.y();
}

inline QPointF QLineF::p1() const
{
   return pt1;
}

inline QPointF QLineF::p2() const
{
   return pt2;
}

inline qreal QLineF::dx() const
{
   return pt2.x() - pt1.x();
}

inline qreal QLineF::dy() const
{
   return pt2.y() - pt1.y();
}

inline QLineF QLineF::normalVector() const
{
   return QLineF(p1(), p1() + QPointF(dy(), -dx()));
}

inline void QLineF::translate(const QPointF &offset)
{
   pt1 += offset;
   pt2 += offset;
}

inline void QLineF::translate(qreal dx, qreal dy)
{
   this->translate(QPointF(dx, dy));
}

inline QLineF QLineF::translated(const QPointF &offset) const
{
   return QLineF(pt1 + offset, pt2 + offset);
}

inline QLineF QLineF::translated(qreal dx, qreal dy) const
{
   return translated(QPointF(dx, dy));
}

inline void QLineF::setLength(qreal len)
{
   if (isNull()) {
      return;
   }

   QLineF v = unitVector();
   pt2 = QPointF(pt1.x() + v.dx() * len, pt1.y() + v.dy() * len);
}

inline QPointF QLineF::pointAt(qreal pos) const
{
   qreal vx = pt2.x() - pt1.x();
   qreal vy = pt2.y() - pt1.y();

   return QPointF(pt1.x() + vx * pos, pt1.y() + vy * pos);
}

inline QLine QLineF::toLine() const
{
   return QLine(pt1.toPoint(), pt2.toPoint());
}

inline void QLineF::setP1(const QPointF &point)
{
   pt1 = point;
}

inline void QLineF::setP2(const QPointF &point)
{
   pt2 = point;
}

inline void QLineF::setPoints(const QPointF &point1, const QPointF &point2)
{
   pt1 = point1;
   pt2 = point2;
}

inline void QLineF::setLine(qreal x1, qreal y1, qreal x2, qreal y2)
{
   pt1 = QPointF(x1, y1);
   pt2 = QPointF(x2, y2);
}

inline bool QLineF::operator==(const QLineF &lineF) const
{
   return pt1 == lineF.pt1 && pt2 == lineF.pt2;
}

Q_CORE_EXPORT QDebug operator<<(QDebug d, const QLineF &point);

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QLineF &lineF);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QLineF &lineF);

#endif
