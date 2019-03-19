/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qgenericmatrix.h"

QT_BEGIN_NAMESPACE

/*!
    \class QGenericMatrix
    \brief The QGenericMatrix class is a template class that represents a NxM transformation matrix with N columns and M rows.
    \since 4.6
    \ingroup painting
    \ingroup painting-3D

    The QGenericMatrix template has three parameters:

    \table
    \row \i N \i Number of columns.
    \row \i M \i Number of rows.
    \row \i T \i Element type that is visible to users of the class.
    \endtable

    \sa QMatrix4x4
*/

/*!
    \fn QGenericMatrix::QGenericMatrix()

    Constructs a NxM identity matrix.
*/

/*!
    \fn QGenericMatrix::QGenericMatrix(const QGenericMatrix<N, M, T>& other)

    Constructs a copy of \a other.
*/

/*!
    \fn QGenericMatrix::QGenericMatrix(const T *values)

    Constructs a matrix from the given N * M floating-point \a values.
    The contents of the array \a values is assumed to be in
    row-major order.

    \sa copyDataTo()
*/

/*!
    \fn const T& QGenericMatrix::operator()(int row, int column) const

    Returns a constant reference to the element at position
    (\a row, \a column) in this matrix.
*/

/*!
    \fn T& QGenericMatrix::operator()(int row, int column)

    Returns a reference to the element at position (\a row, \a column)
    in this matrix so that the element can be assigned to.
*/

/*!
    \fn bool QGenericMatrix::isIdentity() const

    Returns true if this matrix is the identity; false otherwise.

    \sa setToIdentity()
*/

/*!
    \fn void QGenericMatrix::setToIdentity()

    Sets this matrix to the identity.

    \sa isIdentity()
*/

/*!
    \fn void QGenericMatrix::fill(T value)

    Fills all elements of this matrix with \a value.
*/

/*!
    \fn QGenericMatrix<M, N> QGenericMatrix::transposed() const

    Returns this matrix, transposed about its diagonal.
*/

/*!
    \fn QGenericMatrix<N, M, T>& QGenericMatrix::operator+=(const QGenericMatrix<N, M, T>& other)

    Adds the contents of \a other to this matrix.
*/

/*!
    \fn QGenericMatrix<N, M, T>& QGenericMatrix::operator-=(const QGenericMatrix<N, M, T>& other)

    Subtracts the contents of \a other from this matrix.
*/

/*!
    \fn QGenericMatrix<N, M, T>& QGenericMatrix::operator*=(T factor)

    Multiplies all elements of this matrix by \a factor.
*/

/*!
    \fn QGenericMatrix<N, M, T>& QGenericMatrix::operator/=(T divisor)

    Divides all elements of this matrix by \a divisor.
*/

/*!
    \fn bool QGenericMatrix::operator==(const QGenericMatrix<N, M, T>& other) const

    Returns true if this matrix is identical to \a other; false otherwise.
*/

/*!
    \fn bool QGenericMatrix::operator!=(const QGenericMatrix<N, M, T>& other) const

    Returns true if this matrix is not identical to \a other; false otherwise.
*/

/*!
    \fn QGenericMatrix<N, M, T> operator+(const QGenericMatrix<N, M, T>& m1, const QGenericMatrix<N, M, T>& m2)
    \relates QGenericMatrix

    Returns the sum of \a m1 and \a m2.
*/

/*!
    \fn QGenericMatrix<N, M, T> operator-(const QGenericMatrix<N, M, T>& m1, const QGenericMatrix<N, M, T>& m2)
    \relates QGenericMatrix

    Returns the difference of \a m1 and \a m2.
*/

/*!
    \fn QGenericMatrix<M1, M2, T> operator*(const QGenericMatrix<N, M2, T>& m1, const QGenericMatrix<M1, N, T>& m2)
    \relates QGenericMatrix

    Returns the product of the NxM2 matrix \a m1 and the M1xN matrix \a m2
    to produce a M1xM2 matrix result.
*/

