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

#ifndef QSQL_PSQL_H
#define QSQL_PSQL_H

#include <qsqldriver.h>
#include <qsqlresult.h>


#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_PSQL
#else
#define Q_EXPORT_SQLDRIVER_PSQL Q_SQL_EXPORT
#endif

using PGconn   = struct pg_conn;
using PGresult = struct pg_result;

class QPSQLResultPrivate;
class QPSQLDriverPrivate;
class QPSQLDriver;
class QSqlRecordInfo;

class QPSQLResult : public QSqlResult
{
   Q_DECLARE_PRIVATE(QPSQLResult)

 public:
   QPSQLResult(const QPSQLDriver *db);
   ~QPSQLResult();

   QVariant handle() const override;
   void virtual_hook(int id, void *data) override;

 protected:
   void cleanup();
   bool fetch(int i) override;
   bool fetchFirst() override;
   bool fetchLast() override;
   QVariant data(int i) override;
   bool isNull(int field) override;
   bool reset (const QString &query) override;
   int size() override;
   int numRowsAffected() override;
   QSqlRecord record() const override;
   QVariant lastInsertId() const override;
   bool prepare(const QString &query) override;
   bool exec() override;
};

class Q_EXPORT_SQLDRIVER_PSQL QPSQLDriver : public QSqlDriver
{
   CS_OBJECT(QPSQLDriver)

   Q_DECLARE_PRIVATE(QPSQLDriver)

 public:
   enum Protocol {
      VersionUnknown = -1,
      Version6  = 6,
      Version7  = 7,
      Version71 = 8,
      Version73 = 9,
      Version74 = 10,
      Version8  = 11,
      Version81 = 12,
      Version82 = 13,
      Version83 = 14,
      Version84 = 15,
      Version9  = 16,
      Version91 = 17,
      Version92 = 18,
      Version93 = 19,
      Version94 = 20,
      Version95 = 21,
      Version96 = 22,
      Version97 = 23,
      Version98 = 24,
      Version10 = 25,
      Version11 = 26,
      Version12 = 27,
   };

   explicit QPSQLDriver(QObject *parent = nullptr);
   explicit QPSQLDriver(PGconn *conn, QObject *parent = nullptr);
   ~QPSQLDriver();

   bool hasFeature(DriverFeature f) const override;
   bool open(const QString &db, const QString &user, const QString &password,
      const QString &host, int port, const QString &connOpts) override;

   bool isOpen() const override;
   void close() override;
   QSqlResult *createResult() const override;
   QStringList tables(QSql::TableType) const override;
   QSqlIndex primaryIndex(const QString &tablename) const override;
   QSqlRecord record(const QString &tablename) const override;

   Protocol protocol() const;
   QVariant handle() const override;

   QString escapeIdentifier(const QString &identifier, IdentifierType type) const override;
   QString formatValue(const QSqlField &field, bool trimStrings) const override;

   bool subscribeToNotification(const QString &name) override;
   bool unsubscribeFromNotification(const QString &name) override;
   QStringList subscribedToNotifications() const override;

 protected:
   bool beginTransaction() override;
   bool commitTransaction() override;
   bool rollbackTransaction() override;

 private:
   CS_SLOT_1(Private, void _q_handleNotification(int handle))
   CS_SLOT_2(_q_handleNotification)

   friend class QPSQLResultPrivate;
};

CS_DECLARE_METATYPE(pg_conn)
CS_DECLARE_METATYPE(pg_result)

#endif
