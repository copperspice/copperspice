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

#ifndef QQUATERNION_H
#define QQUATERNION_H

#include <qvector3d.h>
#include <qvector4d.h>

#ifndef QT_NO_QUATERNION

class QMatrix4x4;
class QVariant;

class Q_GUI_EXPORT QQuaternion
{

 public:
   QQuaternion();
   QQuaternion(qreal scalar, qreal xpos, qreal ypos, qreal zpos);

#ifndef QT_NO_VECTOR3D
   QQuaternion(qreal scalar, const QVector3D &vector);
#endif

#ifndef QT_NO_VECTOR4D
   explicit QQuaternion(const QVector4D &vector);
#endif

   bool isNull() const;
   bool isIdentity() const;

#ifndef QT_NO_VECTOR3D
   QVector3D vector() const;
   void setVector(const QVector3D &vector);
#endif

   void setVector(qreal x, qreal y, qreal z);

   qreal x() const;
   qreal y() const;
   qreal z() const;
   qreal scalar() const;

   void setX(qreal x);
   void setY(qreal y);
   void setZ(qreal z);
   void setScalar(qreal scalar);

   qreal length() const;
   qreal lengthSquared() const;

   QQuaternion normalized() const;
   void normalize();

   QQuaternion conjugate() const;

   QVector3D rotatedVector(const QVector3D &vector) const;

   QQuaternion &operator+=(const QQuaternion &quaternion);
   QQuaternion &operator-=(const QQuaternion &quaternion);
   QQuaternion &operator*=(qreal factor);
   QQuaternion &operator*=(const QQuaternion &quaternion);
   QQuaternion &operator/=(qreal divisor);

   friend inline bool operator==(const QQuaternion &q1, const QQuaternion &q2);
   friend inline bool operator!=(const QQuaternion &q1, const QQuaternion &q2);
   friend inline const QQuaternion operator+(const QQuaternion &q1, const QQuaternion &q2);
   friend inline const QQuaternion operator-(const QQuaternion &q1, const QQuaternion &q2);
   friend inline const QQuaternion operator*(qreal factor, const QQuaternion &quaternion);
   friend inline const QQuaternion operator*(const QQuaternion &quaternion, qreal factor);
   friend inline const QQuaternion operator*(const QQuaternion &q1, const QQuaternion &q2);
   friend inline const QQuaternion operator-(const QQuaternion &quaternion);
   friend inline const QQuaternion operator/(const QQuaternion &quaternion, qreal divisor);

   friend inline bool qFuzzyCompare(const QQuaternion &q1, const QQuaternion &q2);

#ifndef QT_NO_VECTOR4D
   QVector4D toVector4D() const;
#endif

   operator QVariant() const;

#ifndef QT_NO_VECTOR3D
   static QQuaternion fromAxisAndAngle(const QVector3D &axis, qreal angle);
#endif

   static QQuaternion fromAxisAndAngle
   (qreal x, qreal y, qreal z, qreal angle);

   static QQuaternion slerp
   (const QQuaternion &q1, const QQuaternion &q2, qreal t);
   static QQuaternion nlerp
   (const QQuaternion &q1, const QQuaternion &q2, qreal t);

 private:
   qreal wp, xp, yp, zp;
};

inline QQuaternion::QQuaternion()
   : wp(1.0f), xp(0.0f), yp(0.0f), zp(0.0f)
{
}

inline QQuaternion::QQuaternion(qreal scalar, qreal xpos, qreal ypos, qreal zpos)
   : wp(scalar), xp(xpos), yp(ypos), zp(zpos)
{
}

inline bool QQuaternion::isNull() const
{
   return qIsNull(xp) && qIsNull(yp) && qIsNull(zp) && qIsNull(wp);
}

inline bool QQuaternion::isIdentity() const
{
   return qIsNull(xp) && qIsNull(yp) && qIsNull(zp) && wp == 1.0f;
}

inline qreal QQuaternion::x() const
{
   return qreal(xp);
}

inline qreal QQuaternion::y() const
{
   return qreal(yp);
}

inline qreal QQuaternion::z() const
{
   return qreal(zp);

}

inline qreal QQuaternion::scalar() const
{
   return qreal(wp);
}

inline void QQuaternion::setX(qreal x)
{
   xp = x;
}

inline void QQuaternion::setY(qreal y)
{
   yp = y;
}

inline void QQuaternion::setZ(qreal z)
{
   zp = z;
}

inline void QQuaternion::setScalar(qreal scalar)
{
   wp = scalar;
}

inline QQuaternion QQuaternion::conjugate() const
{
   return QQuaternion(wp, -xp, -yp, -zp);
}

inline QQuaternion &QQuaternion::operator+=(const QQuaternion &quaternion)
{
   xp += quaternion.xp;
   yp += quaternion.yp;
   zp += quaternion.zp;
   wp += quaternion.wp;
   return *this;
}