/*!
    \fn QGenericMatrix<N, M, T> operator-(const QGenericMatrix<N, M, T>& matrix)
    \overload
    \relates QGenericMatrix

    Returns the negation of \a matrix.
*/

/*!
    \fn QGenericMatrix<N, M, T> operator*(T factor, const QGenericMatrix<N, M, T>& matrix)
    \relates QGenericMatrix

    Returns the result of multiplying all elements of \a matrix by \a factor.
*/

/*!
    \fn QGenericMatrix<N, M, T> operator*(const QGenericMatrix<N, M, T>& matrix, T factor)
    \relates QGenericMatrix

    Returns the result of multiplying all elements of \a matrix by \a factor.
*/

/*!
    \fn QGenericMatrix<N, M, T> operator/(const QGenericMatrix<N, M, T>& matrix, T divisor)
    \relates QGenericMatrix

    Returns the result of dividing all elements of \a matrix by \a divisor.
*/

/*!
    \fn void QGenericMatrix::copyDataTo(T *values) const

    Retrieves the N * M items in this matrix and copies them to \a values
    in row-major order.
*/

/*!
    \fn T *QGenericMatrix::data()

    Returns a pointer to the raw data of this matrix.

    \sa constData()
*/

/*!
    \fn const T *QGenericMatrix::data() const

    Returns a constant pointer to the raw data of this matrix.

    \sa constData()
*/

/*!
    \fn const T *QGenericMatrix::constData() const

    Returns a constant pointer to the raw data of this matrix.

    \sa data()
*/

#ifndef QT_NO_DATASTREAM

/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QGenericMatrix<N, M, T> &matrix)
    \relates QGenericMatrix

    Writes the given \a matrix to the given \a stream and returns a
    reference to the stream.

    \sa {Serializing Qt Data Types}
*/

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QGenericMatrix<N, M, T> &matrix)
    \relates QGenericMatrix

    Reads a NxM matrix from the given \a stream into the given \a matrix
    and returns a reference to the stream.

    \sa {Serializing Qt Data Types}
*/

#endif

/*!
    \typedef QMatrix2x2
    \relates QGenericMatrix

    The QMatrix2x2 type defines a convenient instantiation of the
    QGenericMatrix template for 2 columns, 2 rows, and qreal as
    the element type.
*/

/*!
    \typedef QMatrix2x3
    \relates QGenericMatrix

    The QMatrix2x3 type defines a convenient instantiation of the
    QGenericMatrix template for 2 columns, 3 rows, and qreal as
    the element type.
*/

/*!
    \typedef QMatrix2x4
    \relates QGenericMatrix

    The QMatrix2x4 type defines a convenient instantiation of the
    QGenericMatrix template for 2 columns, 4 rows, and qreal as
    the element type.
*/

/*!
    \typedef QMatrix3x2
    \relates QGenericMatrix

    The QMatrix3x2 type defines a convenient instantiation of the
    QGenericMatrix template for 3 columns, 2 rows, and qreal as
    the element type.
*/

/*!
    \typedef QMatrix3x3
    \relates QGenericMatrix

    The QMatrix3x3 type defines a convenient instantiation of the
    QGenericMatrix template for 3 columns, 3 rows, and qreal as
    the element type.
*/

/*!
    \typedef QMatrix3x4
    \relates QGenericMatrix

    The QMatrix3x4 type defines a convenient instantiation of the
    QGenericMatrix template for 3 columns, 4 rows, and qreal as
    the element type.
*/

/*!
    \typedef QMatrix4x2
    \relates QGenericMatrix

    The QMatrix4x2 type defines a convenient instantiation of the
    QGenericMatrix template for 4 columns, 2 rows, and qreal as
    the element type.
*/

/*!
    \typedef QMatrix4x3
    \relates QGenericMatrix

    The QMatrix4x3 type defines a convenient instantiation of the
    QGenericMatrix template for 4 columns, 3 rows, and qreal as
    the element type.
*/

QT_END_NAMESPACE
