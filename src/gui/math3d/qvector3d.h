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

#ifndef QVECTOR3D_H
#define QVECTOR3D_H

#include <qpoint.h>
#include <qvariant.h>

class QMatrix4x4;
class QVector2D;
class QVector4D;

#ifndef QT_NO_VECTOR3D

class Q_GUI_EXPORT QVector3D
{
 public:
   QVector3D();
   QVector3D(qreal xpos, qreal ypos, qreal zpos);
   explicit QVector3D(const QPoint &point);
   explicit QVector3D(const QPointF &point);

#ifndef QT_NO_VECTOR2D
   QVector3D(const QVector2D &vector);
   QVector3D(const QVector2D &vector, qreal zpos);
#endif

#ifndef QT_NO_VECTOR4D
   explicit QVector3D(const QVector4D &vector);
#endif

   bool isNull() const;

   qreal x() const;
   qreal y() const;
   qreal z() const;

   void setX(qreal x);
   void setY(qreal y);
   void setZ(qreal z);

   qreal length() const;
   qreal lengthSquared() const;

   QVector3D normalized() const;
   void normalize();

   QVector3D &operator+=(const QVector3D &vector);
   QVector3D &operator-=(const QVector3D &vector);
   QVector3D &operator*=(qreal factor);
   QVector3D &operator*=(const QVector3D &vector);
   QVector3D &operator/=(qreal divisor);

   static qreal dotProduct(const QVector3D &v1, const QVector3D &v2);
   static QVector3D crossProduct(const QVector3D &v1, const QVector3D &v2);
   static QVector3D normal(const QVector3D &v1, const QVector3D &v2);
   static QVector3D normal
   (const QVector3D &v1, const QVector3D &v2, const QVector3D &v3);

   qreal distanceToPlane(const QVector3D &plane, const QVector3D &normal) const;
   qreal distanceToPlane(const QVector3D &plane1, const QVector3D &plane2, const QVector3D &plane3) const;
   qreal distanceToLine(const QVector3D &point, const QVector3D &direction) const;

   friend inline bool operator==(const QVector3D &v1, const QVector3D &v2);
   friend inline bool operator!=(const QVector3D &v1, const QVector3D &v2);
   friend inline const QVector3D operator+(const QVector3D &v1, const QVector3D &v2);
   friend inline const QVector3D operator-(const QVector3D &v1, const QVector3D &v2);
   friend inline const QVector3D operator*(qreal factor, const QVector3D &vector);
   friend inline const QVector3D operator*(const QVector3D &vector, qreal factor);
   friend const QVector3D operator*(const QVector3D &v1, const QVector3D &v2);
   friend inline const QVector3D operator-(const QVector3D &vector);
   friend inline const QVector3D operator/(const QVector3D &vector, qreal divisor);

   friend inline bool qFuzzyCompare(const QVector3D &v1, const QVector3D &v2);

#ifndef QT_NO_VECTOR2D
   QVector2D toVector2D() const;
#endif

#ifndef QT_NO_VECTOR4D
   QVector4D toVector4D() const;
#endif

   QPoint toPoint() const;
   QPointF toPointF() const;

   operator QVariant() const;

 private:
   float xp, yp, zp;

   QVector3D(float xpos, float ypos, float zpos, int dummy);

   friend class QVector2D;
   friend class QVector4D;

#ifndef QT_NO_MATRIX4X4
   friend QVector3D operator*(const QVector3D &vector, const QMatrix4x4 &matrix);
   friend QVector3D operator*(const QMatrix4x4 &matrix, const QVector3D &vector);
#endif
};

inline QVector3D::QVector3D() : xp(0.0f), yp(0.0f), zp(0.0f) {}

inline QVector3D::QVector3D(qreal xpos, qreal ypos, qreal zpos) : xp(xpos), yp(ypos), zp(zpos) {}

inline QVector3D::QVector3D(float xpos, float ypos, float zpos, int) : xp(xpos), yp(ypos), zp(zpos) {}

