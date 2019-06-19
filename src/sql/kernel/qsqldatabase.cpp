/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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
#include <qsqlquery.h>

#ifdef Q_OS_WIN32
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

Q_GLOBAL_STATIC(QConnectionDict, dbDict)

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
      qWarning("QSqlDatabasePrivate::removeDatabase: Connection '%s' is still in use, all queries about to fail", csPrintable(name));
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

      qWarning("QSqlDatabasePrivate::addDatabase: Duplicate connection name '%s', old connection removed", csPrintable(name));
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
         qWarning() << "QSqlDatabasePrivate::database: unable to open database:" << db.lastError().text();
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
      qWarning("QSqlDatabase: %s driver not loaded", csPrintable(type));
      qWarning("QSqlDatabase: available drivers: %s", QSqlDatabase::drivers().join(" ").toLatin1().data());

      if (QCoreApplication::instance() == 0) {
         qWarning("QSqlDatabase: an instance of QCoreApplication is required to load an SQL driver plugin");
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

/*!
  Commits a transaction to the database if the driver supports
  transactions and a transaction() has been started. Returns \c{true}
  if the operation succeeded. Otherwise it returns \c{false}.

  \note For some databases, the commit will fail and return \c{false}
  if there is an \l{QSqlQuery::isActive()} {active query} using the
  database for a \c{SELECT}. Make the query \l{QSqlQuery::isActive()}
  {inactive} before doing the commit.

  Call lastError() to get information about errors.

  \sa QSqlQuery::isActive() QSqlDriver::hasFeature() rollback()
*/
bool QSqlDatabase::commit()
{
   if (!d->driver->hasFeature(QSqlDriver::Transactions)) {
      return false;
   }
   return d->driver->commitTransaction();
}

/*!
  Rolls back a transaction on the database, if the driver supports
  transactions and a transaction() has been started. Returns \c{true}
  if the operation succeeded. Otherwise it returns \c{false}.

  \note For some databases, the rollback will fail and return
  \c{false} if there is an \l{QSqlQuery::isActive()} {active query}
  using the database for a \c{SELECT}. Make the query
  \l{QSqlQuery::isActive()} {inactive} before doing the rollback.

  Call lastError() to get information about errors.

  \sa QSqlQuery::isActive() QSqlDriver::hasFeature() commit()
*/
bool QSqlDatabase::rollback()
{
   if (!d->driver->hasFeature(QSqlDriver::Transactions)) {
      return false;
   }
   return d->driver->rollbackTransaction();
}

/*!
    Sets the connection's database name to \a name. To have effect,
    the database name must be set \e{before} the connection is
    \l{open()} {opened}.  Alternatively, you can close() the
    connection, set the database name, and call open() again.  \note
    The \e{database name} is not the \e{connection name}. The
    connection name must be passed to addDatabase() at connection
    object create time.

    For the QOCI (Oracle) driver, the database name is the TNS
    Service Name.

    For the QODBC driver, the \a name can either be a DSN, a DSN
    filename (in which case the file must have a \c .dsn extension),
    or a connection string.

    For example, Microsoft Access users can use the following
    connection string to open an \c .mdb file directly, instead of
    having to create a DSN entry in the ODBC manager:

    \snippet doc/src/snippets/code/src_sql_kernel_qsqldatabase.cpp 3

    There is no default value.

    \sa databaseName() setUserName() setPassword() setHostName()
    \sa setPort() setConnectOptions() open()
*/

void QSqlDatabase::setDatabaseName(const QString &name)
{
   if (isValid()) {
      d->dbname = name;
   }
}

/*!
    Sets the connection's user name to \a name. To have effect, the
    user name must be set \e{before} the connection is \l{open()}
    {opened}.  Alternatively, you can close() the connection, set the
    user name, and call open() again.

    There is no default value.

    \sa userName() setDatabaseName() setPassword() setHostName()
    \sa setPort() setConnectOptions() open()
*/

void QSqlDatabase::setUserName(const QString &name)
{
   if (isValid()) {
      d->uname = name;
   }
}

/*!
    Sets the connection's password to \a password. To have effect, the
    password must be set \e{before} the connection is \l{open()}
    {opened}.  Alternatively, you can close() the connection, set the
    password, and call open() again.

    There is no default value.

    \warning This function stores the password in plain text within
    Qt. Use the open() call that takes a password as parameter to
    avoid this behavior.

    \sa password() setUserName() setDatabaseName() setHostName()
    \sa setPort() setConnectOptions() open()
*/

void QSqlDatabase::setPassword(const QString &password)
{
   if (isValid()) {
      d->pword = password;
   }
}

/*!
    Sets the connection's host name to \a host. To have effect, the
    host name must be set \e{before} the connection is \l{open()}
    {opened}.  Alternatively, you can close() the connection, set the
    host name, and call open() again.

    There is no default value.

    \sa hostName() setUserName() setPassword() setDatabaseName()
    \sa setPort() setConnectOptions() open()
*/

void QSqlDatabase::setHostName(const QString &host)
{
   if (isValid()) {
      d->hname = host;
   }
}

/*!
    Sets the connection's port number to \a port. To have effect, the
    port number must be set \e{before} the connection is \l{open()}
    {opened}.  Alternatively, you can close() the connection, set the
    port number, and call open() again..

    There is no default value.

    \sa port() setUserName() setPassword() setHostName()
    \sa setDatabaseName() setConnectOptions() open()
*/

void QSqlDatabase::setPort(int port)
{
   if (isValid()) {
      d->port = port;
   }
}

/*!
    Returns the connection's database name, which may be empty.
    \note The database name is not the connection name.

    \sa setDatabaseName()
*/
QString QSqlDatabase::databaseName() const
{
   return d->dbname;
}

/*!
    Returns the connection's user name; it may be empty.

    \sa setUserName()
*/
QString QSqlDatabase::userName() const
{
   return d->uname;
}

/*!
    Returns the connection's password. If the password was not set
    with setPassword(), and if the password was given in the open()
    call, or if no password was used, an empty string is returned.
*/
QString QSqlDatabase::password() const
{
   return d->pword;
}

/*!
    Returns the connection's host name; it may be empty.

    \sa setHostName()
*/
QString QSqlDatabase::hostName() const
{
   return d->hname;
}

/*!
    Returns the connection's driver name.

    \sa addDatabase(), driver()
*/
QString QSqlDatabase::driverName() const
{
   return d->drvName;
}

/*!
    Returns the connection's port number. The value is undefined if
    the port number has not been set.

    \sa setPort()
*/
int QSqlDatabase::port() const
{
   return d->port;
}

/*!
    Returns the database driver used to access the database
    connection.

    \sa addDatabase() drivers()
*/

QSqlDriver *QSqlDatabase::driver() const
{
   return d->driver;
}

/*!
    Returns information about the last error that occurred on the
    database.

    Failures that occur in conjunction with an individual query are
    reported by QSqlQuery::lastError().

    \sa QSqlError, QSqlQuery::lastError()
*/

QSqlError QSqlDatabase::lastError() const
{
   return d->driver->lastError();
}


/*!
    Returns a list of the database's tables, system tables and views,
    as specified by the parameter \a type.

    \sa primaryIndex(), record()
*/

QStringList QSqlDatabase::tables(QSql::TableType type) const
{
   return d->driver->tables(type);
}

/*!
    Returns the primary index for table \a tablename. If no primary
    index exists an empty QSqlIndex is returned.

    \sa tables(), record()
*/

QSqlIndex QSqlDatabase::primaryIndex(const QString &tablename) const
{
   return d->driver->primaryIndex(tablename);
}


/*!
    Returns a QSqlRecord populated with the names of all the fields in
    the table (or view) called \a tablename. The order in which the
    fields appear in the record is undefined. If no such table (or
    view) exists, an empty record is returned.
*/

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

/*!
    Returns the connection options string used for this connection.
    The string may be empty.

    \sa setConnectOptions()
 */
QString QSqlDatabase::connectOptions() const
{
   return d->connOptions;
}

/*!
    Returns true if a driver called \a name is available; otherwise
    returns false.

    \sa drivers()
*/

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

/*!
    \since 4.4

    Returns the connection name, which may be empty.  \note The
    connection name is not the \l{databaseName()} {database name}.

    \sa addDatabase()
*/
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

/*!
    \since 4.6

    Returns the current default precision policy for the database connection.

    \sa QSql::NumericalPrecisionPolicy, setNumericalPrecisionPolicy(),
    QSqlQuery::numericalPrecisionPolicy(), QSqlQuery::setNumericalPrecisionPolicy()
*/
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

