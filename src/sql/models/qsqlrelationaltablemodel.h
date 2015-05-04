/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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
   CS_OBJECT(QSqlRelationalTableModel)

 public:
   enum JoinMode {
      InnerJoin,
      LeftJoin
   };

   explicit QSqlRelationalTableModel(QObject *parent = 0,
                                     QSqlDatabase db = QSqlDatabase());
   virtual ~QSqlRelationalTableModel();

   QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const;
   bool setData(const QModelIndex &item, const QVariant &value, int role = Qt::EditRole);
   bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

   void clear();
   bool select();

   void setTable(const QString &tableName);
   virtual void setRelation(int column, const QSqlRelation &relation);
   QSqlRelation relation(int column) const;
   virtual QSqlTableModel *relationModel(int column) const;
   void setJoinMode( QSqlRelationalTableModel::JoinMode joinMode );

   SQL_CS_SLOT_1(Public, void revertRow(int row))
   SQL_CS_SLOT_2(revertRow)

 protected:
   QString selectStatement() const;
   bool updateRowInTable(int row, const QSqlRecord &values);
   bool insertRowIntoTable(const QSqlRecord &values);
   QString orderByClause() const;

 private:
   Q_DECLARE_PRIVATE(QSqlRelationalTableModel)
};

QT_END_NAMESPACE

#endif // QSQLRELATIONALTABLEMODEL_H
