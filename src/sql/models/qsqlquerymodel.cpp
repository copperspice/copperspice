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

#include <qsqlquerymodel.h>

#include <qdebug.h>
#include <qsqldriver.h>
#include <qsqlfield.h>

#include <qsqlquerymodel_p.h>

#define QSQL_PREFETCH 255

void QSqlQueryModelPrivate::prefetch(int limit)
{
   Q_Q(QSqlQueryModel);

   if (atEnd || limit <= bottom.row() || bottom.column() == -1) {
      return;
   }

   QModelIndex newBottom;
   const int oldBottomRow = qMax(bottom.row(), 0);

   // try to seek directly
   if (query.seek(limit)) {
      newBottom = q->createIndex(limit, bottom.column());
   } else {
      // have to seek back to our old position for MS Access
      int i = oldBottomRow;
      if (query.seek(i)) {
         while (query.next()) {
            ++i;
         }
         newBottom = q->createIndex(i, bottom.column());
      } else {
         // empty or invalid query
         newBottom = q->createIndex(-1, bottom.column());
      }
      atEnd = true; // this is the end.
   }
   if (newBottom.row() >= 0 && newBottom.row() > bottom.row()) {
      q->beginInsertRows(QModelIndex(), bottom.row() + 1, newBottom.row());
      bottom = newBottom;
      q->endInsertRows();

   } else {
      bottom = newBottom;
   }
}

QSqlQueryModelPrivate::~QSqlQueryModelPrivate()
{
}

void QSqlQueryModelPrivate::initColOffsets(int size)
{
   colOffsets.resize(size);
   memset(colOffsets.data(), 0, colOffsets.size() * sizeof(int));
}

int QSqlQueryModelPrivate::columnInQuery(int modelColumn) const
{
   if (modelColumn < 0 || modelColumn >= rec.count() || !rec.isGenerated(modelColumn) || modelColumn >= colOffsets.size()) {
      return -1;
   }

   return modelColumn - colOffsets[modelColumn];
}

QSqlQueryModel::QSqlQueryModel(QObject *parent)
   : QAbstractTableModel(*new QSqlQueryModelPrivate, parent)
{
}

QSqlQueryModel::QSqlQueryModel(QSqlQueryModelPrivate &dd, QObject *parent)
   : QAbstractTableModel(dd, parent)
{
}

QSqlQueryModel::~QSqlQueryModel()
{
}

void QSqlQueryModel::fetchMore(const QModelIndex &parent)
{
   Q_D(QSqlQueryModel);

   if (parent.isValid()) {
      return;
   }

   d->prefetch(qMax(d->bottom.row(), 0) + QSQL_PREFETCH);
}

bool QSqlQueryModel::canFetchMore(const QModelIndex &parent) const
{
   Q_D(const QSqlQueryModel);
   return (!parent.isValid() && !d->atEnd);
}

void QSqlQueryModel::beginInsertRows(const QModelIndex &parent, int first, int last)
{
   Q_D(QSqlQueryModel);

   if (! d->nestedResetLevel) {
      QAbstractTableModel::beginInsertRows(parent, first, last);
   }
}

void QSqlQueryModel::endInsertRows()
{
   Q_D(QSqlQueryModel);

   if (!d->nestedResetLevel) {
      QAbstractTableModel::endInsertRows();
   }
}

void QSqlQueryModel::beginRemoveRows(const QModelIndex &parent, int first, int last)
{
   Q_D(QSqlQueryModel);

   if (! d->nestedResetLevel) {
      QAbstractTableModel::beginRemoveRows(parent, first, last);
   }
}

void QSqlQueryModel::endRemoveRows()
{
   Q_D(QSqlQueryModel);

   if (! d->nestedResetLevel) {
      QAbstractTableModel::endRemoveRows();
   }
}
void QSqlQueryModel::beginInsertColumns(const QModelIndex &parent, int first, int last)
{
   Q_D(QSqlQueryModel);

   if (!d->nestedResetLevel) {
      QAbstractTableModel::beginInsertColumns(parent, first, last);
   }
}

