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

#ifndef QSQL_SQLITE2_H
#define QSQL_SQLITE2_H

#include <QtSql/qsqldriver.h>
#include <QtSql/qsqlresult.h>
#include <QtSql/qsqlrecord.h>
#include <QtSql/qsqlindex.h>
#include <QtSql/qsqlcachedresult_p.h>

#if defined (Q_OS_WIN32)
# include <QtCore/qt_windows.h>
#endif

struct sqlite;

QT_BEGIN_NAMESPACE

class QSQLite2DriverPrivate;
class QSQLite2ResultPrivate;
class QSQLite2Driver;

class QSQLite2Result : public QSqlCachedResult
{
   friend class QSQLite2Driver;
   friend class QSQLite2ResultPrivate;

 public:
   explicit QSQLite2Result(const QSQLite2Driver *db);
   ~QSQLite2Result();
   QVariant handle() const;

 protected:
   bool gotoNext(QSqlCachedResult::ValueCache &row, int idx);
   bool reset (const QString &query);
   int size();
   int numRowsAffected();
   QSqlRecord record() const;
   void virtual_hook(int id, void *data);

 private:
   QSQLite2ResultPrivate *d;
};

class QSQLite2Driver : public QSqlDriver
{
   CS_OBJECT(QSQLite2Driver)
   friend class QSQLite2Result;

 public:
   explicit QSQLite2Driver(QObject *parent = nullptr);
   explicit QSQLite2Driver(sqlite *connection, QObject *parent = nullptr);
   ~QSQLite2Driver();
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
   QVariant handle() const;
   QString escapeIdentifier(const QString &identifier, IdentifierType) const;

 private:
   QSQLite2DriverPrivate *d;
};

QT_END_NAMESPACE

#endif // QSQL_SQLITE2_H
