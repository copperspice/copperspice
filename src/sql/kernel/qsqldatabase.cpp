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

#include <qsqldatabase.h>

#include <qalgorithms.h>
#include <qsqlquery.h>

#ifdef Q_OS_WIN
// Conflicting declarations of LPCBYTE in sqlfront.h and winscard.h
#define _WINSCARD_H_
#endif

#ifdef QT_SQL_PSQL
#include "../../plugins/sqldrivers/psql/qsql_psql.h"
#endif

#ifdef QT_SQL_MYSQL
#include "./../plugins/sqldrivers/mysql/qsql_mysql.h"
#endif

#ifdef QT_SQL_ODBC
#include "./../plugins/sqldrivers/odbc/qsql_odbc.h"
#endif

#ifdef QT_SQL_OCI
#include "./../plugins/sqldrivers/oci/qsql_oci.h"
#endif

#ifdef QT_SQL_TDS
// conflicting RETCODE typedef between odbc and freetds

#define RETCODE DBRETCODE
#include "./../plugins/sqldrivers/tds/qsql_tds.h"
#undef RETCODE
#endif

#ifdef QT_SQL_DB2
#include "./../plugins/sqldrivers/db2/qsql_db2.h"
#endif

#ifdef QT_SQL_SQLITE
#include "../../plugins/sqldrivers/sqlite/qsql_sqlite.h"
#endif

#ifdef QT_SQL_SQLITE2
#include "./../plugins/sqldrivers/sqlite2/qsql_sqlite2.h"
#endif

#ifdef QT_SQL_IBASE
#undef SQL_FLOAT              // avoid clash with ODBC
#undef SQL_DOUBLE
#undef SQL_TIMESTAMP
#undef SQL_TYPE_TIME
#undef SQL_TYPE_DATE
#undef SQL_DATE

#define SCHAR IBASE_SCHAR
#include "../drivers/ibase/qsql_ibase.h"
#undef SCHAR
#endif

#include <qdebug.h>
#include <qcoreapplication.h>
#include <qreadwritelock.h>
#include <qsqlresult.h>
#include <qsqldriver.h>
#include <qsqldriverplugin.h>
#include <qsqlindex.h>
#include <qfactoryloader_p.h>
#include <qsqlnulldriver_p.h>
#include <qmutex.h>
#include <qhash.h>

#include <stdlib.h>

static QFactoryLoader *loader()
{
   static QFactoryLoader retval(QSqlDriverInterface_ID, "/sqldrivers");
   return &retval;
}

QString QSqlDatabase::defaultConnection = "qt_sql_default_connection";

static bool s_hashCleared = true;

class QConnectionDict: public QHash<QString, QSqlDatabase>
{
 public:
   bool contains_ts(const QString &key) {
      QReadLocker locker(&lock);
      return contains(key);
   }

   QStringList keys_ts() const {
      QReadLocker locker(&lock);
      return keys();
   }

   mutable QReadWriteLock lock;
};

static QConnectionDict *dbDict()
{
   static QConnectionDict retval;
   return &retval;
}

class QSqlDatabasePrivate
{
 public:
   QSqlDatabasePrivate(QSqlDatabase *d, QSqlDriver *dr = nullptr):
      ref(1), q(d), driver(dr), port(-1) {

      precisionPolicy = QSql::LowPrecisionDouble;
   }

   QSqlDatabasePrivate(const QSqlDatabasePrivate &other);
   ~QSqlDatabasePrivate();

   void init(const QString &type);
   void copy(const QSqlDatabasePrivate *other);
   void disable();

   QAtomicInt ref;
   QSqlDatabase *q;
   QSqlDriver *driver;
   QString dbname;
   QString uname;
   QString pword;
   QString hname;
   QString drvName;
   int port;
   QString connOptions;
   QString connName;
   QSql::NumericalPrecisionPolicy precisionPolicy;

   static QSqlDatabasePrivate *shared_null();
   static QSqlDatabase database(const QString &name, bool open);
   static void addDatabase(const QSqlDatabase &db, const QString &name);
   static void removeDatabase(const QString &name);
   static void invalidateDb(const QSqlDatabase &db, const QString &name, bool doWarn = true);
   static QHash<QString, QSqlDriverCreatorBase *> &driverDict();
   static void cleanConnections();
};

QSqlDatabasePrivate::QSqlDatabasePrivate(const QSqlDatabasePrivate &other) : ref(1)
{
   q = other.q;

   dbname  = other.dbname;
   uname   = other.uname;
   pword   = other.pword;
   hname   = other.hname;
   drvName = other.drvName;
   port    = other.port;

   connOptions     = other.connOptions;
   driver          = other.driver;
   precisionPolicy = other.precisionPolicy;
}

