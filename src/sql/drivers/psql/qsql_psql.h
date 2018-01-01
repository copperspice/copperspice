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

QT_BEGIN_NAMESPACE

class QPSQLResultPrivate;
class QPSQLDriverPrivate;
class QPSQLDriver;
class QSqlRecordInfo;

class QPSQLResult : public QSqlResult
{
   friend class QPSQLResultPrivate;

 public:
   QPSQLResult(const QPSQLDriver *db, const QPSQLDriverPrivate *p);
   ~QPSQLResult();

   QVariant handle() const;
   void virtual_hook(int id, void *data);

 protected:
   void cleanup();
   bool fetch(int i);
   bool fetchFirst();
   bool fetchLast();
   QVariant data(int i);
   bool isNull(int field);
   bool reset (const QString &query);
   int size();
   int numRowsAffected();
   QSqlRecord record() const;
   QVariant lastInsertId() const;
   bool prepare(const QString &query);
   bool exec();

 private:
   QPSQLResultPrivate *d;
};

class Q_EXPORT_SQLDRIVER_PSQL QPSQLDriver : public QSqlDriver
{
   CS_OBJECT(QPSQLDriver)

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

   bool hasFeature(DriverFeature f) const;
   bool open(const QString &db, const QString &user, const QString &password, const QString &host,
             int port, const QString &connOpts);

   bool isOpen() const;
   void close();
   QSqlResult *createResult() const;
   QStringList tables(QSql::TableType) const;
   QSqlIndex primaryIndex(const QString &tablename) const;
   QSqlRecord record(const QString &tablename) const;

   Protocol protocol() const;
   QVariant handle() const;

   QString escapeIdentifier(const QString &identifier, IdentifierType type) const;
   QString formatValue(const QSqlField &field, bool trimStrings) const;

 protected:
   bool beginTransaction();
   bool commitTransaction();
   bool rollbackTransaction();

   bool subscribeToNotificationImplementation(const QString &name);
   bool unsubscribeFromNotificationImplementation(const QString &name);
   QStringList subscribedToNotificationsImplementation() const;

 private :
   SQL_CS_SLOT_1(Private, void _q_handleNotification(int un_named_arg1))
   SQL_CS_SLOT_2(_q_handleNotification)

   void init();
   QPSQLDriverPrivate *d;
};

QT_END_NAMESPACE

#endif // QSQL_PSQL_H
