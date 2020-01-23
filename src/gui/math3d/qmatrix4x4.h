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

#ifndef QMATRIX4X4_H
#define QMATRIX4X4_H

#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>
#include <QtGui/qquaternion.h>
#include <QtGui/qgenericmatrix.h>
#include <QtCore/qrect.h>

#ifndef QT_NO_MATRIX4X4

class QMatrix;
class QTransform;
class QVariant;

class Q_GUI_EXPORT QMatrix4x4
{

 public:
   inline QMatrix4x4() {
      setToIdentity();
   }

   explicit QMatrix4x4(const qreal *values);
   inline QMatrix4x4(qreal m11, qreal m12, qreal m13, qreal m14,
                     qreal m21, qreal m22, qreal m23, qreal m24,
                     qreal m31, qreal m32, qreal m33, qreal m34,
                     qreal m41, qreal m42, qreal m43, qreal m44);

   template <int N, int M>
   explicit QMatrix4x4(const QGenericMatrix<N, M, qreal> &matrix);

   QMatrix4x4(const qreal *values, int cols, int rows);
   QMatrix4x4(const QTransform &transform);
   QMatrix4x4(const QMatrix &matrix);

   inline const qreal &operator()(int row, int column) const;
   inline qreal &operator()(int row, int column);

#ifndef QT_NO_VECTOR4D
   inline QVector4D column(int index) const;
   inline void setColumn(int index, const QVector4D &value);

   inline QVector4D row(int index) const;
   inline void setRow(int index, const QVector4D &value);
#endif

   inline bool isAffine() const;
   inline bool isIdentity() const;
   inline void setToIdentity();

   inline void fill(qreal value);

   qreal determinant() const;
   QMatrix4x4 inverted(bool *invertible = nullptr) const;
   QMatrix4x4 transposed() const;
   QMatrix3x3 normalMatrix() const;

   inline QMatrix4x4 &operator+=(const QMatrix4x4 &other);
   inline QMatrix4x4 &operator-=(const QMatrix4x4 &other);
   inline QMatrix4x4 &operator*=(QMatrix4x4 other);
   inline QMatrix4x4 &operator*=(qreal factor);

   QMatrix4x4 &operator/=(qreal divisor);
   inline bool operator==(const QMatrix4x4 &other) const;
   inline bool operator!=(const QMatrix4x4 &other) const;

   friend QMatrix4x4 operator+(const QMatrix4x4 &m1, const QMatrix4x4 &m2);
   friend QMatrix4x4 operator-(const QMatrix4x4 &m1, const QMatrix4x4 &m2);
   friend QMatrix4x4 operator*(const QMatrix4x4 &m1, const QMatrix4x4 &m2);

#ifndef QT_NO_VECTOR3D
   friend QVector3D operator*(const QMatrix4x4 &matrix, const QVector3D &vector);
   friend QVector3D operator*(const QVector3D &vector, const QMatrix4x4 &matrix);
#endif

#ifndef QT_NO_VECTOR4D
   friend QVector4D operator*(const QVector4D &vector, const QMatrix4x4 &matrix);
   friend QVector4D operator*(const QMatrix4x4 &matrix, const QVector4D &vector);
#endif

   friend QPoint operator*(const QPoint &point, const QMatrix4x4 &matrix);
   friend QPointF operator*(const QPointF &point, const QMatrix4x4 &matrix);
   friend QMatrix4x4 operator-(const QMatrix4x4 &matrix);
   friend QPoint operator*(const QMatrix4x4 &matrix, const QPoint &point);
   friend QPointF operator*(const QMatrix4x4 &matrix, const QPointF &point);
   friend QMatrix4x4 operator*(qreal factor, const QMatrix4x4 &matrix);
   friend QMatrix4x4 operator*(const QMatrix4x4 &matrix, qreal factor);
   friend Q_GUI_EXPORT QMatrix4x4 operator/(const QMatrix4x4 &matrix, qreal divisor);

   friend inline bool qFuzzyCompare(const QMatrix4x4 &m1, const QMatrix4x4 &m2);

#ifndef QT_NO_VECTOR3D
   void scale(const QVector3D &vector);
   void translate(const QVector3D &vector);
   void rotate(qreal angle, const QVector3D &vector);
#endif

   void scale(qreal x, qreal y);
   void scale(qreal x, qreal y, qreal z);
   void scale(qreal factor);
   void translate(qreal x, qreal y);
   void translate(qreal x, qreal y, qreal z);
   void rotate(qreal angle, qreal x, qreal y, qreal z = 0.0f);
#ifndef QT_NO_QUATERNION
   void rotate(const QQuaternion &quaternion);
#endif

   void ortho(const QRect &rect);
   void ortho(const QRectF &rect);
   void ortho(qreal left, qreal right, qreal bottom, qreal top, qreal nearPlane, qreal farPlane);
   void frustum(qreal left, qreal right, qreal bottom, qreal top, qreal nearPlane, qreal farPlane);
   void perspective(qreal angle, qreal aspect, qreal nearPlane, qreal farPlane);

#ifndef QT_NO_VECTOR3D
   void lookAt(const QVector3D &eye, const QVector3D &center, const QVector3D &up);
#endif

   void viewport(const QRectF &rect);
   void viewport(qreal left, qreal bottom, qreal width, qreal height, qreal nearPlane = 0.0, qreal farPlane = 1.0);
   void flipCoordinates();

   void copyDataTo(qreal *values) const;

   QMatrix toAffine() const;
   QTransform toTransform() const;
   QTransform toTransform(qreal distanceToPlane) const;

