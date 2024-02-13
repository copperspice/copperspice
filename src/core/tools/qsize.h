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
   constexpr QSize(int width, int height);

   constexpr inline bool isNull() const;
   constexpr inline bool isEmpty() const;
   constexpr inline bool isValid() const;

   constexpr inline int width() const;
   constexpr inline int height() const;

   inline void setWidth(int width);
   inline void setHeight(int height);

   void transpose();
   inline QSize transposed() const;

   inline void scale(int width, int height, Qt::AspectRatioMode mode);
   inline void scale(const QSize &size, Qt::AspectRatioMode mode);

   QSize scaled(int width, int height, Qt::AspectRatioMode mode) const;
   QSize scaled(const QSize &size, Qt::AspectRatioMode mode) const;

   inline QSize expandedTo(const QSize &other) const;
   inline QSize boundedTo(const QSize &other) const;

   inline int &rwidth();
   inline int &rheight();

   inline QSize &operator+=(const QSize &size);
   inline QSize &operator-=(const QSize &size);
   inline QSize &operator*=(qreal factor);
   inline QSize &operator/=(qreal factor);

 private:
   int wd;
   int ht;

   friend inline bool operator==(const QSize &size1, const QSize &size2);
   friend inline bool operator!=(const QSize &size1, const QSize &size2);
   friend inline const QSize operator+(const QSize &size1, const QSize &size2);
   friend inline const QSize operator-(const QSize &size1, const QSize &size2);
   friend inline const QSize operator*(const QSize &size, qreal factor);
   friend inline const QSize operator*(qreal factor, const QSize &size);
   friend inline const QSize operator/(const QSize &size, qreal factor);
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QSize &size);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QSize &size);

constexpr inline QSize::QSize()
   : wd(-1), ht(-1)
{
}

constexpr inline QSize::QSize(int width, int height)
   : wd(width), ht(height)
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

inline void QSize::setWidth(int width)
{
   wd = width;
}

inline void QSize::setHeight(int height)
{
   ht = height;
}

inline QSize QSize::transposed() const
{
   return QSize(ht, wd);
}

inline void QSize::scale(int width, int height, Qt::AspectRatioMode mode)
{
   scale(QSize(width, height), mode);
}

inline void QSize::scale(const QSize &size, Qt::AspectRatioMode mode)
{
   *this = scaled(size, mode);
}

inline QSize QSize::scaled(int width, int height, Qt::AspectRatioMode mode) const
{
   return scaled(QSize(width, height), mode);
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

inline QSize &QSize::operator*=(qreal factor)
{
   wd = qRound(wd * factor);
   ht = qRound(ht * factor);

   return *this;
}

inline bool operator==(const QSize &size1, const QSize &size2)
{
   return size1.wd == size2.wd && size1.ht == size2.ht;
}

inline bool operator!=(const QSize &size1, const QSize &size2)
{
   return size1.wd != size2.wd || size1.ht != size2.ht;
}

inline const QSize operator+(const QSize &size1, const QSize &size2)
{
   return QSize(size1.wd + size2.wd, size1.ht + size2.ht);
}

inline const QSize operator-(const QSize &size1, const QSize &size2)
{
   return QSize(size1.wd - size2.wd, size1.ht - size2.ht);
}

inline const QSize operator*(const QSize &size, qreal factor)
{
   return QSize(qRound(size.wd * factor), qRound(size.ht * factor));
}

inline const QSize operator*(qreal factor, const QSize &size)
{
   return QSize(qRound(size.wd * factor), qRound(size.ht * factor));
}

inline QSize &QSize::operator/=(qreal factor)
{
   Q_ASSERT(!qFuzzyIsNull(factor));
   wd = qRound(wd / factor);
   ht = qRound(ht / factor);

   return *this;
}

inline const QSize operator/(const QSize &size, qreal factor)
{
   Q_ASSERT(!qFuzzyIsNull(factor));
   return QSize(qRound(size.wd / factor), qRound(size.ht / factor));
}

inline QSize QSize::expandedTo(const QSize &other) const
{
   return QSize(qMax(wd, other.wd), qMax(ht, other.ht));
}

inline QSize QSize::boundedTo(const QSize &other) const
{
   return QSize(qMin(wd, other.wd), qMin(ht, other.ht));
}

Q_CORE_EXPORT QDebug operator<<(QDebug, const QSize &size);

class Q_CORE_EXPORT QSizeF
{
 public:
   QSizeF();
   QSizeF(const QSize &size);
   QSizeF(qreal width, qreal height);

   inline bool isNull() const;
   inline bool isEmpty() const;
   inline bool isValid() const;

   inline qreal width() const;
   inline qreal height() const;
   inline void setWidth(qreal width);
   inline void setHeight(qreal height);

   void transpose();
   inline QSizeF transposed() const;

   inline void scale(qreal width, qreal height, Qt::AspectRatioMode mode);
   inline void scale(const QSizeF &size, Qt::AspectRatioMode mode);

   QSizeF scaled(qreal width, qreal height, Qt::AspectRatioMode mode) const;
   QSizeF scaled(const QSizeF &size, Qt::AspectRatioMode mode) const;

   inline QSizeF expandedTo(const QSizeF &other) const;
   inline QSizeF boundedTo(const QSizeF &other) const;

   inline qreal &rwidth();
   inline qreal &rheight();

   inline QSizeF &operator+=(const QSizeF &size);
   inline QSizeF &operator-=(const QSizeF &size);
   inline QSizeF &operator*=(qreal factor);
   inline QSizeF &operator/=(qreal factor);

   inline QSize toSize() const;

 private:
   qreal wd;
   qreal ht;

   friend inline bool operator==(const QSizeF &size1, const QSizeF &size2);
   friend inline bool operator!=(const QSizeF &size1, const QSizeF &size2);
   friend inline const QSizeF operator+(const QSizeF &size1, const QSizeF &size2);
   friend inline const QSizeF operator-(const QSizeF &size1, const QSizeF &size2);
   friend inline const QSizeF operator*(const QSizeF &size, qreal factor);
   friend inline const QSizeF operator*(qreal factor, const QSizeF &size);
   friend inline const QSizeF operator/(const QSizeF &size, qreal factor);
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QSizeF &sizeF);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QSizeF &sizeF);

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
   return wd <= 0.0 || ht <= 0.0;
}

