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

#include <qsql_mysql.h>

#include <qcoreapplication.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlquery.h>
#include <qsqlrecord.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qvector.h>

#include <qdebug.h>

#ifdef Q_OS_WIN32
// comment the next line out if you want to use MySQL/embedded on Win32 systems
// note that it will crash if you do not statically link to the mysql/e library
# define Q_NO_MYSQL_EMBEDDED
#endif

Q_DECLARE_METATYPE(MYSQL_RES *)
Q_DECLARE_METATYPE(MYSQL *)
Q_DECLARE_METATYPE(MYSQL_STMT *)

#define Q_CLIENT_MULTI_STATEMENTS CLIENT_MULTI_STATEMENTS

class QMYSQLDriverPrivate
{
 public:
   QMYSQLDriverPrivate() : mysql(0),

#ifndef QT_NO_TEXTCODEC
      tc(QTextCodec::codecForLocale()),
#else
      tc(0),
#endif
      preparedQuerysEnabled(false) {}
   MYSQL *mysql;
   QTextCodec *tc;

   bool preparedQuerysEnabled;
};

static inline QString toUnicode(QTextCodec *tc, const char *str)
{
#ifdef QT_NO_TEXTCODEC
   return QString::fromLatin1(str);
#else
   return tc->toUnicode(str);
#endif
}

static inline QString toUnicode(QTextCodec *tc, const char *str, int length)
{
#ifdef QT_NO_TEXTCODEC
   return QString::fromLatin1(str, length);
#else
   return tc->toUnicode(str, length);
#endif
}

static inline QByteArray fromUnicode(QTextCodec *tc, const QString &str)
{
#ifdef QT_NO_TEXTCODEC
   return str.toLatin1();
#else
   return tc->fromUnicode(str);
#endif
}

static inline QVariant qDateFromString(const QString &val)
{
#ifdef QT_NO_DATESTRING
   return QVariant(val);
#else
   if (val.isEmpty()) {
      return QVariant(QDate());
   }
   return QVariant(QDate::fromString(val, Qt::ISODate));
#endif
}

static inline QVariant qTimeFromString(const QString &val)
{
#ifdef QT_NO_DATESTRING
   return QVariant(val);
#else
   if (val.isEmpty()) {
      return QVariant(QTime());
   }
   return QVariant(QTime::fromString(val, Qt::ISODate));
#endif
}

static inline QVariant qDateTimeFromString(QString &val)
{
#ifdef QT_NO_DATESTRING
   return QVariant(val);
#else
   if (val.isEmpty()) {
      return QVariant(QDateTime());
   }
   if (val.length() == 14)
      // TIMESTAMPS have the format yyyyMMddhhmmss
      val.insert(4, QLatin1Char('-')).insert(7, QLatin1Char('-')).insert(10,
         QLatin1Char('T')).insert(13, QLatin1Char(':')).insert(16, QLatin1Char(':'));
   return QVariant(QDateTime::fromString(val, Qt::ISODate));
#endif
}

class QMYSQLResultPrivate : public QObject
{
   CS_OBJECT(QMYSQLResultPrivate)

 public:
   QMYSQLResultPrivate(const QMYSQLDriver *dp, const QMYSQLResult *d) : driver(dp), result(0), q(d),
      rowsAffected(0), hasBlobs(false), stmt(0), meta(0), inBinds(0), outBinds(0),
      preparedQuery(false) {
      connect(dp, SIGNAL(destroyed()), this, SLOT(driverDestroyed()));
   }

   const QMYSQLDriver *driver;
   MYSQL_RES *result;
   MYSQL_ROW row;
   const QMYSQLResult *q;

   int rowsAffected;

   bool bindInValues();
   void bindBlobs();

   bool hasBlobs;
   struct QMyField {
      QMyField()
         : outField(0), nullIndicator(false), bufLength(0ul),
           myField(0), type(QVariant::Invalid) {
      }
      char *outField;
      my_bool nullIndicator;
      ulong bufLength;
      MYSQL_FIELD *myField;
      QVariant::Type type;
   };

   QVector<QMyField> fields;

   MYSQL_STMT *stmt;
   MYSQL_RES *meta;

   MYSQL_BIND *inBinds;
   MYSQL_BIND *outBinds;

   bool preparedQuery;

 private:
   SQL_CS_SLOT_1(Private, void driverDestroyed() {
      driver = NULL;
   })
   SQL_CS_SLOT_2(driverDestroyed)

};


#ifndef QT_NO_TEXTCODEC
static QTextCodec *codec(MYSQL *mysql)
{
   QTextCodec *heuristicCodec = QTextCodec::codecForName(mysql_character_set_name(mysql));
   if (heuristicCodec) {
      return heuristicCodec;
   }

   return QTextCodec::codecForLocale();
}

#endif // QT_NO_TEXTCODEC

static QSqlError qMakeError(const QString &err, QSqlError::ErrorType type,
   const QMYSQLDriverPrivate *p)
{
   const char *cerr = p->mysql ? mysql_error(p->mysql) : 0;
   return QSqlError(QLatin1String("QMYSQL: ") + err,
         p->tc ? toUnicode(p->tc, cerr) : QString::fromLatin1(cerr),
         type, mysql_errno(p->mysql));
}