QSqlDatabasePrivate::~QSqlDatabasePrivate()
{
   if (driver != shared_null()->driver) {
      delete driver;
   }
}

void QSqlDatabasePrivate::cleanConnections()
{
   QConnectionDict *dict = dbDict();
   Q_ASSERT(dict);

   QWriteLocker locker(&dict->lock);
   QConnectionDict::iterator it = dict->begin();

   while (it != dict->end()) {
      invalidateDb(it.value(), it.key(), false);
      ++it;
   }

   dict->clear();
}

static void cleanDriverDict()
{
   qDeleteAll(QSqlDatabasePrivate::driverDict());
   QSqlDatabasePrivate::driverDict().clear();
   QSqlDatabasePrivate::cleanConnections();

   s_hashCleared = true;
}

QHash<QString, QSqlDriverCreatorBase *> &QSqlDatabasePrivate::driverDict()
{
   static QHash<QString, QSqlDriverCreatorBase *> dict;

   if (s_hashCleared ) {
      s_hashCleared  = false;
      qAddPostRoutine(cleanDriverDict);
   }

   return dict;
}

QSqlDatabasePrivate *QSqlDatabasePrivate::shared_null()
{
   static QSqlNullDriver dr;
   static QSqlDatabasePrivate n(nullptr, &dr);

   return &n;
}

void QSqlDatabasePrivate::invalidateDb(const QSqlDatabase &db, const QString &name, bool doWarn)
{
   if (db.d->ref.load() != 1 && doWarn) {
      qWarning("QSqlDatabase::invalidateDb() Connection '%s' is still in use, all queries about to fail", csPrintable(name));
      db.d->disable();
      db.d->connName.clear();
   }
}

void QSqlDatabasePrivate::removeDatabase(const QString &name)
{
   QConnectionDict *dict = dbDict();
   Q_ASSERT(dict);

   QWriteLocker locker(&dict->lock);

   if (! dict->contains(name)) {
      return;
   }

   invalidateDb(dict->take(name), name);
}

void QSqlDatabasePrivate::addDatabase(const QSqlDatabase &db, const QString &name)
{
   QConnectionDict *dict = dbDict();
   Q_ASSERT(dict);

   QWriteLocker locker(&dict->lock);

   if (dict->contains(name)) {
      invalidateDb(dict->take(name), name);

      qWarning("QSqlDatabase::addDatabase() Duplicate connection name '%s', old connection removed", csPrintable(name));
   }

   dict->insert(name, db);
   db.d->connName = name;
}

QSqlDatabase QSqlDatabasePrivate::database(const QString &name, bool open)
{
   const QConnectionDict *dict = dbDict();
   Q_ASSERT(dict);

   dict->lock.lockForRead();
   QSqlDatabase db = dict->value(name);
   dict->lock.unlock();

   if (db.isValid() && ! db.isOpen() && open) {
      if (! db.open()) {
         qWarning() << "QSqlDatabase::database() Unable to open database:" << db.lastError().text();
      }

   }
   return db;
}

void QSqlDatabasePrivate::copy(const QSqlDatabasePrivate *other)
{
   q = other->q;
   dbname   = other->dbname;
   uname    = other->uname;
   pword    = other->pword;
   hname    = other->hname;
   drvName  = other->drvName;
   port     = other->port;
   connOptions = other->connOptions;
   precisionPolicy = other->precisionPolicy;
}

void QSqlDatabasePrivate::disable()
{
   if (driver != shared_null()->driver) {
      delete driver;
      driver = shared_null()->driver;
   }
}

QSqlDatabase QSqlDatabase::addDatabase(const QString &type, const QString &connectionName)
{
   QSqlDatabase db(type);
   QSqlDatabasePrivate::addDatabase(db, connectionName);
   return db;
}

QSqlDatabase QSqlDatabase::database(const QString &connectionName, bool open)
{
   return QSqlDatabasePrivate::database(connectionName, open);
}

void QSqlDatabase::removeDatabase(const QString &connectionName)
{
   QSqlDatabasePrivate::removeDatabase(connectionName);
}

