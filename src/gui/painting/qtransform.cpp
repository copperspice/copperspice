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

#include <qtransform.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qhashfunc.h>
#include <qmatrix.h>
#include <qregion.h>
#include <qpainterpath.h>
#include <qvariant.h>
#include <qmath.h>
#include <qnumeric.h>
#include <qbezier_p.h>
#include <qpainterpath_p.h>

#define Q_NEAR_CLIP (sizeof(qreal) == sizeof(double) ? 0.000001 : 0.0001)

#ifdef MAP
#  undef MAP
#endif

#define MAP(x, y, nx, ny) \
    do { \
        qreal FX_ = x;  \
        qreal FY_ = y;  \
        switch(t) {     \
        case TxNone:    \
            nx = FX_;   \
            ny = FY_;   \
            break;      \
        case TxTranslate:                         \
            nx = FX_ + affine._dx;                \
            ny = FY_ + affine._dy;                \
            break;                                \
        case TxScale:                             \
            nx = affine._m11 * FX_ + affine._dx;  \
            ny = affine._m22 * FY_ + affine._dy;  \
            break;                                \
        case TxRotate:                            \
        case TxShear:                             \
        case TxProject:                                               \
            nx = affine._m11 * FX_ + affine._m21 * FY_ + affine._dx;  \
            ny = affine._m12 * FX_ + affine._m22 * FY_ + affine._dy;  \
            if (t == TxProject) {                                     \
                qreal w = (m_13 * FX_ + m_23 * FY_ + m_33);           \
                if (w < qreal(Q_NEAR_CLIP)) w = qreal(Q_NEAR_CLIP);   \
                w = 1./w;                                             \
                nx *= w;                                              \
                ny *= w;                                              \
            }                                                         \
        }                                                             \
    } while (false)

QTransform::QTransform()
   : affine(true),
     m_13(0), m_23(0), m_33(1), m_type(TxNone), m_dirty(TxNone)

{
}

QTransform::QTransform(qreal m11, qreal m12, qreal m13,
            qreal m21, qreal m22, qreal m23, qreal m31, qreal m32, qreal m33)
   : affine(m11, m12, m21, m22, m31, m32, true),
     m_13(m13), m_23(m23), m_33(m33), m_type(TxNone), m_dirty(TxProject)
{
}

QTransform::QTransform(qreal m11, qreal m12, qreal m21, qreal m22, qreal dx, qreal dy)
   : affine(m11, m12, m21, m22, dx, dy, true),
     m_13(0), m_23(0), m_33(1), m_type(TxNone), m_dirty(TxShear)
{
}

QTransform::QTransform(const QMatrix &mtx)
   : affine(mtx._m11, mtx._m12, mtx._m21, mtx._m22, mtx._dx, mtx._dy, true),
     m_13(0), m_23(0), m_33(1), m_type(TxNone), m_dirty(TxShear)
{
}

QTransform QTransform::adjoint() const
{
   qreal m11, m12, m13, m21, m22, m23, m31, m32, m33;
   m11 = affine._m22 * m_33 - m_23 * affine._dy;
   m21 = m_23 * affine._dx - affine._m21 * m_33;
   m31 = affine._m21 * affine._dy - affine._m22 * affine._dx;
   m12 = m_13 * affine._dy - affine._m12 * m_33;
   m22 = affine._m11 * m_33 - m_13 * affine._dx;
   m32 = affine._m12 * affine._dx - affine._m11 * affine._dy;
   m13 = affine._m12 * m_23 - m_13 * affine._m22;
   m23 = m_13 * affine._m21 - affine._m11 * m_23;
   m33 = affine._m11 * affine._m22 - affine._m12 * affine._m21;

   return QTransform(m11, m12, m13, m21, m22, m23, m31, m32, m33, true);
}

QTransform QTransform::transposed() const
{
   QTransform t(affine._m11, affine._m21, affine._dx, affine._m12, affine._m22, affine._dy,
            m_13, m_23, m_33, true);
   t.m_type = m_type;
   t.m_dirty = m_dirty;

   return t;
}

QTransform QTransform::inverted(bool *invertible) const
{
   QTransform invert(true);
   bool inv = true;

   switch (inline_type()) {
      case TxNone:
         break;

      case TxTranslate:
         invert.affine._dx = -affine._dx;
         invert.affine._dy = -affine._dy;
         break;

      case TxScale:
         inv = !qFuzzyIsNull(affine._m11);
         inv &= !qFuzzyIsNull(affine._m22);

         if (inv) {
            invert.affine._m11 = 1. / affine._m11;
            invert.affine._m22 = 1. / affine._m22;
            invert.affine._dx = -affine._dx * invert.affine._m11;
            invert.affine._dy = -affine._dy * invert.affine._m22;
         }
         break;

      case TxRotate:
      case TxShear:
         invert.affine = affine.inverted(&inv);
         break;

      default:
         // general case
         qreal det = determinant();
         inv = !qFuzzyIsNull(det);
         if (inv) {
            invert = adjoint() / det;
         }
         break;
   }

   if (invertible) {
      *invertible = inv;
   }

   if (inv) {
      // inverting doesn't change the type
      invert.m_type = m_type;
      invert.m_dirty = m_dirty;
   }

   return invert;
}

QTransform &QTransform::translate(qreal dx, qreal dy)
{
   if (dx == 0 && dy == 0) {
      return *this;
   }

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   if (qIsNaN(dx) || qIsNaN(dy)) {
      qDebug("QTransform::translate() Value for x or y is invalid");
      return *this;
   }
#endif

   switch (inline_type()) {
      case TxNone:
         affine._dx = dx;
         affine._dy = dy;
         break;

      case TxTranslate:
         affine._dx += dx;
         affine._dy += dy;
         break;

      case TxScale:
         affine._dx += dx * affine._m11;
         affine._dy += dy * affine._m22;
         break;

      case TxProject:
         m_33 += dx * m_13 + dy * m_23;
         [[fallthrough]];

      case TxShear:
      case TxRotate:
         affine._dx += dx * affine._m11 + dy * affine._m21;
         affine._dy += dy * affine._m22 + dx * affine._m12;
         break;
   }

   if (m_dirty < TxTranslate) {
      m_dirty = TxTranslate;
   }
   return *this;
}

