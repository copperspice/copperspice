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

#include <qvector3d.h>

#include <qdebug.h>
#include <qmath.h>
#include <qvariant.h>
#include <qvector2d.h>
#include <qvector4d.h>

#ifndef QT_NO_VECTOR3D

#ifndef QT_NO_VECTOR2D

QVector3D::QVector3D(const QVector2D &vector)
{
   xp = vector.xp;
   yp = vector.yp;
   zp = 0.0f;
}

QVector3D::QVector3D(const QVector2D &vector, qreal zpos)
{
   xp = vector.xp;
   yp = vector.yp;
   zp = zpos;
}

#endif

#ifndef QT_NO_VECTOR4D

QVector3D::QVector3D(const QVector4D &vector)
{
   xp = vector.xp;
   yp = vector.yp;
   zp = vector.zp;
}

#endif

QVector3D QVector3D::normalized() const
{
   // Need some extra precision if the length is very small.
   double len = double(xp) * double(xp) +
                double(yp) * double(yp) +
                double(zp) * double(zp);
   if (qFuzzyIsNull(len - 1.0f)) {
      return *this;
   } else if (!qFuzzyIsNull(len)) {
      return *this / qSqrt(len);
   } else {
      return QVector3D();
   }
}

void QVector3D::normalize()
{
   // Need some extra precision if the length is very small.
   double len = double(xp) * double(xp) +
                double(yp) * double(yp) +
                double(zp) * double(zp);
   if (qFuzzyIsNull(len - 1.0f) || qFuzzyIsNull(len)) {
      return;
   }

   len = qSqrt(len);

   xp /= len;
   yp /= len;
   zp /= len;
}

qreal QVector3D::dotProduct(const QVector3D &v1, const QVector3D &v2)
{
   return v1.xp * v2.xp + v1.yp * v2.yp + v1.zp * v2.zp;
}

QVector3D QVector3D::crossProduct(const QVector3D &v1, const QVector3D &v2)
{
   return QVector3D(v1.yp * v2.zp - v1.zp * v2.yp,
                    v1.zp * v2.xp - v1.xp * v2.zp,
                    v1.xp * v2.yp - v1.yp * v2.xp, 1);
}

QVector3D QVector3D::normal(const QVector3D &v1, const QVector3D &v2)
{
   return crossProduct(v1, v2).normalized();
}

QVector3D QVector3D::normal
(const QVector3D &v1, const QVector3D &v2, const QVector3D &v3)
{
   return crossProduct((v2 - v1), (v3 - v1)).normalized();
}

qreal QVector3D::distanceToPlane(const QVector3D &plane, const QVector3D &normal) const
{
   return dotProduct(*this - plane, normal);
}

qreal QVector3D::distanceToPlane(const QVector3D &plane1, const QVector3D &plane2, const QVector3D &plane3) const
{
   QVector3D n = normal(plane2 - plane1, plane3 - plane1);
   return dotProduct(*this - plane1, n);
}

qreal QVector3D::distanceToLine(const QVector3D &point, const QVector3D &direction) const
{
   if (direction.isNull()) {
      return (*this - point).length();
   }

   QVector3D p = point + dotProduct(*this - point, direction) * direction;

   return (*this - p).length();
}

#ifndef QT_NO_VECTOR2D

QVector2D QVector3D::toVector2D() const
{
   return QVector2D(xp, yp, 1);
}

#endif

#ifndef QT_NO_VECTOR4D

QVector4D QVector3D::toVector4D() const
{
   return QVector4D(xp, yp, zp, 0.0f, 1);
}

#endif

QVector3D::operator QVariant() const
{
   return QVariant(QVariant::Vector3D, this);
}

qreal QVector3D::length() const
{
   return qSqrt(xp * xp + yp * yp + zp * zp);
}

qreal QVector3D::lengthSquared() const
{
   return xp * xp + yp * yp + zp * zp;
}

QDebug operator<<(QDebug dbg, const QVector3D &vector)
{
   dbg.nospace() << "QVector3D("
                 << vector.x() << ", " << vector.y() << ", " << vector.z() << ')';
   return dbg.space();
}

QDataStream &operator<<(QDataStream &stream, const QVector3D &vector)
{
   stream << double(vector.x()) << double(vector.y())
          << double(vector.z());
   return stream;
}

QDataStream &operator>>(QDataStream &stream, QVector3D &vector)
{
   double x, y, z;
   stream >> x;
   stream >> y;
   stream >> z;
   vector.setX(qreal(x));
   vector.setY(qreal(y));
   vector.setZ(qreal(z));
   return stream;
}

#endif // QT_NO_VECTOR3D

