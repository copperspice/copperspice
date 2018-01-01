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

#include <qpoint.h>
#include <qdatastream.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

/*!
    \class QPoint
    \ingroup painting

    \brief The QPoint class defines a point in the plane using integer
    precision.

    A point is specified by a x coordinate and an y coordinate which
    can be accessed using the x() and y() functions. The isNull()
    function returns true if both x and y are set to 0. The
    coordinates can be set (or altered) using the setX() and setY()
    functions, or alternatively the rx() and ry() functions which
    return references to the coordinates (allowing direct
    manipulation).

    Given a point \e p, the following statements are all equivalent:

    \snippet doc/src/snippets/code/src_corelib_tools_qpoint.cpp 0

    A QPoint object can also be used as a vector: Addition and
    subtraction are defined as for vectors (each component is added
    separately). A QPoint object can also be divided or multiplied by
    an \c int or a \c qreal.

    In addition, the QPoint class provides the manhattanLength()
    function which gives an inexpensive approximation of the length of
    the QPoint object interpreted as a vector. Finally, QPoint objects
    can be streamed as well as compared.

    \sa QPointF, QPolygon
*/


/*****************************************************************************
  QPoint member functions
 *****************************************************************************/

/*!
    \fn QPoint::QPoint()

    Constructs a null point, i.e. with coordinates (0, 0)

    \sa isNull()
*/

/*!
    \fn QPoint::QPoint(int x, int y)

    Constructs a point with the given coordinates (\a x, \a  y).

    \sa setX(), setY()
*/

/*!
    \fn bool QPoint::isNull() const

    Returns true if both the x and y coordinates are set to 0,
    otherwise returns false.
*/

/*!
    \fn int QPoint::x() const

    Returns the x coordinate of this point.

    \sa setX(), rx()
*/

/*!
    \fn int QPoint::y() const

    Returns the y coordinate of this point.

    \sa setY(), ry()
*/

/*!
    \fn void QPoint::setX(int x)

    Sets the x coordinate of this point to the given \a x coordinate.

    \sa x() setY()
*/

/*!
    \fn void QPoint::setY(int y)

    Sets the y coordinate of this point to the given \a y coordinate.

    \sa y() setX()
*/


/*!
    \fn int &QPoint::rx()

    Returns a reference to the x coordinate of this point.

    Using a reference makes it possible to directly manipulate x. For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qpoint.cpp 1

    \sa x() setX()
*/

/*!
    \fn int &QPoint::ry()

    Returns a reference to the y coordinate of this point.

    Using a reference makes it possible to directly manipulate y. For
    example:

    \snippet doc/src/snippets/code/src_corelib_tools_qpoint.cpp 2

    \sa y(), setY()
*/


/*!
    \fn QPoint &QPoint::operator+=(const QPoint &point)

    Adds the given \a point to this point and returns a reference to
    this point. For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qpoint.cpp 3

    \sa operator-=()
*/

/*!
    \fn QPoint &QPoint::operator-=(const QPoint &point)

    Subtracts the given \a point from this point and returns a
    reference to this point. For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qpoint.cpp 4

    \sa operator+=()
*/

/*!
    \fn QPoint &QPoint::operator*=(float factor)
    \since 4.8

    Multiplies this point's coordinates by the given \a factor, and
    returns a reference to this point.

    Note that the result is rounded to the nearest integer as points are held as
    integers. Use QPointF for floating point accuracy.

    \sa operator/=()
*/

/*!
    \fn QPoint &QPoint::operator*=(double factor)
    \since 4.8

    Multiplies this point's coordinates by the given \a factor, and
    returns a reference to this point. For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qpoint.cpp 5

    Note that the result is rounded to the nearest integer as points are held as
    integers. Use QPointF for floating point accuracy.

    \sa operator/=()
*/

/*!
    \fn QPoint &QPoint::operator*=(int factor)
    \since 4.8

    Multiplies this point's coordinates by the given \a factor, and
    returns a reference to this point.

    \sa operator/=()
*/

/*!
    \fn bool operator==(const QPoint &p1, const QPoint &p2)
    \relates QPoint

    Returns true if \a p1 and \a p2 are equal; otherwise returns
    false.
*/

/*!
    \fn bool operator!=(const QPoint &p1, const QPoint &p2)
    \relates QPoint

    Returns true if \a p1 and \a p2 are not equal; otherwise returns false.
*/

/*!
    \fn const QPoint operator+(const QPoint &p1, const QPoint &p2)
    \relates QPoint

    Returns a QPoint object that is the sum of the given points, \a p1
    and \a p2; each component is added separately.

    \sa QPoint::operator+=()
*/

