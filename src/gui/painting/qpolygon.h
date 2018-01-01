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

QT_BEGIN_NAMESPACE

class QMatrix;
class QTransform;
class QRect;
class QVariant;
class QRectF;

class Q_GUI_EXPORT QPolygon : public QVector<QPoint>
{
 public:
   inline QPolygon() {}
   inline ~QPolygon() {}
   inline QPolygon(int size);
   inline QPolygon(const QPolygon &a) : QVector<QPoint>(a) {}
   inline QPolygon(const QVector<QPoint> &v) : QVector<QPoint>(v) {}
   QPolygon(const QRect &r, bool closed = false);
   QPolygon(int nPoints, const int *points);
   inline void swap(QPolygon &other) {
      QVector<QPoint>::swap(other);   // prevent QVector<QPoint><->QPolygon swaps
   }

   operator QVariant() const;

   void translate(int dx, int dy);
   void translate(const QPoint &offset);

   QPolygon translated(int dx, int dy) const;
   inline QPolygon translated(const QPoint &offset) const;

   QRect boundingRect() const;

   void point(int i, int *x, int *y) const;
   QPoint point(int i) const;
   void setPoint(int index, int x, int y);
   void setPoint(int index, const QPoint &p);
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

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPolygon &polygon);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPolygon &polygon);
#endif

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
   inline QPolygonF() {}
   inline ~QPolygonF() {}
   inline QPolygonF(int size);
   inline QPolygonF(const QPolygonF &a) : QVector<QPointF>(a) {}
   inline QPolygonF(const QVector<QPointF> &v) : QVector<QPointF>(v) {}
   QPolygonF(const QRectF &r);
   QPolygonF(const QPolygon &a);
   inline void swap(QPolygonF &other) {
      QVector<QPointF>::swap(other);   // prevent QVector<QPointF><->QPolygonF swaps
   }

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

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPolygonF &array);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPolygonF &array);
#endif

inline void QPolygonF::translate(qreal dx, qreal dy)
{
   translate(QPointF(dx, dy));
}

inline QPolygonF QPolygonF::translated(qreal dx, qreal dy) const
{
   return translated(QPointF(dx, dy));
}

QT_END_NAMESPACE


#endif // QPOLYGON_H
