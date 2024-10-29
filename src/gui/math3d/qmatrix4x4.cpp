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

#include <qmatrix4x4.h>

#include <qmath.h>
#include <qmatrix.h>
#include <qtransform.h>
#include <qvariant.h>

#ifndef QT_NO_MATRIX4X4

static constexpr const qreal inv_dist_to_plane = 1.0 / 1024.0;

QMatrix4x4::QMatrix4x4(const qreal *values)
{
   for (int row = 0; row < 4; ++row)
      for (int col = 0; col < 4; ++col) {
         m[col][row] = values[row * 4 + col];
      }
   flagBits = General;
}

QMatrix4x4::QMatrix4x4(const qreal *values, int cols, int rows)
{
   for (int col = 0; col < 4; ++col) {
      for (int row = 0; row < 4; ++row) {
         if (col < cols && row < rows) {
            m[col][row] = values[col * rows + row];
         } else if (col == row) {
            m[col][row] = 1.0f;
         } else {
            m[col][row] = 0.0f;
         }
      }
   }
   flagBits = General;
}

QMatrix4x4::QMatrix4x4(const QMatrix &matrix)
{
   m[0][0] = matrix.m11();
   m[0][1] = matrix.m12();
   m[0][2] = 0.0f;
   m[0][3] = 0.0f;
   m[1][0] = matrix.m21();
   m[1][1] = matrix.m22();
   m[1][2] = 0.0f;
   m[1][3] = 0.0f;
   m[2][0] = 0.0f;
   m[2][1] = 0.0f;
   m[2][2] = 1.0f;
   m[2][3] = 0.0f;
   m[3][0] = matrix.dx();
   m[3][1] = matrix.dy();
   m[3][2] = 0.0f;
   m[3][3] = 1.0f;
   flagBits = General;
}

QMatrix4x4::QMatrix4x4(const QTransform &transform)
{
   m[0][0] = transform.m11();
   m[0][1] = transform.m12();
   m[0][2] = 0.0f;
   m[0][3] = transform.m13();
   m[1][0] = transform.m21();
   m[1][1] = transform.m22();
   m[1][2] = 0.0f;
   m[1][3] = transform.m23();
   m[2][0] = 0.0f;
   m[2][1] = 0.0f;
   m[2][2] = 1.0f;
   m[2][3] = 0.0f;
   m[3][0] = transform.dx();
   m[3][1] = transform.dy();
   m[3][2] = 0.0f;
   m[3][3] = transform.m33();
   flagBits = General;
}

// The 4x4 matrix inverse algorithm is based on that described at:
// http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q24
// Some optimization has been done to avoid making copies of 3x3
// sub-matrices and to unroll the loops.

// Calculate the determinant of a 3x3 sub-matrix.
//     | A B C |
// M = | D E F |   det(M) = A * (EI - HF) - B * (DI - GF) + C * (DH - GE)
//     | G H I |
static inline qreal matrixDet3(const qreal m[4][4], int col0, int col1, int col2, int row0, int row1, int row2)
{
   return m[col0][row0] *
          (m[col1][row1] * m[col2][row2] -
           m[col1][row2] * m[col2][row1]) -
          m[col1][row0] *
          (m[col0][row1] * m[col2][row2] -
           m[col0][row2] * m[col2][row1]) +
          m[col2][row0] *
          (m[col0][row1] * m[col1][row2] -
           m[col0][row2] * m[col1][row1]);
}

// Calculate the determinant of a 4x4 matrix.
static inline qreal matrixDet4(const qreal m[4][4])
{
   qreal det;
   det  = m[0][0] * matrixDet3(m, 1, 2, 3, 1, 2, 3);
   det -= m[1][0] * matrixDet3(m, 0, 2, 3, 1, 2, 3);
   det += m[2][0] * matrixDet3(m, 0, 1, 3, 1, 2, 3);
   det -= m[3][0] * matrixDet3(m, 0, 1, 2, 1, 2, 3);
   return det;
}

qreal QMatrix4x4::determinant() const
{
   return qreal(matrixDet4(m));
}

QMatrix4x4 QMatrix4x4::inverted(bool *invertible) const
{
   // Handle some of the easy cases first.
   if (flagBits == Identity) {
      if (invertible) {
         *invertible = true;
      }
      return QMatrix4x4();
   } else if (flagBits == Translation) {
      QMatrix4x4 inv;
      inv.m[3][0] = -m[3][0];
      inv.m[3][1] = -m[3][1];
      inv.m[3][2] = -m[3][2];
      inv.flagBits = Translation;
      if (invertible) {
         *invertible = true;
      }
      return inv;
   } else if (flagBits == Rotation || flagBits == (Rotation | Translation)) {
      if (invertible) {
         *invertible = true;
      }
      return orthonormalInverse();
   }

   QMatrix4x4 inv(1); // The "1" says to not load the identity.

   qreal det = matrixDet4(m);
   if (det == 0.0f) {
      if (invertible) {
         *invertible = false;
      }
      return QMatrix4x4();
   }
   det = 1.0f / det;

   inv.m[0][0] =  matrixDet3(m, 1, 2, 3, 1, 2, 3) * det;
   inv.m[0][1] = -matrixDet3(m, 0, 2, 3, 1, 2, 3) * det;
   inv.m[0][2] =  matrixDet3(m, 0, 1, 3, 1, 2, 3) * det;
   inv.m[0][3] = -matrixDet3(m, 0, 1, 2, 1, 2, 3) * det;
   inv.m[1][0] = -matrixDet3(m, 1, 2, 3, 0, 2, 3) * det;
   inv.m[1][1] =  matrixDet3(m, 0, 2, 3, 0, 2, 3) * det;
   inv.m[1][2] = -matrixDet3(m, 0, 1, 3, 0, 2, 3) * det;
   inv.m[1][3] =  matrixDet3(m, 0, 1, 2, 0, 2, 3) * det;
   inv.m[2][0] =  matrixDet3(m, 1, 2, 3, 0, 1, 3) * det;
   inv.m[2][1] = -matrixDet3(m, 0, 2, 3, 0, 1, 3) * det;
   inv.m[2][2] =  matrixDet3(m, 0, 1, 3, 0, 1, 3) * det;
   inv.m[2][3] = -matrixDet3(m, 0, 1, 2, 0, 1, 3) * det;
   inv.m[3][0] = -matrixDet3(m, 1, 2, 3, 0, 1, 2) * det;
   inv.m[3][1] =  matrixDet3(m, 0, 2, 3, 0, 1, 2) * det;
   inv.m[3][2] = -matrixDet3(m, 0, 1, 3, 0, 1, 2) * det;
   inv.m[3][3] =  matrixDet3(m, 0, 1, 2, 0, 1, 2) * det;

   if (invertible) {
      *invertible = true;
   }
   return inv;
}