QStringList QSqlDatabase::drivers()
{
   QStringList list;

#ifdef QT_SQL_PSQL
   list << "QPSQL";
   // list << "QPSQL7";
#endif

#ifdef QT_SQL_MYSQL
   list << "QMYSQL";
   // list << "QMYSQL3";
#endif

#ifdef QT_SQL_ODBC
   list << "QODBC"
   // list << "QODBC3";
#endif

#ifdef QT_SQL_OCI
   list << "QOCI"
   // list << "QOCI8";
#endif

#ifdef QT_SQL_TDS
   list << "QTDS"
   // list << "QTDS7";
#endif

#ifdef QT_SQL_DB2
   list << "QDB2";
#endif

#ifdef QT_SQL_SQLITE
   list << "QSQLITE";
#endif

#ifdef QT_SQL_SQLITE2
   list << "QSQLITE2";
#endif

#ifdef QT_SQL_IBASE
   list << "QIBASE";
#endif

   QFactoryLoader *factoryObj = loader();

   if (factoryObj != nullptr) {
      // what keys are available
      const QSet<QString> keySet = factoryObj->keySet();

      for (auto item : keySet) {
         if (! list.contains(item)) {
            list.append(item);
         }
      }
   }

   QHash<QString, QSqlDriverCreatorBase *> dict = QSqlDatabasePrivate::driverDict();

   for (auto i = dict.constBegin(); i != dict.constEnd(); ++i) {
      if (! list.contains(i.key())) {
         list.append(i.key());
      }
   }

   return list;
}

void QSqlDatabase::registerSqlDriver(const QString &name, QSqlDriverCreatorBase *creator)
{
   delete QSqlDatabasePrivate::driverDict().take(name);

   if (creator) {
      QSqlDatabasePrivate::driverDict().insert(name, creator);
   }
}

bool QSqlDatabase::contains(const QString &connectionName)
{
   return dbDict()->contains_ts(connectionName);
}

QStringList QSqlDatabase::connectionNames()
{
   return dbDict()->keys_ts();
}

QSqlDatabase::QSqlDatabase(const QString &type)
{
   d = new QSqlDatabasePrivate(this);
   d->init(type);
}

QSqlDatabase::QSqlDatabase(QSqlDriver *driver)
{
   d = new QSqlDatabasePrivate(this, driver);
}

QSqlDatabase::QSqlDatabase()
{
   d = QSqlDatabasePrivate::shared_null();
   d->ref.ref();
}

QSqlDatabase::QSqlDatabase(const QSqlDatabase &other)
{
   d = other.d;
   d->ref.ref();
}

QSqlDatabase &QSqlDatabase::operator=(const QSqlDatabase &other)
{
   qAtomicAssign(d, other.d);
   return *this;
}

void QSqlDatabasePrivate::init(const QString &type)
{
   drvName = type;

   if (! driver) {

#ifdef QT_SQL_PSQL
      if (type == "QPSQL") {
         driver = new QPSQLDriver();
      }
#endif

#ifdef QT_SQL_MYSQL
      if (type == "QMYSQL") {
         driver = new QMYSQLDriver();
      }
#endif

#ifdef QT_SQL_ODBC
      if (type == "QODBC") {
         driver = new QODBCDriver();
      }
#endif

#ifdef QT_SQL_OCI
      if (type == "QOCI") {
         driver = new QOCIDriver();
      }
#endif

#ifdef QT_SQL_TDS
      if (type == "QTDS") {
         driver = new QTDSDriver();
      }
#endif

#ifdef QT_SQL_DB2
      if (type == "QDB2") {
         driver = new QDB2Driver();
      }
#endif

#ifdef QT_SQL_SQLITE
      if (type == "QSQLITE") {
         driver = new QSQLiteDriver();
      }
#endif

#ifdef QT_SQL_SQLITE2
      if (type == "QSQLITE2") {
         driver = new QSQLite2Driver();
      }
#endif

#ifdef QT_SQL_IBASE
      if (type == "QIBASE") {
         driver = new QIBaseDriver();
      }
#endif

   }

   if (! driver) {
      QHash<QString, QSqlDriverCreatorBase *> dict = QSqlDatabasePrivate::driverDict();

      for (auto it = dict.constBegin(); it != dict.constEnd() && ! driver; ++it) {
         if (type == it.key()) {
            driver = ((QSqlDriverCreatorBase *)(*it))->createObject();
         }
      }
   }

   if (! driver) {
      // does the work to call keyset() in QFactoryLoader
      QSqlDatabase::drivers();

      driver = cs_load_plugin<QSqlDriver, QSqlDriverPlugin>(loader(), type);
   }

   if (! driver) {
      qWarning("QSqlDatabase::init() %s driver not loaded", csPrintable(type));
      qWarning("QSqlDatabase::init() Available drivers: %s", QSqlDatabase::drivers().join(" ").toLatin1().data());

      if (QCoreApplication::instance() == nullptr) {
         qWarning("QSqlDatabase::init() QCoreApplication must be started before loading an SQL driver plugin");
      }

      driver = shared_null()->driver;
   }
}

QSqlDatabase::~QSqlDatabase()
{
   if (!d->ref.deref()) {
      close();
      delete d;
   }
}

QSqlQuery QSqlDatabase::exec(const QString &query) const
{
   QSqlQuery r(d->driver->createResult());
   if (!query.isEmpty()) {
      r.exec(query);
      d->driver->setLastError(r.lastError());
   }
   return r;
}