QTransform QTransform::fromTranslate(qreal dx, qreal dy)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   if (qIsNaN(dx) || qIsNaN(dy)) {
      qDebug("QTransform::fromTranslate() Value for x or y is invalid");
      return QTransform();
   }
#endif

   QTransform transform(1, 0, 0, 0, 1, 0, dx, dy, 1, true);

   if (dx == 0 && dy == 0) {
      transform.m_type = TxNone;
   } else {
      transform.m_type = TxTranslate;
   }

   transform.m_dirty = TxNone;

   return transform;
}

QTransform &QTransform::scale(qreal sx, qreal sy)
{
   if (sx == 1 && sy == 1) {
      return *this;
   }

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   if (qIsNaN(sx) || qIsNaN(sy)) {
      qDebug("QTransform::scale() Value for x or y is invalid");
      return *this;
   }
#endif

   switch (inline_type()) {
      case TxNone:
      case TxTranslate:
         affine._m11 = sx;
         affine._m22 = sy;
         break;

      case TxProject:
         m_13 *= sx;
         m_23 *= sy;
         [[fallthrough]];

      case TxRotate:
      case TxShear:
         affine._m12 *= sx;
         affine._m21 *= sy;
        [[fallthrough]];

      case TxScale:
         affine._m11 *= sx;
         affine._m22 *= sy;
         break;
   }

   if (m_dirty < TxScale) {
      m_dirty = TxScale;
   }

   return *this;
}

QTransform QTransform::fromScale(qreal sx, qreal sy)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   if (qIsNaN(sx) || qIsNaN(sy)) {
      qDebug("QTransform::fromScale() Value for x or y is invalid");
      return QTransform();
   }
#endif

   QTransform transform(sx, 0, 0, 0, sy, 0, 0, 0, 1, true);

   if (sx == 1. && sy == 1.) {
      transform.m_type = TxNone;
   } else {
      transform.m_type = TxScale;
   }

   transform.m_dirty = TxNone;

   return transform;
}

QTransform &QTransform::shear(qreal sh, qreal sv)
{
   if (sh == 0 && sv == 0) {
      return *this;
   }

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   if (qIsNaN(sh) || qIsNaN(sv)) {
      qDebug("QTransform::shear() Value for horizontal or vertical is invalid");
      return *this;
   }
#endif

   switch (inline_type()) {
      case TxNone:
      case TxTranslate:
         affine._m12 = sv;
         affine._m21 = sh;
         break;

      case TxScale:
         affine._m12 = sv * affine._m22;
         affine._m21 = sh * affine._m11;
         break;

      case TxProject: {
         qreal tm13 = sv * m_23;
         qreal tm23 = sh * m_13;
         m_13 += tm13;
         m_23 += tm23;
      }
      [[fallthrough]];

      case TxRotate:
      case TxShear: {
         qreal tm11 = sv * affine._m21;
         qreal tm22 = sh * affine._m12;
         qreal tm12 = sv * affine._m22;
         qreal tm21 = sh * affine._m11;
         affine._m11 += tm11;
         affine._m12 += tm12;
         affine._m21 += tm21;
         affine._m22 += tm22;
         break;
      }
   }

   if (m_dirty < TxShear) {
      m_dirty = TxShear;
   }

   return *this;
}

const qreal deg2rad = qreal(0.017453292519943295769);        // pi/180
const qreal inv_dist_to_plane = 1. / 1024.;

QTransform &QTransform::rotate(qreal a, Qt::Axis axis)
{
   if (a == 0) {
      return *this;
   }

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   if (qIsNaN(a)) {
      qDebug() << "QTransform::rotate() Value is invalid";
      return *this;
   }
#endif

   qreal sina = 0;
   qreal cosa = 0;

   if (a == 90. || a == -270.) {
      sina = 1.;

   } else if (a == 270. || a == -90.) {
      sina = -1.;

   } else if (a == 180.) {
      cosa = -1.;

   } else {
      qreal b = deg2rad * a;        // convert to radians
      sina = qSin(b);               // fast and convenient
      cosa = qCos(b);
   }

   if (axis == Qt::ZAxis) {
      switch (inline_type()) {
         case TxNone:
         case TxTranslate:
            affine._m11 = cosa;
            affine._m12 = sina;
            affine._m21 = -sina;
            affine._m22 = cosa;
            break;

         case TxScale: {
            qreal tm11 = cosa * affine._m11;
            qreal tm12 = sina * affine._m22;
            qreal tm21 = -sina * affine._m11;
            qreal tm22 = cosa * affine._m22;

            affine._m11 = tm11;
            affine._m12 = tm12;
            affine._m21 = tm21;
            affine._m22 = tm22;
            break;
         }

         case TxProject: {
            qreal tm13 = cosa * m_13 + sina * m_23;
            qreal tm23 = -sina * m_13 + cosa * m_23;
            m_13 = tm13;
            m_23 = tm23;
         }
         [[fallthrough]];

         case TxRotate:
         case TxShear: {
            qreal tm11 = cosa * affine._m11 + sina * affine._m21;
            qreal tm12 = cosa * affine._m12 + sina * affine._m22;
            qreal tm21 = -sina * affine._m11 + cosa * affine._m21;
            qreal tm22 = -sina * affine._m12 + cosa * affine._m22;
            affine._m11 = tm11;
            affine._m12 = tm12;
            affine._m21 = tm21;
            affine._m22 = tm22;
            break;
         }
      }

      if (m_dirty < TxRotate) {
         m_dirty = TxRotate;
      }

   } else {
      QTransform result;
      if (axis == Qt::YAxis) {
         result.affine._m11 = cosa;
         result.m_13 = -sina * inv_dist_to_plane;
      } else {
         result.affine._m22 = cosa;
         result.m_23 = -sina * inv_dist_to_plane;
      }

      result.m_type = TxProject;
      *this = result **this;
   }

   return *this;
}

