/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#ifndef QSIZE_H
#define QSIZE_H

#include <qassert.h>
#include <qnamespace.h>

class QDataStream;
class QDebug;

class Q_CORE_EXPORT QSize
{
 public:
   constexpr QSize();
   constexpr QSize(int w, int h);

   constexpr inline bool isNull() const;
   constexpr inline bool isEmpty() const;
   constexpr inline bool isValid() const;

   constexpr inline int width() const;
   constexpr inline int height() const;

   inline void setWidth(int w);
   inline void setHeight(int h);

   void transpose();
   inline QSize transposed() const;

   inline void scale(int w, int h, Qt::AspectRatioMode mode);
   inline void scale(const QSize &s, Qt::AspectRatioMode mode);

   QSize scaled(int w, int h, Qt::AspectRatioMode mode) const;
   QSize scaled(const QSize &s, Qt::AspectRatioMode mode) const;

   inline QSize expandedTo(const QSize &other) const;
   inline QSize boundedTo(const QSize &other) const;

   inline int &rwidth();
   inline int &rheight();

   inline QSize &operator+=(const QSize &size);
   inline QSize &operator-=(const QSize &size);
   inline QSize &operator*=(qreal c);
   inline QSize &operator/=(qreal c);

 private:
   int wd;
   int ht;

   friend inline bool operator==(const QSize &, const QSize &);
   friend inline bool operator!=(const QSize &, const QSize &);
   friend inline const QSize operator+(const QSize &, const QSize &);
   friend inline const QSize operator-(const QSize &, const QSize &);
   friend inline const QSize operator*(const QSize &, qreal);
   friend inline const QSize operator*(qreal, const QSize &);
   friend inline const QSize operator/(const QSize &, qreal);
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QSize &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QSize &);

constexpr inline QSize::QSize()
   : wd(-1), ht(-1)
{
}

constexpr inline QSize::QSize(int w, int h)
   : wd(w), ht(h)
{
}

constexpr inline bool QSize::isNull() const
{
   return wd == 0 && ht == 0;
}

constexpr inline bool QSize::isEmpty() const
{
   return wd < 1 || ht < 1;
}

constexpr inline bool QSize::isValid() const
{
   return wd >= 0 && ht >= 0;
}

constexpr inline int QSize::width() const
{
   return wd;
}

constexpr inline int QSize::height() const
{
   return ht;
}

inline void QSize::setWidth(int w)
{
   wd = w;
}

inline void QSize::setHeight(int h)
{
   ht = h;
}

inline QSize QSize::transposed() const
{
   return QSize(ht, wd);
}

inline void QSize::scale(int w, int h, Qt::AspectRatioMode mode)
{
   scale(QSize(w, h), mode);
}

inline void QSize::scale(const QSize &s, Qt::AspectRatioMode mode)
{
   *this = scaled(s, mode);
}

inline QSize QSize::scaled(int w, int h, Qt::AspectRatioMode mode) const
{
   return scaled(QSize(w, h), mode);
}


inline int &QSize::rwidth()
{
   return wd;
}

inline int &QSize::rheight()
{
   return ht;
}

inline QSize &QSize::operator+=(const QSize &size)
{
   wd += size.wd;
   ht += size.ht;

   return *this;
}

inline QSize &QSize::operator-=(const QSize &size)
{
   wd -= size.wd;
   ht -= size.ht;

   return *this;
}

inline QSize &QSize::operator*=(qreal c)
{
   wd = qRound(wd * c);
   ht = qRound(ht * c);
   return *this;
}

inline bool operator==(const QSize &s1, const QSize &s2)
{
   return s1.wd == s2.wd && s1.ht == s2.ht;
}

inline bool operator!=(const QSize &s1, const QSize &s2)
{
   return s1.wd != s2.wd || s1.ht != s2.ht;
}

inline const QSize operator+(const QSize &s1, const QSize &s2)
{
   return QSize(s1.wd + s2.wd, s1.ht + s2.ht);
}

inline const QSize operator-(const QSize &s1, const QSize &s2)
{
   return QSize(s1.wd - s2.wd, s1.ht - s2.ht);
}

inline const QSize operator*(const QSize &s, qreal c)
{
   return QSize(qRound(s.wd * c), qRound(s.ht * c));
}

inline const QSize operator*(qreal c, const QSize &s)
{
   return QSize(qRound(s.wd * c), qRound(s.ht * c));
}

inline QSize &QSize::operator/=(qreal c)
{
   Q_ASSERT(!qFuzzyIsNull(c));
   wd = qRound(wd / c);
   ht = qRound(ht / c);
   return *this;
}

inline const QSize operator/(const QSize &s, qreal c)
{
   Q_ASSERT(!qFuzzyIsNull(c));
   return QSize(qRound(s.wd / c), qRound(s.ht / c));
}

inline QSize QSize::expandedTo(const QSize &other) const
{
   return QSize(qMax(wd, other.wd), qMax(ht, other.ht));
}

inline QSize QSize::boundedTo(const QSize &other) const
{
   return QSize(qMin(wd, other.wd), qMin(ht, other.ht));
}

Q_CORE_EXPORT QDebug operator<<(QDebug, const QSize &);

class Q_CORE_EXPORT QSizeF
{
 public:
   QSizeF();
   QSizeF(const QSize &size);
   QSizeF(qreal w, qreal h);

   inline bool isNull() const;
   inline bool isEmpty() const;
   inline bool isValid() const;

   inline qreal width() const;
   inline qreal height() const;
   inline void setWidth(qreal w);
   inline void setHeight(qreal h);