   QPoint map(const QPoint &point) const;
   QPointF map(const QPointF &point) const;

#ifndef QT_NO_VECTOR3D
   QVector3D map(const QVector3D &point) const;
   QVector3D mapVector(const QVector3D &vector) const;
#endif

#ifndef QT_NO_VECTOR4D
   QVector4D map(const QVector4D &point) const;
#endif
   QRect mapRect(const QRect &rect) const;
   QRectF mapRect(const QRectF &rect) const;

   template <int N, int M>
   QGenericMatrix<N, M, qreal> toGenericMatrix() const;

   inline qreal *data();
   inline const qreal *data() const {
      return *m;
   }

   inline const qreal *constData() const {
      return *m;
   }

   void optimize();

   operator QVariant() const;

   friend Q_GUI_EXPORT QDebug operator<<(QDebug dbg, const QMatrix4x4 &m);

 private:
   qreal m[4][4];          // Column-major order to match OpenGL.
   int flagBits;           // Flag bits from the enum below.

   enum {
      Identity        = 0x0000,   // Identity matrix
      Translation     = 0x0001,   // Contains a simple translation
      Scale           = 0x0002,   // Contains a simple scale
      Rotation2D      = 0x0004,   // Contains a rotation about the Z axis
      Rotation        = 0x0008,   // Contains a simple rotation
      Perspective     = 0x0010,   // Last row is different from (0, 0, 0, 1)
      General         = 0x001f    // General matrix, unknown contents
   };

   // Construct without initializing identity matrix.
   explicit QMatrix4x4(int) {
   }

   QMatrix4x4 orthonormalInverse() const;

   void projectedRotate(qreal angle, qreal x, qreal y, qreal z);

   friend class QGraphicsRotation;
};

Q_DECLARE_TYPEINFO(QMatrix4x4, Q_MOVABLE_TYPE);

inline QMatrix4x4::QMatrix4x4
(qreal m11, qreal m12, qreal m13, qreal m14,
 qreal m21, qreal m22, qreal m23, qreal m24,
 qreal m31, qreal m32, qreal m33, qreal m34,
 qreal m41, qreal m42, qreal m43, qreal m44)
{
   m[0][0] = m11;
   m[0][1] = m21;
   m[0][2] = m31;
   m[0][3] = m41;
   m[1][0] = m12;
   m[1][1] = m22;
   m[1][2] = m32;
   m[1][3] = m42;
   m[2][0] = m13;
   m[2][1] = m23;
   m[2][2] = m33;
   m[2][3] = m43;
   m[3][0] = m14;
   m[3][1] = m24;
   m[3][2] = m34;
   m[3][3] = m44;
   flagBits = General;
}

template <int N, int M>
inline QMatrix4x4::QMatrix4x4(const QGenericMatrix<N, M, qreal> &matrix)
{
   const qreal *values = matrix.constData();
   for (int matrixCol = 0; matrixCol < 4; ++matrixCol) {
      for (int matrixRow = 0; matrixRow < 4; ++matrixRow) {
         if (matrixCol < N && matrixRow < M) {
            m[matrixCol][matrixRow] = values[matrixCol * M + matrixRow];
         } else if (matrixCol == matrixRow) {
            m[matrixCol][matrixRow] = 1.0f;
         } else {
            m[matrixCol][matrixRow] = 0.0f;
         }
      }
   }

   flagBits = General;
}

template <int N, int M>
QGenericMatrix<N, M, qreal> QMatrix4x4::toGenericMatrix() const
{
   QGenericMatrix<N, M, qreal> result;
   qreal *values = result.data();

   for (int matrixCol = 0; matrixCol < N; ++matrixCol) {
      for (int matrixRow = 0; matrixRow < M; ++matrixRow) {
         if (matrixCol < 4 && matrixRow < 4) {
            values[matrixCol * M + matrixRow] = m[matrixCol][matrixRow];
         } else if (matrixCol == matrixRow) {
            values[matrixCol * M + matrixRow] = 1.0f;
         } else {
            values[matrixCol * M + matrixRow] = 0.0f;
         }
      }
   }
   return result;
}

inline const qreal &QMatrix4x4::operator()(int aRow, int aColumn) const
{
   Q_ASSERT(aRow >= 0 && aRow < 4 && aColumn >= 0 && aColumn < 4);
   return m[aColumn][aRow];
}

inline qreal &QMatrix4x4::operator()(int aRow, int aColumn)
{
   Q_ASSERT(aRow >= 0 && aRow < 4 && aColumn >= 0 && aColumn < 4);
   flagBits = General;
   return m[aColumn][aRow];
}

#ifndef QT_NO_VECTOR4D
inline QVector4D QMatrix4x4::column(int index) const
{
   Q_ASSERT(index >= 0 && index < 4);
   return QVector4D(m[index][0], m[index][1], m[index][2], m[index][3]);
}

inline void QMatrix4x4::setColumn(int index, const QVector4D &value)
{
   Q_ASSERT(index >= 0 && index < 4);
   m[index][0] = value.x();
   m[index][1] = value.y();
   m[index][2] = value.z();
   m[index][3] = value.w();
   flagBits = General;
}

inline QVector4D QMatrix4x4::row(int index) const
{
   Q_ASSERT(index >= 0 && index < 4);
   return QVector4D(m[0][index], m[1][index], m[2][index], m[3][index]);
}

inline void QMatrix4x4::setRow(int index, const QVector4D &value)
{
   Q_ASSERT(index >= 0 && index < 4);
   m[0][index] = value.x();
   m[1][index] = value.y();
   m[2][index] = value.z();
   m[3][index] = value.w();
   flagBits = General;
}
#endif

