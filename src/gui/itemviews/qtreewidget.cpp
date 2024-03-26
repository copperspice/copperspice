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

#include <qtreewidget.h>

#ifndef QT_NO_TREEWIDGET

#include <qheaderview.h>
#include <qpainter.h>
#include <qitemdelegate.h>
#include <qstack.h>
#include <qdebug.h>

#include <qtreewidget_p.h>
#include <qwidgetitemdata_p.h>
#include <qtreewidgetitemiterator_p.h>

#include <algorithm>

// emerald - workaround for VC++ 6.0 linker bug
typedef bool(*LessThan)(const QPair<QTreeWidgetItem *, int> &, const QPair<QTreeWidgetItem *, int> &);

class QTreeModelLessThan
{
 public:
   inline bool operator()(QTreeWidgetItem *i1, QTreeWidgetItem *i2) const {
      return *i1 < *i2;
   }
};

class QTreeModelGreaterThan
{
 public:
   inline bool operator()(QTreeWidgetItem *i1, QTreeWidgetItem *i2) const {
      return *i2 < *i1;
   }
};

QTreeModel::QTreeModel(int columns, QTreeWidget *parent)
   : QAbstractItemModel(parent), rootItem(new QTreeWidgetItem),
     headerItem(new QTreeWidgetItem), skipPendingSort(false)
{
   rootItem->view = parent;
   rootItem->itemFlags = Qt::ItemIsDropEnabled;
   headerItem->view = parent;
   setColumnCount(columns);
}

QTreeModel::QTreeModel(QTreeModelPrivate &dd, QTreeWidget *parent)
   : QAbstractItemModel(dd, parent), rootItem(new QTreeWidgetItem),
     headerItem(new QTreeWidgetItem), skipPendingSort(false)
{
   rootItem->view = parent;
   rootItem->itemFlags = Qt::ItemIsDropEnabled;
   headerItem->view = parent;
}

QTreeModel::~QTreeModel()
{
   clear();
   headerItem->view = nullptr;
   delete headerItem;
   rootItem->view = nullptr;
   delete rootItem;
}

void QTreeModel::clear()
{
   SkipSorting skipSorting(this);
   beginResetModel();
   for (int i = 0; i < rootItem->childCount(); ++i) {
      QTreeWidgetItem *item = rootItem->children.at(i);
      item->par  = nullptr;
      item->view = nullptr;
      delete item;
   }
   rootItem->children.clear();
   sortPendingTimer.stop();
   endResetModel();
}

void QTreeModel::setColumnCount(int columns)
{
   SkipSorting skipSorting(this);
   if (columns < 0) {
      return;
   }
   if (!headerItem) {
      headerItem = new QTreeWidgetItem();
      headerItem->view = view();
   }
   int count = columnCount();
   if (count == columns) {
      return;
   }

   if (columns < count) {
      beginRemoveColumns(QModelIndex(), columns, count - 1);
      headerItem->values.resize(columns);
      endRemoveColumns();
   } else {
      beginInsertColumns(QModelIndex(), count, columns - 1);
      headerItem->values.resize(columns);
      for (int i = count; i < columns; ++i) {// insert data without emitting the dataChanged signal
         headerItem->values[i].append(QWidgetItemData(Qt::DisplayRole, QString::number(i + 1)));
         headerItem->d->display.append(QString::number(i + 1));
      }
      endInsertColumns();
   }
}

QTreeWidgetItem *QTreeModel::item(const QModelIndex &index) const
{
   if (!index.isValid()) {
      return nullptr;
   }
   return static_cast<QTreeWidgetItem *>(index.internalPointer());
}

QModelIndex QTreeModel::index(const QTreeWidgetItem *item, int column) const
{
   executePendingSort();

   if (!item || (item == rootItem)) {
      return QModelIndex();
   }
   const QTreeWidgetItem *par = item->parent();
   QTreeWidgetItem *itm = const_cast<QTreeWidgetItem *>(item);
   if (!par) {
      par = rootItem;
   }
   int row;
   int guess = item->d->rowGuess;
   if (guess >= 0
      && par->children.count() > guess
      && par->children.at(guess) == itm) {
      row = guess;
   } else {
      row = par->children.lastIndexOf(itm);
      itm->d->rowGuess = row;
   }
   return createIndex(row, column, itm);
}

QModelIndex QTreeModel::index(int row, int column, const QModelIndex &parent) const
{
   executePendingSort();

   int c = columnCount(parent);
   if (row < 0 || column < 0 || column >= c) {
      return QModelIndex();
   }

   QTreeWidgetItem *parentItem = parent.isValid() ? item(parent) : rootItem;
   if (parentItem && row < parentItem->childCount()) {
      QTreeWidgetItem *itm = parentItem->child(row);
      if (itm) {
         return createIndex(row, column, itm);
      }
      return QModelIndex();
   }

   return QModelIndex();
}

QModelIndex QTreeModel::parent(const QModelIndex &child) const
{
   SkipSorting skipSorting(this);

   // The reason we do not sort here is that this might be called from a valid QPersistentModelIndex
   // do not want it to become suddenly invalid

   if (!child.isValid()) {
      return QModelIndex();
   }
   QTreeWidgetItem *itm = static_cast<QTreeWidgetItem *>(child.internalPointer());
   if (!itm || itm == rootItem) {
      return QModelIndex();
   }
   QTreeWidgetItem *parent = itm->parent();
   return index(parent, 0);
}

int QTreeModel::rowCount(const QModelIndex &parent) const
{
   if (!parent.isValid()) {
      return rootItem->childCount();
   }

   QTreeWidgetItem *parentItem = item(parent);
   if (parentItem) {
      return parentItem->childCount();
   }
   return 0;
}

int QTreeModel::columnCount(const QModelIndex &index) const
{
   (void) index;

   if (! headerItem) {
      return 0;
   }

   return headerItem->columnCount();
}

bool QTreeModel::hasChildren(const QModelIndex &parent) const
{
   if (!parent.isValid()) {
      return (rootItem->childCount() > 0);
   }

   QTreeWidgetItem *itm = item(parent);
   if (!itm) {
      return false;
   }

   switch (itm->d->policy) {
      case QTreeWidgetItem::ShowIndicator:
         return true;
      case QTreeWidgetItem::DontShowIndicator:
         return false;
      case QTreeWidgetItem::DontShowIndicatorWhenChildless:
         return (itm->childCount() > 0);
   }

   return false;
}

QVariant QTreeModel::data(const QModelIndex &index, int role) const
{
   if (!index.isValid()) {
      return QVariant();
   }
   QTreeWidgetItem *itm = item(index);
   if (itm) {
      return itm->data(index.column(), role);
   }
   return QVariant();
}

bool QTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   if (!index.isValid()) {
      return false;
   }
   QTreeWidgetItem *itm = item(index);
   if (itm) {
      itm->setData(index.column(), role, value);
      return true;
   }
   return false;
}

QMap<int, QVariant> QTreeModel::itemData(const QModelIndex &index) const
{
   QMap<int, QVariant> roles;
   QTreeWidgetItem *itm = item(index);
   if (itm) {
      int column = index.column();
      if (column < itm->values.count()) {
         for (int i = 0; i < itm->values.at(column).count(); ++i) {
            roles.insert(itm->values.at(column).at(i).role,
               itm->values.at(column).at(i).value);
         }
      }

      // the two special cases
      QVariant displayValue = itm->data(column, Qt::DisplayRole);
      if (displayValue.isValid()) {
         roles.insert(Qt::DisplayRole, displayValue);
      }

      QVariant checkValue = itm->data(column, Qt::CheckStateRole);
      if (checkValue.isValid()) {
         roles.insert(Qt::CheckStateRole, checkValue);
      }
   }
   return roles;
}

bool QTreeModel::insertRows(int row, int count, const QModelIndex &parent)
{
   SkipSorting skipSorting(this);
   if (count < 1 || row < 0 || row > rowCount(parent) || parent.column() > 0) {
      return false;
   }

   beginInsertRows(parent, row, row + count - 1);
   QTreeWidgetItem *par = item(parent);
   while (count > 0) {
      QTreeWidgetItem *item = new QTreeWidgetItem();
      item->view = view();
      item->par = par;
      if (par) {
         par->children.insert(row++, item);
      } else {
         rootItem->children.insert(row++, item);
      }
      --count;
   }
   endInsertRows();
   return true;
}

