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

#ifndef QMATH_H
#define QMATH_H

#include <qglobal.h>

#include <math.h>

#define QT_SINE_TABLE_SIZE 256

extern Q_CORE_EXPORT const qreal qt_sine_table[QT_SINE_TABLE_SIZE];

inline int qCeil(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS

   if (sizeof(qreal) == sizeof(float)) {
      return int(ceilf(float(v)));
   } else
#endif
      return int(ceil(v));
}

inline int qFloor(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS

   if (sizeof(qreal) == sizeof(float)) {
      return int(floorf(float(v)));
   } else
#endif
      return int(floor(v));
}

inline qreal qFabs(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS

   if (sizeof(qreal) == sizeof(float)) {
      return fabsf(float(v));
   } else
#endif
      return fabs(v);
}

inline qreal qSin(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS

   if (sizeof(qreal) == sizeof(float)) {
      return sinf(float(v));
   } else
#endif
      return sin(v);
}

inline qreal qCos(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS

   if (sizeof(qreal) == sizeof(float)) {
      return cosf(float(v));
   } else
#endif
      return cos(v);
}

inline qreal qTan(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS

   if (sizeof(qreal) == sizeof(float)) {
      return tanf(float(v));
   } else
#endif
      return tan(v);
}

inline qreal qAcos(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS

   if (sizeof(qreal) == sizeof(float)) {
      return acosf(float(v));
   } else
#endif
      return acos(v);
}

inline qreal qAsin(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS

   if (sizeof(qreal) == sizeof(float)) {
      return asinf(float(v));
   } else
#endif
      return asin(v);
}

inline qreal qAtan(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS

   if (sizeof(qreal) == sizeof(float)) {
      return atanf(float(v));
   } else
#endif
      return atan(v);
}

inline qreal qAtan2(qreal y, qreal x)
{
#ifdef QT_USE_MATH_H_FLOATS

   if (sizeof(qreal) == sizeof(float)) {
      return atan2f(float(y), float(x));
   } else
#endif
      return atan2(y, x);
}

inline qreal qSqrt(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS

   if (sizeof(qreal) == sizeof(float)) {
      return sqrtf(float(v));
   } else
#endif
      return sqrt(v);
}

inline qreal qLn(qreal v)
{
#ifdef QT_USE_MATH_H_FLOATS

   if (sizeof(qreal) == sizeof(float)) {
      return logf(float(v));
   } else
#endif
      return log(v);
}

inline qreal qExp(qreal v)
{
   // only one signature
   // exists, exp(double)
   return exp(v);
}

inline qreal qPow(qreal x, qreal y)
{
#ifdef QT_USE_MATH_H_FLOATS

   if (sizeof(qreal) == sizeof(float)) {
      return powf(float(x), float(y));
   } else
#endif
      return pow(x, y);
}

#ifndef M_PI
#define M_PI (3.14159265358979323846L)
#endif

inline qreal qFastSin(qreal x)
{
   int si = int(x * (0.5 * QT_SINE_TABLE_SIZE / M_PI)); // Would be more accurate with qRound, but slower.
   qreal d = x - si * (2.0 * M_PI / QT_SINE_TABLE_SIZE);
   int ci = si + QT_SINE_TABLE_SIZE / 4;
   si &= QT_SINE_TABLE_SIZE - 1;
   ci &= QT_SINE_TABLE_SIZE - 1;

   return qt_sine_table[si] + (qt_sine_table[ci] - 0.5 * qt_sine_table[si] * d) * d;
}

inline qreal qFastCos(qreal x)
{
   int ci = int(x * (0.5 * QT_SINE_TABLE_SIZE / M_PI)); // Would be more accurate with qRound, but slower.
   qreal d = x - ci * (2.0 * M_PI / QT_SINE_TABLE_SIZE);
   int si = ci + QT_SINE_TABLE_SIZE / 4;
   si &= QT_SINE_TABLE_SIZE - 1;
   ci &= QT_SINE_TABLE_SIZE - 1;

   return qt_sine_table[si] - (qt_sine_table[ci] + 0.5 * qt_sine_table[si] * d) * d;
}

constexpr inline float qDegreesToRadians(float degrees)
{
   return degrees * float(M_PI / 180);
}

constexpr  inline double qDegreesToRadians(double degrees)
{
   return degrees * (M_PI / 180);
}

constexpr  inline float qRadiansToDegrees(float radians)
{
   return radians * float(180 / M_PI);
}

constexpr  inline double qRadiansToDegrees(double radians)
{
   return radians * (180 / M_PI);
}

#if defined(Q_CC_GNU)

inline quint32 qNextPowerOfTwo(quint32 v)
{
   if (v == 0) {
      return 1;
   }

   return 2U << (31 ^ __builtin_clz(v));
}

inline quint64 qNextPowerOfTwo(quint64 v)
{
   if (v == 0) {
      return 1;
   }

   return Q_UINT64_C(2) << (63 ^ __builtin_clzll(v));
}

#else

inline quint32 qNextPowerOfTwo(quint32 v)
{
   v |= v >> 1;
   v |= v >> 2;
   v |= v >> 4;
   v |= v >> 8;
   v |= v >> 16;
   ++v;

   return v;
}

inline quint64 qNextPowerOfTwo(quint64 v)
{
   v |= v >> 1;
   v |= v >> 2;
   v |= v >> 4;
   v |= v >> 8;
   v |= v >> 16;
   v |= v >> 32;
   ++v;

   return v;
}
#endif

inline quint32 qNextPowerOfTwo(qint32 v)
{
   return qNextPowerOfTwo(quint32(v));
}

inline quint64 qNextPowerOfTwo(qint64 v)
{
   return qNextPowerOfTwo(quint64(v));
}

#endif
