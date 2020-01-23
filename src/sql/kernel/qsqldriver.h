/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QSQLDRIVER_H
#define QSQLDRIVER_H

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtSql/qsql.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QSqlDatabase;
class QSqlDriverPrivate;
class QSqlError;
class QSqlField;
class QSqlIndex;
class QSqlRecord;
class QSqlResult;
class QVariant;

class Q_SQL_EXPORT QSqlDriver : public QObject
{
   SQL_CS_OBJECT(QSqlDriver)
   Q_DECLARE_PRIVATE(QSqlDriver)

 public:
   enum DriverFeature { Transactions, QuerySize, BLOB, Unicode, PreparedQueries,
      NamedPlaceholders, PositionalPlaceholders, LastInsertId,
      BatchOperations, SimpleLocking, LowPrecisionNumbers,
      EventNotifications, FinishQuery, MultipleResultSets, CancelQuery
   };

   enum StatementType { WhereStatement, SelectStatement, UpdateStatement,
      InsertStatement, DeleteStatement
   };

   enum IdentifierType { FieldName, TableName };

   enum NotificationSource { UnknownSource, SelfSource, OtherSource };

   enum DbmsType {
      UnknownDbms,
      MSSqlServer,
      MySqlServer,
      PostgreSQL,
      Oracle,
      Sybase,
      SQLite,
      Interbase,
      DB2
   };
   explicit QSqlDriver(QObject *parent = nullptr);
   ~QSqlDriver();
   virtual bool isOpen() const;
   bool isOpenError() const;

   virtual bool beginTransaction();
   virtual bool commitTransaction();
   virtual bool rollbackTransaction();
   virtual QStringList tables(QSql::TableType tableType) const;
   virtual QSqlIndex primaryIndex(const QString &tableName) const;
   virtual QSqlRecord record(const QString &tableName) const;

   virtual QString formatValue(const QSqlField &field, bool trimStrings = false) const;

   virtual QString escapeIdentifier(const QString &identifier, IdentifierType type) const;
   virtual QString sqlStatement(StatementType type, const QString &tableName,
      const QSqlRecord &rec, bool preparedStatement) const;

   QSqlError lastError() const;

   virtual QVariant handle() const;
   virtual bool hasFeature(DriverFeature f) const = 0;
   virtual void close() = 0;
   virtual QSqlResult *createResult() const = 0;

   virtual bool open(const QString &db,
      const QString &user = QString(),
      const QString &password = QString(),
      const QString &host = QString(),
      int port = -1,
      const QString &connOpts = QString()) = 0;

   virtual bool subscribeToNotification(const QString &name);
   virtual bool unsubscribeFromNotification(const QString &name);
   virtual QStringList subscribedToNotifications() const;

   virtual bool isIdentifierEscaped(const QString &identifier, IdentifierType type) const;
   virtual QString stripDelimiters(const QString &identifier, IdentifierType type) const;

   void setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy precisionPolicy);
   QSql::NumericalPrecisionPolicy numericalPrecisionPolicy() const;

   DbmsType dbmsType() const;

   SQL_CS_SLOT_1(Public, virtual bool cancelQuery())
   SQL_CS_SLOT_2(cancelQuery)
   SQL_CS_SIGNAL_1(Public, void notification(const QString &name, QSqlDriver::NotificationSource source, const QVariant &payload))
   SQL_CS_SIGNAL_2(notification, name, source, payload)

 protected:
   QSqlDriver(QSqlDriverPrivate &dd, QObject *parent = nullptr);

   virtual void setOpen(bool o);
   virtual void setOpenError(bool e);
   virtual void setLastError(const QSqlError &e);

   QScopedPointer<QSqlDriverPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QSqlDriver)

   friend class QSqlDatabase;
   friend class QSqlResultPrivate;
};

#endif