bool QSqlDatabase::open()
{
   return d->driver->open(d->dbname, d->uname, d->pword, d->hname, d->port, d->connOptions);
}

bool QSqlDatabase::open(const QString &user, const QString &password)
{
   setUserName(user);
   return d->driver->open(d->dbname, user, password, d->hname, d->port, d->connOptions);
}

void QSqlDatabase::close()
{
   d->driver->close();
}

bool QSqlDatabase::isOpen() const
{
   return d->driver->isOpen();
}

bool QSqlDatabase::isOpenError() const
{
   return d->driver->isOpenError();
}

bool QSqlDatabase::transaction()
{
   if (! d->driver->hasFeature(QSqlDriver::Transactions)) {
      return false;
   }

   return d->driver->beginTransaction();
}

bool QSqlDatabase::commit()
{
   if (!d->driver->hasFeature(QSqlDriver::Transactions)) {
      return false;
   }
   return d->driver->commitTransaction();
}

bool QSqlDatabase::rollback()
{
   if (!d->driver->hasFeature(QSqlDriver::Transactions)) {
      return false;
   }
   return d->driver->rollbackTransaction();
}

void QSqlDatabase::setDatabaseName(const QString &name)
{
   if (isValid()) {
      d->dbname = name;
   }
}

void QSqlDatabase::setUserName(const QString &name)
{
   if (isValid()) {
      d->uname = name;
   }
}

void QSqlDatabase::setPassword(const QString &password)
{
   if (isValid()) {
      d->pword = password;
   }
}

void QSqlDatabase::setHostName(const QString &host)
{
   if (isValid()) {
      d->hname = host;
   }
}

void QSqlDatabase::setPort(int port)
{
   if (isValid()) {
      d->port = port;
   }
}

QString QSqlDatabase::databaseName() const
{
   return d->dbname;
}

QString QSqlDatabase::userName() const
{
   return d->uname;
}

QString QSqlDatabase::password() const
{
   return d->pword;
}

QString QSqlDatabase::hostName() const
{
   return d->hname;
}

QString QSqlDatabase::driverName() const
{
   return d->drvName;
}

int QSqlDatabase::port() const
{
   return d->port;
}

QSqlDriver *QSqlDatabase::driver() const
{
   return d->driver;
}

QSqlError QSqlDatabase::lastError() const
{
   return d->driver->lastError();
}

QStringList QSqlDatabase::tables(QSql::TableType type) const
{
   return d->driver->tables(type);
}

QSqlIndex QSqlDatabase::primaryIndex(const QString &tablename) const
{
   return d->driver->primaryIndex(tablename);
}

QSqlRecord QSqlDatabase::record(const QString &tablename) const
{
   return d->driver->record(tablename);
}

void QSqlDatabase::setConnectOptions(const QString &options)
{
   if (isValid()) {
      d->connOptions = options;
   }
}

QString QSqlDatabase::connectOptions() const
{
   return d->connOptions;
}

bool QSqlDatabase::isDriverAvailable(const QString &name)
{
   return drivers().contains(name);
}

QSqlDatabase QSqlDatabase::addDatabase(QSqlDriver *driver, const QString &connectionName)
{
   QSqlDatabase db(driver);
   QSqlDatabasePrivate::addDatabase(db, connectionName);
   return db;
}

bool QSqlDatabase::isValid() const
{
   return d->driver && d->driver != d->shared_null()->driver;
}

QSqlDatabase QSqlDatabase::cloneDatabase(const QSqlDatabase &other, const QString &connectionName)
{
   if (!other.isValid()) {
      return QSqlDatabase();
   }

   QSqlDatabase db(other.driverName());
   db.d->copy(other.d);
   QSqlDatabasePrivate::addDatabase(db, connectionName);
   return db;
}

QString QSqlDatabase::connectionName() const
{
   return d->connName;
}

void QSqlDatabase::setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy precisionPolicy)
{
   if (driver()) {
      driver()->setNumericalPrecisionPolicy(precisionPolicy);
   }
   d->precisionPolicy = precisionPolicy;
}

QSql::NumericalPrecisionPolicy QSqlDatabase::numericalPrecisionPolicy() const
{
   if (driver()) {
      return driver()->numericalPrecisionPolicy();
   } else {
      return d->precisionPolicy;
   }
}

QDebug operator<<(QDebug dbg, const QSqlDatabase &d)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   dbg.noquote();

   if (!d.isValid()) {
      dbg << "QSqlDatabase(invalid)";
      return dbg;
   }

   dbg << "QSqlDatabase(driver=\"" << d.driverName() << "\", database=\""
      << d.databaseName() << "\", host=\"" << d.hostName() << "\", port=" << d.port()
      << ", user=\"" << d.userName() << "\", open=" << d.isOpen() << ')';
   return dbg;
}

