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

#ifndef QFIXED_P_H
#define QFIXED_P_H

#include <qdebug.h>
#include <qpoint.h>
#include <qsize.h>

struct QFixed {
 public:
   QFixed()
      : m_fixedValue(0)
   { }

   QFixed(int i)
      : m_fixedValue(i << 6)
   { }

   QFixed(long i)
      : m_fixedValue(i << 6)
   { }

   QFixed &operator=(int i) {
      m_fixedValue = (i << 6);
      return *this;
   }

   QFixed &operator=(long i) {
      m_fixedValue = (i << 6);
      return *this;
   }

   static QFixed fromReal(qreal r) {
      return fromFixed((int)(r * qreal(64)));
   }

   static QFixed fromFixed(int fixed) {
      return QFixed(fixed, 0);
   }

   int value() const {
      return m_fixedValue;
   }

   void setValue(int value) {
      m_fixedValue = value;
   }

   int toInt() const {
      return (((m_fixedValue) + 32) & -64) >> 6;
   }

   qreal toReal() const {
      return ((qreal)m_fixedValue) / (qreal)64;
   }

   int truncate() const {
      return m_fixedValue >> 6;
   }

   QFixed round() const {
      return fromFixed(((m_fixedValue) + 32) & -64);
   }

   QFixed floor() const {
      return fromFixed((m_fixedValue) & -64);
   }

   QFixed ceil() const {
      return fromFixed((m_fixedValue + 63) & -64);
   }

   QFixed operator+(int i) const {
      return fromFixed(m_fixedValue + i * 64);
   }

   QFixed operator+(uint i) const {
      return fromFixed((m_fixedValue + (i << 6)));
   }

   QFixed operator+(const QFixed &other) const {
      return fromFixed((m_fixedValue + other.m_fixedValue));
   }

   QFixed &operator+=(int i) {
      m_fixedValue += (i << 6);
      return *this;
   }

   QFixed &operator+=(uint i) {
      m_fixedValue += (i << 6);
      return *this;
   }

   QFixed &operator+=(const QFixed &other) {
      m_fixedValue += other.m_fixedValue;
      return *this;
   }

   QFixed operator-(int i) const {
      return fromFixed(m_fixedValue - i * 64);
   }

   QFixed operator-(uint i) const {
      return fromFixed((m_fixedValue - (i << 6)));
   }

   QFixed operator-(const QFixed &other) const {
      return fromFixed((m_fixedValue - other.m_fixedValue));
   }

   QFixed &operator-=(int i) {
      m_fixedValue -= (i << 6);
      return *this;
   }

   QFixed &operator-=(uint i) {
      m_fixedValue -= (i << 6);
      return *this;
   }

   QFixed &operator-=(const QFixed &other) {
      m_fixedValue -= other.m_fixedValue;
      return *this;
   }

   QFixed operator-() const {
      return fromFixed(-m_fixedValue);
   }

   bool operator==(const QFixed &other) const {
      return m_fixedValue == other.m_fixedValue;
   }

   bool operator!=(const QFixed &other) const {
      return m_fixedValue != other.m_fixedValue;
   }

   bool operator<(const QFixed &other) const {
      return m_fixedValue < other.m_fixedValue;
   }

   bool operator>(const QFixed &other) const {
      return m_fixedValue > other.m_fixedValue;
   }

   bool operator<=(const QFixed &other) const {
      return m_fixedValue <= other.m_fixedValue;
   }

   bool operator>=(const QFixed &other) const {
      return m_fixedValue >= other.m_fixedValue;
   }

   bool operator!() const {
      return ! m_fixedValue;
   }

   QFixed &operator/=(int x) {
      m_fixedValue /= x;
      return *this;
   }

   QFixed &operator/=(const QFixed &other) {
      if (other.m_fixedValue == 0) {
         m_fixedValue = 0x7FFFFFFFL;

      } else {
         bool neg = false;
         qint64 a = m_fixedValue;
         qint64 b = other.m_fixedValue;

         if (a < 0) {
            a = -a;
            neg = true;
         }

         if (b < 0) {
            b = -b;
            neg = !neg;
         }

         int res = (int)(((a << 6) + (b >> 1)) / b);

         m_fixedValue = (neg ? -res : res);
      }

      return *this;
   }

   QFixed operator/(int d) const {
      return fromFixed(m_fixedValue / d);
   }

   QFixed operator/(QFixed b) const {
      QFixed f = *this;
      return (f /= b);
   }

   QFixed operator>>(int d) const {
      QFixed f = *this;
      f.m_fixedValue >>= d;
      return f;
   }

   QFixed &operator*=(int i) {
      m_fixedValue *= i;
      return *this;
   }

   QFixed &operator*=(uint i) {
      m_fixedValue *= i;
      return *this;
   }

   QFixed &operator*=(const QFixed &other) {
      bool neg = false;
      qint64 a = m_fixedValue;
      qint64 b = other.m_fixedValue;

      if (a < 0) {
         a = -a;
         neg = true;
      }

      if (b < 0) {
         b = -b;
         neg = !neg;
      }

      int res = (int)((a * b + 0x20L) >> 6);
      m_fixedValue = neg ? -res : res;

      return *this;
   }

   QFixed operator*(int i) const {
      return fromFixed(m_fixedValue * i);
   }

   QFixed operator*(uint i) const {
      return fromFixed(m_fixedValue * i);
   }

   QFixed operator*(const QFixed &o) const {
      QFixed f = *this;
      return (f *= o);
   }

 private:
   // 2nd int is just a dummy for disambiguation
   constexpr QFixed(int val, int)
      : m_fixedValue(val)
   { }

   constexpr QFixed(qreal i)
      : m_fixedValue((int)(i * qreal(64)))
   { }

   QFixed &operator=(qreal i) {
     m_fixedValue = (int)(i * qreal(64));
      return *this;
   }

   QFixed operator+(qreal i) const {
      return fromFixed((m_fixedValue + (int)(i * qreal(64))));
   }

   QFixed &operator+=(qreal i) {
      m_fixedValue += (int)(i * 64);
      return *this;
   }

   QFixed operator-(qreal i) const {
      return fromFixed((m_fixedValue - (int)(i * qreal(64))));
   }

   QFixed &operator-=(qreal i) {
      m_fixedValue -= (int)(i * 64);
      return *this;
   }

   QFixed &operator/=(qreal r) {
      m_fixedValue = (int)(m_fixedValue / r);
      return *this;
   }

   QFixed operator/(qreal d) const {
      return fromFixed((int)(m_fixedValue / d));
   }

   QFixed &operator*=(qreal d) {
      m_fixedValue = (int) (m_fixedValue * d);
      return *this;
   }

   QFixed operator*(qreal d) const {
      return fromFixed((int) (m_fixedValue * d));
   }

   int m_fixedValue;
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
