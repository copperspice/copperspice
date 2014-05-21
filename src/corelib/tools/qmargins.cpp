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

#include "qmargins.h"
#include "qdatastream.h"
#include "qdebug.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMargins
    \ingroup painting
    \since 4.6

    \brief The QMargins class defines the four margins of a rectangle. 

    QMargin defines a set of four margins; left, top, right and bottom,
    that describe the size of the borders surrounding a rectangle.

    The isNull() function returns true only if all margins are set to zero.

    QMargin objects can be streamed as well as compared.
*/


/*****************************************************************************
  QMargins member functions
 *****************************************************************************/

/*!
    \fn QMargins::QMargins()

    Constructs a margins object with all margins set to 0.

    \sa isNull()
*/

/*!
    \fn QMargins::QMargins(int left, int top, int right, int bottom)

    Constructs margins with the given \a left, \a top, \a right, \a bottom

    \sa setLeft(), setRight(), setTop(), setBottom()
*/

/*!
    \fn bool QMargins::isNull() const

    Returns true if all margins are is 0; otherwise returns
    false.
*/


/*!
    \fn int QMargins::left() const

    Returns the left margin.

    \sa setLeft()
*/

/*!
    \fn int QMargins::top() const

    Returns the top margin.

    \sa setTop()
*/

/*!
    \fn int QMargins::right() const

    Returns the right margin.
*/

/*!
    \fn int QMargins::bottom() const

    Returns the bottom margin.
*/


/*!
    \fn void QMargins::setLeft(int left)

    Sets the left margin to \a left.
*/

/*!
    \fn void QMargins::setTop(int Top)

    Sets the Top margin to \a Top.
*/

/*!
    \fn void QMargins::setRight(int right)

    Sets the right margin to \a right.
*/

/*!
    \fn void QMargins::setBottom(int bottom)

    Sets the bottom margin to \a bottom.
*/

/*!
    \fn bool operator==(const QMargins &m1, const QMargins &m2)
    \relates QMargins

    Returns true if \a m1 and \a m2 are equal; otherwise returns false.
*/

/*!
    \fn bool operator!=(const QMargins &m1, const QMargins &m2)
    \relates QMargins

    Returns true if \a m1 and \a m2 are different; otherwise returns false.
*/

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QMargins &m) {
    dbg.nospace() << "QMargins(" << m.left() << ", "
            << m.top() << ", " << m.right() << ", " << m.bottom() << ')';
    return dbg.space();
}
#endif

QT_END_NAMESPACE
