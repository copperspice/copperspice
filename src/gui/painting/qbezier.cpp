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

#include <qbezier_p.h>

#include <qdebug.h>
#include <qline.h>
#include <qpolygon.h>
#include <qvector.h>
#include <qlist.h>
#include <qmath.h>

#include <qnumeric_p.h>

//#define QDEBUG_BEZIER

QBezier QBezier::fromPoints(const QPointF &p1, const QPointF &p2,
   const QPointF &p3, const QPointF &p4)
{
   QBezier b;
   b.x1 = p1.x();
   b.y1 = p1.y();
   b.x2 = p2.x();
   b.y2 = p2.y();
   b.x3 = p3.x();
   b.y3 = p3.y();
   b.x4 = p4.x();
   b.y4 = p4.y();
   return b;
}

/*!
  \internal
*/
QPolygonF QBezier::toPolygon(qreal bezier_flattening_threshold) const
{
   // flattening is done by splitting the bezier until we can replace the segment by a straight
   // line. We split further until the control points are close enough to the line connecting the
   // boundary points.
   //
   // the Distance of a point p from a line given by the points (a,b) is given by:
   //
   // d = abs( (bx - ax)(ay - py) - (by - ay)(ax - px) ) / line_length
   //
   // We can stop splitting if both control points are close enough to the line.
   // To make the algorithm faster we use the manhattan length of the line.

   QPolygonF polygon;
   polygon.append(QPointF(x1, y1));
   addToPolygon(&polygon, bezier_flattening_threshold);
   return polygon;
}

QBezier QBezier::mapBy(const QTransform &transform) const
{
   return QBezier::fromPoints(transform.map(pt1()), transform.map(pt2()), transform.map(pt3()), transform.map(pt4()));
}

QBezier QBezier::getSubRange(qreal t0, qreal t1) const
{
   QBezier result;
   QBezier temp;

   // cut at t1
   if (qFuzzyIsNull(t1 - qreal(1.))) {
      result = *this;
   } else {
      temp = *this;
      temp.parameterSplitLeft(t1, &result);
   }

   // cut at t0
   if (! qFuzzyIsNull(t0)) {
      result.parameterSplitLeft(t0 / t1, &temp);
   }

   return result;
}

void QBezier::addToPolygon(QPolygonF *polygon, qreal bezier_flattening_threshold) const
{
   QBezier beziers[10];

   int levels[10];
   beziers[0] = *this;
   levels[0]  = 9;

   QBezier *b = beziers;
   int *lvl   = levels;

   while (b >= beziers) {
      // check if we can pop the top bezier curve from the stack
      qreal y4y1 = b->y4 - b->y1;
      qreal x4x1 = b->x4 - b->x1;
      qreal l = qAbs(x4x1) + qAbs(y4y1);
      qreal d;

      if (l > 1.) {
         d = qAbs( (x4x1) * (b->y1 - b->y2) - (y4y1) * (b->x1 - b->x2) )
            + qAbs( (x4x1) * (b->y1 - b->y3) - (y4y1) * (b->x1 - b->x3) );
      } else {
         d = qAbs(b->x1 - b->x2) + qAbs(b->y1 - b->y2) +
            qAbs(b->x1 - b->x3) + qAbs(b->y1 - b->y3);
         l = 1.;
      }
      if (d < bezier_flattening_threshold * l || *lvl == 0) {
         // good enough, we pop it off and add the endpoint
         polygon->append(QPointF(b->x4, b->y4));
         --b;
         --lvl;
      } else {
         // split, second half of the polygon goes lower into the stack
         b->split(b + 1, b);
         lvl[1] = --lvl[0];
         ++b;
         ++lvl;
      }
   }
}

