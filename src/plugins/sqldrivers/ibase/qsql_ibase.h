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

#ifndef QSQL_IBASE_H
#define QSQL_IBASE_H

#include <qsqlresult.h>
#include <qsqldriver.h>
#include <qsqlcachedresult_p.h>
#include <ibase.h>

class QIBaseDriverPrivate;
class QIBaseResultPrivate;
class QIBaseDriver;

class QIBaseResult : public QSqlCachedResult
{
   friend class QIBaseResultPrivate;

 public:
   explicit QIBaseResult(const QIBaseDriver *db);
   virtual ~QIBaseResult();

   bool prepare(const QString &query);
   bool exec();
   QVariant handle() const;

 protected:
   bool gotoNext(QSqlCachedResult::ValueCache &row, int rowIdx);
   bool reset (const QString &query);
   int size();
   int numRowsAffected();
   QSqlRecord record() const;

 private:
   QIBaseResultPrivate *d;
};

class QIBaseDriver : public QSqlDriver
{
   CS_OBJECT(QIBaseDriver)

   friend class QIBaseDriverPrivate;
   friend class QIBaseResultPrivate;

 public:
   explicit QIBaseDriver(QObject *parent = nullptr);
   explicit QIBaseDriver(isc_db_handle connection, QObject *parent = nullptr);
   virtual ~QIBaseDriver();
   bool hasFeature(DriverFeature f) const;
   bool open(const QString &db,
      const QString &user,
      const QString &password,
      const QString &host,
      int port,
      const QString &connOpts);
   bool open(const QString &db,
      const QString &user,
      const QString &password,
      const QString &host,
      int port) {
      return open (db, user, password, host, port, QString());
   }
   void close();
   QSqlResult *createResult() const;
   bool beginTransaction();
   bool commitTransaction();
   bool rollbackTransaction();
   QStringList tables(QSql::TableType) const;

   QSqlRecord record(const QString &tablename) const;
   QSqlIndex primaryIndex(const QString &table) const;

   QString formatValue(const QSqlField &field, bool trimStrings) const;
   QVariant handle() const;

   QString escapeIdentifier(const QString &identifier, IdentifierType type) const;

 protected:
   bool subscribeToNotificationImplementation(const QString &name);
   bool unsubscribeFromNotificationImplementation(const QString &name);
   QStringList subscribedToNotificationsImplementation();

 private:
   SQL_CS_SLOT_1(Private, void qHandleEventNotification(void *updatedResultBuffer))
   SQL_CS_SLOT_2(qHandleEventNotification)

   QIBaseDriverPrivate *d;
};

#endif // QSQL_IBASE_H
