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

#include <qquaternion.h>

#include <qdebug.h>
#include <qmath.h>
#include <qvariant.h>

#ifndef QT_NO_QUATERNION

qreal QQuaternion::length() const
{
   return qSqrt(xp * xp + yp * yp + zp * zp + wp * wp);
}

qreal QQuaternion::lengthSquared() const
{
   return xp * xp + yp * yp + zp * zp + wp * wp;
}

QQuaternion QQuaternion::normalized() const
{
   // Need some extra precision if the length is very small.
   double len = double(xp) * double(xp) +
                double(yp) * double(yp) +
                double(zp) * double(zp) +
                double(wp) * double(wp);

   if (qFuzzyIsNull(len - 1.0f)) {
      return *this;
   } else if (!qFuzzyIsNull(len)) {
      return *this / qSqrt(len);
   } else {
      return QQuaternion(0.0f, 0.0f, 0.0f, 0.0f);
   }
}

void QQuaternion::normalize()
{
   // Need some extra precision if the length is very small.
   double len = double(xp) * double(xp) +
                double(yp) * double(yp) +
                double(zp) * double(zp) +
                double(wp) * double(wp);
   if (qFuzzyIsNull(len - 1.0f) || qFuzzyIsNull(len)) {
      return;
   }

   len = qSqrt(len);

   xp /= len;
   yp /= len;
   zp /= len;
   wp /= len;
}

QVector3D QQuaternion::rotatedVector(const QVector3D &vector) const
{
   return (*this * QQuaternion(0, vector) * conjugate()).vector();
}

#ifndef QT_NO_VECTOR3D

QQuaternion QQuaternion::fromAxisAndAngle(const QVector3D &axis, qreal angle)
{
   // Algorithm from:
   // http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q56
   // We normalize the result just in case the values are close
   // to zero, as suggested in the above FAQ.
   qreal a = (angle / 2.0f) * M_PI / 180.0f;
   qreal s = qSin(a);
   qreal c = qCos(a);
   QVector3D ax = axis.normalized();
   return QQuaternion(c, ax.x() * s, ax.y() * s, ax.z() * s).normalized();
}

#endif

QQuaternion QQuaternion::fromAxisAndAngle(qreal x, qreal y, qreal z, qreal angle)
{
   qreal length = qSqrt(x * x + y * y + z * z);
   if (!qFuzzyIsNull(length - 1.0f) && !qFuzzyIsNull(length)) {
      x /= length;
      y /= length;
      z /= length;
   }
   qreal a = (angle / 2.0f) * M_PI / 180.0f;
   qreal s = qSin(a);
   qreal c = qCos(a);
   return QQuaternion(c, x * s, y * s, z * s).normalized();
}

QQuaternion QQuaternion::slerp(const QQuaternion &q1, const QQuaternion &q2, qreal t)
{
   // Handle the easy cases first.
   if (t <= 0.0f) {
      return q1;
   } else if (t >= 1.0f) {
      return q2;
   }

   // Determine the angle between the two quaternions.
   QQuaternion q2b;
   qreal dot;
   dot = q1.xp * q2.xp + q1.yp * q2.yp + q1.zp * q2.zp + q1.wp * q2.wp;
   if (dot >= 0.0f) {
      q2b = q2;
   } else {
      q2b = -q2;
      dot = -dot;
   }

   // Get the scale factors.  If they are too small,
   // then revert to simple linear interpolation.
   qreal factor1 = 1.0f - t;
   qreal factor2 = t;
   if ((1.0f - dot) > 0.0000001) {
      qreal angle = qreal(qAcos(dot));
      qreal sinOfAngle = qreal(qSin(angle));
      if (sinOfAngle > 0.0000001) {
         factor1 = qreal(qSin((1.0f - t) * angle)) / sinOfAngle;
         factor2 = qreal(qSin(t * angle)) / sinOfAngle;
      }
   }

   // Construct the result quaternion.
   return q1 * factor1 + q2b * factor2;
}

QQuaternion QQuaternion::nlerp(const QQuaternion &q1, const QQuaternion &q2, qreal t)
{
   // Handle the easy cases first.
   if (t <= 0.0f) {
      return q1;
   } else if (t >= 1.0f) {
      return q2;
   }

   // Determine the angle between the two quaternions.
   QQuaternion q2b;
   qreal dot;
   dot = q1.xp * q2.xp + q1.yp * q2.yp + q1.zp * q2.zp + q1.wp * q2.wp;
   if (dot >= 0.0f) {
      q2b = q2;
   } else {
      q2b = -q2;
   }

   // Perform the linear interpolation.
   return (q1 * (1.0f - t) + q2b * t).normalized();
}

/*!
    Returns the quaternion as a QVariant.
*/
QQuaternion::operator QVariant() const
{
   return QVariant(QVariant::Quaternion, this);
}

QDebug operator<<(QDebug dbg, const QQuaternion &q)
{
   dbg.nospace() << "QQuaternion(scalar:" << q.scalar()
                 << ", vector:(" << q.x() << ", "
                 << q.y() << ", " << q.z() << "))";
   return dbg.space();
}

QDataStream &operator<<(QDataStream &stream, const QQuaternion &quaternion)
{
   stream << double(quaternion.scalar()) << double(quaternion.x())
          << double(quaternion.y()) << double(quaternion.z());
   return stream;
}

QDataStream &operator>>(QDataStream &stream, QQuaternion &quaternion)
{
   double scalar, x, y, z;
   stream >> scalar;
   stream >> x;
   stream >> y;
   stream >> z;
   quaternion.setScalar(qreal(scalar));
   quaternion.setX(qreal(x));
   quaternion.setY(qreal(y));
   quaternion.setZ(qreal(z));
   return stream;
}

#endif

