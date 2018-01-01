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

#include <qsqldatabase.h>
#include <qsqlquery.h>

#ifdef Q_OS_WIN32
// Conflicting declarations of LPCBYTE in sqlfront.h and winscard.h
#define _WINSCARD_H_
#endif

#ifdef QT_SQL_PSQL
#include <../drivers/psql/qsql_psql.h
#endif

#ifdef QT_SQL_MYSQL
#include "../drivers/mysql/qsql_mysql.h"
#endif

#ifdef QT_SQL_ODBC
#include "../drivers/odbc/qsql_odbc.h"
#endif

#ifdef QT_SQL_OCI
#include "../drivers/oci/qsql_oci.h"
#endif

#ifdef QT_SQL_TDS
// conflicting RETCODE typedef between odbc and freetds
#define RETCODE DBRETCODE
#include "../drivers/tds/qsql_tds.h"
#undef RETCODE
#endif

#ifdef QT_SQL_DB2
#include "../drivers/db2/qsql_db2.h"
#endif

#ifdef QT_SQL_SQLITE
#include "../drivers/sqlite/qsql_sqlite.h"
#endif

#ifdef QT_SQL_SQLITE2
#include "../drivers/sqlite2/qsql_sqlite2.h"
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

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QSqlDriverFactoryInterface_iid, "/sqldrivers"))

const char *QSqlDatabase::defaultConnection = "qt_sql_default_connection";

typedef QHash<QString, QSqlDriverCreatorBase *> DriverDict;

class QConnectionDict: public QHash<QString, QSqlDatabase>
{
 public:
   inline bool contains_ts(const QString &key) {
      QReadLocker locker(&lock);
      return contains(key);
   }
   inline QStringList keys_ts() const {
      QReadLocker locker(&lock);
      return keys();
   }

   mutable QReadWriteLock lock;
};
Q_GLOBAL_STATIC(QConnectionDict, dbDict)

