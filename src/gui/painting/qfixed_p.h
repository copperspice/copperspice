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

#ifndef QFIXED_P_H
#define QFIXED_P_H

#include <QtCore/qdebug.h>
#include <QtCore/qpoint.h>
#include <QtCore/qsize.h>


struct QFixed {
 public:
   QFixed() : val(0) {}
   QFixed(int i) : val(i << 6) {}
   QFixed(long i) : val(i << 6) {}

   QFixed &operator=(int i) {
      val = (i << 6);
      return *this;
   }

   QFixed &operator=(long i) {
      val = (i << 6);
      return *this;
   }

   static QFixed fromReal(qreal r) {
      return fromFixed((int)(r * qreal(64)));
   }

   static QFixed fromFixed(int fixed) {
      return QFixed(fixed, 0);
   }

   int value() const {
      return val;
   }

   void setValue(int value) {
      val = value;
   }

   int toInt() const {
      return (((val) + 32) & -64) >> 6;
   }

   qreal toReal() const {
      return ((qreal)val) / (qreal)64;
   }

   int truncate() const {
      return val >> 6;
   }

   QFixed round() const {
      return fromFixed(((val) + 32) & -64);
   }

   QFixed floor() const {
      return fromFixed((val) & -64);
   }

   QFixed ceil() const {
      return fromFixed((val + 63) & -64);
   }

   QFixed operator+(int i) const {
      return fromFixed(val + i * 64);
   }

   QFixed operator+(uint i) const {
      return fromFixed((val + (i << 6)));
   }

   QFixed operator+(const QFixed &other) const {
      return fromFixed((val + other.val));
   }

   QFixed &operator+=(int i) {
      val += (i << 6);
      return *this;
   }

   QFixed &operator+=(uint i) {
      val += (i << 6);
      return *this;
   }

   QFixed &operator+=(const QFixed &other) {
      val += other.val;
      return *this;
   }

   QFixed operator-(int i) const {
      return fromFixed(val - i * 64);
   }

   QFixed operator-(uint i) const {
      return fromFixed((val - (i << 6)));
   }

   QFixed operator-(const QFixed &other) const {
      return fromFixed((val - other.val));
   }

   QFixed &operator-=(int i) {
      val -= (i << 6);
      return *this;
   }

   QFixed &operator-=(uint i) {
      val -= (i << 6);
      return *this;
   }

   QFixed &operator-=(const QFixed &other) {
      val -= other.val;
      return *this;
   }

   QFixed operator-() const {
      return fromFixed(-val);
   }

   bool operator==(const QFixed &other) const {
      return val == other.val;
   }

   bool operator!=(const QFixed &other) const {
      return val != other.val;
   }

   bool operator<(const QFixed &other) const {
      return val < other.val;
   }

   bool operator>(const QFixed &other) const {
      return val > other.val;
   }

   bool operator<=(const QFixed &other) const {
      return val <= other.val;
   }

   bool operator>=(const QFixed &other) const {
      return val >= other.val;
   }

   bool operator!() const {
      return !val;
   }

   QFixed &operator/=(int x) {
      val /= x;
      return *this;
   }

   QFixed &operator/=(const QFixed &o) {
      if (o.val == 0) {
         val = 0x7FFFFFFFL;
      } else {
         bool neg = false;
         qint64 a = val;
         qint64 b = o.val;

         if (a < 0) {
            a = -a;
            neg = true;
         }

         if (b < 0) {
            b = -b;
            neg = !neg;
         }

         int res = (int)(((a << 6) + (b >> 1)) / b);

         val = (neg ? -res : res);
      }

      return *this;
   }

   QFixed operator/(int d) const {
      return fromFixed(val / d);
   }

   QFixed operator/(QFixed b) const {
      QFixed f = *this;
      return (f /= b);
   }

   QFixed operator>>(int d) const {
      QFixed f = *this;
      f.val >>= d;
      return f;
   }

   QFixed &operator*=(int i) {
      val *= i;
      return *this;
   }

   QFixed &operator*=(uint i) {
      val *= i;
      return *this;
   }

   QFixed &operator*=(const QFixed &o) {
      bool neg = false;
      qint64 a = val;
      qint64 b = o.val;

      if (a < 0) {
         a = -a;
         neg = true;
      }

      if (b < 0) {
         b = -b;
         neg = !neg;
      }

      int res = (int)((a * b + 0x20L) >> 6);
      val = neg ? -res : res;
      return *this;
   }

   QFixed operator*(int i) const {
      return fromFixed(val * i);
   }

   QFixed operator*(uint i) const {
      return fromFixed(val * i);
   }

