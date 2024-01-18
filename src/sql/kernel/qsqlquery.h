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

#ifndef QSQLQUERY_H
#define QSQLQUERY_H

#include <qsql.h>
#include <qsqldatabase.h>
#include <qstring.h>
#include <qcontainerfwd.h>

class QVariant;
class QSqlDriver;
class QSqlError;
class QSqlResult;
class QSqlRecord;
class QSqlQueryPrivate;

class Q_SQL_EXPORT QSqlQuery
{

 public:
   explicit QSqlQuery(QSqlResult *result);
   explicit QSqlQuery(const QString &query = QString(), QSqlDatabase db = QSqlDatabase());
   explicit QSqlQuery(QSqlDatabase db);

   QSqlQuery(const QSqlQuery &other);
   QSqlQuery &operator=(const QSqlQuery &other);

   ~QSqlQuery();

   bool isValid() const;
   bool isActive() const;
   bool isNull(int field) const;
   bool isNull(const QString &name) const;
   int at() const;
   QString lastQuery() const;
   int numRowsAffected() const;
   QSqlError lastError() const;
   bool isSelect() const;
   int size() const;
   const QSqlDriver *driver() const;
   const QSqlResult *result() const;
   bool isForwardOnly() const;
   QSqlRecord record() const;

   void setForwardOnly(bool forward);
   bool exec(const QString &query);
   QVariant value(int index) const;
   QVariant value(const QString &name) const;

   void setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy precisionPolicy);
   QSql::NumericalPrecisionPolicy numericalPrecisionPolicy() const;

   bool seek(int index, bool relative = false);
   bool next();
   bool previous();
   bool first();
   bool last();

   void clear();

   // prepared query support
   bool exec();
   enum BatchExecutionMode { ValuesAsRows, ValuesAsColumns };
   bool execBatch(BatchExecutionMode mode = ValuesAsRows);
   bool prepare(const QString &query);
   void bindValue(const QString &placeholder, const QVariant &value, QSql::ParamType type = QSql::In);
   void bindValue(int pos, const QVariant &value, QSql::ParamType type = QSql::In);
   void addBindValue(const QVariant &value, QSql::ParamType type = QSql::In);

   QVariant boundValue(const QString &placeholder) const;
   QVariant boundValue(int pos) const;
   QMap<QString, QVariant> boundValues() const;
   QString executedQuery() const;
   QVariant lastInsertId() const;
   void finish();
   bool nextResult();

 private:
   QSqlQueryPrivate *d;
};

#endif // QSQLQUERY_H
