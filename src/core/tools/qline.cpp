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

#include <qline.h>

#include <qdebug.h>
#include <qdatastream.h>
#include <qmath.h>

#include <qnumeric_p.h>

QDebug operator<<(QDebug d, const QLine &p)
{
   d << "QLine(" << p.p1() << ',' << p.p2() << ')';
   return d;
}

QDataStream &operator<<(QDataStream &stream, const QLine &line)
{
   stream << line.p1() << line.p2();
   return stream;
}

QDataStream &operator>>(QDataStream &stream, QLine &line)
{
   QPoint p1;
   QPoint p2;

   stream >> p1;
   stream >> p2;
   line = QLine(p1, p2);

   return stream;
}

static inline qreal q_deg2rad(qreal x)
{
   return x * qreal(0.01745329251994329576923690768489);    /* pi/180 */
}

static inline qreal q_rad2deg(qreal x)
{
   return x * qreal(57.295779513082320876798154814105);    /* 180/pi */
}

bool QLineF::isNull() const
{
   return (qFuzzyCompare(pt1.x(), pt2.x()) && qFuzzyCompare(pt1.y(), pt2.y())) ? true : false;
}

qreal QLineF::length() const
{
   qreal x = pt2.x() - pt1.x();
   qreal y = pt2.y() - pt1.y();
   return qSqrt(x * x + y * y);
}

qreal QLineF::angle() const
{
   const qreal dx = pt2.x() - pt1.x();
   const qreal dy = pt2.y() - pt1.y();

   const qreal theta = q_rad2deg(qAtan2(-dy, dx));

   const qreal theta_normalized = theta < 0 ? theta + 360 : theta;

   if (qFuzzyCompare(theta_normalized, qreal(360))) {
      return qreal(0);
   } else {
      return theta_normalized;
   }
}

void QLineF::setAngle(qreal angle)
{
   const qreal angleR = q_deg2rad(angle);
   const qreal l = length();

   const qreal dx = qCos(angleR) * l;
   const qreal dy = -qSin(angleR) * l;

   pt2.rx() = pt1.x() + dx;
   pt2.ry() = pt1.y() + dy;
}

QLineF QLineF::fromPolar(qreal length, qreal angle)
{
   const qreal angleR = q_deg2rad(angle);
   return QLineF(0, 0, qCos(angleR) * length, -qSin(angleR) * length);
}

QLineF QLineF::unitVector() const
{
   qreal x = pt2.x() - pt1.x();
   qreal y = pt2.y() - pt1.y();

   qreal len = qSqrt(x * x + y * y);
   QLineF f(p1(), QPointF(pt1.x() + x / len, pt1.y() + y / len));

   if (qAbs(f.length() - 1) >= 0.001) {
      qWarning("QLine::unitVector() New line does not have a unit length");
   }

   return f;
}

QLineF::IntersectType QLineF::intersect(const QLineF &l, QPointF *intersectionPoint) const
{
   // ipmlementation is based on Graphics Gems III's "Faster Line Segment Intersection"
   const QPointF a = pt2 - pt1;
   const QPointF b = l.pt1 - l.pt2;
   const QPointF c = pt1 - l.pt1;

   const qreal denominator = a.y() * b.x() - a.x() * b.y();

   if (denominator == 0 || !qt_is_finite(denominator)) {
      return NoIntersection;
   }

   const qreal reciprocal = 1 / denominator;
   const qreal na = (b.y() * c.x() - b.x() * c.y()) * reciprocal;

   if (intersectionPoint) {
      *intersectionPoint = pt1 + a * na;
   }

   if (na < 0 || na > 1) {
      return UnboundedIntersection;
   }

   const qreal nb = (a.x() * c.y() - a.y() * c.x()) * reciprocal;

   if (nb < 0 || nb > 1) {
      return UnboundedIntersection;
   }

   return BoundedIntersection;
}

qreal QLineF::angleTo(const QLineF &l) const
{
   if (isNull() || l.isNull()) {
      return 0;
   }

   const qreal a1 = angle();
   const qreal a2 = l.angle();

   const qreal delta = a2 - a1;
   const qreal delta_normalized = delta < 0 ? delta + 360 : delta;

   if (qFuzzyCompare(delta, qreal(360))) {
      return 0;
   } else {
      return delta_normalized;
   }
}

qreal QLineF::angle(const QLineF &l) const
{
   if (isNull() || l.isNull()) {
      return 0;
   }

   qreal cos_line = (dx() * l.dx() + dy() * l.dy()) / (length() * l.length());
   qreal rad = 0;

   // only accept cos_line in the range [-1,1], if it is outside, use 0 (we return 0 rather than PI for those cases)
   if (cos_line >= qreal(-1.0) && cos_line <= qreal(1.0)) {
      rad = qAcos( cos_line );
   }

   return q_rad2deg(rad);
}

QDebug operator<<(QDebug d, const QLineF &p)
{
   d << "QLineF(" << p.p1() << ',' << p.p2() << ')';
   return d;
}

QDataStream &operator<<(QDataStream &stream, const QLineF &lineF)
{
   stream << lineF.p1() << lineF.p2();
   return stream;
}

QDataStream &operator>>(QDataStream &stream, QLineF &lineF)
{
   QPointF start;
   QPointF end;

   stream >> start;
   stream >> end;
   lineF = QLineF(start, end);

   return stream;
}
