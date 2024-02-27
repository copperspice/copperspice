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

#ifndef QSQL_ODBC_H
#define QSQL_ODBC_H

#include <qsqldriver.h>
#include <qsqlresult.h>

#if defined (Q_OS_WIN)
#include <qt_windows.h>
#endif

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_ODBC
#else
#define Q_EXPORT_SQLDRIVER_ODBC Q_SQL_EXPORT
#endif

#ifdef Q_OS_UNIX
#define HAVE_LONG_LONG 1   // force UnixODBC NOT to fall back to a struct for BIGINTs
#endif

#include <sql.h>
#include <sqlext.h>

class QODBCResultPrivate;
class QODBCDriverPrivate;
class QODBCDriver;
class QSqlRecordInfo;

class QODBCResult : public QSqlResult
{
 public:
   QODBCResult(const QODBCDriver *db, QODBCDriverPrivate *p);
   virtual ~QODBCResult();

   bool prepare(const QString &query) override;
   bool exec() override;

   QVariant handle() const override;
   QVariant lastInsertId() const override;

   void setForwardOnly(bool forward) override;

 protected:
   bool fetchNext() override;
   bool fetchFirst() override;
   bool fetchLast() override;
   bool fetchPrevious() override;
   bool fetch(int i) override;
   bool reset (const QString &query) override;
   QVariant data(int field) override;
   bool isNull(int field) override;
   int size() override;
   int numRowsAffected() override;
   QSqlRecord record() const override;

   void virtual_hook(int id, void *data) override;
   void detachFromResultSet() override;
   bool nextResult() override;

 private:
   Q_DECLARE_PRIVATE(QODBCResult)
};

class Q_EXPORT_SQLDRIVER_ODBC QODBCDriver : public QSqlDriver
{
   CS_OBJECT(QODBCDriver)

 public:
   explicit QODBCDriver(QObject *parent = nullptr);
   QODBCDriver(SQLHANDLE env, SQLHANDLE con, QObject *parent = nullptr);

   ~QODBCDriver();

   bool hasFeature(DriverFeature f) const override;
   void close() override;
   QSqlResult *createResult() const override;
   QStringList tables(QSql::TableType) const override;
   QSqlRecord record(const QString &tablename) const override;
   QSqlIndex primaryIndex(const QString &tablename) const override;
   QVariant handle() const override;
   QString formatValue(const QSqlField &field, bool trimStrings) const override;

   bool open(const QString &db, const QString &user, const QString &password, const QString &host,
         int port, const QString &connOpts) override;

   QString escapeIdentifier(const QString &identifier, IdentifierType type) const override;

 protected:
   bool isIdentifierEscapedImplementation(const QString &identifier, IdentifierType type) const;

   bool beginTransaction() override;
   bool commitTransaction() override;
   bool rollbackTransaction() override;

 private:
   void init();
   bool endTrans();
   void cleanup();

   Q_DECLARE_PRIVATE(QODBCDriver)

   friend class QODBCResultPrivate;
};

#endif
