/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qtablewidget.h>

#ifndef QT_NO_TABLEWIDGET
#include <qitemdelegate.h>
#include <qpainter.h>
#include <qtablewidget_p.h>

#include <algorithm>

QTableModel::QTableModel(int rows, int columns, QTableWidget *parent)
   : QAbstractTableModel(parent),
     prototype(0),
     tableItems(rows * columns, nullptr),
     verticalHeaderItems(rows, nullptr),
     horizontalHeaderItems(columns, nullptr)
{}

QTableModel::~QTableModel()
{
   clear();
   delete prototype;
}

bool QTableModel::insertRows(int row, int count, const QModelIndex &)
{
   if (count < 1 || row < 0 || row > verticalHeaderItems.count()) {
      return false;
   }

   beginInsertRows(QModelIndex(), row, row + count - 1);
   int rc = verticalHeaderItems.count();
   int cc = horizontalHeaderItems.count();
   verticalHeaderItems.insert(row, count, 0);
   if (rc == 0) {
      tableItems.resize(cc * count);
   } else {
      tableItems.insert(tableIndex(row, 0), cc * count, 0);
   }
   endInsertRows();
   return true;
}

bool QTableModel::insertColumns(int column, int count, const QModelIndex &)
{
   if (count < 1 || column < 0 || column > horizontalHeaderItems.count()) {
      return false;
   }

   beginInsertColumns(QModelIndex(), column, column + count - 1);
   int rc = verticalHeaderItems.count();
   int cc = horizontalHeaderItems.count();
   horizontalHeaderItems.insert(column, count, 0);
   if (cc == 0) {
      tableItems.resize(rc * count);
   } else
      for (int row = 0; row < rc; ++row) {
         tableItems.insert(tableIndex(row, column), count, 0);
      }
   endInsertColumns();
   return true;
}

bool QTableModel::removeRows(int row, int count, const QModelIndex &)
{
   if (count < 1 || row < 0 || row + count > verticalHeaderItems.count()) {
      return false;
   }

   beginRemoveRows(QModelIndex(), row, row + count - 1);
   int i = tableIndex(row, 0);
   int n = count * columnCount();
   QTableWidgetItem *oldItem = 0;
   for (int j = i; j < n + i; ++j) {
      oldItem = tableItems.at(j);
      if (oldItem) {
         oldItem->view = 0;
      }
      delete oldItem;
   }
   tableItems.remove(qMax(i, 0), n);
   for (int v = row; v < row + count; ++v) {
      oldItem = verticalHeaderItems.at(v);
      if (oldItem) {
         oldItem->view = 0;
      }
      delete oldItem;
   }
   verticalHeaderItems.remove(row, count);
   endRemoveRows();
   return true;
}

bool QTableModel::removeColumns(int column, int count, const QModelIndex &)
{
   if (count < 1 || column < 0 || column + count >  horizontalHeaderItems.count()) {
      return false;
   }

   beginRemoveColumns(QModelIndex(), column, column + count - 1);
   QTableWidgetItem *oldItem = 0;
   for (int row = rowCount() - 1; row >= 0; --row) {
      int i = tableIndex(row, column);
      for (int j = i; j < i + count; ++j) {
         oldItem = tableItems.at(j);
         if (oldItem) {
            oldItem->view = 0;
         }
         delete oldItem;
      }
      tableItems.remove(i, count);
   }
   for (int h = column; h < column + count; ++h) {
      oldItem = horizontalHeaderItems.at(h);
      if (oldItem) {
         oldItem->view = 0;
      }
      delete oldItem;
   }
   horizontalHeaderItems.remove(column, count);
   endRemoveColumns();
   return true;
}

void QTableModel::setItem(int row, int column, QTableWidgetItem *item)
{
   int i = tableIndex(row, column);
   if (i < 0 || i >= tableItems.count()) {
      return;
   }
   QTableWidgetItem *oldItem = tableItems.at(i);
   if (item == oldItem) {
      return;
   }

   // remove old
   if (oldItem) {
      oldItem->view = 0;
   }
   delete tableItems.at(i);

   QTableWidget *view = qobject_cast<QTableWidget *>(QObject::parent());

   // set new
   if (item) {
      item->d->id = i;
   }
   tableItems[i] = item;

   if (view && view->isSortingEnabled()
      && view->horizontalHeader()->sortIndicatorSection() == column) {
      // sorted insertion
      Qt::SortOrder order = view->horizontalHeader()->sortIndicatorOrder();
      QVector<QTableWidgetItem *> colItems = columnItems(column);
      if (row < colItems.count()) {
         colItems.remove(row);
      }

      int sortedRow;
      if (item == 0) {
         // move to after all non-0 (sortable) items
         sortedRow = colItems.count();
      } else {
         QVector<QTableWidgetItem *>::iterator it;
         it = sortedInsertionIterator(colItems.begin(), colItems.end(), order, item);
         sortedRow = qMax((int)(it - colItems.begin()), 0);
      }

      if (sortedRow != row) {
         emit layoutAboutToBeChanged();
         // move the items @ row to sortedRow
         int cc = columnCount();
         QVector<QTableWidgetItem *> rowItems(cc);
         for (int j = 0; j < cc; ++j) {
            rowItems[j] = tableItems.at(tableIndex(row, j));
         }
         tableItems.remove(tableIndex(row, 0), cc);
         tableItems.insert(tableIndex(sortedRow, 0), cc, 0);
         for (int j = 0; j < cc; ++j) {
            tableItems[tableIndex(sortedRow, j)] = rowItems.at(j);
         }
         QTableWidgetItem *header = verticalHeaderItems.at(row);
         verticalHeaderItems.remove(row);
         verticalHeaderItems.insert(sortedRow, header);
         // update persistent indexes
         QModelIndexList oldPersistentIndexes = persistentIndexList();
         QModelIndexList newPersistentIndexes = oldPersistentIndexes;
         updateRowIndexes(newPersistentIndexes, row, sortedRow);
         changePersistentIndexList(oldPersistentIndexes,
            newPersistentIndexes);

         emit layoutChanged();
         return;
      }
   }

   QModelIndex idx = QAbstractTableModel::index(row, column);
   emit dataChanged(idx, idx);
}

QTableWidgetItem *QTableModel::takeItem(int row, int column)
{
   long i = tableIndex(row, column);
   QTableWidgetItem *itm = tableItems.value(i);
   if (itm) {
      itm->view = 0;
      itm->d->id = -1;
      tableItems[i] = 0;
      QModelIndex ind = index(itm);
      emit dataChanged(ind, ind);
   }
   return itm;
}

QTableWidgetItem *QTableModel::item(int row, int column) const
{
   return item(index(row, column));
}

QTableWidgetItem *QTableModel::item(const QModelIndex &index) const
{
   if (!isValid(index)) {
      return 0;
   }
   return tableItems.at(tableIndex(index.row(), index.column()));
}

void QTableModel::removeItem(QTableWidgetItem *item)
{
   int i = tableItems.indexOf(item);
   if (i != -1) {
      tableItems[i] = 0;
      QModelIndex idx = index(item);
      emit dataChanged(idx, idx);
      return;
   }

   i = verticalHeaderItems.indexOf(item);

   if (i != -1) {
      verticalHeaderItems[i] = 0;
      emit headerDataChanged(Qt::Vertical, i, i);
      return;
   }
   i = horizontalHeaderItems.indexOf(item);
   if (i != -1) {
      horizontalHeaderItems[i] = 0;
      emit headerDataChanged(Qt::Horizontal, i, i);
      return;
   }
}

