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

#ifndef QSQL_PSQL_H
#define QSQL_PSQL_H

#include <qsqlresult.h>
#include <qsqldriver.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_PSQL
#else
#define Q_EXPORT_SQLDRIVER_PSQL Q_SQL_EXPORT
#endif

typedef struct pg_conn PGconn;
typedef struct pg_result PGresult;

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

   QVariant handle() const;
   void virtual_hook(int id, void *data);

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
   SQL_CS_OBJECT(QPSQLDriver)

   Q_DECLARE_PRIVATE(QPSQLDriver)

 public:
   enum Protocol {
      VersionUnknown = -1,
      Version6 = 6,
      Version7 = 7,
      Version71 = 8,
      Version73 = 9,
      Version74 = 10,
      Version8 = 11,
      Version81 = 12,
      Version82 = 13,
      Version83 = 14,
      Version84 = 15,
      Version9 = 16,
   };

   explicit QPSQLDriver(QObject *parent = nullptr);
   explicit QPSQLDriver(PGconn *conn, QObject *parent = nullptr);
   ~QPSQLDriver();

   bool hasFeature(DriverFeature f) const override;
   bool open(const QString &db, const QString &user, const QString &password,
      const QString &host, int port, const QString &connOpts) override;

   bool isOpen() const;
   void close();
   QSqlResult *createResult() const;
   QStringList tables(QSql::TableType) const;
   QSqlIndex primaryIndex(const QString &tablename) const;
   QSqlRecord record(const QString &tablename) const;

   Protocol protocol() const;
   QVariant handle() const override;

   QString escapeIdentifier(const QString &identifier, IdentifierType type) const;
   QString formatValue(const QSqlField &field, bool trimStrings) const;

   bool subscribeToNotification(const QString &name) override;
   bool unsubscribeFromNotification(const QString &name) override;
   QStringList subscribedToNotifications() const override;

 protected:
   bool beginTransaction();
   bool commitTransaction();
   bool rollbackTransaction();

   QPSQLDriverPrivate *d_ptr;

 private:
   SQL_CS_SLOT_1(Private, void _q_handleNotification(int un_named_arg1))
   SQL_CS_SLOT_2(_q_handleNotification)

   friend class QPSQLResultPrivate;
};

#endif // QSQL_PSQL_H
