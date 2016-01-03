/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#include <qstringbuilder.h>
#include <QtCore/qtextcodec.h>

QT_BEGIN_NAMESPACE

/*!
    \class QStringBuilder
    \internal
    \reentrant
    \since 4.6

    \brief The QStringBuilder class is a template class that provides a facility to build up QStrings from smaller chunks.

    \ingroup tools
    \ingroup shared
    \ingroup string-processing


    To build a QString by multiple concatenations, QString::operator+()
    is typically used. This causes \e{n - 1} reallocations when building
    a string from \e{n} chunks.

    QStringBuilder uses expression templates to collect the individual
    chunks, compute the total size, allocate the required amount of
    memory for the final QString object, and copy the chunks into the
    allocated memory.

    The QStringBuilder class is not to be used explicitly in user
    code.  Instances of the class are created as return values of the
    operator%() function, acting on objects of type QString,
    QLatin1String, QStringRef, QChar, QCharRef,
    QLatin1Char, and \c char.

    Concatenating strings with operator%() generally yields better
    performance then using \c QString::operator+() on the same chunks
    if there are three or more of them, and performs equally well in other
    cases.

    \sa QLatin1String, QString
*/

/*! \fn QStringBuilder::QStringBuilder(const A &a, const B &b)
  Constructs a QStringBuilder from \a a and \a b.
 */

/* \fn QStringBuilder::operator%(const A &a, const B &b)

    Returns a \c QStringBuilder object that is converted to a QString object
    when assigned to a variable of QString type or passed to a function that
    takes a QString parameter.

    This function is usable with arguments of type \c QString,
    \c QLatin1String, \c QStringRef,
    \c QChar, \c QCharRef, \c QLatin1Char, and \c char.
*/

/*! \fn QByteArray QStringBuilder::toLatin1() const
  Returns a Latin-1 representation of the string as a QByteArray.  The
  returned byte array is undefined if the string contains non-Latin1
  characters.
 */

/*!
    \fn operator QStringBuilder::QString() const

    Converts the \c QLatin1String into a \c QString object.
*/

/*! \internal
   Note: The len contains the ending \0
 */
void QAbstractConcatenable::convertFromAscii(const char *a, int len, QChar *&out)
{
   if (len == -1) {
      if (!a) {
         return;
      }
      while (*a) {
         *out++ = QLatin1Char(*a++);
      }
   } else {
      for (int i = 0; i < len - 1; ++i) {
         *out++ = QLatin1Char(a[i]);
      }
   }
}

QT_END_NAMESPACE