void QTableModel::setHorizontalHeaderItem(int section, QTableWidgetItem *item)
{
   if (section < 0 || section >= horizontalHeaderItems.count()) {
      return;
   }
   QTableWidgetItem *oldItem = horizontalHeaderItems.at(section);
   if (item == oldItem) {
      return;
   }

   if (oldItem) {
      oldItem->view = 0;
   }
   delete oldItem;

   QTableWidget *view = qobject_cast<QTableWidget *>(QObject::parent());

   if (item) {
      item->view = view;
      item->itemFlags = Qt::ItemFlags(int(item->itemFlags) | ItemIsHeaderItem);
   }
   horizontalHeaderItems[section] = item;
   emit headerDataChanged(Qt::Horizontal, section, section);
}

void QTableModel::setVerticalHeaderItem(int section, QTableWidgetItem *item)
{
   if (section < 0 || section >= verticalHeaderItems.count()) {
      return;
   }
   QTableWidgetItem *oldItem = verticalHeaderItems.at(section);
   if (item == oldItem) {
      return;
   }

   if (oldItem) {
      oldItem->view = 0;
   }
   delete oldItem;

   QTableWidget *view = qobject_cast<QTableWidget *>(QObject::parent());

   if (item) {
      item->view = view;
      item->itemFlags = Qt::ItemFlags(int(item->itemFlags) | ItemIsHeaderItem);
   }
   verticalHeaderItems[section] = item;
   emit headerDataChanged(Qt::Vertical, section, section);
}

QTableWidgetItem *QTableModel::takeHorizontalHeaderItem(int section)
{
   if (section < 0 || section >= horizontalHeaderItems.count()) {
      return 0;
   }
   QTableWidgetItem *itm = horizontalHeaderItems.at(section);
   if (itm) {
      itm->view = 0;
      itm->itemFlags &= ~ItemIsHeaderItem;
      horizontalHeaderItems[section] = 0;
   }
   return itm;
}

QTableWidgetItem *QTableModel::takeVerticalHeaderItem(int section)
{
   if (section < 0 || section >= verticalHeaderItems.count()) {
      return 0;
   }
   QTableWidgetItem *itm = verticalHeaderItems.at(section);
   if (itm) {
      itm->view = 0;
      itm->itemFlags &= ~ItemIsHeaderItem;
      verticalHeaderItems[section] = 0;
   }
   return itm;
}

QTableWidgetItem *QTableModel::horizontalHeaderItem(int section)
{
   return horizontalHeaderItems.value(section);
}

QTableWidgetItem *QTableModel::verticalHeaderItem(int section)
{
   return verticalHeaderItems.value(section);
}

QModelIndex QTableModel::index(const QTableWidgetItem *item) const
{
   if (!item) {
      return QModelIndex();
   }
   int i = -1;
   const int id = item->d->id;
   if (id >= 0 && id < tableItems.count() && tableItems.at(id) == item) {
      i = id;
   } else { // we need to search for the item
      i = tableItems.indexOf(const_cast<QTableWidgetItem *>(item));
      if (i == -1) { // not found
         return QModelIndex();
      }
   }
   int row = i / columnCount();
   int col = i % columnCount();
   return QAbstractTableModel::index(row, col);
}

void QTableModel::setRowCount(int rows)
{
   int rc = verticalHeaderItems.count();
   if (rows < 0 || rc == rows) {
      return;
   }
   if (rc < rows) {
      insertRows(qMax(rc, 0), rows - rc);
   } else {
      removeRows(qMax(rows, 0), rc - rows);
   }
}

void QTableModel::setColumnCount(int columns)
{
   int cc = horizontalHeaderItems.count();
   if (columns < 0 || cc == columns) {
      return;
   }
   if (cc < columns) {
      insertColumns(qMax(cc, 0), columns - cc);
   } else {
      removeColumns(qMax(columns, 0), cc - columns);
   }
}

int QTableModel::rowCount(const QModelIndex &parent) const
{
   return parent.isValid() ? 0 : verticalHeaderItems.count();
}

int QTableModel::columnCount(const QModelIndex &parent) const
{
   return parent.isValid() ? 0 : horizontalHeaderItems.count();
}

QVariant QTableModel::data(const QModelIndex &index, int role) const
{
   QTableWidgetItem *itm = item(index);
   if (itm) {
      return itm->data(role);
   }
   return QVariant();
}

bool QTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   if (!index.isValid()) {
      return false;
   }

   QTableWidgetItem *itm = item(index);
   if (itm) {
      itm->setData(role, value);
      return true;
   }

   // don't create dummy table items for empty values
   if (!value.isValid()) {
      return false;
   }

   QTableWidget *view = qobject_cast<QTableWidget *>(QObject::parent());
   if (!view) {
      return false;
   }

   itm = createItem();
   itm->setData(role, value);
   view->setItem(index.row(), index.column(), itm);
   return true;
}

QMap<int, QVariant> QTableModel::itemData(const QModelIndex &index) const
{
   QMap<int, QVariant> roles;
   QTableWidgetItem *itm = item(index);
   if (itm) {
      for (int i = 0; i < itm->values.count(); ++i) {
         roles.insert(itm->values.at(i).role,
            itm->values.at(i).value);
      }
   }
   return roles;
}

// reimplemented to ensure that only one dataChanged() signal is emitted
bool QTableModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
   if (!index.isValid()) {
      return false;
   }

   QTableWidget *view = qobject_cast<QTableWidget *>(QObject::parent());
   QTableWidgetItem *itm = item(index);
   if (itm) {
      itm->view = 0; // prohibits item from calling itemChanged()
      bool changed = false;

      for (QMap<int, QVariant>::const_iterator it = roles.constBegin(); it != roles.constEnd(); ++it) {
         if (itm->data(it.key()) != it.value()) {
            itm->setData(it.key(), it.value());
            changed = true;
         }
      }
      itm->view = view;
      if (changed) {
         itemChanged(itm);
      }
      return true;
   }

   if (!view) {
      return false;
   }

   itm = createItem();
   for (QMap<int, QVariant>::const_iterator it = roles.constBegin(); it != roles.constEnd(); ++it) {
      itm->setData(it.key(), it.value());
   }
   view->setItem(index.row(), index.column(), itm);
   return true;
}

Qt::ItemFlags QTableModel::flags(const QModelIndex &index) const
{
   if (!index.isValid()) {
      return Qt::ItemIsDropEnabled;
   }
   if (QTableWidgetItem *itm = item(index)) {
      return itm->flags();
   }
   return (Qt::ItemIsEditable
         | Qt::ItemIsSelectable
         | Qt::ItemIsUserCheckable
         | Qt::ItemIsEnabled
         | Qt::ItemIsDragEnabled
         | Qt::ItemIsDropEnabled);
}