Q_GUI_EXPORT QMatrix4x4 operator/(const QMatrix4x4 &matrix, qreal divisor);

inline bool QMatrix4x4::isAffine() const
{
    return m[0][3] == 0.0f && m[1][3] == 0.0f && m[2][3] == 0.0f && m[3][3] == 1.0f;
}
inline bool QMatrix4x4::isIdentity() const
{
   if (flagBits == Identity) {
      return true;
   }
   if (m[0][0] != 1.0f || m[0][1] != 0.0f || m[0][2] != 0.0f) {
      return false;
   }
   if (m[0][3] != 0.0f || m[1][0] != 0.0f || m[1][1] != 1.0f) {
      return false;
   }
   if (m[1][2] != 0.0f || m[1][3] != 0.0f || m[2][0] != 0.0f) {
      return false;
   }
   if (m[2][1] != 0.0f || m[2][2] != 1.0f || m[2][3] != 0.0f) {
      return false;
   }
   if (m[3][0] != 0.0f || m[3][1] != 0.0f || m[3][2] != 0.0f) {
      return false;
   }
   return (m[3][3] == 1.0f);
}

inline void QMatrix4x4::setToIdentity()
{
   m[0][0] = 1.0f;
   m[0][1] = 0.0f;
   m[0][2] = 0.0f;
   m[0][3] = 0.0f;
   m[1][0] = 0.0f;
   m[1][1] = 1.0f;
   m[1][2] = 0.0f;
   m[1][3] = 0.0f;
   m[2][0] = 0.0f;
   m[2][1] = 0.0f;
   m[2][2] = 1.0f;
   m[2][3] = 0.0f;
   m[3][0] = 0.0f;
   m[3][1] = 0.0f;
   m[3][2] = 0.0f;
   m[3][3] = 1.0f;
   flagBits = Identity;
}

inline void QMatrix4x4::fill(qreal value)
{
   m[0][0] = value;
   m[0][1] = value;
   m[0][2] = value;
   m[0][3] = value;
   m[1][0] = value;
   m[1][1] = value;
   m[1][2] = value;
   m[1][3] = value;
   m[2][0] = value;
   m[2][1] = value;
   m[2][2] = value;
   m[2][3] = value;
   m[3][0] = value;
   m[3][1] = value;
   m[3][2] = value;
   m[3][3] = value;
   flagBits = General;
}

inline QMatrix4x4 &QMatrix4x4::operator+=(const QMatrix4x4 &other)
{
   m[0][0] += other.m[0][0];
   m[0][1] += other.m[0][1];
   m[0][2] += other.m[0][2];
   m[0][3] += other.m[0][3];
   m[1][0] += other.m[1][0];
   m[1][1] += other.m[1][1];
   m[1][2] += other.m[1][2];
   m[1][3] += other.m[1][3];
   m[2][0] += other.m[2][0];
   m[2][1] += other.m[2][1];
   m[2][2] += other.m[2][2];
   m[2][3] += other.m[2][3];
   m[3][0] += other.m[3][0];
   m[3][1] += other.m[3][1];
   m[3][2] += other.m[3][2];
   m[3][3] += other.m[3][3];
   flagBits = General;
   return *this;
}

inline QMatrix4x4 &QMatrix4x4::operator-=(const QMatrix4x4 &other)
{
   m[0][0] -= other.m[0][0];
   m[0][1] -= other.m[0][1];
   m[0][2] -= other.m[0][2];
   m[0][3] -= other.m[0][3];
   m[1][0] -= other.m[1][0];
   m[1][1] -= other.m[1][1];
   m[1][2] -= other.m[1][2];
   m[1][3] -= other.m[1][3];
   m[2][0] -= other.m[2][0];
   m[2][1] -= other.m[2][1];
   m[2][2] -= other.m[2][2];
   m[2][3] -= other.m[2][3];
   m[3][0] -= other.m[3][0];
   m[3][1] -= other.m[3][1];
   m[3][2] -= other.m[3][2];
   m[3][3] -= other.m[3][3];
   flagBits = General;
   return *this;
}