QMatrix3x3 QMatrix4x4::normalMatrix() const
{
   QMatrix3x3 inv;

   // Handle the simple cases first.
   if (flagBits == Identity || flagBits == Translation) {
      return inv;
   } else if (flagBits == Scale || flagBits == (Translation | Scale)) {
      if (m[0][0] == 0.0f || m[1][1] == 0.0f || m[2][2] == 0.0f) {
         return inv;
      }
      inv.data()[0] = 1.0f / m[0][0];
      inv.data()[4] = 1.0f / m[1][1];
      inv.data()[8] = 1.0f / m[2][2];
      return inv;
   }

   qreal det = matrixDet3(m, 0, 1, 2, 0, 1, 2);
   if (det == 0.0f) {
      return inv;
   }
   det = 1.0f / det;

   qreal *invm = inv.data();

   // Invert and transpose in a single step.
   invm[0 + 0 * 3] =  (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * det;
   invm[1 + 0 * 3] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * det;
   invm[2 + 0 * 3] =  (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * det;
   invm[0 + 1 * 3] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]) * det;
   invm[1 + 1 * 3] =  (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * det;
   invm[2 + 1 * 3] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * det;
   invm[0 + 2 * 3] =  (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * det;
   invm[1 + 2 * 3] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * det;
   invm[2 + 2 * 3] =  (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * det;

   return inv;
}

QMatrix4x4 QMatrix4x4::transposed() const
{
   QMatrix4x4 result(1); // The "1" says to not load the identity.
   for (int row = 0; row < 4; ++row) {
      for (int col = 0; col < 4; ++col) {
         result.m[col][row] = m[row][col];
      }
   }
   return result;
}

QMatrix4x4 &QMatrix4x4::operator/=(qreal divisor)
{
   m[0][0] /= divisor;
   m[0][1] /= divisor;
   m[0][2] /= divisor;
   m[0][3] /= divisor;
   m[1][0] /= divisor;
   m[1][1] /= divisor;
   m[1][2] /= divisor;
   m[1][3] /= divisor;
   m[2][0] /= divisor;
   m[2][1] /= divisor;
   m[2][2] /= divisor;
   m[2][3] /= divisor;
   m[3][0] /= divisor;
   m[3][1] /= divisor;
   m[3][2] /= divisor;
   m[3][3] /= divisor;
   flagBits = General;
   return *this;
}

QMatrix4x4 operator/(const QMatrix4x4 &matrix, qreal divisor)
{
   QMatrix4x4 m(1); // The "1" says to not load the identity.
   m.m[0][0] = matrix.m[0][0] / divisor;
   m.m[0][1] = matrix.m[0][1] / divisor;
   m.m[0][2] = matrix.m[0][2] / divisor;
   m.m[0][3] = matrix.m[0][3] / divisor;
   m.m[1][0] = matrix.m[1][0] / divisor;
   m.m[1][1] = matrix.m[1][1] / divisor;
   m.m[1][2] = matrix.m[1][2] / divisor;
   m.m[1][3] = matrix.m[1][3] / divisor;
   m.m[2][0] = matrix.m[2][0] / divisor;
   m.m[2][1] = matrix.m[2][1] / divisor;
   m.m[2][2] = matrix.m[2][2] / divisor;
   m.m[2][3] = matrix.m[2][3] / divisor;
   m.m[3][0] = matrix.m[3][0] / divisor;
   m.m[3][1] = matrix.m[3][1] / divisor;
   m.m[3][2] = matrix.m[3][2] / divisor;
   m.m[3][3] = matrix.m[3][3] / divisor;
   return m;
}

#ifndef QT_NO_VECTOR3D

void QMatrix4x4::scale(const QVector3D &vector)
{
   qreal vx = vector.x();
   qreal vy = vector.y();
   qreal vz = vector.z();
   if (flagBits == Identity) {
      m[0][0] = vx;
      m[1][1] = vy;
      m[2][2] = vz;
      flagBits = Scale;
   } else if (flagBits == Scale || flagBits == (Scale | Translation)) {
      m[0][0] *= vx;
      m[1][1] *= vy;
      m[2][2] *= vz;
   } else if (flagBits == Translation) {
      m[0][0] = vx;
      m[1][1] = vy;
      m[2][2] = vz;
      flagBits |= Scale;
   } else {
      m[0][0] *= vx;
      m[0][1] *= vx;
      m[0][2] *= vx;
      m[0][3] *= vx;
      m[1][0] *= vy;
      m[1][1] *= vy;
      m[1][2] *= vy;
      m[1][3] *= vy;
      m[2][0] *= vz;
      m[2][1] *= vz;
      m[2][2] *= vz;
      m[2][3] *= vz;
      flagBits = General;
   }
}
#endif

void QMatrix4x4::scale(qreal x, qreal y)
{
   if (flagBits == Identity) {
      m[0][0] = x;
      m[1][1] = y;
      flagBits = Scale;
   } else if (flagBits == Scale || flagBits == (Scale | Translation)) {
      m[0][0] *= x;
      m[1][1] *= y;
   } else if (flagBits == Translation) {
      m[0][0] = x;
      m[1][1] = y;
      flagBits |= Scale;
   } else {
      m[0][0] *= x;
      m[0][1] *= x;
      m[0][2] *= x;
      m[0][3] *= x;
      m[1][0] *= y;
      m[1][1] *= y;
      m[1][2] *= y;
      m[1][3] *= y;
      flagBits = General;
   }
}

void QMatrix4x4::scale(qreal x, qreal y, qreal z)
{
   if (flagBits == Identity) {
      m[0][0] = x;
      m[1][1] = y;
      m[2][2] = z;
      flagBits = Scale;
   } else if (flagBits == Scale || flagBits == (Scale | Translation)) {
      m[0][0] *= x;
      m[1][1] *= y;
      m[2][2] *= z;
   } else if (flagBits == Translation) {
      m[0][0] = x;
      m[1][1] = y;
      m[2][2] = z;
      flagBits |= Scale;
   } else {
      m[0][0] *= x;
      m[0][1] *= x;
      m[0][2] *= x;
      m[0][3] *= x;
      m[1][0] *= y;
      m[1][1] *= y;
      m[1][2] *= y;
      m[1][3] *= y;
      m[2][0] *= z;
      m[2][1] *= z;
      m[2][2] *= z;
      m[2][3] *= z;
      flagBits = General;
   }
}

void QMatrix4x4::scale(qreal factor)
{
   if (flagBits == Identity) {
      m[0][0] = factor;
      m[1][1] = factor;
      m[2][2] = factor;
      flagBits = Scale;
   } else if (flagBits == Scale || flagBits == (Scale | Translation)) {
      m[0][0] *= factor;
      m[1][1] *= factor;
      m[2][2] *= factor;
   } else if (flagBits == Translation) {
      m[0][0] = factor;
      m[1][1] = factor;
      m[2][2] = factor;
      flagBits |= Scale;
   } else {
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
      flagBits = General;
   }
}

#ifndef QT_NO_VECTOR3D

void QMatrix4x4::translate(const QVector3D &vector)
{
   qreal vx = vector.x();
   qreal vy = vector.y();
   qreal vz = vector.z();
   if (flagBits == Identity) {
      m[3][0] = vx;
      m[3][1] = vy;
      m[3][2] = vz;
      flagBits = Translation;
   } else if (flagBits == Translation) {
      m[3][0] += vx;
      m[3][1] += vy;
      m[3][2] += vz;
   } else if (flagBits == Scale) {
      m[3][0] = m[0][0] * vx;
      m[3][1] = m[1][1] * vy;
      m[3][2] = m[2][2] * vz;
      flagBits |= Translation;
   } else if (flagBits == (Scale | Translation)) {
      m[3][0] += m[0][0] * vx;
      m[3][1] += m[1][1] * vy;
      m[3][2] += m[2][2] * vz;
   } else {
      m[3][0] += m[0][0] * vx + m[1][0] * vy + m[2][0] * vz;
      m[3][1] += m[0][1] * vx + m[1][1] * vy + m[2][1] * vz;
      m[3][2] += m[0][2] * vx + m[1][2] * vy + m[2][2] * vz;
      m[3][3] += m[0][3] * vx + m[1][3] * vy + m[2][3] * vz;
      if (flagBits == Rotation) {
         flagBits |= Translation;
      } else if (flagBits != (Rotation | Translation)) {
         flagBits = General;
      }
   }
}

#endif

void QMatrix4x4::translate(qreal x, qreal y)
{
   if (flagBits == Identity) {
      m[3][0] = x;
      m[3][1] = y;
      flagBits = Translation;
   } else if (flagBits == Translation) {
      m[3][0] += x;
      m[3][1] += y;
   } else if (flagBits == Scale) {
      m[3][0] = m[0][0] * x;
      m[3][1] = m[1][1] * y;
      m[3][2] = 0.;
      flagBits |= Translation;
   } else if (flagBits == (Scale | Translation)) {
      m[3][0] += m[0][0] * x;
      m[3][1] += m[1][1] * y;
   } else {
      m[3][0] += m[0][0] * x + m[1][0] * y;
      m[3][1] += m[0][1] * x + m[1][1] * y;
      m[3][2] += m[0][2] * x + m[1][2] * y;
      m[3][3] += m[0][3] * x + m[1][3] * y;
      if (flagBits == Rotation) {
         flagBits |= Translation;
      } else if (flagBits != (Rotation | Translation)) {
         flagBits = General;
      }
   }
}

void QMatrix4x4::translate(qreal x, qreal y, qreal z)
{
   if (flagBits == Identity) {
      m[3][0] = x;
      m[3][1] = y;
      m[3][2] = z;
      flagBits = Translation;
   } else if (flagBits == Translation) {
      m[3][0] += x;
      m[3][1] += y;
      m[3][2] += z;
   } else if (flagBits == Scale) {
      m[3][0] = m[0][0] * x;
      m[3][1] = m[1][1] * y;
      m[3][2] = m[2][2] * z;
      flagBits |= Translation;
   } else if (flagBits == (Scale | Translation)) {
      m[3][0] += m[0][0] * x;
      m[3][1] += m[1][1] * y;
      m[3][2] += m[2][2] * z;
   } else {
      m[3][0] += m[0][0] * x + m[1][0] * y + m[2][0] * z;
      m[3][1] += m[0][1] * x + m[1][1] * y + m[2][1] * z;
      m[3][2] += m[0][2] * x + m[1][2] * y + m[2][2] * z;
      m[3][3] += m[0][3] * x + m[1][3] * y + m[2][3] * z;
      if (flagBits == Rotation) {
         flagBits |= Translation;
      } else if (flagBits != (Rotation | Translation)) {
         flagBits = General;
      }
   }
}

#ifndef QT_NO_VECTOR3D

void QMatrix4x4::rotate(qreal angle, const QVector3D &vector)
{
   rotate(angle, vector.x(), vector.y(), vector.z());
}

#endif

void QMatrix4x4::rotate(qreal angle, qreal x, qreal y, qreal z)
{
   if (angle == 0.0f) {
      return;
   }
   QMatrix4x4 m(1); // The "1" says to not load the identity.
   qreal c, s, ic;
   if (angle == 90.0f || angle == -270.0f) {
      s = 1.0f;
      c = 0.0f;
   } else if (angle == -90.0f || angle == 270.0f) {
      s = -1.0f;
      c = 0.0f;
   } else if (angle == 180.0f || angle == -180.0f) {
      s = 0.0f;
      c = -1.0f;
   } else {
      qreal a = angle * M_PI / 180.0f;
      c = qCos(a);
      s = qSin(a);
   }
   bool quick = false;
   if (x == 0.0f) {
      if (y == 0.0f) {
         if (z != 0.0f) {
            // Rotate around the Z axis.
            m.setToIdentity();
            m.m[0][0] = c;
            m.m[1][1] = c;
            if (z < 0.0f) {
               m.m[1][0] = s;
               m.m[0][1] = -s;
            } else {
               m.m[1][0] = -s;
               m.m[0][1] = s;
            }
            m.flagBits = General;
            quick = true;
         }
      } else if (z == 0.0f) {
         // Rotate around the Y axis.
         m.setToIdentity();
         m.m[0][0] = c;
         m.m[2][2] = c;
         if (y < 0.0f) {
            m.m[2][0] = -s;
            m.m[0][2] = s;
         } else {
            m.m[2][0] = s;
            m.m[0][2] = -s;
         }
         m.flagBits = General;
         quick = true;
      }
   } else if (y == 0.0f && z == 0.0f) {
      // Rotate around the X axis.
      m.setToIdentity();
      m.m[1][1] = c;
      m.m[2][2] = c;
      if (x < 0.0f) {
         m.m[2][1] = s;
         m.m[1][2] = -s;
      } else {
         m.m[2][1] = -s;
         m.m[1][2] = s;
      }
      m.flagBits = General;
      quick = true;
   }
   if (!quick) {
      qreal len = x * x + y * y + z * z;
      if (!qFuzzyIsNull(len - 1.0f) && !qFuzzyIsNull(len)) {
         len = qSqrt(len);
         x /= len;
         y /= len;
         z /= len;
      }
      ic = 1.0f - c;
      m.m[0][0] = x * x * ic + c;
      m.m[1][0] = x * y * ic - z * s;
      m.m[2][0] = x * z * ic + y * s;
      m.m[3][0] = 0.0f;
      m.m[0][1] = y * x * ic + z * s;
      m.m[1][1] = y * y * ic + c;
      m.m[2][1] = y * z * ic - x * s;
      m.m[3][1] = 0.0f;
      m.m[0][2] = x * z * ic - y * s;
      m.m[1][2] = y * z * ic + x * s;
      m.m[2][2] = z * z * ic + c;
      m.m[3][2] = 0.0f;
      m.m[0][3] = 0.0f;
      m.m[1][3] = 0.0f;
      m.m[2][3] = 0.0f;
      m.m[3][3] = 1.0f;
   }
   int flags = flagBits;
   *this *= m;
   if (flags != Identity) {
      flagBits = flags | Rotation;
   } else {
      flagBits = Rotation;
   }
}

void QMatrix4x4::projectedRotate(qreal angle, qreal x, qreal y, qreal z)
{
   // Used by QGraphicsRotation::applyTo() to perform a rotation
   // and projection back to 2D in a single step.
   if (angle == 0.0f) {
      return;
   }
   QMatrix4x4 m(1); // The "1" says to not load the identity.
   qreal c, s, ic;
   if (angle == 90.0f || angle == -270.0f) {
      s = 1.0f;
      c = 0.0f;
   } else if (angle == -90.0f || angle == 270.0f) {
      s = -1.0f;
      c = 0.0f;
   } else if (angle == 180.0f || angle == -180.0f) {
      s = 0.0f;
      c = -1.0f;
   } else {
      qreal a = angle * M_PI / 180.0f;
      c = qCos(a);
      s = qSin(a);
   }
   bool quick = false;
   if (x == 0.0f) {
      if (y == 0.0f) {
         if (z != 0.0f) {
            // Rotate around the Z axis.
            m.setToIdentity();
            m.m[0][0] = c;
            m.m[1][1] = c;
            if (z < 0.0f) {
               m.m[1][0] = s;
               m.m[0][1] = -s;
            } else {
               m.m[1][0] = -s;
               m.m[0][1] = s;
            }
            m.flagBits = General;
            quick = true;
         }
      } else if (z == 0.0f) {
         // Rotate around the Y axis.
         m.setToIdentity();
         m.m[0][0] = c;
         m.m[2][2] = 1.0f;

         if (y < 0.0f) {
            m.m[0][3] = -s * inv_dist_to_plane;
         } else {
            m.m[0][3] = s * inv_dist_to_plane;
         }
         m.flagBits = General;
         quick = true;
      }

   } else if (y == 0.0f && z == 0.0f) {
      // Rotate around the X axis.
      m.setToIdentity();
      m.m[1][1] = c;
      m.m[2][2] = 1.0f;

      if (x < 0.0f) {
         m.m[1][3] = s * inv_dist_to_plane;
      } else {
         m.m[1][3] = -s * inv_dist_to_plane;
      }
      m.flagBits = General;
      quick = true;
   }

   if (! quick) {
      qreal len = x * x + y * y + z * z;
      if (!qFuzzyIsNull(len - 1.0f) && !qFuzzyIsNull(len)) {
         len = qSqrt(len);
         x /= len;
         y /= len;
         z /= len;
      }
      ic = 1.0f - c;
      m.m[0][0] = x * x * ic + c;
      m.m[1][0] = x * y * ic - z * s;
      m.m[2][0] = 0.0f;
      m.m[3][0] = 0.0f;
      m.m[0][1] = y * x * ic + z * s;
      m.m[1][1] = y * y * ic + c;
      m.m[2][1] = 0.0f;
      m.m[3][1] = 0.0f;
      m.m[0][2] = 0.0f;
      m.m[1][2] = 0.0f;
      m.m[2][2] = 1.0f;
      m.m[3][2] = 0.0f;
      m.m[0][3] = (x * z * ic - y * s) * -inv_dist_to_plane;
      m.m[1][3] = (y * z * ic + x * s) * -inv_dist_to_plane;
      m.m[2][3] = 0.0f;
      m.m[3][3] = 1.0f;
   }

   int flags = flagBits;
   *this *= m;

   if (flags != Identity) {
      flagBits = flags | Rotation;
   } else {
      flagBits = Rotation;
   }
}

#ifndef QT_NO_QUATERNION

void QMatrix4x4::rotate(const QQuaternion &quaternion)
{
   // Algorithm from: http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54
   QMatrix4x4 tmp(1);

   qreal xx = quaternion.x() * quaternion.x();
   qreal xy = quaternion.x() * quaternion.y();
   qreal xz = quaternion.x() * quaternion.z();
   qreal xw = quaternion.x() * quaternion.scalar();
   qreal yy = quaternion.y() * quaternion.y();
   qreal yz = quaternion.y() * quaternion.z();
   qreal yw = quaternion.y() * quaternion.scalar();
   qreal zz = quaternion.z() * quaternion.z();
   qreal zw = quaternion.z() * quaternion.scalar();

   tmp.m[0][0] = 1.0f - 2 * (yy + zz);
   tmp.m[1][0] =        2 * (xy - zw);
   tmp.m[2][0] =        2 * (xz + yw);
   tmp.m[3][0] = 0.0f;
   tmp.m[0][1] =        2 * (xy + zw);
   tmp.m[1][1] = 1.0f - 2 * (xx + zz);
   tmp.m[2][1] =        2 * (yz - xw);
   tmp.m[3][1] = 0.0f;
   tmp.m[0][2] =        2 * (xz - yw);
   tmp.m[1][2] =        2 * (yz + xw);
   tmp.m[2][2] = 1.0f - 2 * (xx + yy);
   tmp.m[3][2] = 0.0f;
   tmp.m[0][3] = 0.0f;
   tmp.m[1][3] = 0.0f;
   tmp.m[2][3] = 0.0f;
   tmp.m[3][3] = 1.0f;

   tmp.flagBits = QMatrix4x4::Rotation;

   QMatrix4x4 &self = *this;
   self *= tmp;
}

#endif

void QMatrix4x4::ortho(const QRect &rect)
{
   // Note: rect.right() and rect.bottom() subtract 1 in QRect,
   // which gives the location of a pixel within the rectangle,
   // instead of the extent of the rectangle.  We want the extent.
   // QRectF expresses the extent properly.
   ortho(rect.x(), rect.x() + rect.width(), rect.y() + rect.height(), rect.y(), -1.0f, 1.0f);
}

void QMatrix4x4::ortho(const QRectF &rect)
{
   ortho(rect.left(), rect.right(), rect.bottom(), rect.top(), -1.0f, 1.0f);
}

void QMatrix4x4::ortho(qreal left, qreal right, qreal bottom, qreal top, qreal nearPlane, qreal farPlane)
{
   // Bail out if the projection volume is zero-sized.
   if (left == right || bottom == top || nearPlane == farPlane) {
      return;
   }

   // Construct the projection.
   qreal width     = right - left;
   qreal invheight = top - bottom;
   qreal clip      = farPlane - nearPlane;

#ifndef QT_NO_VECTOR3D
   if (clip == 2.0f && (nearPlane + farPlane) == 0.0f) {
      // We can express this projection as a translate and scale
      // which will be more efficient to modify with further
      // transformations than producing a "General" matrix.

      translate(QVector3D(-(left + right) / width, -(top + bottom) / invheight, 0.0f));
      scale(QVector3D(2.0f / width, 2.0f / invheight, -1.0f));

      return;
   }
#endif

   QMatrix4x4 tmp(1);

   tmp.m[0][0] = 2.0f / width;
   tmp.m[1][0] = 0.0f;
   tmp.m[2][0] = 0.0f;
   tmp.m[3][0] = -(left + right) / width;
   tmp.m[0][1] = 0.0f;
   tmp.m[1][1] = 2.0f / invheight;
   tmp.m[2][1] = 0.0f;
   tmp.m[3][1] = -(top + bottom) / invheight;
   tmp.m[0][2] = 0.0f;
   tmp.m[1][2] = 0.0f;
   tmp.m[2][2] = -2.0f / clip;
   tmp.m[3][2] = -(nearPlane + farPlane) / clip;
   tmp.m[0][3] = 0.0f;
   tmp.m[1][3] = 0.0f;
   tmp.m[2][3] = 0.0f;
   tmp.m[3][3] = 1.0f;

   tmp.flagBits = QMatrix4x4::General;

   // Apply the projection
   QMatrix4x4 &self = *this;
   self *= tmp;

   return;
}

void QMatrix4x4::frustum(qreal left, qreal right, qreal bottom, qreal top, qreal nearPlane, qreal farPlane)
{
   // Bail out if the projection volume is zero-sized.
   if (left == right || bottom == top || nearPlane == farPlane) {
      return;
   }

   // Construct the projection
   QMatrix4x4 tmp(1);

   qreal width     = right - left;
   qreal invheight = top - bottom;
   qreal clip      = farPlane - nearPlane;

   tmp.m[0][0] = 2.0f * nearPlane / width;
   tmp.m[1][0] = 0.0f;
   tmp.m[2][0] = (left + right) / width;
   tmp.m[3][0] = 0.0f;
   tmp.m[0][1] = 0.0f;
   tmp.m[1][1] = 2.0f * nearPlane / invheight;
   tmp.m[2][1] = (top + bottom) / invheight;
   tmp.m[3][1] = 0.0f;
   tmp.m[0][2] = 0.0f;
   tmp.m[1][2] = 0.0f;
   tmp.m[2][2] = -(nearPlane + farPlane) / clip;
   tmp.m[3][2] = -2.0f * nearPlane * farPlane / clip;
   tmp.m[0][3] = 0.0f;
   tmp.m[1][3] = 0.0f;
   tmp.m[2][3] = -1.0f;
   tmp.m[3][3] = 0.0f;

   tmp.flagBits = QMatrix4x4::General;

   // Apply the projection
   QMatrix4x4 &self = *this;
   self *= tmp;
}

void QMatrix4x4::perspective(qreal angle, qreal aspect, qreal nearPlane, qreal farPlane)
{
   // Bail out if the projection volume is zero-sized.
   if (nearPlane == farPlane || aspect == 0.0f) {
      return;
   }

   // Construct the projection.
   QMatrix4x4 tmp(1);

   qreal radians = (angle / 2.0f) * M_PI / 180.0f;
   qreal sine = qSin(radians);

   if (sine == 0.0f) {
      return;
   }

   qreal cotan = qCos(radians) / sine;
   qreal clip = farPlane - nearPlane;

   tmp.m[0][0] = cotan / aspect;
   tmp.m[1][0] = 0.0f;
   tmp.m[2][0] = 0.0f;
   tmp.m[3][0] = 0.0f;
   tmp.m[0][1] = 0.0f;
   tmp.m[1][1] = cotan;
   tmp.m[2][1] = 0.0f;
   tmp.m[3][1] = 0.0f;
   tmp.m[0][2] = 0.0f;
   tmp.m[1][2] = 0.0f;
   tmp.m[2][2] = -(nearPlane + farPlane) / clip;
   tmp.m[3][2] = -(2.0f * nearPlane * farPlane) / clip;
   tmp.m[0][3] = 0.0f;
   tmp.m[1][3] = 0.0f;
   tmp.m[2][3] = -1.0f;
   tmp.m[3][3] = 0.0f;

   tmp.flagBits = QMatrix4x4::Perspective;

   // Apply the projection
   QMatrix4x4 &self = *this;
   self *= tmp;
}

#ifndef QT_NO_VECTOR3D

void QMatrix4x4::lookAt(const QVector3D &eye, const QVector3D &center, const QVector3D &up)
{
   QVector3D forward  = (center - eye).normalized();
   QVector3D side     = QVector3D::crossProduct(forward, up).normalized();
   QVector3D upVector = QVector3D::crossProduct(side, forward);

   QMatrix4x4 tmp(1);

   tmp.m[0][0] = side.x();
   tmp.m[1][0] = side.y();
   tmp.m[2][0] = side.z();
   tmp.m[3][0] = 0.0f;
   tmp.m[0][1] = upVector.x();
   tmp.m[1][1] = upVector.y();
   tmp.m[2][1] = upVector.z();
   tmp.m[3][1] = 0.0f;
   tmp.m[0][2] = -forward.x();
   tmp.m[1][2] = -forward.y();
   tmp.m[2][2] = -forward.z();
   tmp.m[3][2] = 0.0f;
   tmp.m[0][3] = 0.0f;
   tmp.m[1][3] = 0.0f;
   tmp.m[2][3] = 0.0f;
   tmp.m[3][3] = 1.0f;

   tmp.flagBits = QMatrix4x4::General;

   // Apply the projection
   QMatrix4x4 &self = *this;
   self *= tmp;

   translate(-eye);
}

#endif

void QMatrix4x4::viewport(qreal left, qreal  bottom, qreal  width, qreal  height, qreal  nearPlane, qreal farPlane)
{
    const float w2 = width / 2.0f;
    const float h2 = height / 2.0f;

    QMatrix4x4 m(1);
    m.m[0][0] = w2;
    m.m[1][0] = 0.0f;
    m.m[2][0] = 0.0f;
    m.m[3][0] = left + w2;
    m.m[0][1] = 0.0f;
    m.m[1][1] = h2;
    m.m[2][1] = 0.0f;
    m.m[3][1] = bottom + h2;
    m.m[0][2] = 0.0f;
    m.m[1][2] = 0.0f;
    m.m[2][2] = (farPlane - nearPlane) / 2.0f;
    m.m[3][2] = (nearPlane + farPlane) / 2.0f;
    m.m[0][3] = 0.0f;
    m.m[1][3] = 0.0f;
    m.m[2][3] = 0.0f;
    m.m[3][3] = 1.0f;
    m.flagBits = General;

    *this *= m;
}

void QMatrix4x4::flipCoordinates()
{
   if (flagBits == Scale || flagBits == (Scale | Translation)) {
      m[1][1] = -m[1][1];
      m[2][2] = -m[2][2];
   } else if (flagBits == Translation) {
      m[1][1] = -m[1][1];
      m[2][2] = -m[2][2];
      flagBits |= Scale;
   } else if (flagBits == Identity) {
      m[1][1] = -1.0f;
      m[2][2] = -1.0f;
      flagBits = Scale;
   } else {
      m[1][0] = -m[1][0];
      m[1][1] = -m[1][1];
      m[1][2] = -m[1][2];
      m[1][3] = -m[1][3];
      m[2][0] = -m[2][0];
      m[2][1] = -m[2][1];
      m[2][2] = -m[2][2];
      m[2][3] = -m[2][3];
      flagBits = General;
   }
}

void QMatrix4x4::copyDataTo(qreal *values) const
{
   for (int row = 0; row < 4; ++row)
      for (int col = 0; col < 4; ++col) {
         values[row * 4 + col] = qreal(m[col][row]);
      }
}

QMatrix QMatrix4x4::toAffine() const
{
   return QMatrix(m[0][0], m[0][1],
                  m[1][0], m[1][1],
                  m[3][0], m[3][1]);
}

QTransform QMatrix4x4::toTransform() const
{
   return QTransform(m[0][0], m[0][1], m[0][3],
                     m[1][0], m[1][1], m[1][3],
                     m[3][0], m[3][1], m[3][3]);
}

QTransform QMatrix4x4::toTransform(qreal distanceToPlane) const
{
   if (distanceToPlane == 1024.0f) {
      // Optimize the common case with constants.
      return QTransform(m[0][0], m[0][1],
                        m[0][3] - m[0][2] * inv_dist_to_plane,
                        m[1][0], m[1][1],
                        m[1][3] - m[1][2] * inv_dist_to_plane,
                        m[3][0], m[3][1],
                        m[3][3] - m[3][2] * inv_dist_to_plane);
   } else if (distanceToPlane != 0.0f) {
      // The following projection matrix is pre-multiplied with "matrix":
      //      | 1 0 0 0 |
      //      | 0 1 0 0 |
      //      | 0 0 1 0 |
      //      | 0 0 d 1 |
      // where d = -1 / distanceToPlane.  After projection, row 3 and
      // column 3 are dropped to form the final QTransform.
      qreal d = 1.0f / distanceToPlane;
      return QTransform(m[0][0], m[0][1], m[0][3] - m[0][2] * d,
                        m[1][0], m[1][1], m[1][3] - m[1][2] * d,
                        m[3][0], m[3][1], m[3][3] - m[3][2] * d);
   } else {
      // Orthographic projection: drop row 3 and column 3.
      return QTransform(m[0][0], m[0][1], m[0][3],
                        m[1][0], m[1][1], m[1][3],
                        m[3][0], m[3][1], m[3][3]);
   }
}

QRect QMatrix4x4::mapRect(const QRect &rect) const
{
   if (flagBits == (Translation | Scale) || flagBits == Scale) {
      qreal x = rect.x() * m[0][0] + m[3][0];
      qreal y = rect.y() * m[1][1] + m[3][1];
      qreal w = rect.width() * m[0][0];
      qreal h = rect.height() * m[1][1];
      if (w < 0) {
         w = -w;
         x -= w;
      }
      if (h < 0) {
         h = -h;
         y -= h;
      }
      return QRect(qRound(x), qRound(y), qRound(w), qRound(h));
   } else if (flagBits == Translation) {
      return QRect(qRound(rect.x() + m[3][0]),
                   qRound(rect.y() + m[3][1]),
                   rect.width(), rect.height());
   }

   QPoint tl = map(rect.topLeft());
   QPoint tr = map(QPoint(rect.x() + rect.width(), rect.y()));
   QPoint bl = map(QPoint(rect.x(), rect.y() + rect.height()));
   QPoint br = map(QPoint(rect.x() + rect.width(),
                          rect.y() + rect.height()));

   int xmin = qMin(qMin(tl.x(), tr.x()), qMin(bl.x(), br.x()));
   int xmax = qMax(qMax(tl.x(), tr.x()), qMax(bl.x(), br.x()));
   int ymin = qMin(qMin(tl.y(), tr.y()), qMin(bl.y(), br.y()));
   int ymax = qMax(qMax(tl.y(), tr.y()), qMax(bl.y(), br.y()));

   return QRect(xmin, ymin, xmax - xmin, ymax - ymin);
}

QRectF QMatrix4x4::mapRect(const QRectF &rect) const
{
   if (flagBits == (Translation | Scale) || flagBits == Scale) {
      qreal x = rect.x() * m[0][0] + m[3][0];
      qreal y = rect.y() * m[1][1] + m[3][1];
      qreal w = rect.width() * m[0][0];
      qreal h = rect.height() * m[1][1];
      if (w < 0) {
         w = -w;
         x -= w;
      }
      if (h < 0) {
         h = -h;
         y -= h;
      }
      return QRectF(x, y, w, h);
   } else if (flagBits == Translation) {
      return rect.translated(m[3][0], m[3][1]);
   }

   QPointF tl = map(rect.topLeft());
   QPointF tr = map(rect.topRight());
   QPointF bl = map(rect.bottomLeft());
   QPointF br = map(rect.bottomRight());

   qreal xmin = qMin(qMin(tl.x(), tr.x()), qMin(bl.x(), br.x()));
   qreal xmax = qMax(qMax(tl.x(), tr.x()), qMax(bl.x(), br.x()));
   qreal ymin = qMin(qMin(tl.y(), tr.y()), qMin(bl.y(), br.y()));
   qreal ymax = qMax(qMax(tl.y(), tr.y()), qMax(bl.y(), br.y()));

   return QRectF(QPointF(xmin, ymin), QPointF(xmax, ymax));
}

// Helper routine for inverting orthonormal matrices that consist
// of just rotations and translations.
QMatrix4x4 QMatrix4x4::orthonormalInverse() const
{
   QMatrix4x4 result(1);  // The '1' says not to load identity

   result.m[0][0] = m[0][0];
   result.m[1][0] = m[0][1];
   result.m[2][0] = m[0][2];

   result.m[0][1] = m[1][0];
   result.m[1][1] = m[1][1];
   result.m[2][1] = m[1][2];

   result.m[0][2] = m[2][0];
   result.m[1][2] = m[2][1];
   result.m[2][2] = m[2][2];

   result.m[0][3] = 0.0f;
   result.m[1][3] = 0.0f;
   result.m[2][3] = 0.0f;

   result.m[3][0] = -(result.m[0][0] * m[3][0] + result.m[1][0] * m[3][1] + result.m[2][0] * m[3][2]);
   result.m[3][1] = -(result.m[0][1] * m[3][0] + result.m[1][1] * m[3][1] + result.m[2][1] * m[3][2]);
   result.m[3][2] = -(result.m[0][2] * m[3][0] + result.m[1][2] * m[3][1] + result.m[2][2] * m[3][2]);
   result.m[3][3] = 1.0f;

   return result;
}

void QMatrix4x4::optimize()
{
   // If the last element is not 1, then it can never be special.
   if (m[3][3] != 1.0f) {
      flagBits = General;
      return;
   }

   // If the upper three elements m12, m13, and m21 are not all zero,
   // or the lower elements below the diagonal are not all zero, then
   // the matrix can never be special.
   if (m[1][0] != 0.0f || m[2][0] != 0.0f || m[2][1] != 0.0f) {
      flagBits = General;
      return;
   }
   if (m[0][1] != 0.0f || m[0][2] != 0.0f || m[0][3] != 0.0f ||
         m[1][2] != 0.0f || m[1][3] != 0.0f || m[2][3] != 0.0f) {
      flagBits = General;
      return;
   }

   // Determine what we have in the remaining regions of the matrix.
   bool identityAlongDiagonal
      = (m[0][0] == 1.0f && m[1][1] == 1.0f && m[2][2] == 1.0f);
   bool translationPresent
      = (m[3][0] != 0.0f || m[3][1] != 0.0f || m[3][2] != 0.0f);

   // Now determine the special matrix type.
   if (translationPresent && identityAlongDiagonal) {
      flagBits = Translation;
   } else if (translationPresent) {
      flagBits = (Translation | Scale);
   } else if (identityAlongDiagonal) {
      flagBits = Identity;
   } else {
      flagBits = Scale;
   }
}

QMatrix4x4::operator QVariant() const
{
   return QVariant(QVariant::Matrix4x4, this);
}

QDebug operator<<(QDebug dbg, const QMatrix4x4 &m)
{
   // Create a string that represents the matrix type.
   QByteArray bits;
   if ((m.flagBits & QMatrix4x4::Identity) != 0) {
      bits += "Identity,";
   }
   if ((m.flagBits & QMatrix4x4::General) != 0) {
      bits += "General,";
   }
   if ((m.flagBits & QMatrix4x4::Translation) != 0) {
      bits += "Translation,";
   }
   if ((m.flagBits & QMatrix4x4::Scale) != 0) {
      bits += "Scale,";
   }
   if ((m.flagBits & QMatrix4x4::Rotation) != 0) {
      bits += "Rotation,";
   }
   if (bits.size() > 0) {
      bits = bits.left(bits.size() - 1);
   }

   // Output in row-major order because it is more human-readable.
   dbg.nospace() << "QMatrix4x4(type:" << bits.constData() << endl
                 << qSetFieldWidth(10)
                 << m(0, 0) << m(0, 1) << m(0, 2) << m(0, 3) << endl
                 << m(1, 0) << m(1, 1) << m(1, 2) << m(1, 3) << endl
                 << m(2, 0) << m(2, 1) << m(2, 2) << m(2, 3) << endl
                 << m(3, 0) << m(3, 1) << m(3, 2) << m(3, 3) << endl
                 << qSetFieldWidth(0) << ')';
   return dbg.space();
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &stream, const QMatrix4x4 &matrix)
{
   for (int row = 0; row < 4; ++row)
      for (int col = 0; col < 4; ++col) {
         stream << double(matrix(row, col));
      }
   return stream;
}

QDataStream &operator>>(QDataStream &stream, QMatrix4x4 &matrix)
{
   double x;
   for (int row = 0; row < 4; ++row) {
      for (int col = 0; col < 4; ++col) {
         stream >> x;
         matrix(row, col) = qreal(x);
      }
   }
   matrix.optimize();
   return stream;
}

#endif // QT_NO_DATASTREAM

#endif // QT_NO_MATRIX4X4