void QTableModel::sort(int column, Qt::SortOrder order)
{
   QVector<QPair<QTableWidgetItem *, int>> sortable;
   QVector<int> unsortable;

   sortable.reserve(rowCount());
   unsortable.reserve(rowCount());

   for (int row = 0; row < rowCount(); ++row) {
      if (QTableWidgetItem *itm = item(row, column)) {
         sortable.append(QPair<QTableWidgetItem *, int>(itm, row));
      } else {
         unsortable.append(row);
      }
   }

   LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
   std::stable_sort(sortable.begin(), sortable.end(), compare);

   QVector<QTableWidgetItem *> sorted_table(tableItems.count());
   QModelIndexList from;
   QModelIndexList to;

   const int numRows    = rowCount();
   const int numColumns = columnCount();
   for (int i = 0; i < numRows; ++i) {

      int r = (i < sortable.count()
            ? sortable.at(i).second
            : unsortable.at(i - sortable.count()));

      for (int c = 0; c < numColumns; ++c) {
         sorted_table[tableIndex(i, c)] = item(r, c);
         from.append(createIndex(r, c));
         to.append(createIndex(i, c));
      }
   }

   emit layoutAboutToBeChanged();

   tableItems = sorted_table;
   changePersistentIndexList(from, to); // ### slow

   emit layoutChanged();
}

/*
  \internal

  Ensures that rows in the interval [start, end] are
  sorted according to the contents of column \a column
  and the given sort \a order.
*/
void QTableModel::ensureSorted(int column, Qt::SortOrder order,
   int start, int end)
{
   int count = end - start + 1;
   QVector < QPair<QTableWidgetItem *, int>> sorting;
   sorting.reserve(count);

   for (int row = start; row <= end; ++row) {
      QTableWidgetItem *itm = item(row, column);
      if (itm == 0) {
         // no more sortable items (all 0-items are
         // at the end of the table when it is sorted)
         break;
      }
      sorting.append(QPair<QTableWidgetItem *, int>(itm, row));
   }

   LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
   std::stable_sort(sorting.begin(), sorting.end(), compare);

   QModelIndexList oldPersistentIndexes;
   QModelIndexList newPersistentIndexes;

   QVector<QTableWidgetItem *> newTable = tableItems;
   QVector<QTableWidgetItem *> newVertical = verticalHeaderItems;
   QVector<QTableWidgetItem *> colItems = columnItems(column);
   QVector<QTableWidgetItem *>::iterator vit = colItems.begin();
   bool changed = false;

   for (int i = 0; i < sorting.count(); ++i) {
      int oldRow = sorting.at(i).second;
      QTableWidgetItem *item = colItems.at(oldRow);
      colItems.remove(oldRow);
      vit = sortedInsertionIterator(vit, colItems.end(), order, item);
      int newRow = qMax((int)(vit - colItems.begin()), 0);

      if ((newRow < oldRow) && !(*item < *colItems.at(oldRow - 1)) && !(*colItems.at(oldRow - 1) < *item)) {
         newRow = oldRow;
      }

      vit = colItems.insert(vit, item);
      if (newRow != oldRow) {

         if (!changed) {
            emit layoutAboutToBeChanged();
            oldPersistentIndexes = persistentIndexList();
            newPersistentIndexes = oldPersistentIndexes;
            changed = true;
         }

         // move the items @ oldRow to newRow
         int cc = columnCount();
         QVector<QTableWidgetItem *> rowItems(cc);

         for (int j = 0; j < cc; ++j) {
            rowItems[j] = newTable.at(tableIndex(oldRow, j));
         }

         newTable.remove(tableIndex(oldRow, 0), cc);
         newTable.insert(tableIndex(newRow, 0), cc, 0);
         for (int j = 0; j < cc; ++j) {
            newTable[tableIndex(newRow, j)] = rowItems.at(j);
         }

         QTableWidgetItem *header = newVertical.at(oldRow);
         newVertical.remove(oldRow);
         newVertical.insert(newRow, header);

         // update persistent indexes
         updateRowIndexes(newPersistentIndexes, oldRow, newRow);

         // the index of the remaining rows may have changed
         for (int j = i + 1; j < sorting.count(); ++j) {
            int otherRow = sorting.at(j).second;
            if (oldRow < otherRow && newRow >= otherRow) {
               --sorting[j].second;
            } else if (oldRow > otherRow && newRow <= otherRow) {
               ++sorting[j].second;
            }
         }
      }
   }

   if (changed) {
      tableItems = newTable;
      verticalHeaderItems = newVertical;

      changePersistentIndexList(oldPersistentIndexes, newPersistentIndexes);
      emit layoutChanged();
   }
}

/*
  \internal

  Returns the non-0 items in column \a column.
*/
QVector<QTableWidgetItem *> QTableModel::columnItems(int column) const
{
   QVector<QTableWidgetItem *> items;
   int rc = rowCount();
   items.reserve(rc);
   for (int row = 0; row < rc; ++row) {
      QTableWidgetItem *itm = item(row, column);
      if (itm == 0) {
         // no more sortable items (all 0-items are
         // at the end of the table when it is sorted)
         break;
      }
      items.append(itm);
   }
   return items;
}

/*
  \internal

  Adjusts the row of each index in \a indexes if necessary, given
  that a row of items has been moved from row \a movedFrom to row
  \a movedTo.
*/
void QTableModel::updateRowIndexes(QModelIndexList &indexes,
   int movedFromRow, int movedToRow)
{
   QModelIndexList::iterator it;
   for (it = indexes.begin(); it != indexes.end(); ++it) {
      int oldRow = (*it).row();
      int newRow = oldRow;
      if (oldRow == movedFromRow) {
         newRow = movedToRow;
      } else if (movedFromRow < oldRow && movedToRow >= oldRow) {
         newRow = oldRow - 1;
      } else if (movedFromRow > oldRow && movedToRow <= oldRow) {
         newRow = oldRow + 1;
      }
      if (newRow != oldRow) {
         *it = index(newRow, (*it).column(), (*it).parent());
      }
   }
}

/*
  \internal

  Returns an iterator to the item where \a item should be
  inserted in the interval (\a begin, \a end) according to
  the given sort \a order.
*/
QVector<QTableWidgetItem *>::iterator QTableModel::sortedInsertionIterator(
   const QVector<QTableWidgetItem *>::iterator &begin,
   const QVector<QTableWidgetItem *>::iterator &end,
   Qt::SortOrder order, QTableWidgetItem *item)
{
   if (order == Qt::AscendingOrder) {
      return std::lower_bound(begin, end, item, QTableModelLessThan());
   }
   return std::lower_bound(begin, end, item, QTableModelGreaterThan());
}

bool QTableModel::itemLessThan(const QPair<QTableWidgetItem *, int> &left,
   const QPair<QTableWidgetItem *, int> &right)
{
   return *(left.first) < *(right.first);
}

bool QTableModel::itemGreaterThan(const QPair<QTableWidgetItem *, int> &left,
   const QPair<QTableWidgetItem *, int> &right)
{
   return (*(right.first) < * (left .first));
}

QVariant QTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (section < 0) {
      return QVariant();
   }

   QTableWidgetItem *itm = 0;
   if (orientation == Qt::Horizontal && section < horizontalHeaderItems.count()) {
      itm = horizontalHeaderItems.at(section);
   } else if (orientation == Qt::Vertical && section < verticalHeaderItems.count()) {
      itm = verticalHeaderItems.at(section);
   } else {
      return QVariant();   // section is out of bounds
   }

   if (itm) {
      return itm->data(role);
   }
   if (role == Qt::DisplayRole) {
      return section + 1;
   }
   return QVariant();
}

