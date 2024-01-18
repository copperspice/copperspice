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

#ifndef QPOLYGON_H
#define QPOLYGON_H

#include <qvector.h>
#include <qpoint.h>
#include <qrect.h>

class QMatrix;
class QTransform;
class QRect;
class QVariant;
class QRectF;

class Q_GUI_EXPORT QPolygon : public QVector<QPoint>
{
 public:
   QPolygon()
   { }

   inline explicit QPolygon(int size);

   QPolygon(const QPolygon &other)
      : QVector<QPoint>(other)
   { }

   QPolygon(QPolygon &&other)
      : QVector<QPoint>(std::move(other))
   { }

   QPolygon(const QVector<QPoint> &points)
      : QVector<QPoint>(points)
   { }

   QPolygon(QVector<QPoint> &&points)
      : QVector<QPoint>(std::move(points))
   { }

   QPolygon(const QRect &rect, bool closed = false);
   QPolygon(int pointCount, const int *points);

   ~QPolygon()
   { }

   QPolygon &operator=(const QPolygon &other) {
      QVector<QPoint>::operator=(other);
      return *this;
   }

   QPolygon &operator=(QPolygon &&other)  {
      swap(other);
      return *this;
   }

   void swap(QPolygon &other) {
      QVector<QPoint>::swap(other);
   }

   operator QVariant() const;

   void translate(int dx, int dy);
   inline void translate(const QPoint &offset);

   QPolygon translated(int dx, int dy) const;
   inline QPolygon translated(const QPoint &offset) const;

   QRect boundingRect() const;

   void point(int index, int *x, int *y) const;

   inline QPoint point(int index) const;
   inline void setPoint(int index, int x, int y);
   inline void setPoint(int index, const QPoint &point);

   void setPoints(int nPoints, const int *points);
   void setPoints(int nPoints, int firstx, int firsty, ...);
   void putPoints(int index, int nPoints, const int *points);
   void putPoints(int index, int nPoints, int firstx, int firsty, ...);
   void putPoints(int index, int nPoints, const QPolygon &fromPolygon, int fromIndex = 0);

   bool containsPoint(const QPoint &point, Qt::FillRule fillRule) const;

   QPolygon united(const QPolygon &other) const;
   QPolygon intersected(const QPolygon &other) const;
   QPolygon subtracted(const QPolygon &other) const;

   bool intersects(const QPolygon &other) const;
};

inline QPolygon::QPolygon(int size)
   : QVector<QPoint>(size)
{}

Q_GUI_EXPORT QDebug operator<<(QDebug, const QPolygon &);

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPolygon &polygon);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPolygon &polygon);

inline void QPolygon::setPoint(int index, const QPoint &point)
{
   (*this)[index] = point;
}

inline void QPolygon::setPoint(int index, int x, int y)
{
   (*this)[index] = QPoint(x, y);
}

inline QPoint QPolygon::point(int index) const
{
   return at(index);
}

inline void QPolygon::translate(const QPoint &offset)
{
   translate(offset.x(), offset.y());
}

inline QPolygon QPolygon::translated(const QPoint &offset) const
{
   return translated(offset.x(), offset.y());
}

class Q_GUI_EXPORT QPolygonF : public QVector<QPointF>
{
 public:
   QPolygonF()
   { }

   inline explicit QPolygonF(int size);

   QPolygonF(const QPolygonF &other)
      : QVector<QPointF>(other)
   { }

   QPolygonF(QPolygonF &&other)
      : QVector<QPointF>(std::move(other))
   { }

   QPolygonF(const QVector<QPointF> &points)
      : QVector<QPointF>(points)
   { }

   QPolygonF(QVector<QPointF> &&points)
      : QVector<QPointF>(std::move(points))
   { }

   QPolygonF(const QRectF &rect);
   QPolygonF(const QPolygon &polygon);   // not a copy constructor

   ~QPolygonF()
   { }

   void swap(QPolygonF &other) {
      QVector<QPointF>::swap(other);
   }

   QPolygonF &operator=(const QPolygonF &other) {
      QVector<QPointF>::operator=(other);
      return *this;
   }

   QPolygonF &operator=(QPolygonF &&other) {
      swap(other);
      return *this;
   }

   operator QVariant() const;

   inline void translate(qreal dx, qreal dy);
   void translate(const QPointF &offset);

   inline QPolygonF translated(qreal dx, qreal dy) const;
   QPolygonF translated(const QPointF &offset) const;

   QPolygon toPolygon() const;

   bool isClosed() const {
      return ! isEmpty() && first() == last();
   }

   QRectF boundingRect() const;

   bool containsPoint(const QPointF &point, Qt::FillRule fillRule) const;

   QPolygonF united(const QPolygonF &other) const;
   QPolygonF intersected(const QPolygonF &other) const;
   QPolygonF subtracted(const QPolygonF &other) const;

   bool intersects(const QPolygonF &other) const;
};

inline QPolygonF::QPolygonF(int size)
   : QVector<QPointF>(size)
{
}

Q_GUI_EXPORT QDebug operator<<(QDebug, const QPolygonF &);

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPolygonF &polygon);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPolygonF &polygon);

inline void QPolygonF::translate(qreal dx, qreal dy)
{
   translate(QPointF(dx, dy));
}

inline QPolygonF QPolygonF::translated(qreal dx, qreal dy) const
{
   return translated(QPointF(dx, dy));
}

#endif
