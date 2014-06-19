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

#ifndef QSQLQUERYMODEL_H
#define QSQLQUERYMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtSql/qsqldatabase.h>

QT_BEGIN_NAMESPACE

class QSqlQueryModelPrivate;
class QSqlError;
class QSqlRecord;
class QSqlQuery;

class Q_SQL_EXPORT QSqlQueryModel: public QAbstractTableModel
{
   CS_OBJECT(QSqlQueryModel)
   Q_DECLARE_PRIVATE(QSqlQueryModel)

 public:
   explicit QSqlQueryModel(QObject *parent = 0);
   virtual ~QSqlQueryModel();

   int rowCount(const QModelIndex &parent = QModelIndex()) const;
   int columnCount(const QModelIndex &parent = QModelIndex()) const;
   QSqlRecord record(int row) const;
   QSqlRecord record() const;

   QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const;
   QVariant headerData(int section, Qt::Orientation orientation,
                       int role = Qt::DisplayRole) const;
   bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                      int role = Qt::EditRole);

   bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
   bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

   void setQuery(const QSqlQuery &query);
   void setQuery(const QString &query, const QSqlDatabase &db = QSqlDatabase());
   QSqlQuery query() const;

   virtual void clear();

   QSqlError lastError() const;

   void fetchMore(const QModelIndex &parent = QModelIndex());
   bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;

 protected:
   virtual void queryChange();

   QModelIndex indexInQuery(const QModelIndex &item) const;
   void setLastError(const QSqlError &error);
   QSqlQueryModel(QSqlQueryModelPrivate &dd, QObject *parent = 0);
};

QT_END_NAMESPACE

#endif // QSQLQUERYMODEL_H
