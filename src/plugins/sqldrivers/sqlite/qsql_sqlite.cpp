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

#include <qsql_sqlite.h>

#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qvariant.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlquery.h>
#include <qsqlcachedresult_p.h>
#include <qsqldriver_p.h>
#include <qstringlist.h>
#include <qvector.h>
#include <qdebug.h>
#include <qtimezone.h>

#if defined Q_OS_WIN
# include <qt_windows.h>
#else
# include <unistd.h>
#endif

#include <sqlite3.h>

class QSQLiteResultPrivate;

static QString _q_escapeIdentifier(const QString &identifier)
{
   QString retval = identifier;

   if (! identifier.isEmpty() && ! identifier.startsWith('"') && ! identifier.endsWith('"')) {
      retval.replace('"', "\"\"");

      retval.prepend('"');
      retval.append('"');

      retval.replace('.', "\".\"");
   }

   return retval;
}

static QVariant::Type qGetColumnType(const QString &tpName)
{
   const QString typeName = tpName.toLower();

   if (typeName == "integer" || typeName == "int") {
      return QVariant::Int;
   }

   if (typeName == "double" || typeName == "float" || typeName == "real" || typeName.startsWith("numeric")) {
      return QVariant::Double;
   }

   if (typeName == "blob") {
      return QVariant::ByteArray;
   }

   if (typeName == "boolean" || typeName == "bool") {
      return QVariant::Bool;
   }

   return QVariant::String;
}

static QSqlError qMakeError(sqlite3 *access, const QString &descr, QSqlError::ErrorType type,
      int errorCode = -1)
{
   return QSqlError(descr,
         QString::fromUtf8(sqlite3_errmsg(access)), type, QString::number(errorCode));
}

class QSQLiteResult : public QSqlCachedResult
{
 public:
   explicit QSQLiteResult(const QSQLiteDriver *db);
   ~QSQLiteResult();
   QVariant handle() const override;

 protected:
   bool gotoNext(QSqlCachedResult::ValueCache &row, int idx) override;
   bool reset(const QString &query) override;
   bool prepare(const QString &query) override;
   bool exec() override;
   int size() override;
   int numRowsAffected() override;
   QVariant lastInsertId() const override;
   QSqlRecord record() const override;
   void detachFromResultSet() override;
   void virtual_hook(int id, void *data) override;

 private:
   QSQLiteResultPrivate *d;

   friend class QSQLiteDriver;
   friend class QSQLiteResultPrivate;
};

class QSQLiteDriverPrivate : public QSqlDriverPrivate
{
 public:
   QSQLiteDriverPrivate()
      : QSqlDriverPrivate(), access(nullptr)
   {
      dbmsType = QSqlDriver::SQLite;
   }

   sqlite3 *access;
   QList <QSQLiteResult *> results;
};

class QSQLiteResultPrivate
{
 public:
   QSQLiteResultPrivate(QSQLiteResult *res);
   void cleanup();
   bool fetchNext(QSqlCachedResult::ValueCache &values, int idx, bool initialFetch);
   // initializes the recordInfo and the cache
   void initColumns(bool emptyResultset);
   void finalize();

   QSQLiteResult *q;
   sqlite3 *access;

   sqlite3_stmt *stmt;

   bool skippedStatus;    // the status of the fetchNext() that's skipped
   bool skipRow;          // skip the next fetchNext()?
   QSqlRecord rInf;
   QVector<QVariant> firstRow;
};

QSQLiteResultPrivate::QSQLiteResultPrivate(QSQLiteResult *res)
   : q(res), access(nullptr), stmt(nullptr), skippedStatus(false), skipRow(false)
{
}

void QSQLiteResultPrivate::cleanup()
{
   finalize();
   rInf.clear();
   skippedStatus = false;
   skipRow = false;
   q->setAt(QSql::BeforeFirstRow);
   q->setActive(false);
   q->cleanup();
}

void QSQLiteResultPrivate::finalize()
{
   if (! stmt) {
      return;
   }

   sqlite3_finalize(stmt);
   stmt = nullptr;
}

