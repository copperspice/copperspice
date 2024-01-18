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

#include <qsqlresult.h>
#include <qvariant.h>
#include <qhash.h>
#include <qregularexpression.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlrecord.h>

#include <qvector.h>
#include <qsqldriver.h>
#include <qpointer.h>

#include "qsqlresult_p.h"
#include "qsqldriver_p.h"
#include <qdebug.h>

QString QSqlResultPrivate::holderAt(int index) const
{
   return holders.size() > index ? holders.at(index).holderName : fieldSerial(index);
}

// return a unique id for bound names
QString QSqlResultPrivate::fieldSerial(int i) const
{
   QString retval;

   do {
      retval.prepend('a' + (i % 26));
      i = i / 26;
   } while (i != 0);

   retval.prepend(":");

   return retval;
}

static bool qIsAlnum(QChar ch)
{
   uint u = uint(ch.unicode());
   // matches [a-zA-Z0-9_]
   return u - 'a' < 26 || u - 'A' < 26 || u - '0' < 10 || u == '_';
}

QString QSqlResultPrivate::positionalToNamedBinding(const QString &query) const
{
   int n = query.size();

   QString result;
   QChar closingQuote;
   int count = 0;
   bool ignoreBraces = (sqldriver->d_func()->dbmsType == QSqlDriver::PostgreSQL);

   for (int i = 0; i < n; ++i) {
      QChar ch = query.at(i);
      if (!closingQuote.isNull()) {
         if (ch == closingQuote) {
            if (closingQuote == QLatin1Char(']')
               && i + 1 < n && query.at(i + 1) == closingQuote) {
               // consume the extra character. don't close.
               ++i;
               result += ch;
            } else {
               closingQuote = QChar();
            }
         }
         result += ch;
      } else {
         if (ch == QLatin1Char('?')) {
            result += fieldSerial(count++);
         } else {
            if (ch == QLatin1Char('\'') || ch == QLatin1Char('"') || ch == QLatin1Char('`')) {
               closingQuote = ch;
            } else if (!ignoreBraces && ch == QLatin1Char('[')) {
               closingQuote = QLatin1Char(']');
            }
            result += ch;
         }
      }
   }
   result.squeeze();
   return result;
}

QString QSqlResultPrivate::namedToPositionalBinding(const QString &query)
{
   int n = query.size();

   QString result;

   QChar closingQuote;
   int count = 0;
   int i = 0;
   bool ignoreBraces = (sqldriver->d_func()->dbmsType == QSqlDriver::PostgreSQL);

   while (i < n) {
      QChar ch = query.at(i);
      if (!closingQuote.isNull()) {
         if (ch == closingQuote) {
            if (closingQuote == QLatin1Char(']')
               && i + 1 < n && query.at(i + 1) == closingQuote) {
               // consume the extra character. don't close.
               ++i;
               result += ch;
            } else {
               closingQuote = QChar();
            }
         }
         result += ch;
         ++i;
      } else {
         if (ch == QLatin1Char(':')
            && (i == 0 || query.at(i - 1) != QLatin1Char(':'))
            && (i + 1 < n && qIsAlnum(query.at(i + 1)))) {
            int pos = i + 2;
            while (pos < n && qIsAlnum(query.at(pos))) {
               ++pos;
            }
            QString holder(query.mid(i, pos - i));
            indexes[holder].append(count++);
            holders.append(QHolder(holder, i));
            result += QLatin1Char('?');
            i = pos;
         } else {
            if (ch == QLatin1Char('\'') || ch == QLatin1Char('"') || ch == QLatin1Char('`')) {
               closingQuote = ch;
            } else if (!ignoreBraces && ch == QLatin1Char('[')) {
               closingQuote = QLatin1Char(']');
            }
            result += ch;
            ++i;
         }
      }
   }
   result.squeeze();
   values.resize(holders.size());
   return result;
}


QSqlResult::QSqlResult(const QSqlDriver *db)
{
   d_ptr = new QSqlResultPrivate;
   Q_D(QSqlResult);
   d->q_ptr = this;
   d->sqldriver = const_cast<QSqlDriver *>(db);
   if (d->sqldriver) {
      setNumericalPrecisionPolicy(d->sqldriver->numericalPrecisionPolicy());
   }
}
QSqlResult::QSqlResult(QSqlResultPrivate &dd, const QSqlDriver *db)
{
   d_ptr = &dd;
   Q_D(QSqlResult);
   d->q_ptr = this;
   d->sqldriver = const_cast<QSqlDriver *>(db);
   if (d->sqldriver) {
      setNumericalPrecisionPolicy(d->sqldriver->numericalPrecisionPolicy());
   }
}