inline QMatrix4x4 &QMatrix4x4::operator*=(QMatrix4x4 other)
{
    flagBits |= other.flagBits;
    if (flagBits < Rotation2D) {
        m[3][0] += m[0][0] * other.m[3][0];
        m[3][1] += m[1][1] * other.m[3][1];
        m[3][2] += m[2][2] * other.m[3][2];

        m[0][0] *= other.m[0][0];
        m[1][1] *= other.m[1][1];
        m[2][2] *= other.m[2][2];
        return *this;
    }

    qreal m0, m1, m2;

    m0 = m[0][0] * other.m[0][0]
            + m[1][0] * other.m[0][1]
            + m[2][0] * other.m[0][2]
            + m[3][0] * other.m[0][3];
    m1 = m[0][0] * other.m[1][0]
            + m[1][0] * other.m[1][1]
            + m[2][0] * other.m[1][2]
            + m[3][0] * other.m[1][3];
    m2 = m[0][0] * other.m[2][0]
            + m[1][0] * other.m[2][1]
            + m[2][0] * other.m[2][2]
            + m[3][0] * other.m[2][3];
    m[3][0] = m[0][0] * other.m[3][0]
            + m[1][0] * other.m[3][1]
            + m[2][0] * other.m[3][2]
            + m[3][0] * other.m[3][3];
    m[0][0] = m0;
    m[1][0] = m1;
    m[2][0] = m2;

    m0 = m[0][1] * other.m[0][0]
            + m[1][1] * other.m[0][1]
            + m[2][1] * other.m[0][2]
            + m[3][1] * other.m[0][3];
    m1 = m[0][1] * other.m[1][0]
            + m[1][1] * other.m[1][1]
            + m[2][1] * other.m[1][2]
            + m[3][1] * other.m[1][3];
    m2 = m[0][1] * other.m[2][0]
            + m[1][1] * other.m[2][1]
            + m[2][1] * other.m[2][2]
            + m[3][1] * other.m[2][3];
    m[3][1] = m[0][1] * other.m[3][0]
            + m[1][1] * other.m[3][1]
            + m[2][1] * other.m[3][2]
            + m[3][1] * other.m[3][3];
    m[0][1] = m0;
    m[1][1] = m1;
    m[2][1] = m2;

    m0 = m[0][2] * other.m[0][0]
            + m[1][2] * other.m[0][1]
            + m[2][2] * other.m[0][2]
            + m[3][2] * other.m[0][3];
    m1 = m[0][2] * other.m[1][0]
            + m[1][2] * other.m[1][1]
            + m[2][2] * other.m[1][2]
            + m[3][2] * other.m[1][3];
    m2 = m[0][2] * other.m[2][0]
            + m[1][2] * other.m[2][1]
            + m[2][2] * other.m[2][2]
            + m[3][2] * other.m[2][3];
    m[3][2] = m[0][2] * other.m[3][0]
            + m[1][2] * other.m[3][1]
            + m[2][2] * other.m[3][2]
            + m[3][2] * other.m[3][3];
    m[0][2] = m0;
    m[1][2] = m1;
    m[2][2] = m2;

    m0 = m[0][3] * other.m[0][0]
            + m[1][3] * other.m[0][1]
            + m[2][3] * other.m[0][2]
            + m[3][3] * other.m[0][3];
    m1 = m[0][3] * other.m[1][0]
            + m[1][3] * other.m[1][1]
            + m[2][3] * other.m[1][2]
            + m[3][3] * other.m[1][3];
    m2 = m[0][3] * other.m[2][0]
            + m[1][3] * other.m[2][1]
            + m[2][3] * other.m[2][2]
            + m[3][3] * other.m[2][3];
    m[3][3] = m[0][3] * other.m[3][0]
            + m[1][3] * other.m[3][1]
            + m[2][3] * other.m[3][2]
            + m[3][3] * other.m[3][3];
    m[0][3] = m0;
    m[1][3] = m1;
    m[2][3] = m2;

    return *this;
}

inline QMatrix4x4 &QMatrix4x4::operator*=(qreal factor)
{
   m[0][0] *= factor;
   m[0][1] *= factor;
   m[0][2] *= factor;
   m[0][3] *= factor;
   m[1][0] *= factor;
   m[1][1] *= factor;
   m[1][2] *= factor;
   m[1][3] *= factor;
   m[2][0] *= factor;
   m[2][1] *= factor;
   m[2][2] *= factor;
   m[2][3] *= factor;
   m[3][0] *= factor;
   m[3][1] *= factor;
   m[3][2] *= factor;
   m[3][3] *= factor;
   flagBits = General;
   return *this;
}

inline bool QMatrix4x4::operator==(const QMatrix4x4 &other) const
{
   return m[0][0] == other.m[0][0] &&
          m[0][1] == other.m[0][1] &&
          m[0][2] == other.m[0][2] &&
          m[0][3] == other.m[0][3] &&
          m[1][0] == other.m[1][0] &&
          m[1][1] == other.m[1][1] &&
          m[1][2] == other.m[1][2] &&
          m[1][3] == other.m[1][3] &&
          m[2][0] == other.m[2][0] &&
          m[2][1] == other.m[2][1] &&
          m[2][2] == other.m[2][2] &&
          m[2][3] == other.m[2][3] &&
          m[3][0] == other.m[3][0] &&
          m[3][1] == other.m[3][1] &&
          m[3][2] == other.m[3][2] &&
          m[3][3] == other.m[3][3];
}

inline bool QMatrix4x4::operator!=(const QMatrix4x4 &other) const
{
   return m[0][0] != other.m[0][0] ||
          m[0][1] != other.m[0][1] ||
          m[0][2] != other.m[0][2] ||
          m[0][3] != other.m[0][3] ||
          m[1][0] != other.m[1][0] ||
          m[1][1] != other.m[1][1] ||
          m[1][2] != other.m[1][2] ||
          m[1][3] != other.m[1][3] ||
          m[2][0] != other.m[2][0] ||
          m[2][1] != other.m[2][1] ||
          m[2][2] != other.m[2][2] ||
          m[2][3] != other.m[2][3] ||
          m[3][0] != other.m[3][0] ||
          m[3][1] != other.m[3][1] ||
          m[3][2] != other.m[3][2] ||
          m[3][3] != other.m[3][3];
}

inline QMatrix4x4 operator+(const QMatrix4x4 &m1, const QMatrix4x4 &m2)
{
   QMatrix4x4 m(1);
   m.m[0][0] = m1.m[0][0] + m2.m[0][0];
   m.m[0][1] = m1.m[0][1] + m2.m[0][1];
   m.m[0][2] = m1.m[0][2] + m2.m[0][2];
   m.m[0][3] = m1.m[0][3] + m2.m[0][3];
   m.m[1][0] = m1.m[1][0] + m2.m[1][0];
   m.m[1][1] = m1.m[1][1] + m2.m[1][1];
   m.m[1][2] = m1.m[1][2] + m2.m[1][2];
   m.m[1][3] = m1.m[1][3] + m2.m[1][3];
   m.m[2][0] = m1.m[2][0] + m2.m[2][0];
   m.m[2][1] = m1.m[2][1] + m2.m[2][1];
   m.m[2][2] = m1.m[2][2] + m2.m[2][2];
   m.m[2][3] = m1.m[2][3] + m2.m[2][3];
   m.m[3][0] = m1.m[3][0] + m2.m[3][0];
   m.m[3][1] = m1.m[3][1] + m2.m[3][1];
   m.m[3][2] = m1.m[3][2] + m2.m[3][2];
   m.m[3][3] = m1.m[3][3] + m2.m[3][3];
   m.flagBits = QMatrix4x4::General;
   return m;
}

