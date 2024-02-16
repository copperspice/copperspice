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

#include <qsqltablemodel.h>

#include <qsqldriver.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlquery.h>
#include <qsqlrecord.h>
#include <qsqlresult.h>

#include <qsqltablemodel_p.h>

#include <qdebug.h>

typedef QSqlTableModelSql Sql;

QSqlTableModelPrivate::~QSqlTableModelPrivate()
{
}

QSqlRecord QSqlTableModelPrivate::record(const QVector<QVariant> &values) const
{
   QSqlRecord r = rec;
   for (int i = 0; i < r.count() && i < values.count(); ++i) {
      r.setValue(i, values.at(i));
   }
   return r;
}

int QSqlTableModelPrivate::nameToIndex(const QString &name) const
{
   return rec.indexOf(strippedFieldName(name));
}

QString QSqlTableModelPrivate::strippedFieldName(const QString &name) const
{
   QString fieldname = name;
   if (db.driver()->isIdentifierEscaped(fieldname, QSqlDriver::FieldName)) {
      fieldname = db.driver()->stripDelimiters(fieldname, QSqlDriver::FieldName);
   }
   return fieldname;
}

int QSqlTableModelPrivate::insertCount(int maxRow) const
{
   int cnt = 0;
   CacheMap::const_iterator i = cache.constBegin();
   const CacheMap::const_iterator e = cache.constEnd();

   for ( ; i != e && (maxRow < 0 || i.key() <= maxRow); ++i)
      if (i.value().insert()) {
         ++cnt;
      }

   return cnt;
}

void QSqlTableModelPrivate::initRecordAndPrimaryIndex()
{
   rec = db.record(tableName);
   primaryIndex = db.primaryIndex(tableName);
   initColOffsets(rec.count());
}

void QSqlTableModelPrivate::clear()
{

   sortColumn = -1;
   sortOrder = Qt::AscendingOrder;
   tableName.clear();
   editQuery.clear();

   cache.clear();
   primaryIndex.clear();
   rec.clear();
   filter.clear();
}

void QSqlTableModelPrivate::clearCache()
{
   cache.clear();
}

void QSqlTableModelPrivate::revertCachedRow(int row)
{
   Q_Q(QSqlTableModel);
   ModifiedRow r = cache.value(row);

   switch (r.op()) {
      case QSqlTableModelPrivate::None:
         Q_ASSERT_X(false, "QSqlTableModelPrivate::revertCachedRow()", "Invalid entry in cache map");
         return;
      case QSqlTableModelPrivate::Update:
      case QSqlTableModelPrivate::Delete:
         if (!r.submitted()) {
            cache[row].revert();
            emit q->dataChanged(q->createIndex(row, 0),
               q->createIndex(row, q->columnCount() - 1));
         }
         break;
      case QSqlTableModelPrivate::Insert: {
         QMap<int, QSqlTableModelPrivate::ModifiedRow>::iterator it = cache.find(row);

         if (it == cache.end()) {
            return;
         }

         q->beginRemoveRows(QModelIndex(), row, row);
         it = cache.erase(it);

         while (it != cache.end()) {
            int oldKey = it.key();
            const QSqlTableModelPrivate::ModifiedRow oldValue = it.value();
            cache.erase(it);
            it = cache.insert(oldKey - 1, oldValue);
            ++it;
         }
         q->endRemoveRows();
         break;
      }
   }
}

