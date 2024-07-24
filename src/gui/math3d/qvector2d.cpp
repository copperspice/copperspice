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

#include <qvector2d.h>

#include <qdebug.h>
#include <qmath.h>
#include <qvariant.h>
#include <qvector3d.h>
#include <qvector4d.h>

#ifndef QT_NO_VECTOR2D

#ifndef QT_NO_VECTOR3D

QVector2D::QVector2D(const QVector3D &vector)
{
   xp = vector.xp;
   yp = vector.yp;
}

#endif

#ifndef QT_NO_VECTOR4D

QVector2D::QVector2D(const QVector4D &vector)
{
   xp = vector.xp;
   yp = vector.yp;
}

#endif

qreal QVector2D::length() const
{
   return qSqrt(xp * xp + yp * yp);
}

qreal QVector2D::lengthSquared() const
{
   return xp * xp + yp * yp;
}

QVector2D QVector2D::normalized() const
{
   // Need some extra precision if the length is very small.
   double len = double(xp) * double(xp) +
                double(yp) * double(yp);
   if (qFuzzyIsNull(len - 1.0f)) {
      return *this;
   } else if (!qFuzzyIsNull(len)) {
      return *this / qSqrt(len);
   } else {
      return QVector2D();
   }
}

void QVector2D::normalize()
{
   // Need some extra precision if the length is very small.
   double len = double(xp) * double(xp) +
                double(yp) * double(yp);
   if (qFuzzyIsNull(len - 1.0f) || qFuzzyIsNull(len)) {
      return;
   }

   len = qSqrt(len);

   xp /= len;
   yp /= len;
}

qreal QVector2D::dotProduct(const QVector2D &v1, const QVector2D &v2)
{
   return v1.xp * v2.xp + v1.yp * v2.yp;
}

#ifndef QT_NO_VECTOR3D

QVector3D QVector2D::toVector3D() const
{
   return QVector3D(xp, yp, 0.0f, 1);
}

#endif

#ifndef QT_NO_VECTOR4D

QVector4D QVector2D::toVector4D() const
{
   return QVector4D(xp, yp, 0.0f, 0.0f, 1);
}

#endif

QVector2D::operator QVariant() const
{
   return QVariant(QVariant::Vector2D, this);
}

QDebug operator<<(QDebug dbg, const QVector2D &vector)
{
   dbg.nospace() << "QVector2D(" << vector.x() << ", " << vector.y() << ')';
   return dbg.space();
}

QDataStream &operator<<(QDataStream &stream, const QVector2D &vector)
{
   stream << double(vector.x()) << double(vector.y());
   return stream;
}

QDataStream &operator>>(QDataStream &stream, QVector2D &vector)
{
   double x, y;
   stream >> x;
   stream >> y;
   vector.setX(qreal(x));
   vector.setY(qreal(y));
   return stream;
}

#endif // QT_NO_VECTOR2D

