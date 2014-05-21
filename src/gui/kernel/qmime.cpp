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

#include "qmime.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMimeSource
    \brief The QMimeSource class is an abstraction of objects that
           provided formatted data of a certain MIME type.

    \obsolete

    The preferred approach to drag and drop is to use QDrag in
    conjunction with QMimeData. See \l{Drag and Drop} for details.

    \sa QMimeData, QDrag
*/

/*!
    Destroys the MIME source.
*/
QMimeSource::~QMimeSource()
{
}

/*!
    \fn const char *QMimeSource::format(int i) const

    Returns the (\a i - 1)-th supported MIME format, or 0.
*/

/*!
    \fn QByteArray QMimeSource::encodedData(const char *format) const

    Returns the encoded data of this object in the specified MIME
    \a format.
*/

/*!
    Returns true if the object can provide the data in format \a
    mimeType; otherwise returns false.

    If you inherit from QMimeSource, for consistency reasons it is
    better to implement the more abstract canDecode() functions such
    as QTextDrag::canDecode() and QImageDrag::canDecode().
*/
bool QMimeSource::provides(const char* mimeType) const
{
    const char* fmt;
    for (int i=0; (fmt = format(i)); i++) {
        if (!qstricmp(mimeType,fmt))
            return true;
    }
    return false;
}

QT_END_NAMESPACE
