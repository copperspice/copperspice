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

#include <qvariant.h>
#include <qhash.h>
#include <qregularexpression.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlrecord.h>
#include <qsqlresult.h>
#include <qvector.h>
#include <qsqldriver.h>
#include <qpointer.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

struct QHolder {
   QHolder(const QString &hldr = QString(), int index = -1): holderName(hldr), holderPos(index) {}
   bool operator==(const QHolder &h) const {
      return h.holderPos == holderPos && h.holderName == holderName;
   }
   bool operator!=(const QHolder &h) const {
      return h.holderPos != holderPos || h.holderName != holderName;
   }
   QString holderName;
   int holderPos;
};

class QSqlResultPrivate
{
 public:
   QSqlResultPrivate(QSqlResult *d)
      : q(d), idx(QSql::BeforeFirstRow), active(false),
        isSel(false), forwardOnly(false), precisionPolicy(QSql::LowPrecisionDouble), bindCount(0),
        binds(QSqlResult::PositionalBinding) {
   }

   void clearValues() {
      values.clear();
      bindCount = 0;
   }

   void resetBindCount() {
      bindCount = 0;
   }

   void clearIndex() {
      indexes.clear();
      holders.clear();
      types.clear();
   }

   void clear() {
      clearValues();
      clearIndex();;
   }

   QString positionalToNamedBinding();
   QString namedToPositionalBinding();
   QString holderAt(int index) const;

 public:
   QSqlResult *q;
   QPointer<QSqlDriver> sqldriver;
   int idx;
   QString sql;
   bool active;
   bool isSel;
   QSqlError error;
   bool forwardOnly;
   QSql::NumericalPrecisionPolicy precisionPolicy;

   int bindCount;
   QSqlResult::BindingSyntax binds;

   QString executedQuery;
   QHash<int, QSql::ParamType> types;
   QVector<QVariant> values;
   typedef QHash<QString, int> IndexMap;
   IndexMap indexes;

   typedef QVector<QHolder> QHolderVector;
   QHolderVector holders;
};

QString QSqlResultPrivate::holderAt(int index) const
{
   return indexes.key(index);
}