inline QVector3D::QVector3D(const QPoint &point) : xp(point.x()), yp(point.y()), zp(0.0f) {}

inline QVector3D::QVector3D(const QPointF &point) : xp(point.x()), yp(point.y()), zp(0.0f) {}

inline bool QVector3D::isNull() const
{
   return qIsNull(xp) && qIsNull(yp) && qIsNull(zp);
}

inline qreal QVector3D::x() const
{
   return qreal(xp);
}

inline qreal QVector3D::y() const
{
   return qreal(yp);
}

inline qreal QVector3D::z() const
{
   return qreal(zp);
}

inline void QVector3D::setX(qreal x)
{
   xp = x;
}

inline void QVector3D::setY(qreal y)
{
   yp = y;
}

inline void QVector3D::setZ(qreal z)
{
   zp = z;
}

inline QVector3D &QVector3D::operator+=(const QVector3D &vector)
{
   xp += vector.xp;
   yp += vector.yp;
   zp += vector.zp;
   return *this;
}

inline QVector3D &QVector3D::operator-=(const QVector3D &vector)
{
   xp -= vector.xp;
   yp -= vector.yp;
   zp -= vector.zp;
   return *this;
}

inline QVector3D &QVector3D::operator*=(qreal factor)
{
   xp *= factor;
   yp *= factor;
   zp *= factor;
   return *this;
}

inline QVector3D &QVector3D::operator*=(const QVector3D &vector)
{
   xp *= vector.xp;
   yp *= vector.yp;
   zp *= vector.zp;
   return *this;
}

inline QVector3D &QVector3D::operator/=(qreal divisor)
{
   xp /= divisor;
   yp /= divisor;
   zp /= divisor;
   return *this;
}

inline bool operator==(const QVector3D &v1, const QVector3D &v2)
{
   return v1.xp == v2.xp && v1.yp == v2.yp && v1.zp == v2.zp;
}

inline bool operator!=(const QVector3D &v1, const QVector3D &v2)
{
   return v1.xp != v2.xp || v1.yp != v2.yp || v1.zp != v2.zp;
}

inline const QVector3D operator+(const QVector3D &v1, const QVector3D &v2)
{
   return QVector3D(v1.xp + v2.xp, v1.yp + v2.yp, v1.zp + v2.zp, 1);
}

inline const QVector3D operator-(const QVector3D &v1, const QVector3D &v2)
{
   return QVector3D(v1.xp - v2.xp, v1.yp - v2.yp, v1.zp - v2.zp, 1);
}

inline const QVector3D operator*(qreal factor, const QVector3D &vector)
{
   return QVector3D(vector.xp * factor, vector.yp * factor, vector.zp * factor, 1);
}

inline const QVector3D operator*(const QVector3D &vector, qreal factor)
{
   return QVector3D(vector.xp * factor, vector.yp * factor, vector.zp * factor, 1);
}

inline const QVector3D operator*(const QVector3D &v1, const QVector3D &v2)
{
   return QVector3D(v1.xp * v2.xp, v1.yp * v2.yp, v1.zp * v2.zp, 1);
}

inline const QVector3D operator-(const QVector3D &vector)
{
   return QVector3D(-vector.xp, -vector.yp, -vector.zp, 1);
}

inline const QVector3D operator/(const QVector3D &vector, qreal divisor)
{
   return QVector3D(vector.xp / divisor, vector.yp / divisor, vector.zp / divisor, 1);
}

inline bool qFuzzyCompare(const QVector3D &v1, const QVector3D &v2)
{
   return qFuzzyCompare(v1.xp, v2.xp) && qFuzzyCompare(v1.yp, v2.yp) && qFuzzyCompare(v1.zp, v2.zp);
}

inline QPoint QVector3D::toPoint() const
{
   return QPoint(qRound(xp), qRound(yp));
}

inline QPointF QVector3D::toPointF() const
{
   return QPointF(qreal(xp), qreal(yp));
}

Q_GUI_EXPORT QDebug operator<<(QDebug dbg, const QVector3D &vector);

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QVector3D &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QVector3D &);

#endif

#endif
