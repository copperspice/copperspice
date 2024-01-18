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

#ifndef QMATRIX_H
#define QMATRIX_H

#include <qpolygon.h>
#include <qregion.h>
#include <qwindowdefs.h>
#include <qline.h>
#include <qpoint.h>
#include <qrect.h>

class QPainterPath;
class QVariant;

// 2D transform matrix

class Q_GUI_EXPORT QMatrix
{
 public:
   inline explicit QMatrix(Qt::NoDataOverload dummy) {
      (void) dummy;
   }

   QMatrix();
   QMatrix(qreal m11, qreal m12, qreal m21, qreal m22, qreal dx, qreal dy);
   QMatrix(const QMatrix &other);

   void setMatrix(qreal m11, qreal m12, qreal m21, qreal m22, qreal dx, qreal dy);

   qreal m11() const {
      return _m11;
   }

   qreal m12() const {
      return _m12;
   }

   qreal m21() const {
      return _m21;
   }

   qreal m22() const {
      return _m22;
   }

   qreal dx() const {
      return _dx;
   }

   qreal dy() const {
      return _dy;
   }

   void map(int x, int y, int *tx, int *ty) const;
   void map(qreal x, qreal y, qreal *tx, qreal *ty) const;

   QRect mapRect(const QRect &rect) const;
   QRectF mapRect(const QRectF &rect) const;

   QPoint map(const QPoint &point) const;
   QPointF map(const QPointF &point) const;
   QLine map(const QLine &line) const;
   QLineF map(const QLineF &line) const;
   QPolygonF map(const QPolygonF &polygon) const;
   QPolygon map(const QPolygon &polygon) const;
   QRegion map(const QRegion &region) const;
   QPainterPath map(const QPainterPath &path) const;
   QPolygon mapToPolygon(const QRect &rect) const;

   void reset();
   inline bool isIdentity() const;

   QMatrix &translate(qreal dx, qreal dy);
   QMatrix &scale(qreal sx, qreal sy);
   QMatrix &shear(qreal sh, qreal sv);
   QMatrix &rotate(qreal  degrees);

   bool isInvertible() const {
      return ! qFuzzyIsNull(_m11 * _m22 - _m12 * _m21);
   }

   qreal determinant() const {
      return _m11 * _m22 - _m12 * _m21;
   }

   QMatrix inverted(bool *invertible = nullptr) const;

   bool operator==(const QMatrix &matrix) const;
   bool operator!=(const QMatrix &matrix) const;

   QMatrix &operator*=(const QMatrix &matrix);
   QMatrix operator*(const QMatrix &matrix) const;

   QMatrix &operator=(const QMatrix &other);

   operator QVariant() const;

 private:
   inline QMatrix(bool)
      : _m11(1.), _m12(0.), _m21(0.), _m22(1.), _dx(0.), _dy(0.)
   {
   }


   inline QMatrix(qreal am11, qreal am12, qreal am21, qreal am22, qreal adx, qreal ady, bool)
      : _m11(am11), _m12(am12), _m21(am21), _m22(am22), _dx(adx), _dy(ady)
   {
   }

   qreal _m11;
   qreal _m12;

   qreal _m21;
   qreal _m22;

   qreal _dx;
   qreal _dy;

   friend class QTransform;
};

Q_GUI_EXPORT uint qHash(const QMatrix &key, uint seed = 0);

inline QPoint operator*(const QPoint &p, const QMatrix &m)
{
   return m.map(p);
}

inline QPointF operator*(const QPointF &p, const QMatrix &m)
{
   return m.map(p);
}

inline QLineF operator*(const QLineF &l, const QMatrix &m)
{
   return m.map(l);
}

inline QLine operator*(const QLine &l, const QMatrix &m)
{
   return m.map(l);
}

inline QPolygon operator *(const QPolygon &a, const QMatrix &m)
{
   return m.map(a);
}

inline QPolygonF operator *(const QPolygonF &a, const QMatrix &m)
{
   return m.map(a);
}

inline QRegion operator *(const QRegion &r, const QMatrix &m)
{
   return m.map(r);
}

Q_GUI_EXPORT QPainterPath operator *(const QPainterPath &p, const QMatrix &m);

inline bool QMatrix::isIdentity() const
{
   return qFuzzyIsNull(_m11 - 1) && qFuzzyIsNull(_m22 - 1) && qFuzzyIsNull(_m12)
      && qFuzzyIsNull(_m21) && qFuzzyIsNull(_dx) && qFuzzyIsNull(_dy);
}

inline bool qFuzzyCompare(const QMatrix &m1, const QMatrix &m2)
{
   return qFuzzyCompare(m1.m11(), m2.m11())
         && qFuzzyCompare(m1.m12(), m2.m12())
         && qFuzzyCompare(m1.m21(), m2.m21())
         && qFuzzyCompare(m1.m22(), m2.m22())
         && qFuzzyCompare(m1.dx(),  m2.dx())
         && qFuzzyCompare(m1.dy(),  m2.dy());
}


Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QMatrix &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QMatrix &);


Q_GUI_EXPORT QDebug operator<<(QDebug, const QMatrix &);


#endif // QMATRIX_H
