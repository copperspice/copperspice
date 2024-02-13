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

#ifndef QMARGINS_H
#define QMARGINS_H

#include <qdebug.h>
#include <qnamespace.h>

class QMargins
{
 public:
   QMargins();
   QMargins(int left, int top, int right, int bottom);

   bool isNull() const;

   int left() const;
   int top() const;
   int right() const;
   int bottom() const;

   void setLeft(int left);
   void setTop(int top);
   void setRight(int right);
   void setBottom(int bottom);

   QMargins &operator+=(const QMargins &other);
   QMargins &operator-=(const QMargins &other);

   QMargins &operator+=(int delta);
   QMargins &operator-=(int delta);
   QMargins &operator*=(int factor);
   QMargins &operator/=(int factor);
   QMargins &operator*=(qreal factor);
   QMargins &operator/=(qreal factor);

 private:
   int m_left;
   int m_top;
   int m_right;
   int m_bottom;

   friend inline bool operator==(const QMargins &m1, const QMargins &m2);
   friend inline bool operator!=(const QMargins &m1, const QMargins &m2);
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QMargins &margins);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QMargins &margins);

inline QMargins::QMargins()
   :  m_left(0), m_top(0), m_right(0), m_bottom(0)
{ }

inline QMargins::QMargins(int left, int top, int right, int bottom)
   : m_left(left), m_top(top), m_right(right), m_bottom(bottom)
{ }

inline bool QMargins::isNull() const
{
   return m_left == 0 && m_top == 0 && m_right == 0 && m_bottom == 0;
}

inline int QMargins::left() const
{
   return m_left;
}

inline int QMargins::top() const
{
   return m_top;
}

inline int QMargins::right() const
{
   return m_right;
}

inline int QMargins::bottom() const
{
   return m_bottom;
}

inline void QMargins::setLeft(int left)
{
   m_left = left;
}

inline void QMargins::setTop(int top)
{
   m_top = top;
}

inline void QMargins::setRight(int right)
{
   m_right = right;
}

inline void QMargins::setBottom(int bottom)
{
   m_bottom = bottom;
}

inline bool operator==(const QMargins &m1, const QMargins &m2)
{
   return
         m1.m_left == m2.m_left &&
         m1.m_top == m2.m_top &&
         m1.m_right == m2.m_right &&
         m1.m_bottom == m2.m_bottom;
}

inline bool operator!=(const QMargins &m1, const QMargins &m2)
{
   return
         m1.m_left != m2.m_left ||
         m1.m_top != m2.m_top ||
         m1.m_right != m2.m_right ||
         m1.m_bottom != m2.m_bottom;
}

inline QMargins operator+(const QMargins &m1, const QMargins &m2)
{
   return QMargins(m1.left() + m2.left(), m1.top() + m2.top(),
         m1.right() + m2.right(), m1.bottom() + m2.bottom());
}

inline QMargins operator-(const QMargins &m1, const QMargins &m2)
{
   return QMargins(m1.left() - m2.left(), m1.top() - m2.top(),
         m1.right() - m2.right(), m1.bottom() - m2.bottom());
}

inline QMargins operator+(const QMargins &margins, int delta)
{
   return QMargins(margins.left() + delta, margins.top() + delta,
         margins.right() + delta, margins.bottom() + delta);
}

inline QMargins operator+(int delta, const QMargins &margins)
{
   return QMargins(margins.left() + delta, margins.top() + delta,
         margins.right() + delta, margins.bottom() + delta);
}

inline QMargins operator-(const QMargins &margins, int delta)
{
   return QMargins(margins.left() - delta, margins.top() - delta,
         margins.right() - delta, margins.bottom() - delta);
}

inline QMargins operator*(const QMargins &margins, int factor)
{
   return QMargins(margins.left() * factor, margins.top() * factor,
         margins.right() * factor, margins.bottom() * factor);
}

inline QMargins operator*(int factor, const QMargins &margins)
{
   return QMargins(margins.left() * factor, margins.top() * factor,
         margins.right() * factor, margins.bottom() * factor);
}