QTransform &QTransform::rotateRadians(qreal a, Qt::Axis axis)
{
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
   if (qIsNaN(a)) {
      qDebug("QTransform::rotateRadians() Value is invalid");
      return *this;
   }
#endif

   qreal sina = qSin(a);
   qreal cosa = qCos(a);

   if (axis == Qt::ZAxis) {
      switch (inline_type()) {
         case TxNone:
         case TxTranslate:
            affine._m11 = cosa;
            affine._m12 = sina;
            affine._m21 = -sina;
            affine._m22 = cosa;
            break;

         case TxScale: {
            qreal tm11 = cosa * affine._m11;
            qreal tm12 = sina * affine._m22;
            qreal tm21 = -sina * affine._m11;
            qreal tm22 = cosa * affine._m22;
            affine._m11 = tm11;
            affine._m12 = tm12;
            affine._m21 = tm21;
            affine._m22 = tm22;
            break;
         }

         case TxProject: {
            qreal tm13 = cosa * m_13 + sina * m_23;
            qreal tm23 = -sina * m_13 + cosa * m_23;
            m_13 = tm13;
            m_23 = tm23;
         }
         [[fallthrough]];

         case TxRotate:
         case TxShear: {
            qreal tm11 = cosa * affine._m11 + sina * affine._m21;
            qreal tm12 = cosa * affine._m12 + sina * affine._m22;
            qreal tm21 = -sina * affine._m11 + cosa * affine._m21;
            qreal tm22 = -sina * affine._m12 + cosa * affine._m22;
            affine._m11 = tm11;
            affine._m12 = tm12;
            affine._m21 = tm21;
            affine._m22 = tm22;
            break;
         }
      }

      if (m_dirty < TxRotate) {
         m_dirty = TxRotate;
      }

   } else {
      QTransform result;
      if (axis == Qt::YAxis) {
         result.affine._m11 = cosa;
         result.m_13 = -sina * inv_dist_to_plane;
      } else {
         result.affine._m22 = cosa;
         result.m_23 = -sina * inv_dist_to_plane;
      }
      result.m_type = TxProject;
      *this = result **this;
   }

   return *this;
}

bool QTransform::operator==(const QTransform &o) const
{
   return affine._m11 == o.affine._m11 &&
      affine._m12 == o.affine._m12 &&
      affine._m21 == o.affine._m21 &&
      affine._m22 == o.affine._m22 &&
      affine._dx == o.affine._dx &&
      affine._dy == o.affine._dy &&
      m_13 == o.m_13 &&
      m_23 == o.m_23 &&
      m_33 == o.m_33;
}

uint qHash(const QTransform &key, uint seed)
{
   seed = qHash(key.m11(), seed);
   seed = qHash(key.m12(), seed);
   seed = qHash(key.m21(), seed);
   seed = qHash(key.m22(), seed);
   seed = qHash(key.dx(),  seed);
   seed = qHash(key.dy(),  seed);
   seed = qHash(key.m13(), seed);
   seed = qHash(key.m23(), seed);
   seed = qHash(key.m33(), seed);

   return seed;
}

bool QTransform::operator!=(const QTransform &o) const
{
   return !operator==(o);
}