bool QTreeModel::insertColumns(int column, int count, const QModelIndex &parent)
{
   SkipSorting skipSorting(this);
   if (count < 1 || column < 0 || column > columnCount(parent) || parent.column() > 0 || !headerItem) {
      return false;
   }

   beginInsertColumns(parent, column, column + count - 1);

   int oldCount = columnCount(parent);
   column = qBound(0, column, oldCount);
   headerItem->values.resize(oldCount + count);
   for (int i = oldCount; i < oldCount + count; ++i) {
      headerItem->values[i].append(QWidgetItemData(Qt::DisplayRole, QString::number(i + 1)));
      headerItem->d->display.append(QString::number(i + 1));
   }

   QStack<QTreeWidgetItem *> itemstack;
   itemstack.push(nullptr);

   while (!itemstack.isEmpty()) {
      QTreeWidgetItem *par = itemstack.pop();
      QList<QTreeWidgetItem *> children = par ? par->children : rootItem->children;
      for (int row = 0; row < children.count(); ++row) {
         QTreeWidgetItem *child = children.at(row);
         if (child->children.count()) {
            itemstack.push(child);
         }
         child->values.insert(column, count, QVector<QWidgetItemData>());
      }
   }

   endInsertColumns();
   return true;
}

bool QTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
   if (count < 1 || row < 0 || (row + count) > rowCount(parent)) {
      return false;
   }

   beginRemoveRows(parent, row, row + count - 1);

   bool blockSignal = signalsBlocked();
   blockSignals(true);

   QTreeWidgetItem *itm = item(parent);
   for (int i = row + count - 1; i >= row; --i) {
      QTreeWidgetItem *child = itm ? itm->takeChild(i) : rootItem->children.takeAt(i);
      Q_ASSERT(child);
      child->view = nullptr;
      delete child;
      child = nullptr;
   }
   blockSignals(blockSignal);

   endRemoveRows();
   return true;
}

QVariant QTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (orientation != Qt::Horizontal) {
      return QVariant();
   }

   if (headerItem) {
      return headerItem->data(section, role);
   }
   if (role == Qt::DisplayRole) {
      return QString::number(section + 1);
   }
   return QVariant();
}

bool QTreeModel::setHeaderData(int section, Qt::Orientation orientation,
   const QVariant &value, int role)
{
   if (section < 0 || orientation != Qt::Horizontal || !headerItem || section >= columnCount()) {
      return false;
   }

   headerItem->setData(section, role, value);
   return true;
}

Qt::ItemFlags QTreeModel::flags(const QModelIndex &index) const
{
   if (!index.isValid()) {
      return rootItem->flags();
   }
   QTreeWidgetItem *itm = item(index);
   Q_ASSERT(itm);
   return itm->flags();
}

void QTreeModel::sort(int column, Qt::SortOrder order)
{
   SkipSorting skipSorting(this);
   sortPendingTimer.stop();

   if (column < 0 || column >= columnCount()) {
      return;
   }

   //layoutAboutToBeChanged and layoutChanged will be called by sortChildren
   rootItem->sortChildren(column, order, true);
}

void QTreeModel::ensureSorted(int column, Qt::SortOrder order,
   int start, int end, const QModelIndex &parent)
{
   if (isChanging()) {
      return;
   }

   sortPendingTimer.stop();

   if (column < 0 || column >= columnCount()) {
      return;
   }

   SkipSorting skipSorting(this);

   QTreeWidgetItem *itm = item(parent);
   if (!itm) {
      itm = rootItem;
   }
   QList<QTreeWidgetItem *> lst = itm->children;

   int count = end - start + 1;
   QVector < QPair<QTreeWidgetItem *, int>> sorting(count);
   for (int i = 0; i < count; ++i) {
      sorting[i].first = lst.at(start + i);
      sorting[i].second = start + i;
   }

   LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
   std::stable_sort(sorting.begin(), sorting.end(), compare);

   QModelIndexList oldPersistentIndexes;
   QModelIndexList newPersistentIndexes;
   QList<QTreeWidgetItem *>::iterator lit = lst.begin();
   bool changed = false;

   for (int i = 0; i < count; ++i) {
      int oldRow = sorting.at(i).second;
      int tmpitepos = lit - lst.begin();
      QTreeWidgetItem *item = lst.takeAt(oldRow);
      if (tmpitepos > lst.size()) {
         --tmpitepos;
      }
      lit = lst.begin() + tmpitepos;

      lit = sortedInsertionIterator(lit, lst.end(), order, item);
      int newRow = qMax(lit - lst.begin(), 0);

      if ((newRow < oldRow) && !(*item < *lst.at(oldRow - 1)) && !(*lst.at(oldRow - 1) < *item )) {
         newRow = oldRow;
      }

      lit = lst.insert(lit, item);
      if (newRow != oldRow) {
         // we are going to change the persistent indexes, so we need to prepare
         if (!changed) { // this will only happen once
            changed = true;
            emit layoutAboutToBeChanged(); // the selection model needs to know
            oldPersistentIndexes = persistentIndexList();
            newPersistentIndexes = oldPersistentIndexes;
         }
         for (int j = i + 1; j < count; ++j) {
            int otherRow = sorting.at(j).second;
            if (oldRow < otherRow && newRow >= otherRow) {
               --sorting[j].second;
            } else if (oldRow > otherRow && newRow <= otherRow) {
               ++sorting[j].second;
            }
         }

         for (int k = 0; k < newPersistentIndexes.count(); ++k) {
            QModelIndex pi = newPersistentIndexes.at(k);
            if (pi.parent() != parent) {
               continue;
            }

            int oldPersistentRow = pi.row();
            int newPersistentRow = oldPersistentRow;

            if (oldPersistentRow == oldRow) {
               newPersistentRow = newRow;
            } else if (oldRow < oldPersistentRow && newRow >= oldPersistentRow) {
               newPersistentRow = oldPersistentRow - 1;
            } else if (oldRow > oldPersistentRow && newRow <= oldPersistentRow) {
               newPersistentRow = oldPersistentRow + 1;
            }

            if (newPersistentRow != oldPersistentRow)
               newPersistentIndexes[k] = createIndex(newPersistentRow,
                     pi.column(), pi.internalPointer());
         }
      }
   }

   if (changed) {
      itm->children = lst;
      changePersistentIndexList(oldPersistentIndexes, newPersistentIndexes);
      emit layoutChanged();
   }
}

bool QTreeModel::itemLessThan(const QPair<QTreeWidgetItem *, int> &left,
   const QPair<QTreeWidgetItem *, int> &right)
{
   return *(left.first) < *(right.first);
}

bool QTreeModel::itemGreaterThan(const QPair<QTreeWidgetItem *, int> &left,
   const QPair<QTreeWidgetItem *, int> &right)
{
   return *(right.first) < *(left.first);
}

QList<QTreeWidgetItem *>::iterator QTreeModel::sortedInsertionIterator(
   const QList<QTreeWidgetItem *>::iterator &begin,
   const QList<QTreeWidgetItem *>::iterator &end,
   Qt::SortOrder order, QTreeWidgetItem *item)
{
   if (order == Qt::AscendingOrder) {
      return std::lower_bound(begin, end, item, QTreeModelLessThan());
   }

   return std::lower_bound(begin, end, item, QTreeModelGreaterThan());
}

QStringList QTreeModel::mimeTypes() const
{
   return view()->mimeTypes();
}

QMimeData *QTreeModel::internalMimeData()  const
{
   return QAbstractItemModel::mimeData(cachedIndexes);
}

QMimeData *QTreeModel::mimeData(const QModelIndexList &indexes) const
{
   QList<QTreeWidgetItem *> items;
   for (int i = 0; i < indexes.count(); ++i) {
      if (indexes.at(i).column() == 0) {
         // only one item per row
         items << item(indexes.at(i));
      }
   }

   // cachedIndexes is a little hack to avoid copying from QModelIndexList to
   // QList<QTreeWidgetItem*> and back again in the view
   cachedIndexes = indexes;
   QMimeData *mimeData = view()->mimeData(items);
   cachedIndexes.clear();

   return mimeData;
}

