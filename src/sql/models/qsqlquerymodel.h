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
   SQL_CS_OBJECT(QSqlQueryModel)
   Q_DECLARE_PRIVATE(QSqlQueryModel)

 public:
   explicit QSqlQueryModel(QObject *parent = nullptr);
   virtual ~QSqlQueryModel();

   int rowCount(const QModelIndex &parent = QModelIndex()) const override;
   int columnCount(const QModelIndex &parent = QModelIndex()) const override;
   QSqlRecord record(int row) const;
   QSqlRecord record() const;

   QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const override;
   QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
   bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

   bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
   bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

   void setQuery(const QSqlQuery &query);
   void setQuery(const QString &query, const QSqlDatabase &db = QSqlDatabase());
   QSqlQuery query() const;

   virtual void clear();

   QSqlError lastError() const;

   void fetchMore(const QModelIndex &parent = QModelIndex()) override;
   bool canFetchMore(const QModelIndex &parent = QModelIndex()) const override;

 protected:
   virtual void queryChange();

   QModelIndex indexInQuery(const QModelIndex &item) const;
   void setLastError(const QSqlError &error);
   QSqlQueryModel(QSqlQueryModelPrivate &dd, QObject *parent = nullptr);
};

QT_END_NAMESPACE

#endif // QSQLQUERYMODEL_H
