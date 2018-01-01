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

#ifndef QSQL_MYSQL_H
#define QSQL_MYSQL_H

#include <QtSql/qsqldriver.h>
#include <QtSql/qsqlresult.h>

#if defined (Q_OS_WIN32)
#include <QtCore/qt_windows.h>
#endif

#include <mysql.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_MYSQL
#else
#define Q_EXPORT_SQLDRIVER_MYSQL Q_SQL_EXPORT
#endif

QT_BEGIN_NAMESPACE

class QMYSQLDriverPrivate;
class QMYSQLResultPrivate;
class QMYSQLDriver;
class QSqlRecordInfo;

class QMYSQLResult : public QSqlResult
{
   friend class QMYSQLDriver;
   friend class QMYSQLResultPrivate;

 public:
   explicit QMYSQLResult(const QMYSQLDriver *db);
   ~QMYSQLResult();

   QVariant handle() const;
 protected:
   void cleanup();
   bool fetch(int i);
   bool fetchNext();
   bool fetchLast();
   bool fetchFirst();
   QVariant data(int field);
   bool isNull(int field);
   bool reset (const QString &query);
   int size();
   int numRowsAffected();
   QVariant lastInsertId() const;
   QSqlRecord record() const;
   void virtual_hook(int id, void *data);
   bool nextResult();

#if MYSQL_VERSION_ID >= 40108
   bool prepare(const QString &stmt);
   bool exec();
#endif
 private:
   QMYSQLResultPrivate *d;
};

class Q_EXPORT_SQLDRIVER_MYSQL QMYSQLDriver : public QSqlDriver
{
   CS_OBJECT(QMYSQLDriver)
   friend class QMYSQLResult;

 public:
   explicit QMYSQLDriver(QObject *parent = nullptr);
   explicit QMYSQLDriver(MYSQL *con, QObject *parent = nullptr);
   ~QMYSQLDriver();

   bool hasFeature(DriverFeature f) const;
   bool open(const QString &db,
             const QString &user,
             const QString &password,
             const QString &host,
             int port,
             const QString &connOpts);
   void close();

   QSqlResult *createResult() const;
   QStringList tables(QSql::TableType) const;
   QSqlIndex primaryIndex(const QString &tablename) const;
   QSqlRecord record(const QString &tablename) const;
   QString formatValue(const QSqlField &field,
                       bool trimStrings) const;
   QVariant handle() const;
   QString escapeIdentifier(const QString &identifier, IdentifierType type) const;

 protected :
   bool isIdentifierEscapedImplementation(const QString &identifier, IdentifierType type) const;

   bool beginTransaction();
   bool commitTransaction();
   bool rollbackTransaction();

 private:
   void init();
   QMYSQLDriverPrivate *d;
};

QT_END_NAMESPACE

#endif // QSQL_MYSQL_H