static QVariant::Type qDecodeMYSQLType(int mysqltype, uint flags)
{
   QVariant::Type type;
   switch (mysqltype) {
      case FIELD_TYPE_TINY :
      case FIELD_TYPE_SHORT :
      case FIELD_TYPE_LONG :
      case FIELD_TYPE_INT24 :
         type = (flags & UNSIGNED_FLAG) ? QVariant::UInt : QVariant::Int;
         break;
      case FIELD_TYPE_YEAR :
         type = QVariant::Int;
         break;
      case FIELD_TYPE_LONGLONG :
         type = (flags & UNSIGNED_FLAG) ? QVariant::ULongLong : QVariant::LongLong;
         break;
      case FIELD_TYPE_FLOAT :
      case FIELD_TYPE_DOUBLE :
      case FIELD_TYPE_DECIMAL :
#if defined(FIELD_TYPE_NEWDECIMAL)
      case FIELD_TYPE_NEWDECIMAL:
#endif
         type = QVariant::Double;
         break;
      case FIELD_TYPE_DATE :
         type = QVariant::Date;
         break;
      case FIELD_TYPE_TIME :
         type = QVariant::Time;
         break;
      case FIELD_TYPE_DATETIME :
      case FIELD_TYPE_TIMESTAMP :
         type = QVariant::DateTime;
         break;
      case FIELD_TYPE_STRING :
      case FIELD_TYPE_VAR_STRING :
      case FIELD_TYPE_BLOB :
      case FIELD_TYPE_TINY_BLOB :
      case FIELD_TYPE_MEDIUM_BLOB :
      case FIELD_TYPE_LONG_BLOB :
         type = (flags & BINARY_FLAG) ? QVariant::ByteArray : QVariant::String;
         break;
      default:
      case FIELD_TYPE_ENUM :
      case FIELD_TYPE_SET :
         type = QVariant::String;
         break;
   }
   return type;
}

static QSqlField qToField(MYSQL_FIELD *field, QTextCodec *tc)
{
   QSqlField f(toUnicode(tc, field->name),
      qDecodeMYSQLType(int(field->type), field->flags));
   f.setRequired(IS_NOT_NULL(field->flags));
   f.setLength(field->length);
   f.setPrecision(field->decimals);
   f.setSqlType(field->type);
   f.setAutoValue(field->flags & AUTO_INCREMENT_FLAG);
   return f;
}

static QSqlError qMakeStmtError(const QString &err, QSqlError::ErrorType type,
   MYSQL_STMT *stmt)
{
   const char *cerr = mysql_stmt_error(stmt);
   return QSqlError(QLatin1String("QMYSQL3: ") + err,
         QString::fromLatin1(cerr),
         type, mysql_stmt_errno(stmt));
}

static bool qIsBlob(int t)
{
   return t == MYSQL_TYPE_TINY_BLOB
      || t == MYSQL_TYPE_BLOB
      || t == MYSQL_TYPE_MEDIUM_BLOB
      || t == MYSQL_TYPE_LONG_BLOB;
}

static bool qIsInteger(int t)
{
   return t == MYSQL_TYPE_TINY
      || t == MYSQL_TYPE_SHORT
      || t == MYSQL_TYPE_LONG
      || t == MYSQL_TYPE_LONGLONG
      || t == MYSQL_TYPE_INT24;
}

void QMYSQLResultPrivate::bindBlobs()
{
   int i;
   MYSQL_FIELD *fieldInfo;
   MYSQL_BIND *bind;

   for (i = 0; i < fields.count(); ++i) {
      fieldInfo = fields.at(i).myField;
      if (qIsBlob(inBinds[i].buffer_type) && meta && fieldInfo) {
         bind = &inBinds[i];
         bind->buffer_length = fieldInfo->max_length;
         delete[] static_cast<char *>(bind->buffer);
         bind->buffer = new char[fieldInfo->max_length];
         fields[i].outField = static_cast<char *>(bind->buffer);
      }
   }
}

bool QMYSQLResultPrivate::bindInValues()
{
   MYSQL_BIND *bind;
   char *field;
   int i = 0;

   if (!meta) {
      meta = mysql_stmt_result_metadata(stmt);
   }
   if (!meta) {
      return false;
   }

   fields.resize(mysql_num_fields(meta));

   inBinds = new MYSQL_BIND[fields.size()];
   memset(inBinds, 0, fields.size() * sizeof(MYSQL_BIND));

   MYSQL_FIELD *fieldInfo;

   while ((fieldInfo = mysql_fetch_field(meta))) {
      QMyField &f = fields[i];
      f.myField = fieldInfo;

      f.type = qDecodeMYSQLType(fieldInfo->type, fieldInfo->flags);
      if (qIsBlob(fieldInfo->type)) {
         // the size of a blob-field is available as soon as we call
         // mysql_stmt_store_result()
         // after mysql_stmt_exec() in QMYSQLResult::exec()
         fieldInfo->length = 0;
         hasBlobs = true;
      } else {
         // fieldInfo->length specifies the display width, which may be too
         // small to hold valid integer values (see
         // http://dev.mysql.com/doc/refman/5.0/en/numeric-types.html ), so
         // always use the MAX_BIGINT_WIDTH for integer types
         if (qIsInteger(fieldInfo->type)) {
            fieldInfo->length = MAX_BIGINT_WIDTH;
         }
         fieldInfo->type = MYSQL_TYPE_STRING;
      }
      bind = &inBinds[i];
      field = new char[fieldInfo->length + 1];
      memset(field, 0, fieldInfo->length + 1);

      bind->buffer_type = fieldInfo->type;
      bind->buffer = field;
      bind->buffer_length = f.bufLength = fieldInfo->length + 1;
      bind->is_null = &f.nullIndicator;
      bind->length = &f.bufLength;
      f.outField = field;

      ++i;
   }
   return true;
}

QMYSQLResult::QMYSQLResult(const QMYSQLDriver *db)
   : QSqlResult(db)
{
   d = new QMYSQLResultPrivate(db, this);
}