   QFixed operator*(const QFixed &o) const {
      QFixed f = *this;
      return (f *= o);
   }

 private:
   constexpr QFixed(int val, int) : val(val)
   {} // 2nd int is just a dummy for disambiguation

   constexpr  QFixed(qreal i) : val((int)(i * qreal(64)))
   {}

   QFixed &operator=(qreal i) {
      val = (int)(i * qreal(64));
      return *this;
   }

   QFixed operator+(qreal i) const {
      return fromFixed((val + (int)(i * qreal(64))));
   }

   QFixed &operator+=(qreal i) {
      val += (int)(i * 64);
      return *this;
   }

   QFixed operator-(qreal i) const {
      return fromFixed((val - (int)(i * qreal(64))));
   }

   QFixed &operator-=(qreal i) {
      val -= (int)(i * 64);
      return *this;
   }

   QFixed &operator/=(qreal r) {
      val = (int)(val / r);
      return *this;
   }

   QFixed operator/(qreal d) const {
      return fromFixed((int)(val / d));
   }

   QFixed &operator*=(qreal d) {
      val = (int) (val * d);
      return *this;
   }

   QFixed operator*(qreal d) const {
      return fromFixed((int) (val * d));
   }

   int val;
};

#define QFIXED_MAX (INT_MAX/256)

inline int qRound(const QFixed &f)
{
   return f.toInt();
}

inline int qFloor(const QFixed &f)
{
   return f.floor().truncate();
}

inline QFixed operator*(int i, const QFixed &d)
{
   return d * i;
}

inline QFixed operator+(int i, const QFixed &d)
{
   return d + i;
}

inline QFixed operator-(int i, const QFixed &d)
{
   return -(d - i);
}

inline QFixed operator*(uint i, const QFixed &d)
{
   return d * i;
}

inline QFixed operator+(uint i, const QFixed &d)
{
   return d + i;
}

inline QFixed operator-(uint i, const QFixed &d)
{
   return -(d - i);
}

// inline QFixed operator*(qreal d, const QFixed &d2) { return d2*d; }

inline bool operator==(const QFixed &f, int i)
{
   return f.value() == (i << 6);
}

inline bool operator==(int i, const QFixed &f)
{
   return f.value() == (i << 6);
}

inline bool operator!=(const QFixed &f, int i)
{
   return f.value() != (i << 6);
}

inline bool operator!=(int i, const QFixed &f)
{
   return f.value() != (i << 6);
}

inline bool operator<=(const QFixed &f, int i)
{
   return f.value() <= (i << 6);
}

inline bool operator<=(int i, const QFixed &f)
{
   return (i << 6) <= f.value();
}

inline bool operator>=(const QFixed &f, int i)
{
   return f.value() >= (i << 6);
}

inline bool operator>=(int i, const QFixed &f)
{
   return (i << 6) >= f.value();
}

inline bool operator<(const QFixed &f, int i)
{
   return f.value() < (i << 6);
}

inline bool operator<(int i, const QFixed &f)
{
   return (i << 6) < f.value();
}

inline bool operator>(const QFixed &f, int i)
{
   return f.value() > (i << 6);
}

inline bool operator>(int i, const QFixed &f)
{
   return (i << 6) > f.value();
}

inline QDebug &operator<<(QDebug &dbg, const QFixed &f)
{
   return dbg << f.toReal();
}

struct QFixedPoint {
   QFixed x;
   QFixed y;

   QFixedPoint() = default;

   QFixedPoint(const QFixed &_x, const QFixed &_y)
      : x(_x), y(_y)
   {}

   QPointF toPointF() const {
      return QPointF(x.toReal(), y.toReal());
   }

   static QFixedPoint fromPointF(const QPointF &p) {
      return QFixedPoint(QFixed::fromReal(p.x()), QFixed::fromReal(p.y()));
   }
};

inline QFixedPoint operator-(const QFixedPoint &p1, const QFixedPoint &p2)
{
   return QFixedPoint(p1.x - p2.x, p1.y - p2.y);
}

inline QFixedPoint operator+(const QFixedPoint &p1, const QFixedPoint &p2)
{
   return QFixedPoint(p1.x + p2.x, p1.y + p2.y);
}

struct QFixedSize {
   QFixed width;
   QFixed height;

   QFixedSize()
   { }

   QFixedSize(QFixed _width, QFixed _height) : width(_width), height(_height)
   { }

   QSizeF toSizeF() const {
      return QSizeF(width.toReal(), height.toReal());
   }

   static QFixedSize fromSizeF(const QSizeF &s) {
      return QFixedSize(QFixed::fromReal(s.width()), QFixed::fromReal(s.height()));

   }
};

#endif