bool QTableModel::setHeaderData(int section, Qt::Orientation orientation,
   const QVariant &value, int role)
{
   if (section < 0 ||
      (orientation == Qt::Horizontal && horizontalHeaderItems.size() <= section) ||
      (orientation == Qt::Vertical && verticalHeaderItems.size() <= section)) {
      return false;
   }

   QTableWidgetItem *itm = 0;
   if (orientation == Qt::Horizontal) {
      itm = horizontalHeaderItems.at(section);
   } else {
      itm = verticalHeaderItems.at(section);
   }
   if (itm) {
      itm->setData(role, value);
      return true;
   }
   return false;
}

bool QTableModel::isValid(const QModelIndex &index) const
{
   return (index.isValid()
         && index.row() < verticalHeaderItems.count()
         && index.column() < horizontalHeaderItems.count());
}

void QTableModel::clear()
{
   for (int j = 0; j < verticalHeaderItems.count(); ++j) {
      if (verticalHeaderItems.at(j)) {
         verticalHeaderItems.at(j)->view = 0;
         delete verticalHeaderItems.at(j);
         verticalHeaderItems[j] = 0;
      }
   }
   for (int k = 0; k < horizontalHeaderItems.count(); ++k) {
      if (horizontalHeaderItems.at(k)) {
         horizontalHeaderItems.at(k)->view = 0;
         delete horizontalHeaderItems.at(k);
         horizontalHeaderItems[k] = 0;
      }
   }
   clearContents();
}

void QTableModel::clearContents()
{
   beginResetModel();
   for (int i = 0; i < tableItems.count(); ++i) {
      if (tableItems.at(i)) {
         tableItems.at(i)->view = 0;
         delete tableItems.at(i);
         tableItems[i] = 0;
      }
   }
   endResetModel();
}

void QTableModel::itemChanged(QTableWidgetItem *item)
{
   if (!item) {
      return;
   }

   if (item->flags() & ItemIsHeaderItem) {
      int row = verticalHeaderItems.indexOf(item);
      if (row >= 0) {
         emit headerDataChanged(Qt::Vertical, row, row);
      } else {
         int column = horizontalHeaderItems.indexOf(item);
         if (column >= 0) {
            emit headerDataChanged(Qt::Horizontal, column, column);
         }
      }

   } else {
      QModelIndex idx = index(item);
      if (idx.isValid()) {
         emit dataChanged(idx, idx);
      }
   }
}

QTableWidgetItem *QTableModel::createItem() const
{
   return prototype ? prototype->clone() : new QTableWidgetItem;
}

const QTableWidgetItem *QTableModel::itemPrototype() const
{
   return prototype;
}

void QTableModel::setItemPrototype(const QTableWidgetItem *item)
{
   if (prototype != item) {
      delete prototype;
      prototype = item;
   }
}

QStringList QTableModel::mimeTypes() const
{
   const QTableWidget *view = qobject_cast<const QTableWidget *>(QObject::parent());
   return (view ? view->mimeTypes() : QStringList());
}

QMimeData *QTableModel::internalMimeData()  const
{
   return QAbstractTableModel::mimeData(cachedIndexes);
}

QMimeData *QTableModel::mimeData(const QModelIndexList &indexes) const
{
   QList<QTableWidgetItem *> items;
   for (int i = 0; i < indexes.count(); ++i) {
      items << item(indexes.at(i));
   }
   const QTableWidget *view = qobject_cast<const QTableWidget *>(QObject::parent());

   // cachedIndexes is a little hack to avoid copying from QModelIndexList to
   // QList<QTreeWidgetItem*> and back again in the view
   cachedIndexes = indexes;
   QMimeData *mimeData = (view ? view->mimeData(items) : 0);
   cachedIndexes.clear();
   return mimeData;
}

bool QTableModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
   int row, int column, const QModelIndex &index)
{
   if (index.isValid()) {
      row = index.row();
      column = index.column();
   } else if (row == -1 || column == -1) {  // The user dropped outside the table.
      row = rowCount();
      column = 0;
   }

   QTableWidget *view = qobject_cast<QTableWidget *>(QObject::parent());
   return (view ? view->dropMimeData(row, column, data, action) : false);
}

Qt::DropActions QTableModel::supportedDropActions() const
{
   const QTableWidget *view = qobject_cast<const QTableWidget *>(QObject::parent());
   return (view ? view->supportedDropActions() : Qt::DropActions(Qt::IgnoreAction));
}

QTableWidgetSelectionRange::QTableWidgetSelectionRange()
   : top(-1), left(-1), bottom(-2), right(-2)
{
}

/*!
    Constructs the table selection range from the given \a top, \a
    left, \a bottom and \a right table rows and columns.

    \sa topRow(), leftColumn(), bottomRow(), rightColumn()
*/
QTableWidgetSelectionRange::QTableWidgetSelectionRange(int top, int left, int bottom, int right)
   : top(top), left(left), bottom(bottom), right(right)
{
}

/*!
    Constructs a the table selection range by copying the given \a
    other table selection range.
*/
QTableWidgetSelectionRange::QTableWidgetSelectionRange(const QTableWidgetSelectionRange &other)
   : top(other.top), left(other.left), bottom(other.bottom), right(other.right)
{
}

/*!
    Destroys the table selection range.
*/
QTableWidgetSelectionRange::~QTableWidgetSelectionRange()
{
}

void QTableWidgetItem::setFlags(Qt::ItemFlags aflags)
{
   itemFlags = aflags;
   if (QTableModel *model = (view ? qobject_cast<QTableModel *>(view->model()) : 0)) {
      model->itemChanged(this);
   }
}


QTableWidgetItem::QTableWidgetItem(int type)
   :  rtti(type), view(0), d(new QTableWidgetItemPrivate(this)),
      itemFlags(Qt::ItemIsEditable
         | Qt::ItemIsSelectable
         | Qt::ItemIsUserCheckable
         | Qt::ItemIsEnabled
         | Qt::ItemIsDragEnabled
         | Qt::ItemIsDropEnabled)
{
}


QTableWidgetItem::QTableWidgetItem(const QString &text, int type)
   :  rtti(type), view(0), d(new QTableWidgetItemPrivate(this)),
      itemFlags(Qt::ItemIsEditable
         | Qt::ItemIsSelectable
         | Qt::ItemIsUserCheckable
         | Qt::ItemIsEnabled
         | Qt::ItemIsDragEnabled
         | Qt::ItemIsDropEnabled)
{
   setData(Qt::DisplayRole, text);
}

/*!
    Constructs a table item with the given \a icon and \a text.

    \sa type()
*/
QTableWidgetItem::QTableWidgetItem(const QIcon &icon, const QString &text, int type)
   :  rtti(type), view(0), d(new QTableWidgetItemPrivate(this)),
      itemFlags(Qt::ItemIsEditable
         | Qt::ItemIsSelectable
         | Qt::ItemIsUserCheckable
         | Qt::ItemIsEnabled
         | Qt::ItemIsDragEnabled
         | Qt::ItemIsDropEnabled)
{
   setData(Qt::DecorationRole, icon);
   setData(Qt::DisplayRole, text);
}

/*!
    Destroys the table item.
*/
QTableWidgetItem::~QTableWidgetItem()
{
   if (QTableModel *model = (view ? qobject_cast<QTableModel *>(view->model()) : 0)) {
      model->removeItem(this);
   }
   view = 0;
   delete d;
}