inline QMargins operator*(const QMargins &margins, qreal factor)
{
   return QMargins(qRound(margins.left() * factor), qRound(margins.top() * factor),
         qRound(margins.right() * factor), qRound(margins.bottom() * factor));
}

inline QMargins operator*(qreal factor, const QMargins &margins)
{
   return QMargins(qRound(margins.left() * factor), qRound(margins.top() * factor),
         qRound(margins.right() * factor), qRound(margins.bottom() * factor));
}

inline QMargins operator/(const QMargins &margins, int factor)
{
   return QMargins(margins.left() / factor, margins.top() / factor,
         margins.right() / factor, margins.bottom() / factor);
}

inline QMargins operator/(const QMargins &margins, qreal factor)
{
   return QMargins(qRound(margins.left() / factor), qRound(margins.top() / factor),
         qRound(margins.right() / factor), qRound(margins.bottom() / factor));
}

inline QMargins &QMargins::operator+=(const QMargins &margins)
{
   return *this = *this + margins;
}

inline QMargins &QMargins::operator-=(const QMargins &margins)
{
   return *this = *this - margins;
}

inline QMargins &QMargins::operator+=(int delta)
{
   m_left   += delta;
   m_top    += delta;
   m_right  += delta;
   m_bottom += delta;

   return *this;
}

inline QMargins &QMargins::operator-=(int delta)
{
   m_left   -= delta;
   m_top    -= delta;
   m_right  -= delta;
   m_bottom -= delta;

   return *this;
}

inline QMargins &QMargins::operator*=(int delta)
{
   return *this = *this * delta;
}

inline QMargins &QMargins::operator/=(int delta)
{
   return *this = *this / delta;
}

inline QMargins &QMargins::operator*=(qreal delta)
{
   return *this = *this * delta;
}

inline QMargins &QMargins::operator/=(qreal delta)
{
   return *this = *this / delta;
}

inline QMargins operator+(const QMargins &margins)
{
   return margins;
}

inline QMargins operator-(const QMargins &margins)
{
   return QMargins(-margins.left(), -margins.top(), -margins.right(), -margins.bottom());
}

Q_CORE_EXPORT QDebug operator<<(QDebug, const QMargins &);

class QMarginsF
{
 public:
   QMarginsF();
   QMarginsF(qreal left, qreal top, qreal right, qreal bottom);
   QMarginsF(const QMargins &margins);

   bool isNull() const;

   qreal left() const;
   qreal top() const;
   qreal right() const;
   qreal bottom() const;

   void setLeft(qreal left);
   void setTop(qreal top);
   void setRight(qreal right);
   void setBottom(qreal bottom);

   QMarginsF &operator+=(const QMarginsF &other);
   QMarginsF &operator-=(const QMarginsF &other);

   QMarginsF &operator+=(qreal delta);
   QMarginsF &operator-=(qreal delta);
   QMarginsF &operator*=(qreal factor);
   QMarginsF &operator/=(qreal factor);

   inline QMargins toMargins() const;

 private:
   qreal m_left;
   qreal m_top;
   qreal m_right;
   qreal m_bottom;
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QMarginsF &margins);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QMarginsF &margins);

inline QMarginsF::QMarginsF()
   : m_left(0), m_top(0), m_right(0), m_bottom(0)
{
}

inline QMarginsF::QMarginsF(qreal left, qreal top, qreal right, qreal bottom)
   : m_left(left), m_top(top), m_right(right), m_bottom(bottom)
{
}

inline QMarginsF::QMarginsF(const QMargins &margins)
   : m_left(margins.left()), m_top(margins.top()), m_right(margins.right()), m_bottom(margins.bottom())
{
}

inline bool QMarginsF::isNull() const
{
   return qFuzzyIsNull(m_left) && qFuzzyIsNull(m_top) && qFuzzyIsNull(m_right) && qFuzzyIsNull(m_bottom);
}

inline qreal QMarginsF::left() const
{
   return m_left;
}

inline qreal QMarginsF::top() const
{
   return m_top;
}

inline qreal QMarginsF::right() const
{
   return m_right;
}

inline qreal QMarginsF::bottom() const
{
   return m_bottom;
}

inline void QMarginsF::setLeft(qreal left)
{
   m_left = left;
}