inline bool QSizeF::isValid() const
{
   return wd >= 0.0 && ht >= 0.0;
}

inline qreal QSizeF::width() const
{
   return wd;
}

inline qreal QSizeF::height() const
{
   return ht;
}

inline void QSizeF::setWidth(qreal width)
{
   wd = width;
}

inline void QSizeF::setHeight(qreal height)
{
   ht = height;
}

inline QSizeF QSizeF::transposed() const
{
   return QSizeF(ht, wd);
}

inline void QSizeF::scale(qreal width, qreal height, Qt::AspectRatioMode mode)
{
   scale(QSizeF(width, height), mode);
}

inline void QSizeF::scale(const QSizeF &size, Qt::AspectRatioMode mode)
{
   *this = scaled(size, mode);
}

inline QSizeF QSizeF::scaled(qreal width, qreal height, Qt::AspectRatioMode mode) const
{
   return scaled(QSizeF(width, height), mode);
}

inline qreal &QSizeF::rwidth()
{
   return wd;
}

inline qreal &QSizeF::rheight()
{
   return ht;
}

inline QSizeF &QSizeF::operator+=(const QSizeF &size)
{
   wd += size.wd;
   ht += size.ht;
   return *this;
}

inline QSizeF &QSizeF::operator-=(const QSizeF &size)
{
   wd -= size.wd;
   ht -= size.ht;
   return *this;
}

inline QSizeF &QSizeF::operator*=(qreal factor)
{
   wd *= factor;
   ht *= factor;
   return *this;
}

inline bool operator==(const QSizeF &size1, const QSizeF &size2)
{
   return qFuzzyCompare(size1.wd, size2.wd) && qFuzzyCompare(size1.ht, size2.ht);
}

inline bool operator!=(const QSizeF &size1, const QSizeF &size2)
{
   return !qFuzzyCompare(size1.wd, size2.wd) || !qFuzzyCompare(size1.ht, size2.ht);
}

inline const QSizeF operator+(const QSizeF &size1, const QSizeF &size2)
{
   return QSizeF(size1.wd + size2.wd, size1.ht + size2.ht);
}

inline const QSizeF operator-(const QSizeF &size1, const QSizeF &size2)
{
   return QSizeF(size1.wd - size2.wd, size1.ht - size2.ht);
}

inline const QSizeF operator*(const QSizeF &size, qreal factor)
{
   return QSizeF(size.wd * factor, size.ht * factor);
}

inline const QSizeF operator*(qreal factor, const QSizeF &size)
{
   return QSizeF(size.wd * factor, size.ht * factor);
}

inline QSizeF &QSizeF::operator/=(qreal factor)
{
   Q_ASSERT(!qFuzzyIsNull(factor));
   wd = wd / factor;
   ht = ht / factor;
   return *this;
}

inline const QSizeF operator/(const QSizeF &size, qreal factor)
{
   Q_ASSERT(!qFuzzyIsNull(factor));
   return QSizeF(size.wd / factor, size.ht / factor);
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

Q_CORE_EXPORT QDebug operator<<(QDebug, const QSizeF &sizeF);

#endif // QSIZE_H