/*!
    Creates a copy of the item.
*/
QTableWidgetItem *QTableWidgetItem::clone() const
{
   return new QTableWidgetItem(*this);
}

/*!
    Sets the item's data for the given \a role to the specified \a value.

    \sa Qt::ItemDataRole, data()
*/
void QTableWidgetItem::setData(int role, const QVariant &value)
{
   bool found = false;
   role = (role == Qt::EditRole ? Qt::DisplayRole : role);
   for (int i = 0; i < values.count(); ++i) {
      if (values.at(i).role == role) {
         if (values[i].value == value) {
            return;
         }

         values[i].value = value;
         found = true;
         break;
      }
   }
   if (!found) {
      values.append(QWidgetItemData(role, value));
   }
   if (QTableModel *model = (view ? qobject_cast<QTableModel *>(view->model()) : 0)) {
      model->itemChanged(this);
   }
}

/*!
    Returns the item's data for the given \a role.
*/
QVariant QTableWidgetItem::data(int role) const
{
   role = (role == Qt::EditRole ? Qt::DisplayRole : role);
   for (int i = 0; i < values.count(); ++i)
      if (values.at(i).role == role) {
         return values.at(i).value;
      }
   return QVariant();
}

/*!
    Returns true if the item is less than the \a other item; otherwise returns
    false.
*/
bool QTableWidgetItem::operator<(const QTableWidgetItem &other) const
{
   const QVariant v1 = data(Qt::DisplayRole), v2 = other.data(Qt::DisplayRole);
   return QAbstractItemModelPrivate::variantLessThan(v1, v2);
}



void QTableWidgetItem::read(QDataStream &in)
{
   in >> values;
}

void QTableWidgetItem::write(QDataStream &out) const
{
   out << values;
}

QDataStream &operator>>(QDataStream &in, QTableWidgetItem &item)
{
   item.read(in);
   return in;
}

QDataStream &operator<<(QDataStream &out, const QTableWidgetItem &item)
{
   item.write(out);
   return out;
}

QTableWidgetItem::QTableWidgetItem(const QTableWidgetItem &other)
   : rtti(Type), values(other.values), view(0),
     d(new QTableWidgetItemPrivate(this)),
     itemFlags(other.itemFlags)
{
}

QTableWidgetItem &QTableWidgetItem::operator=(const QTableWidgetItem &other)
{
   values    = other.values;
   itemFlags = other.itemFlags;

   return *this;
}

void QTableWidgetPrivate::setup()
{
   Q_Q(QTableWidget);

   // view signals
   QObject::connect(q, &QTableWidget::pressed,        q, &QTableWidget::_q_emitItemPressed);
   QObject::connect(q, &QTableWidget::clicked,        q, &QTableWidget::_q_emitItemClicked);
   QObject::connect(q, &QTableWidget::doubleClicked,  q, &QTableWidget::_q_emitItemDoubleClicked);
   QObject::connect(q, &QTableWidget::activated,      q, &QTableWidget::_q_emitItemActivated);
   QObject::connect(q, &QTableWidget::entered,        q, &QTableWidget::_q_emitItemEntered);

   // model signals
   QObject::connect(model, &QAbstractItemModel::dataChanged, q, &QTableWidget::_q_emitItemChanged);

   // selection signals
   QObject::connect(q->selectionModel(), &QItemSelectionModel::currentChanged,   q, &QTableWidget::_q_emitCurrentItemChanged);
   QObject::connect(q->selectionModel(), &QItemSelectionModel::selectionChanged, q, &QTableWidget::itemSelectionChanged);

   // sorting
   QObject::connect(model, &QAbstractItemModel::dataChanged,    q, &QTableWidget::_q_dataChanged);

   QObject::connect(model, &QAbstractItemModel::columnsRemoved, q, &QTableWidget::_q_sort);
}

void QTableWidgetPrivate::_q_emitItemPressed(const QModelIndex &index)
{
   Q_Q(QTableWidget);

   if (QTableWidgetItem *item = tableModel()->item(index)) {
      emit q->itemPressed(item);
   }

   emit q->cellPressed(index.row(), index.column());
}

void QTableWidgetPrivate::_q_emitItemClicked(const QModelIndex &index)
{
   Q_Q(QTableWidget);

   if (QTableWidgetItem *item = tableModel()->item(index)) {
      emit q->itemClicked(item);
   }

   emit q->cellClicked(index.row(), index.column());
}

void QTableWidgetPrivate::_q_emitItemDoubleClicked(const QModelIndex &index)
{
   Q_Q(QTableWidget);

   if (QTableWidgetItem *item = tableModel()->item(index)) {
      emit q->itemDoubleClicked(item);
   }

   emit q->cellDoubleClicked(index.row(), index.column());
}

void QTableWidgetPrivate::_q_emitItemActivated(const QModelIndex &index)
{
   Q_Q(QTableWidget);

   if (QTableWidgetItem *item = tableModel()->item(index)) {
      emit q->itemActivated(item);
   }

   emit q->cellActivated(index.row(), index.column());
}

void QTableWidgetPrivate::_q_emitItemEntered(const QModelIndex &index)
{
   Q_Q(QTableWidget);

   if (QTableWidgetItem *item = tableModel()->item(index)) {
      emit q->itemEntered(item);
   }

   emit q->cellEntered(index.row(), index.column());
}

void QTableWidgetPrivate::_q_emitItemChanged(const QModelIndex &index)
{
   Q_Q(QTableWidget);

   if (QTableWidgetItem *item = tableModel()->item(index)) {
      emit q->itemChanged(item);
   }
   emit q->cellChanged(index.row(), index.column());
}

void QTableWidgetPrivate::_q_emitCurrentItemChanged(const QModelIndex &current,
   const QModelIndex &previous)
{
   Q_Q(QTableWidget);
   QTableWidgetItem *currentItem  = tableModel()->item(current);
   QTableWidgetItem *previousItem = tableModel()->item(previous);

   if (currentItem || previousItem) {
      emit q->currentItemChanged(currentItem, previousItem);
   }
   emit q->currentCellChanged(current.row(), current.column(), previous.row(), previous.column());
}

void QTableWidgetPrivate::_q_sort()
{
   if (sortingEnabled) {
      int column = horizontalHeader->sortIndicatorSection();
      Qt::SortOrder order = horizontalHeader->sortIndicatorOrder();
      model->sort(column, order);
   }
}

void QTableWidgetPrivate::_q_dataChanged(const QModelIndex &topLeft,
   const QModelIndex &bottomRight)
{
   if (sortingEnabled && topLeft.isValid() && bottomRight.isValid()) {
      int column = horizontalHeader->sortIndicatorSection();
      if (column >= topLeft.column() && column <= bottomRight.column()) {
         Qt::SortOrder order = horizontalHeader->sortIndicatorOrder();
         tableModel()->ensureSorted(column, order, topLeft.row(), bottomRight.row());
      }
   }
}

QTableWidget::QTableWidget(QWidget *parent)
   : QTableView(*new QTableWidgetPrivate, parent)
{
   Q_D(QTableWidget);
   QTableView::setModel(new QTableModel(0, 0, this));
   d->setup();
}