QTransform &QTransform::operator*=(const QTransform &o)
{
   const TransformationType otherType = o.inline_type();
   if (otherType == TxNone) {
      return *this;
   }

   const TransformationType thisType = inline_type();
   if (thisType == TxNone) {
      return operator=(o);
   }

   TransformationType t = qMax(thisType, otherType);
   switch (t) {
      case TxNone:
         break;

      case TxTranslate:
         affine._dx += o.affine._dx;
         affine._dy += o.affine._dy;
         break;

      case TxScale: {
         qreal m11 = affine._m11 * o.affine._m11;
         qreal m22 = affine._m22 * o.affine._m22;

         qreal m31 = affine._dx * o.affine._m11 + o.affine._dx;
         qreal m32 = affine._dy * o.affine._m22 + o.affine._dy;

         affine._m11 = m11;
         affine._m22 = m22;
         affine._dx = m31;
         affine._dy = m32;
         break;
      }

      case TxRotate:
      case TxShear: {
         qreal m11 = affine._m11 * o.affine._m11 + affine._m12 * o.affine._m21;
         qreal m12 = affine._m11 * o.affine._m12 + affine._m12 * o.affine._m22;

         qreal m21 = affine._m21 * o.affine._m11 + affine._m22 * o.affine._m21;
         qreal m22 = affine._m21 * o.affine._m12 + affine._m22 * o.affine._m22;

         qreal m31 = affine._dx * o.affine._m11 + affine._dy * o.affine._m21 + o.affine._dx;
         qreal m32 = affine._dx * o.affine._m12 + affine._dy * o.affine._m22 + o.affine._dy;

         affine._m11 = m11;
         affine._m12 = m12;
         affine._m21 = m21;
         affine._m22 = m22;
         affine._dx = m31;
         affine._dy = m32;
         break;
      }

      case TxProject: {
         qreal m11 = affine._m11 * o.affine._m11 + affine._m12 * o.affine._m21 + m_13 * o.affine._dx;
         qreal m12 = affine._m11 * o.affine._m12 + affine._m12 * o.affine._m22 + m_13 * o.affine._dy;
         qreal m13 = affine._m11 * o.m_13 + affine._m12 * o.m_23 + m_13 * o.m_33;

         qreal m21 = affine._m21 * o.affine._m11 + affine._m22 * o.affine._m21 + m_23 * o.affine._dx;
         qreal m22 = affine._m21 * o.affine._m12 + affine._m22 * o.affine._m22 + m_23 * o.affine._dy;
         qreal m23 = affine._m21 * o.m_13 + affine._m22 * o.m_23 + m_23 * o.m_33;

         qreal m31 = affine._dx * o.affine._m11 + affine._dy * o.affine._m21 + m_33 * o.affine._dx;
         qreal m32 = affine._dx * o.affine._m12 + affine._dy * o.affine._m22 + m_33 * o.affine._dy;
         qreal m33 = affine._dx * o.m_13 + affine._dy * o.m_23 + m_33 * o.m_33;

         affine._m11 = m11;
         affine._m12 = m12;
         m_13 = m13;
         affine._m21 = m21;
         affine._m22 = m22;
         m_23 = m23;
         affine._dx = m31;
         affine._dy = m32;
         m_33 = m33;
      }
   }

   m_dirty = t;
   m_type = t;

   return *this;
}
QTransform QTransform::operator*(const QTransform &m) const
{
   const TransformationType otherType = m.inline_type();
   if (otherType == TxNone) {
      return *this;
   }

   const TransformationType thisType = inline_type();
   if (thisType == TxNone) {
      return m;
   }

   QTransform t(true);
   TransformationType type = qMax(thisType, otherType);

   switch (type) {
      case TxNone:
         break;

      case TxTranslate:
         t.affine._dx = affine._dx + m.affine._dx;
         t.affine._dy += affine._dy + m.affine._dy;
         break;

      case TxScale: {
         qreal m11 = affine._m11 * m.affine._m11;
         qreal m22 = affine._m22 * m.affine._m22;

         qreal m31 = affine._dx * m.affine._m11 + m.affine._dx;
         qreal m32 = affine._dy * m.affine._m22 + m.affine._dy;

         t.affine._m11 = m11;
         t.affine._m22 = m22;
         t.affine._dx = m31;
         t.affine._dy = m32;
         break;
      }

      case TxRotate:
      case TxShear: {
         qreal m11 = affine._m11 * m.affine._m11 + affine._m12 * m.affine._m21;
         qreal m12 = affine._m11 * m.affine._m12 + affine._m12 * m.affine._m22;

         qreal m21 = affine._m21 * m.affine._m11 + affine._m22 * m.affine._m21;
         qreal m22 = affine._m21 * m.affine._m12 + affine._m22 * m.affine._m22;

         qreal m31 = affine._dx * m.affine._m11 + affine._dy * m.affine._m21 + m.affine._dx;
         qreal m32 = affine._dx * m.affine._m12 + affine._dy * m.affine._m22 + m.affine._dy;

         t.affine._m11 = m11;
         t.affine._m12 = m12;
         t.affine._m21 = m21;
         t.affine._m22 = m22;
         t.affine._dx = m31;
         t.affine._dy = m32;
         break;
      }

      case TxProject: {
         qreal m11 = affine._m11 * m.affine._m11 + affine._m12 * m.affine._m21 + m_13 * m.affine._dx;
         qreal m12 = affine._m11 * m.affine._m12 + affine._m12 * m.affine._m22 + m_13 * m.affine._dy;
         qreal m13 = affine._m11 * m.m_13 + affine._m12 * m.m_23 + m_13 * m.m_33;

         qreal m21 = affine._m21 * m.affine._m11 + affine._m22 * m.affine._m21 + m_23 * m.affine._dx;
         qreal m22 = affine._m21 * m.affine._m12 + affine._m22 * m.affine._m22 + m_23 * m.affine._dy;
         qreal m23 = affine._m21 * m.m_13 + affine._m22 * m.m_23 + m_23 * m.m_33;

         qreal m31 = affine._dx * m.affine._m11 + affine._dy * m.affine._m21 + m_33 * m.affine._dx;
         qreal m32 = affine._dx * m.affine._m12 + affine._dy * m.affine._m22 + m_33 * m.affine._dy;
         qreal m33 = affine._dx * m.m_13 + affine._dy * m.m_23 + m_33 * m.m_33;

         t.affine._m11 = m11;
         t.affine._m12 = m12;
         t.m_13 = m13;
         t.affine._m21 = m21;
         t.affine._m22 = m22;
         t.m_23 = m23;
         t.affine._dx = m31;
         t.affine._dy = m32;
         t.m_33 = m33;
      }
   }

   t.m_dirty = type;
   t.m_type = type;

   return t;
}

QTransform &QTransform::operator=(const QTransform &matrix)
{
   affine._m11 = matrix.affine._m11;
   affine._m12 = matrix.affine._m12;
   affine._m21 = matrix.affine._m21;
   affine._m22 = matrix.affine._m22;
   affine._dx = matrix.affine._dx;
   affine._dy = matrix.affine._dy;
   m_13 = matrix.m_13;
   m_23 = matrix.m_23;
   m_33 = matrix.m_33;
   m_type = matrix.m_type;
   m_dirty = matrix.m_dirty;

   return *this;
}

void QTransform::reset()
{
   affine._m11 = affine._m22 = m_33 = 1.0;
   affine._m12 = m_13 = affine._m21 = m_23 = affine._dx = affine._dy = 0;
   m_type = TxNone;
   m_dirty = TxNone;
}

QDataStream &operator<<(QDataStream &s, const QTransform &m)
{
   s << double(m.m11())
      << double(m.m12())
      << double(m.m13())
      << double(m.m21())
      << double(m.m22())
      << double(m.m23())
      << double(m.m31())
      << double(m.m32())
      << double(m.m33());

   return s;
}

QDataStream &operator>>(QDataStream &s, QTransform &t)
{
   double m11, m12, m13, m21, m22, m23, m31, m32, m33;

   s >> m11;
   s >> m12;
   s >> m13;
   s >> m21;
   s >> m22;
   s >> m23;
   s >> m31;
   s >> m32;
   s >> m33;

   t.setMatrix(m11, m12, m13, m21, m22, m23, m31, m32, m33);

   return s;
}