inline void QMarginsF::setTop(qreal top)
{
   m_top = top;
}

inline void QMarginsF::setRight(qreal right)
{
   m_right = right;
}

inline void QMarginsF::setBottom(qreal bottom)
{
   m_bottom = bottom;
}

inline bool operator==(const QMarginsF &lhs, const QMarginsF &rhs)
{
   return qFuzzyCompare(lhs.left(), rhs.left())
         && qFuzzyCompare(lhs.top(), rhs.top())
         && qFuzzyCompare(lhs.right(), rhs.right())
         && qFuzzyCompare(lhs.bottom(), rhs.bottom());
}

inline bool operator!=(const QMarginsF &lhs, const QMarginsF &rhs)
{
   return !operator==(lhs, rhs);
}

inline QMarginsF operator+(const QMarginsF &lhs, const QMarginsF &rhs)
{
   return QMarginsF(lhs.left() + rhs.left(), lhs.top() + rhs.top(),
         lhs.right() + rhs.right(), lhs.bottom() + rhs.bottom());
}

inline QMarginsF operator-(const QMarginsF &lhs, const QMarginsF &rhs)
{
   return QMarginsF(lhs.left() - rhs.left(), lhs.top() - rhs.top(),
         lhs.right() - rhs.right(), lhs.bottom() - rhs.bottom());
}

inline QMarginsF operator+(const QMarginsF &lhs, qreal rhs)
{
   return QMarginsF(lhs.left() + rhs, lhs.top() + rhs,
         lhs.right() + rhs, lhs.bottom() + rhs);
}

inline QMarginsF operator+(qreal lhs, const QMarginsF &rhs)
{
   return QMarginsF(rhs.left() + lhs, rhs.top() + lhs,
         rhs.right() + lhs, rhs.bottom() + lhs);
}

inline QMarginsF operator-(const QMarginsF &lhs, qreal rhs)
{
   return QMarginsF(lhs.left() - rhs, lhs.top() - rhs,
         lhs.right() - rhs, lhs.bottom() - rhs);
}

inline QMarginsF operator*(const QMarginsF &lhs, qreal rhs)
{
   return QMarginsF(lhs.left() * rhs, lhs.top() * rhs,
         lhs.right() * rhs, lhs.bottom() * rhs);
}

inline QMarginsF operator*(qreal lhs, const QMarginsF &rhs)
{
   return QMarginsF(rhs.left() * lhs, rhs.top() * lhs,
         rhs.right() * lhs, rhs.bottom() * lhs);
}

inline QMarginsF operator/(const QMarginsF &lhs, qreal delta)
{
   return QMarginsF(lhs.left() / delta, lhs.top() / delta,
         lhs.right() / delta, lhs.bottom() / delta);
}

inline QMarginsF &QMarginsF::operator+=(const QMarginsF &margins)
{
   return *this = *this + margins;
}

inline QMarginsF &QMarginsF::operator-=(const QMarginsF &margins)
{
   return *this = *this - margins;
}

inline QMarginsF &QMarginsF::operator+=(qreal delta)
{
   m_left   += delta;
   m_top    += delta;
   m_right  += delta;
   m_bottom += delta;

   return *this;
}

inline QMarginsF &QMarginsF::operator-=(qreal delta)
{
   m_left   -= delta;
   m_top    -= delta;
   m_right  -= delta;
   m_bottom -= delta;

   return *this;
}

inline QMarginsF &QMarginsF::operator*=(qreal delta)
{
   return *this = *this * delta;
}

inline QMarginsF &QMarginsF::operator/=(qreal delta)
{
   return *this = *this / delta;
}

inline QMarginsF operator+(const QMarginsF &margins)
{
   return margins;
}

inline QMarginsF operator-(const QMarginsF &margins)
{
   return QMarginsF(-margins.left(), -margins.top(), -margins.right(), -margins.bottom());
}

inline QMargins QMarginsF::toMargins() const
{
   return QMargins(qRound(m_left), qRound(m_top), qRound(m_right), qRound(m_bottom));
}

Q_CORE_EXPORT QDebug operator<<(QDebug, const QMarginsF &);

CS_DECLARE_METATYPE(QMarginsF)

#endif
