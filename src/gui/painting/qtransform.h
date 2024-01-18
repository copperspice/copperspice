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

#ifndef QTRANSFORM_H
#define QTRANSFORM_H

#include <qmatrix.h>
#include <qpainterpath.h>
#include <qpolygon.h>
#include <qregion.h>
#include <qwindowdefs.h>
#include <qline.h>
#include <qpoint.h>
#include <qrect.h>

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

   QTransform();

   inline explicit QTransform(Qt::NoDataOverload dummy)
      : affine(Qt::NoData)
   {
      (void) dummy;
   }

   QTransform(qreal m11, qreal m12, qreal m13, qreal m21, qreal m22, qreal m23,
            qreal m31, qreal m32, qreal m33 = 1.0);

   QTransform(qreal m11, qreal m12, qreal m21, qreal m22, qreal dx, qreal dy);

   explicit QTransform(const QMatrix &matrix);

   QTransform(const QTransform &other) {
      *this = other;
   }

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

   void setMatrix(qreal m11, qreal m12, qreal m13, qreal m21, qreal m22, qreal m23,
                  qreal m31, qreal m32, qreal m33);

   [[nodiscard]] QTransform inverted(bool *invertible = nullptr) const;
   [[nodiscard]] QTransform adjoint() const;
   [[nodiscard]] QTransform transposed() const;

   QTransform &translate(qreal dx, qreal dy);
   QTransform &scale(qreal sx, qreal sy);
   QTransform &shear(qreal sh, qreal sv);
   QTransform &rotate(qreal angle, Qt::Axis axis = Qt::ZAxis);
   QTransform &rotateRadians(qreal angle, Qt::Axis axis = Qt::ZAxis);

   static bool squareToQuad(const QPolygonF &quad, QTransform &result);
   static bool quadToSquare(const QPolygonF &quad, QTransform &result);
   static bool quadToQuad(const QPolygonF &quad1, const QPolygonF &quad2, QTransform &result);

   void reset();

   QPoint       map(const QPoint &point) const;
   QPointF      map(const QPointF &point) const;
   QLine        map(const QLine &line) const;
   QLineF       map(const QLineF &line) const;
   QPolygonF    map(const QPolygonF &polygon) const;
   QPolygon     map(const QPolygon &polygon) const;
   QRegion      map(const QRegion &region) const;
   QPainterPath map(const QPainterPath &path) const;
   QPolygon     mapToPolygon(const QRect &rect) const;

   QRect mapRect(const QRect &rect) const;
   QRectF mapRect(const QRectF &rect) const;

   void map(int x, int y, int *tx, int *ty) const;
   void map(qreal x, qreal y, qreal *tx, qreal *ty) const;

   const QMatrix &toAffine() const;

   static QTransform fromTranslate(qreal dx, qreal dy);
   static QTransform fromScale(qreal sx, qreal sy);

   // operators
   operator QVariant() const;      // emerald - review design

   bool operator==(const QTransform &transform) const;
   bool operator!=(const QTransform &transform) const;

   QTransform &operator*=(const QTransform &transform);
   QTransform operator*(const QTransform &transform) const;

   QTransform &operator=(const QTransform &other);

   inline QTransform &operator*=(qreal factor);
   inline QTransform &operator/=(qreal factor);
   inline QTransform &operator+=(qreal delta);
   inline QTransform &operator-=(qreal delta);

 private:
   inline QTransform(qreal m11, qreal m12, qreal m13, qreal m21, qreal m22, qreal m23,
            qreal m31, qreal m32, qreal m33, bool)
      : affine(m11, m12, m21, m22, m31, m32, true),
        m_13(m13), m_23(m23), m_33(m33), m_type(TxNone), m_dirty(TxProject)
   {
   }

   inline QTransform(bool)
      : affine(true), m_13(0), m_23(0), m_33(1), m_type(TxNone), m_dirty(TxNone)
   {
   }

   inline TransformationType inline_type() const;
   QMatrix affine;

   qreal m_13;
   qreal m_23;
   qreal m_33;

   mutable uint m_type  : 5;
   mutable uint m_dirty : 5;
};

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

inline QTransform &QTransform::operator*=(qreal factor)
{
   if (factor == 1.) {
      return *this;
   }

   affine._m11 *= factor;
   affine._m12 *= factor;
   m_13        *= factor;
   affine._m21 *= factor;
   affine._m22 *= factor;
   m_23        *= factor;
   affine._dx  *= factor;
   affine._dy  *= factor;
   m_33        *= factor;

   if (m_dirty < TxScale) {
      m_dirty = TxScale;
   }

   return *this;
}

inline QTransform &QTransform::operator/=(qreal factor)
{
   if (factor == 0) {
      return *this;
   }

   factor = 1 / factor;

   return operator*=(factor);
}

inline QTransform &QTransform::operator+=(qreal delta)
{
   if (delta == 0) {
      return *this;
   }

   affine._m11 += delta;
   affine._m12 += delta;
   m_13        += delta;
   affine._m21 += delta;
   affine._m22 += delta;
   m_23        += delta;
   affine._dx  += delta;
   affine._dy  += delta;
   m_33        += delta;

   m_dirty     = TxProject;

   return *this;
}

inline QTransform &QTransform::operator-=(qreal delta)
{
   if (delta == 0) {
      return *this;
   }

   affine._m11 -= delta;
   affine._m12 -= delta;
   m_13        -= delta;
   affine._m21 -= delta;
   affine._m22 -= delta;
   m_23        -= delta;
   affine._dx  -= delta;
   affine._dy  -= delta;
   m_33        -= delta;

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

inline QPolygon operator*(const QPolygon &a, const QTransform &m)
{
   return m.map(a);
}

inline QPolygonF operator*(const QPolygonF &a, const QTransform &m)
{
   return m.map(a);
}

inline QRegion operator *(const QRegion &r, const QTransform &m)
{
   return m.map(r);
}

inline QPainterPath operator*(const QPainterPath &p, const QTransform &m)
{
   return m.map(p);
}

inline QTransform operator*(const QTransform &a, qreal n)
{
   QTransform t(a);
   t *= n;
   return t;
}

inline QTransform operator/(const QTransform &a, qreal n)
{
   QTransform t(a);
   t /= n;
   return t;
}

inline QTransform operator+(const QTransform &a, qreal n)
{
   QTransform t(a);
   t += n;
   return t;
}

inline QTransform operator-(const QTransform &a, qreal n)
{
   QTransform t(a);
   t -= n;
   return t;
}

#endif
