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

#ifndef QTRANSFORM_H
#define QTRANSFORM_H

#include <QtGui/qmatrix.h>
#include <QtGui/qpainterpath.h>
#include <QtGui/qpolygon.h>
#include <QtGui/qregion.h>
#include <QtGui/qwindowdefs.h>
#include <QtCore/qline.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>

class QVariant;

class Q_GUI_EXPORT QTransform
{
 public:
   enum TransformationType {
      TxNone      = 0x00,
      TxTranslate = 0x01,
      TxScale     = 0x02,
      TxRotate    = 0x04,
      TxShear     = 0x08,
      TxProject   = 0x10
   };

   inline explicit QTransform(Qt::Initialization) : affine(Qt::Uninitialized) {}
   QTransform();
   QTransform(qreal h11, qreal h12, qreal h13,
      qreal h21, qreal h22, qreal h23,
      qreal h31, qreal h32, qreal h33 = 1.0);
   QTransform(qreal h11, qreal h12, qreal h21,
      qreal h22, qreal dx, qreal dy);
   explicit QTransform(const QMatrix &mtx);

   inline bool isAffine() const;
   inline bool isIdentity() const;
   inline bool isInvertible() const;
   inline bool isScaling() const;
   inline bool isRotating() const;
   inline bool isTranslating() const;

   TransformationType type() const;

   inline qreal determinant() const;
   inline qreal det() const;

   inline qreal m11() const;
   inline qreal m12() const;
   inline qreal m13() const;
   inline qreal m21() const;
   inline qreal m22() const;
   inline qreal m23() const;
   inline qreal m31() const;
   inline qreal m32() const;
   inline qreal m33() const;

   inline qreal dx() const;
   inline qreal dy() const;

   void setMatrix(qreal m11, qreal m12, qreal m13, qreal m21, qreal m22, qreal m23, qreal m31, qreal m32, qreal m33);

   QTransform inverted(bool *invertible = nullptr) const;
   QTransform adjoint() const;
   QTransform transposed() const;

   QTransform &translate(qreal dx, qreal dy);
   QTransform &scale(qreal sx, qreal sy);
   QTransform &shear(qreal sh, qreal sv);
   QTransform &rotate(qreal a, Qt::Axis axis = Qt::ZAxis);
   QTransform &rotateRadians(qreal a, Qt::Axis axis = Qt::ZAxis);

   static bool squareToQuad(const QPolygonF &square, QTransform &result);
   static bool quadToSquare(const QPolygonF &quad, QTransform &result);
   static bool quadToQuad(const QPolygonF &one, const QPolygonF &two, QTransform &result);

   bool operator==(const QTransform &) const;
   bool operator!=(const QTransform &) const;

   QTransform &operator*=(const QTransform &);
   QTransform operator*(const QTransform &o) const;

   QTransform &operator=(const QTransform &);

   operator QVariant() const;

   void reset();
   QPoint       map(const QPoint &p) const;
   QPointF      map(const QPointF &p) const;
   QLine        map(const QLine &l) const;
   QLineF       map(const QLineF &l) const;
   QPolygonF    map(const QPolygonF &a) const;
   QPolygon     map(const QPolygon &a) const;
   QRegion      map(const QRegion &r) const;
   QPainterPath map(const QPainterPath &p) const;
   QPolygon     mapToPolygon(const QRect &r) const;
   QRect mapRect(const QRect &) const;
   QRectF mapRect(const QRectF &) const;
   void map(int x, int y, int *tx, int *ty) const;
   void map(qreal x, qreal y, qreal *tx, qreal *ty) const;

   const QMatrix &toAffine() const;

   inline QTransform &operator*=(qreal div);
   inline QTransform &operator/=(qreal div);
   inline QTransform &operator+=(qreal div);
   inline QTransform &operator-=(qreal div);

   static QTransform fromTranslate(qreal dx, qreal dy);
   static QTransform fromScale(qreal dx, qreal dy);

 private:
   inline QTransform(qreal h11, qreal h12, qreal h13, qreal h21, qreal h22, qreal h23, qreal h31, qreal h32, qreal h33, bool)
      : affine(h11, h12, h21, h22, h31, h32, true)
      , m_13(h13), m_23(h23), m_33(h33)
      , m_type(TxNone)
      , m_dirty(TxProject), d(nullptr)
   {  }
   inline QTransform(bool)
      : affine(true)
      , m_13(0), m_23(0), m_33(1)
      , m_type(TxNone)
      , m_dirty(TxNone), d(nullptr)
   {  }
   inline TransformationType inline_type() const;
   QMatrix affine;
   qreal   m_13;
   qreal   m_23;
   qreal   m_33;

   mutable uint m_type : 5;
   mutable uint m_dirty : 5;

   class Private;
   Private *d;
};

Q_DECLARE_TYPEINFO(QTransform, Q_MOVABLE_TYPE);

Q_GUI_EXPORT uint qHash(const QTransform &key, uint seed = 0);
inline QTransform::TransformationType QTransform::inline_type() const
{
   if (m_dirty == TxNone) {
      return static_cast<TransformationType>(m_type);
   }

   return type();
}

inline bool QTransform::isAffine() const
{
   return inline_type() < TxProject;
}

