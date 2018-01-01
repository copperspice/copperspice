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

#include <qsqlerror.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

QDebug operator<<(QDebug dbg, const QSqlError &s)
{
   dbg.nospace() << "QSqlError(" << s.number() << ", " << s.driverText() <<
                 ", " << s.databaseText() << ')';
   return dbg.space();
}

QSqlError::QSqlError(const QString &driverText, const QString &databaseText, ErrorType type,
                     int number)
   : driverError(driverText), databaseError(databaseText), errorType(type), errorNumber(number)
{
}

/*!
    Creates a copy of \a other.
*/
QSqlError::QSqlError(const QSqlError &other)
   : driverError(other.driverError), databaseError(other.databaseError),
     errorType(other.errorType),
     errorNumber(other.errorNumber)
{
}

/*!
    Assigns the \a other error's values to this error.
*/

QSqlError &QSqlError::operator=(const QSqlError &other)
{
   driverError = other.driverError;
   databaseError = other.databaseError;
   errorType = other.errorType;
   errorNumber = other.errorNumber;
   return *this;
}

/*!
    Compare the \a other error's values to this error and returns true, if it equal.
*/

bool QSqlError::operator==(const QSqlError &other)
{
   return (errorType == other.errorType);
}


/*!
    Compare the \a other error's values to this error and returns true if it is not equal.
*/

bool QSqlError::operator!=(const QSqlError &other)
{
   return (errorType != other.errorType);
}


/*!
    Destroys the object and frees any allocated resources.
*/

QSqlError::~QSqlError()
{
}

/*!
    Returns the text of the error as reported by the driver. This may
    contain database-specific descriptions. It may also be empty.

    \sa setDriverText() databaseText() text()
*/
QString QSqlError::driverText() const
{
   return driverError;
}

/*!
    Sets the driver error text to the value of \a driverText.

    \sa driverText() setDatabaseText() text()
*/

void QSqlError::setDriverText(const QString &driverText)
{
   driverError = driverText;
}

/*!
    Returns the text of the error as reported by the database. This
    may contain database-specific descriptions; it may be empty.

    \sa setDatabaseText() driverText() text()
*/

QString QSqlError::databaseText() const
{
   return databaseError;
}

/*!
    Sets the database error text to the value of \a databaseText.

    \sa databaseText() setDriverText() text()
*/

void QSqlError::setDatabaseText(const QString &databaseText)
{
   databaseError = databaseText;
}

/*!
    Returns the error type, or -1 if the type cannot be determined.

    \sa setType()
*/

QSqlError::ErrorType QSqlError::type() const
{
   return errorType;
}

/*!
    Sets the error type to the value of \a type.

    \sa type()
*/

void QSqlError::setType(ErrorType type)
{
   errorType = type;
}

/*!
    Returns the database-specific error number, or -1 if it cannot be
    determined.

    \sa setNumber()
*/

int QSqlError::number() const
{
   return errorNumber;
}

/*!
    Sets the database-specific error number to \a number.

    \sa number()
*/

void QSqlError::setNumber(int number)
{
   errorNumber = number;
}

/*!
    This is a convenience function that returns databaseText() and
    driverText() concatenated into a single string.

    \sa driverText() databaseText()
*/

QString QSqlError::text() const
{
   QString result = databaseError;
   if (!databaseError.endsWith(QLatin1String("\n"))) {
      result += QLatin1Char(' ');
   }
   result += driverError;
   return result;
}

/*!
    Returns true if an error is set, otherwise false.

    Example:
    \snippet doc/src/snippets/code/src_sql_kernel_qsqlerror.cpp 0

    \sa type()
*/
bool QSqlError::isValid() const
{
   return errorType != NoError;
}

QT_END_NAMESPACE