// return a unique id for bound names
static QString qFieldSerial(int i)
{
   ushort arr[] = { ':', 'f', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   ushort *ptr = &arr[1];

   while (i > 0) {
      *(++ptr) = 'a' + i % 16;
      i >>= 4;
   }

   return QString(reinterpret_cast<const QChar *>(arr), int(ptr - arr) + 1);
}

static bool qIsAlnum(QChar ch)
{
   uint u = uint(ch.unicode());
   // matches [a-zA-Z0-9_]
   return u - 'a' < 26 || u - 'A' < 26 || u - '0' < 10 || u == '_';
}

QString QSqlResultPrivate::positionalToNamedBinding()
{
   int n = sql.size();

   QString result;

   bool inQuote = false;
   int count    = 0;

   for (int i = 0; i < n; ++i) {
      QChar ch = sql.at(i);
      if (ch == QLatin1Char('?') && !inQuote) {
         result += qFieldSerial(count++);
      } else {
         if (ch == QLatin1Char('\'')) {
            inQuote = !inQuote;
         }
         result += ch;
      }
   }
   result.squeeze();
   return result;
}

QString QSqlResultPrivate::namedToPositionalBinding()
{
   int n = sql.size();

   QString result;

   bool inQuote = false;
   int count    = 0;
   int i        = 0;

   while (i < n) {
      QChar ch = sql.at(i);
      if (ch == QLatin1Char(':') && !inQuote
            && (i == 0 || sql.at(i - 1) != QLatin1Char(':'))
            && (i + 1 < n && qIsAlnum(sql.at(i + 1)))) {
         int pos = i + 2;
         while (pos < n && qIsAlnum(sql.at(pos))) {
            ++pos;
         }
         indexes[sql.mid(i, pos - i)] = count++;
         result += QLatin1Char('?');
         i = pos;
      } else {
         if (ch == QLatin1Char('\'')) {
            inQuote = !inQuote;
         }
         result += ch;
         ++i;
      }
   }
   result.squeeze();
   return result;
}

/*!
    \class QSqlResult
    \brief The QSqlResult class provides an abstract interface for
    accessing data from specific SQL databases.

    \ingroup database
    \inmodule QtSql

    Normally, you would use QSqlQuery instead of QSqlResult, since
    QSqlQuery provides a generic wrapper for database-specific
    implementations of QSqlResult.

    If you are implementing your own SQL driver (by subclassing
    QSqlDriver), you will need to provide your own QSqlResult
    subclass that implements all the pure virtual functions and other
    virtual functions that you need.

    \sa QSqlDriver
*/

/*!
    \enum QSqlResult::BindingSyntax

    This enum type specifies the different syntaxes for specifying
    placeholders in prepared queries.

    \value PositionalBinding Use the ODBC-style positional syntax, with "?" as placeholders.
    \value NamedBinding Use the Oracle-style syntax with named placeholders (e.g., ":id")
    \omitvalue BindByPosition
    \omitvalue BindByName

    \sa bindingSyntax()
*/

/*!
    \enum QSqlResult::VirtualHookOperation
    \internal
*/

/*!
    Creates a QSqlResult using database driver \a db. The object is
    initialized to an inactive state.

    \sa isActive(), driver()
*/

QSqlResult::QSqlResult(const QSqlDriver *db)
{
   d = new QSqlResultPrivate(this);
   d->sqldriver = const_cast<QSqlDriver *>(db);
   if (db) {
      setNumericalPrecisionPolicy(db->numericalPrecisionPolicy());
   }
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlResult::~QSqlResult()
{
   delete d;
}

void QSqlResult::setQuery(const QString &query)
{
   d->sql = query;
}

QString QSqlResult::lastQuery() const
{
   return d->sql;
}

/*!
    Returns the current (zero-based) row position of the result. May
    return the special values QSql::BeforeFirstRow or
    QSql::AfterLastRow.

    \sa setAt(), isValid()
*/
int QSqlResult::at() const
{
   return d->idx;
}


/*!
    Returns true if the result is positioned on a valid record (that
    is, the result is not positioned before the first or after the
    last record); otherwise returns false.

    \sa at()
*/

bool QSqlResult::isValid() const
{
   return d->idx != QSql::BeforeFirstRow && d->idx != QSql::AfterLastRow;
}

/*!
    \fn bool QSqlResult::isNull(int index)

    Returns true if the field at position \a index in the current row
    is null; otherwise returns false.
*/

/*!
    Returns true if the result has records to be retrieved; otherwise
    returns false.
*/

bool QSqlResult::isActive() const
{
   return d->active;
}

/*!
    This function is provided for derived classes to set the
    internal (zero-based) row position to \a index.

    \sa at()
*/

void QSqlResult::setAt(int index)
{
   d->idx = index;
}


/*!
    This function is provided for derived classes to indicate whether
    or not the current statement is a SQL \c SELECT statement. The \a
    select parameter should be true if the statement is a \c SELECT
    statement; otherwise it should be false.

    \sa isSelect()
*/

void QSqlResult::setSelect(bool select)
{
   d->isSel = select;
}

/*!
    Returns true if the current result is from a \c SELECT statement;
    otherwise returns false.

    \sa setSelect()
*/

bool QSqlResult::isSelect() const
{
   return d->isSel;
}

/*!
    Returns the driver associated with the result. This is the object
    that was passed to the constructor.
*/

const QSqlDriver *QSqlResult::driver() const
{
   return d->sqldriver;
}


/*!
    This function is provided for derived classes to set the internal
    active state to \a active.

    \sa isActive()
*/

void QSqlResult::setActive(bool active)
{
   if (active && d->executedQuery.isEmpty()) {
      d->executedQuery = d->sql;
   }

   d->active = active;
}

/*!
    This function is provided for derived classes to set the last
    error to \a error.

    \sa lastError()
*/

void QSqlResult::setLastError(const QSqlError &error)
{
   d->error = error;
}


/*!
    Returns the last error associated with the result.
*/

QSqlError QSqlResult::lastError() const
{
   return d->error;
}

/*!
    \fn int QSqlResult::size()

    Returns the size of the \c SELECT result, or -1 if it cannot be
    determined or if the query is not a \c SELECT statement.

    \sa numRowsAffected()
*/

bool QSqlResult::fetchNext()
{
   return fetch(at() + 1);
}

/*!
    Positions the result to the previous record (row) in the result.

    This function is only called if the result is in an active state.
    The default implementation calls fetch() with the previous index.
    Derived classes can reimplement this function and position the
    result to the next record in some other way, and call setAt()
    with an appropriate value. Return true to indicate success, or
    false to signify failure.
*/

bool QSqlResult::fetchPrevious()
{
   return fetch(at() - 1);
}

/*!
    Returns true if you can only scroll forward through the result
    set; otherwise returns false.

    \sa setForwardOnly()
*/
bool QSqlResult::isForwardOnly() const
{
   return d->forwardOnly;
}

/*!
    Sets forward only mode to \a forward. If \a forward is true, only
    fetchNext() is allowed for navigating the results. Forward only
    mode needs much less memory since results do not have to be
    cached. By default, this feature is disabled.

    Setting forward only to false is a suggestion to the database engine,
    which has the final say on whether a result set is forward only or
    scrollable. isForwardOnly() will always return the correct status of
    the result set.

    \note Calling setForwardOnly after execution of the query will result
    in unexpected results at best, and crashes at worst.

    \sa isForwardOnly(), fetchNext(), QSqlQuery::setForwardOnly()
*/
void QSqlResult::setForwardOnly(bool forward)
{
   d->forwardOnly = forward;
}

/*!
    Prepares the given \a query, using the underlying database
    functionality where possible. Returns true if the query is
    prepared successfully; otherwise returns false.

    \sa prepare()
*/
bool QSqlResult::savePrepare(const QString &query)
{
   if (!driver()) {
      return false;
   }
   d->clear();
   d->sql = query;
   if (!driver()->hasFeature(QSqlDriver::PreparedQueries)) {
      return prepare(query);
   }

   if (driver()->hasFeature(QSqlDriver::NamedPlaceholders)) {
      // parse the query to memorize parameter location
      d->namedToPositionalBinding();
      d->executedQuery = d->positionalToNamedBinding();
   } else {
      d->executedQuery = d->namedToPositionalBinding();
   }
   return prepare(d->executedQuery);
}

/*!
    Prepares the given \a query for execution; the query will normally
    use placeholders so that it can be executed repeatedly. Returns
    true if the query is prepared successfully; otherwise returns false.

    \sa exec()
*/
bool QSqlResult::prepare(const QString &query)
{
   int n = query.size();

   bool inQuote = false;
   int i = 0;

   while (i < n) {
      QChar ch = query.at(i);
      if (ch == QLatin1Char(':') && !inQuote
            && (i == 0 || query.at(i - 1) != QLatin1Char(':'))
            && (i + 1 < n && qIsAlnum(query.at(i + 1)))) {
         int pos = i + 2;
         while (pos < n && qIsAlnum(query.at(pos))) {
            ++pos;
         }

         d->holders.append(QHolder(query.mid(i, pos - i), i));
         i = pos;
      } else {
         if (ch == QLatin1Char('\'')) {
            inQuote = !inQuote;
         }
         ++i;
      }
   }
   d->sql = query;
   return true; // fake prepares should always succeed
}

/*!
    Executes the query, returning true if successful; otherwise returns
    false.

    \sa prepare()
*/
bool QSqlResult::exec()
{
   bool ret;
   // fake preparation - just replace the placeholders..
   QString query = lastQuery();
   if (d->binds == NamedBinding) {
      int i;
      QVariant val;
      QString holder;
      for (i = d->holders.count() - 1; i >= 0; --i) {
         holder = d->holders.at(i).holderName;
         val = d->values.value(d->indexes.value(holder));
         QSqlField f(QLatin1String(""), val.type());
         f.setValue(val);
         query = query.replace(d->holders.at(i).holderPos,
                               holder.length(), driver()->formatValue(f));
      }
   } else {
      QString val;
      int i = 0;
      int idx = 0;
      for (idx = 0; idx < d->values.count(); ++idx) {
         i = query.indexOf(QLatin1Char('?'), i);
         if (i == -1) {
            continue;
         }
         QVariant var = d->values.value(idx);
         QSqlField f(QLatin1String(""), var.type());
         if (var.isNull()) {
            f.clear();
         } else {
            f.setValue(var);
         }
         val = driver()->formatValue(f);
         query = query.replace(i, 1, driver()->formatValue(f));
         i += val.length();
      }
   }

   // have to retain the original query with placeholders
   QString orig = lastQuery();
   ret = reset(query);
   d->executedQuery = query;
   setQuery(orig);
   d->resetBindCount();

   return ret;
}

/*!
    Binds the value \a val of parameter type \a paramType to position \a index
    in the current record (row).

    \sa addBindValue()
*/
void QSqlResult::bindValue(int index, const QVariant &val, QSql::ParamType paramType)
{
   d->binds = PositionalBinding;
   d->indexes[qFieldSerial(index)] = index;
   if (d->values.count() <= index) {
      d->values.resize(index + 1);
   }
   d->values[index] = val;
   if (paramType != QSql::In || !d->types.isEmpty()) {
      d->types[index] = paramType;
   }
}

/*!
    \overload

    Binds the value \a val of parameter type \a paramType to the \a
    placeholder name in the current record (row).

   Values cannot be bound to multiple locations in the query, eg:
   \code
   INSERT INTO testtable (id, name, samename) VALUES (:id, :name, :name)
   \endcode
   Binding to name will bind to the first :name, but not the second.

    \note Binding an undefined placeholder will result in undefined behavior.

    \sa QSqlQuery::bindValue()
*/
void QSqlResult::bindValue(const QString &placeholder, const QVariant &val,
                           QSql::ParamType paramType)
{
   d->binds = NamedBinding;

   // if the index has already been set when doing emulated named
   // bindings - don't reset it

   int idx = d->indexes.value(placeholder, -1);
   if (idx >= 0) {
      if (d->values.count() <= idx) {
         d->values.resize(idx + 1);
      }
      d->values[idx] = val;
   } else {
      d->values.append(val);
      idx = d->values.count() - 1;
      d->indexes[placeholder] = idx;
   }

   if (paramType != QSql::In || !d->types.isEmpty()) {
      d->types[idx] = paramType;
   }
}

/*!
    Binds the value \a val of parameter type \a paramType to the next
    available position in the current record (row).

    \sa bindValue()
*/
void QSqlResult::addBindValue(const QVariant &val, QSql::ParamType paramType)
{
   d->binds = PositionalBinding;
   bindValue(d->bindCount, val, paramType);
   ++d->bindCount;
}

/*!
    Returns the value bound at position \a index in the current record
    (row).

    \sa bindValue(), boundValues()
*/
QVariant QSqlResult::boundValue(int index) const
{
   return d->values.value(index);
}

/*!
    \overload

    Returns the value bound by the given \a placeholder name in the
    current record (row).

    \sa bindValueType()
*/
QVariant QSqlResult::boundValue(const QString &placeholder) const
{
   int idx = d->indexes.value(placeholder, -1);
   return d->values.value(idx);
}

/*!
    Returns the parameter type for the value bound at position \a index.

    \sa boundValue()
*/
QSql::ParamType QSqlResult::bindValueType(int index) const
{
   return d->types.value(index, QSql::In);
}

/*!
    \overload

    Returns the parameter type for the value bound with the given \a
    placeholder name.
*/
QSql::ParamType QSqlResult::bindValueType(const QString &placeholder) const
{
   return d->types.value(d->indexes.value(placeholder, -1), QSql::In);
}

/*!
    Returns the number of bound values in the result.

    \sa boundValues()
*/
int QSqlResult::boundValueCount() const
{
   return d->values.count();
}

/*!
    Returns a vector of the result's bound values for the current
    record (row).

    \sa boundValueCount()
*/
QVector<QVariant> &QSqlResult::boundValues() const
{
   return d->values;
}

/*!
    Returns the binding syntax used by prepared queries.
*/
QSqlResult::BindingSyntax QSqlResult::bindingSyntax() const
{
   return d->binds;
}

/*!
    Clears the entire result set and releases any associated
    resources.
*/
void QSqlResult::clear()
{
   d->clear();
}

/*!
    Returns the query that was actually executed. This may differ from
    the query that was passed, for example if bound values were used
    with a prepared query and the underlying database doesn't support
    prepared queries.

    \sa exec(), setQuery()
*/
QString QSqlResult::executedQuery() const
{
   return d->executedQuery;
}

void QSqlResult::resetBindCount()
{
   d->resetBindCount();
}

/*!
    Returns the name of the bound value at position \a index in the
    current record (row).

    \sa boundValue()
*/
QString QSqlResult::boundValueName(int index) const
{
   return d->holderAt(index);
}

/*!
    Returns true if at least one of the query's bound values is a \c
    QSql::Out or a QSql::InOut; otherwise returns false.

    \sa bindValueType()
*/
bool QSqlResult::hasOutValues() const
{
   if (d->types.isEmpty()) {
      return false;
   }
   QHash<int, QSql::ParamType>::const_iterator it;
   for (it = d->types.constBegin(); it != d->types.constEnd(); ++it) {
      if (it.value() != QSql::In) {
         return true;
      }
   }
   return false;
}

/*!
    Returns the current record if the query is active; otherwise
    returns an empty QSqlRecord.

    The default implementation always returns an empty QSqlRecord.

    \sa isActive()
*/
QSqlRecord QSqlResult::record() const
{
   return QSqlRecord();
}

/*!
    Returns the object ID of the most recent inserted row if the
    database supports it.
    An invalid QVariant will be returned if the query did not
    insert any value or if the database does not report the id back.
    If more than one row was touched by the insert, the behavior is
    undefined.

    Note that for Oracle databases the row's ROWID will be returned,
    while for MySQL databases the row's auto-increment field will
    be returned.

    \sa QSqlDriver::hasFeature()
*/
QVariant QSqlResult::lastInsertId() const
{
   return QVariant();
}

/*! \internal
*/
void QSqlResult::virtual_hook(int, void *)
{
}

/*! \internal
    \since 4.2

    Executes a prepared query in batch mode if the driver supports it,
    otherwise emulates a batch execution using bindValue() and exec().
    QSqlDriver::hasFeature() can be used to find out whether a driver
    supports batch execution.

    Batch execution can be faster for large amounts of data since it
    reduces network roundtrips.

    For batch executions, bound values have to be provided as lists
    of variants (QVariantList).

    Each list must contain values of the same type. All lists must
    contain equal amount of values (rows).

    NULL values are passed in as typed QVariants, for example
    \c {QVariant(QVariant::Int)} for an integer NULL value.

    Example:

    \snippet doc/src/snippets/code/src_sql_kernel_qsqlresult.cpp 0

    Here, we insert two rows into a SQL table, with each row containing three values.

    \sa exec(), QSqlDriver::hasFeature()
*/
bool QSqlResult::execBatch(bool arrayBind)
{
   if (driver()->hasFeature(QSqlDriver::BatchOperations)) {
      virtual_hook(BatchOperation, &arrayBind);
      d->resetBindCount();
      return d->error.type() == QSqlError::NoError;
   } else {
      QVector<QVariant> values = d->values;
      if (values.count() == 0) {
         return false;
      }
      for (int i = 0; i < values.at(0).toList().count(); ++i) {
         for (int j = 0; j < values.count(); ++j) {
            bindValue(j, values.at(j).toList().at(i), QSql::In);
         }
         if (!exec()) {
            return false;
         }
      }
      return true;
   }
   return false;
}

/*! \internal
 */
void QSqlResult::detachFromResultSet()
{
   if (driver()->hasFeature(QSqlDriver::FinishQuery)
         || driver()->hasFeature(QSqlDriver::SimpleLocking)) {
      virtual_hook(DetachFromResultSet, 0);
   }
}

/*! \internal
 */
void QSqlResult::setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy policy)
{
   d->precisionPolicy = policy;
   virtual_hook(SetNumericalPrecision, &policy);
}

/*! \internal
 */
QSql::NumericalPrecisionPolicy QSqlResult::numericalPrecisionPolicy() const
{
   return d->precisionPolicy;
}

/*! \internal
*/
bool QSqlResult::nextResult()
{
   if (driver()->hasFeature(QSqlDriver::MultipleResultSets)) {
      bool result = false;
      virtual_hook(NextResult, &result);
      return result;
   }
   return false;
}

/*!
    Returns the low-level database handle for this result set
    wrapped in a QVariant or an invalid QVariant if there is no handle.

    \warning Use this with uttermost care and only if you know what you're doing.

    \warning The handle returned here can become a stale pointer if the result
    is modified (for example, if you clear it).

    \warning The handle can be NULL if the result was not executed yet.

    The handle returned here is database-dependent, you should query the type
    name of the variant before accessing it.

    This example retrieves the handle for a sqlite result:

    \snippet doc/src/snippets/code/src_sql_kernel_qsqlresult.cpp 1

    This snippet returns the handle for PostgreSQL or MySQL:

    \snippet doc/src/snippets/code/src_sql_kernel_qsqlresult.cpp 2

    \sa QSqlDriver::handle()
*/
QVariant QSqlResult::handle() const
{
   return QVariant();
}

QT_END_NAMESPACE