QMYSQLResult::~QMYSQLResult()
{
   cleanup();
   delete d;
}

QVariant QMYSQLResult::handle() const
{

   if (d->preparedQuery) {
      return d->meta ? QVariant::fromValue(d->meta) : QVariant::fromValue(d->stmt);

   } else {
      return QVariant::fromValue(d->result);
   }
}

void QMYSQLResult::cleanup()
{
   if (d->result) {
      mysql_free_result(d->result);
   }

   // must iterate trough leftover result sets from multi-selects or stored procedures
   // if this isn't done subsequent queries will fail with "Commands out of sync"

   while (d->driver && d->driver->d->mysql && mysql_next_result(d->driver->d->mysql) == 0) {
      MYSQL_RES *res = mysql_store_result(d->driver->d->mysql);
      if (res) {
         mysql_free_result(res);
      }
   }

   if (d->stmt) {
      if (mysql_stmt_close(d->stmt)) {
         qWarning("QMYSQLResult::cleanup: unable to free statement handle");
      }
      d->stmt = 0;
   }

   if (d->meta) {
      mysql_free_result(d->meta);
      d->meta = 0;
   }

   int i;
   for (i = 0; i < d->fields.count(); ++i) {
      delete[] d->fields[i].outField;
   }

   if (d->outBinds) {
      delete[] d->outBinds;
      d->outBinds = 0;
   }

   if (d->inBinds) {
      delete[] d->inBinds;
      d->inBinds = 0;
   }

   d->hasBlobs = false;
   d->fields.clear();
   d->result = NULL;
   d->row = NULL;
   setAt(-1);
   setActive(false);
}

bool QMYSQLResult::fetch(int i)
{
   if (!d->driver) {
      return false;
   }
   if (isForwardOnly()) { // fake a forward seek
      if (at() < i) {
         int x = i - at();
         while (--x && fetchNext()) {};
         return fetchNext();
      } else {
         return false;
      }
   }
   if (at() == i) {
      return true;
   }
   if (d->preparedQuery) {

      mysql_stmt_data_seek(d->stmt, i);

      int nRC = mysql_stmt_fetch(d->stmt);
      if (nRC) {

#ifdef MYSQL_DATA_TRUNCATED
         if (nRC == 1 || nRC == MYSQL_DATA_TRUNCATED)
#else
         if (nRC == 1)
#endif
            setLastError(qMakeStmtError(QCoreApplication::translate("QMYSQLResult",
                     "Unable to fetch data"), QSqlError::StatementError, d->stmt));
         return false;
      }

   } else {
      mysql_data_seek(d->result, i);
      d->row = mysql_fetch_row(d->result);
      if (!d->row) {
         return false;
      }
   }

   setAt(i);
   return true;
}

bool QMYSQLResult::fetchNext()
{
   if (!d->driver) {
      return false;
   }
   if (d->preparedQuery) {
      int nRC = mysql_stmt_fetch(d->stmt);

      if (nRC) {
#ifdef MYSQL_DATA_TRUNCATED
         if (nRC == 1 || nRC == MYSQL_DATA_TRUNCATED)
#else
         if (nRC == 1)
#endif // MYSQL_DATA_TRUNCATED
            setLastError(qMakeStmtError(QCoreApplication::translate("QMYSQLResult",
                     "Unable to fetch data"), QSqlError::StatementError, d->stmt));
         return false;
      }

   } else {
      d->row = mysql_fetch_row(d->result);
      if (!d->row) {
         return false;
      }
   }
   setAt(at() + 1);
   return true;
}

bool QMYSQLResult::fetchLast()
{
   if (!d->driver) {
      return false;
   }
   if (isForwardOnly()) { // fake this since MySQL can't seek on forward only queries
      bool success = fetchNext(); // did we move at all?
      while (fetchNext()) {};
      return success;
   }

   my_ulonglong numRows;
   if (d->preparedQuery) {
      numRows = mysql_stmt_num_rows(d->stmt);

   } else {
      numRows = mysql_num_rows(d->result);
   }
   if (at() == int(numRows)) {
      return true;
   }
   if (!numRows) {
      return false;
   }
   return fetch(numRows - 1);
}

bool QMYSQLResult::fetchFirst()
{
   if (at() == 0) {
      return true;
   }

   if (isForwardOnly()) {
      return (at() == QSql::BeforeFirstRow) ? fetchNext() : false;
   }
   return fetch(0);
}

