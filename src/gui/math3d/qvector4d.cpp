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

#include <qvector4d.h>
#include <qvector3d.h>
#include <qvector2d.h>
#include <qdebug.h>
#include <qvariant.h>
#include <qmath.h>

#ifndef QT_NO_VECTOR4D

#ifndef QT_NO_VECTOR2D

QVector4D::QVector4D(const QVector2D &vector)
{
   xp = vector.xp;
   yp = vector.yp;
   zp = 0.0f;
   wp = 0.0f;
}

QVector4D::QVector4D(const QVector2D &vector, qreal zpos, qreal wpos)
{
   xp = vector.xp;
   yp = vector.yp;
   zp = zpos;
   wp = wpos;
}

#endif

#ifndef QT_NO_VECTOR3D

QVector4D::QVector4D(const QVector3D &vector)
{
   xp = vector.xp;
   yp = vector.yp;
   zp = vector.zp;
   wp = 0.0f;
}

QVector4D::QVector4D(const QVector3D &vector, qreal wpos)
{
   xp = vector.xp;
   yp = vector.yp;
   zp = vector.zp;
   wp = wpos;
}

#endif

qreal QVector4D::length() const
{
   return qSqrt(xp * xp + yp * yp + zp * zp + wp * wp);
}

qreal QVector4D::lengthSquared() const
{
   return xp * xp + yp * yp + zp * zp + wp * wp;
}

QVector4D QVector4D::normalized() const
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
      return QVector4D();

   }
}

void QVector4D::normalize()
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

qreal QVector4D::dotProduct(const QVector4D &v1, const QVector4D &v2)
{
   return v1.xp * v2.xp + v1.yp * v2.yp + v1.zp * v2.zp + v1.wp * v2.wp;
}

#ifndef QT_NO_VECTOR2D

QVector2D QVector4D::toVector2D() const
{
   return QVector2D(xp, yp, 1);
}

QVector2D QVector4D::toVector2DAffine() const
{
   if (qIsNull(wp)) {
      return QVector2D();
   }
   return QVector2D(xp / wp, yp / wp, 1);
}

#endif

#ifndef QT_NO_VECTOR3D

QVector3D QVector4D::toVector3D() const
{
   return QVector3D(xp, yp, zp, 1);
}

QVector3D QVector4D::toVector3DAffine() const
{
   if (qIsNull(wp)) {
      return QVector3D();
   }
   return QVector3D(xp / wp, yp / wp, zp / wp, 1);
}

#endif

QVector4D::operator QVariant() const
{
   return QVariant(QVariant::Vector4D, this);
}

QDebug operator<<(QDebug dbg, const QVector4D &vector)
{
   dbg.nospace() << "QVector4D("
                 << vector.x() << ", " << vector.y() << ", "
                 << vector.z() << ", " << vector.w() << ')';
   return dbg.space();
}

QDataStream &operator<<(QDataStream &stream, const QVector4D &vector)
{
   stream << double(vector.x()) << double(vector.y())
          << double(vector.z()) << double(vector.w());
   return stream;
}

QDataStream &operator>>(QDataStream &stream, QVector4D &vector)
{
   double x, y, z, w;
   stream >> x;
   stream >> y;
   stream >> z;
   stream >> w;
   vector.setX(qreal(x));
   vector.setY(qreal(y));
   vector.setZ(qreal(z));
   vector.setW(qreal(w));
   return stream;
}

#endif // QT_NO_VECTOR4D

