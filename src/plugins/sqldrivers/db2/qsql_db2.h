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

#ifndef QSQL_DB2_H
#define QSQL_DB2_H

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_DB2
#else
#define Q_EXPORT_SQLDRIVER_DB2 Q_SQL_EXPORT
#endif

#include <qsqlresult.h>
#include <qsqldriver.h>

class QDB2Driver;
class QDB2DriverPrivate;
class QDB2ResultPrivate;
class QSqlRecord;

class QDB2Result : public QSqlResult
{

 public:
   QDB2Result(const QDB2Driver *dr, const QDB2DriverPrivate *dp);
   ~QDB2Result();
   bool prepare(const QString &query);
   bool exec();
   QVariant handle() const;

 protected:
   QVariant data(int field);
   bool reset (const QString &query);
   bool fetch(int i);
   bool fetchNext();
   bool fetchFirst();
   bool fetchLast();
   bool isNull(int i);
   int size();
   int numRowsAffected();
   QSqlRecord record() const;
   void virtual_hook(int id, void *data);
   bool nextResult();

 private:
   QDB2ResultPrivate *d;
};

class Q_EXPORT_SQLDRIVER_DB2 QDB2Driver : public QSqlDriver
{
   CS_OBJECT(QDB2Driver)

 public:
   explicit QDB2Driver(QObject *parent = nullptr);
   QDB2Driver(Qt::HANDLE env, Qt::HANDLE con, QObject *parent = nullptr);
   ~QDB2Driver();
   bool hasFeature(DriverFeature) const;
   void close();
   QSqlRecord record(const QString &tableName) const;
   QStringList tables(QSql::TableType type) const;
   QSqlResult *createResult() const;
   QSqlIndex primaryIndex(const QString &tablename) const;
   bool beginTransaction();
   bool commitTransaction();
   bool rollbackTransaction();
   QString formatValue(const QSqlField &field, bool trimStrings) const;
   QVariant handle() const;
   bool open(const QString &db,
      const QString &user,
      const QString &password,
      const QString &host,
      int port,
      const QString &connOpts);
   QString escapeIdentifier(const QString &identifier, IdentifierType type) const;

 private:
   bool setAutoCommit(bool autoCommit);
   QDB2DriverPrivate *d;
};

#endif