bool QSqlTableModelPrivate::exec(const QString &stmt, bool prepStatement,
   const QSqlRecord &rec, const QSqlRecord &whereValues)
{
   if (stmt.isEmpty()) {
      return false;
   }

   // lazy initialization of editQuery
   if (editQuery.driver() != db.driver()) {
      editQuery = QSqlQuery(db);
   }

   // workaround for In-Process databases - remove all read locks
   // from the table to make sure the editQuery succeeds
   if (db.driver()->hasFeature(QSqlDriver::SimpleLocking)) {
      const_cast<QSqlResult *>(query.result())->detachFromResultSet();
   }

   if (prepStatement) {
      if (editQuery.lastQuery() != stmt) {
         if (!editQuery.prepare(stmt)) {
            error = editQuery.lastError();
            return false;
         }
      }
      int i;
      for (i = 0; i < rec.count(); ++i) {
         if (rec.isGenerated(i)) {
            editQuery.addBindValue(rec.value(i));
         }
      }
      for (i = 0; i < whereValues.count(); ++i) {
         if (whereValues.isGenerated(i) && !whereValues.isNull(i)) {
            editQuery.addBindValue(whereValues.value(i));
         }
      }

      if (!editQuery.exec()) {
         error = editQuery.lastError();
         return false;
      }
   } else {
      if (!editQuery.exec(stmt)) {
         error = editQuery.lastError();
         return false;
      }
   }
   return true;
}

QSqlTableModel::QSqlTableModel(QObject *parent, QSqlDatabase db)
   : QSqlQueryModel(*new QSqlTableModelPrivate, parent)
{
   Q_D(QSqlTableModel);
   d->db = db.isValid() ? db : QSqlDatabase::database();
}

QSqlTableModel::QSqlTableModel(QSqlTableModelPrivate &dd, QObject *parent, QSqlDatabase db)
   : QSqlQueryModel(dd, parent)
{
   Q_D(QSqlTableModel);
   d->db = db.isValid() ? db : QSqlDatabase::database();
}

QSqlTableModel::~QSqlTableModel()
{
}

void QSqlTableModel::setTable(const QString &tableName)
{
   Q_D(QSqlTableModel);
   clear();
   d->tableName = tableName;
   d->initRecordAndPrimaryIndex();

   if (d->rec.count() == 0)
      d->error = QSqlError(QLatin1String("Unable to find table ") + d->tableName, QString(),
            QSqlError::StatementError);
   d->autoColumn.clear();
   for (int c = 0; c < d->rec.count(); ++c) {
      if (d->rec.field(c).isAutoValue()) {
         d->autoColumn = d->rec.fieldName(c);
         break;
      }
   }
}

QString QSqlTableModel::tableName() const
{
   Q_D(const QSqlTableModel);
   return d->tableName;
}

bool QSqlTableModel::select()
{
   Q_D(QSqlTableModel);
   const QString query = selectStatement();
   if (query.isEmpty()) {
      return false;
   }

   beginResetModel();

   d->clearCache();

   QSqlQuery qu(query, d->db);
   setQuery(qu);

   if (!qu.isActive() || lastError().isValid()) {
      // something went wrong - revert to non-select state
      d->initRecordAndPrimaryIndex();
      endResetModel();
      return false;
   }
   endResetModel();
   return true;
}

bool QSqlTableModel::selectRow(int row)
{
   Q_D(QSqlTableModel);

   if (row < 0 || row >= rowCount()) {
      return false;
   }

   const int table_sort_col = d->sortColumn;
   d->sortColumn = -1;
   const QString table_filter = d->filter;
   d->filter = d->db.driver()->sqlStatement(QSqlDriver::WhereStatement,
         d->tableName,
         primaryValues(row),
         false);
   static const QString wh = Sql::where() + Sql::sp();
   if (d->filter.startsWith(wh, Qt::CaseInsensitive)) {
      d->filter.remove(0, wh.length());
   }

   QString stmt;

   if (!d->filter.isEmpty()) {
      stmt = selectStatement();
   }

   d->sortColumn = table_sort_col;
   d->filter = table_filter;

   if (stmt.isEmpty()) {
      return false;
   }

   bool exists;
   QSqlRecord newValues;

   {
      QSqlQuery q(d->db);
      q.setForwardOnly(true);
      if (!q.exec(stmt)) {
         return false;
      }

      exists = q.next();
      newValues = q.record();
   }

   bool needsAddingToCache = !exists || d->cache.contains(row);

   if (!needsAddingToCache) {
      const QSqlRecord curValues = record(row);
      needsAddingToCache = curValues.count() != newValues.count();
      if (!needsAddingToCache) {
         // Look for changed values. Primary key fields are customarily first
         // and probably change less often than other fields, so start at the end.
         for (int f = curValues.count() - 1; f >= 0; --f) {
            if (curValues.value(f) != newValues.value(f)) {
               needsAddingToCache = true;
               break;
            }
         }
      }
   }

   if (needsAddingToCache) {
      d->cache[row].refresh(exists, newValues);
      emit headerDataChanged(Qt::Vertical, row, row);
      emit dataChanged(createIndex(row, 0), createIndex(row, columnCount() - 1));
   }

   return true;
}

