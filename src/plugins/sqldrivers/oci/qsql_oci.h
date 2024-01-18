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

#ifndef QSQL_OCI_H
#define QSQL_OCI_H

#include <qsqlresult.h>
#include <qsqldriver.h>
#include <qsqlcachedresult_p.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_OCI
#else
#define Q_EXPORT_SQLDRIVER_OCI Q_SQL_EXPORT
#endif

typedef struct OCIEnv OCIEnv;
typedef struct OCISvcCtx OCISvcCtx;

class QOCIDriver;
class QOCICols;
struct QOCIDriverPrivate;
struct QOCIResultPrivate;

class Q_EXPORT_SQLDRIVER_OCI QOCIResult : public QSqlCachedResult
{
   friend class QOCIDriver;
   friend struct QOCIResultPrivate;
   friend class QOCICols;
 public:
   QOCIResult(const QOCIDriver *db, const QOCIDriverPrivate *p);
   ~QOCIResult();
   bool prepare(const QString &query);
   bool exec();
   QVariant handle() const;

 protected:
   bool gotoNext(ValueCache &values, int index);
   bool reset (const QString &query);
   int size();
   int numRowsAffected();
   QSqlRecord record() const;
   QVariant lastInsertId() const;
   void virtual_hook(int id, void *data);

 private:
   QOCIResultPrivate *d;
};

class Q_EXPORT_SQLDRIVER_OCI QOCIDriver : public QSqlDriver
{
   SQL_CS_OBJECT(QOCIDriver)

   friend struct QOCIResultPrivate;
   friend class QOCIPrivate;

 public:
   explicit QOCIDriver(QObject *parent = nullptr);
   QOCIDriver(OCIEnv *env, OCISvcCtx *ctx, QObject *parent = nullptr);
   ~QOCIDriver();
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
   QSqlRecord record(const QString &tablename) const;
   QSqlIndex primaryIndex(const QString &tablename) const;
   QString formatValue(const QSqlField &field,
      bool trimStrings) const;
   QVariant handle() const;
   QString escapeIdentifier(const QString &identifier, IdentifierType) const;

 protected:
   bool                beginTransaction();
   bool                commitTransaction();
   bool                rollbackTransaction();

 private:
   QOCIDriverPrivate *d;
};

#endif // QSQL_OCI_H