inline QQuaternion &QQuaternion::operator-=(const QQuaternion &quaternion)
{
   xp -= quaternion.xp;
   yp -= quaternion.yp;
   zp -= quaternion.zp;
   wp -= quaternion.wp;
   return *this;
}

inline QQuaternion &QQuaternion::operator*=(qreal factor)
{
   xp *= factor;
   yp *= factor;
   zp *= factor;
   wp *= factor;
   return *this;
}

inline const QQuaternion operator*(const QQuaternion &q1, const QQuaternion &q2)
{
   qreal ww = (q1.zp + q1.xp) * (q2.xp + q2.yp);
   qreal yy = (q1.wp - q1.yp) * (q2.wp + q2.zp);
   qreal zz = (q1.wp + q1.yp) * (q2.wp - q2.zp);
   qreal xx = ww + yy + zz;
   qreal qq = 0.5 * (xx + (q1.zp - q1.xp) * (q2.xp - q2.yp));

   qreal w = qq - ww + (q1.zp - q1.yp) * (q2.yp - q2.zp);
   qreal x = qq - xx + (q1.xp + q1.wp) * (q2.xp + q2.wp);
   qreal y = qq - yy + (q1.wp - q1.xp) * (q2.yp + q2.zp);
   qreal z = qq - zz + (q1.zp + q1.yp) * (q2.wp - q2.xp);

   return QQuaternion(w, x, y, z);
}

inline QQuaternion &QQuaternion::operator*=(const QQuaternion &quaternion)
{
   *this = *this * quaternion;
   return *this;
}

inline QQuaternion &QQuaternion::operator/=(qreal divisor)
{
   xp /= divisor;
   yp /= divisor;
   zp /= divisor;
   wp /= divisor;
   return *this;
}

inline bool operator==(const QQuaternion &q1, const QQuaternion &q2)
{
   return q1.xp == q2.xp && q1.yp == q2.yp && q1.zp == q2.zp && q1.wp == q2.wp;
}

inline bool operator!=(const QQuaternion &q1, const QQuaternion &q2)
{
   return q1.xp != q2.xp || q1.yp != q2.yp || q1.zp != q2.zp || q1.wp != q2.wp;
}

inline const QQuaternion operator+(const QQuaternion &q1, const QQuaternion &q2)
{
   return QQuaternion(q1.wp + q2.wp, q1.xp + q2.xp, q1.yp + q2.yp, q1.zp + q2.zp);
}

inline const QQuaternion operator-(const QQuaternion &q1, const QQuaternion &q2)
{
   return QQuaternion(q1.wp - q2.wp, q1.xp - q2.xp, q1.yp - q2.yp, q1.zp - q2.zp);
}

inline const QQuaternion operator*(qreal factor, const QQuaternion &quaternion)
{
   return QQuaternion(quaternion.wp * factor, quaternion.xp * factor, quaternion.yp * factor, quaternion.zp * factor);
}

inline const QQuaternion operator*(const QQuaternion &quaternion, qreal factor)
{
   return QQuaternion(quaternion.wp * factor, quaternion.xp * factor, quaternion.yp * factor, quaternion.zp * factor);
}

inline const QQuaternion operator-(const QQuaternion &quaternion)
{
   return QQuaternion(-quaternion.wp, -quaternion.xp, -quaternion.yp, -quaternion.zp);
}

inline const QQuaternion operator/(const QQuaternion &quaternion, qreal divisor)
{
   return QQuaternion(quaternion.wp / divisor, quaternion.xp / divisor, quaternion.yp / divisor, quaternion.zp / divisor);
}

inline bool qFuzzyCompare(const QQuaternion &q1, const QQuaternion &q2)
{
   return qFuzzyCompare(q1.xp, q2.xp) &&
          qFuzzyCompare(q1.yp, q2.yp) &&
          qFuzzyCompare(q1.zp, q2.zp) &&
          qFuzzyCompare(q1.wp, q2.wp);
}

#ifndef QT_NO_VECTOR3D

inline QQuaternion::QQuaternion(qreal scalar, const QVector3D &vector)
   : wp(scalar), xp(vector.x()), yp(vector.y()), zp(vector.z()) {}

inline void QQuaternion::setVector(const QVector3D &vector)
{
   xp = vector.x();
   yp = vector.y();
   zp = vector.z();
}

inline QVector3D QQuaternion::vector() const
{
   return QVector3D(xp, yp, zp);
}

#endif

inline void QQuaternion::setVector(qreal x, qreal y, qreal z)
{
   xp = x;
   yp = y;
   zp = z;
}

#ifndef QT_NO_VECTOR4D

inline QQuaternion::QQuaternion(const QVector4D &vector)
   : wp(vector.w()), xp(vector.x()), yp(vector.y()), zp(vector.z())
{
}

inline QVector4D QQuaternion::toVector4D() const
{
   return QVector4D(xp, yp, zp, wp);
}

#endif

Q_GUI_EXPORT QDebug operator<<(QDebug dbg, const QQuaternion &q);

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QQuaternion &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QQuaternion &);

#endif

#endif
