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

#ifndef QSQLRESULT_H
#define QSQLRESULT_H

#include <qvariant.h>
#include <qvector.h>
#include <qsql.h>
#include <qstring.h>
class QSqlRecord;
class QVariant;
class QSqlDriver;
class QSqlError;
class QSqlResultPrivate;


class Q_SQL_EXPORT QSqlResult
{
   Q_DECLARE_PRIVATE(QSqlResult)

   friend class QSqlQuery;
   friend class QSqlTableModelPrivate;


 public:
   virtual ~QSqlResult();
   virtual QVariant handle() const;

 protected:
   enum BindingSyntax {
      PositionalBinding,
      NamedBinding
   };

   explicit QSqlResult(const QSqlDriver *db);
   QSqlResult(QSqlResultPrivate &dd, const QSqlDriver *db);

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

   enum VirtualHookOperation {  };

   virtual void virtual_hook(int id, void *data);
   virtual bool execBatch(bool arrayBind = false);
   virtual void detachFromResultSet();
   virtual void setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy policy);

   QSql::NumericalPrecisionPolicy numericalPrecisionPolicy() const;

   virtual bool nextResult();
   void resetBindCount(); // HACK

   QSqlResultPrivate *d_ptr;
 private:
   Q_DISABLE_COPY(QSqlResult)
};

#endif // QSQLRESULT_H