QVariant QSqlTableModel::data(const QModelIndex &index, int role) const
{
   Q_D(const QSqlTableModel);

   if (! index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole)) {
      return QVariant();
   }

   const QSqlTableModelPrivate::ModifiedRow mrow = d->cache.value(index.row());
   if (mrow.op() != QSqlTableModelPrivate::None) {
      return mrow.rec().value(index.column());
   }

   return QSqlQueryModel::data(index, role);
}

QVariant QSqlTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_D(const QSqlTableModel);

   if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
      const QSqlTableModelPrivate::Op op = d->cache.value(section).op();
      if (op == QSqlTableModelPrivate::Insert) {
         return QLatin1String("*");
      } else if (op == QSqlTableModelPrivate::Delete) {
         return QLatin1String("!");
      }
   }

   return QSqlQueryModel::headerData(section, orientation, role);
}

bool QSqlTableModel::isDirty() const
{
   Q_D(const QSqlTableModel);
   QSqlTableModelPrivate::CacheMap::const_iterator i = d->cache.constBegin();
   const QSqlTableModelPrivate::CacheMap::const_iterator e = d->cache.constEnd();
   for (; i != e; ++i) {
      if (!i.value().submitted()) {
         return true;
      }
   }
   return false;
}

bool QSqlTableModel::isDirty(const QModelIndex &index) const
{
   Q_D(const QSqlTableModel);
   if (!index.isValid()) {
      return false;
   }

   const QSqlTableModelPrivate::ModifiedRow row = d->cache.value(index.row());
   if (row.submitted()) {
      return false;
   }

   return row.op() == QSqlTableModelPrivate::Insert
      || row.op() == QSqlTableModelPrivate::Delete
      || (row.op() == QSqlTableModelPrivate::Update
         && row.rec().isGenerated(index.column()));
}

bool QSqlTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   Q_D(QSqlTableModel);
   if (d->busyInsertingRows) {
      return false;
   }
   if (role != Qt::EditRole) {
      return QSqlQueryModel::setData(index, value, role);
   }

   if (!index.isValid() || index.column() >= d->rec.count() || index.row() >= rowCount()) {
      return false;
   }
   if (!(flags(index) & Qt::ItemIsEditable)) {
      return false;
   }

   const QVariant oldValue = QSqlTableModel::data(index, role);

   if (value == oldValue && d->cache.value(index.row()).op() != QSqlTableModelPrivate::Insert) {
      return true;
   }

   QSqlTableModelPrivate::ModifiedRow &row = d->cache[index.row()];

   if (row.op() == QSqlTableModelPrivate::None)
      row = QSqlTableModelPrivate::ModifiedRow(QSqlTableModelPrivate::Update,
            QSqlQueryModel::record(index.row()));

   row.setValue(index.column(), value);
   emit dataChanged(index, index);

   if (d->strategy == OnFieldChange && row.op() != QSqlTableModelPrivate::Insert) {
      return submit();
   }

   return true;
}

void QSqlTableModel::setQuery(const QSqlQuery &query)
{
   QSqlQueryModel::setQuery(query);
}

