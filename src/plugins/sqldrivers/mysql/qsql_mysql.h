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

#ifndef QSQL_MYSQL_H
#define QSQL_MYSQL_H

#include <qsqldriver.h>
#include <qsqlresult.h>

#if defined (Q_OS_WIN)
#include <qt_windows.h>
#endif

#include <mysql.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_MYSQL
#else
#define Q_EXPORT_SQLDRIVER_MYSQL Q_SQL_EXPORT
#endif

class QMYSQLDriverPrivate;
class QMYSQLResultPrivate;
class QMYSQLDriver;
class QSqlRecordInfo;

class QMYSQLResult : public QSqlResult
{
 public:
   explicit QMYSQLResult(const QMYSQLDriver *db);
   ~QMYSQLResult();

   QVariant handle() const override;

 protected:
   void cleanup();
   bool fetch(int i) override;
   bool fetchNext() override;
   bool fetchLast() override;
   bool fetchFirst() override;
   QVariant data(int field) override;
   bool isNull(int field) override;
   bool reset (const QString &query) override;
   int size() override;
   int numRowsAffected() override;
   QVariant lastInsertId() const override;
   QSqlRecord record() const override;
   void virtual_hook(int id, void *data) override;
   bool nextResult() override;

   bool prepare(const QString &stmt) override;
   bool exec() override;

 private:
   friend class QMYSQLDriver;
   friend class QMYSQLResultPrivate;

   QMYSQLResultPrivate *d;
};

class Q_EXPORT_SQLDRIVER_MYSQL QMYSQLDriver : public QSqlDriver
{
   CS_OBJECT(QMYSQLDriver)

   Q_DECLARE_PRIVATE(QMYSQLDriver)

 public:
   explicit QMYSQLDriver(QObject *parent = nullptr);
   explicit QMYSQLDriver(MYSQL *con, QObject *parent = nullptr);
   ~QMYSQLDriver();

   bool hasFeature(DriverFeature f) const override;
   bool open(const QString &db, const QString &user, const QString &password,
      const QString &host, int port, const QString &connOpts) override;

   void close() override;

   QSqlResult *createResult() const override;
   QStringList tables(QSql::TableType) const override;
   QSqlIndex primaryIndex(const QString &tablename) const override;
   QSqlRecord record(const QString &tablename) const override;

   QString formatValue(const QSqlField &field, bool trimStrings) const override;
   QVariant handle() const override;
   QString escapeIdentifier(const QString &identifier, IdentifierType type) const override;

   bool isIdentifierEscaped(const QString &identifier, IdentifierType type) const override;

 protected:
   bool beginTransaction() override;
   bool commitTransaction() override;
   bool rollbackTransaction() override;

 private:
   friend class QMYSQLResult;

   void init();
};

CS_DECLARE_METATYPE(MYSQL)
CS_DECLARE_METATYPE(MYSQL_RES)
CS_DECLARE_METATYPE(MYSQL_STMT)

#endif
