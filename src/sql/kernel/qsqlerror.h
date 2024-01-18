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

#ifndef QSQLERROR_H
#define QSQLERROR_H

#include <qstring.h>
#include <qsql.h>


class QSqlErrorPrivate;

class Q_SQL_EXPORT QSqlError
{

 public:
   enum ErrorType {
      NoError,
      ConnectionError,
      StatementError,
      TransactionError,
      UnknownError
   };

   QSqlError( const QString &driverText = QString(),
      const QString &databaseText = QString(),
      ErrorType type = NoError,
      const QString &errorCode = QString());

   QSqlError(const QSqlError &other);
   QSqlError &operator=(const QSqlError &other);
   bool operator==(const QSqlError &other) const;
   bool operator!=(const QSqlError &other) const;
   ~QSqlError();

   QString driverText() const;

   QString databaseText() const;

   ErrorType type() const;


   QString nativeErrorCode() const;
   QString text() const;
   bool isValid() const;

 private:
   QSqlErrorPrivate *d;


};

Q_SQL_EXPORT QDebug operator<<(QDebug, const QSqlError &);


#endif // QSQLERROR_H