bool QSqlTableModel::updateRowInTable(int row, const QSqlRecord &values)
{
   Q_D(QSqlTableModel);
   QSqlRecord rec(values);
   emit beforeUpdate(row, rec);

   const QSqlRecord whereValues = primaryValues(row);
   const bool prepStatement = d->db.driver()->hasFeature(QSqlDriver::PreparedQueries);
   const QString stmt = d->db.driver()->sqlStatement(QSqlDriver::UpdateStatement, d->tableName,
         rec, prepStatement);
   const QString where = d->db.driver()->sqlStatement(QSqlDriver::WhereStatement, d->tableName,
         whereValues, prepStatement);

   if (stmt.isEmpty() || where.isEmpty() || row < 0 || row >= rowCount()) {
      d->error = QSqlError(QLatin1String("No Fields to update"), QString(),
            QSqlError::StatementError);
      return false;
   }


   return d->exec(Sql::concat(stmt, where), prepStatement, rec, whereValues);
}

bool QSqlTableModel::insertRowIntoTable(const QSqlRecord &values)
{
   Q_D(QSqlTableModel);
   QSqlRecord rec = values;
   emit beforeInsert(rec);

   const bool prepStatement = d->db.driver()->hasFeature(QSqlDriver::PreparedQueries);
   const QString stmt = d->db.driver()->sqlStatement(QSqlDriver::InsertStatement, d->tableName,
         rec, prepStatement);

   if (stmt.isEmpty()) {
      d->error = QSqlError(QLatin1String("No Fields to update"), QString(),
            QSqlError::StatementError);
      return false;
   }

   return d->exec(stmt, prepStatement, rec, QSqlRecord());
}

bool QSqlTableModel::deleteRowFromTable(int row)
{
   Q_D(QSqlTableModel);

   emit beforeDelete(row);

   const QSqlRecord whereValues = primaryValues(row);
   const bool prepStatement = d->db.driver()->hasFeature(QSqlDriver::PreparedQueries);

   const QString stmt = d->db.driver()->sqlStatement(QSqlDriver::DeleteStatement,
         d->tableName, QSqlRecord(), prepStatement);

   const QString where = d->db.driver()->sqlStatement(QSqlDriver::WhereStatement,
         d->tableName, whereValues, prepStatement);

   if (stmt.isEmpty() || where.isEmpty()) {
      d->error = QSqlError(QLatin1String("Unable to delete row"), QString(), QSqlError::StatementError);
      return false;
   }

   return d->exec(Sql::concat(stmt, where), prepStatement, QSqlRecord(), whereValues);
}

bool QSqlTableModel::submitAll()
{
   Q_D(QSqlTableModel);

   bool success = true;

   for (int row : d->cache.keys()) {
      // be sure cache *still* contains the row since overridden selectRow() could have called select()
      QSqlTableModelPrivate::CacheMap::iterator it = d->cache.find(row);
      if (it == d->cache.end()) {
         continue;
      }

      QSqlTableModelPrivate::ModifiedRow &mrow = it.value();
      if (mrow.submitted()) {
         continue;
      }

      switch (mrow.op()) {
         case QSqlTableModelPrivate::Insert:
            success = insertRowIntoTable(mrow.rec());
            break;

         case QSqlTableModelPrivate::Update:
            success = updateRowInTable(row, mrow.rec());
            break;

         case QSqlTableModelPrivate::Delete:
            success = deleteRowFromTable(row);
            break;

         case QSqlTableModelPrivate::None:
            Q_ASSERT_X(false, "QSqlTableModel::submitAll()", "Invalid cache operation");
            break;
      }

      if (success) {
         if (d->strategy != OnManualSubmit && mrow.op() == QSqlTableModelPrivate::Insert) {
            int c = mrow.rec().indexOf(d->autoColumn);
            if (c != -1 && !mrow.rec().isGenerated(c)) {
               mrow.setValue(c, d->editQuery.lastInsertId());
            }
         }
         mrow.setSubmitted();
         if (d->strategy != OnManualSubmit) {
            success = selectRow(row);
         }
      }

      if (! success) {
         break;
      }
   }

   if (success) {
      if (d->strategy == OnManualSubmit) {
         success = select();
      }
   }

   return success;
}

bool QSqlTableModel::submit()
{
   Q_D(QSqlTableModel);

   if (d->strategy == OnRowChange || d->strategy == OnFieldChange) {
      return submitAll();
   }

   return true;
}