void QSqlQueryModel::endInsertColumns()
{
   Q_D(QSqlQueryModel);
   if (!d->nestedResetLevel) {
      QAbstractTableModel::endInsertColumns();
   }
}
void QSqlQueryModel::beginRemoveColumns(const QModelIndex &parent, int first, int last)
{
   Q_D(QSqlQueryModel);
   if (!d->nestedResetLevel) {
      QAbstractTableModel::beginRemoveColumns(parent, first, last);
   }
}

void QSqlQueryModel::endRemoveColumns()
{
   Q_D(QSqlQueryModel);
   if (!d->nestedResetLevel) {
      QAbstractTableModel::endRemoveColumns();
   }
}

void QSqlQueryModel::beginResetModel()
{
   Q_D(QSqlQueryModel);
   if (!d->nestedResetLevel) {
      QAbstractTableModel::beginResetModel();
   }
   ++d->nestedResetLevel;
}

void QSqlQueryModel::endResetModel()
{
   Q_D(QSqlQueryModel);
   --d->nestedResetLevel;
   if (!d->nestedResetLevel) {
      QAbstractTableModel::endResetModel();
   }
}

int QSqlQueryModel::rowCount(const QModelIndex &index) const
{
   Q_D(const QSqlQueryModel);
   return index.isValid() ? 0 : d->bottom.row() + 1;
}

int QSqlQueryModel::columnCount(const QModelIndex &index) const
{
   Q_D(const QSqlQueryModel);
   return index.isValid() ? 0 : d->rec.count();
}

QVariant QSqlQueryModel::data(const QModelIndex &item, int role) const
{
   Q_D(const QSqlQueryModel);

   if (! item.isValid()) {
      return QVariant();
   }

   QVariant v;
   if (role & ~(Qt::DisplayRole | Qt::EditRole)) {
      return v;
   }

   if (! d->rec.isGenerated(item.column())) {
      return v;
   }

   QModelIndex dItem = indexInQuery(item);
   if (dItem.row() > d->bottom.row()) {
      const_cast<QSqlQueryModelPrivate *>(d)->prefetch(dItem.row());
   }

   if (! d->query.seek(dItem.row())) {
      d->error = d->query.lastError();
      return v;
   }

   return d->query.value(dItem.column());
}

QVariant QSqlQueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_D(const QSqlQueryModel);
   if (orientation == Qt::Horizontal) {
      QVariant val = d->headers.value(section).value(role);
      if (role == Qt::DisplayRole && !val.isValid()) {
         val = d->headers.value(section).value(Qt::EditRole);
      }
      if (val.isValid()) {
         return val;
      }
      if (role == Qt::DisplayRole && d->rec.count() > section && d->columnInQuery(section) != -1) {
         return d->rec.fieldName(section);
      }
   }
   return QAbstractItemModel::headerData(section, orientation, role);
}

void QSqlQueryModel::queryChange()
{
   // do nothing
}

void QSqlQueryModel::setQuery(const QSqlQuery &query)
{
   Q_D(QSqlQueryModel);
   beginResetModel();

   QSqlRecord newRec = query.record();
   bool columnsChanged = (newRec != d->rec);

   if (d->colOffsets.size() != newRec.count() || columnsChanged) {
      d->initColOffsets(newRec.count());
   }

   d->bottom = QModelIndex();
   d->error = QSqlError();
   d->query = query;
   d->rec = newRec;
   d->atEnd = true;

   if (query.isForwardOnly()) {
      d->error = QSqlError(QLatin1String("Forward-only queries "
               "cannot be used in a data model"),
            QString(), QSqlError::ConnectionError);
      endResetModel();
      return;
   }

   if (!query.isActive()) {
      d->error = query.lastError();
      endResetModel();
      return;
   }

   if (query.driver()->hasFeature(QSqlDriver::QuerySize) && d->query.size() > 0) {
      d->bottom = createIndex(d->query.size() - 1, d->rec.count() - 1);
   } else {
      d->bottom = createIndex(-1, d->rec.count() - 1);
      d->atEnd = false;
   }


   // fetchMore does the rowsInserted stuff for incremental models
   fetchMore();

   endResetModel();
   queryChange();
}

void QSqlQueryModel::setQuery(const QString &query, const QSqlDatabase &db)
{
   setQuery(QSqlQuery(query, db));
}

