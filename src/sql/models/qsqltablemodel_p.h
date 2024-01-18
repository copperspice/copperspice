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

#ifndef QSQLTABLEMODEL_P_H
#define QSQLTABLEMODEL_P_H

#include <qmap.h>
#include <qsqlquerymodel_p.h>

class QSqlTableModelPrivate: public QSqlQueryModelPrivate
{
   Q_DECLARE_PUBLIC(QSqlTableModel)

 public:
   QSqlTableModelPrivate()
      : sortColumn(-1), sortOrder(Qt::AscendingOrder),
        strategy(QSqlTableModel::OnRowChange), busyInsertingRows(false)
   { }

   ~QSqlTableModelPrivate();
   void clear();

   virtual void clearCache();

   QSqlRecord record(const QVector<QVariant> &values) const;

   bool exec(const QString &stmt, bool prepStatement, const QSqlRecord &rec, const QSqlRecord &whereValues);
   virtual void revertCachedRow(int row);

   virtual int nameToIndex(const QString &name) const;
   QString strippedFieldName(const QString &name) const;
   int insertCount(int maxRow = -1) const;
   void initRecordAndPrimaryIndex();

   QSqlDatabase db;

   int sortColumn;
   Qt::SortOrder sortOrder;

   QSqlTableModel::EditStrategy strategy;
   bool busyInsertingRows;

   QSqlQuery editQuery;
   QSqlIndex primaryIndex;
   QString tableName;
   QString filter;
   QString autoColumn;

   enum Op { None, Insert, Update, Delete };

   class ModifiedRow
   {
    public:
      ModifiedRow(Op o = None, const QSqlRecord &r = QSqlRecord())
         : m_op(None), m_db_values(r), m_insert(o == Insert)
      {
         setOp(o);
      }

      Op op() const {
         return m_op;
      }

      void setOp(Op o) {
         if (o == None) {
            m_submitted = true;
         }

         if (o == m_op) {
            return;
         }

         m_submitted = (o != Insert && o != Delete);
         m_op = o;
         m_rec = m_db_values;
         setGenerated(m_rec, m_op == Delete);
      }

      QSqlRecord rec() const {
         return m_rec;
      }

      QSqlRecord &recRef() {
         return m_rec;
      }

      void setValue(int c, const QVariant &v) {
         m_submitted = false;
         m_rec.setValue(c, v);
         m_rec.setGenerated(c, true);
      }

      bool submitted() const {
         return m_submitted;
      }

      void setSubmitted() {
         m_submitted = true;
         setGenerated(m_rec, false);

         if (m_op == Delete) {
            m_rec.clearValues();
         } else {
            m_op = Update;
            m_db_values = m_rec;
            setGenerated(m_db_values, true);
         }
      }

      void refresh(bool exists, const QSqlRecord &newvals) {
         m_submitted = true;

         if (exists) {
            m_op = Update;
            m_db_values = newvals;
            m_rec = newvals;
            setGenerated(m_rec, false);
         } else {
            m_op = Delete;
            m_rec.clear();
            m_db_values.clear();
         }
      }

      bool insert() const {
         return m_insert;
      }

      void revert() {
         if (m_submitted) {
            return;
         }
         if (m_op == Delete) {
            m_op = Update;
         }
         m_rec = m_db_values;
         setGenerated(m_rec, false);
         m_submitted = true;
      }

      QSqlRecord primaryValues(const QSqlRecord &pi) const {
         if (m_op == None || m_op == Insert) {
            return QSqlRecord();
         }

         return m_db_values.keyValues(pi);
      }

    private:
      static void setGenerated(QSqlRecord &r, bool g) {
         for (int i = r.count() - 1; i >= 0; --i) {
            r.setGenerated(i, g);
         }
      }

      Op m_op;
      QSqlRecord m_rec;
      QSqlRecord m_db_values;
      bool m_submitted;
      bool m_insert;
   };

   using CacheMap = QMap<int, ModifiedRow>;
   CacheMap cache;
};

class QSqlTableModelSql: public QSqlQueryModelSql
{
 public:
};

#endif
