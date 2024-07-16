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

#include <qsqlquery.h>

#include <qatomic.h>
#include <qsqlrecord.h>
#include <qsqlresult.h>
#include <qsqldriver.h>
#include <qsqldatabase.h>
#include <qsqlnulldriver_p.h>
#include <qvector.h>
#include <qmap.h>

class QSqlQueryPrivate
{
 public:
   QSqlQueryPrivate(QSqlResult *result);
   ~QSqlQueryPrivate();

   QAtomicInt ref;
   QSqlResult *sqlResult;

   static QSqlQueryPrivate *shared_null();
};

static QSqlQueryPrivate *nullQueryPrivate()
{
   static QSqlQueryPrivate retval(nullptr);
   return &retval;
}

static QSqlNullDriver *nullDriver()
{
   static QSqlNullDriver retval;
   return &retval;
}

static QSqlNullResult *nullResult()
{
   static QSqlNullResult retval(nullDriver());
   return &retval;
}

QSqlQueryPrivate *QSqlQueryPrivate::shared_null()
{
   QSqlQueryPrivate *null = nullQueryPrivate();
   null->ref.ref();
   return null;
}

QSqlQueryPrivate::QSqlQueryPrivate(QSqlResult *result)
   : ref(1), sqlResult(result)
{
   if (!sqlResult) {
      sqlResult = nullResult();
   }
}

QSqlQueryPrivate::~QSqlQueryPrivate()
{
   QSqlResult *nr = nullResult();
   if (!nr || sqlResult == nr) {
      return;
   }
   delete sqlResult;
}

QSqlQuery::QSqlQuery(QSqlResult *result)
{
   d = new QSqlQueryPrivate(result);
}

QSqlQuery::~QSqlQuery()
{
   if (!d->ref.deref()) {
      delete d;
   }
}

QSqlQuery::QSqlQuery(const QSqlQuery &other)
{
   d = other.d;
   d->ref.ref();
}

/*!
    \internal
*/
static void qInit(QSqlQuery *q, const QString &query, QSqlDatabase db)
{
   QSqlDatabase database = db;
   if (!database.isValid()) {
      database = QSqlDatabase::database(QLatin1String(QSqlDatabase::defaultConnection), false);
   }
   if (database.isValid()) {
      *q = QSqlQuery(database.driver()->createResult());
   }
   if (!query.isEmpty()) {
      q->exec(query);
   }
}

/*!
    Constructs a QSqlQuery object using the SQL \a query and the
    database \a db. If \a db is not specified, or is invalid, the application's
    default database is used. If \a query is not an empty string, it
    will be executed.

    \sa QSqlDatabase
*/
QSqlQuery::QSqlQuery(const QString &query, QSqlDatabase db)
{
   d = QSqlQueryPrivate::shared_null();
   qInit(this, query, db);
}

/*!
    Constructs a QSqlQuery object using the database \a db.
    If \a db is invalid, the application's default database will be used.

    \sa QSqlDatabase
*/

QSqlQuery::QSqlQuery(QSqlDatabase db)
{
   d = QSqlQueryPrivate::shared_null();
   qInit(this, QString(), db);
}


/*!
    Assigns \a other to this object.
*/

QSqlQuery &QSqlQuery::operator=(const QSqlQuery &other)
{
   qAtomicAssign(d, other.d);
   return *this;
}

/*!
  Returns true if the query is \l{isActive()}{active} and positioned
  on a valid record and the \a field is NULL; otherwise returns
  false. Note that for some drivers, isNull() will not return accurate
  information until after an attempt is made to retrieve data.

  \sa isActive(), isValid(), value()
*/

bool QSqlQuery::isNull(int field) const
{
   if (d->sqlResult->isActive() && d->sqlResult->isValid()) {
      return d->sqlResult->isNull(field);
   }
   return true;
}


bool QSqlQuery::isNull(const QString &name) const
{
   int index = d->sqlResult->record().indexOf(name);
   if (index > -1) {
      return isNull(index);
   }
   qWarning("QSqlQuery::isNull: unknown field name '%s'", csPrintable(name));
   return true;
}

