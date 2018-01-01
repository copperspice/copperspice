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

#ifndef QSQLRESULT_H
#define QSQLRESULT_H

#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>
#include <QtSql/qsql.h>

QT_BEGIN_NAMESPACE

class QString;
class QSqlRecord;
class QVariant;
class QSqlDriver;
class QSqlError;
class QSqlResultPrivate;

template <typename T> class QVector;

class Q_SQL_EXPORT QSqlResult
{
   friend class QSqlQuery;
   friend class QSqlTableModelPrivate;
   friend class QSqlResultPrivate;

 public:
   virtual ~QSqlResult();
   virtual QVariant handle() const;

 protected:
   enum BindingSyntax {
      PositionalBinding,
      NamedBinding
   };

   explicit QSqlResult(const QSqlDriver *db);
   int at() const;
   QString lastQuery() const;
   QSqlError lastError() const;
   bool isValid() const;
   bool isActive() const;
   bool isSelect() const;
   bool isForwardOnly() const;
   const QSqlDriver *driver() const;
   virtual void setAt(int at);
   virtual void setActive(bool a);
   virtual void setLastError(const QSqlError &e);
   virtual void setQuery(const QString &query);
   virtual void setSelect(bool s);
   virtual void setForwardOnly(bool forward);

   // prepared query support
   virtual bool exec();
   virtual bool prepare(const QString &query);
   virtual bool savePrepare(const QString &sqlquery);
   virtual void bindValue(int pos, const QVariant &val, QSql::ParamType type);
   virtual void bindValue(const QString &placeholder, const QVariant &val,
                          QSql::ParamType type);
   void addBindValue(const QVariant &val, QSql::ParamType type);
   QVariant boundValue(const QString &placeholder) const;
   QVariant boundValue(int pos) const;
   QSql::ParamType bindValueType(const QString &placeholder) const;
   QSql::ParamType bindValueType(int pos) const;
   int boundValueCount() const;
   QVector<QVariant> &boundValues() const;
   QString executedQuery() const;
   QString boundValueName(int pos) const;
   void clear();
   bool hasOutValues() const;

   BindingSyntax bindingSyntax() const;

   virtual QVariant data(int i) = 0;
   virtual bool isNull(int i) = 0;
   virtual bool reset(const QString &sqlquery) = 0;
   virtual bool fetch(int i) = 0;
   virtual bool fetchNext();
   virtual bool fetchPrevious();
   virtual bool fetchFirst() = 0;
   virtual bool fetchLast() = 0;
   virtual int size() = 0;
   virtual int numRowsAffected() = 0;
   virtual QSqlRecord record() const;
   virtual QVariant lastInsertId() const;

   enum VirtualHookOperation { BatchOperation, DetachFromResultSet, SetNumericalPrecision, NextResult };
   virtual void virtual_hook(int id, void *data);
   bool execBatch(bool arrayBind = false);
   void detachFromResultSet();
   void setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy policy);
   QSql::NumericalPrecisionPolicy numericalPrecisionPolicy() const;
   bool nextResult();

 private:
   QSqlResultPrivate *d;
   void resetBindCount(); // HACK

   Q_DISABLE_COPY(QSqlResult)
};

QT_END_NAMESPACE

#endif // QSQLRESULT_H