   void transpose();
   inline QSizeF transposed() const;

   inline void scale(qreal w, qreal h, Qt::AspectRatioMode mode);
   inline void scale(const QSizeF &s, Qt::AspectRatioMode mode);

   QSizeF scaled(qreal w, qreal h, Qt::AspectRatioMode mode) const;
   QSizeF scaled(const QSizeF &sizeF, Qt::AspectRatioMode mode) const;

   inline QSizeF expandedTo(const QSizeF &other) const;
   inline QSizeF boundedTo(const QSizeF &other) const;

   inline qreal &rwidth();
   inline qreal &rheight();

   inline QSizeF &operator+=(const QSizeF &sizeF);
   inline QSizeF &operator-=(const QSizeF &sizeF);
   inline QSizeF &operator*=(qreal c);
   inline QSizeF &operator/=(qreal c);

   inline QSize toSize() const;

 private:
   qreal wd;
   qreal ht;

   friend inline bool operator==(const QSizeF &, const QSizeF &);
   friend inline bool operator!=(const QSizeF &, const QSizeF &);
   friend inline const QSizeF operator+(const QSizeF &, const QSizeF &);
   friend inline const QSizeF operator-(const QSizeF &, const QSizeF &);
   friend inline const QSizeF operator*(const QSizeF &, qreal);
   friend inline const QSizeF operator*(qreal, const QSizeF &);
   friend inline const QSizeF operator/(const QSizeF &, qreal);
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QSizeF &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QSizeF &);

inline QSizeF::QSizeF()
{
   wd = ht = -1.;
}

inline QSizeF::QSizeF(const QSize &size)
   : wd(size.width()), ht(size.height())
{
}

inline QSizeF::QSizeF(qreal w, qreal h)
{
   wd = w;
   ht = h;
}

inline bool QSizeF::isNull() const
{
   return qIsNull(wd) && qIsNull(ht);
}

inline bool QSizeF::isEmpty() const
{
   return wd <= 0. || ht <= 0.;
}

inline bool QSizeF::isValid() const
{
   return wd >= 0. && ht >= 0.;
}

inline qreal QSizeF::width() const
{
   return wd;
}

inline qreal QSizeF::height() const
{
   return ht;
}

inline void QSizeF::setWidth(qreal w)
{
   wd = w;
}

inline void QSizeF::setHeight(qreal h)
{
   ht = h;
}

inline QSizeF QSizeF::transposed() const
{
   return QSizeF(ht, wd);
}

inline void QSizeF::scale(qreal w, qreal h, Qt::AspectRatioMode mode)
{
   scale(QSizeF(w, h), mode);
}

inline void QSizeF::scale(const QSizeF &s, Qt::AspectRatioMode mode)
{
   *this = scaled(s, mode);
}

inline QSizeF QSizeF::scaled(qreal w, qreal h, Qt::AspectRatioMode mode) const
{
   return scaled(QSizeF(w, h), mode);
}


inline qreal &QSizeF::rwidth()
{
   return wd;
}

inline qreal &QSizeF::rheight()
{
   return ht;
}

inline QSizeF &QSizeF::operator+=(const QSizeF &sizeF)
{
   wd += sizeF.wd;
   ht += sizeF.ht;
   return *this;
}

inline QSizeF &QSizeF::operator-=(const QSizeF &sizeF)
{
   wd -= sizeF.wd;
   ht -= sizeF.ht;
   return *this;
}

inline QSizeF &QSizeF::operator*=(qreal c)
{
   wd *= c;
   ht *= c;
   return *this;
}

inline bool operator==(const QSizeF &s1, const QSizeF &s2)
{
   return qFuzzyCompare(s1.wd, s2.wd) && qFuzzyCompare(s1.ht, s2.ht);
}

inline bool operator!=(const QSizeF &s1, const QSizeF &s2)
{
   return !qFuzzyCompare(s1.wd, s2.wd) || !qFuzzyCompare(s1.ht, s2.ht);
}

inline const QSizeF operator+(const QSizeF &s1, const QSizeF &s2)
{
   return QSizeF(s1.wd + s2.wd, s1.ht + s2.ht);
}

inline const QSizeF operator-(const QSizeF &s1, const QSizeF &s2)
{
   return QSizeF(s1.wd - s2.wd, s1.ht - s2.ht);
}

inline const QSizeF operator*(const QSizeF &sizeF, qreal c)
{
   return QSizeF(sizeF.wd * c, sizeF.ht * c);
}

inline const QSizeF operator*(qreal c, const QSizeF &sizeF)
{
   return QSizeF(sizeF.wd * c, sizeF.ht * c);
}

inline QSizeF &QSizeF::operator/=(qreal c)
{
   Q_ASSERT(!qFuzzyIsNull(c));
   wd = wd / c;
   ht = ht / c;
   return *this;
}

inline const QSizeF operator/(const QSizeF &s, qreal c)
{
   Q_ASSERT(!qFuzzyIsNull(c));
   return QSizeF(s.wd / c, s.ht / c);
}

inline QSizeF QSizeF::expandedTo(const QSizeF &other) const
{
   return QSizeF(qMax(wd, other.wd), qMax(ht, other.ht));
}

inline QSizeF QSizeF::boundedTo(const QSizeF &other) const
{
   return QSizeF(qMin(wd, other.wd), qMin(ht, other.ht));
}

inline QSize QSizeF::toSize() const
{
   return QSize(qRound(wd), qRound(ht));
}

Q_CORE_EXPORT QDebug operator<<(QDebug, const QSizeF &);


#endif // QSIZE_H