QSqlResult::~QSqlResult()
{
   Q_D(QSqlResult);
   delete d;
}

void QSqlResult::setQuery(const QString &query)
{
   Q_D(QSqlResult);
   d->sql = query;
}

QString QSqlResult::lastQuery() const
{
   Q_D(const QSqlResult);
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
   Q_D(const QSqlResult);
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
   Q_D(const QSqlResult);
   return d->idx != QSql::BeforeFirstRow && d->idx != QSql::AfterLastRow;
}


bool QSqlResult::isActive() const
{
   Q_D(const QSqlResult);
   return d->active;
}

/*!
    This function is provided for derived classes to set the
    internal (zero-based) row position to \a index.

    \sa at()
*/

void QSqlResult::setAt(int index)
{
   Q_D(QSqlResult);
   d->idx = index;
}



void QSqlResult::setSelect(bool select)
{
   Q_D(QSqlResult);
   d->isSel = select;
}

/*!
    Returns true if the current result is from a \c SELECT statement;
    otherwise returns false.

    \sa setSelect()
*/

bool QSqlResult::isSelect() const
{
   Q_D(const QSqlResult);
   return d->isSel;
}

/*!
    Returns the driver associated with the result. This is the object
    that was passed to the constructor.
*/

const QSqlDriver *QSqlResult::driver() const
{
   Q_D(const QSqlResult);
   return d->sqldriver;
}


/*!
    This function is provided for derived classes to set the internal
    active state to \a active.

    \sa isActive()
*/

void QSqlResult::setActive(bool active)
{
   Q_D(QSqlResult);

   if (active && d->executedQuery.isEmpty()) {
      d->executedQuery = d->sql;
   }

   d->active = active;
}


void QSqlResult::setLastError(const QSqlError &error)
{
   Q_D(QSqlResult);
   d->error = error;
}

QSqlError QSqlResult::lastError() const
{
   Q_D(const QSqlResult);
   return d->error;
}


bool QSqlResult::fetchNext()
{
   return fetch(at() + 1);
}


bool QSqlResult::fetchPrevious()
{
   return fetch(at() - 1);
}

bool QSqlResult::isForwardOnly() const
{
   Q_D(const QSqlResult);
   return d->forwardOnly;
}


void QSqlResult::setForwardOnly(bool forward)
{
   Q_D(QSqlResult);
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
   Q_D(QSqlResult);

   if (!driver()) {
      return false;
   }

   d->clear();
   d->sql = query;

   if (!driver()->hasFeature(QSqlDriver::PreparedQueries)) {
      return prepare(query);
   }

   // parse the query to memorize parameter location
   d->executedQuery = d->namedToPositionalBinding(query);

   if (driver()->hasFeature(QSqlDriver::NamedPlaceholders)) {
      d->executedQuery = d->positionalToNamedBinding(query);
   }

   return prepare(d->executedQuery);
}


bool QSqlResult::prepare(const QString &query)
{

   Q_D(QSqlResult);
   d->sql = query;
   if (d->holders.isEmpty()) {
      // parse the query to memorize parameter location
      d->namedToPositionalBinding(query);
   }
   return true; // fake prepares should always succeed
}