bool QSqlQuery::exec(const QString &query)
{
   if (d->ref.load() != 1) {
      bool fo = isForwardOnly();
      *this = QSqlQuery(driver()->createResult());
      d->sqlResult->setNumericalPrecisionPolicy(d->sqlResult->numericalPrecisionPolicy());
      setForwardOnly(fo);

   } else {
      d->sqlResult->clear();
      d->sqlResult->setActive(false);
      d->sqlResult->setLastError(QSqlError());
      d->sqlResult->setAt(QSql::BeforeFirstRow);
      d->sqlResult->setNumericalPrecisionPolicy(d->sqlResult->numericalPrecisionPolicy());
   }

   d->sqlResult->setQuery(query.trimmed());

   if (! driver()->isOpen() || driver()->isOpenError()) {
      qWarning("QSqlQuery::exec: database not open");
      return false;
   }

   if (query.isEmpty()) {
      qWarning("QSqlQuery::exec: empty query");
      return false;
   }



   return d->sqlResult->reset(query);
}

QVariant QSqlQuery::value(int index) const
{
   if (isActive() && isValid() && (index > -1)) {
      return d->sqlResult->data(index);
   }
   qWarning("QSqlQuery::value: not positioned on a valid record");

   return QVariant();
}

QVariant QSqlQuery::value(const QString &name) const
{
   int index = d->sqlResult->record().indexOf(name);

   if (index > -1) {
      return value(index);
   }

   qWarning("QSqlQuery::value: unknown field name '%s'", csPrintable(name));

   return QVariant();
}

int QSqlQuery::at() const
{
   return d->sqlResult->at();
}

QString QSqlQuery::lastQuery() const
{
   return d->sqlResult->lastQuery();
}

/*!
    Returns the database driver associated with the query.
*/

const QSqlDriver *QSqlQuery::driver() const
{
   return d->sqlResult->driver();
}

/*!
    Returns the result associated with the query.
*/

const QSqlResult *QSqlQuery::result() const
{
   return d->sqlResult;
}

/*!
  Retrieves the record at position \a index, if available, and
  positions the query on the retrieved record. The first record is at
  position 0. Note that the query must be in an \l{isActive()}
  {active} state and isSelect() must return true before calling this
  function.

  If \a relative is false (the default), the following rules apply:

  \list

  \o If \a index is negative, the result is positioned before the
  first record and false is returned.

  \o Otherwise, an attempt is made to move to the record at position
  \a index. If the record at position \a index could not be retrieved,
  the result is positioned after the last record and false is
  returned. If the record is successfully retrieved, true is returned.

  \endlist

  If \a relative is true, the following rules apply:

  \list

  \o If the result is currently positioned before the first record or
  on the first record, and \a index is negative, there is no change,
  and false is returned.

  \o If the result is currently located after the last record, and \a
  index is positive, there is no change, and false is returned.

  \o If the result is currently located somewhere in the middle, and
  the relative offset \a index moves the result below zero, the result
  is positioned before the first record and false is returned.

  \o Otherwise, an attempt is made to move to the record \a index
  records ahead of the current record (or \a index records behind the
  current record if \a index is negative). If the record at offset \a
  index could not be retrieved, the result is positioned after the
  last record if \a index >= 0, (or before the first record if \a
  index is negative), and false is returned. If the record is
  successfully retrieved, true is returned.

  \endlist

  \sa next() previous() first() last() at() isActive() isValid()
*/
bool QSqlQuery::seek(int index, bool relative)
{
   if (!isSelect() || !isActive()) {
      return false;
   }
   int actualIdx;
   if (!relative) { // arbitrary seek
      if (index < 0) {
         d->sqlResult->setAt(QSql::BeforeFirstRow);
         return false;
      }
      actualIdx = index;

   } else {
      switch (at()) { // relative seek

         case QSql::BeforeFirstRow:
            if (index > 0) {
               actualIdx = index - 1;
            } else {
               return false;
            }
            break;

         case QSql::AfterLastRow:
            if (index < 0) {
               d->sqlResult->fetchLast();
               actualIdx = at() + index  + 1;
            } else {
               return false;
            }
            break;
         default:
            if ((at() + index) < 0) {
               d->sqlResult->setAt(QSql::BeforeFirstRow);
               return false;
            }
            actualIdx = at() + index;
            break;
      }
   }
   // let drivers optimize
   if (isForwardOnly() && actualIdx < at()) {
      qWarning("QSqlQuery::seek: cannot seek backwards in a forward only query");
      return false;
   }
   if (actualIdx == (at() + 1) && at() != QSql::BeforeFirstRow) {
      if (!d->sqlResult->fetchNext()) {
         d->sqlResult->setAt(QSql::AfterLastRow);
         return false;
      }
      return true;
   }
   if (actualIdx == (at() - 1)) {
      if (!d->sqlResult->fetchPrevious()) {
         d->sqlResult->setAt(QSql::BeforeFirstRow);
         return false;
      }
      return true;
   }
   if (!d->sqlResult->fetch(actualIdx)) {
      d->sqlResult->setAt(QSql::AfterLastRow);
      return false;
   }
   return true;
}