bool QTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
   int row, int column, const QModelIndex &parent)
{
   if (row == -1 && column == -1) {
      row = rowCount(parent);   // append
   }

   return view()->dropMimeData(item(parent), row, data, action);
}

Qt::DropActions QTreeModel::supportedDropActions() const
{
   return view()->supportedDropActions();
}

void QTreeModel::itemChanged(QTreeWidgetItem *item)
{
   SkipSorting skipSorting(this);       // not optimal, done for performance
   QModelIndex left  = index(item, 0);
   QModelIndex right = index(item, item->columnCount() - 1);

   emit dataChanged(left, right);
}

bool QTreeModel::isChanging() const
{
   Q_D(const QTreeModel);
   return ! d->changes.isEmpty();
}

void QTreeModel::emitDataChanged(QTreeWidgetItem *item, int column)
{
   if (signalsBlocked()) {
      return;
   }

   if (headerItem == item && column < item->columnCount()) {
      if (column == -1) {
         emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
      } else {
         emit headerDataChanged(Qt::Horizontal, column, column);
      }

      return;
   }

   SkipSorting skipSorting(this); //This is a little bit wrong, but not doing it would kill performence

   QModelIndex bottomRight, topLeft;
   if (column == -1) {
      topLeft = index(item, 0);
      bottomRight = createIndex(topLeft.row(), columnCount() - 1, item);
   } else {
      topLeft = index(item, column);
      bottomRight = topLeft;
   }
   emit dataChanged(topLeft, bottomRight);
}

void QTreeModel::beginInsertItems(QTreeWidgetItem *parent, int row, int count)
{
   QModelIndex par = index(parent, 0);
   beginInsertRows(par, row, row + count - 1);
}

void QTreeModel::endInsertItems()
{
   endInsertRows();
}

void QTreeModel::beginRemoveItems(QTreeWidgetItem *parent, int row, int count)
{
   Q_ASSERT(row >= 0);
   Q_ASSERT(count > 0);
   beginRemoveRows(index(parent, 0), row, row + count - 1);
   if (!parent) {
      parent = rootItem;
   }
   // now update the iterators
   for (int i = 0; i < iterators.count(); ++i) {
      for (int j = 0; j < count; j++) {
         QTreeWidgetItem *c = parent->child(row + j);
         iterators[i]->d_func()->ensureValidIterator(c);
      }
   }
}

void QTreeModel::endRemoveItems()
{
   endRemoveRows();
}

void QTreeModel::sortItems(QList<QTreeWidgetItem *> *items, int column, Qt::SortOrder order)
{
   // see QTreeViewItem::operator<
   (void) column;

   if (isChanging()) {
      return;
   }

   // store the original order of indexes
   QVector< QPair<QTreeWidgetItem *, int>> sorting(items->count());
   for (int i = 0; i < sorting.count(); ++i) {
      sorting[i].first = items->at(i);
      sorting[i].second = i;
   }

   // do the sorting
   LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
   std::stable_sort(sorting.begin(), sorting.end(), compare);

   QModelIndexList fromList;
   QModelIndexList toList;
   int colCount = columnCount();

   for (int r = 0; r < sorting.count(); ++r) {
      int oldRow = sorting.at(r).second;

      if (oldRow == r) {
         continue;
      }

      QTreeWidgetItem *item = sorting.at(r).first;
      items->replace(r, item);

      for (int c = 0; c < colCount; ++c) {
         QModelIndex from = createIndex(oldRow, c, item);

         if (static_cast<QAbstractItemModelPrivate *>(d_ptr.data())->persistent.m_indexes.contains(from)) {
            QModelIndex to = createIndex(r, c, item);
            fromList << from;
            toList << to;
         }
      }
   }

   changePersistentIndexList(fromList, toList);
}

void QTreeModel::timerEvent(QTimerEvent *ev)
{
   if (ev->timerId() == sortPendingTimer.timerId()) {
      executePendingSort();
   } else {
      QAbstractItemModel::timerEvent(ev);
   }
}

QTreeWidgetItem::QTreeWidgetItem(int type)
   : rtti(type), view(nullptr), d(new QTreeWidgetItemPrivate(this)), par(nullptr),
     itemFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled
         | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled)
{
}

QTreeWidgetItem::QTreeWidgetItem(const QStringList &strings, int type)
   : rtti(type), view(nullptr), d(new QTreeWidgetItemPrivate(this)), par(nullptr),
     itemFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled
        | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled)
{
   for (int i = 0; i < strings.count(); ++i) {
      setText(i, strings.at(i));
   }
}

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view, int type)
   : rtti(type), view(nullptr), d(new QTreeWidgetItemPrivate(this)), par(nullptr),
     itemFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled
        | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled)
{
   if (view && view->model()) {
      QTreeModel *model = qobject_cast<QTreeModel *>(view->model());
      model->rootItem->addChild(this);
      values.reserve(model->headerItem->columnCount());
   }
}

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view, const QStringList &strings, int type)
   : rtti(type), view(nullptr), d(new QTreeWidgetItemPrivate(this)), par(nullptr),
     itemFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled
        | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled)
{
   for (int i = 0; i < strings.count(); ++i) {
      setText(i, strings.at(i));
   }
   if (view && view->model()) {
      QTreeModel *model = qobject_cast<QTreeModel *>(view->model());
      model->rootItem->addChild(this);
      values.reserve(model->headerItem->columnCount());
   }
}

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view, QTreeWidgetItem *after, int type)
   : rtti(type), view(nullptr), d(new QTreeWidgetItemPrivate(this)), par(nullptr),
     itemFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled
        | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled)
{
   if (view) {
      QTreeModel *model = qobject_cast<QTreeModel *>(view->model());
      if (model) {
         int i = model->rootItem->children.indexOf(after) + 1;
         model->rootItem->insertChild(i, this);
         values.reserve(model->headerItem->columnCount());
      }
   }
}

QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, int type)
   : rtti(type), view(nullptr), d(new QTreeWidgetItemPrivate(this)), par(nullptr),
     itemFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled
        | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled)
{
   if (parent) {
      parent->addChild(this);
   }
}

QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, const QStringList &strings, int type)
   : rtti(type), view(nullptr), d(new QTreeWidgetItemPrivate(this)), par(nullptr),
     itemFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled
        | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled)
{
   for (int i = 0; i < strings.count(); ++i) {
      setText(i, strings.at(i));
   }

   if (parent) {
      parent->addChild(this);
   }
}

QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, int type)
   : rtti(type), view(nullptr), d(new QTreeWidgetItemPrivate(this)), par(nullptr),
     itemFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled
        | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled)
{
   if (parent) {
      int i = parent->children.indexOf(after) + 1;
      parent->insertChild(i, this);
   }
}

QTreeWidgetItem::~QTreeWidgetItem()
{
   QTreeModel *model = (view ? qobject_cast<QTreeModel *>(view->model()) : nullptr);
   bool wasSkipSort = false;

   if (model) {
      wasSkipSort = model->skipPendingSort;
      model->skipPendingSort = true;
   }

   if (par) {
      int i = par->children.indexOf(this);

      if (i >= 0) {
         if (model) {
            model->beginRemoveItems(par, i, 1);
         }

         // users _could_ do changes when connected to rowsAboutToBeRemoved,
         // so we check again to make sure 'i' is valid
         if (!par->children.isEmpty() && par->children.at(i) == this) {
            par->children.takeAt(i);
         }

         if (model) {
            model->endRemoveItems();
         }
      }

   } else if (model) {
      if (this == model->headerItem) {
         model->headerItem = nullptr;

      } else {
         int i = model->rootItem->children.indexOf(this);

         if (i >= 0) {
            model->beginRemoveItems(nullptr, i, 1);
            // users _could_ do changes when connected to rowsAboutToBeRemoved,
            // so we check again to make sure 'i' is valid
            if (!model->rootItem->children.isEmpty() && model->rootItem->children.at(i) == this) {
               model->rootItem->children.takeAt(i);
            }
            model->endRemoveItems();
         }
      }
   }

   // at this point the persistent indexes for the children should also be invalidated
   // since we invalidated the parent

   for (int i = 0; i < children.count(); ++i) {
      QTreeWidgetItem *child = children.at(i);

      // make sure the child does not try to remove itself from our children list
      child->par = nullptr;

      // make sure the child does not try to remove itself from the top level list
      child->view = nullptr;
      delete child;
   }

   children.clear();
   delete d;

   if (model) {
      model->skipPendingSort = wasSkipSort;
   }
}

