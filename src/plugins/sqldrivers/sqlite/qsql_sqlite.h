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

#ifndef QSQL_SQLITE_H
#define QSQL_SQLITE_H

#include <qsqldriver.h>
#include <qsqlresult.h>

struct sqlite3;
struct sqlite3_stmt;

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_SQLITE
#else
#define Q_EXPORT_SQLDRIVER_SQLITE Q_SQL_EXPORT
#endif

class QSQLiteDriverPrivate;
class QSQLiteDriver;

class Q_EXPORT_SQLDRIVER_SQLITE QSQLiteDriver : public QSqlDriver
{
   SQL_CS_OBJECT(QSQLiteDriver)
   Q_DECLARE_PRIVATE(QSQLiteDriver)

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
   friend class QSQLiteResult;

};

CS_DECLARE_METATYPE(sqlite3)
CS_DECLARE_METATYPE(sqlite3_stmt)

#endif