void QBezier::addToPolygon(QDataBuffer<QPointF> &polygon, qreal bezier_flattening_threshold) const
{
   QBezier beziers[10];
   int levels[10];
   beziers[0] = *this;
   levels[0] = 9;
   QBezier *b = beziers;
   int *lvl = levels;

   while (b >= beziers) {
      // check if we can pop the top bezier curve from the stack
      qreal y4y1 = b->y4 - b->y1;
      qreal x4x1 = b->x4 - b->x1;
      qreal l = qAbs(x4x1) + qAbs(y4y1);
      qreal d;
      if (l > 1.) {
         d = qAbs( (x4x1) * (b->y1 - b->y2) - (y4y1) * (b->x1 - b->x2) )
            + qAbs( (x4x1) * (b->y1 - b->y3) - (y4y1) * (b->x1 - b->x3) );
      } else {
         d = qAbs(b->x1 - b->x2) + qAbs(b->y1 - b->y2) +
            qAbs(b->x1 - b->x3) + qAbs(b->y1 - b->y3);
         l = 1.;
      }
      if (d < bezier_flattening_threshold * l || *lvl == 0) {
         // good enough, we pop it off and add the endpoint
         polygon.add(QPointF(b->x4, b->y4));
         --b;
         --lvl;
      } else {
         // split, second half of the polygon goes lower into the stack
         b->split(b + 1, b);
         lvl[1] = --lvl[0];
         ++b;
         ++lvl;
      }
   }
}
QRectF QBezier::bounds() const
{
   qreal xmin = x1;
   qreal xmax = x1;
   if (x2 < xmin) {
      xmin = x2;
   } else if (x2 > xmax) {
      xmax = x2;
   }
   if (x3 < xmin) {
      xmin = x3;
   } else if (x3 > xmax) {
      xmax = x3;
   }
   if (x4 < xmin) {
      xmin = x4;
   } else if (x4 > xmax) {
      xmax = x4;
   }

   qreal ymin = y1;
   qreal ymax = y1;
   if (y2 < ymin) {
      ymin = y2;
   } else if (y2 > ymax) {
      ymax = y2;
   }
   if (y3 < ymin) {
      ymin = y3;
   } else if (y3 > ymax) {
      ymax = y3;
   }
   if (y4 < ymin) {
      ymin = y4;
   } else if (y4 > ymax) {
      ymax = y4;
   }
   return QRectF(xmin, ymin, xmax - xmin, ymax - ymin);
}

enum ShiftResult {
   Ok,
   Discard,
   Split,
   Circle
};

static ShiftResult good_offset(const QBezier *b1, const QBezier *b2, qreal offset, qreal threshold)
{
   const qreal o2 = offset * offset;
   const qreal max_dist_line = threshold * offset * offset;
   const qreal max_dist_normal = threshold * offset;
   const qreal spacing = qreal(0.25);
   for (qreal i = spacing; i < qreal(0.99); i += spacing) {
      QPointF p1 = b1->pointAt(i);
      QPointF p2 = b2->pointAt(i);
      qreal d = (p1.x() - p2.x()) * (p1.x() - p2.x()) + (p1.y() - p2.y()) * (p1.y() - p2.y());
      if (qAbs(d - o2) > max_dist_line) {
         return Split;
      }

      QPointF normalPoint = b1->normalVector(i);
      qreal l = qAbs(normalPoint.x()) + qAbs(normalPoint.y());
      if (l != qreal(0.0)) {
         d = qAbs( normalPoint.x() * (p1.y() - p2.y()) - normalPoint.y() * (p1.x() - p2.x()) ) / l;
         if (d > max_dist_normal) {
            return Split;
         }
      }
   }
   return Ok;
}