QVariant QMYSQLResult::data(int field)
{

   if (!isSelect() || field >= d->fields.count()) {
      qWarning("QMYSQLResult::data: column %d out of range", field);
      return QVariant();
   }

   if (!d->driver) {
      return QVariant();
   }

   int fieldLength = 0;
   const QMYSQLResultPrivate::QMyField &f = d->fields.at(field);
   QString val;
   if (d->preparedQuery) {
      if (f.nullIndicator) {
         return QVariant(f.type);
      }

      if (f.type != QVariant::ByteArray) {
         val = toUnicode(d->driver->d->tc, f.outField, f.bufLength);
      }
   } else {
      if (d->row[field] == NULL) {
         // NULL value
         return QVariant(f.type);
      }
      fieldLength = mysql_fetch_lengths(d->result)[field];
      if (f.type != QVariant::ByteArray) {
         val = toUnicode(d->driver->d->tc, d->row[field], fieldLength);
      }
   }

   switch (f.type) {
      case QVariant::LongLong:
         return QVariant(val.toInteger<qint64>());

      case QVariant::ULongLong:
         return QVariant(val.toInteger<quint64>());

      case QVariant::Int:
         return QVariant(val.toInteger<int>());

      case QVariant::UInt:
         return QVariant(val.toInteger<uint>());

      case QVariant::Double: {
         QVariant v;
         bool ok = false;
         double dbl = val.toDouble(&ok);
         switch (numericalPrecisionPolicy()) {
            case QSql::LowPrecisionInt32:
               v = QVariant(dbl).toInt();
               break;
            case QSql::LowPrecisionInt64:
               v = QVariant(dbl).toLongLong();
               break;
            case QSql::LowPrecisionDouble:
               v = QVariant(dbl);
               break;
            case QSql::HighPrecision:
            default:
               v = val;
               ok = true;
               break;
         }
         if (ok) {
            return v;
         } else {
            return QVariant();
         }
      }
      return QVariant(val.toDouble());
      case QVariant::Date:
         return qDateFromString(val);
      case QVariant::Time:
         return qTimeFromString(val);
      case QVariant::DateTime:
         return qDateTimeFromString(val);
      case QVariant::ByteArray: {

         QByteArray ba;
         if (d->preparedQuery) {
            ba = QByteArray(f.outField, f.bufLength);
         } else {
            ba = QByteArray(d->row[field], fieldLength);
         }
         return QVariant(ba);
      }
      default:
      case QVariant::String:
         return QVariant(val);
   }
   qWarning("QMYSQLResult::data: unknown data type");
   return QVariant();
}

bool QMYSQLResult::isNull(int field)
{
   if (d->preparedQuery) {
      return d->fields.at(field).nullIndicator;
   } else {
      return d->row[field] == NULL;
   }
}

bool QMYSQLResult::reset (const QString &query)
{
   if (!driver() || !driver()->isOpen() || driver()->isOpenError() || !d->driver) {
      return false;
   }

   d->preparedQuery = false;

   cleanup();

   const QByteArray encQuery(fromUnicode(d->driver->d->tc, query));
   if (mysql_real_query(d->driver->d->mysql, encQuery.data(), encQuery.length())) {
      setLastError(qMakeError(QCoreApplication::translate("QMYSQLResult", "Unable to execute query"),
            QSqlError::StatementError, d->driver->d));
      return false;
   }
   d->result = mysql_store_result(d->driver->d->mysql);
   if (!d->result && mysql_field_count(d->driver->d->mysql) > 0) {
      setLastError(qMakeError(QCoreApplication::translate("QMYSQLResult", "Unable to store result"),
            QSqlError::StatementError, d->driver->d));
      return false;
   }
   int numFields = mysql_field_count(d->driver->d->mysql);
   setSelect(numFields != 0);
   d->fields.resize(numFields);
   d->rowsAffected = mysql_affected_rows(d->driver->d->mysql);

   if (isSelect()) {
      for (int i = 0; i < numFields; i++) {
         MYSQL_FIELD *field = mysql_fetch_field_direct(d->result, i);
         d->fields[i].type = qDecodeMYSQLType(field->type, field->flags);
      }
      setAt(QSql::BeforeFirstRow);
   }
   setActive(true);
   return isActive();
}

int QMYSQLResult::size()
{
   if (d->driver && isSelect())
      if (d->preparedQuery) {
         return mysql_stmt_num_rows(d->stmt);
      }

      else {
         return int(mysql_num_rows(d->result));
      } else {
      return -1;
   }
}

int QMYSQLResult::numRowsAffected()
{
   return d->rowsAffected;
}

QVariant QMYSQLResult::lastInsertId() const
{
   if (!isActive() || !d->driver) {
      return QVariant();
   }

   if (d->preparedQuery) {

      quint64 id = mysql_stmt_insert_id(d->stmt);
      if (id) {
         return QVariant(id);
      }

   } else {
      quint64 id = mysql_insert_id(d->driver->d->mysql);
      if (id) {
         return QVariant(id);
      }
   }
   return QVariant();
}

QSqlRecord QMYSQLResult::record() const
{
   QSqlRecord info;
   MYSQL_RES *res;

   if (!isActive() || !isSelect() || !d->driver) {
      return info;
   }

   res = d->preparedQuery ? d->meta : d->result;

   if (!mysql_errno(d->driver->d->mysql)) {
      mysql_field_seek(res, 0);
      MYSQL_FIELD *field = mysql_fetch_field(res);
      while (field) {
         info.append(qToField(field, d->driver->d->tc));
         field = mysql_fetch_field(res);
      }
   }
   mysql_field_seek(res, 0);
   return info;
}

bool QMYSQLResult::nextResult()
{
   if (!d->driver) {
      return false;
   }

   setAt(-1);
   setActive(false);

   if (d->result && isSelect()) {
      mysql_free_result(d->result);
   }
   d->result = 0;
   setSelect(false);

   for (int i = 0; i < d->fields.count(); ++i) {
      delete[] d->fields[i].outField;
   }
   d->fields.clear();

   int status = mysql_next_result(d->driver->d->mysql);
   if (status > 0) {
      setLastError(qMakeError(QCoreApplication::translate("QMYSQLResult", "Unable to execute next query"),
            QSqlError::StatementError, d->driver->d));
      return false;
   } else if (status == -1) {
      return false;   // No more result sets
   }

   d->result = mysql_store_result(d->driver->d->mysql);
   int numFields = mysql_field_count(d->driver->d->mysql);
   if (!d->result && numFields > 0) {
      setLastError(qMakeError(QCoreApplication::translate("QMYSQLResult", "Unable to store next result"),
            QSqlError::StatementError, d->driver->d));
      return false;
   }

   setSelect(numFields > 0);
   d->fields.resize(numFields);
   d->rowsAffected = mysql_affected_rows(d->driver->d->mysql);

   if (isSelect()) {
      for (int i = 0; i < numFields; i++) {
         MYSQL_FIELD *field = mysql_fetch_field_direct(d->result, i);
         d->fields[i].type = qDecodeMYSQLType(field->type, field->flags);
      }
   }

   setActive(true);

   return true;
}

