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

#ifndef QSQLRESULT_P_H
#define QSQLRESULT_P_H

#include <qpointer.h>
#include <qsqldriver.h>
#include <qsqlerror.h>
#include <qsqlresult.h>
#include <qhash.h>

struct QHolder {
   QHolder(const QString &hldr = QString(), int index = -1): holderName(hldr), holderPos(index) { }
   bool operator==(const QHolder &h) const {
      return h.holderPos == holderPos && h.holderName == holderName;
   }
   bool operator!=(const QHolder &h) const {
      return h.holderPos != holderPos || h.holderName != holderName;
   }
   QString holderName;
   int holderPos;
};

class Q_SQL_EXPORT QSqlResultPrivate
{

 public:
   QSqlResultPrivate()
      : q_ptr(nullptr),
        idx(QSql::BeforeFirstRow),
        active(false),
        isSel(false),
        forwardOnly(false),
        precisionPolicy(QSql::LowPrecisionDouble),
        bindCount(0),
        binds(QSqlResult::PositionalBinding)
   { }

   virtual ~QSqlResultPrivate() { }

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

   virtual QString fieldSerial(int) const;
   QString positionalToNamedBinding(const QString &query) const;
   QString namedToPositionalBinding(const QString &query);
   QString holderAt(int index) const;

   QSqlResult *q_ptr;
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
   typedef QHash<QString, QList<int>> IndexMap;
   IndexMap indexes;

   typedef QVector<QHolder> QHolderVector;
   QHolderVector holders;
};


#endif