inline QMatrix4x4 operator-(const QMatrix4x4 &m1, const QMatrix4x4 &m2)
{
   QMatrix4x4 m(1);
   m.m[0][0] = m1.m[0][0] - m2.m[0][0];
   m.m[0][1] = m1.m[0][1] - m2.m[0][1];
   m.m[0][2] = m1.m[0][2] - m2.m[0][2];
   m.m[0][3] = m1.m[0][3] - m2.m[0][3];
   m.m[1][0] = m1.m[1][0] - m2.m[1][0];
   m.m[1][1] = m1.m[1][1] - m2.m[1][1];
   m.m[1][2] = m1.m[1][2] - m2.m[1][2];
   m.m[1][3] = m1.m[1][3] - m2.m[1][3];
   m.m[2][0] = m1.m[2][0] - m2.m[2][0];
   m.m[2][1] = m1.m[2][1] - m2.m[2][1];
   m.m[2][2] = m1.m[2][2] - m2.m[2][2];
   m.m[2][3] = m1.m[2][3] - m2.m[2][3];
   m.m[3][0] = m1.m[3][0] - m2.m[3][0];
   m.m[3][1] = m1.m[3][1] - m2.m[3][1];
   m.m[3][2] = m1.m[3][2] - m2.m[3][2];
   m.m[3][3] = m1.m[3][3] - m2.m[3][3];
    m.flagBits = QMatrix4x4::General;
   return m;
}

inline QMatrix4x4 operator*(const QMatrix4x4 &m1, const QMatrix4x4 &m2)
{
    int flagBits = m1.flagBits | m2.flagBits;

    if (flagBits < QMatrix4x4::Rotation2D) {
        QMatrix4x4 m = m1;
        m.m[3][0] += m.m[0][0] * m2.m[3][0];
        m.m[3][1] += m.m[1][1] * m2.m[3][1];
        m.m[3][2] += m.m[2][2] * m2.m[3][2];

        m.m[0][0] *= m2.m[0][0];
        m.m[1][1] *= m2.m[1][1];
        m.m[2][2] *= m2.m[2][2];
        m.flagBits = flagBits;
        return m;
   }

   QMatrix4x4 m(1);
   m.m[0][0] = m1.m[0][0] * m2.m[0][0] +
               m1.m[1][0] * m2.m[0][1] +
               m1.m[2][0] * m2.m[0][2] +
               m1.m[3][0] * m2.m[0][3];
   m.m[0][1] = m1.m[0][1] * m2.m[0][0] +
               m1.m[1][1] * m2.m[0][1] +
               m1.m[2][1] * m2.m[0][2] +
               m1.m[3][1] * m2.m[0][3];
   m.m[0][2] = m1.m[0][2] * m2.m[0][0] +
               m1.m[1][2] * m2.m[0][1] +
               m1.m[2][2] * m2.m[0][2] +
               m1.m[3][2] * m2.m[0][3];
   m.m[0][3] = m1.m[0][3] * m2.m[0][0] +
               m1.m[1][3] * m2.m[0][1] +
               m1.m[2][3] * m2.m[0][2] +
               m1.m[3][3] * m2.m[0][3];
   m.m[1][0] = m1.m[0][0] * m2.m[1][0] +
               m1.m[1][0] * m2.m[1][1] +
               m1.m[2][0] * m2.m[1][2] +
               m1.m[3][0] * m2.m[1][3];
   m.m[1][1] = m1.m[0][1] * m2.m[1][0] +
               m1.m[1][1] * m2.m[1][1] +
               m1.m[2][1] * m2.m[1][2] +
               m1.m[3][1] * m2.m[1][3];
   m.m[1][2] = m1.m[0][2] * m2.m[1][0] +
               m1.m[1][2] * m2.m[1][1] +
               m1.m[2][2] * m2.m[1][2] +
               m1.m[3][2] * m2.m[1][3];
   m.m[1][3] = m1.m[0][3] * m2.m[1][0] +
               m1.m[1][3] * m2.m[1][1] +
               m1.m[2][3] * m2.m[1][2] +
               m1.m[3][3] * m2.m[1][3];
   m.m[2][0] = m1.m[0][0] * m2.m[2][0] +
               m1.m[1][0] * m2.m[2][1] +
               m1.m[2][0] * m2.m[2][2] +
               m1.m[3][0] * m2.m[2][3];
   m.m[2][1] = m1.m[0][1] * m2.m[2][0] +
               m1.m[1][1] * m2.m[2][1] +
               m1.m[2][1] * m2.m[2][2] +
               m1.m[3][1] * m2.m[2][3];
   m.m[2][2] = m1.m[0][2] * m2.m[2][0] +
               m1.m[1][2] * m2.m[2][1] +
               m1.m[2][2] * m2.m[2][2] +
               m1.m[3][2] * m2.m[2][3];
   m.m[2][3] = m1.m[0][3] * m2.m[2][0] +
               m1.m[1][3] * m2.m[2][1] +
               m1.m[2][3] * m2.m[2][2] +
               m1.m[3][3] * m2.m[2][3];
   m.m[3][0] = m1.m[0][0] * m2.m[3][0] +
               m1.m[1][0] * m2.m[3][1] +
               m1.m[2][0] * m2.m[3][2] +
               m1.m[3][0] * m2.m[3][3];
   m.m[3][1] = m1.m[0][1] * m2.m[3][0] +
               m1.m[1][1] * m2.m[3][1] +
               m1.m[2][1] * m2.m[3][2] +
               m1.m[3][1] * m2.m[3][3];
   m.m[3][2] = m1.m[0][2] * m2.m[3][0] +
               m1.m[1][2] * m2.m[3][1] +
               m1.m[2][2] * m2.m[3][2] +
               m1.m[3][2] * m2.m[3][3];
   m.m[3][3] = m1.m[0][3] * m2.m[3][0] +
               m1.m[1][3] * m2.m[3][1] +
               m1.m[2][3] * m2.m[3][2] +
               m1.m[3][3] * m2.m[3][3];
   m.flagBits = flagBits;

   return m;
}