/*!

  Retrieves the next record in the result, if available, and positions
  the query on the retrieved record. Note that the result must be in
  the \l{isActive()}{active} state and isSelect() must return true
  before calling this function or it will do nothing and return false.

  The following rules apply:

  \list

  \o If the result is currently located before the first record,
  e.g. immediately after a query is executed, an attempt is made to
  retrieve the first record.

  \o If the result is currently located after the last record, there
  is no change and false is returned.

  \o If the result is located somewhere in the middle, an attempt is
  made to retrieve the next record.

  \endlist

  If the record could not be retrieved, the result is positioned after
  the last record and false is returned. If the record is successfully
  retrieved, true is returned.

  \sa previous() first() last() seek() at() isActive() isValid()
*/
bool QSqlQuery::next()
{
   if (!isSelect() || !isActive()) {
      return false;
   }
   bool b = false;
   switch (at()) {
      case QSql::BeforeFirstRow:
         b = d->sqlResult->fetchFirst();
         return b;
      case QSql::AfterLastRow:
         return false;
      default:
         if (!d->sqlResult->fetchNext()) {
            d->sqlResult->setAt(QSql::AfterLastRow);
            return false;
         }
         return true;
   }
}

/*!

  Retrieves the previous record in the result, if available, and
  positions the query on the retrieved record. Note that the result
  must be in the \l{isActive()}{active} state and isSelect() must
  return true before calling this function or it will do nothing and
  return false.

  The following rules apply:

  \list

  \o If the result is currently located before the first record, there
  is no change and false is returned.

  \o If the result is currently located after the last record, an
  attempt is made to retrieve the last record.

  \o If the result is somewhere in the middle, an attempt is made to
  retrieve the previous record.

  \endlist

  If the record could not be retrieved, the result is positioned
  before the first record and false is returned. If the record is
  successfully retrieved, true is returned.

  \sa next() first() last() seek() at() isActive() isValid()
*/
bool QSqlQuery::previous()
{
   if (!isSelect() || !isActive()) {
      return false;
   }
   if (isForwardOnly()) {
      qWarning("QSqlQuery::seek: cannot seek backwards in a forward only query");
      return false;
   }

   bool b = false;
   switch (at()) {
      case QSql::BeforeFirstRow:
         return false;
      case QSql::AfterLastRow:
         b = d->sqlResult->fetchLast();
         return b;
      default:
         if (!d->sqlResult->fetchPrevious()) {
            d->sqlResult->setAt(QSql::BeforeFirstRow);
            return false;
         }
         return true;
   }
}

/*!
  Retrieves the first record in the result, if available, and
  positions the query on the retrieved record. Note that the result
  must be in the \l{isActive()}{active} state and isSelect() must
  return true before calling this function or it will do nothing and
  return false.  Returns true if successful. If unsuccessful the query
  position is set to an invalid position and false is returned.

  \sa next() previous() last() seek() at() isActive() isValid()
 */
bool QSqlQuery::first()
{
   if (!isSelect() || !isActive()) {
      return false;
   }
   if (isForwardOnly() && at() > QSql::BeforeFirstRow) {
      qWarning("QSqlQuery::seek: cannot seek backwards in a forward only query");
      return false;
   }
   bool b = false;
   b = d->sqlResult->fetchFirst();
   return b;
}

/*!

  Retrieves the last record in the result, if available, and positions
  the query on the retrieved record. Note that the result must be in
  the \l{isActive()}{active} state and isSelect() must return true
  before calling this function or it will do nothing and return false.
  Returns true if successful. If unsuccessful the query position is
  set to an invalid position and false is returned.

  \sa next() previous() first() seek() at() isActive() isValid()
*/

bool QSqlQuery::last()
{
   if (!isSelect() || !isActive()) {
      return false;
   }
   bool b = false;
   b = d->sqlResult->fetchLast();
   return b;
}