void QMYSQLResult::virtual_hook(int id, void *data)
{
   switch (id) {
      case QSqlResult::NextResult:
         Q_ASSERT(data);
         *static_cast<bool *>(data) = nextResult();
         break;
      default:
         QSqlResult::virtual_hook(id, data);
   }
}



static MYSQL_TIME *toMySqlDate(QDate date, QTime time, QVariant::Type type)
{
   Q_ASSERT(type == QVariant::Time || type == QVariant::Date
      || type == QVariant::DateTime);

   MYSQL_TIME *myTime = new MYSQL_TIME;
   memset(myTime, 0, sizeof(MYSQL_TIME));

   if (type == QVariant::Time || type == QVariant::DateTime) {
      myTime->hour = time.hour();
      myTime->minute = time.minute();
      myTime->second = time.second();
      myTime->second_part = time.msec();
   }
   if (type == QVariant::Date || type == QVariant::DateTime) {
      myTime->year = date.year();
      myTime->month = date.month();
      myTime->day = date.day();
   }

   return myTime;
}

bool QMYSQLResult::prepare(const QString &query)
{
   if (!d->driver) {
      return false;
   }

   cleanup();
   if (!d->driver->d->preparedQuerysEnabled) {
      return QSqlResult::prepare(query);
   }

   int r;

   if (query.isEmpty()) {
      return false;
   }

   if (!d->stmt) {
      d->stmt = mysql_stmt_init(d->driver->d->mysql);
   }
   if (!d->stmt) {
      setLastError(qMakeError(QCoreApplication::translate("QMYSQLResult", "Unable to prepare statement"),
            QSqlError::StatementError, d->driver->d));
      return false;
   }

   const QByteArray encQuery(fromUnicode(d->driver->d->tc, query));
   r = mysql_stmt_prepare(d->stmt, encQuery.constData(), encQuery.length());
   if (r != 0) {
      setLastError(qMakeStmtError(QCoreApplication::translate("QMYSQLResult",
               "Unable to prepare statement"), QSqlError::StatementError, d->stmt));
      cleanup();
      return false;
   }

   if (mysql_stmt_param_count(d->stmt) > 0) {// allocate memory for outvalues
      d->outBinds = new MYSQL_BIND[mysql_stmt_param_count(d->stmt)];
   }

   setSelect(d->bindInValues());
   d->preparedQuery = true;
   return true;

}

