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

#ifndef QVECTOR4D_H
#define QVECTOR4D_H

#include <qpoint.h>
#include <qvariant.h>

class QMatrix4x4;
class QVector2D;
class QVector3D;

#ifndef QT_NO_VECTOR4D

class Q_GUI_EXPORT QVector4D
{
 public:
   QVector4D();

   QVector4D(qreal xpos, qreal ypos, qreal zpos, qreal wpos);
   explicit QVector4D(const QPoint &point);
   explicit QVector4D(const QPointF &point);

#ifndef QT_NO_VECTOR2D
   QVector4D(const QVector2D &vector);
   QVector4D(const QVector2D &vector, qreal zpos, qreal wpos);
#endif

#ifndef QT_NO_VECTOR3D
   QVector4D(const QVector3D &vector);
   QVector4D(const QVector3D &vector, qreal wpos);
#endif

   bool isNull() const;

   qreal x() const;
   qreal y() const;
   qreal z() const;
   qreal w() const;

   void setX(qreal x);
   void setY(qreal y);
   void setZ(qreal z);
   void setW(qreal w);

   qreal length() const;
   qreal lengthSquared() const;

   QVector4D normalized() const;
   void normalize();

   QVector4D &operator+=(const QVector4D &vector);
   QVector4D &operator-=(const QVector4D &vector);
   QVector4D &operator*=(qreal factor);
   QVector4D &operator*=(const QVector4D &vector);
   QVector4D &operator/=(qreal divisor);

   static qreal dotProduct(const QVector4D &v1, const QVector4D &v2);

   friend inline bool operator==(const QVector4D &v1, const QVector4D &v2);
   friend inline bool operator!=(const QVector4D &v1, const QVector4D &v2);
   friend inline const QVector4D operator+(const QVector4D &v1, const QVector4D &v2);
   friend inline const QVector4D operator-(const QVector4D &v1, const QVector4D &v2);
   friend inline const QVector4D operator*(qreal factor, const QVector4D &vector);
   friend inline const QVector4D operator*(const QVector4D &vector, qreal factor);
   friend inline const QVector4D operator*(const QVector4D &v1, const QVector4D &v2);
   friend inline const QVector4D operator-(const QVector4D &vector);
   friend inline const QVector4D operator/(const QVector4D &vector, qreal divisor);

   friend inline bool qFuzzyCompare(const QVector4D &v1, const QVector4D &v2);

#ifndef QT_NO_VECTOR2D
   QVector2D toVector2D() const;
   QVector2D toVector2DAffine() const;
#endif

#ifndef QT_NO_VECTOR3D
   QVector3D toVector3D() const;
   QVector3D toVector3DAffine() const;
#endif

   QPoint toPoint() const;
   QPointF toPointF() const;

   operator QVariant() const;

 private:
   float xp, yp, zp, wp;

   QVector4D(float xpos, float ypos, float zpos, float wpos, int dummy);

   friend class QVector2D;
   friend class QVector3D;

#ifndef QT_NO_MATRIX4X4
   friend QVector4D operator*(const QVector4D &vector, const QMatrix4x4 &matrix);
   friend QVector4D operator*(const QMatrix4x4 &matrix, const QVector4D &vector);
#endif

};

inline QVector4D::QVector4D() : xp(0.0f), yp(0.0f), zp(0.0f), wp(0.0f)
{}

inline QVector4D::QVector4D(qreal xpos, qreal ypos, qreal zpos, qreal wpos) : xp(xpos), yp(ypos), zp(zpos), wp(wpos)
{}

inline QVector4D::QVector4D(float xpos, float ypos, float zpos, float wpos, int) : xp(xpos), yp(ypos), zp(zpos), wp(wpos)
{}

inline QVector4D::QVector4D(const QPoint &point) : xp(point.x()), yp(point.y()), zp(0.0f), wp(0.0f)
{}

inline QVector4D::QVector4D(const QPointF &point) : xp(point.x()), yp(point.y()), zp(0.0f), wp(0.0f)
{}

inline bool QVector4D::isNull() const
{
   return qIsNull(xp) && qIsNull(yp) && qIsNull(zp) && qIsNull(wp);
}

inline qreal QVector4D::x() const
{
   return qreal(xp);
}

