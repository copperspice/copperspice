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

#ifndef QSQLDATABASE_H
#define QSQLDATABASE_H

#include <qstring.h>
#include <qsql.h>

class QSqlError;
class QSqlDriver;
class QSqlIndex;
class QSqlRecord;
class QSqlQuery;
class QSqlDatabasePrivate;

class Q_SQL_EXPORT QSqlDriverCreatorBase
{
 public:
   virtual ~QSqlDriverCreatorBase() {}
   virtual QSqlDriver *createObject() const = 0;
};

template <class T>
class QSqlDriverCreator : public QSqlDriverCreatorBase
{
 public:
   QSqlDriver *createObject() const override {
      return new T;
   }
};

class Q_SQL_EXPORT QSqlDatabase
{
 public:
   QSqlDatabase();
   QSqlDatabase(const QSqlDatabase &other);
   ~QSqlDatabase();

   QSqlDatabase &operator=(const QSqlDatabase &other);

   bool open();
   bool open(const QString &user, const QString &password);
   void close();
   bool isOpen() const;
   bool isOpenError() const;
   QStringList tables(QSql::TableType type = QSql::Tables) const;
   QSqlIndex primaryIndex(const QString &tablename) const;
   QSqlRecord record(const QString &tablename) const;

   QSqlQuery exec(const QString &query = QString()) const;
   QSqlError lastError() const;
   bool isValid() const;

   bool transaction();
   bool commit();
   bool rollback();

   void setDatabaseName(const QString &name);
   void setUserName(const QString &name);
   void setPassword(const QString &password);
   void setHostName(const QString &host);
   void setPort(int port);
   void setConnectOptions(const QString &options = QString());
   QString databaseName() const;
   QString userName() const;
   QString password() const;
   QString hostName() const;
   QString driverName() const;
   int port() const;
   QString connectOptions() const;
   QString connectionName() const;
   void setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy precisionPolicy);
   QSql::NumericalPrecisionPolicy numericalPrecisionPolicy() const;

   QSqlDriver *driver() const;

   static QString defaultConnection;

   static QSqlDatabase addDatabase(const QString &type, const QString &connectionName = defaultConnection);
   static QSqlDatabase addDatabase(QSqlDriver *driver,  const QString &connectionName = defaultConnection);
   static QSqlDatabase cloneDatabase(const QSqlDatabase &other, const QString &connectionName);
   static QSqlDatabase database(const QString &connectionName = defaultConnection, bool open = true);

   static void removeDatabase(const QString &connectionName);
   static bool contains(const QString &connectionName = defaultConnection);
   static QStringList drivers();
   static QStringList connectionNames();
   static void registerSqlDriver(const QString &name, QSqlDriverCreatorBase *creator);
   static bool isDriverAvailable(const QString &name);

 protected:
   explicit QSqlDatabase(const QString &type);
   explicit QSqlDatabase(QSqlDriver *driver);

 private:
   friend class QSqlDatabasePrivate;
   QSqlDatabasePrivate *d;
};

Q_SQL_EXPORT QDebug operator<<(QDebug, const QSqlDatabase &);

#endif