/*!
    Creates a new table view with the given \a rows and \a columns, and with the given \a parent.
*/
QTableWidget::QTableWidget(int rows, int columns, QWidget *parent)
   : QTableView(*new QTableWidgetPrivate, parent)
{
   Q_D(QTableWidget);
   QTableView::setModel(new QTableModel(rows, columns, this));
   d->setup();
}

/*!
    Destroys this QTableWidget.
*/
QTableWidget::~QTableWidget()
{
}

/*!
    Sets the number of rows in this table's model to \a rows. If
    this is less than rowCount(), the data in the unwanted rows
    is discarded.

    \sa setColumnCount()
*/
void QTableWidget::setRowCount(int rows)
{
   Q_D(QTableWidget);
   d->tableModel()->setRowCount(rows);
}

/*!
  Returns the number of rows.
*/

int QTableWidget::rowCount() const
{
   Q_D(const QTableWidget);
   return d->model->rowCount();
}

/*!
    Sets the number of columns in this table's model to \a columns. If
    this is less than columnCount(), the data in the unwanted columns
    is discarded.

    \sa setRowCount()
*/
void QTableWidget::setColumnCount(int columns)
{
   Q_D(QTableWidget);
   d->tableModel()->setColumnCount(columns);
}

/*!
  Returns the number of columns.
*/

int QTableWidget::columnCount() const
{
   Q_D(const QTableWidget);
   return d->model->columnCount();
}

/*!
  Returns the row for the \a item.
*/
int QTableWidget::row(const QTableWidgetItem *item) const
{
   Q_D(const QTableWidget);
   return d->tableModel()->index(item).row();
}

/*!
  Returns the column for the \a item.
*/
int QTableWidget::column(const QTableWidgetItem *item) const
{
   Q_D(const QTableWidget);
   return d->tableModel()->index(item).column();
}


/*!
    Returns the item for the given \a row and \a column if one has been set; otherwise
    returns 0.

    \sa setItem()
*/
QTableWidgetItem *QTableWidget::item(int row, int column) const
{
   Q_D(const QTableWidget);
   return d->tableModel()->item(row, column);
}

void QTableWidget::setItem(int row, int column, QTableWidgetItem *item)
{
   Q_D(QTableWidget);
   if (item) {
      if (item->view != 0) {
         qWarning("QTableWidget: cannot insert an item that is already owned by another QTableWidget");
      } else {
         item->view = this;
         d->tableModel()->setItem(row, column, item);
      }
   } else {
      delete takeItem(row, column);
   }
}

/*!
    Removes the item at \a row and \a column from the table without deleting it.
*/
QTableWidgetItem *QTableWidget::takeItem(int row, int column)
{
   Q_D(QTableWidget);
   QTableWidgetItem *item = d->tableModel()->takeItem(row, column);

   if (item) {
      item->view = 0;
   }
   return item;
}

/*!
  Returns the vertical header item for row \a row.
*/
QTableWidgetItem *QTableWidget::verticalHeaderItem(int row) const
{
   Q_D(const QTableWidget);
   return d->tableModel()->verticalHeaderItem(row);
}

/*!
  Sets the vertical header item for row \a row to \a item.
*/
void QTableWidget::setVerticalHeaderItem(int row, QTableWidgetItem *item)
{
   Q_D(QTableWidget);
   if (item) {
      item->view = this;
      d->tableModel()->setVerticalHeaderItem(row, item);
   } else {
      delete takeVerticalHeaderItem(row);
   }
}


QTableWidgetItem *QTableWidget::takeVerticalHeaderItem(int row)
{
   Q_D(QTableWidget);
   QTableWidgetItem *itm = d->tableModel()->takeVerticalHeaderItem(row);
   if (itm) {
      itm->view = 0;
   }
   return itm;
}

/*!
    Returns the horizontal header item for column, \a column, if one has been
    set; otherwise returns 0.
*/
QTableWidgetItem *QTableWidget::horizontalHeaderItem(int column) const
{
   Q_D(const QTableWidget);
   return d->tableModel()->horizontalHeaderItem(column);
}

/*!
  Sets the horizontal header item for column \a column to \a item.
*/
void QTableWidget::setHorizontalHeaderItem(int column, QTableWidgetItem *item)
{
   Q_D(QTableWidget);
   if (item) {
      item->view = this;
      d->tableModel()->setHorizontalHeaderItem(column, item);
   } else {
      delete takeHorizontalHeaderItem(column);
   }
}

/*!
  \since 4.1
    Removes the horizontal header item at \a column from the header without deleting it.
*/
QTableWidgetItem *QTableWidget::takeHorizontalHeaderItem(int column)
{
   Q_D(QTableWidget);
   QTableWidgetItem *itm = d->tableModel()->takeHorizontalHeaderItem(column);
   if (itm) {
      itm->view = 0;
   }
   return itm;
}

/*!
  Sets the vertical header labels using \a labels.
*/
void QTableWidget::setVerticalHeaderLabels(const QStringList &labels)
{
   Q_D(QTableWidget);
   QTableModel *model = d->tableModel();
   QTableWidgetItem *item = 0;
   for (int i = 0; i < model->rowCount() && i < labels.count(); ++i) {
      item = model->verticalHeaderItem(i);
      if (!item) {
         item = model->createItem();
         setVerticalHeaderItem(i, item);
      }
      item->setText(labels.at(i));
   }
}

/*!
  Sets the horizontal header labels using \a labels.
*/
void QTableWidget::setHorizontalHeaderLabels(const QStringList &labels)
{
   Q_D(QTableWidget);
   QTableModel *model = d->tableModel();
   QTableWidgetItem *item = 0;
   for (int i = 0; i < model->columnCount() && i < labels.count(); ++i) {
      item = model->horizontalHeaderItem(i);
      if (!item) {
         item = model->createItem();
         setHorizontalHeaderItem(i, item);
      }
      item->setText(labels.at(i));
   }
}

/*!
    Returns the row of the current item.

    \sa currentColumn(), setCurrentCell()
*/
int QTableWidget::currentRow() const
{
   return currentIndex().row();
}

/*!
    Returns the column of the current item.

    \sa currentRow(), setCurrentCell()
*/
int QTableWidget::currentColumn() const
{
   return currentIndex().column();
}

/*!
    Returns the current item.

    \sa setCurrentItem()
*/
QTableWidgetItem *QTableWidget::currentItem() const
{
   Q_D(const QTableWidget);
   return d->tableModel()->item(currentIndex());
}


void QTableWidget::setCurrentItem(QTableWidgetItem *item)
{
   Q_D(QTableWidget);
   setCurrentIndex(d->tableModel()->index(item));
}

void QTableWidget::setCurrentItem(QTableWidgetItem *item, QItemSelectionModel::SelectionFlags command)
{
   Q_D(QTableWidget);
   d->selectionModel->setCurrentIndex(d->tableModel()->index(item), command);
}

void QTableWidget::setCurrentCell(int row, int column)
{
   setCurrentIndex(model()->index(row, column, QModelIndex()));
}

void QTableWidget::setCurrentCell(int row, int column, QItemSelectionModel::SelectionFlags command)
{
   Q_D(QTableWidget);
   d->selectionModel->setCurrentIndex(model()->index(row, column, QModelIndex()), command);
}