void QSqlTableModel::revert()
{
   Q_D(QSqlTableModel);
   if (d->strategy == OnRowChange || d->strategy == OnFieldChange) {
      revertAll();
   }
}

void QSqlTableModel::setEditStrategy(EditStrategy strategy)
{
   Q_D(QSqlTableModel);
   revertAll();
   d->strategy = strategy;
}

QSqlTableModel::EditStrategy QSqlTableModel::editStrategy() const
{
   Q_D(const QSqlTableModel);
   return d->strategy;
}

void QSqlTableModel::revertAll()
{
   Q_D(QSqlTableModel);

   const QList<int> rows(d->cache.keys());
   for (int i = rows.size() - 1; i >= 0; --i) {
      revertRow(rows.value(i));
   }
}


void QSqlTableModel::revertRow(int row)
{
   if (row < 0) {
      return;
   }

   Q_D(QSqlTableModel);
   d->revertCachedRow(row);


}

QSqlIndex QSqlTableModel::primaryKey() const
{
   Q_D(const QSqlTableModel);
   return d->primaryIndex;
}

void QSqlTableModel::setPrimaryKey(const QSqlIndex &key)
{
   Q_D(QSqlTableModel);
   d->primaryIndex = key;
}

QSqlDatabase QSqlTableModel::database() const
{
   Q_D(const QSqlTableModel);
   return d->db;
}

void QSqlTableModel::sort(int column, Qt::SortOrder order)
{
   setSort(column, order);
   select();
}

void QSqlTableModel::setSort(int column, Qt::SortOrder order)
{
   Q_D(QSqlTableModel);
   d->sortColumn = column;
   d->sortOrder = order;
}

QString QSqlTableModel::orderByClause() const
{
   Q_D(const QSqlTableModel);

   QSqlField f = d->rec.field(d->sortColumn);
   if (!f.isValid()) {
      return QString();
   }

   //we can safely escape the field because it would have been obtained from the database
   //and have the correct case
   QString field = d->db.driver()->escapeIdentifier(f.name(), QSqlDriver::FieldName);
   field.prepend(QLatin1Char('.')).prepend(d->tableName);
   field = d->sortOrder == Qt::AscendingOrder ? Sql::asc(field) : Sql::desc(field);
   return Sql::orderBy(field);

}

int QSqlTableModel::fieldIndex(const QString &fieldName) const
{
   Q_D(const QSqlTableModel);
   return d->rec.indexOf(fieldName);
}


QString QSqlTableModel::selectStatement() const
{
   Q_D(const QSqlTableModel);
   if (d->tableName.isEmpty()) {
      d->error = QSqlError(QLatin1String("No table name given"), QString(),
            QSqlError::StatementError);
      return QString();
   }
   if (d->rec.isEmpty()) {
      d->error = QSqlError(QLatin1String("Unable to find table ") + d->tableName, QString(),
            QSqlError::StatementError);
      return QString();
   }

   const QString stmt = d->db.driver()->sqlStatement(QSqlDriver::SelectStatement,
         d->tableName,
         d->rec,
         false);
   if (stmt.isEmpty()) {
      d->error = QSqlError(QLatin1String("Unable to select fields from table ") + d->tableName,
            QString(), QSqlError::StatementError);
      return stmt;
   }
   return Sql::concat(Sql::concat(stmt, Sql::where(d->filter)), orderByClause());
}


bool QSqlTableModel::removeColumns(int column, int count, const QModelIndex &parent)
{
   Q_D(QSqlTableModel);
   if (parent.isValid() || column < 0 || column + count > d->rec.count()) {
      return false;
   }
   for (int i = 0; i < count; ++i) {
      d->rec.remove(column);
   }
   if (d->query.isActive()) {
      return select();
   }
   return true;
}

