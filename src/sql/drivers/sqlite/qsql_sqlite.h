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

#ifndef QSQL_SQLITE_H
#define QSQL_SQLITE_H

#include <QtSql/qsqldriver.h>
#include <QtSql/qsqlresult.h>
#include <qsqlcachedresult_p.h>

struct sqlite3;

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_SQLITE
#else
#define Q_EXPORT_SQLDRIVER_SQLITE Q_SQL_EXPORT
#endif

QT_BEGIN_NAMESPACE

class QSQLiteDriverPrivate;
class QSQLiteResultPrivate;
class QSQLiteDriver;

class QSQLiteResult : public QSqlCachedResult
{
   friend class QSQLiteDriver;
   friend class QSQLiteResultPrivate;

 public:
   explicit QSQLiteResult(const QSQLiteDriver *db);
   ~QSQLiteResult();
   QVariant handle() const override;

 protected:
   bool gotoNext(QSqlCachedResult::ValueCache &row, int idx) override;
   bool reset(const QString &query) override;
   bool prepare(const QString &query) override;
   bool exec() override;
   int size() override;
   int numRowsAffected() override;
   QVariant lastInsertId() const override;
   QSqlRecord record() const override;
   void virtual_hook(int id, void *data) override;

 private:
   QSQLiteResultPrivate *d;
};

class Q_EXPORT_SQLDRIVER_SQLITE QSQLiteDriver : public QSqlDriver
{
   CS_OBJECT(QSQLiteDriver)
   friend class QSQLiteResult;

 public:
   explicit QSQLiteDriver(QObject *parent = nullptr);
   explicit QSQLiteDriver(sqlite3 *connection, QObject *parent = nullptr);
   ~QSQLiteDriver();

   bool hasFeature(DriverFeature f) const override;

   bool open(const QString &db, const QString &user, const QString &password, const QString &host,
                  int port, const QString &connOpts) override;

   void close() override;
   QSqlResult *createResult() const override;
   bool beginTransaction() override;
   bool commitTransaction() override;
   bool rollbackTransaction() override;
   QStringList tables(QSql::TableType) const override;

   QSqlRecord record(const QString &tablename) const override;
   QSqlIndex primaryIndex(const QString &table) const override;
   QVariant handle() const override;
   QString escapeIdentifier(const QString &identifier, IdentifierType) const override;

 private:
   QSQLiteDriverPrivate *d;
};

QT_END_NAMESPACE

#endif // QSQL_SQLITE_H
