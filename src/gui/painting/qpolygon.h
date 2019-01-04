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

#ifndef QPOLYGON_H
#define QPOLYGON_H

#include <QtCore/qvector.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>

class QMatrix;
class QTransform;
class QRect;
class QVariant;
class QRectF;

class Q_GUI_EXPORT QPolygon : public QVector<QPoint>
{
 public:
   inline QPolygon() {}
   inline explicit QPolygon(int size);

   QPolygon(const QPolygon &other) : QVector<QPoint>(other)
   {}

   QPolygon(const QVector<QPoint> &v) : QVector<QPoint>(v)
   { }

   QPolygon(QVector<QPoint> &&v)  : QVector<QPoint>(std::move(v))
   { }

   QPolygon(const QRect &rect, bool closed = false);
   QPolygon(int nPoints, const int *points);

   inline ~QPolygon() {}

   void swap(QPolygon &other) {
      QVector<QPoint>::swap(other);
   }

   QPolygon(QPolygon &&other) : QVector<QPoint>(std::move(other))
   { }

   QPolygon &operator=(QPolygon &&other)  {
      swap(other);
      return *this;
   }

   operator QVariant() const;

   void translate(int dx, int dy);
   inline void translate(const QPoint &offset);

   QPolygon translated(int dx, int dy) const;
   inline QPolygon translated(const QPoint &offset) const;

   QRect boundingRect() const;

   void point(int i, int *x, int *y) const;

   inline QPoint point(int i) const;
   inline void setPoint(int index, int x, int y);
   inline void setPoint(int index, const QPoint &p);

   void setPoints(int nPoints, const int *points);
   void setPoints(int nPoints, int firstx, int firsty, ...);
   void putPoints(int index, int nPoints, const int *points);
   void putPoints(int index, int nPoints, int firstx, int firsty, ...);
   void putPoints(int index, int nPoints, const QPolygon &from, int fromIndex = 0);

   bool containsPoint(const QPoint &pt, Qt::FillRule fillRule) const;

   QPolygon united(const QPolygon &r) const;
   QPolygon intersected(const QPolygon &r) const;
   QPolygon subtracted(const QPolygon &r) const;
};

inline QPolygon::QPolygon(int asize) : QVector<QPoint>(asize) {}

Q_GUI_EXPORT QDebug operator<<(QDebug, const QPolygon &);

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPolygon &polygon);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPolygon &polygon);

inline void QPolygon::setPoint(int index, const QPoint &pt)
{
   (*this)[index] = pt;
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

   QPolygonF(const QPolygonF &other) : QVector<QPointF>(other)
   { }

   QPolygonF(const QVector<QPointF> &v) : QVector<QPointF>(v)
   { }

   QPolygonF(QPolygonF &&other) : QVector<QPointF>(std::move(other))
   { }

   QPolygonF(QVector<QPointF> &&v) : QVector<QPointF>(std::move(v))
   { }


   QPolygonF(const QRectF &r);
   QPolygonF(const QPolygon &a);

   ~QPolygonF() {}

   QPolygonF &operator=(const QPolygonF &other) {
      QVector<QPointF>::operator=(other);
      return *this;
   }

   QPolygonF &operator=(QPolygonF &&other) {
      swap(other);
      return *this;
   }

   void swap(QPolygonF &other) {
      QVector<QPointF>::swap(other);
   }

   operator QVariant() const;

   inline void translate(qreal dx, qreal dy);
   void translate(const QPointF &offset);

   inline QPolygonF translated(qreal dx, qreal dy) const;
   QPolygonF translated(const QPointF &offset) const;

   QPolygon toPolygon() const;

   bool isClosed() const {
      return !isEmpty() && first() == last();
   }

   QRectF boundingRect() const;

   bool containsPoint(const QPointF &pt, Qt::FillRule fillRule) const;

   QPolygonF united(const QPolygonF &r) const;
   QPolygonF intersected(const QPolygonF &r) const;
   QPolygonF subtracted(const QPolygonF &r) const;
};

inline QPolygonF::QPolygonF(int asize) : QVector<QPointF>(asize) {}

Q_GUI_EXPORT QDebug operator<<(QDebug, const QPolygonF &);


Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPolygonF &array);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPolygonF &array);


inline void QPolygonF::translate(qreal dx, qreal dy)
{
   translate(QPointF(dx, dy));
}

inline QPolygonF QPolygonF::translated(qreal dx, qreal dy) const
{
   return translated(QPointF(dx, dy));
}




#endif