/*!
  Returns the size of the result (number of rows returned), or -1 if
  the size cannot be determined or if the database does not support
  reporting information about query sizes. Note that for non-\c SELECT
  statements (isSelect() returns false), size() will return -1. If the
  query is not active (isActive() returns false), -1 is returned.

  To determine the number of rows affected by a non-\c SELECT
  statement, use numRowsAffected().

  \sa isActive() numRowsAffected() QSqlDriver::hasFeature()
*/
int QSqlQuery::size() const
{
   if (isActive() && d->sqlResult->driver()->hasFeature(QSqlDriver::QuerySize)) {
      return d->sqlResult->size();
   }
   return -1;
}

/*!
  Returns the number of rows affected by the result's SQL statement,
  or -1 if it cannot be determined. Note that for \c SELECT
  statements, the value is undefined; use size() instead. If the query
  is not \l{isActive()}{active}, -1 is returned.

  \sa size() QSqlDriver::hasFeature()
*/

int QSqlQuery::numRowsAffected() const
{
   if (isActive()) {
      return d->sqlResult->numRowsAffected();
   }
   return -1;
}

/*!
  Returns error information about the last error (if any) that
  occurred with this query.

  \sa QSqlError, QSqlDatabase::lastError()
*/

QSqlError QSqlQuery::lastError() const
{
   return d->sqlResult->lastError();
}

/*!
  Returns true if the query is currently positioned on a valid
  record; otherwise returns false.
*/

bool QSqlQuery::isValid() const
{
   return d->sqlResult->isValid();
}

/*!

  Returns true if the query is \e{active}. An active QSqlQuery is one
  that has been \l{QSqlQuery::exec()} {exec()'d} successfully but not
  yet finished with.  When you are finished with an active query, you
  can make make the query inactive by calling finish() or clear(), or
  you can delete the QSqlQuery instance.

  \note Of particular interest is an active query that is a \c{SELECT}
  statement. For some databases that support transactions, an active
  query that is a \c{SELECT} statement can cause a \l{QSqlDatabase::}
  {commit()} or a \l{QSqlDatabase::} {rollback()} to fail, so before
  committing or rolling back, you should make your active \c{SELECT}
  statement query inactive using one of the ways listed above.

  \sa isSelect()
 */
bool QSqlQuery::isActive() const
{
   return d->sqlResult->isActive();
}

/*!
  Returns true if the current query is a \c SELECT statement;
  otherwise returns false.
*/

bool QSqlQuery::isSelect() const
{
   return d->sqlResult->isSelect();
}

/*!
  Returns true if you can only scroll forward through a result set;
  otherwise returns false.

  \sa setForwardOnly(), next()
*/
bool QSqlQuery::isForwardOnly() const
{
   return d->sqlResult->isForwardOnly();
}

/*!
  Sets forward only mode to \a forward. If \a forward is true, only
  next() and seek() with positive values, are allowed for navigating
  the results.

  Forward only mode can be (depending on the driver) more memory
  efficient since results do not need to be cached. It will also
  improve performance on some databases. For this to be true, you must
  call \c setForwardOnly() before the query is prepared or executed.
  Note that the constructor that takes a query and a database may
  execute the query.

  Forward only mode is off by default.

  Setting forward only to false is a suggestion to the database engine,
  which has the final say on whether a result set is forward only or
  scrollable. isForwardOnly() will always return the correct status of
  the result set.

  \note Calling setForwardOnly after execution of the query will result
  in unexpected results at best, and crashes at worst.

  \sa isForwardOnly(), next(), seek(), QSqlResult::setForwardOnly()
*/
void QSqlQuery::setForwardOnly(bool forward)
{
   d->sqlResult->setForwardOnly(forward);
}

/*!
  Returns a QSqlRecord containing the field information for the
  current query. If the query points to a valid row (isValid() returns
  true), the record is populated with the row's values.  An empty
  record is returned when there is no active query (isActive() returns
  false).

  To retrieve values from a query, value() should be used since
  its index-based lookup is faster.

  In the following example, a \c{SELECT * FROM} query is executed.
  Since the order of the columns is not defined, QSqlRecord::indexOf()
  is used to obtain the index of a column.

  \snippet doc/src/snippets/code/src_sql_kernel_qsqlquery.cpp 1

  \sa value()
*/
QSqlRecord QSqlQuery::record() const
{
   QSqlRecord rec = d->sqlResult->record();

   if (isValid()) {
      for (int i = 0; i < rec.count(); ++i) {
         rec.setValue(i, value(i));
      }
   }
   return rec;
}

/*!
  Clears the result set and releases any resources held by the
  query. Sets the query state to inactive. You should rarely if ever
  need to call this function.
*/
void QSqlQuery::clear()
{
   *this = QSqlQuery(driver()->createResult());
}