QTreeWidgetItem *QTreeWidgetItem::clone() const
{
   QTreeWidgetItem *copy = nullptr;

   QStack<const QTreeWidgetItem *> stack;
   QStack<QTreeWidgetItem *> parentStack;
   stack.push(this);
   parentStack.push(nullptr);

   QTreeWidgetItem *root       = nullptr;
   const QTreeWidgetItem *item = nullptr;
   QTreeWidgetItem *parent     = nullptr;

   while (!stack.isEmpty()) {
      // get current item, and copied parent
      item = stack.pop();
      parent = parentStack.pop();

      // copy item
      copy = new QTreeWidgetItem(*item);
      if (!root) {
         root = copy;
      }

      // set parent and add to parents children list
      if (parent) {
         copy->par = parent;
         parent->children.insert(0, copy);
      }

      for (int i = 0; i < item->childCount(); ++i) {
         stack.push(item->child(i));
         parentStack.push(copy);
      }
   }
   return root;
}

void QTreeWidgetItem::setChildIndicatorPolicy(QTreeWidgetItem::ChildIndicatorPolicy policy)
{
   if (d->policy == policy) {
      return;
   }

   d->policy = policy;

   if (!view) {
      return;
   }

   view->scheduleDelayedItemsLayout();
}

QTreeWidgetItem::ChildIndicatorPolicy QTreeWidgetItem::childIndicatorPolicy() const
{
   return d->policy;
}

void QTreeWidgetItem::setFlags(Qt::ItemFlags flags)
{
   const bool enable          = (flags & Qt::ItemIsEnabled);
   const bool changedState    = bool(itemFlags & Qt::ItemIsEnabled) != enable;
   const bool changedExplicit = d->disabled != !enable;

   d->disabled = ! enable;

   if (enable && par && ! (par->itemFlags & Qt::ItemIsEnabled)) {
      // inherit from parent
      itemFlags = flags & ~Qt::ItemIsEnabled;

   } else {
      // this item is explicitly disabled or has no parent
      itemFlags = flags;
   }

   if (changedState && changedExplicit) {
      // propagate the change to the children

      QStack<QTreeWidgetItem *> parents;
      parents.push(this);

      while (! parents.isEmpty()) {
         QTreeWidgetItem *parent = parents.pop();

         for (int i = 0; i < parent->children.count(); ++i) {
            QTreeWidgetItem *child = parent->children.at(i);

            if (!child->d->disabled) {
               // if not explicitly disabled
               parents.push(child);

               if (enable) {
                  child->itemFlags = child->itemFlags | Qt::ItemIsEnabled;
               } else {
                  child->itemFlags = child->itemFlags & ~Qt::ItemIsEnabled;
               }

               child->itemChanged();    // ### we may want to optimize this
            }
         }
      }
   }

   itemChanged();
}

void QTreeWidgetItemPrivate::propagateDisabled(QTreeWidgetItem *item)
{
   Q_ASSERT(item);
   const bool enable = item->par ? (item->par->itemFlags.testFlag(Qt::ItemIsEnabled)) : true;

   QStack<QTreeWidgetItem *> parents;
   parents.push(item);

   while (!parents.isEmpty()) {
      QTreeWidgetItem *parent = parents.pop();

      if (! parent->d->disabled) {
         // if not explicitly disabled
         Qt::ItemFlags oldFlags = parent->itemFlags;

         if (enable) {
            parent->itemFlags = parent->itemFlags | Qt::ItemIsEnabled;
         } else {
            parent->itemFlags = parent->itemFlags & ~Qt::ItemIsEnabled;
         }

         if (parent->itemFlags != oldFlags) {
            parent->itemChanged();
         }
      }

      for (int i = 0; i < parent->children.count(); ++i) {
         QTreeWidgetItem *child = parent->children.at(i);
         parents.push(child);
      }
   }
}

Qt::ItemFlags QTreeWidgetItem::flags() const
{
   return itemFlags;
}

void QTreeWidgetItem::setData(int column, int role, const QVariant &value)
{
   if (column < 0) {
      return;
   }

   QTreeModel *model = (view ? qobject_cast<QTreeModel *>(view->model()) : nullptr);
   switch (role) {
      case Qt::EditRole:
      case Qt::DisplayRole: {
         if (values.count() <= column) {
            if (model && this == model->headerItem) {
               model->setColumnCount(column + 1);
            } else {
               values.resize(column + 1);
            }
         }
         if (d->display.count() <= column) {
            for (int i = d->display.count() - 1; i < column - 1; ++i) {
               d->display.append(QVariant());
            }
            d->display.append(value);
         } else if (d->display[column] != value) {
            d->display[column] = value;
         } else {
            return; // value is unchanged
         }
      }
      break;

      case Qt::CheckStateRole:
         if ((itemFlags & Qt::ItemIsAutoTristate) && value != Qt::PartiallyChecked) {
            for (int i = 0; i < children.count(); ++i) {
               QTreeWidgetItem *child = children.at(i);
               if (child->data(column, role).isValid()) {// has a CheckState
                  Qt::ItemFlags f = itemFlags; // a little hack to avoid multiple dataChanged signals
                  itemFlags &= ~Qt::ItemIsAutoTristate;
                  child->setData(column, role, value);
                  itemFlags = f;
               }
            }
         }
         [[fallthrough]];

      default:
         if (column < values.count()) {
            bool found = false;
            QVector<QWidgetItemData> column_values = values.at(column);
            for (int i = 0; i < column_values.count(); ++i) {
               if (column_values.at(i).role == role) {
                  if (column_values.at(i).value == value) {
                     return;   // value is unchanged
                  }
                  values[column][i].value = value;
                  found = true;
                  break;
               }
            }

            if (!found) {
               values[column].append(QWidgetItemData(role, value));
            }

         } else {
            if (model && this == model->headerItem) {
               model->setColumnCount(column + 1);
            } else {
               values.resize(column + 1);
            }
            values[column].append(QWidgetItemData(role, value));
         }
   }

   if (model) {
      model->emitDataChanged(this, column);

      if (role == Qt::CheckStateRole) {
         QTreeWidgetItem *p;
         for (p = par; p && (p->itemFlags & Qt::ItemIsAutoTristate); p = p->par) {
            model->emitDataChanged(p, column);
         }
      }
   }
}

QVariant QTreeWidgetItem::data(int column, int role) const
{
   switch (role) {
      case Qt::EditRole:
      case Qt::DisplayRole:
         if (column >= 0 && column < d->display.count()) {
            return d->display.at(column);
         }
         break;

      case Qt::CheckStateRole:
         // special case for check state in tristate
         if (children.count() && (itemFlags & Qt::ItemIsAutoTristate)) {
            return childrenCheckState(column);
         }
         [[fallthrough]];

      default:
         if (column >= 0 && column < values.size()) {
            const QVector<QWidgetItemData> &column_values = values.at(column);

            for (int i = 0; i < column_values.count(); ++i)
               if (column_values.at(i).role == role) {
                  return column_values.at(i).value;
               }
         }
   }

   return QVariant();
}

bool QTreeWidgetItem::operator<(const QTreeWidgetItem &other) const
{
   int column = view ? view->sortColumn() : 0;
   const QVariant v1 = data(column, Qt::DisplayRole);
   const QVariant v2 = other.data(column, Qt::DisplayRole);
   return QAbstractItemModelPrivate::variantLessThan(v1, v2);
}

void QTreeWidgetItem::read(QDataStream &in)
{
   in >> values >> d->display;
}

void QTreeWidgetItem::write(QDataStream &out) const
{
   out << values << d->display;
}

