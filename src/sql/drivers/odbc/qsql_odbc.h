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

#ifndef QSQL_ODBC_H
#define QSQL_ODBC_H

#include <QtSql/qsqldriver.h>
#include <QtSql/qsqlresult.h>

#if defined (Q_OS_WIN32)
#include <QtCore/qt_windows.h>
#endif

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_ODBC
#else
#define Q_EXPORT_SQLDRIVER_ODBC Q_SQL_EXPORT
#endif

#ifdef Q_OS_UNIX
#define HAVE_LONG_LONG 1 // force UnixODBC NOT to fall back to a struct for BIGINTs
#endif

#include <sql.h>
#include <sqlext.h>


QT_BEGIN_NAMESPACE

class QODBCPrivate;
class QODBCDriverPrivate;
class QODBCDriver;
class QSqlRecordInfo;

class QODBCResult : public QSqlResult
{
 public:
   QODBCResult(const QODBCDriver *db, QODBCDriverPrivate *p);
   virtual ~QODBCResult();

   bool prepare(const QString &query);
   bool exec();

   QVariant handle() const;
   virtual void setForwardOnly(bool forward);

 protected:
   bool fetchNext();
   bool fetchFirst();
   bool fetchLast();
   bool fetchPrevious();
   bool fetch(int i);
   bool reset (const QString &query);
   QVariant data(int field);
   bool isNull(int field);
   int size();
   int numRowsAffected();
   QSqlRecord record() const;
   void virtual_hook(int id, void *data);
   bool nextResult();

 private:
   QODBCPrivate *d;
};

class Q_EXPORT_SQLDRIVER_ODBC QODBCDriver : public QSqlDriver
{
   CS_OBJECT(QODBCDriver)

 public:
   explicit QODBCDriver(QObject *parent = nullptr);
   QODBCDriver(SQLHANDLE env, SQLHANDLE con, QObject *parent = nullptr);
   virtual ~QODBCDriver();
   bool hasFeature(DriverFeature f) const;
   void close();
   QSqlResult *createResult() const;
   QStringList tables(QSql::TableType) const;
   QSqlRecord record(const QString &tablename) const;
   QSqlIndex primaryIndex(const QString &tablename) const;
   QVariant handle() const;
   QString formatValue(const QSqlField &field,
                       bool trimStrings) const;
   bool open(const QString &db,
             const QString &user,
             const QString &password,
             const QString &host,
             int port,
             const QString &connOpts);

   QString escapeIdentifier(const QString &identifier, IdentifierType type) const;

 protected :
   isIdentifierEscapedImplementation(const QString &identifier, IdentifierType type) const;

   bool beginTransaction();
   bool commitTransaction();
   bool rollbackTransaction();

 private:
   void init();
   bool endTrans();
   void cleanup();

   QODBCDriverPrivate *d;
   friend class QODBCPrivate;
};

QT_END_NAMESPACE


#endif // QSQL_ODBC_H