/*!
  Sorts all the rows in the table widget based on \a column and \a order.
*/
void QTableWidget::sortItems(int column, Qt::SortOrder order)
{
   Q_D(QTableWidget);
   d->model->sort(column, order);
   horizontalHeader()->setSortIndicator(column, order);
}

/*!
    \internal
*/
void QTableWidget::setSortingEnabled(bool enable)
{
   QTableView::setSortingEnabled(enable);
}

/*!
    \internal
*/
bool QTableWidget::isSortingEnabled() const
{
   return QTableView::isSortingEnabled();
}

/*!
  Starts editing the \a item if it is editable.
*/

void QTableWidget::editItem(QTableWidgetItem *item)
{
   Q_D(QTableWidget);
   if (!item) {
      return;
   }
   edit(d->tableModel()->index(item));
}

/*!
  Opens an editor for the give \a item. The editor remains open after editing.

  \sa closePersistentEditor()
*/
void QTableWidget::openPersistentEditor(QTableWidgetItem *item)
{
   Q_D(QTableWidget);
   if (!item) {
      return;
   }
   QModelIndex index = d->tableModel()->index(item);
   QAbstractItemView::openPersistentEditor(index);
}

/*!
  Closes the persistent editor for \a item.

  \sa openPersistentEditor()
*/
void QTableWidget::closePersistentEditor(QTableWidgetItem *item)
{
   Q_D(QTableWidget);
   if (!item) {
      return;
   }
   QModelIndex index = d->tableModel()->index(item);
   QAbstractItemView::closePersistentEditor(index);
}

/*!
    \since 4.1

    Returns the widget displayed in the cell in the given \a row and \a column.

    \note The table takes ownership of the widget.

    \sa setCellWidget()
*/
QWidget *QTableWidget::cellWidget(int row, int column) const
{
   QModelIndex index = model()->index(row, column, QModelIndex());
   return QAbstractItemView::indexWidget(index);
}


void QTableWidget::setCellWidget(int row, int column, QWidget *widget)
{
   QModelIndex index = model()->index(row, column, QModelIndex());
   QAbstractItemView::setIndexWidget(index, widget);
}

/*!
  Returns true if the \a item is selected, otherwise returns false.

  \obsolete

  This function is deprecated. Use \l{QTableWidgetItem::isSelected()} instead.
*/

bool QTableWidget::isItemSelected(const QTableWidgetItem *item) const
{
   Q_D(const QTableWidget);
   QModelIndex index = d->tableModel()->index(item);
   return selectionModel()->isSelected(index);
}