QTreeWidgetItem::QTreeWidgetItem(const QTreeWidgetItem &other)
   : rtti(Type), values(other.values), view(nullptr), d(new QTreeWidgetItemPrivate(this)),
     par(nullptr),  itemFlags(other.itemFlags)
{
   d->display = other.d->display;
}

QTreeWidgetItem &QTreeWidgetItem::operator=(const QTreeWidgetItem &other)
{
   values = other.values;
   d->display = other.d->display;
   d->policy = other.d->policy;
   itemFlags = other.itemFlags;

   return *this;
}

void QTreeWidgetItem::addChild(QTreeWidgetItem *child)
{
   if (child) {
      insertChild(children.count(), child);
      child->d->rowGuess = children.count() - 1;
   }
}

void QTreeWidgetItem::insertChild(int index, QTreeWidgetItem *child)
{
   if (index < 0 || index > children.count() || child == nullptr || child->view != nullptr || child->par != nullptr) {
      return;
   }

   if (QTreeModel *model = (view ? qobject_cast<QTreeModel *>(view->model()) : nullptr)) {
      const bool wasSkipSort = model->skipPendingSort;
      model->skipPendingSort = true;

      if (model->rootItem == this) {
         child->par = nullptr;
      } else {
         child->par = this;
      }

      if (view->isSortingEnabled()) {
         // do a delayed sort instead
         if (!model->sortPendingTimer.isActive()) {
            model->sortPendingTimer.start(0, model);
         }
      }

      model->beginInsertItems(this, index, 1);
      int cols = model->columnCount();
      QStack<QTreeWidgetItem *> stack;
      stack.push(child);

      while (!stack.isEmpty()) {
         QTreeWidgetItem *i = stack.pop();
         i->view = view;
         i->values.reserve(cols);

         for (int c = 0; c < i->children.count(); ++c) {
            stack.push(i->children.at(c));
         }
      }

      children.insert(index, child);
      model->endInsertItems();
      model->skipPendingSort = wasSkipSort;

   } else {
      child->par = this;
      children.insert(index, child);
   }

   if (child->par) {
      d->propagateDisabled(child);
   }
}

void QTreeWidgetItem::removeChild(QTreeWidgetItem *child)
{
   (void)takeChild(children.indexOf(child));
}

QTreeWidgetItem *QTreeWidgetItem::takeChild(int index)
{
   // we move this outside the check of the index to allow executing
   // pending sorts from inline functions, using this function (hack)
   QTreeModel *model = (view ? qobject_cast<QTreeModel *>(view->model()) : nullptr);

   if (model) {
      // This will trigger a layoutChanged signal, thus we might want to optimize
      // this function by not emitting the rowsRemoved signal etc to the view.
      // On the other hand we also need to make sure that the selectionmodel
      // is updated in case we take an item that is selected.
      model->skipPendingSort = false;
      model->executePendingSort();
   }
   if (index >= 0 && index < children.count()) {
      if (model) {
         model->beginRemoveItems(this, index, 1);
      }

      QTreeWidgetItem *item = children.takeAt(index);
      item->par = nullptr;

      QStack<QTreeWidgetItem *> stack;
      stack.push(item);
      while (!stack.isEmpty()) {
         QTreeWidgetItem *i = stack.pop();
         i->view = nullptr;
         for (int c = 0; c < i->children.count(); ++c) {
            stack.push(i->children.at(c));
         }
      }
      d->propagateDisabled(item);
      if (model) {
         model->endRemoveRows();
      }
      return item;
   }

   return nullptr;
}

void QTreeWidgetItem::addChildren(const QList<QTreeWidgetItem *> &children)
{
   insertChildren(this->children.count(), children);
}

void QTreeWidgetItem::insertChildren(int index, const QList<QTreeWidgetItem *> &children)
{
   if (view && view->isSortingEnabled()) {
      for (int n = 0; n < children.count(); ++n) {
         insertChild(index, children.at(n));
      }
      return;
   }

   QTreeModel *model = (view ? qobject_cast<QTreeModel *>(view->model()) : nullptr);
   QStack<QTreeWidgetItem *> stack;
   QList<QTreeWidgetItem *> itemsToInsert;

   for (int n = 0; n < children.count(); ++n) {
      QTreeWidgetItem *child = children.at(n);
      if (child->view || child->par) {
         continue;
      }
      itemsToInsert.append(child);
      if (view && model) {
         if (child->childCount() == 0) {
            child->view = view;
         } else {
            stack.push(child);
         }
      }
      if (model && (model->rootItem == this)) {
         child->par = nullptr;
      } else {
         child->par = this;
      }
   }
   if (!itemsToInsert.isEmpty()) {
      while (!stack.isEmpty()) {
         QTreeWidgetItem *i = stack.pop();
         i->view = view;
         for (int c = 0; c < i->children.count(); ++c) {
            stack.push(i->children.at(c));
         }
      }
      if (model) {
         model->beginInsertItems(this, index, itemsToInsert.count());
      }
      for (int n = 0; n < itemsToInsert.count(); ++n) {
         QTreeWidgetItem *child = itemsToInsert.at(n);
         this->children.insert(index + n, child);
         if (child->par) {
            d->propagateDisabled(child);
         }
      }
      if (model) {
         model->endInsertItems();
      }
   }
}

QList<QTreeWidgetItem *> QTreeWidgetItem::takeChildren()
{
   QList<QTreeWidgetItem *> removed;

   if (children.count() > 0) {
      QTreeModel *model = (view ? qobject_cast<QTreeModel *>(view->model()) : nullptr);
      if (model) {
         // This will trigger a layoutChanged signal, thus we might want to optimize
         // this function by not emitting the rowsRemoved signal etc to the view.
         // On the other hand we also need to make sure that the selectionmodel
         // is updated in case we take an item that is selected.
         model->executePendingSort();
      }

      if (model) {
         model->beginRemoveItems(this, 0, children.count());
      }

      for (int n = 0; n < children.count(); ++n) {
         QTreeWidgetItem *item = children.at(n);
         item->par = nullptr;
         QStack<QTreeWidgetItem *> stack;
         stack.push(item);

         while (!stack.isEmpty()) {
            QTreeWidgetItem *i = stack.pop();
            i->view = nullptr;
            for (int c = 0; c < i->children.count(); ++c) {
               stack.push(i->children.at(c));
            }
         }
         d->propagateDisabled(item);
      }
      removed = children;
      children.clear(); // detach
      if (model) {
         model->endRemoveItems();
      }
   }
   return removed;
}


void QTreeWidgetItemPrivate::sortChildren(int column, Qt::SortOrder order, bool climb)
{
   QTreeModel *model = (q->view ? qobject_cast<QTreeModel *>(q->view->model()) : nullptr);
   if (! model) {
      return;
   }
   model->sortItems(&q->children, column, order);
   if (climb) {
      QList<QTreeWidgetItem *>::iterator it = q->children.begin();
      for (; it != q->children.end(); ++it) {
         //here we call the private object's method to avoid emitting
         //the layoutAboutToBeChanged and layoutChanged signals
         (*it)->d->sortChildren(column, order, climb);
      }
   }
}

void QTreeWidgetItem::sortChildren(int column, Qt::SortOrder order, bool climb)
{
   QTreeModel *model = (view ? qobject_cast<QTreeModel *>(view->model()) : nullptr);
   if (! model) {
      return;
   }
   if (model->isChanging()) {
      return;
   }
   QTreeModel::SkipSorting skipSorting(model);
   int oldSortColumn = view->d_func()->explicitSortColumn;
   view->d_func()->explicitSortColumn = column;
   emit model->layoutAboutToBeChanged();
   d->sortChildren(column, order, climb);
   emit model->layoutChanged();
   view->d_func()->explicitSortColumn = oldSortColumn;
}

QVariant QTreeWidgetItem::childrenCheckState(int column) const
{
   if (column < 0) {
      return QVariant();
   }
   bool checkedChildren = false;
   bool uncheckedChildren = false;
   for (int i = 0; i < children.count(); ++i) {
      QVariant value = children.at(i)->data(column, Qt::CheckStateRole);
      if (!value.isValid()) {
         return QVariant();
      }

      switch (static_cast<Qt::CheckState>(value.toInt())) {
         case Qt::Unchecked:
            uncheckedChildren = true;
            break;
         case Qt::Checked:
            checkedChildren = true;
            break;
         case Qt::PartiallyChecked:
         default:
            return Qt::PartiallyChecked;
      }
   }

   if (uncheckedChildren && checkedChildren) {
      return Qt::PartiallyChecked;
   }

   if (uncheckedChildren) {
      return Qt::Unchecked;
   } else if (checkedChildren) {
      return Qt::Checked;
   } else {
      return QVariant();   // value was not defined
   }
}