bool QSqlTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
   Q_D(QSqlTableModel);

   if (parent.isValid() || row < 0 || count <= 0) {
      return false;

   } else if (row + count > rowCount()) {
      return false;

   } else if (!count) {
      return true;
   }


   if (d->strategy != OnManualSubmit)
      if (count > 1 || (d->cache.value(row).submitted() && isDirty())) {
         return false;
      }

   // Iterate backwards so we don't have to worry about removed rows causing
   // higher cache entries to shift downwards.
   for (int idx = row + count - 1; idx >= row; --idx) {
      QSqlTableModelPrivate::ModifiedRow &mrow = d->cache[idx];
      if (mrow.op() == QSqlTableModelPrivate::Insert) {
         revertRow(idx);
      } else {
         if (mrow.op() == QSqlTableModelPrivate::None)
            mrow = QSqlTableModelPrivate::ModifiedRow(QSqlTableModelPrivate::Delete,
                  QSqlQueryModel::record(idx));
         else {
            mrow.setOp(QSqlTableModelPrivate::Delete);
         }
         if (d->strategy == OnManualSubmit) {
            emit headerDataChanged(Qt::Vertical, idx, idx);
         }
      }
   }

   if (d->strategy != OnManualSubmit) {
      return submit();
   }

   return true;
}

bool QSqlTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
   Q_D(QSqlTableModel);

   if (row < 0 || count <= 0 || row > rowCount() || parent.isValid()) {
      return false;
   }

   if (d->strategy != OnManualSubmit)
      if (count != 1 || isDirty()) {
         return false;
      }

   d->busyInsertingRows = true;



   beginInsertRows(parent, row, row + count - 1);

   if (d->strategy != OnManualSubmit) {
      d->cache.empty();
   }
   if (! d->cache.isEmpty()) {
      QMap<int, QSqlTableModelPrivate::ModifiedRow>::iterator it = d->cache.end();

      while (it != d->cache.begin() && (--it).key() >= row) {
         int oldKey = it.key();

         const QSqlTableModelPrivate::ModifiedRow oldValue = it.value();

         d->cache.erase(it);
         it = d->cache.insert(oldKey + count, oldValue);
      }
   }

   for (int i = 0; i < count; ++i) {
      d->cache[row + i] = QSqlTableModelPrivate::ModifiedRow(QSqlTableModelPrivate::Insert, d->rec);
      emit primeInsert(row + i, d->cache[row + i].recRef());
   }




   endInsertRows();
   d->busyInsertingRows = false;

   return true;
}

bool QSqlTableModel::insertRecord(int row, const QSqlRecord &record)
{

   if (row < 0) {
      row = rowCount();
   }

   if (!insertRow(row, QModelIndex())) {
      return false;
   }

   if (!setRecord(row, record)) {
      revertRow(row);
      return false;
   }

   return true;
}

int QSqlTableModel::rowCount(const QModelIndex &parent) const
{
   Q_D(const QSqlTableModel);

   if (parent.isValid()) {
      return 0;
   }

   return QSqlQueryModel::rowCount() + d->insertCount();
}


QModelIndex QSqlTableModel::indexInQuery(const QModelIndex &item) const
{
   Q_D(const QSqlTableModel);

   if (d->cache.value(item.row()).insert()) {
      return QModelIndex();
   }

   const int rowOffset = d->insertCount(item.row());
   return QSqlQueryModel::indexInQuery(createIndex(item.row() - rowOffset, item.column(), item.internalPointer()));
}

QString QSqlTableModel::filter() const
{
   Q_D(const QSqlTableModel);
   return d->filter;
}

void QSqlTableModel::setFilter(const QString &filter)
{
   Q_D(QSqlTableModel);
   d->filter = filter;
   if (d->query.isActive()) {
      select();
   }
}

void QSqlTableModel::clear()
{
   Q_D(QSqlTableModel);
   beginResetModel();
   d->clear();
   QSqlQueryModel::clear();
   endResetModel();
}