#ifndef QT_NO_VECTOR3D

inline QVector3D operator*(const QVector3D &vector, const QMatrix4x4 &matrix)
{
   qreal x, y, z, w;
   x = vector.x() * matrix.m[0][0] +
       vector.y() * matrix.m[0][1] +
       vector.z() * matrix.m[0][2] +
       matrix.m[0][3];
   y = vector.x() * matrix.m[1][0] +
       vector.y() * matrix.m[1][1] +
       vector.z() * matrix.m[1][2] +
       matrix.m[1][3];
   z = vector.x() * matrix.m[2][0] +
       vector.y() * matrix.m[2][1] +
       vector.z() * matrix.m[2][2] +
       matrix.m[2][3];
   w = vector.x() * matrix.m[3][0] +
       vector.y() * matrix.m[3][1] +
       vector.z() * matrix.m[3][2] +
       matrix.m[3][3];
   if (w == 1.0f) {
      return QVector3D(x, y, z);
   } else {
      return QVector3D(x / w, y / w, z / w);
   }
}

inline QVector3D operator*(const QMatrix4x4 &matrix, const QVector3D &vector)
{
   qreal x, y, z, w;
   if (matrix.flagBits == QMatrix4x4::Identity) {
      return vector;

    } else if (matrix.flagBits < QMatrix4x4::Rotation2D) {
        // Translation | Scale
      return QVector3D(vector.x() * matrix.m[0][0] + matrix.m[3][0],
                       vector.y() * matrix.m[1][1] + matrix.m[3][1],
                       vector.z() * matrix.m[2][2] + matrix.m[3][2]);
    } else if (matrix.flagBits < QMatrix4x4::Rotation) {
       return QVector3D(vector.x() * matrix.m[0][0] + vector.y() * matrix.m[1][0] + matrix.m[3][0],
                         vector.x() * matrix.m[0][1] + vector.y() * matrix.m[1][1] + matrix.m[3][1],
                         vector.z() * matrix.m[2][2] + matrix.m[3][2]);
   } else {
      x = vector.x() * matrix.m[0][0] +
          vector.y() * matrix.m[1][0] +
          vector.z() * matrix.m[2][0] +
          matrix.m[3][0];
      y = vector.x() * matrix.m[0][1] +
          vector.y() * matrix.m[1][1] +
          vector.z() * matrix.m[2][1] +
          matrix.m[3][1];
      z = vector.x() * matrix.m[0][2] +
          vector.y() * matrix.m[1][2] +
          vector.z() * matrix.m[2][2] +
          matrix.m[3][2];
      w = vector.x() * matrix.m[0][3] +
          vector.y() * matrix.m[1][3] +
          vector.z() * matrix.m[2][3] +
          matrix.m[3][3];

      if (w == 1.0f) {
         return QVector3D(x, y, z);
      } else {
         return QVector3D(x / w, y / w, z / w);
      }
   }
}

#endif

#ifndef QT_NO_VECTOR4D

inline QVector4D operator*(const QVector4D &vector, const QMatrix4x4 &matrix)
{
   qreal x, y, z, w;
   x = vector.x() * matrix.m[0][0] +
       vector.y() * matrix.m[0][1] +
       vector.z() * matrix.m[0][2] +
       vector.w() * matrix.m[0][3];
   y = vector.x() * matrix.m[1][0] +
       vector.y() * matrix.m[1][1] +
       vector.z() * matrix.m[1][2] +
       vector.w() * matrix.m[1][3];
   z = vector.x() * matrix.m[2][0] +
       vector.y() * matrix.m[2][1] +
       vector.z() * matrix.m[2][2] +
       vector.w() * matrix.m[2][3];
   w = vector.x() * matrix.m[3][0] +
       vector.y() * matrix.m[3][1] +
       vector.z() * matrix.m[3][2] +
       vector.w() * matrix.m[3][3];
   return QVector4D(x, y, z, w);
}

inline QVector4D operator*(const QMatrix4x4 &matrix, const QVector4D &vector)
{
   qreal x, y, z, w;
   x = vector.x() * matrix.m[0][0] +
       vector.y() * matrix.m[1][0] +
       vector.z() * matrix.m[2][0] +
       vector.w() * matrix.m[3][0];
   y = vector.x() * matrix.m[0][1] +
       vector.y() * matrix.m[1][1] +
       vector.z() * matrix.m[2][1] +
       vector.w() * matrix.m[3][1];
   z = vector.x() * matrix.m[0][2] +
       vector.y() * matrix.m[1][2] +
       vector.z() * matrix.m[2][2] +
       vector.w() * matrix.m[3][2];
   w = vector.x() * matrix.m[0][3] +
       vector.y() * matrix.m[1][3] +
       vector.z() * matrix.m[2][3] +
       vector.w() * matrix.m[3][3];
   return QVector4D(x, y, z, w);
}