bool QSqlResult::exec()
{
   Q_D(QSqlResult);
   bool ret;
   // fake preparation - just replace the placeholders..
   QString query = lastQuery();
   if (d->binds == NamedBinding) {
      int i;
      QVariant val;
      QString holder;
      for (i = d->holders.count() - 1; i >= 0; --i) {
         holder = d->holders.at(i).holderName;
         val = d->values.value(d->indexes.value(holder).value(0, -1));
         QSqlField f(QLatin1String(""), val.type());
         f.setValue(val);
         query = query.replace(d->holders.at(i).holderPos,
               holder.length(), driver()->formatValue(f));
      }
   } else {
      QString val;

      int i   = 0;
      int idx = 0;

      for (idx = 0; idx < d->values.count(); ++idx) {
         i = query.indexOf(QLatin1Char('?'), i);
         if (i == -1) {
            continue;
         }
         QVariant var = d->values.value(idx);
         QSqlField f(QLatin1String(""), var.type());

         if (! var.isValid()) {
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


void QSqlResult::bindValue(int index, const QVariant &val, QSql::ParamType paramType)
{
   Q_D(QSqlResult);

   d->binds = PositionalBinding;
   QList<int> &indexes = d->indexes[d->fieldSerial(index)];

   if (!indexes.contains(index)) {
      indexes.append(index);
   }

   if (d->values.count() <= index) {
      d->values.resize(index + 1);
   }
   d->values[index] = val;
   if (paramType != QSql::In || !d->types.isEmpty()) {
      d->types[index] = paramType;
   }
}


void QSqlResult::bindValue(const QString &placeholder, const QVariant &val, QSql::ParamType paramType)
{
   Q_D(QSqlResult);
   d->binds = NamedBinding;

   // if the index has already been set when doing emulated named
   // bindings - don't reset it

   QList<int> indexes = d->indexes.value(placeholder);
   for (int idx : indexes) {
      if (d->values.count() <= idx) {
         d->values.resize(idx + 1);
      }

      d->values[idx] = val;



      if (paramType != QSql::In || !d->types.isEmpty()) {
         d->types[idx] = paramType;
      }
   }
}


void QSqlResult::addBindValue(const QVariant &val, QSql::ParamType paramType)
{
   Q_D(QSqlResult);
   d->binds = PositionalBinding;
   bindValue(d->bindCount, val, paramType);
   ++d->bindCount;
}


QVariant QSqlResult::boundValue(int index) const
{
   Q_D(const QSqlResult);
   return d->values.value(index);
}


QVariant QSqlResult::boundValue(const QString &placeholder) const
{
   Q_D(const QSqlResult);
   QList<int> indexes = d->indexes.value(placeholder);
   return d->values.value(indexes.value(0, -1));
}


QSql::ParamType QSqlResult::bindValueType(int index) const
{
   Q_D(const QSqlResult);
   return d->types.value(index, QSql::In);
}


QSql::ParamType QSqlResult::bindValueType(const QString &placeholder) const
{
   Q_D(const QSqlResult);
   return d->types.value(d->indexes.value(placeholder).value(0, -1), QSql::In);
}


int QSqlResult::boundValueCount() const
{
   Q_D(const QSqlResult);
   return d->values.count();
}

QVector<QVariant> &QSqlResult::boundValues() const
{
   Q_D(const QSqlResult);
   return const_cast<QSqlResultPrivate *>(d)->values;
}


QSqlResult::BindingSyntax QSqlResult::bindingSyntax() const
{
   Q_D(const QSqlResult);
   return d->binds;
}

/*!
    Clears the entire result set and releases any associated
    resources.
*/
void QSqlResult::clear()
{
   Q_D(QSqlResult);
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
   Q_D(const QSqlResult);
   return d->executedQuery;
}

void QSqlResult::resetBindCount()
{
   Q_D(QSqlResult);
   d->resetBindCount();
}


QString QSqlResult::boundValueName(int index) const
{
   Q_D(const QSqlResult);
   return d->holderAt(index);
}


bool QSqlResult::hasOutValues() const
{
   Q_D(const QSqlResult);

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

QSqlRecord QSqlResult::record() const
{
   return QSqlRecord();
}

QVariant QSqlResult::lastInsertId() const
{
   return QVariant();
}

/*! \internal
*/
void QSqlResult::virtual_hook(int, void *)
{
}

bool QSqlResult::execBatch(bool arrayBind)
{
   (void) arrayBind;

   Q_D(QSqlResult);

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

void QSqlResult::detachFromResultSet()
{
}


void QSqlResult::setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy policy)
{
   Q_D(QSqlResult);
   d->precisionPolicy = policy;

}


QSql::NumericalPrecisionPolicy QSqlResult::numericalPrecisionPolicy() const
{
   Q_D(const QSqlResult);
   return d->precisionPolicy;
}


bool QSqlResult::nextResult()
{
   return false;
}


QVariant QSqlResult::handle() const
{
   return QVariant();
}