void QSQLiteResultPrivate::initColumns(bool emptyResultset)
{
   int nCols = sqlite3_column_count(stmt);

   if (nCols <= 0) {
      return;
   }

   q->init(nCols);

   for (int i = 0; i < nCols; ++i) {
      QString colName = QString::fromUtf8(sqlite3_column_name(stmt, i)).remove('"');

      // must use typeName for resolving the type to match QSqliteDriver::record
      QString typeName = QString::fromUtf8(sqlite3_column_decltype(stmt, i));

      // sqlite3_column_type is documented to have undefined behavior if the result set is empty
      int stp = emptyResultset ? -1 : sqlite3_column_type(stmt, i);

      QVariant::Type fieldType;

      if (!typeName.isEmpty()) {
         fieldType = qGetColumnType(typeName);

      } else {
         // Get the proper type for the field based on stp value
         switch (stp) {
            case SQLITE_INTEGER:
               fieldType = QVariant::Int;
               break;
            case SQLITE_FLOAT:
               fieldType = QVariant::Double;
               break;
            case SQLITE_BLOB:
               fieldType = QVariant::ByteArray;
               break;
            case SQLITE_TEXT:
               fieldType = QVariant::String;
               break;
            case SQLITE_NULL:
            default:
               fieldType = QVariant::Invalid;
               break;
         }
      }

      QSqlField fld(colName, fieldType);
      fld.setSqlType(stp);
      rInf.append(fld);
   }
}

bool QSQLiteResultPrivate::fetchNext(QSqlCachedResult::ValueCache &values, int idx, bool initialFetch)
{
   int res;
   int i;

   if (skipRow) {
      // already fetched
      Q_ASSERT(!initialFetch);
      skipRow = false;
      for (int i = 0; i < firstRow.count(); i++) {
         values[i] = firstRow[i];
      }
      return skippedStatus;
   }
   skipRow = initialFetch;

   if (initialFetch) {
      firstRow.clear();
      firstRow.resize(sqlite3_column_count(stmt));
   }

   if (!stmt) {
      q->setLastError(QSqlError(QCoreApplication::translate("QSQLiteResult", "Unable to fetch row"),
            QCoreApplication::translate("QSQLiteResult", "No query"), QSqlError::ConnectionError));
      q->setAt(QSql::AfterLastRow);
      return false;
   }
   res = sqlite3_step(stmt);

   switch (res) {
      case SQLITE_ROW:
         // check to see if should fill out columns
         if (rInf.isEmpty())
            // must be first call.
         {
            initColumns(false);
         }
         if (idx < 0 && !initialFetch) {
            return true;
         }
         for (i = 0; i < rInf.count(); ++i) {
            switch (sqlite3_column_type(stmt, i)) {
               case SQLITE_BLOB:
                  values[i + idx] = QByteArray(static_cast<const char *>(
                           sqlite3_column_blob(stmt, i)),
                        sqlite3_column_bytes(stmt, i));
                  break;
               case SQLITE_INTEGER:
                  values[i + idx] = sqlite3_column_int64(stmt, i);
                  break;
               case SQLITE_FLOAT:
                  switch (q->numericalPrecisionPolicy()) {
                     case QSql::LowPrecisionInt32:
                        values[i + idx] = sqlite3_column_int(stmt, i);
                        break;
                     case QSql::LowPrecisionInt64:
                        values[i + idx] = sqlite3_column_int64(stmt, i);
                        break;
                     case QSql::LowPrecisionDouble:
                     case QSql::HighPrecision:
                     default:
                        values[i + idx] = sqlite3_column_double(stmt, i);
                        break;
                  };
                  break;

               case SQLITE_NULL:
                  values[i + idx] = QVariant();
                  break;

               default:
                  values[i + idx] = QString::fromUtf8((const char *) sqlite3_column_text(stmt, i),
                        sqlite3_column_bytes(stmt, i));
                  break;
            }
         }
         return true;

      case SQLITE_DONE:
         if (rInf.isEmpty())
            // must be first call.
         {
            initColumns(true);
         }

         q->setAt(QSql::AfterLastRow);
         sqlite3_reset(stmt);
         return false;

      case SQLITE_CONSTRAINT:
      case SQLITE_ERROR:
         // SQLITE_ERROR is a generic error code and we must call sqlite3_reset()
         // to get the specific error message.
         res = sqlite3_reset(stmt);
         q->setLastError(qMakeError(access, QCoreApplication::translate("QSQLiteResult",
                  "Unable to fetch row"), QSqlError::ConnectionError, res));
         q->setAt(QSql::AfterLastRow);
         return false;
      case SQLITE_MISUSE:
      case SQLITE_BUSY:
      default:
         // something wrong, don't get col info, but still return false
         q->setLastError(qMakeError(access, QCoreApplication::translate("QSQLiteResult",
                  "Unable to fetch row"), QSqlError::ConnectionError, res));
         sqlite3_reset(stmt);
         q->setAt(QSql::AfterLastRow);
         return false;
   }
   return false;
}