class QSqlDatabasePrivate
{
 public:
   QSqlDatabasePrivate(QSqlDatabase *d, QSqlDriver *dr = 0):
      q(d),
      driver(dr),
      port(-1) {
      ref = 1;
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
   static DriverDict &driverDict();
   static void cleanConnections();
};

QSqlDatabasePrivate::QSqlDatabasePrivate(const QSqlDatabasePrivate &other)
{
   ref = 1;
   q = other.q;
   dbname = other.dbname;
   uname = other.uname;
   pword = other.pword;
   hname = other.hname;
   drvName = other.drvName;
   port = other.port;
   connOptions = other.connOptions;
   driver = other.driver;
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

static bool qDriverDictInit = false;
static void cleanDriverDict()
{
   qDeleteAll(QSqlDatabasePrivate::driverDict());
   QSqlDatabasePrivate::driverDict().clear();
   QSqlDatabasePrivate::cleanConnections();
   qDriverDictInit = false;
}

DriverDict &QSqlDatabasePrivate::driverDict()
{
   static DriverDict dict;

   if (! qDriverDictInit) {
      qDriverDictInit = true;
      qAddPostRoutine(cleanDriverDict);
   }
   return dict;
}

QSqlDatabasePrivate *QSqlDatabasePrivate::shared_null()
{
   static QSqlNullDriver dr;
   static QSqlDatabasePrivate n(NULL, &dr);
   return &n;
}

void QSqlDatabasePrivate::invalidateDb(const QSqlDatabase &db, const QString &name, bool doWarn)
{
   if (db.d->ref.load() != 1 && doWarn) {
      qWarning("QSqlDatabasePrivate::removeDatabase: connection '%s' is still in use, "
               "all queries will cease to work.", name.toLocal8Bit().constData());
      db.d->disable();
      db.d->connName.clear();
   }
}

void QSqlDatabasePrivate::removeDatabase(const QString &name)
{
   QConnectionDict *dict = dbDict();
   Q_ASSERT(dict);
   QWriteLocker locker(&dict->lock);

   if (!dict->contains(name)) {
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
      qWarning("QSqlDatabasePrivate::addDatabase: duplicate connection name '%s', old "
               "connection removed.", name.toLocal8Bit().data());
   }
   dict->insert(name, db);
   db.d->connName = name;
}

/*! \internal
*/
QSqlDatabase QSqlDatabasePrivate::database(const QString &name, bool open)
{
   const QConnectionDict *dict = dbDict();
   Q_ASSERT(dict);

   dict->lock.lockForRead();
   QSqlDatabase db = dict->value(name);
   dict->lock.unlock();
   if (db.isValid() && !db.isOpen() && open) {
      if (!db.open()) {
         qWarning() << "QSqlDatabasePrivate::database: unable to open database:" << db.lastError().text();
      }

   }
   return db;
}


/*! \internal
    Copies the connection data from \a other.
*/
void QSqlDatabasePrivate::copy(const QSqlDatabasePrivate *other)
{
   q = other->q;
   dbname = other->dbname;
   uname = other->uname;
   pword = other->pword;
   hname = other->hname;
   drvName = other->drvName;
   port = other->port;
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
#endif

#ifdef QT_SQL_MYSQL
   list << "QMYSQL";
#endif

#ifdef QT_SQL_ODBC
   list << "QODBC";
#endif

#ifdef QT_SQL_OCI
   list << "QOCI";
#endif

#ifdef QT_SQL_TDS
   list << "QTDS";
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

   if (QFactoryLoader *fl = loader()) {
      QStringList keys = fl->keys();
      for (QStringList::const_iterator i = keys.constBegin(); i != keys.constEnd(); ++i) {
         if (!list.contains(*i)) {
            list << *i;
         }
      }
   }

   DriverDict dict = QSqlDatabasePrivate::driverDict();
   for (DriverDict::const_iterator i = dict.constBegin(); i != dict.constEnd(); ++i) {
      if (!list.contains(i.key())) {
         list << i.key();
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
      DriverDict dict = QSqlDatabasePrivate::driverDict();

      for (DriverDict::const_iterator it = dict.constBegin(); it != dict.constEnd() && ! driver; ++it) {

         if (type == it.key()) {
            driver = ((QSqlDriverCreatorBase *)(*it))->createObject();
         }
      }
   }

   if (! driver && loader()) {

      if (QSqlDriverFactoryInterface *factory = qobject_cast<QSqlDriverFactoryInterface *>(loader()->instance(type))) {
         driver = factory->create(type);
      }
   }

   if (! driver) {
      qWarning("QSqlDatabase: %s driver not loaded", type.toLatin1().data());
      qWarning("QSqlDatabase: available drivers: %s", QSqlDatabase::drivers().join(" ").toLatin1().data());

      if (QCoreApplication::instance() == 0) {
         qWarning("QSqlDatabase: an instance of QCoreApplication is required for loading driver plugins");
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

/*!
  Begins a transaction on the database if the driver supports
  transactions. Returns \c{true} if the operation succeeded.
  Otherwise it returns \c{false}.

  \sa QSqlDriver::hasFeature(), commit(), rollback()
*/
bool QSqlDatabase::transaction()
{
   if (!d->driver->hasFeature(QSqlDriver::Transactions)) {
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


/*!
    Sets database-specific \a options. This must be done before the
    connection is opened or it has no effect (or you can close() the
    connection, call this function and open() the connection again).

    The format of the \a options string is a semicolon separated list
    of option names or option=value pairs. The options depend on the
    database client used:

    \table
    \header \i ODBC \i MySQL \i PostgreSQL
    \row

    \i
    \list
    \i SQL_ATTR_ACCESS_MODE
    \i SQL_ATTR_LOGIN_TIMEOUT
    \i SQL_ATTR_CONNECTION_TIMEOUT
    \i SQL_ATTR_CURRENT_CATALOG
    \i SQL_ATTR_METADATA_ID
    \i SQL_ATTR_PACKET_SIZE
    \i SQL_ATTR_TRACEFILE
    \i SQL_ATTR_TRACE
    \i SQL_ATTR_CONNECTION_POOLING
    \i SQL_ATTR_ODBC_VERSION
    \endlist

    \i
    \list
    \i CLIENT_COMPRESS
    \i CLIENT_FOUND_ROWS
    \i CLIENT_IGNORE_SPACE
    \i CLIENT_SSL
    \i CLIENT_ODBC
    \i CLIENT_NO_SCHEMA
    \i CLIENT_INTERACTIVE
    \i UNIX_SOCKET
    \i MYSQL_OPT_RECONNECT
    \endlist

    \i
    \list
    \i connect_timeout
    \i options
    \i tty
    \i requiressl
    \i service
    \endlist

    \header \i DB2 \i OCI \i TDS
    \row

    \i
    \list
    \i SQL_ATTR_ACCESS_MODE
    \i SQL_ATTR_LOGIN_TIMEOUT
    \endlist

    \i
    \list
    \i OCI_ATTR_PREFETCH_ROWS
    \i OCI_ATTR_PREFETCH_MEMORY
    \endlist

    \i
    \e none

    \header \i SQLite \i Interbase
    \row

    \i
    \list
    \i QSQLITE_BUSY_TIMEOUT
    \i QSQLITE_OPEN_READONLY
    \i QSQLITE_ENABLE_SHARED_CACHE
    \endlist

    \i
    \list
    \i ISC_DPB_LC_CTYPE
    \i ISC_DPB_SQL_ROLE_NAME
    \endlist

    \endtable

    Examples:
    \snippet doc/src/snippets/code/src_sql_kernel_qsqldatabase.cpp 4

    Refer to the client library documentation for more information
    about the different options.

    \sa connectOptions()
*/

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

/*! \fn QSqlDatabase QSqlDatabase::addDatabase(QSqlDriver* driver, const QString& connectionName)

    This overload is useful when you want to create a database
    connection with a \l{QSqlDriver} {driver} you instantiated
    yourself. It might be your own database driver, or you might just
    need to instantiate one of the Qt drivers yourself. If you do
    this, it is recommended that you include the driver code in your
    application. For example, you can create a PostgreSQL connection
    with your own QPSQL driver like this:

    \snippet doc/src/snippets/code/src_sql_kernel_qsqldatabase.cpp 5
    \codeline
    \snippet doc/src/snippets/code/src_sql_kernel_qsqldatabase.cpp 6

    The above code sets up a PostgreSQL connection and instantiates a
    QPSQLDriver object. Next, addDatabase() is called to add the
    connection to the known connections so that it can be used by the
    Qt SQL classes. When a driver is instantiated with a connection
    handle (or set of handles), Qt assumes that you have already
    opened the database connection.

    \note We assume that \c qtdir is the directory where Qt is
    installed. This will pull in the code that is needed to use the
    PostgreSQL client library and to instantiate a QPSQLDriver object,
    assuming that you have the PostgreSQL headers somewhere in your
    include search path.

    Remember that you must link your application against the database
    client library. Make sure the client library is in your linker's
    search path, and add lines like these to your \c{.pro} file:

    \snippet doc/src/snippets/code/src_sql_kernel_qsqldatabase.cpp 7

    The method described works for all the supplied drivers.  The only
    difference will be in the driver constructor arguments.  Here is a
    table of the drivers included with Qt, their source code files,
    and their constructor arguments:

    \table
    \header \i Driver \i Class name \i Constructor arguments \i File to include
    \row
    \i QPSQL
    \i QPSQLDriver
    \i PGconn *connection
    \i \c qsql_psql.cpp
    \row
    \i QMYSQL
    \i QMYSQLDriver
    \i MYSQL *connection
    \i \c qsql_mysql.cpp
    \row
    \i QOCI
    \i QOCIDriver
    \i OCIEnv *environment, OCISvcCtx *serviceContext
    \i \c qsql_oci.cpp
    \row
    \i QODBC
    \i QODBCDriver
    \i SQLHANDLE environment, SQLHANDLE connection
    \i \c qsql_odbc.cpp
    \row
    \i QDB2
    \i QDB2
    \i SQLHANDLE environment, SQLHANDLE connection
    \i \c qsql_db2.cpp
    \row
    \i QTDS
    \i QTDSDriver
    \i LOGINREC *loginRecord, DBPROCESS *dbProcess, const QString &hostName
    \i \c qsql_tds.cpp
    \row
    \i QSQLITE
    \i QSQLiteDriver
    \i sqlite *connection
    \i \c qsql_sqlite.cpp
    \row
    \i QIBASE
    \i QIBaseDriver
    \i isc_db_handle connection
    \i \c qsql_ibase.cpp
    \endtable

    The host name (or service name) is needed when constructing the
    QTDSDriver for creating new connections for internal queries. This
    is to prevent blocking when several QSqlQuery objects are used
    simultaneously.

    \warning Adding a database connection with the same connection
    name as an existing connection, causes the existing connection to
    be replaced by the new one.

    \warning The SQL framework takes ownership of the \a driver. It
    must not be deleted. To remove the connection, use
    removeDatabase().

    \sa drivers()
*/
QSqlDatabase QSqlDatabase::addDatabase(QSqlDriver *driver, const QString &connectionName)
{
   QSqlDatabase db(driver);
   QSqlDatabasePrivate::addDatabase(db, connectionName);
   return db;
}

/*!
    Returns true if the QSqlDatabase has a valid driver.

    Example:
    \snippet doc/src/snippets/code/src_sql_kernel_qsqldatabase.cpp 8
*/
bool QSqlDatabase::isValid() const
{
   return d->driver && d->driver != d->shared_null()->driver;
}

/*!
    Clones the database connection \a other and and stores it as \a
    connectionName. All the settings from the original database, e.g.
    databaseName(), hostName(), etc., are copied across. Does nothing
    if \a other is an invalid database. Returns the newly created
    database connection.

    \note The new connection has not been opened. Before using the new
    connection, you must call open().
*/
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

/*!
    \since 4.6

    Sets the default numerical precision policy used by queries created
    on this database connection to \a precisionPolicy.

    Note: Drivers that don't support fetching numerical values with low
    precision will ignore the precision policy. You can use
    QSqlDriver::hasFeature() to find out whether a driver supports this
    feature.

    Note: Setting the default precision policy to \a precisionPolicy
    doesn't affect any currently active queries.

    \sa QSql::NumericalPrecisionPolicy, numericalPrecisionPolicy(),
    QSqlQuery::setNumericalPrecisionPolicy(), QSqlQuery::numericalPrecisionPolicy()
*/
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
   if (!d.isValid()) {
      dbg.nospace() << "QSqlDatabase(invalid)";
      return dbg.space();
   }

   dbg.nospace() << "QSqlDatabase(driver=\"" << d.driverName() << "\", database=\""
                 << d.databaseName() << "\", host=\"" << d.hostName() << "\", port=" << d.port()
                 << ", user=\"" << d.userName() << "\", open=" << d.isOpen() << ")";
   return dbg.space();
}

QT_END_NAMESPACE