Qt::ItemFlags QSqlTableModel::flags(const QModelIndex &index) const
{
   Q_D(const QSqlTableModel);

   if (index.internalPointer() || index.column() < 0 || index.column() >= d->rec.count() || index.row() < 0) {
      return Qt::EmptyFlag;
   }

   bool editable = true;

   if (d->rec.field(index.column()).isReadOnly()) {
      editable = false;
   } else {
      const QSqlTableModelPrivate::ModifiedRow mrow = d->cache.value(index.row());

      if (mrow.op() == QSqlTableModelPrivate::Delete) {
         editable = false;

      } else if (d->strategy == OnFieldChange) {
         if (mrow.op() != QSqlTableModelPrivate::Insert)
            if (!isDirty(index) && isDirty()) {
               editable = false;
            }
      } else if (d->strategy == OnRowChange) {
         if (mrow.submitted() && isDirty()) {
            editable = false;
         }
      }
   }

   if (!editable) {
      return QSqlQueryModel::flags(index);
   } else {
      return QSqlQueryModel::flags(index) | Qt::ItemIsEditable;
   }
}

QSqlRecord QSqlTableModel::record() const
{
   return QSqlQueryModel::record();
}

QSqlRecord QSqlTableModel::record(int row) const
{
   Q_D(const QSqlTableModel);

   // the query gets the values from virtual data()
   QSqlRecord rec = QSqlQueryModel::record(row);

   // get generated flags from the cache
   const QSqlTableModelPrivate::ModifiedRow mrow = d->cache.value(row);
   if (mrow.op() != QSqlTableModelPrivate::None) {
      const QSqlRecord crec = mrow.rec();
      for (int i = 0, cnt = rec.count(); i < cnt; ++i) {
         rec.setGenerated(i, crec.isGenerated(i));
      }
   }

   return rec;
}

bool QSqlTableModel::setRecord(int row, const QSqlRecord &values)
{
   Q_D(QSqlTableModel);
   Q_ASSERT_X(row >= 0, "QSqlTableModel::setRecord()", "Cannot set a record to a row less than 0");
   if (d->busyInsertingRows) {
      return false;
   }

   if (row >= rowCount()) {
      return false;
   }

   if (d->cache.value(row).op() == QSqlTableModelPrivate::Delete) {
      return false;
   }

   if (d->strategy != OnManualSubmit && d->cache.value(row).submitted() && isDirty()) {
      return false;
   }

   // Check field names and remember mapping
   typedef QMap<int, int> Map;
   Map map;
   for (int i = 0; i < values.count(); ++i) {
      int idx = d->nameToIndex(values.fieldName(i));
      if (idx == -1) {
         return false;
      }
      map[i] = idx;
   }

   QSqlTableModelPrivate::ModifiedRow &mrow = d->cache[row];
   if (mrow.op() == QSqlTableModelPrivate::None)
      mrow = QSqlTableModelPrivate::ModifiedRow(QSqlTableModelPrivate::Update,
            QSqlQueryModel::record(row));

   Map::const_iterator i = map.constBegin();
   const Map::const_iterator e = map.constEnd();
   for ( ; i != e; ++i) {
      // have to use virtual setData() here rather than mrow.setValue()
      EditStrategy strategy = d->strategy;
      d->strategy = OnManualSubmit;
      QModelIndex cIndex = createIndex(row, i.value());
      setData(cIndex, values.value(i.key()));
      d->strategy = strategy;
      // setData() sets generated to TRUE, but source record should prevail.
      if (!values.isGenerated(i.key())) {
         mrow.recRef().setGenerated(i.value(), false);
      }
   }

   if (d->strategy != OnManualSubmit) {
      return submit();
   }

   return true;
}

QSqlRecord QSqlTableModel::primaryValues(int row) const
{
   Q_D(const QSqlTableModel);
   const QSqlRecord &pIndex = d->primaryIndex.isEmpty() ? d->rec : d->primaryIndex;
   QSqlTableModelPrivate::ModifiedRow mr = d->cache.value(row);
   if (mr.op() != QSqlTableModelPrivate::None) {
      return mr.primaryValues(pIndex);
   } else {
      return QSqlQueryModel::record(row).keyValues(pIndex);
   }
}