static ShiftResult shift(const QBezier *orig, QBezier *shifted, qreal offset, qreal threshold)
{
   int map[4];
   bool p1_p2_equal = (orig->x1 == orig->x2 && orig->y1 == orig->y2);
   bool p2_p3_equal = (orig->x2 == orig->x3 && orig->y2 == orig->y3);
   bool p3_p4_equal = (orig->x3 == orig->x4 && orig->y3 == orig->y4);

   QPointF points[4];
   int np = 0;
   points[np] = QPointF(orig->x1, orig->y1);
   map[0] = 0;
   ++np;
   if (!p1_p2_equal) {
      points[np] = QPointF(orig->x2, orig->y2);
      ++np;
   }
   map[1] = np - 1;
   if (!p2_p3_equal) {
      points[np] = QPointF(orig->x3, orig->y3);
      ++np;
   }
   map[2] = np - 1;
   if (!p3_p4_equal) {
      points[np] = QPointF(orig->x4, orig->y4);
      ++np;
   }
   map[3] = np - 1;
   if (np == 1) {
      return Discard;
   }

   QRectF b = orig->bounds();
   if (np == 4 && b.width() < .1 * offset && b.height() < .1 * offset) {
      qreal l = (orig->x1 - orig->x2) * (orig->x1 - orig->x2) +
         (orig->y1 - orig->y2) * (orig->y1 - orig->y2) *
         (orig->x3 - orig->x4) * (orig->x3 - orig->x4) +
         (orig->y3 - orig->y4) * (orig->y3 - orig->y4);
      qreal dot = (orig->x1 - orig->x2) * (orig->x3 - orig->x4) +
         (orig->y1 - orig->y2) * (orig->y3 - orig->y4);
      if (dot < 0 && dot * dot < 0.8 * l)
         // the points are close and reverse dirction. Approximate the whole
         // thing by a semi circle
      {
         return Circle;
      }
   }

   QPointF points_shifted[4];

   QLineF prev = QLineF(QPointF(), points[1] - points[0]);
   QPointF prev_normal = prev.normalVector().unitVector().p2();

   points_shifted[0] = points[0] + offset * prev_normal;

   for (int i = 1; i < np - 1; ++i) {
      QLineF next = QLineF(QPointF(), points[i + 1] - points[i]);
      QPointF next_normal = next.normalVector().unitVector().p2();

      QPointF normal_sum = prev_normal + next_normal;

      qreal r = qreal(1.0) + prev_normal.x() * next_normal.x()
         + prev_normal.y() * next_normal.y();

      if (qFuzzyIsNull(r)) {
         points_shifted[i] = points[i] + offset * prev_normal;
      } else {
         qreal k = offset / r;
         points_shifted[i] = points[i] + k * normal_sum;
      }

      prev_normal = next_normal;
   }

   points_shifted[np - 1] = points[np - 1] + offset * prev_normal;

   *shifted = QBezier::fromPoints(points_shifted[map[0]], points_shifted[map[1]],
         points_shifted[map[2]], points_shifted[map[3]]);

   return good_offset(orig, shifted, offset, threshold);
}

// This value is used to determine the length of control point vectors
// when approximating arc segments as curves. The factor is multiplied
// with the radius of the circle.
#define KAPPA qreal(0.5522847498)


static bool addCircle(const QBezier *b, qreal offset, QBezier *o)
{
   QPointF normals[3];

   normals[0] = QPointF(b->y2 - b->y1, b->x1 - b->x2);
   qreal dist = qSqrt(normals[0].x() * normals[0].x() + normals[0].y() * normals[0].y());
   if (qFuzzyIsNull(dist)) {
      return false;
   }

   normals[0] /= dist;
   normals[2] = QPointF(b->y4 - b->y3, b->x3 - b->x4);
   dist = qSqrt(normals[2].x() * normals[2].x() + normals[2].y() * normals[2].y());
   if (qFuzzyIsNull(dist)) {
      return false;
   }

   normals[2] /= dist;

   normals[1] = QPointF(b->x1 - b->x2 - b->x3 + b->x4, b->y1 - b->y2 - b->y3 + b->y4);
   normals[1] /= -1 * qSqrt(normals[1].x() * normals[1].x() + normals[1].y() * normals[1].y());

   qreal angles[2];
   qreal sign = 1.;

   for (int i = 0; i < 2; ++i) {
      qreal cos_a = normals[i].x() * normals[i + 1].x() + normals[i].y() * normals[i + 1].y();
      if (cos_a > 1.) {
         cos_a = 1.;
      }
      if (cos_a < -1.) {
         cos_a = -1;
      }

      angles[i] = qAcos(cos_a) * qreal(1 / M_PI);
   }

   if (angles[0] + angles[1] > 1.) {
      // more than 180 degrees
      normals[1] = -normals[1];
      angles[0] = 1. - angles[0];
      angles[1] = 1. - angles[1];
      sign = -1.;

   }

   QPointF circle[3];
   circle[0] = QPointF(b->x1, b->y1) + normals[0] * offset;
   circle[1] = QPointF(qreal(0.5) * (b->x1 + b->x4), qreal(0.5) * (b->y1 + b->y4)) + normals[1] * offset;
   circle[2] = QPointF(b->x4, b->y4) + normals[2] * offset;

   for (int i = 0; i < 2; ++i) {
      qreal kappa = qreal(2.0) * KAPPA * sign * offset * angles[i];

      o->x1 = circle[i].x();
      o->y1 = circle[i].y();
      o->x2 = circle[i].x() - normals[i].y() * kappa;
      o->y2 = circle[i].y() + normals[i].x() * kappa;
      o->x3 = circle[i + 1].x() + normals[i + 1].y() * kappa;
      o->y3 = circle[i + 1].y() - normals[i + 1].x() * kappa;
      o->x4 = circle[i + 1].x();
      o->y4 = circle[i + 1].y();

      ++o;
   }
   return true;
}

