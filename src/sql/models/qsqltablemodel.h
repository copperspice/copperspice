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

#ifndef QSQLTABLEMODEL_H
#define QSQLTABLEMODEL_H

#include <qsqldatabase.h>
#include <qsqlquerymodel.h>
#include <QSqlRecord>

class QSqlTableModelPrivate;
class QSqlRecord;
class QSqlField;
class QSqlIndex;

class Q_SQL_EXPORT QSqlTableModel: public QSqlQueryModel
{
   SQL_CS_OBJECT(QSqlTableModel)
   Q_DECLARE_PRIVATE(QSqlTableModel)

 public:
   enum EditStrategy {OnFieldChange, OnRowChange, OnManualSubmit};

   explicit QSqlTableModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());
   virtual ~QSqlTableModel();

   virtual void setTable(const QString &tableName);
   QString tableName() const;

   Qt::ItemFlags flags(const QModelIndex &index) const override;

   QSqlRecord record() const;
   QSqlRecord record(int row) const;
   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

   QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

   bool isDirty() const;
   bool isDirty(const QModelIndex &index) const;
   void clear() override;

   virtual void setEditStrategy(EditStrategy strategy);
   EditStrategy editStrategy() const;

   QSqlIndex primaryKey() const;
   QSqlDatabase database() const;
   int fieldIndex(const QString &fieldName) const;

   void sort(int column, Qt::SortOrder order) override;
   virtual void setSort(int column, Qt::SortOrder order);

   QString filter() const;
   virtual void setFilter(const QString &filter);

   int rowCount(const QModelIndex &parent = QModelIndex()) const override;

   bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
   bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
   bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

   bool insertRecord(int row, const QSqlRecord &record);
   bool setRecord(int row, const QSqlRecord &record);

   virtual void revertRow(int row);

   SQL_CS_SLOT_1(Public, virtual bool select())
   SQL_CS_SLOT_2(select)

   SQL_CS_SLOT_1(Public, virtual bool selectRow(int row))
   SQL_CS_SLOT_2(selectRow)
   SQL_CS_SLOT_1(Public, bool submit() override)
   SQL_CS_SLOT_2(submit)

   SQL_CS_SLOT_1(Public, void revert() override)
   SQL_CS_SLOT_2(revert)

   SQL_CS_SLOT_1(Public, bool submitAll())
   SQL_CS_SLOT_2(submitAll)

   SQL_CS_SLOT_1(Public, void revertAll())
   SQL_CS_SLOT_2(revertAll)

   SQL_CS_SIGNAL_1(Public, void primeInsert(int row, QSqlRecord &record))
   SQL_CS_SIGNAL_2(primeInsert, row, record)

   SQL_CS_SIGNAL_1(Public, void beforeInsert(QSqlRecord &record))
   SQL_CS_SIGNAL_2(beforeInsert, record)

   SQL_CS_SIGNAL_1(Public, void beforeUpdate(int row, QSqlRecord &record))
   SQL_CS_SIGNAL_2(beforeUpdate, row, record)

   SQL_CS_SIGNAL_1(Public, void beforeDelete(int row))
   SQL_CS_SIGNAL_2(beforeDelete, row)

 protected:
   QSqlTableModel(QSqlTableModelPrivate &dd, QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());

   virtual bool updateRowInTable(int row, const QSqlRecord &values);
   virtual bool insertRowIntoTable(const QSqlRecord &values);
   virtual bool deleteRowFromTable(int row);
   virtual QString orderByClause() const;
   virtual QString selectStatement() const;

   void setPrimaryKey(const QSqlIndex &key);
   void setQuery(const QSqlQuery &query);
   QModelIndex indexInQuery(const QModelIndex &item) const override;
   QSqlRecord primaryValues(int row) const;
};


#endif // QSQLTABLEMODEL_H
