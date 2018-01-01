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

#ifndef QSQLRELATIONALTABLEMODEL_H
#define QSQLRELATIONALTABLEMODEL_H

#include <QtSql/qsqltablemodel.h>

QT_BEGIN_NAMESPACE

class QSqlRelationalTableModelPrivate;

class Q_SQL_EXPORT QSqlRelation
{

 public:
   QSqlRelation() {}
   QSqlRelation(const QString &aTableName, const QString &indexCol, const QString &displayCol)
      : tName(aTableName), iColumn(indexCol), dColumn(displayCol) {}

   inline QString tableName() const {
      return tName;
   }

   inline QString indexColumn() const {
      return iColumn;
   }

   inline QString displayColumn() const {
      return dColumn;
   }

   inline bool isValid() const {
      return !(tName.isEmpty() || iColumn.isEmpty() || dColumn.isEmpty());
   }

 private:
   QString tName, iColumn, dColumn;

};

class Q_SQL_EXPORT QSqlRelationalTableModel: public QSqlTableModel
{
   SQL_CS_OBJECT(QSqlRelationalTableModel)

 public:
   enum JoinMode {
      InnerJoin,
      LeftJoin
   };

   explicit QSqlRelationalTableModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());
   virtual ~QSqlRelationalTableModel();

   QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const override;
   bool setData(const QModelIndex &item, const QVariant &value, int role = Qt::EditRole) override;
   bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

   void clear() override;
   bool select() override;

   void setTable(const QString &tableName) override;
   virtual void setRelation(int column, const QSqlRelation &relation);
   QSqlRelation relation(int column) const;
   virtual QSqlTableModel *relationModel(int column) const;
   void setJoinMode( QSqlRelationalTableModel::JoinMode joinMode );

   SQL_CS_SLOT_1(Public, void revertRow(int row) override)
   SQL_CS_SLOT_2(revertRow)

 protected:
   QString selectStatement() const override;
   bool updateRowInTable(int row, const QSqlRecord &values) override;
   bool insertRowIntoTable(const QSqlRecord &values) override;
   QString orderByClause() const override;

 private:
   Q_DECLARE_PRIVATE(QSqlRelationalTableModel)
};

QT_END_NAMESPACE

#endif // QSQLRELATIONALTABLEMODEL_H