int QBezier::shifted(QBezier *curveSegments, int maxSegments, qreal offset, float threshold) const
{
   Q_ASSERT(curveSegments);
   Q_ASSERT(maxSegments > 0);

   if (qFuzzyCompare(x1, x2) && qFuzzyCompare(x1, x3) && qFuzzyCompare(x1, x4) &&
      qFuzzyCompare(y1, y2) && qFuzzyCompare(y1, y3) && qFuzzyCompare(y1, y4)) {
      return 0;
   }

   --maxSegments;
   QBezier beziers[10];

redo:
   beziers[0] = *this;
   QBezier *b = beziers;
   QBezier *o = curveSegments;

   while (b >= beziers) {
      int stack_segments = b - beziers + 1;
      if ((stack_segments == 10) || (o - curveSegments == maxSegments - stack_segments)) {
         threshold *= qreal(1.5);

         if (threshold > qreal(2.0)) {
            goto give_up;
         }
         goto redo;
      }
      ShiftResult res = shift(b, o, offset, threshold);
      if (res == Discard) {
         --b;
      } else if (res == Ok) {
         ++o;
         --b;
         continue;
      } else if (res == Circle && maxSegments - (o - curveSegments) >= 2) {
         // add semi circle
         if (addCircle(b, offset, o)) {
            o += 2;
         }
         --b;
      } else {
         b->split(b + 1, b);
         ++b;
      }
   }

give_up:
   while (b >= beziers) {
      ShiftResult res = shift(b, o, offset, threshold);

      // if res isn't Ok or Split then *o is undefined
      if (res == Ok || res == Split) {
         ++o;
      }

      --b;
   }

   Q_ASSERT(o - curveSegments <= maxSegments);
   return o - curveSegments;
}

#ifdef QDEBUG_BEZIER
static QDebug operator<<(QDebug dbg, const QBezier &bz)
{
   dbg << '[' << bz.x1 << ", " << bz.y1 << "], "
      << '[' << bz.x2 << ", " << bz.y2 << "], "
      << '[' << bz.x3 << ", " << bz.y3 << "], "
      << '[' << bz.x4 << ", " << bz.y4 << ']';
   return dbg;
}
#endif

qreal QBezier::length(qreal error) const
{
   qreal length = qreal(0.0);

   addIfClose(&length, error);

   return length;
}

void QBezier::addIfClose(qreal *length, qreal error) const
{
   QBezier left, right;     /* bez poly splits */

   qreal len = qreal(0.0);  /* arc length */
   qreal chord;             /* chord length */

   len = len + QLineF(QPointF(x1, y1), QPointF(x2, y2)).length();
   len = len + QLineF(QPointF(x2, y2), QPointF(x3, y3)).length();
   len = len + QLineF(QPointF(x3, y3), QPointF(x4, y4)).length();

   chord = QLineF(QPointF(x1, y1), QPointF(x4, y4)).length();

   if ((len - chord) > error) {
      split(&left, &right);                 /* split in two */
      left.addIfClose(length, error);       /* try left side */
      right.addIfClose(length, error);      /* try right side */
      return;
   }

   *length = *length + len;

   return;
}