QSQLiteResult::QSQLiteResult(const QSQLiteDriver *db)
   : QSqlCachedResult(db)
{
   d = new QSQLiteResultPrivate(this);
   d->access = db->d_func()->access;
   const_cast<QSQLiteDriverPrivate *>(db->d_func())->results.append(this);
}

QSQLiteResult::~QSQLiteResult()
{
   const QSqlDriver *sqlDriver = driver();
   if (sqlDriver) {
      const_cast<QSQLiteDriverPrivate *>(dynamic_cast<const QSQLiteDriver *>(sqlDriver)->d_func())->results.removeOne(this);
   }

   d->cleanup();
   delete d;
}

void QSQLiteResult::virtual_hook(int id, void *data)
{

   QSqlCachedResult::virtual_hook(id, data);
}

bool QSQLiteResult::reset(const QString &query)
{
   if (! prepare(query)) {
      return false;
   }
   return exec();
}

bool QSQLiteResult::prepare(const QString &query)
{
   if (! driver() || ! driver()->isOpen() || driver()->isOpenError()) {
      return false;
   }

   d->cleanup();

   setSelect(false);

   const char *pzTail = nullptr;

#if (SQLITE_VERSION_NUMBER >= 3003011)
   int res = sqlite3_prepare_v2(d->access, query.constData(), (query.size_storage() + 1),
         &d->stmt, &pzTail);
#else
   int res = sqlite3_prepare(d->access, query.constData(), (query.size_storage() + 1),
         &d->stmt, &pzTail);
#endif

   if (res != SQLITE_OK) {
      setLastError(qMakeError(d->access, QCoreApplication::translate("QSQLiteResult",
            "Unable to execute statement"), QSqlError::StatementError, res));

      d->finalize();
      return false;

   } else if (pzTail && ! QString::fromUtf8(pzTail).trimmed().isEmpty()) {
      setLastError(qMakeError(d->access, QCoreApplication::translate("QSQLiteResult",
            "Unable to execute multiple statements at a time"), QSqlError::StatementError, SQLITE_MISUSE));

      d->finalize();
      return false;
   }

   return true;
}

static QString secondsToOffset(int seconds)
{
   const QChar sign = ushort(seconds < 0 ? '-' : '+');
   seconds = qAbs(seconds);

   const int hours   = seconds / 3600;
   const int minutes = (seconds % 3600) / 60;

   return QString("%1%2:%3").formatArg(sign).formatArg(hours, 2, 10, '0').formatArg(minutes, 2, 10, '0');
}

static QString timespecToString(const QDateTime &dateTime)
{
   return secondsToOffset(dateTime.timeZone().offsetFromUtc(dateTime));
}

