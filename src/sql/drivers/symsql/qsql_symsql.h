/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSQL_SYMSQL_H
#define QSQL_SYMSQL_H

#include <QtSql/qsqldriver.h>
#include <QtSql/qsqlresult.h>
#include <QtSql/qsqlcachedresult_p.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_SYMSQL
#else
#define Q_EXPORT_SQLDRIVER_SYMSQL Q_SQL_EXPORT
#endif

QT_BEGIN_NAMESPACE
class QSymSQLDriverPrivate;
class QSymSQLResultPrivate;
class QSymSQLDriver;
class RSqlDatabase;

class QSymSQLResult : public QSqlResult
{
    friend class QSymSQLDriver;
    friend class QSymSQLResultPrivate;

public:
    explicit QSymSQLResult(const QSymSQLDriver* db);
    ~QSymSQLResult();
    QVariant handle() const;

protected:
    QVariant data(int field);
    bool isNull(int i);
    bool fetch(int i);
    bool fetchNext();
    bool fetchPrevious();
    bool fetchFirst();
    bool fetchLast();
    
    bool reset(const QString &query);
    bool prepare(const QString &query);
    bool exec();
    int size();
    int numRowsAffected();
    QSqlRecord record() const;
    void virtual_hook(int id, void *data);
    
    QVariant lastInsertId() const;
    
private:
    QSymSQLResultPrivate* d;
};

class Q_EXPORT_SQLDRIVER_SYMSQL QSymSQLDriver : public QSqlDriver
{
    CS_OBJECT(QSymSQLDriver)
    friend class QSymSQLResult;

public:
    explicit QSymSQLDriver(QObject *parent = 0);
    explicit QSymSQLDriver(RSqlDatabase& connection, QObject *parent = 0);
    ~QSymSQLDriver();
    bool hasFeature(DriverFeature f) const;
    bool open(const QString & db,
                   const QString & user,
                   const QString & password,
                   const QString & host,
                   int port,
                   const QString & connOpts);
    void close();
    QSqlResult *createResult() const;
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    QStringList tables(QSql::TableType)const;

    QSqlRecord record(const QString& tablename) const;
    QSqlIndex primaryIndex(const QString &table) const;
    QVariant handle() const;
    QString escapeIdentifier(const QString &identifier, IdentifierType) const;
    
private:
    QSymSQLDriverPrivate* d;
};

QT_END_NAMESPACE

#endif // QSQL_SYMSQL_H