void QSqlQueryModel::clear()
{
   Q_D(QSqlQueryModel);
   beginResetModel();
   d->error = QSqlError();
   d->atEnd = true;
   d->query.clear();
   d->rec.clear();
   d->colOffsets.clear();
   d->bottom = QModelIndex();
   d->headers.clear();
   endResetModel();
}

bool QSqlQueryModel::setHeaderData(int section, Qt::Orientation orientation,
   const QVariant &value, int role)
{
   Q_D(QSqlQueryModel);
   if (orientation != Qt::Horizontal || section < 0 || columnCount() <= section) {
      return false;
   }

   if (d->headers.size() <= section) {
      d->headers.resize(qMax(section + 1, 16));
   }
   d->headers[section][role] = value;
   emit headerDataChanged(orientation, section, section);
   return true;
}

QSqlQuery QSqlQueryModel::query() const
{
   Q_D(const QSqlQueryModel);
   return d->query;
}

QSqlError QSqlQueryModel::lastError() const
{
   Q_D(const QSqlQueryModel);
   return d->error;
}

void QSqlQueryModel::setLastError(const QSqlError &error)
{
   Q_D(QSqlQueryModel);
   d->error = error;
}


QSqlRecord QSqlQueryModel::record(int row) const
{
   Q_D(const QSqlQueryModel);
   if (row < 0) {
      return d->rec;
   }

   QSqlRecord rec = d->rec;
   for (int i = 0; i < rec.count(); ++i) {
      rec.setValue(i, data(createIndex(row, i), Qt::EditRole));
   }
   return rec;
}

/*! \overload

    Returns an empty record containing information about the fields
    of the current query.

    If the model is not initialized, an empty record will be
    returned.

    \sa QSqlRecord::isEmpty()
 */
QSqlRecord QSqlQueryModel::record() const
{
   Q_D(const QSqlQueryModel);
   return d->rec;
}

/*!
    Inserts \a count columns into the model at position \a column. The
    \a parent parameter must always be an invalid QModelIndex, since
    the model does not support parent-child relationships.

    Returns true if \a column is within bounds; otherwise returns false.

    By default, inserted columns are empty. To fill them with data,
    reimplement data() and handle any inserted column separately:

    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 23

    \sa removeColumns()
*/
bool QSqlQueryModel::insertColumns(int column, int count, const QModelIndex &parent)
{
   Q_D(QSqlQueryModel);
   if (count <= 0 || parent.isValid() || column < 0 || column > d->rec.count()) {
      return false;
   }

   beginInsertColumns(parent, column, column + count - 1);
   for (int c = 0; c < count; ++c) {
      QSqlField field;
      field.setReadOnly(true);
      field.setGenerated(false);
      d->rec.insert(column, field);
      if (d->colOffsets.size() < d->rec.count()) {
         int nVal = d->colOffsets.isEmpty() ? 0 : d->colOffsets[d->colOffsets.size() - 1];
         d->colOffsets.append(nVal);
         Q_ASSERT(d->colOffsets.size() >= d->rec.count());
      }
      for (int i = column + 1; i < d->colOffsets.count(); ++i) {
         ++d->colOffsets[i];
      }
   }
   endInsertColumns();
   return true;
}

bool QSqlQueryModel::removeColumns(int column, int count, const QModelIndex &parent)
{
   Q_D(QSqlQueryModel);
   if (count <= 0 || parent.isValid() || column < 0 || column >= d->rec.count()) {
      return false;
   }

   beginRemoveColumns(parent, column, column + count - 1);

   int i;
   for (i = 0; i < count; ++i) {
      d->rec.remove(column);
   }
   for (i = column; i < d->colOffsets.count(); ++i) {
      d->colOffsets[i] -= count;
   }

   endRemoveColumns();
   return true;
}

QModelIndex QSqlQueryModel::indexInQuery(const QModelIndex &item) const
{
   Q_D(const QSqlQueryModel);
   int modelColumn = d->columnInQuery(item.column());

   if (modelColumn < 0) {
      return QModelIndex();
   }
   return createIndex(item.row(), modelColumn, item.internalPointer());

}