bool QSQLiteResult::exec()
{
   const QVector<QVariant> values = boundValues();

   d->skippedStatus = false;
   d->skipRow       = false;
   d->rInf.clear();

   clearValues();
   setLastError(QSqlError());

   int res = sqlite3_reset(d->stmt);

   if (res != SQLITE_OK) {
      setLastError(qMakeError(d->access, QCoreApplication::translate("QSQLiteResult",
               "Unable to reset statement"), QSqlError::StatementError, res));
      d->finalize();
      return false;
   }
   int paramCount = sqlite3_bind_parameter_count(d->stmt);

   if (paramCount == values.count()) {
      for (int i = 0; i < paramCount; ++i) {
         res = SQLITE_OK;
         const QVariant value = values.at(i);

         if (! value.isValid()) {
            res = sqlite3_bind_null(d->stmt, i + 1);

         } else {
            switch (value.type()) {

               case QVariant::ByteArray: {
                  const QByteArray ba = value.getData<QByteArray>();
                  res = sqlite3_bind_blob(d->stmt, i + 1, ba.constData(), ba.size(), SQLITE_STATIC);
                  break;
               }

               case QVariant::Int:
               case QVariant::Bool:
                  res = sqlite3_bind_int(d->stmt, i + 1, value.toInt());
                  break;

               case QVariant::Double:
                  res = sqlite3_bind_double(d->stmt, i + 1, value.toDouble());
                  break;

               case QVariant::UInt:
               case QVariant::LongLong:
                  res = sqlite3_bind_int64(d->stmt, i + 1, value.toLongLong());
                  break;

               case QVariant::DateTime: {
                  const QDateTime dateTime = value.toDateTime();
                  const QString str = dateTime.toString("yyyy-MM-ddThh:mm:ss.zzz" + timespecToString(dateTime));
                  res = sqlite3_bind_text64(d->stmt, i + 1, str.constData(), str.size_storage(), SQLITE_TRANSIENT, SQLITE_UTF8);
                  break;
               }

               case QVariant::Time: {
                  const QTime time  = value.toTime();
                  const QString str = time.toString("hh:mm:ss.zzz");
                  res = sqlite3_bind_text64(d->stmt, i + 1, str.constData(), str.size_storage(), SQLITE_TRANSIENT, SQLITE_UTF8);
                  break;
               }

               case QVariant::String: {
                  const QString str = value.getData<QString>();
                  res = sqlite3_bind_text64(d->stmt, i + 1, str.constData(), str.size_storage(), SQLITE_TRANSIENT, SQLITE_UTF8);
                  break;
               }

               default: {
                  QString str = value.toString();
                    res = sqlite3_bind_text64(d->stmt, i + 1, str.constData(), str.size_storage(), SQLITE_TRANSIENT, SQLITE_UTF8);
                  break;
               }
            }
         }

         if (res != SQLITE_OK) {
            setLastError(qMakeError(d->access, QCoreApplication::translate("QSQLiteResult",
                  "Unable to bind parameters"), QSqlError::StatementError, res));

            d->finalize();
            return false;
         }
      }

   } else {
      setLastError(QSqlError(QCoreApplication::translate("QSQLiteResult",
               "Parameter count mismatch"), QString(), QSqlError::StatementError));

      return false;
   }

   d->skippedStatus = d->fetchNext(d->firstRow, 0, true);

   if (lastError().isValid()) {
      setSelect(false);
      setActive(false);
      return false;
   }

   setSelect(!d->rInf.isEmpty());
   setActive(true);

   return true;
}

bool QSQLiteResult::gotoNext(QSqlCachedResult::ValueCache &row, int idx)
{
   return d->fetchNext(row, idx, false);
}

int QSQLiteResult::size()
{
   return -1;
}

int QSQLiteResult::numRowsAffected()
{
   return sqlite3_changes(d->access);
}

QVariant QSQLiteResult::lastInsertId() const
{
   if (isActive()) {
      qint64 id = sqlite3_last_insert_rowid(d->access);
      if (id) {
         return id;
      }
   }

   return QVariant();
}

QSqlRecord QSQLiteResult::record() const
{
   if (!isActive() || !isSelect()) {
      return QSqlRecord();
   }
   return d->rInf;
}

void QSQLiteResult::detachFromResultSet()
{
   if (d->stmt) {
      sqlite3_reset(d->stmt);
   }
}


