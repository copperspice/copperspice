/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qsize.h>
#include <qdatastream.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

void QSize::transpose()
{
   int tmp = wd;
   wd = ht;
   ht = tmp;
}

void QSize::scale(const QSize &s, Qt::AspectRatioMode mode)
{
   if (mode == Qt::IgnoreAspectRatio || wd == 0 || ht == 0) {
      wd = s.wd;
      ht = s.ht;
   } else {
      bool useHeight;
      qint64 rw = qint64(s.ht) * qint64(wd) / qint64(ht);

      if (mode == Qt::KeepAspectRatio) {
         useHeight = (rw <= s.wd);
      } else { // mode == Qt::KeepAspectRatioByExpanding
         useHeight = (rw >= s.wd);
      }

      if (useHeight) {
         wd = rw;
         ht = s.ht;
      } else {
         ht = qint32(qint64(s.wd) * qint64(ht) / qint64(wd));
         wd = s.wd;
      }
   }
}

/*!
    \fn int &QSize::rwidth()

    Returns a reference to the width.

    Using a reference makes it possible to manipulate the width
    directly. For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qsize.cpp 1

    \sa rheight(), setWidth()
*/

/*!
    \fn int &QSize::rheight()

    Returns a reference to the height.

    Using a reference makes it possible to manipulate the height
    directly. For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qsize.cpp 2

    \sa rwidth(), setHeight()
*/

/*!
    \fn QSize &QSize::operator+=(const QSize &size)

    Adds the given \a size to \e this size, and returns a reference to
    this size. For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qsize.cpp 3
*/

/*!
    \fn QSize &QSize::operator-=(const QSize &size)

    Subtracts the given \a size from \e this size, and returns a
    reference to this size. For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qsize.cpp 4
*/

/*!
    \fn QSize &QSize::operator*=(qreal factor)
    \overload

    Multiplies both the width and height by the given \a factor, and
    returns a reference to the size.

    Note that the result is rounded to the nearest integer.

    \sa scale()
*/

/*!
    \fn bool operator==(const QSize &s1, const QSize &s2)
    \relates QSize

    Returns true if \a s1 and \a s2 are equal; otherwise returns false.
*/

/*!
    \fn bool operator!=(const QSize &s1, const QSize &s2)
    \relates QSize

    Returns true if \a s1 and \a s2 are different; otherwise returns false.
*/

/*!
    \fn const QSize operator+(const QSize &s1, const QSize &s2)
    \relates QSize

    Returns the sum of \a s1 and \a s2; each component is added separately.
*/

/*!
    \fn const QSize operator-(const QSize &s1, const QSize &s2)
    \relates QSize

    Returns \a s2 subtracted from \a s1; each component is subtracted
    separately.
*/

/*!
    \fn const QSize operator*(const QSize &size, qreal factor)
    \relates QSize

    Multiplies the given \a size by the given \a factor, and returns
    the result rounded to the nearest integer.

    \sa QSize::scale()
*/

/*!
    \fn const QSize operator*(qreal factor, const QSize &size)
    \overload
    \relates QSize

    Multiplies the given \a size by the given \a factor, and returns
    the result rounded to the nearest integer.
*/

/*!
    \fn QSize &QSize::operator/=(qreal divisor)
    \overload

    Divides both the width and height by the given \a divisor, and
    returns a reference to the size.

    Note that the result is rounded to the nearest integer.

    \sa QSize::scale()
*/

/*!
    \fn const QSize operator/(const QSize &size, qreal divisor)
    \relates QSize
    \overload

    Divides the given \a size by the given \a divisor, and returns the
    result rounded to the nearest integer.

    \sa QSize::scale()
*/

/*!
    \fn QSize QSize::expandedTo(const QSize & otherSize) const

    Returns a size holding the maximum width and height of this size
    and the given \a otherSize.

    \sa boundedTo(), scale()
*/

/*!
    \fn QSize QSize::boundedTo(const QSize & otherSize) const

    Returns a size holding the minimum width and height of this size
    and the given \a otherSize.

    \sa expandedTo(), scale()
*/



/*****************************************************************************
  QSize stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QSize &size)
    \relates QSize

    Writes the given \a size to the given \a stream, and returns a
    reference to the stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &s, const QSize &sz)
{
   if (s.version() == 1) {
      s << (qint16)sz.width() << (qint16)sz.height();
   } else {
      s << (qint32)sz.width() << (qint32)sz.height();
   }
   return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QSize &size)
    \relates QSize

    Reads a size from the given \a stream into the given \a size, and
    returns a reference to the stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &s, QSize &sz)
{
   if (s.version() == 1) {
      qint16 w, h;
      s >> w;
      sz.rwidth() = w;
      s >> h;
      sz.rheight() = h;
   } else {
      qint32 w, h;
      s >> w;
      sz.rwidth() = w;
      s >> h;
      sz.rheight() = h;
   }
   return s;
}
#endif // QT_NO_DATASTREAM

QDebug operator<<(QDebug dbg, const QSize &s)
{
   dbg.nospace() << "QSize(" << s.width() << ", " << s.height() << ')';
   return dbg.space();
}

void QSizeF::transpose()
{
   qreal tmp = wd;
   wd = ht;
   ht = tmp;
}
void QSizeF::scale(const QSizeF &s, Qt::AspectRatioMode mode)
{
   if (mode == Qt::IgnoreAspectRatio || qIsNull(wd) || qIsNull(ht)) {
      wd = s.wd;
      ht = s.ht;
   } else {
      bool useHeight;
      qreal rw = s.ht * wd / ht;

      if (mode == Qt::KeepAspectRatio) {
         useHeight = (rw <= s.wd);
      } else { // mode == Qt::KeepAspectRatioByExpanding
         useHeight = (rw >= s.wd);
      }

      if (useHeight) {
         wd = rw;
         ht = s.ht;
      } else {
         ht = s.wd * ht / wd;
         wd = s.wd;
      }
   }
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &s, const QSizeF &sz)
{
   s << double(sz.width()) << double(sz.height());
   return s;
}

QDataStream &operator>>(QDataStream &s, QSizeF &sz)
{
   double w, h;
   s >> w;
   s >> h;
   sz.setWidth(qreal(w));
   sz.setHeight(qreal(h));
   return s;
}
#endif

QDebug operator<<(QDebug dbg, const QSizeF &s)
{
   dbg.nospace() << "QSizeF(" << s.width() << ", " << s.height() << ')';
   return dbg.space();
}

QT_END_NAMESPACE