bool QMYSQLResult::exec()
{
   if (!d->driver) {
      return false;
   }
   if (!d->preparedQuery) {
      return QSqlResult::exec();
   }
   if (!d->stmt) {
      return false;
   }

   int r = 0;
   MYSQL_BIND *currBind;
   QVector<MYSQL_TIME *> timeVector;
   QVector<QByteArray> stringVector;
   QVector<my_bool> nullVector;

   const QVector<QVariant> values = boundValues();

   r = mysql_stmt_reset(d->stmt);
   if (r != 0) {
      setLastError(qMakeStmtError(QCoreApplication::translate("QMYSQLResult",
               "Unable to reset statement"), QSqlError::StatementError, d->stmt));
      return false;
   }

   if (mysql_stmt_param_count(d->stmt) > 0 &&
      mysql_stmt_param_count(d->stmt) == (uint)values.count()) {

      nullVector.resize(values.count());
      for (int i = 0; i < values.count(); ++i) {
         const QVariant &val = boundValues().at(i);
         void *data = const_cast<void *>(val.constData());

         currBind = &d->outBinds[i];

         nullVector[i] = static_cast<my_bool>(val.isNull());
         currBind->is_null = &nullVector[i];
         currBind->length = 0;
         currBind->is_unsigned = 0;

         switch (val.type()) {
            case QVariant::ByteArray:
               currBind->buffer_type = MYSQL_TYPE_BLOB;
               currBind->buffer = const_cast<char *>(val.toByteArray().constData());
               currBind->buffer_length = val.toByteArray().size();
               break;

            case QVariant::Time:
            case QVariant::Date:
            case QVariant::DateTime: {
               MYSQL_TIME *myTime = toMySqlDate(val.toDate(), val.toTime(), val.type());
               timeVector.append(myTime);

               currBind->buffer = myTime;
               switch (val.type()) {
                  case QVariant::Time:
                     currBind->buffer_type = MYSQL_TYPE_TIME;
                     myTime->time_type = MYSQL_TIMESTAMP_TIME;
                     break;
                  case QVariant::Date:
                     currBind->buffer_type = MYSQL_TYPE_DATE;
                     myTime->time_type = MYSQL_TIMESTAMP_DATE;
                     break;
                  case QVariant::DateTime:
                     currBind->buffer_type = MYSQL_TYPE_DATETIME;
                     myTime->time_type = MYSQL_TIMESTAMP_DATETIME;
                     break;
                  default:
                     break;
               }
               currBind->buffer_length = sizeof(MYSQL_TIME);
               currBind->length = 0;
               break;
            }
            case QVariant::UInt:
            case QVariant::Int:
               currBind->buffer_type = MYSQL_TYPE_LONG;
               currBind->buffer = data;
               currBind->buffer_length = sizeof(int);
               currBind->is_unsigned = (val.type() != QVariant::Int);
               break;

            case QVariant::Bool:
               currBind->buffer_type = MYSQL_TYPE_TINY;
               currBind->buffer = data;
               currBind->buffer_length = sizeof(bool);
               currBind->is_unsigned = false;
               break;

            case QVariant::Double:
               currBind->buffer_type = MYSQL_TYPE_DOUBLE;
               currBind->buffer = data;
               currBind->buffer_length = sizeof(double);
               break;

            case QVariant::LongLong:
            case QVariant::ULongLong:
               currBind->buffer_type = MYSQL_TYPE_LONGLONG;
               currBind->buffer = data;
               currBind->buffer_length = sizeof(qint64);
               currBind->is_unsigned = (val.type() == QVariant::ULongLong);
               break;

            case QVariant::String:
            default: {
               QByteArray ba = fromUnicode(d->driver->d->tc, val.toString());
               stringVector.append(ba);
               currBind->buffer_type = MYSQL_TYPE_STRING;
               currBind->buffer = const_cast<char *>(ba.constData());
               currBind->buffer_length = ba.length();
               break;
            }
         }
      }

      r = mysql_stmt_bind_param(d->stmt, d->outBinds);
      if (r != 0) {
         setLastError(qMakeStmtError(QCoreApplication::translate("QMYSQLResult",
                  "Unable to bind value"), QSqlError::StatementError, d->stmt));
         qDeleteAll(timeVector);
         return false;
      }
   }
   r = mysql_stmt_execute(d->stmt);

   qDeleteAll(timeVector);

   if (r != 0) {
      setLastError(qMakeStmtError(QCoreApplication::translate("QMYSQLResult",
               "Unable to execute statement"), QSqlError::StatementError, d->stmt));
      return false;
   }
   //if there is meta-data there is also data
   setSelect(d->meta);

   d->rowsAffected = mysql_stmt_affected_rows(d->stmt);

   if (isSelect()) {
      my_bool update_max_length = true;

      r = mysql_stmt_bind_result(d->stmt, d->inBinds);
      if (r != 0) {
         setLastError(qMakeStmtError(QCoreApplication::translate("QMYSQLResult",
                  "Unable to bind outvalues"), QSqlError::StatementError, d->stmt));
         return false;
      }
      if (d->hasBlobs) {
         mysql_stmt_attr_set(d->stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &update_max_length);
      }

      r = mysql_stmt_store_result(d->stmt);
      if (r != 0) {
         setLastError(qMakeStmtError(QCoreApplication::translate("QMYSQLResult",
                  "Unable to store statement results"), QSqlError::StatementError, d->stmt));
         return false;
      }

      if (d->hasBlobs) {
         // mysql_stmt_store_result() with STMT_ATTR_UPDATE_MAX_LENGTH set to true crashes
         // when called without a preceding call to mysql_stmt_bind_result() in versions < 4.1.8
         d->bindBlobs();

         r = mysql_stmt_bind_result(d->stmt, d->inBinds);
         if (r != 0) {
            setLastError(qMakeStmtError(QCoreApplication::translate("QMYSQLResult",
                     "Unable to bind outvalues"), QSqlError::StatementError, d->stmt));
            return false;
         }
      }
      setAt(QSql::BeforeFirstRow);
   }
   setActive(true);
   return true;
}


static int qMySqlConnectionCount = 0;
static bool qMySqlInitHandledByUser = false;