inline qreal QVector4D::y() const
{
   return qreal(yp);
}

inline qreal QVector4D::z() const
{
   return qreal(zp);
}

inline qreal QVector4D::w() const
{
   return qreal(wp);
}

inline void QVector4D::setX(qreal x)
{
   xp = x;
}

inline void QVector4D::setY(qreal y)
{
   yp = y;
}

inline void QVector4D::setZ(qreal z)
{
   zp = z;
}

inline void QVector4D::setW(qreal w)
{
   wp = w;
}

inline QVector4D &QVector4D::operator+=(const QVector4D &vector)
{
   xp += vector.xp;
   yp += vector.yp;
   zp += vector.zp;
   wp += vector.wp;

   return *this;
}

inline QVector4D &QVector4D::operator-=(const QVector4D &vector)
{
   xp -= vector.xp;
   yp -= vector.yp;
   zp -= vector.zp;
   wp -= vector.wp;
   return *this;
}

inline QVector4D &QVector4D::operator*=(qreal factor)
{
   xp *= factor;
   yp *= factor;
   zp *= factor;
   wp *= factor;
   return *this;
}

inline QVector4D &QVector4D::operator*=(const QVector4D &vector)
{
   xp *= vector.xp;
   yp *= vector.yp;
   zp *= vector.zp;
   wp *= vector.wp;
   return *this;
}

inline QVector4D &QVector4D::operator/=(qreal divisor)
{
   xp /= divisor;
   yp /= divisor;
   zp /= divisor;
   wp /= divisor;
   return *this;
}

inline bool operator==(const QVector4D &v1, const QVector4D &v2)
{
   return v1.xp == v2.xp && v1.yp == v2.yp && v1.zp == v2.zp && v1.wp == v2.wp;
}

inline bool operator!=(const QVector4D &v1, const QVector4D &v2)
{
   return v1.xp != v2.xp || v1.yp != v2.yp || v1.zp != v2.zp || v1.wp != v2.wp;
}

inline const QVector4D operator+(const QVector4D &v1, const QVector4D &v2)
{
   return QVector4D(v1.xp + v2.xp, v1.yp + v2.yp, v1.zp + v2.zp, v1.wp + v2.wp, 1);
}

inline const QVector4D operator-(const QVector4D &v1, const QVector4D &v2)
{
   return QVector4D(v1.xp - v2.xp, v1.yp - v2.yp, v1.zp - v2.zp, v1.wp - v2.wp, 1);
}

inline const QVector4D operator*(qreal factor, const QVector4D &vector)
{
   return QVector4D(vector.xp * factor, vector.yp * factor, vector.zp * factor, vector.wp * factor, 1);
}

inline const QVector4D operator*(const QVector4D &vector, qreal factor)
{
   return QVector4D(vector.xp * factor, vector.yp * factor, vector.zp * factor, vector.wp * factor, 1);
}

inline const QVector4D operator*(const QVector4D &v1, const QVector4D &v2)
{
   return QVector4D(v1.xp * v2.xp, v1.yp * v2.yp, v1.zp * v2.zp, v1.wp * v2.wp, 1);
}

inline const QVector4D operator-(const QVector4D &vector)
{
   return QVector4D(-vector.xp, -vector.yp, -vector.zp, -vector.wp, 1);
}

inline const QVector4D operator/(const QVector4D &vector, qreal divisor)
{
   return QVector4D(vector.xp / divisor, vector.yp / divisor, vector.zp / divisor, vector.wp / divisor, 1);
}

inline bool qFuzzyCompare(const QVector4D &v1, const QVector4D &v2)
{
   return qFuzzyCompare(v1.xp, v2.xp) &&
          qFuzzyCompare(v1.yp, v2.yp) &&
          qFuzzyCompare(v1.zp, v2.zp) &&
          qFuzzyCompare(v1.wp, v2.wp);
}

inline QPoint QVector4D::toPoint() const
{
   return QPoint(qRound(xp), qRound(yp));
}

inline QPointF QVector4D::toPointF() const
{
   return QPointF(qreal(xp), qreal(yp));
}

Q_GUI_EXPORT QDebug operator<<(QDebug dbg, const QVector4D &vector);

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QVector4D &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QVector4D &);

#endif

#endif
