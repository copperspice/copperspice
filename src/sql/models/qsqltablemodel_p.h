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

#ifndef QSQLTABLEMODEL_P_H
#define QSQLTABLEMODEL_P_H

#include <QtCore/qmap.h>
#include <qsqlquerymodel_p.h>

QT_BEGIN_NAMESPACE

class QSqlTableModelPrivate: public QSqlQueryModelPrivate
{
   Q_DECLARE_PUBLIC(QSqlTableModel)

 public:
   QSqlTableModelPrivate()
      : editIndex(-1), insertIndex(-1), sortColumn(-1),
        sortOrder(Qt::AscendingOrder),
        strategy(QSqlTableModel::OnRowChange) {
   }
   void clear();
   QSqlRecord primaryValues(int index);
   virtual void clearEditBuffer();
   virtual void clearCache();
   static void clearGenerated(QSqlRecord &rec);
   static void setGeneratedValue(QSqlRecord &rec, int c, QVariant v);
   QSqlRecord record(const QVector<QVariant> &values) const;

   bool exec(const QString &stmt, bool prepStatement, const QSqlRecord &rec, const QSqlRecord &whereValues);
   virtual void revertCachedRow(int row);
   void revertInsertedRow();
   bool setRecord(int row, const QSqlRecord &record);
   virtual int nameToIndex(const QString &name) const;
   void initRecordAndPrimaryIndex();

   QSqlDatabase db;
   int editIndex;
   int insertIndex;

   int sortColumn;
   Qt::SortOrder sortOrder;

   QSqlTableModel::EditStrategy strategy;

   QSqlQuery editQuery;
   QSqlIndex primaryIndex;
   QString tableName;
   QString filter;

   enum Op { None, Insert, Update, Delete };

   struct ModifiedRow {
      ModifiedRow(Op o = None, const QSqlRecord &r = QSqlRecord())
         : op(o), rec(r) {
         clearGenerated(rec);
      }

      ModifiedRow(const ModifiedRow &other): op(other.op), rec(other.rec), primaryValues(other.primaryValues) {}

      Op op;
      QSqlRecord rec;
      QSqlRecord primaryValues;
   };

   QSqlRecord editBuffer;

   using CacheMap = QMap<int, ModifiedRow>;
   CacheMap cache;
};

QT_END_NAMESPACE

#endif // QSQLTABLEMODEL_P_H