qreal QBezier::tForY(qreal t0, qreal t1, qreal y) const
{
   qreal py0 = pointAt(t0).y();
   qreal py1 = pointAt(t1).y();

   if (py0 > py1) {
      qSwap(py0, py1);
      qSwap(t0, t1);
   }

   Q_ASSERT(py0 <= py1);

   if (py0 >= y) {
      return t0;
   } else if (py1 <= y) {
      return t1;
   }

   Q_ASSERT(py0 < y && y < py1);

   qreal lt = t0;
   qreal dt;
   do {
      qreal t = qreal(0.5) * (t0 + t1);

      qreal a, b, c, d;
      QBezier::coefficients(t, a, b, c, d);
      qreal yt = a * y1 + b * y2 + c * y3 + d * y4;

      if (yt < y) {
         t0 = t;
         py0 = yt;
      } else {
         t1 = t;
         py1 = yt;
      }
      dt = lt - t;
      lt = t;
   } while (qAbs(dt) > qreal(1e-7));

   return t0;
}

int QBezier::stationaryYPoints(qreal &t0, qreal &t1) const
{
   // y(t) = (1 - t)^3 * y1 + 3 * (1 - t)^2 * t * y2 + 3 * (1 - t) * t^2 * y3 + t^3 * y4
   // y'(t) = 3 * (-(1-2t+t^2) * y1 + (1 - 4 * t + 3 * t^2) * y2 + (2 * t - 3 * t^2) * y3 + t^2 * y4)
   // y'(t) = 3 * ((-y1 + 3 * y2 - 3 * y3 + y4)t^2 + (2 * y1 - 4 * y2 + 2 * y3)t + (-y1 + y2))

   const qreal a = -y1 + 3 * y2 - 3 * y3 + y4;
   const qreal b = 2 * y1 - 4 * y2 + 2 * y3;
   const qreal c = -y1 + y2;

   if (qFuzzyIsNull(a)) {
      if (qFuzzyIsNull(b)) {
         return 0;
      }

      t0 = -c / b;
      return t0 > 0 && t0 < 1;
   }

   qreal reciprocal = b * b - 4 * a * c;

   if (qFuzzyIsNull(reciprocal)) {
      t0 = -b / (2 * a);
      return t0 > 0 && t0 < 1;
   } else if (reciprocal > 0) {
      qreal temp = qSqrt(reciprocal);

      t0 = (-b - temp) / (2 * a);
      t1 = (-b + temp) / (2 * a);

      if (t1 < t0) {
         qSwap(t0, t1);
      }

      int count = 0;
      qreal t[2] = { 0, 1 };

      if (t0 > 0 && t0 < 1) {
         t[count++] = t0;
      }
      if (t1 > 0 && t1 < 1) {
         t[count++] = t1;
      }

      t0 = t[0];
      t1 = t[1];

      return count;
   }

   return 0;
}

qreal QBezier::tAtLength(qreal l) const
{
   qreal len = length();
   qreal t   = qreal(1.0);
   const qreal error = qreal(0.01);
   if (l > len || qFuzzyCompare(l, len)) {
      return t;
   }

   t *= qreal(0.5);
   //int iters = 0;
   //qDebug()<<"LEN is "<<l<<len;
   qreal lastBigger = qreal(1.0);
   while (1) {
      //qDebug()<<"\tt is "<<t;
      QBezier right = *this;
      QBezier left;
      right.parameterSplitLeft(t, &left);
      qreal lLen = left.length();
      if (qAbs(lLen - l) < error) {
         break;
      }

      if (lLen < l) {
         t += (lastBigger - t) * qreal(0.5);
      } else {
         lastBigger = t;
         t -= t * qreal(0.5);
      }
      //++iters;
   }
   //qDebug()<<"number of iters is "<<iters;
   return t;
}

QBezier QBezier::bezierOnInterval(qreal t0, qreal t1) const
{
   if (t0 == 0 && t1 == 1) {
      return *this;
   }

   QBezier bezier = *this;

   QBezier result;
   bezier.parameterSplitLeft(t0, &result);
   qreal trueT = (t1 - t0) / (1 - t0);
   bezier.parameterSplitLeft(trueT, &result);

   return result;
}