inline bool QTransform::isIdentity() const
{
   return inline_type() == TxNone;
}

inline bool QTransform::isInvertible() const
{
   return !qFuzzyIsNull(determinant());
}

inline bool QTransform::isScaling() const
{
   return type() >= TxScale;
}
inline bool QTransform::isRotating() const
{
   return inline_type() >= TxRotate;
}

inline bool QTransform::isTranslating() const
{
   return inline_type() >= TxTranslate;
}

inline qreal QTransform::determinant() const
{
   return affine._m11 * (m_33 * affine._m22 - affine._dy * m_23) -
      affine._m21 * (m_33 * affine._m12 - affine._dy * m_13) + affine._dx * (m_23 * affine._m12 - affine._m22 * m_13);
}
inline qreal QTransform::det() const
{
   return determinant();
}
inline qreal QTransform::m11() const
{
   return affine._m11;
}
inline qreal QTransform::m12() const
{
   return affine._m12;
}
inline qreal QTransform::m13() const
{
   return m_13;
}
inline qreal QTransform::m21() const
{
   return affine._m21;
}
inline qreal QTransform::m22() const
{
   return affine._m22;
}
inline qreal QTransform::m23() const
{
   return m_23;
}
inline qreal QTransform::m31() const
{
   return affine._dx;
}
inline qreal QTransform::m32() const
{
   return affine._dy;
}
inline qreal QTransform::m33() const
{
   return m_33;
}
inline qreal QTransform::dx() const
{
   return affine._dx;
}
inline qreal QTransform::dy() const
{
   return affine._dy;
}

inline QTransform &QTransform::operator*=(qreal num)
{
   if (num == 1.) {
      return *this;
   }

   affine._m11 *= num;
   affine._m12 *= num;
   m_13        *= num;
   affine._m21 *= num;
   affine._m22 *= num;
   m_23        *= num;
   affine._dx  *= num;
   affine._dy  *= num;
   m_33        *= num;

   if (m_dirty < TxScale) {
      m_dirty = TxScale;
   }
   return *this;
}

inline QTransform &QTransform::operator/=(qreal div)
{
   if (div == 0) {
      return *this;
   }
   div = 1 / div;
   return operator*=(div);
}

inline QTransform &QTransform::operator+=(qreal num)
{
   if (num == 0) {
      return *this;
   }

   affine._m11 += num;
   affine._m12 += num;
   m_13        += num;
   affine._m21 += num;
   affine._m22 += num;
   m_23        += num;
   affine._dx  += num;
   affine._dy  += num;
   m_33        += num;
   m_dirty     = TxProject;
   return *this;
}

inline QTransform &QTransform::operator-=(qreal num)
{
   if (num == 0) {
      return *this;
   }

   affine._m11 -= num;
   affine._m12 -= num;
   m_13        -= num;
   affine._m21 -= num;
   affine._m22 -= num;
   m_23        -= num;
   affine._dx  -= num;
   affine._dy  -= num;
   m_33        -= num;
   m_dirty     = TxProject;
   return *this;
}

inline bool qFuzzyCompare(const QTransform &t1, const QTransform &t2)
{
   return qFuzzyCompare(t1.m11(), t2.m11())
      && qFuzzyCompare(t1.m12(), t2.m12())
      && qFuzzyCompare(t1.m13(), t2.m13())
      && qFuzzyCompare(t1.m21(), t2.m21())
      && qFuzzyCompare(t1.m22(), t2.m22())
      && qFuzzyCompare(t1.m23(), t2.m23())
      && qFuzzyCompare(t1.m31(), t2.m31())
      && qFuzzyCompare(t1.m32(), t2.m32())
      && qFuzzyCompare(t1.m33(), t2.m33());
}



Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QTransform &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QTransform &);


Q_GUI_EXPORT QDebug operator<<(QDebug, const QTransform &);

inline QPoint operator*(const QPoint &p, const QTransform &m)
{
   return m.map(p);
}

inline QPointF operator*(const QPointF &p, const QTransform &m)
{
   return m.map(p);
}

inline QLineF operator*(const QLineF &l, const QTransform &m)
{
   return m.map(l);
}

inline QLine operator*(const QLine &l, const QTransform &m)
{
   return m.map(l);
}

inline QPolygon operator *(const QPolygon &a, const QTransform &m)
{
   return m.map(a);
}

inline QPolygonF operator *(const QPolygonF &a, const QTransform &m)
{
   return m.map(a);
}

inline QRegion operator *(const QRegion &r, const QTransform &m)
{
   return m.map(r);
}

inline QPainterPath operator *(const QPainterPath &p, const QTransform &m)
{
   return m.map(p);
}

inline QTransform operator *(const QTransform &a, qreal n)
{
   QTransform t(a);
   t *= n;
   return t;
}

inline QTransform operator /(const QTransform &a, qreal n)
{
   QTransform t(a);
   t /= n;
   return t;
}

inline QTransform operator +(const QTransform &a, qreal n)
{
   QTransform t(a);
   t += n;
   return t;
}

inline QTransform operator -(const QTransform &a, qreal n)
{
   QTransform t(a);
   t -= n;
   return t;
}


#endif