static void qLibraryInit()
{

#ifndef Q_NO_MYSQL_EMBEDDED
# if MYSQL_VERSION_ID >= 40000
   if (qMySqlInitHandledByUser || qMySqlConnectionCount > 1) {
      return;
   }

# if (MYSQL_VERSION_ID >= 40110 && MYSQL_VERSION_ID < 50000) || MYSQL_VERSION_ID >= 50003
   if (mysql_library_init(0, 0, 0)) {
# else
   if (mysql_server_init(0, 0, 0)) {
# endif
      qWarning("QMYSQLDriver::qServerInit: unable to start server.");
   }
# endif // MYSQL_VERSION_ID
#endif
}

static void qLibraryEnd()
{
#ifndef Q_NO_MYSQL_EMBEDDED
   mysql_library_end();
#endif
}

QMYSQLDriver::QMYSQLDriver(QObject *parent)
   : QSqlDriver(parent)
{
   init();
   qLibraryInit();
}

QMYSQLDriver::QMYSQLDriver(MYSQL *con, QObject *parent)
   : QSqlDriver(parent)
{
   init();
   if (con) {
      d->mysql = (MYSQL *) con;
#ifndef QT_NO_TEXTCODEC
      d->tc = codec(con);
#endif
      setOpen(true);
      setOpenError(false);
      if (qMySqlConnectionCount == 1) {
         qMySqlInitHandledByUser = true;
      }
   } else {
      qLibraryInit();
   }
}

void QMYSQLDriver::init()
{
   d = new QMYSQLDriverPrivate();
   d->mysql = 0;
   qMySqlConnectionCount++;
}

QMYSQLDriver::~QMYSQLDriver()
{
   qMySqlConnectionCount--;
   if (qMySqlConnectionCount == 0 && !qMySqlInitHandledByUser) {
      qLibraryEnd();
   }
   delete d;
}

bool QMYSQLDriver::hasFeature(DriverFeature f) const
{
   switch (f) {
      case Transactions:
         // CLIENT_TRANSACTION should be defined in all recent mysql client libs > 3.23.34

#ifdef CLIENT_TRANSACTIONS
         if (d->mysql) {
            if ((d->mysql->server_capabilities & CLIENT_TRANSACTIONS) == CLIENT_TRANSACTIONS) {
               return true;
            }
         }
#endif
         return false;
      case NamedPlaceholders:
      case BatchOperations:
      case SimpleLocking:
      case EventNotifications:
      case FinishQuery:
         return false;
      case QuerySize:
      case BLOB:
      case LastInsertId:
      case Unicode:
      case LowPrecisionNumbers:
         return true;

      case PreparedQueries:
      case PositionalPlaceholders:
         return d->preparedQuerysEnabled;

      case MultipleResultSets:
         return true;

   }

   return false;
}

static void setOptionFlag(uint &optionFlags, const QString &opt)
{
   if (opt == QLatin1String("CLIENT_COMPRESS")) {
      optionFlags |= CLIENT_COMPRESS;

   } else if (opt == QLatin1String("CLIENT_FOUND_ROWS")) {
      optionFlags |= CLIENT_FOUND_ROWS;

   } else if (opt == QLatin1String("CLIENT_IGNORE_SPACE")) {
      optionFlags |= CLIENT_IGNORE_SPACE;

   } else if (opt == QLatin1String("CLIENT_INTERACTIVE")) {
      optionFlags |= CLIENT_INTERACTIVE;

   } else if (opt == QLatin1String("CLIENT_NO_SCHEMA")) {
      optionFlags |= CLIENT_NO_SCHEMA;

   } else if (opt == QLatin1String("CLIENT_ODBC")) {
      optionFlags |= CLIENT_ODBC;

   } else if (opt == QLatin1String("CLIENT_SSL")) {
      optionFlags |= CLIENT_SSL;

   } else {
      qWarning("QMYSQLDriver::open: Unknown connect option '%s'", opt.constData());
   }
}

bool QMYSQLDriver::open(const QString &db, const QString &user, const QString &password,
   const QString &host, int port, const QString &connOpts)
{
   if (isOpen()) {
      close();
   }

   /* This is a hack to get MySQL's stored procedure support working.
      Since a stored procedure _may_ return multiple result sets,
      we have to enable CLIEN_MULTI_STATEMENTS here, otherwise _any_
      stored procedure call will fail.
   */
   unsigned int optionFlags = Q_CLIENT_MULTI_STATEMENTS;
   const QStringList opts(connOpts.split(QLatin1Char(';'), QStringParser::SkipEmptyParts));
   QString unixSocket;

   my_bool reconnect = false;

   // extract the real options from the string
   for (int i = 0; i < opts.count(); ++i) {
      QString tmp(opts.at(i).simplified());

      int idx;
      if ((idx = tmp.indexOf(QLatin1Char('='))) != -1) {
         QString val = tmp.mid(idx + 1).simplified();
         QString opt = tmp.left(idx).simplified();

         if (opt == QLatin1String("UNIX_SOCKET")) {
            unixSocket = val;
         }

         else if (opt == QLatin1String("MYSQL_OPT_RECONNECT")) {
            if (val == QLatin1String("TRUE") || val == QLatin1String("1") || val.isEmpty()) {
               reconnect = true;
            }
         }

         else if (val == QLatin1String("TRUE") || val == QLatin1String("1")) {
            setOptionFlag(optionFlags, tmp.left(idx).simplified());
         } else {
            qWarning("QMYSQLDriver::open: Illegal connect option value '%s'", tmp.constData());
         }

      } else {
         setOptionFlag(optionFlags, tmp);
      }
   }

   if ((d->mysql = mysql_init((MYSQL *) 0)) &&
      mysql_real_connect(d->mysql,
         host.isEmpty() ? static_cast<const char *>(0) : host.constData(),
         user.isEmpty() ? static_cast<const char *>(0) : user.constData(),
         password.isEmpty() ? static_cast<const char *>(0) : password.constData(),
         db.isEmpty() ? static_cast<const char *>(0) : db.constData(),
         (port > -1) ? port : 0,
         unixSocket.isEmpty() ? static_cast<const char *>(0) : unixSocket.constData(),
         optionFlags)) {

      if (!db.isEmpty() && mysql_select_db(d->mysql, db.constData())) {
         setLastError(qMakeError(tr("Unable to open database '") + db + '\'', QSqlError::ConnectionError, d));
         mysql_close(d->mysql);
         setOpenError(true);
         return false;
      }

      if (reconnect) {
         mysql_options(d->mysql, MYSQL_OPT_RECONNECT, &reconnect);
      }

   } else {
      setLastError(qMakeError(tr("Unable to connect"), QSqlError::ConnectionError, d));
      mysql_close(d->mysql);
      d->mysql = NULL;
      setOpenError(true);
      return false;
   }

   // force the communication to be utf8
   mysql_set_character_set(d->mysql, "utf8");

#ifndef QT_NO_TEXTCODEC
   d->tc = codec(d->mysql);
#endif

   d->preparedQuerysEnabled = true;

   mysql_thread_init();

   setOpen(true);
   setOpenError(false);
   return true;
}

void QMYSQLDriver::close()
{
   if (isOpen()) {
      mysql_thread_end();
      mysql_close(d->mysql);
      d->mysql = NULL;
      setOpen(false);
      setOpenError(false);
   }
}

QSqlResult *QMYSQLDriver::createResult() const
{
   return new QMYSQLResult(this);
}

QStringList QMYSQLDriver::tables(QSql::TableType type) const
{
   QStringList tl;

   if ( mysql_get_server_version(d->mysql) < 50000) {

      if (!isOpen()) {
         return tl;
      }
      if (!(type & QSql::Tables)) {
         return tl;
      }

      MYSQL_RES *tableRes = mysql_list_tables(d->mysql, NULL);
      MYSQL_ROW row;
      int i = 0;
      while (tableRes) {
         mysql_data_seek(tableRes, i);
         row = mysql_fetch_row(tableRes);
         if (!row) {
            break;
         }
         tl.append(toUnicode(d->tc, row[0]));
         i++;
      }
      mysql_free_result(tableRes);


   } else {
      QSqlQuery q(createResult());

      if (type & QSql::Tables) {
         QString sql = "select table_name from information_schema.tables where table_schema = '" +
            QString::fromUtf8(d->mysql->db) + "' and table_type = 'BASE TABLE'";
         q.exec(sql);

         while (q.next()) {
            tl.append(q.value(0).toString());
         }
      }

      if (type & QSql::Views) {
         QString sql = "select table_name from information_schema.tables where table_schema = '" +
            QString::fromUtf8(d->mysql->db) + "' and table_type = 'VIEW'";
         q.exec(sql);

         while (q.next()) {
            tl.append(q.value(0).toString());
         }
      }
   }


   return tl;
}

QSqlIndex QMYSQLDriver::primaryIndex(const QString &tablename) const
{
   QSqlIndex idx;
   if (!isOpen()) {
      return idx;
   }

   QSqlQuery i(createResult());
   QString stmt("show index from %1;");

   QSqlRecord fil = record(tablename);
   i.exec(stmt.formatArg(tablename));

   while (i.isActive() && i.next()) {
      if (i.value(2).toString() == "PRIMARY") {
         idx.append(fil.field(i.value(4).toString()));
         idx.setCursorName(i.value(0).toString());
         idx.setName(i.value(2).toString());
      }
   }

   return idx;
}

QSqlRecord QMYSQLDriver::record(const QString &tablename) const
{
   QString table = tablename;

   if (isIdentifierEscaped(table, QSqlDriver::TableName)) {
      table = stripDelimiters(table, QSqlDriver::TableName);
   }

   QSqlRecord info;
   if (!isOpen()) {
      return info;
   }
   MYSQL_RES *r = mysql_list_fields(d->mysql, table.constData(), 0);
   if (!r) {
      return info;
   }
   MYSQL_FIELD *field;

   while ((field = mysql_fetch_field(r))) {
      info.append(qToField(field, d->tc));
   }
   mysql_free_result(r);
   return info;
}

QVariant QMYSQLDriver::handle() const
{
   return QVariant::fromValue(d->mysql);
}

bool QMYSQLDriver::beginTransaction()
{
#ifndef CLIENT_TRANSACTIONS
   return false;
#endif
   if (!isOpen()) {
      qWarning("QMYSQLDriver::beginTransaction: Database not open");
      return false;
   }
   if (mysql_query(d->mysql, "BEGIN WORK")) {
      setLastError(qMakeError(tr("Unable to begin transaction"), QSqlError::StatementError, d));
      return false;
   }

   return true;
}

bool QMYSQLDriver::commitTransaction()
{
#ifndef CLIENT_TRANSACTIONS
   return false;
#endif
   if (!isOpen()) {
      qWarning("QMYSQLDriver::commitTransaction: Database not open");
      return false;
   }
   if (mysql_query(d->mysql, "COMMIT")) {
      setLastError(qMakeError(tr("Unable to commit transaction"),
            QSqlError::StatementError, d));
      return false;
   }
   return true;
}

bool QMYSQLDriver::rollbackTransaction()
{
#ifndef CLIENT_TRANSACTIONS
   return false;
#endif

   if (!isOpen()) {
      qWarning("QMYSQLDriver::rollbackTransaction: Database not open");
      return false;
   }

   if (mysql_query(d->mysql, "ROLLBACK")) {
      setLastError(qMakeError(tr("Unable to rollback transaction"), QSqlError::StatementError, d));
      return false;
   }
   return true;
}

QString QMYSQLDriver::formatValue(const QSqlField &field, bool trimStrings) const
{
   QString r;

   if (field.isNull()) {
      r = QLatin1String("NULL");
   } else {
      switch (field.type()) {
         case QVariant::String:
            // Escape '\' characters
            r = QSqlDriver::formatValue(field, trimStrings);
            r.replace(QLatin1String("\\"), QLatin1String("\\\\"));
            break;

         case QVariant::ByteArray:
            if (isOpen()) {
               const QByteArray ba = field.value().toByteArray();
               // buffer has to be at least length*2+1 bytes
               char *buffer = new char[ba.size() * 2 + 1];

               int escapedSize = int(mysql_real_escape_string(d->mysql, buffer, ba.data(), ba.size()));

               r.append('\'').append(toUnicode(d->tc, buffer)).append('\'');
               delete[] buffer;
               break;

            } else {
               qWarning("QMYSQLDriver::formatValue: Database not open");
            }

         // fall through
         default:
            r = QSqlDriver::formatValue(field, trimStrings);
      }
   }
   return r;
}

QString QMYSQLDriver::escapeIdentifier(const QString &identifier, IdentifierType) const
{
   QString res = identifier;
   if (!identifier.isEmpty() && !identifier.startsWith(QLatin1Char('`')) && !identifier.endsWith(QLatin1Char('`')) ) {
      res.prepend(QLatin1Char('`')).append(QLatin1Char('`'));
      res.replace(QLatin1Char('.'), QLatin1String("`.`"));
   }
   return res;
}

bool QMYSQLDriver::isIdentifierEscapedImplementation(const QString &identifier, IdentifierType type) const
{
   Q_UNUSED(type);
   return identifier.size() > 2
      && identifier.startsWith(QLatin1Char('`')) //left delimited
      && identifier.endsWith(QLatin1Char('`')); //right delimited
}


