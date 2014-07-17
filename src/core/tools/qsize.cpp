/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qsize.h"
#include "qdatastream.h"
#include "qdebug.h"

QT_BEGIN_NAMESPACE

/*!
    \class QSize
    \ingroup painting

    \brief The QSize class defines the size of a two-dimensional
    object using integer point precision.

    A size is specified by a width() and a height().  It can be set in
    the constructor and changed using the setWidth(), setHeight(), or
    scale() functions, or using arithmetic operators. A size can also
    be manipulated directly by retrieving references to the width and
    height using the rwidth() and rheight() functions. Finally, the
    width and height can be swapped using the transpose() function.

    The isValid() function determines if a size is valid (a valid size
    has both width and height greater than zero). The isEmpty()
    function returns true if either of the width and height is less
    than, or equal to, zero, while the isNull() function returns true
    only if both the width and the height is zero.

    Use the expandedTo() function to retrieve a size which holds the
    maximum height and width of \e this size and a given
    size. Similarly, the boundedTo() function returns a size which
    holds the minimum height and width of \e this size and a given
    size.

    QSize objects can be streamed as well as compared.

    \sa QSizeF, QPoint, QRect
*/


/*****************************************************************************
  QSize member functions
 *****************************************************************************/

/*!
    \fn QSize::QSize()

    Constructs a size with an invalid width and height (i.e., isValid()
    returns false).

    \sa isValid()
*/

/*!
    \fn QSize::QSize(int width, int height)

    Constructs a size with the given \a width and \a height.

    \sa setWidth(), setHeight()
*/

/*!
    \fn bool QSize::isNull() const

    Returns true if both the width and height is 0; otherwise returns
    false.

    \sa isValid(), isEmpty()
*/

/*!
    \fn bool QSize::isEmpty() const

    Returns true if either of the width and height is less than or
    equal to 0; otherwise returns false.

    \sa isNull(), isValid()
*/

/*!
    \fn bool QSize::isValid() const

    Returns true if both the width and height is equal to or greater
    than 0; otherwise returns false.

    \sa isNull(), isEmpty()
*/

/*!
    \fn int QSize::width() const

    Returns the width.

    \sa height(), setWidth()
*/

/*!
    \fn int QSize::height() const

    Returns the height.

    \sa width(), setHeight()
*/

/*!
    \fn void QSize::setWidth(int width)

    Sets the width to the given \a width.

    \sa rwidth(), width(), setHeight()
*/

/*!
    \fn void QSize::setHeight(int height)

    Sets the height to the given \a height.

    \sa rheight(), height(), setWidth()
*/

/*!
    Swaps the width and height values.

    \sa setWidth(), setHeight()
*/

void QSize::transpose()
{
   int tmp = wd;
   wd = ht;
   ht = tmp;
}

/*!
  \fn void QSize::scale(int width, int height, Qt::AspectRatioMode mode)

    Scales the size to a rectangle with the given \a width and \a
    height, according to the specified \a mode:

    \list
    \i If \a mode is Qt::IgnoreAspectRatio, the size is set to (\a width, \a height).
    \i If \a mode is Qt::KeepAspectRatio, the current size is scaled to a rectangle
       as large as possible inside (\a width, \a height), preserving the aspect ratio.
    \i If \a mode is Qt::KeepAspectRatioByExpanding, the current size is scaled to a rectangle
       as small as possible outside (\a width, \a height), preserving the aspect ratio.
    \endlist

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qsize.cpp 0

    \sa setWidth(), setHeight()
*/

/*!
    \fn void QSize::scale(const QSize &size, Qt::AspectRatioMode mode)
    \overload

    Scales the size to a rectangle with the given \a size, according to
    the specified \a mode.
*/
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