void QTreeWidgetItem::emitDataChanged()
{
   itemChanged();
}

void QTreeWidgetItem::itemChanged()
{
   if (QTreeModel *model = (view ? qobject_cast<QTreeModel *>(view->model()) : nullptr)) {
      model->itemChanged(this);
   }
}

void QTreeWidgetItem::executePendingSort() const
{
   if (QTreeModel *model = (view ? qobject_cast<QTreeModel *>(view->model()) : nullptr)) {
      model->executePendingSort();
   }
}

QDataStream &operator<<(QDataStream &out, const QTreeWidgetItem &item)
{
   item.write(out);
   return out;
}

QDataStream &operator>>(QDataStream &in, QTreeWidgetItem &item)
{
   item.read(in);
   return in;
}

void QTreeWidgetPrivate::_q_emitItemPressed(const QModelIndex &index)
{
   Q_Q(QTreeWidget);
   emit q->itemPressed(item(index), index.column());
}

void QTreeWidgetPrivate::_q_emitItemClicked(const QModelIndex &index)
{
   Q_Q(QTreeWidget);
   emit q->itemClicked(item(index), index.column());
}

void QTreeWidgetPrivate::_q_emitItemDoubleClicked(const QModelIndex &index)
{
   Q_Q(QTreeWidget);
   emit q->itemDoubleClicked(item(index), index.column());
}

void QTreeWidgetPrivate::_q_emitItemActivated(const QModelIndex &index)
{
   Q_Q(QTreeWidget);
   emit q->itemActivated(item(index), index.column());
}

void QTreeWidgetPrivate::_q_emitItemEntered(const QModelIndex &index)
{
   Q_Q(QTreeWidget);
   emit q->itemEntered(item(index), index.column());
}

void QTreeWidgetPrivate::_q_emitItemChanged(const QModelIndex &index)
{
   Q_Q(QTreeWidget);
   QTreeWidgetItem *indexItem = item(index);
   if (indexItem) {
      emit q->itemChanged(indexItem, index.column());
   }
}

void QTreeWidgetPrivate::_q_emitItemExpanded(const QModelIndex &index)
{
   Q_Q(QTreeWidget);
   emit q->itemExpanded(item(index));
}

void QTreeWidgetPrivate::_q_emitItemCollapsed(const QModelIndex &index)
{
   Q_Q(QTreeWidget);
   emit q->itemCollapsed(item(index));
}

void QTreeWidgetPrivate::_q_emitCurrentItemChanged(const QModelIndex &current,
   const QModelIndex &previous)
{
   Q_Q(QTreeWidget);
   QTreeWidgetItem *currentItem = item(current);
   QTreeWidgetItem *previousItem = item(previous);
   emit q->currentItemChanged(currentItem, previousItem);
}

void QTreeWidgetPrivate::_q_sort()
{
   if (sortingEnabled) {
      int column = header->sortIndicatorSection();
      Qt::SortOrder order = header->sortIndicatorOrder();
      treeModel()->sort(column, order);
   }
}

void QTreeWidgetPrivate::_q_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
   Q_Q(QTreeWidget);
   QModelIndexList indices = selected.indexes();
   int i;
   QTreeModel *m = treeModel();
   for (i = 0; i < indices.count(); ++i) {
      QTreeWidgetItem *item = m->item(indices.at(i));
      item->d->selected = true;
   }

   indices = deselected.indexes();
   for (i = 0; i < indices.count(); ++i) {
      QTreeWidgetItem *item = m->item(indices.at(i));
      item->d->selected = false;
   }

   emit q->itemSelectionChanged();
}

void QTreeWidgetPrivate::_q_dataChanged(const QModelIndex &topLeft,
   const QModelIndex &bottomRight)
{
   if (sortingEnabled && topLeft.isValid() && bottomRight.isValid()
      && !treeModel()->sortPendingTimer.isActive()) {
      int column = header->sortIndicatorSection();
      if (column >= topLeft.column() && column <= bottomRight.column()) {
         Qt::SortOrder order = header->sortIndicatorOrder();
         treeModel()->ensureSorted(column, order, topLeft.row(),
            bottomRight.row(), topLeft.parent());
      }
   }
}

QTreeWidget::QTreeWidget(QWidget *parent)
   : QTreeView(*new QTreeWidgetPrivate(), parent)
{
   QTreeView::setModel(new QTreeModel(1, this));

   connect(this, &QTreeWidget::pressed,             this, &QTreeWidget::_q_emitItemPressed);
   connect(this, &QTreeWidget::clicked,             this, &QTreeWidget::_q_emitItemClicked);
   connect(this, &QTreeWidget::doubleClicked,       this, &QTreeWidget::_q_emitItemDoubleClicked);
   connect(this, &QTreeWidget::activated,           this, &QTreeWidget::_q_emitItemActivated);
   connect(this, &QTreeWidget::entered,             this, &QTreeWidget::_q_emitItemEntered);
   connect(this, &QTreeWidget::expanded,            this, &QTreeWidget::_q_emitItemExpanded);
   connect(this, &QTreeWidget::collapsed,           this, &QTreeWidget::_q_emitItemCollapsed);

   connect(selectionModel(), &QItemSelectionModel::currentChanged,   this, &QTreeWidget::_q_emitCurrentItemChanged);
   connect(model(), &QAbstractItemModel::dataChanged,                this, &QTreeWidget::_q_emitItemChanged);
   connect(model(), &QAbstractItemModel::dataChanged,                this, &QTreeWidget::_q_dataChanged);
   connect(model(), &QAbstractItemModel::columnsRemoved,             this, &QTreeWidget::_q_sort);
   connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &QTreeWidget::_q_selectionChanged);

   header()->setSectionsClickable(false);
}

QTreeWidget::~QTreeWidget()
{
}

int QTreeWidget::columnCount() const
{
   Q_D(const QTreeWidget);
   return d->model->columnCount();
}

void QTreeWidget::setColumnCount(int columns)
{
   Q_D(QTreeWidget);
   if (columns < 0) {
      return;
   }
   d->treeModel()->setColumnCount(columns);
}

QTreeWidgetItem *QTreeWidget::invisibleRootItem() const
{
   Q_D(const QTreeWidget);
   return d->treeModel()->rootItem;
}

QTreeWidgetItem *QTreeWidget::topLevelItem(int index) const
{
   Q_D(const QTreeWidget);
   return d->treeModel()->rootItem->child(index);
}

int QTreeWidget::topLevelItemCount() const
{
   Q_D(const QTreeWidget);
   return d->treeModel()->rootItem->childCount();
}

void QTreeWidget::insertTopLevelItem(int index, QTreeWidgetItem *item)
{
   Q_D(QTreeWidget);
   d->treeModel()->rootItem->insertChild(index, item);
}

void QTreeWidget::addTopLevelItem(QTreeWidgetItem *item)
{
   insertTopLevelItem(topLevelItemCount(), item);
}

QTreeWidgetItem *QTreeWidget::takeTopLevelItem(int index)
{
   Q_D(QTreeWidget);
   return d->treeModel()->rootItem->takeChild(index);
}

int QTreeWidget::indexOfTopLevelItem(QTreeWidgetItem *item) const
{
   Q_D(const QTreeWidget);
   d->treeModel()->executePendingSort();
   return d->treeModel()->rootItem->children.indexOf(item);
}

void QTreeWidget::insertTopLevelItems(int index, const QList<QTreeWidgetItem *> &items)
{
   Q_D(QTreeWidget);
   d->treeModel()->rootItem->insertChildren(index, items);
}

void QTreeWidget::addTopLevelItems(const QList<QTreeWidgetItem *> &items)
{
   insertTopLevelItems(topLevelItemCount(), items);
}