/*!
    \fn const QPoint operator-(const QPoint &p1, const QPoint &p2)
    \relates QPoint

    Returns a QPoint object that is formed by subtracting \a p2 from
    \a p1; each component is subtracted separately.

    \sa QPoint::operator-=()
*/

/*!
    \fn const QPoint operator*(const QPoint &point, float factor)
    \relates QPoint
    \since 4.8

    Returns a copy of the given \a point multiplied by the given \a factor.

    Note that the result is rounded to the nearest integer as points
    are held as integers. Use QPointF for floating point accuracy.

    \sa QPoint::operator*=()
*/

/*!
    \fn const QPoint operator*(const QPoint &point, double factor)
    \relates QPoint
    \since 4.8

    Returns a copy of the given \a point multiplied by the given \a factor.

    Note that the result is rounded to the nearest integer as points
    are held as integers. Use QPointF for floating point accuracy.

    \sa QPoint::operator*=()
*/

/*!
    \fn const QPoint operator*(const QPoint &point, int factor)
    \relates QPoint
    \since 4.8

    Returns a copy of the given \a point multiplied by the given \a factor.

    \sa QPoint::operator*=()
*/

/*!
    \fn const QPoint operator*(float factor, const QPoint &point)
    \overload
    \relates QPoint
    \since 4.8

    Returns a copy of the given \a point multiplied by the given \a factor.
*/

/*!
    \fn const QPoint operator*(double factor, const QPoint &point)
    \overload
    \relates QPoint
    \since 4.8

    Returns a copy of the given \a point multiplied by the given \a factor.
*/

/*!
    \fn const QPoint operator*(int factor, const QPoint &point)
    \overload
    \relates QPoint
    \since 4.8

    Returns a copy of the given \a point multiplied by the given \a factor.
*/

/*!
    \fn const QPoint operator-(const QPoint &point)
    \overload
    \relates QPoint

    Returns a QPoint object that is formed by changing the sign of
    both components of the given \a point.

    Equivalent to \c{QPoint(0,0) - point}.
*/

/*!
    \fn QPoint &QPoint::operator/=(qreal divisor)
    \overload

    Divides both x and y by the given \a divisor, and returns a reference to this
    point. For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qpoint.cpp 6

    Note that the result is rounded to the nearest integer as points are held as
    integers. Use QPointF for floating point accuracy.

    \sa operator*=()
*/

/*!
    \fn const QPoint operator/(const QPoint &point, qreal divisor)
    \relates QPoint

    Returns the QPoint formed by dividing both components of the given \a point
    by the given \a divisor.

    Note that the result is rounded to the nearest integer as points are held as
    integers. Use QPointF for floating point accuracy.

    \sa QPoint::operator/=()
*/

/*****************************************************************************
  QPoint stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QPoint &point)
    \relates QPoint

    Writes the given \a point to the given \a stream and returns a
    reference to the stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &s, const QPoint &p)
{
   if (s.version() == 1) {
      s << (qint16)p.x() << (qint16)p.y();
   } else {
      s << (qint32)p.x() << (qint32)p.y();
   }
   return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QPoint &point)
    \relates QPoint

    Reads a point from the given \a stream into the given \a point
    and returns a reference to the stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &s, QPoint &p)
{
   if (s.version() == 1) {
      qint16 x, y;
      s >> x;
      p.rx() = x;
      s >> y;
      p.ry() = y;

   } else {
      qint32 x, y;
      s >> x;
      p.rx() = x;
      s >> y;
      p.ry() = y;
   }
   return s;
}

#endif // QT_NO_DATASTREAM

int QPoint::manhattanLength() const
{
   return qAbs(x()) + qAbs(y());
}

QDebug operator<<(QDebug dbg, const QPoint &p)
{
   dbg.nospace() << "QPoint(" << p.x() << ',' << p.y() << ')';
   return dbg.space();
}

QDebug operator<<(QDebug d, const QPointF &p)
{
   d.nospace() << "QPointF(" << p.x() << ", " << p.y() << ')';
   return d.space();
}


qreal QPointF::manhattanLength() const
{
   return qAbs(x()) + qAbs(y());
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &s, const QPointF &p)
{
   s << double(p.x()) << double(p.y());
   return s;
}

QDataStream &operator>>(QDataStream &s, QPointF &p)
{
   double x, y;
   s >> x;
   s >> y;
   p.setX(qreal(x));
   p.setY(qreal(y));
   return s;
}
#endif // QT_NO_DATASTREAM

QT_END_NAMESPACE