#endif

inline QPoint operator*(const QPoint &point, const QMatrix4x4 &matrix)
{
   qreal xin, yin;
   qreal x, y, w;

   xin = point.x();
   yin = point.y();

   x = xin * matrix.m[0][0] +
       yin * matrix.m[0][1] +
       matrix.m[0][3];
   y = xin * matrix.m[1][0] +
       yin * matrix.m[1][1] +
       matrix.m[1][3];
   w = xin * matrix.m[3][0] +
       yin * matrix.m[3][1] +
       matrix.m[3][3];
   if (w == 1.0f) {
      return QPoint(qRound(x), qRound(y));
   } else {
      return QPoint(qRound(x / w), qRound(y / w));
   }
}

inline QPointF operator*(const QPointF &point, const QMatrix4x4 &matrix)
{
   qreal xin, yin;
   qreal x, y, w;

   xin = point.x();
   yin = point.y();
   x = xin * matrix.m[0][0] +
       yin * matrix.m[0][1] +
       matrix.m[0][3];
   y = xin * matrix.m[1][0] +
       yin * matrix.m[1][1] +
       matrix.m[1][3];
   w = xin * matrix.m[3][0] +
       yin * matrix.m[3][1] +
       matrix.m[3][3];

   if (w == 1.0f) {
      return QPointF(qreal(x), qreal(y));
   } else {
      return QPointF(qreal(x / w), qreal(y / w));
   }
}

inline QPoint operator*(const QMatrix4x4 &matrix, const QPoint &point)
{
   qreal xin, yin;
   qreal x, y, w;

   xin = point.x();
   yin = point.y();

   if (matrix.flagBits == QMatrix4x4::Identity) {
      return point;

    } else if (matrix.flagBits < QMatrix4x4::Rotation2D) {
        // Translation | Scale
      return QPoint(qRound(xin * matrix.m[0][0] + matrix.m[3][0]),
                    qRound(yin * matrix.m[1][1] + matrix.m[3][1]));

    } else if (matrix.flagBits < QMatrix4x4::Perspective) {
        return QPoint(qRound(xin * matrix.m[0][0] + yin * matrix.m[1][0] + matrix.m[3][0]),
                      qRound(xin * matrix.m[0][1] + yin * matrix.m[1][1] + matrix.m[3][1]));

    } else {
        x = xin * matrix.m[0][0] +
            yin * matrix.m[1][0] +
            matrix.m[3][0];
        y = xin * matrix.m[0][1] +
            yin * matrix.m[1][1] +
            matrix.m[3][1];
        w = xin * matrix.m[0][3] +
            yin * matrix.m[1][3] +
            matrix.m[3][3];
        if (w == 1.0f)
            return QPoint(qRound(x), qRound(y));
        else
            return QPoint(qRound(x / w), qRound(y / w));
    }
}


inline QPointF operator*(const QMatrix4x4 &matrix, const QPointF &point)
{
   qreal xin, yin;
   qreal x, y, w;

   xin = point.x();
   yin = point.y();
   if (matrix.flagBits == QMatrix4x4::Identity) {
      return point;

    } else if (matrix.flagBits < QMatrix4x4::Rotation2D) {
        // Translation | Scale
      return QPointF(xin * matrix.m[0][0] + matrix.m[3][0],
                     yin * matrix.m[1][1] + matrix.m[3][1]);
    } else if (matrix.flagBits < QMatrix4x4::Perspective) {
        return QPointF(xin * matrix.m[0][0] + yin * matrix.m[1][0] + matrix.m[3][0],
                       xin * matrix.m[0][1] + yin * matrix.m[1][1] + matrix.m[3][1]);
   } else {
      x = xin * matrix.m[0][0] +
          yin * matrix.m[1][0] +
          matrix.m[3][0];
      y = xin * matrix.m[0][1] +
          yin * matrix.m[1][1] +
          matrix.m[3][1];
      w = xin * matrix.m[0][3] +
          yin * matrix.m[1][3] +
          matrix.m[3][3];

      if (w == 1.0f) {
         return QPointF(qreal(x), qreal(y));
      } else {
         return QPointF(qreal(x / w), qreal(y / w));
      }
   }
}

inline QMatrix4x4 operator-(const QMatrix4x4 &matrix)
{
   QMatrix4x4 m(1);
   m.m[0][0] = -matrix.m[0][0];
   m.m[0][1] = -matrix.m[0][1];
   m.m[0][2] = -matrix.m[0][2];
   m.m[0][3] = -matrix.m[0][3];
   m.m[1][0] = -matrix.m[1][0];
   m.m[1][1] = -matrix.m[1][1];
   m.m[1][2] = -matrix.m[1][2];
   m.m[1][3] = -matrix.m[1][3];
   m.m[2][0] = -matrix.m[2][0];
   m.m[2][1] = -matrix.m[2][1];
   m.m[2][2] = -matrix.m[2][2];
   m.m[2][3] = -matrix.m[2][3];
   m.m[3][0] = -matrix.m[3][0];
   m.m[3][1] = -matrix.m[3][1];
   m.m[3][2] = -matrix.m[3][2];
   m.m[3][3] = -matrix.m[3][3];
    m.flagBits = QMatrix4x4::General;
   return m;
}

