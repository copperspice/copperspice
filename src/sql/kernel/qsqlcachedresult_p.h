/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSQLCACHEDRESULT_P_H
#define QSQLCACHEDRESULT_P_H

#include <QtSql/qsqlresult.h>

QT_BEGIN_NAMESPACE

class QVariant;
template <typename T> class QVector;

class QSqlCachedResultPrivate;

class Q_SQL_EXPORT QSqlCachedResult: public QSqlResult
{
 public:
   virtual ~QSqlCachedResult();

   typedef QVector<QVariant> ValueCache;

 protected:
   QSqlCachedResult(const QSqlDriver *db);

   void init(int colCount);
   void cleanup();
   void clearValues();

   virtual bool gotoNext(ValueCache &values, int index) = 0;

   QVariant data(int i);
   bool isNull(int i);
   bool fetch(int i);
   bool fetchNext();
   bool fetchPrevious();
   bool fetchFirst();
   bool fetchLast();

   int colCount() const;
   ValueCache &cache();

   void virtual_hook(int id, void *data);
 private:
   bool cacheNext();
   QSqlCachedResultPrivate *d;
};

QT_END_NAMESPACE

#endif // QSQLCACHEDRESULT_P_H
