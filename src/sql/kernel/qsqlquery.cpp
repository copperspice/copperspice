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
#include <qmap.h>
#include <qsqldatabase.h>
#include <qsqldriver.h>
#include <qsqlrecord.h>
#include <qsqlresult.h>
#include <qvector.h>

#include <qsqlnulldriver_p.h>

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

QSqlQuery::QSqlQuery(const QString &query, QSqlDatabase db)
{
   d = QSqlQueryPrivate::shared_null();
   qInit(this, query, db);
}


QSqlQuery::QSqlQuery(QSqlDatabase db)
{
   d = QSqlQueryPrivate::shared_null();
   qInit(this, QString(), db);
}

QSqlQuery &QSqlQuery::operator=(const QSqlQuery &other)
{
   qAtomicAssign(d, other.d);
   return *this;
}

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

const QSqlDriver *QSqlQuery::driver() const
{
   return d->sqlResult->driver();
}

const QSqlResult *QSqlQuery::result() const
{
   return d->sqlResult;
}

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

bool QSqlQuery::last()
{
   if (!isSelect() || !isActive()) {
      return false;
   }
   bool b = false;
   b = d->sqlResult->fetchLast();
   return b;
}

int QSqlQuery::size() const
{
   if (isActive() && d->sqlResult->driver()->hasFeature(QSqlDriver::QuerySize)) {
      return d->sqlResult->size();
   }
   return -1;
}

int QSqlQuery::numRowsAffected() const
{
   if (isActive()) {
      return d->sqlResult->numRowsAffected();
   }
   return -1;
}

QSqlError QSqlQuery::lastError() const
{
   return d->sqlResult->lastError();
}

bool QSqlQuery::isValid() const
{
   return d->sqlResult->isValid();
}

bool QSqlQuery::isActive() const
{
   return d->sqlResult->isActive();
}

bool QSqlQuery::isSelect() const
{
   return d->sqlResult->isSelect();
}

bool QSqlQuery::isForwardOnly() const
{
   return d->sqlResult->isForwardOnly();
}

void QSqlQuery::setForwardOnly(bool forward)
{
   d->sqlResult->setForwardOnly(forward);
}

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

void QSqlQuery::clear()
{
   *this = QSqlQuery(driver()->createResult());
}

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
