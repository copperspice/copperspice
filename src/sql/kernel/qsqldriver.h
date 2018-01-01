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
                        EventNotifications, FinishQuery, MultipleResultSets
                      };

   enum StatementType { WhereStatement, SelectStatement, UpdateStatement,
                        InsertStatement, DeleteStatement
                      };

   enum IdentifierType { FieldName, TableName };

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
   bool subscribeToNotification(const QString &name);	    // ### Qt5/make virtual
   bool unsubscribeFromNotification(const QString &name);  // ### Qt5/make virtual
   QStringList subscribedToNotifications() const;          // ### Qt5/make virtual

   bool isIdentifierEscaped(const QString &identifier, IdentifierType type) const; // ### Qt5/make virtual
   QString stripDelimiters(const QString &identifier, IdentifierType type) const;  // ### Qt5/make virtual

   void setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy precisionPolicy);
   QSql::NumericalPrecisionPolicy numericalPrecisionPolicy() const;

   SQL_CS_SIGNAL_1(Public, void notification(const QString &name))
   SQL_CS_SIGNAL_2(notification, name)

 protected:
   virtual void setOpen(bool o);
   virtual void setOpenError(bool e);
   virtual void setLastError(const QSqlError &e);

   virtual bool subscribeToNotificationImplementation(const QString &name);
   virtual bool unsubscribeFromNotificationImplementation(const QString &name);
   virtual QStringList subscribedToNotificationsImplementation() const;
   virtual bool isIdentifierEscapedImplementation(const QString &identifier, IdentifierType type) const;
   virtual QString stripDelimitersImplementation(const QString &identifier, IdentifierType type) const;

   QScopedPointer<QSqlDriverPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QSqlDriver)

   friend class QSqlDatabase;

};

QT_END_NAMESPACE

#endif // QSQLDRIVER_H