inline QMatrix4x4 operator*(qreal factor, const QMatrix4x4 &matrix)
{
   QMatrix4x4 m(1);
   m.m[0][0] = matrix.m[0][0] * factor;
   m.m[0][1] = matrix.m[0][1] * factor;
   m.m[0][2] = matrix.m[0][2] * factor;
   m.m[0][3] = matrix.m[0][3] * factor;
   m.m[1][0] = matrix.m[1][0] * factor;
   m.m[1][1] = matrix.m[1][1] * factor;
   m.m[1][2] = matrix.m[1][2] * factor;
   m.m[1][3] = matrix.m[1][3] * factor;
   m.m[2][0] = matrix.m[2][0] * factor;
   m.m[2][1] = matrix.m[2][1] * factor;
   m.m[2][2] = matrix.m[2][2] * factor;
   m.m[2][3] = matrix.m[2][3] * factor;
   m.m[3][0] = matrix.m[3][0] * factor;
   m.m[3][1] = matrix.m[3][1] * factor;
   m.m[3][2] = matrix.m[3][2] * factor;
   m.m[3][3] = matrix.m[3][3] * factor;
    m.flagBits = QMatrix4x4::General;
   return m;
}

inline QMatrix4x4 operator*(const QMatrix4x4 &matrix, qreal factor)
{
   QMatrix4x4 m(1);
   m.m[0][0] = matrix.m[0][0] * factor;
   m.m[0][1] = matrix.m[0][1] * factor;
   m.m[0][2] = matrix.m[0][2] * factor;
   m.m[0][3] = matrix.m[0][3] * factor;
   m.m[1][0] = matrix.m[1][0] * factor;
   m.m[1][1] = matrix.m[1][1] * factor;
   m.m[1][2] = matrix.m[1][2] * factor;
   m.m[1][3] = matrix.m[1][3] * factor;
   m.m[2][0] = matrix.m[2][0] * factor;
   m.m[2][1] = matrix.m[2][1] * factor;
   m.m[2][2] = matrix.m[2][2] * factor;
   m.m[2][3] = matrix.m[2][3] * factor;
   m.m[3][0] = matrix.m[3][0] * factor;
   m.m[3][1] = matrix.m[3][1] * factor;
   m.m[3][2] = matrix.m[3][2] * factor;
   m.m[3][3] = matrix.m[3][3] * factor;
    m.flagBits = QMatrix4x4::General;
   return m;
}

inline bool qFuzzyCompare(const QMatrix4x4 &m1, const QMatrix4x4 &m2)
{
   return qFuzzyCompare(m1.m[0][0], m2.m[0][0]) &&
          qFuzzyCompare(m1.m[0][1], m2.m[0][1]) &&
          qFuzzyCompare(m1.m[0][2], m2.m[0][2]) &&
          qFuzzyCompare(m1.m[0][3], m2.m[0][3]) &&
          qFuzzyCompare(m1.m[1][0], m2.m[1][0]) &&
          qFuzzyCompare(m1.m[1][1], m2.m[1][1]) &&
          qFuzzyCompare(m1.m[1][2], m2.m[1][2]) &&
          qFuzzyCompare(m1.m[1][3], m2.m[1][3]) &&
          qFuzzyCompare(m1.m[2][0], m2.m[2][0]) &&
          qFuzzyCompare(m1.m[2][1], m2.m[2][1]) &&
          qFuzzyCompare(m1.m[2][2], m2.m[2][2]) &&
          qFuzzyCompare(m1.m[2][3], m2.m[2][3]) &&
          qFuzzyCompare(m1.m[3][0], m2.m[3][0]) &&
          qFuzzyCompare(m1.m[3][1], m2.m[3][1]) &&
          qFuzzyCompare(m1.m[3][2], m2.m[3][2]) &&
          qFuzzyCompare(m1.m[3][3], m2.m[3][3]);
}

inline QPoint QMatrix4x4::map(const QPoint &point) const
{
   return *this * point;
}

inline QPointF QMatrix4x4::map(const QPointF &point) const
{
   return *this * point;
}

#ifndef QT_NO_VECTOR3D

inline QVector3D QMatrix4x4::map(const QVector3D &point) const
{
   return *this * point;
}

inline QVector3D QMatrix4x4::mapVector(const QVector3D &vector) const
{
    if (flagBits < Scale) {
      return vector;

    } else if (flagBits < Rotation2D) {
      return QVector3D(vector.x() * m[0][0],
                       vector.y() * m[1][1],
                       vector.z() * m[2][2]);
   } else {
      return QVector3D(vector.x() * m[0][0] +
                       vector.y() * m[1][0] +
                       vector.z() * m[2][0],
                       vector.x() * m[0][1] +
                       vector.y() * m[1][1] +
                       vector.z() * m[2][1],
                       vector.x() * m[0][2] +
                       vector.y() * m[1][2] +
                       vector.z() * m[2][2]);
   }
}

#endif

#ifndef QT_NO_VECTOR4D

inline QVector4D QMatrix4x4::map(const QVector4D &point) const
{
   return *this * point;
}

#endif

inline qreal *QMatrix4x4::data()
{
   // We have to assume that the caller will modify the matrix elements,
   // so we flip it over to "General" mode.
   flagBits = General;
   return *m;
}

inline void QMatrix4x4::viewport(const QRectF &rect)
{
    viewport(rect.x(), rect.y(), rect.width(), rect.height());
}
Q_GUI_EXPORT QDebug operator<<(QDebug dbg, const QMatrix4x4 &m);


Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QMatrix4x4 &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QMatrix4x4 &);


#endif

#endif