QTreeWidgetItem *QTreeWidget::headerItem() const
{
   Q_D(const QTreeWidget);
   return d->treeModel()->headerItem;
}

void QTreeWidget::setHeaderItem(QTreeWidgetItem *item)
{
   Q_D(QTreeWidget);
   if (!item) {
      return;
   }
   item->view = this;

   int oldCount = columnCount();
   if (oldCount < item->columnCount()) {
      d->treeModel()->beginInsertColumns(QModelIndex(), oldCount, item->columnCount());
   } else {
      d->treeModel()->beginRemoveColumns(QModelIndex(), item->columnCount(), oldCount);
   }

   delete d->treeModel()->headerItem;
   d->treeModel()->headerItem = item;

   if (oldCount < item->columnCount()) {
      d->treeModel()->endInsertColumns();
   } else {
      d->treeModel()->endRemoveColumns();
   }

   d->treeModel()->headerDataChanged(Qt::Horizontal, 0, oldCount);
}

void QTreeWidget::setHeaderLabels(const QStringList &labels)
{
   Q_D(QTreeWidget);
   if (columnCount() < labels.count()) {
      setColumnCount(labels.count());
   }
   QTreeWidgetItem *item = d->treeModel()->headerItem;
   for (int i = 0; i < labels.count(); ++i) {
      item->setText(i, labels.at(i));
   }
}

QTreeWidgetItem *QTreeWidget::currentItem() const
{
   Q_D(const QTreeWidget);
   return d->item(currentIndex());
}

int QTreeWidget::currentColumn() const
{
   return currentIndex().column();
}

void QTreeWidget::setCurrentItem(QTreeWidgetItem *item)
{
   setCurrentItem(item, 0);
}

void QTreeWidget::setCurrentItem(QTreeWidgetItem *item, int column)
{
   Q_D(QTreeWidget);
   setCurrentIndex(d->index(item, column));
}

void QTreeWidget::setCurrentItem(QTreeWidgetItem *item, int column,
   QItemSelectionModel::SelectionFlags command)
{
   Q_D(QTreeWidget);
   d->selectionModel->setCurrentIndex(d->index(item, column), command);
}

QTreeWidgetItem *QTreeWidget::itemAt(const QPoint &p) const
{
   Q_D(const QTreeWidget);
   return d->item(indexAt(p));
}

QRect QTreeWidget::visualItemRect(const QTreeWidgetItem *item) const
{
   Q_D(const QTreeWidget);
   //the visual rect for an item is across all columns. So we need to determine
   //what is the first and last column and get their visual index rects

   QModelIndex base = d->index(item);
   const int firstVisiblesection = header()->logicalIndexAt(- header()->offset());
   const int lastVisibleSection = header()->logicalIndexAt(header()->length() - header()->offset() - 1);

   QModelIndex first = base.sibling(base.row(), header()->logicalIndex(firstVisiblesection));
   QModelIndex last  = base.sibling(base.row(), header()->logicalIndex(lastVisibleSection));

   return visualRect(first) | visualRect(last);
}

int QTreeWidget::sortColumn() const
{
   Q_D(const QTreeWidget);

   return (d->explicitSortColumn != -1 ? d->explicitSortColumn : header()->sortIndicatorSection());
}

void QTreeWidget::sortItems(int column, Qt::SortOrder order)
{
   Q_D(QTreeWidget);
   header()->setSortIndicator(column, order);
   d->model->sort(column, order);
}

void QTreeWidget::editItem(QTreeWidgetItem *item, int column)
{
   Q_D(QTreeWidget);
   edit(d->index(item, column));
}

void QTreeWidget::openPersistentEditor(QTreeWidgetItem *item, int column)
{
   Q_D(QTreeWidget);
   QAbstractItemView::openPersistentEditor(d->index(item, column));
}

void QTreeWidget::closePersistentEditor(QTreeWidgetItem *item, int column)
{
   Q_D(QTreeWidget);
   QAbstractItemView::closePersistentEditor(d->index(item, column));
}

QWidget *QTreeWidget::itemWidget(QTreeWidgetItem *item, int column) const
{
   Q_D(const QTreeWidget);
   return QAbstractItemView::indexWidget(d->index(item, column));
}

void QTreeWidget::setItemWidget(QTreeWidgetItem *item, int column, QWidget *widget)
{
   Q_D(QTreeWidget);
   QAbstractItemView::setIndexWidget(d->index(item, column), widget);
}

bool QTreeWidget::isItemSelected(const QTreeWidgetItem *item) const
{
   if (! item) {
      return false;
   }

   return item->d->selected;
}

void QTreeWidget::setItemSelected(const QTreeWidgetItem *item, bool select)
{
   Q_D(QTreeWidget);

   if (! item) {
      return;
   }

   selectionModel()->select(d->index(item), (select ? QItemSelectionModel::Select
         : QItemSelectionModel::Deselect) | QItemSelectionModel::Rows);
   item->d->selected = select;
}

QList<QTreeWidgetItem *> QTreeWidget::selectedItems() const
{
   Q_D(const QTreeWidget);

   QModelIndexList indexes = selectionModel()->selectedIndexes();
   QList<QTreeWidgetItem *> items;

   QSet<QTreeWidgetItem *> seen;
   seen.reserve(indexes.count());

   for (int i = 0; i < indexes.count(); ++i) {
      QTreeWidgetItem *item = d->item(indexes.at(i));

      if (isItemHidden(item) || seen.contains(item)) {
         continue;
      }
      seen.insert(item);
      items.append(item);
   }
   return items;
}

QList<QTreeWidgetItem *> QTreeWidget::findItems(const QString &text, Qt::MatchFlags flags, int column) const
{
   Q_D(const QTreeWidget);
   QModelIndexList indexes = d->model->match(model()->index(0, column, QModelIndex()),
         Qt::DisplayRole, text, -1, flags);
   QList<QTreeWidgetItem *> items;
   for (int i = 0; i < indexes.size(); ++i) {
      items.append(d->item(indexes.at(i)));
   }
   return items;
}

bool QTreeWidget::isItemHidden(const QTreeWidgetItem *item) const
{
   Q_D(const QTreeWidget);
   if (item == d->treeModel()->headerItem) {
      return header()->isHidden();
   }
   if (d->hiddenIndexes.isEmpty()) {
      return false;
   }
   QTreeModel::SkipSorting skipSorting(d->treeModel());
   return d->isRowHidden(d->index(item));
}

void QTreeWidget::setItemHidden(const QTreeWidgetItem *item, bool hide)
{
   Q_D(QTreeWidget);
   if (item == d->treeModel()->headerItem) {
      header()->setHidden(hide);
   } else {
      const QModelIndex index = d->index(item);
      setRowHidden(index.row(), index.parent(), hide);
   }
}

bool QTreeWidget::isItemExpanded(const QTreeWidgetItem *item) const
{
   Q_D(const QTreeWidget);
   QTreeModel::SkipSorting skipSorting(d->treeModel());
   return isExpanded(d->index(item));
}

void QTreeWidget::setItemExpanded(const QTreeWidgetItem *item, bool expand)
{
   Q_D(QTreeWidget);
   QTreeModel::SkipSorting skipSorting(d->treeModel());
   setExpanded(d->index(item), expand);
}

bool QTreeWidget::isFirstItemColumnSpanned(const QTreeWidgetItem *item) const
{
   Q_D(const QTreeWidget);
   if (item == d->treeModel()->headerItem) {
      return false;   // We can not set the header items to spanning
   }

   const QModelIndex index = d->index(item);
   return isFirstColumnSpanned(index.row(), index.parent());
}

void QTreeWidget::setFirstItemColumnSpanned(const QTreeWidgetItem *item, bool span)
{
   Q_D(QTreeWidget);
   if (item == d->treeModel()->headerItem) {
      return;   // We can not set header items to spanning
   }

   const QModelIndex index = d->index(item);
   setFirstColumnSpanned(index.row(), index.parent(), span);
}

QTreeWidgetItem *QTreeWidget::itemAbove(const QTreeWidgetItem *item) const
{
   Q_D(const QTreeWidget);
   if (item == d->treeModel()->headerItem) {
      return nullptr;
   }

   const QModelIndex index = d->index(item);
   const QModelIndex above = indexAbove(index);
   return d->item(above);
}

