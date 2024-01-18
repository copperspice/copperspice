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

#ifndef QSQL_TDS_H
#define QSQL_TDS_H

#include <qsqlresult.h>
#include <qsqldriver.h>
#include <qsqlcachedresult_p.h>

#ifdef Q_OS_WIN32
#define WIN32_LEAN_AND_MEAN

#ifndef Q_USE_SYBASE
#define DBNTWIN32 // indicates 32bit windows dblib
#endif

#include <winsock2.h>
#include <qt_windows.h>
#include <sqlfront.h>
#include <sqldb.h>
#define CS_PUBLIC

#else
#include <sybfront.h>
#include <sybdb.h>
#endif //Q_OS_WIN32

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_TDS
#else
#define Q_EXPORT_SQLDRIVER_TDS Q_SQL_EXPORT
#endif

class QTDSDriverPrivate;
class QTDSResultPrivate;
class QTDSDriver;

class QTDSResult : public QSqlCachedResult
{
 public:
   explicit QTDSResult(const QTDSDriver *db);
   ~QTDSResult();
   QVariant handle() const;

 protected:
   void cleanup();
   bool reset (const QString &query);
   int size();
   int numRowsAffected();
   bool gotoNext(QSqlCachedResult::ValueCache &values, int index);
   QSqlRecord record() const;

 private:
   QTDSResultPrivate *d;
};

class Q_EXPORT_SQLDRIVER_TDS QTDSDriver : public QSqlDriver
{
   CS_OBJECT(QTDSDriver)
   friend class QTDSResult;

 public:
   explicit QTDSDriver(QObject *parent = nullptr);
   QTDSDriver(LOGINREC *rec, const QString &host, const QString &db, QObject *parent = nullptr);
   ~QTDSDriver();
   bool hasFeature(DriverFeature f) const;
   bool open(const QString &db,
      const QString &user,
      const QString &password,
      const QString &host,
      int port,
      const QString &connOpts);
   void close();
   QStringList tables(QSql::TableType) const;
   QSqlResult *createResult() const;
   QSqlRecord record(const QString &tablename) const;
   QSqlIndex primaryIndex(const QString &tablename) const;

   QString formatValue(const QSqlField &field,
      bool trimStrings) const;
   QVariant handle() const;

   QString escapeIdentifier(const QString &identifier, IdentifierType type) const;

 protected:
   bool beginTransaction();
   bool commitTransaction();
   bool rollbackTransaction();
 private:
   void init();
   QTDSDriverPrivate *d;
};

#endif
