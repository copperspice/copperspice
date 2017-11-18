/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QSQLERROR_H
#define QSQLERROR_H

#include <qstring.h>

class QDebug;

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
              int number = -1);
   QSqlError(const QSqlError &other);
   QSqlError &operator=(const QSqlError &other);
   bool operator==(const QSqlError &other);
   bool operator!=(const QSqlError &other);
   ~QSqlError();

   QString driverText() const;
   void setDriverText(const QString &driverText);
   QString databaseText() const;
   void setDatabaseText(const QString &databaseText);
   ErrorType type() const;
   void setType(ErrorType type);
   int number() const;
   void setNumber(int number);
   QString text() const;
   bool isValid() const;

 private:
   QString driverError;
   QString databaseError;
   ErrorType errorType;
   int errorNumber;
};

Q_SQL_EXPORT QDebug operator<<(QDebug, const QSqlError &);


#endif // QSQLERROR_H