/*!
  Selects or deselects \a item depending on \a select.

  \obsolete

  This function is deprecated. Use \l{QTableWidgetItem::setSelected()} instead.
*/
void QTableWidget::setItemSelected(const QTableWidgetItem *item, bool select)
{
   Q_D(QTableWidget);
   QModelIndex index = d->tableModel()->index(item);
   selectionModel()->select(index, select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

/*!
  Selects or deselects the \a range depending on \a select.
*/
void QTableWidget::setRangeSelected(const QTableWidgetSelectionRange &range, bool select)
{
   if (!model()->hasIndex(range.topRow(), range.leftColumn(), rootIndex()) ||
      !model()->hasIndex(range.bottomRow(), range.rightColumn(), rootIndex())) {
      return;
   }

   QModelIndex topLeft = model()->index(range.topRow(), range.leftColumn(), rootIndex());
   QModelIndex bottomRight = model()->index(range.bottomRow(), range.rightColumn(), rootIndex());

   selectionModel()->select(QItemSelection(topLeft, bottomRight),
      select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

/*!
  Returns a list of all selected ranges.

  \sa QTableWidgetSelectionRange
*/

QList<QTableWidgetSelectionRange> QTableWidget::selectedRanges() const
{
   const QList<QItemSelectionRange> ranges = selectionModel()->selection();
   QList<QTableWidgetSelectionRange> result;
   for (int i = 0; i < ranges.count(); ++i) {
      result.append(QTableWidgetSelectionRange(ranges.at(i).top(),
            ranges.at(i).left(),
            ranges.at(i).bottom(),
            ranges.at(i).right()));
   }

   return result;
}

/*!
  Returns a list of all selected items.

  This function returns a list of pointers to the contents of the
  selected cells. Use the selectedIndexes() function to retrieve the
  complete selection \e including empty cells.

  \sa selectedIndexes()
*/

QList<QTableWidgetItem *> QTableWidget::selectedItems() const
{
   Q_D(const QTableWidget);

   QModelIndexList indexes = selectionModel()->selectedIndexes();
   QList<QTableWidgetItem *> items;

   for (int i = 0; i < indexes.count(); ++i) {
      QModelIndex index = indexes.at(i);
      if (isIndexHidden(index)) {
         continue;
      }
      QTableWidgetItem *item = d->tableModel()->item(index);
      if (item) {
         items.append(item);
      }
   }
   return items;
}

/*!
  Finds items that matches the \a text using the given \a flags.
*/

QList<QTableWidgetItem *> QTableWidget::findItems(const QString &text, Qt::MatchFlags flags) const
{
   Q_D(const QTableWidget);
   QModelIndexList indexes;
   for (int column = 0; column < columnCount(); ++column)
      indexes += d->model->match(model()->index(0, column, QModelIndex()),
            Qt::DisplayRole, text, -1, flags);
   QList<QTableWidgetItem *> items;
   for (int i = 0; i < indexes.size(); ++i) {
      items.append(d->tableModel()->item(indexes.at(i)));
   }
   return items;
}

/*!
  Returns the visual row of the given \a logicalRow.
*/

int QTableWidget::visualRow(int logicalRow) const
{
   return verticalHeader()->visualIndex(logicalRow);
}

/*!
  Returns the visual column of the given \a logicalColumn.
*/

int QTableWidget::visualColumn(int logicalColumn) const
{
   return horizontalHeader()->visualIndex(logicalColumn);
}

/*!
  \fn QTableWidgetItem *QTableWidget::itemAt(const QPoint &point) const

  Returns a pointer to the item at the given \a point, or returns 0 if
  \a point is not covered by an item in the table widget.

  \sa item()
*/

QTableWidgetItem *QTableWidget::itemAt(const QPoint &p) const
{
   Q_D(const QTableWidget);
   return d->tableModel()->item(indexAt(p));
}

/*!
  Returns the rectangle on the viewport occupied by the item at \a item.
*/
QRect QTableWidget::visualItemRect(const QTableWidgetItem *item) const
{
   Q_D(const QTableWidget);
   if (!item) {
      return QRect();
   }

   QModelIndex index = d->tableModel()->index(const_cast<QTableWidgetItem *>(item));
   Q_ASSERT(index.isValid());
   return visualRect(index);
}

/*!
    Scrolls the view if necessary to ensure that the \a item is visible.
    The \a hint parameter specifies more precisely where the
    \a item should be located after the operation.
*/

void QTableWidget::scrollToItem(const QTableWidgetItem *item, QAbstractItemView::ScrollHint hint)
{
   Q_D(QTableWidget);
   if (!item) {
      return;
   }

   QModelIndex index = d->tableModel()->index(const_cast<QTableWidgetItem *>(item));
   Q_ASSERT(index.isValid());
   QTableView::scrollTo(index, hint);
}

/*!
    Returns the item prototype used by the table.

    \sa setItemPrototype()
*/
const QTableWidgetItem *QTableWidget::itemPrototype() const
{
   Q_D(const QTableWidget);
   return d->tableModel()->itemPrototype();
}


void QTableWidget::setItemPrototype(const QTableWidgetItem *item)
{
   Q_D(QTableWidget);
   d->tableModel()->setItemPrototype(item);
}

/*!
  Inserts an empty row into the table at \a row.
*/
void QTableWidget::insertRow(int row)
{
   Q_D(QTableWidget);
   d->tableModel()->insertRows(row);
}

/*!
  Inserts an empty column into the table at \a column.
*/
void QTableWidget::insertColumn(int column)
{
   Q_D(QTableWidget);
   d->tableModel()->insertColumns(column);
}

/*!
  Removes the row \a row and all its items from the table.
*/
void QTableWidget::removeRow(int row)
{
   Q_D(QTableWidget);
   d->tableModel()->removeRows(row);
}

/*!
  Removes the column \a column and all its items from the table.
*/
void QTableWidget::removeColumn(int column)
{
   Q_D(QTableWidget);
   d->tableModel()->removeColumns(column);
}

/*!
   Removes all items in the view.
   This will also remove all selections.
   The table dimensions stay the same.
*/

void QTableWidget::clear()
{
   Q_D(QTableWidget);
   selectionModel()->clear();
   d->tableModel()->clear();
}


void QTableWidget::clearContents()
{
   Q_D(QTableWidget);
   selectionModel()->clear();
   d->tableModel()->clearContents();
}

/*!
    Returns a list of MIME types that can be used to describe a list of
    tablewidget items.

    \sa mimeData()
*/
QStringList QTableWidget::mimeTypes() const
{
   return d_func()->tableModel()->QAbstractTableModel::mimeTypes();
}

QMimeData *QTableWidget::mimeData(const QList<QTableWidgetItem *> &items) const

{
   Q_D(const QTableWidget);

   QModelIndexList &cachedIndexes = d->tableModel()->cachedIndexes;

   // if non empty, it's called from the model's own mimeData
   if (cachedIndexes.isEmpty()) {

      for (QTableWidgetItem *item : items) {
         cachedIndexes << indexFromItem(item);
      }

      QMimeData *result = d->tableModel()->internalMimeData();

      cachedIndexes.clear();
      return result;
   }

   return d->tableModel()->internalMimeData();
}


bool QTableWidget::dropMimeData(int row, int column, const QMimeData *data, Qt::DropAction action)
{
   QModelIndex idx;
#ifndef QT_NO_DRAGANDDROP
   if (dropIndicatorPosition() == QAbstractItemView::OnItem) {
      // QAbstractTableModel::dropMimeData will overwrite on the index if row == -1 and column == -1
      idx = model()->index(row, column);
      row = -1;
      column = -1;
   }
#endif
   return d_func()->tableModel()->QAbstractTableModel::dropMimeData(data, action, row, column, idx);
}

/*!
  Returns the drop actions supported by this view.

  \sa Qt::DropActions
*/
Qt::DropActions QTableWidget::supportedDropActions() const
{
   return d_func()->tableModel()->QAbstractTableModel::supportedDropActions() | Qt::MoveAction;
}

/*!
  Returns a list of pointers to the items contained in the \a data object.
  If the object was not created by a QTreeWidget in the same process, the list
  is empty.

*/
QList<QTableWidgetItem *> QTableWidget::items(const QMimeData *data) const
{
   const QTableWidgetMimeData *twd = qobject_cast<const QTableWidgetMimeData *>(data);
   if (twd) {
      return twd->items;
   }
   return QList<QTableWidgetItem *>();
}

/*!
  Returns the QModelIndex assocated with the given \a item.
*/

QModelIndex QTableWidget::indexFromItem(QTableWidgetItem *item) const
{
   Q_D(const QTableWidget);
   return d->tableModel()->index(item);
}

/*!
  Returns a pointer to the QTableWidgetItem assocated with the given \a index.
*/

QTableWidgetItem *QTableWidget::itemFromIndex(const QModelIndex &index) const
{
   Q_D(const QTableWidget);
   return d->tableModel()->item(index);
}

/*!
    \internal
*/
void QTableWidget::setModel(QAbstractItemModel * /*model*/)
{
   Q_ASSERT(!"QTableWidget::setModel() - Changing the model of the QTableWidget is not allowed.");
}

/*! \reimp */
bool QTableWidget::event(QEvent *e)
{
   return QTableView::event(e);
}

#ifndef QT_NO_DRAGANDDROP
/*! \reimp */
void QTableWidget::dropEvent(QDropEvent *event)
{
   Q_D(QTableWidget);
   if (event->source() == this && (event->dropAction() == Qt::MoveAction ||
         dragDropMode() == QAbstractItemView::InternalMove)) {
      QModelIndex topIndex;
      int col = -1;
      int row = -1;
      if (d->dropOn(event, &row, &col, &topIndex)) {
         QModelIndexList indexes = selectedIndexes();
         int top = INT_MAX;
         int left = INT_MAX;
         for (int i = 0; i < indexes.count(); ++i) {
            top = qMin(indexes.at(i).row(), top);
            left = qMin(indexes.at(i).column(), left);
         }

         QList<QTableWidgetItem *> taken;
         for (int i = 0; i < indexes.count(); ++i) {
            taken.append(takeItem(indexes.at(i).row(), indexes.at(i).column()));
         }

         for (int i = 0; i < indexes.count(); ++i) {
            QModelIndex index = indexes.at(i);
            int r = index.row() - top + topIndex.row();
            int c = index.column() - left + topIndex.column();
            setItem(r, c, taken.takeFirst());
         }

         event->accept();
         // Don't want QAbstractItemView to delete it because it was "moved" we already did it
         event->setDropAction(Qt::CopyAction);
      }
   }

   QTableView::dropEvent(event);
}
#endif

void QTableWidget::_q_emitItemPressed(const QModelIndex &index)
{
   Q_D(QTableWidget);
   d->_q_emitItemPressed(index);
}

void QTableWidget::_q_emitItemClicked(const QModelIndex &index)
{
   Q_D(QTableWidget);
   d->_q_emitItemClicked(index);
}

void QTableWidget::_q_emitItemDoubleClicked(const QModelIndex &index)
{
   Q_D(QTableWidget);
   d->_q_emitItemDoubleClicked(index);
}

void QTableWidget::_q_emitItemActivated(const QModelIndex &index)
{
   Q_D(QTableWidget);
   d->_q_emitItemActivated(index);
}

void QTableWidget::_q_emitItemEntered(const QModelIndex &index)
{
   Q_D(QTableWidget);
   d->_q_emitItemEntered(index);
}

void QTableWidget::_q_emitItemChanged(const QModelIndex &index)
{
   Q_D(QTableWidget);
   d->_q_emitItemChanged(index);
}

void QTableWidget::_q_emitCurrentItemChanged(const QModelIndex &previous, const QModelIndex &current)
{
   Q_D(QTableWidget);
   d->_q_emitCurrentItemChanged(previous, current);
}

void QTableWidget::_q_sort()
{
   Q_D(QTableWidget);
   d->_q_sort();
}

void QTableWidget::_q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
   Q_D(QTableWidget);
   d->_q_dataChanged(topLeft, bottomRight);
}



#endif // QT_NO_TABLEWIDGET