/*!
  Prepares the SQL query \a query for execution. Returns true if the
  query is prepared successfully; otherwise returns false.

  The query may contain placeholders for binding values. Both Oracle
  style colon-name (e.g., \c{:surname}), and ODBC style (\c{?})
  placeholders are supported; but they cannot be mixed in the same
  query. See the \l{QSqlQuery examples}{Detailed Description} for
  examples.

  Portability note: Some databases choose to delay preparing a query
  until it is executed the first time. In this case, preparing a
  syntactically wrong query succeeds, but every consecutive exec()
  will fail.

  For SQLite, the query string can contain only one statement at a time.
  If more than one statements are give, the function returns false.

  Example:

  \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 9

  \sa exec(), bindValue(), addBindValue()
*/
bool QSqlQuery::prepare(const QString &query)
{
   if (d->ref.load() != 1) {
      bool fo = isForwardOnly();
      *this = QSqlQuery(driver()->createResult());
      setForwardOnly(fo);
      d->sqlResult->setNumericalPrecisionPolicy(d->sqlResult->numericalPrecisionPolicy());
   } else {
      d->sqlResult->setActive(false);
      d->sqlResult->setLastError(QSqlError());
      d->sqlResult->setAt(QSql::BeforeFirstRow);
      d->sqlResult->setNumericalPrecisionPolicy(d->sqlResult->numericalPrecisionPolicy());
   }

   if (! driver()) {
      qWarning("QSqlQuery::prepare: no driver");
      return false;
   }

   if (! driver()->isOpen() || driver()->isOpenError()) {
      qWarning("QSqlQuery::prepare: database not open");
      return false;
   }

   if (query.isEmpty()) {
      qWarning("QSqlQuery::prepare: empty query");
      return false;
   }

#if defined(CS_SHOW_DEBUG_SQL)
   qDebug("\n QSqlQuery::prepare: %s", csPrintable(query));
#endif

   return d->sqlResult->savePrepare(query);
}

bool QSqlQuery::exec()
{
   d->sqlResult->resetBindCount();

   if (d->sqlResult->lastError().isValid()) {
      d->sqlResult->setLastError(QSqlError());
   }

   return d->sqlResult->exec();
}

bool QSqlQuery::execBatch(BatchExecutionMode mode)
{
   d->sqlResult->resetBindCount();
   return d->sqlResult->execBatch(mode == ValuesAsColumns);
}

void QSqlQuery::bindValue(const QString &placeholder, const QVariant &val,
   QSql::ParamType paramType
)
{
   d->sqlResult->bindValue(placeholder, val, paramType);
}

void QSqlQuery::bindValue(int pos, const QVariant &val, QSql::ParamType paramType)
{
   d->sqlResult->bindValue(pos, val, paramType);
}

void QSqlQuery::addBindValue(const QVariant &val, QSql::ParamType paramType)
{
   d->sqlResult->addBindValue(val, paramType);
}

QVariant QSqlQuery::boundValue(const QString &placeholder) const
{
   return d->sqlResult->boundValue(placeholder);
}

QVariant QSqlQuery::boundValue(int pos) const
{
   return d->sqlResult->boundValue(pos);
}

QMap<QString, QVariant> QSqlQuery::boundValues() const
{
   QMap<QString, QVariant> map;

   const QVector<QVariant> values(d->sqlResult->boundValues());
   for (int i = 0; i < values.count(); ++i) {
      map[d->sqlResult->boundValueName(i)] = values.at(i);
   }
   return map;
}

QString QSqlQuery::executedQuery() const
{
   return d->sqlResult->executedQuery();
}

QVariant QSqlQuery::lastInsertId() const
{
   return d->sqlResult->lastInsertId();
}

void QSqlQuery::setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy precisionPolicy)
{
   d->sqlResult->setNumericalPrecisionPolicy(precisionPolicy);
}

QSql::NumericalPrecisionPolicy QSqlQuery::numericalPrecisionPolicy() const
{
   return d->sqlResult->numericalPrecisionPolicy();
}

void QSqlQuery::finish()
{
   if (isActive()) {
      d->sqlResult->setLastError(QSqlError());
      d->sqlResult->setAt(QSql::BeforeFirstRow);
      d->sqlResult->detachFromResultSet();
      d->sqlResult->setActive(false);
   }
}

bool QSqlQuery::nextResult()
{
   if (isActive()) {
      return d->sqlResult->nextResult();
   }
   return false;
}