QDebug operator<<(QDebug dbg, const QTransform &m)
{
   static const char *const typeStr[] = {
      "TxNone",
      "TxTranslate",
      "TxScale",
      nullptr,
      "TxRotate",
      nullptr, nullptr, nullptr,
      "TxShear",
      nullptr, nullptr, nullptr,
      nullptr, nullptr, nullptr, nullptr,
      "TxProject"
   };

   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QTransform(type=" << typeStr[m.type()] << ','
      << " 11=" << m.m11()
      << " 12=" << m.m12()
      << " 13=" << m.m13()
      << " 21=" << m.m21()
      << " 22=" << m.m22()
      << " 23=" << m.m23()
      << " 31=" << m.m31()
      << " 32=" << m.m32()
      << " 33=" << m.m33()
      << ')';

   return dbg;
}

QPoint QTransform::map(const QPoint &p) const
{
   qreal fx = p.x();
   qreal fy = p.y();

   qreal x = 0, y = 0;

   TransformationType t = inline_type();

   switch (t) {
      case TxNone:
         x = fx;
         y = fy;
         break;

      case TxTranslate:
         x = fx + affine._dx;
         y = fy + affine._dy;
         break;

      case TxScale:
         x = affine._m11 * fx + affine._dx;
         y = affine._m22 * fy + affine._dy;
         break;

      case TxRotate:
      case TxShear:
      case TxProject:
         x = affine._m11 * fx + affine._m21 * fy + affine._dx;
         y = affine._m12 * fx + affine._m22 * fy + affine._dy;
         if (t == TxProject) {
            qreal w = 1. / (m_13 * fx + m_23 * fy + m_33);
            x *= w;
            y *= w;
         }
   }

   return QPoint(qRound(x), qRound(y));
}



QPointF QTransform::map(const QPointF &p) const
{
   qreal fx = p.x();
   qreal fy = p.y();

   qreal x = 0, y = 0;

   TransformationType t = inline_type();
   switch (t) {
      case TxNone:
         x = fx;
         y = fy;
         break;

      case TxTranslate:
         x = fx + affine._dx;
         y = fy + affine._dy;
         break;

      case TxScale:
         x = affine._m11 * fx + affine._dx;
         y = affine._m22 * fy + affine._dy;
         break;

      case TxRotate:
      case TxShear:
      case TxProject:
         x = affine._m11 * fx + affine._m21 * fy + affine._dx;
         y = affine._m12 * fx + affine._m22 * fy + affine._dy;

         if (t == TxProject) {
            qreal w = 1. / (m_13 * fx + m_23 * fy + m_33);
            x *= w;
            y *= w;
         }
   }
   return QPointF(x, y);
}

QLine QTransform::map(const QLine &l) const
{
   qreal fx1 = l.x1();
   qreal fy1 = l.y1();
   qreal fx2 = l.x2();
   qreal fy2 = l.y2();

   qreal x1 = 0, y1 = 0, x2 = 0, y2 = 0;

   TransformationType t = inline_type();
   switch (t) {
      case TxNone:
         x1 = fx1;
         y1 = fy1;
         x2 = fx2;
         y2 = fy2;
         break;

      case TxTranslate:
         x1 = fx1 + affine._dx;
         y1 = fy1 + affine._dy;
         x2 = fx2 + affine._dx;
         y2 = fy2 + affine._dy;
         break;

      case TxScale:
         x1 = affine._m11 * fx1 + affine._dx;
         y1 = affine._m22 * fy1 + affine._dy;
         x2 = affine._m11 * fx2 + affine._dx;
         y2 = affine._m22 * fy2 + affine._dy;
         break;

      case TxRotate:
      case TxShear:
      case TxProject:
         x1 = affine._m11 * fx1 + affine._m21 * fy1 + affine._dx;
         y1 = affine._m12 * fx1 + affine._m22 * fy1 + affine._dy;
         x2 = affine._m11 * fx2 + affine._m21 * fy2 + affine._dx;
         y2 = affine._m12 * fx2 + affine._m22 * fy2 + affine._dy;
         if (t == TxProject) {
            qreal w = 1. / (m_13 * fx1 + m_23 * fy1 + m_33);
            x1 *= w;
            y1 *= w;
            w = 1. / (m_13 * fx2 + m_23 * fy2 + m_33);
            x2 *= w;
            y2 *= w;
         }
   }

   return QLine(qRound(x1), qRound(y1), qRound(x2), qRound(y2));
}

QLineF QTransform::map(const QLineF &l) const
{
   qreal fx1 = l.x1();
   qreal fy1 = l.y1();
   qreal fx2 = l.x2();
   qreal fy2 = l.y2();

   qreal x1 = 0, y1 = 0, x2 = 0, y2 = 0;

   TransformationType t = inline_type();
   switch (t) {
      case TxNone:
         x1 = fx1;
         y1 = fy1;
         x2 = fx2;
         y2 = fy2;
         break;

      case TxTranslate:
         x1 = fx1 + affine._dx;
         y1 = fy1 + affine._dy;
         x2 = fx2 + affine._dx;
         y2 = fy2 + affine._dy;
         break;

      case TxScale:
         x1 = affine._m11 * fx1 + affine._dx;
         y1 = affine._m22 * fy1 + affine._dy;
         x2 = affine._m11 * fx2 + affine._dx;
         y2 = affine._m22 * fy2 + affine._dy;
         break;

      case TxRotate:
      case TxShear:
      case TxProject:
         x1 = affine._m11 * fx1 + affine._m21 * fy1 + affine._dx;
         y1 = affine._m12 * fx1 + affine._m22 * fy1 + affine._dy;
         x2 = affine._m11 * fx2 + affine._m21 * fy2 + affine._dx;
         y2 = affine._m12 * fx2 + affine._m22 * fy2 + affine._dy;
         if (t == TxProject) {
            qreal w = 1. / (m_13 * fx1 + m_23 * fy1 + m_33);
            x1 *= w;
            y1 *= w;
            w = 1. / (m_13 * fx2 + m_23 * fy2 + m_33);
            x2 *= w;
            y2 *= w;
         }
   }

   return QLineF(x1, y1, x2, y2);
}

static QPolygonF mapProjective(const QTransform &transform, const QPolygonF &poly)
{
   if (poly.size() == 0) {
      return poly;
   }

   if (poly.size() == 1) {
      return QPolygonF() << transform.map(poly.at(0));
   }

   QPainterPath path;
   path.addPolygon(poly);

   path = transform.map(path);

   QPolygonF result;
   const int elementCount = path.elementCount();
   result.reserve(elementCount);

   for (int i = 0; i < elementCount; ++i) {
      result << path.elementAt(i);
   }

   return result;
}

QPolygonF QTransform::map(const QPolygonF &a) const
{
   TransformationType t = inline_type();
   if (t <= TxTranslate) {
      return a.translated(affine._dx, affine._dy);
   }

   if (t >= QTransform::TxProject) {
      return mapProjective(*this, a);
   }

   int size = a.size();
   int i;
   QPolygonF p(size);
   const QPointF *da = a.constData();
   QPointF *dp = p.data();

   for (i = 0; i < size; ++i) {
      MAP(da[i].xp, da[i].yp, dp[i].xp, dp[i].yp);
   }
   return p;
}

QPolygon QTransform::map(const QPolygon &a) const
{
   TransformationType t = inline_type();
   if (t <= TxTranslate) {
      return a.translated(qRound(affine._dx), qRound(affine._dy));
   }

   if (t >= QTransform::TxProject) {
      return mapProjective(*this, QPolygonF(a)).toPolygon();
   }

   int size = a.size();
   int i;
   QPolygon p(size);
   const QPoint *da = a.constData();
   QPoint *dp = p.data();

   for (i = 0; i < size; ++i) {
      qreal nx = 0, ny = 0;
      MAP(da[i].xp, da[i].yp, nx, ny);
      dp[i].xp = qRound(nx);
      dp[i].yp = qRound(ny);
   }

   return p;
}

extern QPainterPath qt_regionToPath(const QRegion &region);

QRegion QTransform::map(const QRegion &r) const
{
   TransformationType t = inline_type();
   if (t == TxNone) {
      return r;
   }

   if (t == TxTranslate) {
      QRegion copy(r);
      copy.translate(qRound(affine._dx), qRound(affine._dy));
      return copy;
   }

   if (t == TxScale && r.rectCount() == 1) {
      return QRegion(mapRect(r.boundingRect()));
   }

   QPainterPath p = map(qt_regionToPath(r));
   return p.toFillPolygon(QTransform()).toPolygon();
}

struct QHomogeneousCoordinate {
   qreal x;
   qreal y;
   qreal w;

   QHomogeneousCoordinate() {}
   QHomogeneousCoordinate(qreal x_, qreal y_, qreal w_) : x(x_), y(y_), w(w_) {}

   const QPointF toPoint() const {
      qreal iw = 1. / w;
      return QPointF(x * iw, y * iw);
   }
};

static inline QHomogeneousCoordinate mapHomogeneous(const QTransform &transform, const QPointF &p)
{
   QHomogeneousCoordinate c;
   c.x = transform.m11() * p.x() + transform.m21() * p.y() + transform.m31();
   c.y = transform.m12() * p.x() + transform.m22() * p.y() + transform.m32();
   c.w = transform.m13() * p.x() + transform.m23() * p.y() + transform.m33();
   return c;
}

static inline bool lineTo_clipped(QPainterPath &path, const QTransform &transform, const QPointF &a, const QPointF &b,
   bool needsMoveTo, bool needsLineTo = true)
{
   QHomogeneousCoordinate ha = mapHomogeneous(transform, a);
   QHomogeneousCoordinate hb = mapHomogeneous(transform, b);

   if (ha.w < Q_NEAR_CLIP && hb.w < Q_NEAR_CLIP) {
      return false;
   }

   if (hb.w < Q_NEAR_CLIP) {
      const qreal t = (Q_NEAR_CLIP - hb.w) / (ha.w - hb.w);

      hb.x += (ha.x - hb.x) * t;
      hb.y += (ha.y - hb.y) * t;
      hb.w = qreal(Q_NEAR_CLIP);
   } else if (ha.w < Q_NEAR_CLIP) {
      const qreal t = (Q_NEAR_CLIP - ha.w) / (hb.w - ha.w);

      ha.x += (hb.x - ha.x) * t;
      ha.y += (hb.y - ha.y) * t;
      ha.w = qreal(Q_NEAR_CLIP);

      const QPointF p = ha.toPoint();
      if (needsMoveTo) {
         path.moveTo(p);
         needsMoveTo = false;
      } else {
         path.lineTo(p);
      }
   }

   if (needsMoveTo) {
      path.moveTo(ha.toPoint());
   }

   if (needsLineTo) {
      path.lineTo(hb.toPoint());
   }

   return true;
}

Q_GUI_EXPORT bool qt_scaleForTransform(const QTransform &transform, qreal *scale);

static inline bool cubicTo_clipped(QPainterPath &path, const QTransform &transform, const QPointF &a, const QPointF &b,
   const QPointF &c, const QPointF &d, bool needsMoveTo)
{
   // Convert projective xformed curves to line
   // segments so they can be transformed more accurately

   qreal scale;
   qt_scaleForTransform(transform, &scale);

   qreal curveThreshold = scale == 0 ? qreal(0.25) : (qreal(0.25) / scale);

   QPolygonF segment = QBezier::fromPoints(a, b, c, d).toPolygon(curveThreshold);

   for (int i = 0; i < segment.size() - 1; ++i)
      if (lineTo_clipped(path, transform, segment.at(i), segment.at(i + 1), needsMoveTo)) {
         needsMoveTo = false;
      }

   return !needsMoveTo;
}

static QPainterPath mapProjective(const QTransform &transform, const QPainterPath &path)
{
   QPainterPath result;

   QPointF last;
   QPointF lastMoveTo;
   bool needsMoveTo = true;

   for (int i = 0; i < path.elementCount(); ++i) {
      switch (path.elementAt(i).type) {
         case QPainterPath::MoveToElement:
            if (i > 0 && lastMoveTo != last) {
               lineTo_clipped(result, transform, last, lastMoveTo, needsMoveTo);
            }

            lastMoveTo = path.elementAt(i);
            last = path.elementAt(i);
            needsMoveTo = true;
            break;

         case QPainterPath::LineToElement:
            if (lineTo_clipped(result, transform, last, path.elementAt(i), needsMoveTo)) {
               needsMoveTo = false;
            }
            last = path.elementAt(i);
            break;

         case QPainterPath::CurveToElement:
            if (cubicTo_clipped(result, transform, last, path.elementAt(i), path.elementAt(i + 1), path.elementAt(i + 2),
                  needsMoveTo)) {
               needsMoveTo = false;
            }
            i += 2;
            last = path.elementAt(i);
            break;

         default:
            Q_ASSERT(false);
      }
   }

   if (path.elementCount() > 0 && lastMoveTo != last) {
      lineTo_clipped(result, transform, last, lastMoveTo, needsMoveTo, false);
   }

   result.setFillRule(path.fillRule());
   return result;
}

QPainterPath QTransform::map(const QPainterPath &path) const
{
   TransformationType t = inline_type();
   if (t == TxNone || path.elementCount() == 0) {
      return path;
   }

   if (t >= TxProject) {
      return mapProjective(*this, path);
   }

   QPainterPath copy = path;

   if (t == TxTranslate) {
      copy.translate(affine._dx, affine._dy);
   } else {
      copy.detach();
      // Full xform
      for (int i = 0; i < path.elementCount(); ++i) {
         QPainterPath::Element &e = copy.d_ptr->elements[i];
         MAP(e.x, e.y, e.x, e.y);
      }
   }

   return copy;
}

QPolygon QTransform::mapToPolygon(const QRect &rect) const
{
   TransformationType t = inline_type();

   QPolygon a(4);
   qreal x[4] = { 0, 0, 0, 0 }, y[4] = { 0, 0, 0, 0 };
   if (t <= TxScale) {
      x[0] = affine._m11 * rect.x() + affine._dx;
      y[0] = affine._m22 * rect.y() + affine._dy;
      qreal w = affine._m11 * rect.width();
      qreal h = affine._m22 * rect.height();
      if (w < 0) {
         w = -w;
         x[0] -= w;
      }
      if (h < 0) {
         h = -h;
         y[0] -= h;
      }
      x[1] = x[0] + w;
      x[2] = x[1];
      x[3] = x[0];
      y[1] = y[0];
      y[2] = y[0] + h;
      y[3] = y[2];

   } else {
      qreal right = rect.x() + rect.width();
      qreal bottom = rect.y() + rect.height();
      MAP(rect.x(), rect.y(), x[0], y[0]);
      MAP(right, rect.y(), x[1], y[1]);
      MAP(right, bottom, x[2], y[2]);
      MAP(rect.x(), bottom, x[3], y[3]);
   }

   // all coordinates are correctly, tranform to a pointarray
   // (rounding to the next integer)
   a.setPoints(4, qRound(x[0]), qRound(y[0]),
      qRound(x[1]), qRound(y[1]),
      qRound(x[2]), qRound(y[2]),
      qRound(x[3]), qRound(y[3]));
   return a;
}

bool QTransform::squareToQuad(const QPolygonF &quad, QTransform &trans)
{
   if (quad.count() != 4) {
      return false;
   }

   qreal dx0 = quad[0].x();
   qreal dx1 = quad[1].x();
   qreal dx2 = quad[2].x();
   qreal dx3 = quad[3].x();

   qreal dy0 = quad[0].y();
   qreal dy1 = quad[1].y();
   qreal dy2 = quad[2].y();
   qreal dy3 = quad[3].y();

   double ax  = dx0 - dx1 + dx2 - dx3;
   double ay  = dy0 - dy1 + dy2 - dy3;

   if (!ax && !ay) { //afine transform
      trans.setMatrix(dx1 - dx0, dy1 - dy0,  0,
         dx2 - dx1, dy2 - dy1,  0,
         dx0,       dy0,  1);

   } else {
      double ax1 = dx1 - dx2;
      double ax2 = dx3 - dx2;
      double ay1 = dy1 - dy2;
      double ay2 = dy3 - dy2;

      /*determinants */
      double gtop    =  ax  * ay2 - ax2 * ay;
      double htop    =  ax1 * ay  - ax  * ay1;
      double bottom  =  ax1 * ay2 - ax2 * ay1;

      double a, b, c, d, e, f, g, h;  /*i is always 1*/

      if (!bottom) {
         return false;
      }

      g = gtop / bottom;
      h = htop / bottom;

      a = dx1 - dx0 + g * dx1;
      b = dx3 - dx0 + h * dx3;
      c = dx0;
      d = dy1 - dy0 + g * dy1;
      e = dy3 - dy0 + h * dy3;
      f = dy0;

      trans.setMatrix(a, d, g,
         b, e, h,
         c, f, 1.0);
   }

   return true;
}

bool QTransform::quadToSquare(const QPolygonF &quad, QTransform &trans)
{
   if (!squareToQuad(quad, trans)) {
      return false;
   }

   bool invertible = false;
   trans = trans.inverted(&invertible);

   return invertible;
}

bool QTransform::quadToQuad(const QPolygonF &one, const QPolygonF &two, QTransform &trans)
{
   QTransform stq;
   if (!quadToSquare(one, trans)) {
      return false;
   }

   if (!squareToQuad(two, stq)) {
      return false;
   }

   trans *= stq;

   return true;
}

void QTransform::setMatrix(qreal m11, qreal m12, qreal m13,
   qreal m21, qreal m22, qreal m23, qreal m31, qreal m32, qreal m33)
{
   affine._m11 = m11;
   affine._m12 = m12;
   m_13 = m13;

   affine._m21 = m21;
   affine._m22 = m22;
   m_23 = m23;

   affine._dx = m31;
   affine._dy = m32;
   m_33 = m33;

   m_type  = TxNone;
   m_dirty = TxProject;
}

static inline bool needsPerspectiveClipping(const QRectF &rect, const QTransform &transform)
{
   const qreal wx = qMin(transform.m13() * rect.left(), transform.m13() * rect.right());
   const qreal wy = qMin(transform.m23() * rect.top(), transform.m23() * rect.bottom());

   return wx + wy + transform.m33() < Q_NEAR_CLIP;
}

QRect QTransform::mapRect(const QRect &rect) const
{
   TransformationType t = inline_type();
   if (t <= TxTranslate) {
      return rect.translated(qRound(affine._dx), qRound(affine._dy));
   }

   if (t <= TxScale) {
      int x = qRound(affine._m11 * rect.x() + affine._dx);
      int y = qRound(affine._m22 * rect.y() + affine._dy);
      int w = qRound(affine._m11 * rect.width());
      int h = qRound(affine._m22 * rect.height());
      if (w < 0) {
         w = -w;
         x -= w;
      }

      if (h < 0) {
         h = -h;
         y -= h;
      }

      return QRect(x, y, w, h);

   } else if (t < TxProject || !needsPerspectiveClipping(rect, *this)) {
      // see mapToPolygon for explanations of the algorithm.
      qreal x = 0, y = 0;
      MAP(rect.left(), rect.top(), x, y);
      qreal xmin = x;
      qreal ymin = y;
      qreal xmax = x;
      qreal ymax = y;

      MAP(rect.right() + 1, rect.top(), x, y);
      xmin = qMin(xmin, x);
      ymin = qMin(ymin, y);
      xmax = qMax(xmax, x);
      ymax = qMax(ymax, y);

      MAP(rect.right() + 1, rect.bottom() + 1, x, y);
      xmin = qMin(xmin, x);
      ymin = qMin(ymin, y);
      xmax = qMax(xmax, x);
      ymax = qMax(ymax, y);

      MAP(rect.left(), rect.bottom() + 1, x, y);
      xmin = qMin(xmin, x);
      ymin = qMin(ymin, y);
      xmax = qMax(xmax, x);
      ymax = qMax(ymax, y);

      return QRect(qRound(xmin), qRound(ymin), qRound(xmax) - qRound(xmin), qRound(ymax) - qRound(ymin));

   } else {
      QPainterPath path;
      path.addRect(rect);
      return map(path).boundingRect().toRect();
   }
}

QRectF QTransform::mapRect(const QRectF &rect) const
{
   TransformationType t = inline_type();

   if (t <= TxTranslate) {
      return rect.translated(affine._dx, affine._dy);
   }

   if (t <= TxScale) {
      qreal x = affine._m11 * rect.x() + affine._dx;
      qreal y = affine._m22 * rect.y() + affine._dy;
      qreal w = affine._m11 * rect.width();
      qreal h = affine._m22 * rect.height();
      if (w < 0) {
         w = -w;
         x -= w;
      }
      if (h < 0) {
         h = -h;
         y -= h;
      }
      return QRectF(x, y, w, h);

   } else if (t < TxProject || !needsPerspectiveClipping(rect, *this)) {
      qreal x = 0, y = 0;
      MAP(rect.x(), rect.y(), x, y);
      qreal xmin = x;
      qreal ymin = y;
      qreal xmax = x;
      qreal ymax = y;

      MAP(rect.x() + rect.width(), rect.y(), x, y);
      xmin = qMin(xmin, x);
      ymin = qMin(ymin, y);
      xmax = qMax(xmax, x);
      ymax = qMax(ymax, y);

      MAP(rect.x() + rect.width(), rect.y() + rect.height(), x, y);
      xmin = qMin(xmin, x);
      ymin = qMin(ymin, y);
      xmax = qMax(xmax, x);
      ymax = qMax(ymax, y);

      MAP(rect.x(), rect.y() + rect.height(), x, y);
      xmin = qMin(xmin, x);
      ymin = qMin(ymin, y);
      xmax = qMax(xmax, x);
      ymax = qMax(ymax, y);
      return QRectF(xmin, ymin, xmax - xmin, ymax - ymin);

   } else {
      QPainterPath path;
      path.addRect(rect);
      return map(path).boundingRect();
   }
}

void QTransform::map(qreal x, qreal y, qreal *tx, qreal *ty) const
{
   TransformationType t = inline_type();
   MAP(x, y, *tx, *ty);
}

void QTransform::map(int x, int y, int *tx, int *ty) const
{
   TransformationType t = inline_type();
   qreal fx = 0, fy = 0;
   MAP(x, y, fx, fy);
   *tx = qRound(fx);
   *ty = qRound(fy);
}

const QMatrix &QTransform::toAffine() const
{
   return affine;
}

QTransform::TransformationType QTransform::type() const
{
   if (m_dirty == TxNone || m_dirty < m_type) {
      return static_cast<TransformationType>(m_type);
   }

   switch (static_cast<TransformationType>(m_dirty)) {
      case TxProject:
         if (! qFuzzyIsNull(m_13) || ! qFuzzyIsNull(m_23) || ! qFuzzyIsNull(m_33 - 1)) {
            m_type = TxProject;
            break;
         }
         [[fallthrough]];

      case TxShear:
      case TxRotate:
         if (! qFuzzyIsNull(affine._m12) || ! qFuzzyIsNull(affine._m21)) {
            const qreal dot = affine._m11 * affine._m12 + affine._m21 * affine._m22;
            if (qFuzzyIsNull(dot)) {
               m_type = TxRotate;
            } else {
               m_type = TxShear;
            }
            break;
         }
         [[fallthrough]];

      case TxScale:
         if (!qFuzzyIsNull(affine._m11 - 1) || !qFuzzyIsNull(affine._m22 - 1)) {
            m_type = TxScale;
            break;
         }
         [[fallthrough]];

      case TxTranslate:
         if (!qFuzzyIsNull(affine._dx) || !qFuzzyIsNull(affine._dy)) {
            m_type = TxTranslate;
            break;
         }
         [[fallthrough]];

      case TxNone:
         m_type = TxNone;
         break;
   }

   m_dirty = TxNone;
   return static_cast<TransformationType>(m_type);
}

QTransform::operator QVariant() const
{
   return QVariant(QVariant::Transform, this);
}

// returns true if the transform is uniformly scaling
// (same scale in x and y direction)
// scale is set to the max of x and y scaling factors

Q_GUI_EXPORT bool qt_scaleForTransform(const QTransform &transform, qreal *scale)
{
   const QTransform::TransformationType type = transform.type();
   if (type <= QTransform::TxTranslate) {
      if (scale) {
         *scale = 1;
      }
      return true;

   } else if (type == QTransform::TxScale) {
      const qreal xScale = qAbs(transform.m11());
      const qreal yScale = qAbs(transform.m22());

      if (scale) {
         *scale = qMax(xScale, yScale);
      }
      return qFuzzyCompare(xScale, yScale);
   }

   // rotate then scale: compare columns
   const qreal xScale1 = transform.m11() * transform.m11()
      + transform.m21() * transform.m21();

   const qreal yScale1 = transform.m12() * transform.m12()
      + transform.m22() * transform.m22();

   // scale then rotate: compare rows
   const qreal xScale2 = transform.m11() * transform.m11()
      + transform.m12() * transform.m12();

   const qreal yScale2 = transform.m21() * transform.m21()
      + transform.m22() * transform.m22();

   // decide the order of rotate and scale operations
   if (qAbs(xScale1 - yScale1) > qAbs(xScale2 - yScale2)) {
      if (scale) {
         *scale = qSqrt(qMax(xScale1, yScale1));
      }

      return type == QTransform::TxRotate && qFuzzyCompare(xScale1, yScale1);

   } else {
      if (scale) {
         *scale = qSqrt(qMax(xScale2, yScale2));
      }

      return type == QTransform::TxRotate && qFuzzyCompare(xScale2, yScale2);
   }
}