QTreeWidgetItem *QTreeWidget::itemBelow(const QTreeWidgetItem *item) const
{
   Q_D(const QTreeWidget);
   if (item == d->treeModel()->headerItem) {
      return nullptr;
   }

   const QModelIndex index = d->index(item);
   const QModelIndex below = indexBelow(index);
   return d->item(below);
}

void QTreeWidget::setSelectionModel(QItemSelectionModel *selectionModel)
{
   Q_D(QTreeWidget);
   QTreeView::setSelectionModel(selectionModel);
   QItemSelection newSelection = selectionModel->selection();
   if (!newSelection.isEmpty()) {
      d->_q_selectionChanged(newSelection, QItemSelection());
   }
}

void QTreeWidget::scrollToItem(const QTreeWidgetItem *item, QAbstractItemView::ScrollHint hint)
{
   Q_D(QTreeWidget);
   QTreeView::scrollTo(d->index(item), hint);
}

void QTreeWidget::expandItem(const QTreeWidgetItem *item)
{
   Q_D(QTreeWidget);
   QTreeModel::SkipSorting skipSorting(d->treeModel());
   expand(d->index(item));
}

void QTreeWidget::collapseItem(const QTreeWidgetItem *item)
{
   Q_D(QTreeWidget);
   QTreeModel::SkipSorting skipSorting(d->treeModel());
   collapse(d->index(item));
}

void QTreeWidget::clear()
{
   Q_D(QTreeWidget);
   selectionModel()->clear();
   d->treeModel()->clear();
}

QStringList QTreeWidget::mimeTypes() const
{
   return model()->QAbstractItemModel::mimeTypes();
}

QMimeData *QTreeWidget::mimeData(const QList<QTreeWidgetItem *> &items) const
{
   Q_D(const QTreeWidget);

   if (d->treeModel()->cachedIndexes.isEmpty()) {
      QList<QModelIndex> indexes;

      for (int i = 0; i < items.count(); ++i) {
         QTreeWidgetItem *item = items.at(i);

         if (! item) {
            qWarning("QTreeWidget::mimeData() Item %d was invalid (nullptr)", i);
            return nullptr;
         }

         for (int c = 0; c < item->values.count(); ++c) {
            const QModelIndex index = indexFromItem(item, c);

            if (! index.isValid()) {
               qWarning("QTreeWidget::mimeData() No index associated with item %p at element %d",
                     static_cast<void *>(item), i);
               return nullptr;
            }

            indexes << index;
         }
      }

      return d->model->QAbstractItemModel::mimeData(indexes);
   }

   return d->treeModel()->internalMimeData();
}

bool QTreeWidget::dropMimeData(QTreeWidgetItem *parent, int index,
      const QMimeData *data, Qt::DropAction action)
{
   QModelIndex idx;

   if (parent) {
      idx = indexFromItem(parent);
   }

   return model()->QAbstractItemModel::dropMimeData(data, action, index, 0, idx);
}

Qt::DropActions QTreeWidget::supportedDropActions() const
{
   return model()->QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}

QModelIndex QTreeWidget::indexFromItem(QTreeWidgetItem *item, int column) const
{
   Q_D(const QTreeWidget);
   return d->index(item, column);
}

QTreeWidgetItem *QTreeWidget::itemFromIndex(const QModelIndex &index) const
{
   Q_D(const QTreeWidget);
   return d->item(index);
}

#ifndef QT_NO_DRAGANDDROP

void QTreeWidget::dropEvent(QDropEvent *event)
{
   Q_D(QTreeWidget);

   if (event->source() == this && (event->dropAction() == Qt::MoveAction ||
         dragDropMode() == QAbstractItemView::InternalMove)) {
      QModelIndex topIndex;

      int col = -1;
      int row = -1;

      if (d->dropOn(event, &row, &col, &topIndex)) {
         QList<QModelIndex> idxs = selectedIndexes();
         QList<QPersistentModelIndex> indexes;
         for (int i = 0; i < idxs.count(); i++) {
            indexes.append(idxs.at(i));
         }

         if (indexes.contains(topIndex)) {
            return;
         }

         // When removing items the drop location could shift
         QPersistentModelIndex dropRow = model()->index(row, col, topIndex);

         // Remove the items
         QList<QTreeWidgetItem *> taken;
         for (int i = 0; i < indexes.count(); ++i) {
            QTreeWidgetItem *parent = itemFromIndex(indexes.at(i));

            if (!parent || !parent->parent()) {
               taken.append(takeTopLevelItem(indexes.at(i).row()));
            } else {
               taken.append(parent->parent()->takeChild(indexes.at(i).row()));
            }
         }

         // insert them back in at their new positions
         for (int i = 0; i < indexes.count(); ++i) {
            // Either at a specific point or appended
            if (row == -1) {
               if (topIndex.isValid()) {
                  QTreeWidgetItem *parent = itemFromIndex(topIndex);
                  parent->insertChild(parent->childCount(), taken.takeFirst());
               } else {
                  insertTopLevelItem(topLevelItemCount(), taken.takeFirst());
               }
            } else {
               int r = dropRow.row() >= 0 ? dropRow.row() : row;
               if (topIndex.isValid()) {
                  QTreeWidgetItem *parent = itemFromIndex(topIndex);
                  parent->insertChild(qMin(r, parent->childCount()), taken.takeFirst());
               } else {
                  insertTopLevelItem(qMin(r, topLevelItemCount()), taken.takeFirst());
               }
            }
         }

         event->accept();
         // Don't want QAbstractItemView to delete it because it was "moved" we already did it
         event->setDropAction(Qt::CopyAction);
      }
   }

   QTreeView::dropEvent(event);
}
#endif

void QTreeWidget::setModel(QAbstractItemModel *)
{
   Q_ASSERT(!"QTreeWidget::setModel() - Changing the model of the QTreeWidget is not allowed.");
}

bool QTreeWidget::event(QEvent *e)
{
   Q_D(QTreeWidget);
   if (e->type() == QEvent::Polish) {
      d->treeModel()->executePendingSort();
   }
   return QTreeView::event(e);
}

void QTreeWidget::_q_emitItemPressed(const QModelIndex &index)
{
   Q_D(QTreeWidget);
   d->_q_emitItemPressed(index);
}

void QTreeWidget::_q_emitItemClicked(const QModelIndex &index)
{
   Q_D(QTreeWidget);
   d->_q_emitItemClicked(index);
}

void QTreeWidget::_q_emitItemDoubleClicked(const QModelIndex &index)
{
   Q_D(QTreeWidget);
   d->_q_emitItemDoubleClicked(index);
}

void QTreeWidget::_q_emitItemActivated(const QModelIndex &index)
{
   Q_D(QTreeWidget);
   d->_q_emitItemActivated(index);
}

void QTreeWidget::_q_emitItemEntered(const QModelIndex &index)
{
   Q_D(QTreeWidget);
   d->_q_emitItemEntered(index);
}

void QTreeWidget::_q_emitItemChanged(const QModelIndex &index)
{
   Q_D(QTreeWidget);
   d->_q_emitItemChanged(index);
}

void QTreeWidget::_q_emitItemExpanded(const QModelIndex &index)
{
   Q_D(QTreeWidget);
   d->_q_emitItemExpanded(index);
}

void QTreeWidget::_q_emitItemCollapsed(const QModelIndex &index)
{
   Q_D(QTreeWidget);
   d->_q_emitItemCollapsed(index);
}

void QTreeWidget::_q_emitCurrentItemChanged(const QModelIndex &previous, const QModelIndex &current)
{
   Q_D(QTreeWidget);
   d->_q_emitCurrentItemChanged(previous, current);
}

void QTreeWidget::_q_sort()
{
   Q_D(QTreeWidget);
   d->_q_sort();
}

void QTreeWidget::_q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
   Q_D(QTreeWidget);
   d->_q_dataChanged(topLeft, bottomRight);
}

void QTreeWidget::_q_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
   Q_D(QTreeWidget);
   d->_q_selectionChanged(selected, deselected);
}

#endif // QT_NO_TREEWIDGET
