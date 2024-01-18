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

#include <qsqlerror.h>
#include <qdebug.h>

QDebug operator<<(QDebug dbg, const QSqlError &s)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   dbg << "QSqlError(" << s.nativeErrorCode() << ", " << s.driverText()
      << ", " << s.databaseText() << ')';
   return dbg;
}

class QSqlErrorPrivate
{
 public:
   QString driverError;
   QString databaseError;
   QSqlError::ErrorType errorType;
   QString errorCode;
};
QSqlError::QSqlError(const QString &driverText, const QString &databaseText,
   ErrorType type, const QString &code)
{
   d = new QSqlErrorPrivate;

   d->driverError = driverText;
   d->databaseError = databaseText;
   d->errorType = type;
   d->errorCode = code;
}

/*!
    Creates a copy of \a other.
*/
QSqlError::QSqlError(const QSqlError &other)
{
   d = new QSqlErrorPrivate;

   *d = *other.d;
}

/*!
    Assigns the \a other error's values to this error.
*/

QSqlError &QSqlError::operator=(const QSqlError &other)
{
   *d = *other.d;

   return *this;
}

/*!
    Compare the \a other error's values to this error and returns true, if it equal.
*/

bool QSqlError::operator==(const QSqlError &other) const
{
   return (d->errorType == other.d->errorType);
}


/*!
    Compare the \a other error's values to this error and returns true if it is not equal.
*/

bool QSqlError::operator!=(const QSqlError &other) const
{
   return (d->errorType != other.d->errorType);
}


/*!
    Destroys the object and frees any allocated resources.
*/

QSqlError::~QSqlError()
{
   delete d;
}

/*!
    Returns the text of the error as reported by the driver. This may
    contain database-specific descriptions. It may also be empty.

    \sa setDriverText() databaseText() text()
*/
QString QSqlError::driverText() const
{
   return d->driverError;
}




QString QSqlError::databaseText() const
{
   return d->databaseError;
}



QSqlError::ErrorType QSqlError::type() const
{
   return d->errorType;
}



QString QSqlError::nativeErrorCode() const
{
   return d->errorCode;
}


QString QSqlError::text() const
{
   QString result = d->databaseError;
   if (!d->databaseError.endsWith(QLatin1String("\n"))) {
      result += QLatin1Char(' ');
   }
   result += d->driverError;
   return result;
}


bool QSqlError::isValid() const
{
   return d->errorType != NoError;
}