QVariant QSQLiteResult::handle() const
{
   return QVariant::fromValue(d->stmt);
}

QSQLiteDriver::QSQLiteDriver(QObject *parent)
   : QSqlDriver(*new QSQLiteDriverPrivate, parent)
{
}

QSQLiteDriver::QSQLiteDriver(sqlite3 *connection, QObject *parent)
   : QSqlDriver(*new QSQLiteDriverPrivate, parent)
{
   Q_D(QSQLiteDriver);
   d->access = connection;
   setOpen(true);
   setOpenError(false);
}

QSQLiteDriver::~QSQLiteDriver()
{
}

bool QSQLiteDriver::hasFeature(DriverFeature f) const
{
   switch (f) {
      case BLOB:
      case Transactions:
      case Unicode:
      case LastInsertId:
      case PreparedQueries:
      case PositionalPlaceholders:
      case SimpleLocking:
      case FinishQuery:
      case LowPrecisionNumbers:
         return true;
      case QuerySize:
      case NamedPlaceholders:
      case BatchOperations:
      case EventNotifications:
      case MultipleResultSets:
      case CancelQuery:
         return false;
   }
   return false;
}

/*
   SQLite dbs have no user name, passwords, hosts or ports.
   just file names.
*/
bool QSQLiteDriver::open(const QString &db, const QString &, const QString &, const QString &, int,
   const QString &conOpts)
{
   Q_D(QSQLiteDriver);

   if (isOpen()) {
      close();
   }

   int timeOut = 5000;
   bool sharedCache = false;
   bool openReadOnlyOption = false;
   bool openUriOption = false;

   QStringList opts = QString(conOpts).remove(' ').split(';');

   for (const QString &option : opts) {
      if (option.startsWith("QSQLITE_BUSY_TIMEOUT=")) {
         bool ok;
         int nt = option.mid(21).toInteger<int>(&ok);

         if (ok) {
            timeOut = nt;
         }


      } else if (option == "QSQLITE_OPEN_READONLY") {
         openReadOnlyOption = true;

      } else if (option == "QSQLITE_OPEN_URI") {
         openUriOption = true;

      } else if (option == "QSQLITE_ENABLE_SHARED_CACHE") {
         sharedCache = true;
      }
   }

   int openMode = (openReadOnlyOption ? SQLITE_OPEN_READONLY : (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
   if (openUriOption) {
      openMode |= SQLITE_OPEN_URI;
   }
   sqlite3_enable_shared_cache(sharedCache);

   if (sqlite3_open_v2(db.toUtf8().constData(), &d->access, openMode, nullptr) == SQLITE_OK) {
      sqlite3_busy_timeout(d->access, timeOut);
      setOpen(true);
      setOpenError(false);
      return true;

   } else {
      if (d->access) {
         sqlite3_close(d->access);
         d->access = nullptr;
      }

      setLastError(qMakeError(d->access, tr("Error opening database"), QSqlError::ConnectionError));
      setOpenError(true);
      return false;
   }
}

void QSQLiteDriver::close()
{
   Q_D(QSQLiteDriver);

   if (isOpen()) {
      for (QSQLiteResult *result : d->results) {
         result->d->finalize();
      }

      if (sqlite3_close(d->access) != SQLITE_OK) {
         setLastError(qMakeError(d->access, tr("Error closing database"), QSqlError::ConnectionError));
      }

      d->access = nullptr;
      setOpen(false);
      setOpenError(false);
   }
}

QSqlResult *QSQLiteDriver::createResult() const
{
   return new QSQLiteResult(this);
}

bool QSQLiteDriver::beginTransaction()
{
   if (!isOpen() || isOpenError()) {
      return false;
   }

   QSqlQuery q(createResult());
   if (! q.exec("BEGIN")) {
      setLastError(QSqlError(tr("Unable to begin transaction"),
            q.lastError().databaseText(), QSqlError::TransactionError));

      return false;
   }

   return true;
}

bool QSQLiteDriver::commitTransaction()
{
   if (!isOpen() || isOpenError()) {
      return false;
   }

   QSqlQuery q(createResult());
   if (! q.exec("COMMIT")) {
      setLastError(QSqlError(tr("Unable to commit transaction"),
            q.lastError().databaseText(), QSqlError::TransactionError));

      return false;
   }

   return true;
}

bool QSQLiteDriver::rollbackTransaction()
{
   if (!isOpen() || isOpenError()) {
      return false;
   }

   QSqlQuery q(createResult());
   if (!q.exec("ROLLBACK")) {
      setLastError(QSqlError(tr("Unable to rollback transaction"),
            q.lastError().databaseText(), QSqlError::TransactionError));
      return false;
   }

   return true;
}

QStringList QSQLiteDriver::tables(QSql::TableType type) const
{
   QStringList res;

   if (! isOpen()) {
      return res;
   }

   QSqlQuery q(createResult());
   q.setForwardOnly(true);

   QString sql = "SELECT name FROM sqlite_master WHERE %1 UNION ALL SELECT name FROM sqlite_temp_master WHERE %1";

   if ((type & QSql::Tables) && (type & QSql::Views)) {
      sql = sql.formatArg("type='table' OR type='view'");

   } else if (type & QSql::Tables) {
      sql = sql.formatArg("type='table'");

   } else if (type & QSql::Views) {
      sql = sql.formatArg("type='view'");

   } else {
      sql.clear();
   }

   if (!sql.isEmpty() && q.exec(sql)) {
      while (q.next()) {
         res.append(q.value(0).toString());
      }
   }

   if (type & QSql::SystemTables) {
      // there are no internal tables beside this one:
      res.append("sqlite_master");
   }

   return res;
}

static QSqlIndex qGetTableInfo(QSqlQuery &q, const QString &tableName, bool onlyPIndex = false)
{
   QString schema;
   QString table(tableName);

   int indexOfSeparator = tableName.indexOf('.');

   if (indexOfSeparator > -1) {
      schema = tableName.left(indexOfSeparator).append('.');
      table  = tableName.mid(indexOfSeparator + 1);
   }

   q.exec("PRAGMA " + schema + "table_info (" + _q_escapeIdentifier(table) + ")");

   QSqlIndex ind;

   while (q.next()) {
      bool isPk = q.value(5).toInt();

      if (onlyPIndex && !isPk) {
         continue;
      }

      QString typeName = q.value(2).toString().toLower();
      QSqlField fld(q.value(1).toString(), qGetColumnType(typeName));

      if (isPk && (typeName == "integer")) {
         // INTEGER PRIMARY KEY fields are auto-generated in sqlite
         // INT PRIMARY KEY is not the same as INTEGER PRIMARY KEY

         fld.setAutoValue(true);
      }

      fld.setRequired(q.value(3).toInt() != 0);
      fld.setDefaultValue(q.value(4));
      ind.append(fld);
   }

   return ind;
}

QSqlIndex QSQLiteDriver::primaryIndex(const QString &tblname) const
{
   if (! isOpen()) {
      return QSqlIndex();
   }

   QString table = tblname;
   if (isIdentifierEscaped(table, QSqlDriver::TableName)) {
      table = stripDelimiters(table, QSqlDriver::TableName);
   }

   QSqlQuery q(createResult());
   q.setForwardOnly(true);

   return qGetTableInfo(q, table, true);
}

QSqlRecord QSQLiteDriver::record(const QString &tbl) const
{
   if (!isOpen()) {
      return QSqlRecord();
   }

   QString table = tbl;
   if (isIdentifierEscaped(table, QSqlDriver::TableName)) {
      table = stripDelimiters(table, QSqlDriver::TableName);
   }

   QSqlQuery q(createResult());
   q.setForwardOnly(true);

   return qGetTableInfo(q, table);
}

QVariant QSQLiteDriver::handle() const
{
   Q_D(const QSQLiteDriver);
   return QVariant::fromValue(d->access);
}

QString QSQLiteDriver::escapeIdentifier(const QString &identifier, IdentifierType type) const
{
   (void) type;
   return _q_escapeIdentifier(identifier);
}

