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

#include <qstandarditemmodel.h>

#ifndef QT_NO_STANDARDITEMMODEL

#include <qalgorithms.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qlist.h>
#include <qmap.h>
#include <qpair.h>
#include <qstringlist.h>
#include <qbitarray.h>
#include <qmimedata.h>
#include <qvariant.h>
#include <qvector.h>

#include <qstandarditemmodel_p.h>

#include <algorithm>

class QStandardItemModelLessThan
{
 public:
   QStandardItemModelLessThan() {
   }

   bool operator()(const QPair<QStandardItem *, int> &l, const QPair<QStandardItem *, int> &r) const {
      return *(l.first) < *(r.first);
   }
};

class QStandardItemModelGreaterThan
{
 public:
   QStandardItemModelGreaterThan() {
   }

   bool operator()(const QPair<QStandardItem *, int> &l,
      const QPair<QStandardItem *, int> &r) const {
      return *(r.first) < *(l.first);
   }
};

QStandardItemPrivate::~QStandardItemPrivate()
{
   QVector<QStandardItem *>::const_iterator it;

   for (it = children.constBegin(); it != children.constEnd(); ++it) {
      QStandardItem *child = *it;

      if (model == nullptr) {
         // do nothing

      } else if (child != nullptr && model->d_func() != nullptr) {
         child->d_func()->setModel(nullptr);
      }

      delete child;
   }

   children.clear();

   if (parent != nullptr && model != nullptr && model->d_func() != nullptr) {
      parent->d_func()->childDeleted(q_func());
   }
}

QPair<int, int> QStandardItemPrivate::position() const
{
   if (QStandardItem *par = parent) {
      int idx = par->d_func()->childIndex(q_func());

      if (idx == -1) {
         return QPair<int, int>(-1, -1);
      }

      return QPair<int, int>(idx / par->columnCount(), idx % par->columnCount());
   }

   // ### support header items?
   return QPair<int, int>(-1, -1);
}

void QStandardItemPrivate::setChild(int row, int column, QStandardItem *item,
   bool emitChanged)
{
   Q_Q(QStandardItem);

   if (item == q) {
      qWarning("QStandardItem::setChild() Unable to make an item a child of itself");
      return;
   }

   if ((row < 0) || (column < 0)) {
      return;
   }

   if (rows <= row) {
      q->setRowCount(row + 1);
   }

   if (columns <= column) {
      q->setColumnCount(column + 1);
   }

   int index = childIndex(row, column);
   Q_ASSERT(index != -1);

   QStandardItem *oldItem = children.at(index);
   if (item == oldItem) {
      return;
   }

   if (model && emitChanged) {
      emit model->layoutAboutToBeChanged();
   }

   if (item) {
      if (item->d_func()->parent == nullptr) {
         item->d_func()->setParentAndModel(q, model);
      } else {
         qWarning("QStandardItem::setChild() Ignoring duplicate insertion of item");
         return;
      }
   }

   if (oldItem) {
      oldItem->d_func()->setModel(nullptr);
   }

   delete oldItem;
   children.replace(index, item);
   if (model && emitChanged) {
      emit model->layoutChanged();
   }

   if (emitChanged && model) {
      model->d_func()->itemChanged(item);
   }

}

void QStandardItemPrivate::changeFlags(bool enable, Qt::ItemFlags f)
{
   Q_Q(QStandardItem);
   Qt::ItemFlags flags = q->flags();

   if (enable) {
      flags |= f;
   } else {
      flags &= ~f;
   }

   q->setFlags(flags);
}

void QStandardItemPrivate::childDeleted(QStandardItem *child)
{
   int index = childIndex(child);
   Q_ASSERT(index != -1);
   children.replace(index, nullptr);
}

void QStandardItemPrivate::setItemData(const QMap<int, QVariant> &roles)
{
   Q_Q(QStandardItem);

   // build the vector of new values
   QVector<QStandardItemData> newValues;
   QMap<int, QVariant>::const_iterator it;

   for (it = roles.begin(); it != roles.end(); ++it) {
      QVariant value = it.value();

      if (value.isValid()) {
         int role = it.key();
         role = (role == Qt::EditRole) ? Qt::DisplayRole : role;

         QStandardItemData wid(role, it.value());
         newValues.append(wid);
      }
   }

   if (values != newValues) {
      values = newValues;

      if (model) {
         model->d_func()->itemChanged(q);
      }
   }
}

const QMap<int, QVariant> QStandardItemPrivate::itemData() const
{
   QMap<int, QVariant> result;
   QVector<QStandardItemData>::const_iterator it;

   for (it = values.begin(); it != values.end(); ++it) {
      result.insert((*it).role, (*it).value);
   }
   return result;
}

void QStandardItemPrivate::sortChildren(int column, Qt::SortOrder order)
{
   Q_Q(QStandardItem);

   if (column >= columnCount()) {
      return;
   }

   QVector<QPair<QStandardItem *, int>> sortable;
   QVector<int> unsortable;

   sortable.reserve(rowCount());
   unsortable.reserve(rowCount());

   for (int row = 0; row < rowCount(); ++row) {
      QStandardItem *itm = q->child(row, column);
      if (itm) {
         sortable.append(QPair<QStandardItem *, int>(itm, row));
      } else {
         unsortable.append(row);
      }
   }

   if (order == Qt::AscendingOrder) {
      QStandardItemModelLessThan lt;
      std::stable_sort(sortable.begin(), sortable.end(), lt);
   } else {
      QStandardItemModelGreaterThan gt;
      std::stable_sort(sortable.begin(), sortable.end(), gt);
   }

   QModelIndexList changedPersistentIndexesFrom, changedPersistentIndexesTo;
   QVector<QStandardItem *> sorted_children(children.count());
   for (int i = 0; i < rowCount(); ++i) {
      int r = (i < sortable.count() ? sortable.at(i).second : unsortable.at(i - sortable.count()));

      for (int c = 0; c < columnCount(); ++c) {
         QStandardItem *itm = q->child(r, c);
         sorted_children[childIndex(i, c)] = itm;

         if (model) {
            QModelIndex from = model->createIndex(r, c, q);

            if (model->d_func()->persistent.m_indexes.contains(from)) {
               QModelIndex to = model->createIndex(i, c, q);
               changedPersistentIndexesFrom.append(from);
               changedPersistentIndexesTo.append(to);
            }
         }
      }
   }

   children = sorted_children;

   if (model) {
      model->changePersistentIndexList(changedPersistentIndexesFrom, changedPersistentIndexesTo);
   }

   QVector<QStandardItem *>::iterator it;
   for (it = children.begin(); it != children.end(); ++it) {
      if (*it) {
         (*it)->d_func()->sortChildren(column, order);
      }
   }
}

void QStandardItemPrivate::setModel(QStandardItemModel *newModel)
{
   if (children.isEmpty()) {
      if (model != nullptr) {
         model->d_func()->invalidatePersistentIndex(model->indexFromItem(q_ptr));
      }

      model = newModel;

   } else {
      QStack<QStandardItem *> stack;
      stack.push(q_ptr);

      while (! stack.isEmpty()) {
         QStandardItem *itm = stack.pop();

         if (itm->d_func()->model != nullptr) {
            itm->d_func()->model->d_func()->invalidatePersistentIndex(itm->d_func()->model->indexFromItem(itm));
         }

         itm->d_func()->model = newModel;
         const QVector<QStandardItem *> &childList = itm->d_func()->children;

         for (int i = 0; i < childList.count(); ++i) {
            QStandardItem *chi = childList.at(i);

            if (chi != nullptr) {
               stack.push(chi);
            }
         }
      }
   }
}

QStandardItemModelPrivate::QStandardItemModelPrivate()
   : root(new QStandardItem), itemPrototype(nullptr), sortRole(Qt::DisplayRole)
{
   root->setFlags(Qt::ItemIsDropEnabled);
}

QStandardItemModelPrivate::~QStandardItemModelPrivate()
{
   delete itemPrototype;
   qDeleteAll(columnHeaderItems);
   qDeleteAll(rowHeaderItems);
}

void QStandardItemModelPrivate::init()
{
   Q_Q(QStandardItemModel);

   QObject::connect(q, &QStandardItemModel::dataChanged, q, &QStandardItemModel::_q_emitItemChanged);
}

void QStandardItemModelPrivate::_q_emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
   Q_Q(QStandardItemModel);
   QModelIndex parent = topLeft.parent();

   for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
      for (int column = topLeft.column(); column <= bottomRight.column(); ++column) {
         QModelIndex index = q->index(row, column, parent);

         if (QStandardItem *item = itemFromIndex(index)) {
            emit q->itemChanged(item);
         }
      }
   }
}

bool QStandardItemPrivate::insertRows(int row, const QList<QStandardItem *> &items)
{
   Q_Q(QStandardItem);

   if ((row < 0) || (row > rowCount())) {
      return false;
   }

   int count = items.count();

   if (model != nullptr) {
      model->d_func()->rowsAboutToBeInserted(q, row, row + count - 1);
   }

   if (rowCount() == 0) {
      if (columnCount() == 0) {
         q->setColumnCount(1);
      }

      children.resize(columnCount() * count);
      rows = count;

   } else {
      rows += count;
      int index = childIndex(row, 0);

      if (index != -1) {
         children.insert(index, columnCount() * count, nullptr);
      }
   }

   for (int i = 0; i < items.count(); ++i) {
      QStandardItem *item    = items.at(i);
      item->d_func()->model  = model;
      item->d_func()->parent = q;

      int index = childIndex(i + row, 0);
      children.replace(index, item);
   }

   if (model != nullptr) {
      model->d_func()->rowsInserted(q, row, count);
   }

   return true;
}

bool QStandardItemPrivate::insertRows(int row, int count, const QList<QStandardItem *> &items)
{
   Q_Q(QStandardItem);

   if ((count < 1) || (row < 0) || (row > rowCount())) {
      return false;
   }

   if (model != nullptr) {
      model->d_func()->rowsAboutToBeInserted(q, row, row + count - 1);
   }

   if (rowCount() == 0) {
      children.resize(columnCount() * count);
      rows = count;

   } else {
      rows += count;
      int index = childIndex(row, 0);
      if (index != -1) {
         children.insert(index, columnCount() * count, nullptr);
      }
   }

   if (!items.isEmpty()) {
      int index = childIndex(row, 0);
      int limit = qMin(items.count(), columnCount() * count);

      for (int i = 0; i < limit; ++i) {
         QStandardItem *item = items.at(i);

         if (item) {
            if (item->d_func()->parent == nullptr) {
               item->d_func()->setParentAndModel(q, model);
            } else {
               qWarning("QStandardItem::insertRows() Ignoring duplicate insertion of item");
               item = nullptr;
            }
         }

         children.replace(index, item);
         ++index;
      }
   }

   if (model != nullptr) {
      model->d_func()->rowsInserted(q, row, count);
   }

   return true;
}

bool QStandardItemPrivate::insertColumns(int column, int count, const QList<QStandardItem *> &items)
{
   Q_Q(QStandardItem);

   if ((count < 1) || (column < 0) || (column > columnCount())) {
      return false;
   }

   if (model != nullptr) {
      model->d_func()->columnsAboutToBeInserted(q, column, column + count - 1);
   }

   if (columnCount() == 0) {
      children.resize(rowCount() * count);
      columns = count;

   } else {
      columns += count;
      int index = childIndex(0, column);
      for (int row = 0; row < rowCount(); ++row) {
         children.insert(index, count, nullptr);
         index += columnCount();
      }
   }

   if (! items.isEmpty()) {
      int limit = qMin(items.count(), rowCount() * count);

      for (int i = 0; i < limit; ++i) {
         QStandardItem *item = items.at(i);

         if (item != nullptr) {
            if (item->d_func()->parent == nullptr) {
               item->d_func()->setParentAndModel(q, model);
            } else {
               qWarning("QStandardItem::insertColumns() Ignoring duplicate insertion of item");
               item = nullptr;
            }
         }

         int r = i / count;
         int c = column + (i % count);
         int index = childIndex(r, c);
         children.replace(index, item);
      }
   }

   if (model != nullptr) {
      model->d_func()->columnsInserted(q, column, count);
   }

   return true;
}

void QStandardItemModelPrivate::itemChanged(QStandardItem *item)
{
   Q_Q(QStandardItemModel);

   if (item->d_func()->parent == nullptr) {
      // Header item
      int idx = columnHeaderItems.indexOf(item);

      if (idx != -1) {
         emit q->headerDataChanged(Qt::Horizontal, idx, idx);
      } else {
         idx = rowHeaderItems.indexOf(item);
         if (idx != -1) {
            emit q->headerDataChanged(Qt::Vertical, idx, idx);
         }
      }

   } else {
      // Normal item
      QModelIndex index = q->indexFromItem(item);
      emit q->dataChanged(index, index);
   }
}

void QStandardItemModelPrivate::rowsAboutToBeInserted(QStandardItem *parent,
   int start, int end)
{
   Q_Q(QStandardItemModel);

   QModelIndex index = q->indexFromItem(parent);
   q->beginInsertRows(index, start, end);
}

void QStandardItemModelPrivate::columnsAboutToBeInserted(QStandardItem *parent,
   int start, int end)
{
   Q_Q(QStandardItemModel);
   QModelIndex index = q->indexFromItem(parent);
   q->beginInsertColumns(index, start, end);
}

void QStandardItemModelPrivate::rowsAboutToBeRemoved(QStandardItem *parent,
   int start, int end)
{
   Q_Q(QStandardItemModel);
   QModelIndex index = q->indexFromItem(parent);
   q->beginRemoveRows(index, start, end);
}

void QStandardItemModelPrivate::columnsAboutToBeRemoved(QStandardItem *parent,
   int start, int end)
{
   Q_Q(QStandardItemModel);
   QModelIndex index = q->indexFromItem(parent);
   q->beginRemoveColumns(index, start, end);
}

void QStandardItemModelPrivate::rowsInserted(QStandardItem *parent,
   int row, int count)
{
   Q_Q(QStandardItemModel);

   if (parent == root.data()) {
      rowHeaderItems.insert(row, count, nullptr);
   }
   q->endInsertRows();
}

void QStandardItemModelPrivate::columnsInserted(QStandardItem *parent,
   int column, int count)
{
   Q_Q(QStandardItemModel);

   if (parent == root.data()) {
      columnHeaderItems.insert(column, count, nullptr);
   }

   q->endInsertColumns();
}

void QStandardItemModelPrivate::rowsRemoved(QStandardItem *parent,
   int row, int count)
{
   Q_Q(QStandardItemModel);

   if (parent == root.data()) {
      for (int i = row; i < row + count; ++i) {
         QStandardItem *oldItem = rowHeaderItems.at(i);
         if (oldItem) {
            oldItem->d_func()->setModel(nullptr);
         }

         delete oldItem;
      }

      rowHeaderItems.remove(row, count);
   }
   q->endRemoveRows();
}

void QStandardItemModelPrivate::columnsRemoved(QStandardItem *parent,
   int column, int count)
{
   Q_Q(QStandardItemModel);

   if (parent == root.data()) {
      for (int i = column; i < column + count; ++i) {
         QStandardItem *oldItem = columnHeaderItems.at(i);

         if (oldItem) {
            oldItem->d_func()->setModel(nullptr);
         }
         delete oldItem;
      }

      columnHeaderItems.remove(column, count);
   }
   q->endRemoveColumns();
}

QStandardItem::QStandardItem()
   : d_ptr(new QStandardItemPrivate)
{
   Q_D(QStandardItem);
   d->q_ptr = this;
}

QStandardItem::QStandardItem(const QString &text)
   : d_ptr(new QStandardItemPrivate)
{
   Q_D(QStandardItem);
   d->q_ptr = this;
   setText(text);
}

QStandardItem::QStandardItem(const QIcon &icon, const QString &text)
   : d_ptr(new QStandardItemPrivate)
{
   Q_D(QStandardItem);
   d->q_ptr = this;
   setIcon(icon);
   setText(text);
}

QStandardItem::QStandardItem(int rows, int columns)
   : d_ptr(new QStandardItemPrivate)
{
   Q_D(QStandardItem);
   d->q_ptr = this;
   setRowCount(rows);
   setColumnCount(columns);
}

QStandardItem::QStandardItem(QStandardItemPrivate &dd)
   : d_ptr(&dd)
{
   Q_D(QStandardItem);
   d->q_ptr = this;
}

QStandardItem::QStandardItem(const QStandardItem &other)
   : d_ptr(new QStandardItemPrivate)
{
   Q_D(QStandardItem);
   d->q_ptr = this;
   operator=(other);
}

QStandardItem &QStandardItem::operator=(const QStandardItem &other)
{
   Q_D(QStandardItem);
   d->values = other.d_func()->values;
   return *this;
}

QStandardItem::~QStandardItem()
{
}

QStandardItem *QStandardItem::parent() const
{
   Q_D(const QStandardItem);

   if (! d->model || (d->model->d_func()->root.data() != d->parent)) {
      return d->parent;
   }

   return nullptr;
}

void QStandardItem::setData(const QVariant &value, int role)
{
   Q_D(QStandardItem);
   role = (role == Qt::EditRole) ? Qt::DisplayRole : role;
   QVector<QStandardItemData>::iterator it;

   for (it = d->values.begin(); it != d->values.end(); ++it) {
      if ((*it).role == role) {
         if (value.isValid()) {
            if ((*it).value.type() == value.type() && (*it).value == value) {
               return;
            }
            (*it).value = value;
         } else {
            d->values.erase(it);
         }
         if (d->model) {
            d->model->d_func()->itemChanged(this);
         }
         return;
      }
   }

   d->values.append(QStandardItemData(role, value));

   if (d->model) {
      d->model->d_func()->itemChanged(this);
   }
}

QVariant QStandardItem::data(int role) const
{
   Q_D(const QStandardItem);
   role = (role == Qt::EditRole) ? Qt::DisplayRole : role;
   QVector<QStandardItemData>::const_iterator it;

   for (it = d->values.begin(); it != d->values.end(); ++it) {
      if ((*it).role == role) {
         return (*it).value;
      }
   }

   return QVariant();
}

void QStandardItem::emitDataChanged()
{
   Q_D(QStandardItem);
   if (d->model) {
      d->model->d_func()->itemChanged(this);
   }
}

void QStandardItem::setFlags(Qt::ItemFlags flags)
{
   setData((int)flags, Qt::UserRole - 1);
}

Qt::ItemFlags QStandardItem::flags() const
{
   QVariant v = data(Qt::UserRole - 1);
   if (!v.isValid())
      return (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable
            | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
   return Qt::ItemFlags(v.toInt());
}

void QStandardItem::setEnabled(bool enabled)
{
   Q_D(QStandardItem);
   d->changeFlags(enabled, Qt::ItemIsEnabled);
}

void QStandardItem::setEditable(bool editable)
{
   Q_D(QStandardItem);
   d->changeFlags(editable, Qt::ItemIsEditable);
}

void QStandardItem::setSelectable(bool selectable)
{
   Q_D(QStandardItem);
   d->changeFlags(selectable, Qt::ItemIsSelectable);
}

void QStandardItem::setCheckable(bool checkable)
{
   Q_D(QStandardItem);

   if (checkable && !isCheckable()) {
      // make sure there's data for the checkstate role
      if (!data(Qt::CheckStateRole).isValid()) {
         setData(Qt::Unchecked, Qt::CheckStateRole);
      }
   }

   d->changeFlags(checkable, Qt::ItemIsUserCheckable);
}

void QStandardItem::setAutoTristate(bool tristate)
{
    Q_D(QStandardItem);
    d->changeFlags(tristate, Qt::ItemIsAutoTristate);
}

void QStandardItem::setUserTristate(bool tristate)
{
   Q_D(QStandardItem);
   d->changeFlags(tristate, Qt::ItemIsUserTristate);
}

#ifndef QT_NO_DRAGANDDROP

void QStandardItem::setDragEnabled(bool dragEnabled)
{
   Q_D(QStandardItem);
   d->changeFlags(dragEnabled, Qt::ItemIsDragEnabled);
}

void QStandardItem::setDropEnabled(bool dropEnabled)
{
   Q_D(QStandardItem);
   d->changeFlags(dropEnabled, Qt::ItemIsDropEnabled);
}

#endif

int QStandardItem::row() const
{
   Q_D(const QStandardItem);
   QPair<int, int> pos = d->position();

   return pos.first;
}

int QStandardItem::column() const
{
   Q_D(const QStandardItem);
   QPair<int, int> pos = d->position();
   return pos.second;
}

QModelIndex QStandardItem::index() const
{
   Q_D(const QStandardItem);
   return d->model ? d->model->indexFromItem(this) : QModelIndex();
}

QStandardItemModel *QStandardItem::model() const
{
   Q_D(const QStandardItem);
   return d->model;
}

void QStandardItem::setRowCount(int rows)
{
   int rc = rowCount();
   if (rc == rows) {
      return;
   }

   if (rc < rows) {
      insertRows(qMax(rc, 0), rows - rc);
   } else {
      removeRows(qMax(rows, 0), rc - rows);
   }
}

int QStandardItem::rowCount() const
{
   Q_D(const QStandardItem);
   return d->rowCount();
}

void QStandardItem::setColumnCount(int columns)
{
   int cc = columnCount();
   if (cc == columns) {
      return;
   }

   if (cc < columns) {
      insertColumns(qMax(cc, 0), columns - cc);
   } else {
      removeColumns(qMax(columns, 0), cc - columns);
   }
}

int QStandardItem::columnCount() const
{
   Q_D(const QStandardItem);
   return d->columnCount();
}

void QStandardItem::insertRow(int row, const QList<QStandardItem *> &items)
{
   Q_D(QStandardItem);
   if (row < 0) {
      return;
   }
   if (columnCount() < items.count()) {
      setColumnCount(items.count());
   }
   d->insertRows(row, 1, items);
}

void QStandardItem::insertRows(int row, const QList<QStandardItem *> &items)
{
   Q_D(QStandardItem);
   if (row < 0) {
      return;
   }
   d->insertRows(row, items);
}

void QStandardItem::insertColumn(int column, const QList<QStandardItem *> &items)
{
   Q_D(QStandardItem);
   if (column < 0) {
      return;
   }
   if (rowCount() < items.count()) {
      setRowCount(items.count());
   }
   d->insertColumns(column, 1, items);
}

void QStandardItem::insertRows(int row, int count)
{
   Q_D(QStandardItem);
   if (rowCount() < row) {
      count += row - rowCount();
      row = rowCount();
   }
   d->insertRows(row, count, QList<QStandardItem *>());
}

void QStandardItem::insertColumns(int column, int count)
{
   Q_D(QStandardItem);
   if (columnCount() < column) {
      count += column - columnCount();
      column = columnCount();
   }
   d->insertColumns(column, count, QList<QStandardItem *>());
}

void QStandardItem::removeRow(int row)
{
   removeRows(row, 1);
}

void QStandardItem::removeColumn(int column)
{
   removeColumns(column, 1);
}

void QStandardItem::removeRows(int row, int count)
{
   Q_D(QStandardItem);

   if ((count < 1) || (row < 0) || ((row + count) > rowCount())) {
      return;
   }

   if (d->model != nullptr) {
      d->model->d_func()->rowsAboutToBeRemoved(this, row, row + count - 1);
   }

   int i = d->childIndex(row, 0);
   int n = count * d->columnCount();

   for (int j = i; j < n + i; ++j) {
      QStandardItem *oldItem = d->children.at(j);
      if (oldItem) {
         oldItem->d_func()->setModel(nullptr);
      }
      delete oldItem;
   }

   d->children.remove(qMax(i, 0), n);
   d->rows -= count;

   if (d->model) {
      d->model->d_func()->rowsRemoved(this, row, count);
   }
}

void QStandardItem::removeColumns(int column, int count)
{
   Q_D(QStandardItem);

   if ((count < 1) || (column < 0) || ((column + count) > columnCount())) {
      return;
   }

   if (d->model != nullptr) {
      d->model->d_func()->columnsAboutToBeRemoved(this, column, column + count - 1);
   }

   for (int row = d->rowCount() - 1; row >= 0; --row) {
      int i = d->childIndex(row, column);

      for (int j = i; j < i + count; ++j) {
         QStandardItem *oldItem = d->children.at(j);
         if (oldItem) {
            oldItem->d_func()->setModel(nullptr);
         }
         delete oldItem;
      }
      d->children.remove(i, count);
   }

   d->columns -= count;

   if (d->model) {
      d->model->d_func()->columnsRemoved(this, column, count);
   }
}

bool QStandardItem::hasChildren() const
{
   return (rowCount() > 0) && (columnCount() > 0);
}

void QStandardItem::setChild(int row, int column, QStandardItem *item)
{
   Q_D(QStandardItem);
   d->setChild(row, column, item, true);
}

QStandardItem *QStandardItem::child(int row, int column) const
{
   Q_D(const QStandardItem);

   int index = d->childIndex(row, column);

   if (index == -1) {
      return nullptr;
   }

   return d->children.at(index);
}

QStandardItem *QStandardItem::takeChild(int row, int column)
{
   Q_D(QStandardItem);
   QStandardItem *item = nullptr;
   int index = d->childIndex(row, column);

   if (index != -1) {
      item = d->children.at(index);

      if (item != nullptr) {
         item->d_func()->setParentAndModel(nullptr, nullptr);
      }

      d->children.replace(index, nullptr);
   }

   return item;
}

QList<QStandardItem *> QStandardItem::takeRow(int row)
{
   Q_D(QStandardItem);

   if ((row < 0) || (row >= rowCount())) {
      return QList<QStandardItem *>();
   }

   if (d->model) {
      d->model->d_func()->rowsAboutToBeRemoved(this, row, row);
   }

   QList<QStandardItem *> items;
   int index = d->childIndex(row, 0);  // Will return -1 if there are no columns

   if (index != -1) {
      int col_count = d->columnCount();
      for (int column = 0; column < col_count; ++column) {
         QStandardItem *ch = d->children.at(index + column);
         if (ch) {
            ch->d_func()->setParentAndModel(nullptr, nullptr);
         }
         items.append(ch);
      }
      d->children.remove(index, col_count);
   }

   --d->rows;

   if (d->model != nullptr) {
      d->model->d_func()->rowsRemoved(this, row, 1);
   }

   return items;
}

QList<QStandardItem *> QStandardItem::takeColumn(int column)
{
   Q_D(QStandardItem);

   if ((column < 0) || (column >= columnCount())) {
      return QList<QStandardItem *>();
   }

   if (d->model != nullptr) {
      d->model->d_func()->columnsAboutToBeRemoved(this, column, column);
   }

   QList<QStandardItem *> items;

   for (int row = d->rowCount() - 1; row >= 0; --row) {
      int index = d->childIndex(row, column);
      QStandardItem *ch = d->children.at(index);

      if (ch != nullptr) {
         ch->d_func()->setParentAndModel(nullptr, nullptr);
      }

      d->children.remove(index);
      items.prepend(ch);
   }

   --d->columns;

   if (d->model != nullptr) {
      d->model->d_func()->columnsRemoved(this, column, 1);
   }

   return items;
}

bool QStandardItem::operator<(const QStandardItem &other) const
{
   const int role = model() ? model()->sortRole() : Qt::DisplayRole;
   const QVariant l = data(role), r = other.data(role);

   // code is copied from QSortFilterProxyModel::lessThan()
   if (l.userType() == QVariant::Invalid) {
      return false;
   }

   if (r.userType() == QVariant::Invalid) {
      return true;
   }

   switch (l.userType()) {

      case QVariant::Int:
         return l.toInt() < r.toInt();

      case QVariant::UInt:
         return l.toUInt() < r.toUInt();

      case QVariant::LongLong:
         return l.toLongLong() < r.toLongLong();

      case QVariant::ULongLong:
         return l.toULongLong() < r.toULongLong();

      case QVariant::Float:
         return l.toFloat() < r.toFloat();

      case QVariant::Double:
         return l.toDouble() < r.toDouble();

      case QVariant::Char:
         return l.toChar() < r.toChar();

      case QVariant::Date:
         return l.toDate() < r.toDate();

      case QVariant::Time:
         return l.toTime() < r.toTime();

      case QVariant::DateTime:
         return l.toDateTime() < r.toDateTime();

      case QVariant::String:
      default:
         return l.toString().compare(r.toString()) < 0;
   }
}

void QStandardItem::sortChildren(int column, Qt::SortOrder order)
{
   Q_D(QStandardItem);

   if ((column < 0) || (rowCount() == 0)) {
      return;
   }

   QList<QPersistentModelIndex> parents;

   if (d->model) {
      parents << index();
      emit d->model->layoutAboutToBeChanged(parents, QAbstractItemModel::VerticalSortHint);
   }

   d->sortChildren(column, order);

   if (d->model != nullptr) {
      emit d->model->layoutChanged(parents, QAbstractItemModel::VerticalSortHint);
   }
}

QStandardItem *QStandardItem::clone() const
{
   return new QStandardItem(*this);
}

int QStandardItem::type() const
{
   return Type;
}

void QStandardItem::read(QDataStream &in)
{
   Q_D(QStandardItem);
   in >> d->values;
   qint32 flags;
   in >> flags;
   setFlags(Qt::ItemFlags(flags));
}

void QStandardItem::write(QDataStream &out) const
{
   Q_D(const QStandardItem);
   out << d->values;
   out << flags();
}

QDataStream &operator>>(QDataStream &in, QStandardItem &item)
{
   item.read(in);
   return in;
}

QDataStream &operator<<(QDataStream &out, const QStandardItem &item)
{
   item.write(out);
   return out;
}

QStandardItemModel::QStandardItemModel(QObject *parent)
   : QAbstractItemModel(*new QStandardItemModelPrivate, parent)
{
   Q_D(QStandardItemModel);
   d->init();
   d->root->d_func()->setModel(this);
}

QStandardItemModel::QStandardItemModel(int rows, int columns, QObject *parent)
   : QAbstractItemModel(*new QStandardItemModelPrivate, parent)
{
   Q_D(QStandardItemModel);
   d->init();

   d->root->insertColumns(0, columns);
   d->columnHeaderItems.insert(0, columns, nullptr);
   d->root->insertRows(0, rows);
   d->rowHeaderItems.insert(0, rows, nullptr);
   d->root->d_func()->setModel(this);
}

QStandardItemModel::QStandardItemModel(QStandardItemModelPrivate &dd, QObject *parent)
   : QAbstractItemModel(dd, parent)
{
   Q_D(QStandardItemModel);
   d->init();
}

QStandardItemModel::~QStandardItemModel()
{
}

void QStandardItemModel::setItemRoleNames(const QMultiHash<int, QString> &roleNames)
{
   Q_D(QStandardItemModel);
   d->roleNames = roleNames;
}

void QStandardItemModel::clear()
{
   Q_D(QStandardItemModel);
   beginResetModel();

   d->root.reset(new QStandardItem);
   d->root->d_func()->setModel(this);
   qDeleteAll(d->columnHeaderItems);

   d->columnHeaderItems.clear();
   qDeleteAll(d->rowHeaderItems);

   d->rowHeaderItems.clear();
   endResetModel();
}

QStandardItem *QStandardItemModel::itemFromIndex(const QModelIndex &index) const
{
   Q_D(const QStandardItemModel);

   if ((index.row() < 0) || (index.column() < 0) || (index.model() != this)) {
      return nullptr;
   }

   QStandardItem *parent = static_cast<QStandardItem *>(index.internalPointer());
   if (parent == nullptr) {
      return nullptr;
   }

   QStandardItem *item = parent->child(index.row(), index.column());
   // lazy part
   if (item == nullptr) {
      item = d->createItem();
      parent->d_func()->setChild(index.row(), index.column(), item);
   }

   return item;
}

QModelIndex QStandardItemModel::indexFromItem(const QStandardItem *item) const
{
   if (item && item->d_func()->parent) {
      QPair<int, int> pos = item->d_func()->position();
      return createIndex(pos.first, pos.second, item->d_func()->parent);
   }

   return QModelIndex();
}

void QStandardItemModel::setRowCount(int rows)
{
   Q_D(QStandardItemModel);
   d->root->setRowCount(rows);
}

void QStandardItemModel::setColumnCount(int columns)
{
   Q_D(QStandardItemModel);
   d->root->setColumnCount(columns);
}

void QStandardItemModel::setItem(int row, int column, QStandardItem *item)
{
   Q_D(QStandardItemModel);
   d->root->d_func()->setChild(row, column, item, true);
}

QStandardItem *QStandardItemModel::item(int row, int column) const
{
   Q_D(const QStandardItemModel);
   return d->root->child(row, column);
}

QStandardItem *QStandardItemModel::invisibleRootItem() const
{
   Q_D(const QStandardItemModel);
   return d->root.data();
}

void QStandardItemModel::setHorizontalHeaderItem(int column, QStandardItem *item)
{
   Q_D(QStandardItemModel);

   if (column < 0) {
      return;
   }
   if (columnCount() <= column) {
      setColumnCount(column + 1);
   }

   QStandardItem *oldItem = d->columnHeaderItems.at(column);
   if (item == oldItem) {
      return;
   }

   if (item != nullptr) {
      if (item->model() == nullptr) {
         item->d_func()->setModel(this);
      } else {
         qWarning("QStandardItem::setHorizontalHeaderItem() Ignoring duplicate insertion of item");
         return;
      }
   }

   if (oldItem != nullptr) {
      oldItem->d_func()->setModel(nullptr);
   }

   delete oldItem;
   d->columnHeaderItems.replace(column, item);

   emit headerDataChanged(Qt::Horizontal, column, column);
}

QStandardItem *QStandardItemModel::horizontalHeaderItem(int column) const
{
   Q_D(const QStandardItemModel);

   if ((column < 0) || (column >= columnCount())) {
      return nullptr;
   }

   return d->columnHeaderItems.at(column);
}

void QStandardItemModel::setVerticalHeaderItem(int row, QStandardItem *item)
{
   Q_D(QStandardItemModel);

   if (row < 0) {
      return;
   }

   if (rowCount() <= row) {
      setRowCount(row + 1);
   }

   QStandardItem *oldItem = d->rowHeaderItems.at(row);
   if (item == oldItem) {
      return;
   }

   if (item != nullptr) {
      if (item->model() == nullptr) {
         item->d_func()->setModel(this);
      } else {
         qWarning("QStandardItem::setVerticalHeaderItem() Ignoring duplicate insertion of item");
         return;
      }
   }

   if (oldItem != nullptr) {
      oldItem->d_func()->setModel(nullptr);
   }
   delete oldItem;

   d->rowHeaderItems.replace(row, item);
   emit headerDataChanged(Qt::Vertical, row, row);
}

QStandardItem *QStandardItemModel::verticalHeaderItem(int row) const
{
   Q_D(const QStandardItemModel);

   if ((row < 0) || (row >= rowCount())) {
      return nullptr;
   }

   return d->rowHeaderItems.at(row);
}

void QStandardItemModel::setHorizontalHeaderLabels(const QStringList &labels)
{
   Q_D(QStandardItemModel);

   if (columnCount() < labels.count()) {
      setColumnCount(labels.count());
   }

   for (int i = 0; i < labels.count(); ++i) {
      QStandardItem *item = horizontalHeaderItem(i);

      if (item == nullptr) {
         item = d->createItem();
         setHorizontalHeaderItem(i, item);
      }

      item->setText(labels.at(i));
   }
}

void QStandardItemModel::setVerticalHeaderLabels(const QStringList &labels)
{
   Q_D(QStandardItemModel);

   if (rowCount() < labels.count()) {
      setRowCount(labels.count());
   }

   for (int i = 0; i < labels.count(); ++i) {
      QStandardItem *item = verticalHeaderItem(i);

      if (item == nullptr) {
         item = d->createItem();
         setVerticalHeaderItem(i, item);
      }

      item->setText(labels.at(i));
   }
}

void QStandardItemModel::setItemPrototype(const QStandardItem *item)
{
   Q_D(QStandardItemModel);

   if (d->itemPrototype != item) {
      delete d->itemPrototype;
      d->itemPrototype = item;
   }
}

const QStandardItem *QStandardItemModel::itemPrototype() const
{
   Q_D(const QStandardItemModel);
   return d->itemPrototype;
}

QList<QStandardItem *> QStandardItemModel::findItems(const QString &text,
   Qt::MatchFlags flags, int column) const
{
   QModelIndexList indexes = match(index(0, column, QModelIndex()),
         Qt::DisplayRole, text, -1, flags);

   QList<QStandardItem *> items;

   for (int i = 0; i < indexes.size(); ++i) {
      items.append(itemFromIndex(indexes.at(i)));
   }

   return items;
}

void QStandardItemModel::appendRow(const QList<QStandardItem *> &items)
{
   invisibleRootItem()->appendRow(items);
}

void QStandardItemModel::appendColumn(const QList<QStandardItem *> &items)
{
   invisibleRootItem()->appendColumn(items);
}

void QStandardItemModel::insertRow(int row, const QList<QStandardItem *> &items)
{
   invisibleRootItem()->insertRow(row, items);
}

void QStandardItemModel::insertColumn(int column, const QList<QStandardItem *> &items)
{
   invisibleRootItem()->insertColumn(column, items);
}

QStandardItem *QStandardItemModel::takeItem(int row, int column)
{
   Q_D(QStandardItemModel);
   return d->root->takeChild(row, column);
}

QList<QStandardItem *> QStandardItemModel::takeRow(int row)
{
   Q_D(QStandardItemModel);
   return d->root->takeRow(row);
}

QList<QStandardItem *> QStandardItemModel::takeColumn(int column)
{
   Q_D(QStandardItemModel);
   return d->root->takeColumn(column);
}

QStandardItem *QStandardItemModel::takeHorizontalHeaderItem(int column)
{
   Q_D(QStandardItemModel);

   if ((column < 0) || (column >= columnCount())) {
      return nullptr;
   }

   QStandardItem *headerItem = d->columnHeaderItems.at(column);

   if (headerItem != nullptr) {
      headerItem->d_func()->setParentAndModel(nullptr, nullptr);
      d->columnHeaderItems.replace(column, nullptr);
   }

   return headerItem;
}

QStandardItem *QStandardItemModel::takeVerticalHeaderItem(int row)
{
   Q_D(QStandardItemModel);

   if ((row < 0) || (row >= rowCount())) {
      return nullptr;
   }

   QStandardItem *headerItem = d->rowHeaderItems.at(row);

   if (headerItem != nullptr) {
      headerItem->d_func()->setParentAndModel(nullptr, nullptr);
      d->rowHeaderItems.replace(row, nullptr);
   }

   return headerItem;
}

int QStandardItemModel::sortRole() const
{
   Q_D(const QStandardItemModel);
   return d->sortRole;
}

void QStandardItemModel::setSortRole(int role)
{
   Q_D(QStandardItemModel);
   d->sortRole = role;
}

int QStandardItemModel::columnCount(const QModelIndex &parent) const
{
   Q_D(const QStandardItemModel);
   QStandardItem *item = d->itemFromIndex(parent);

   return item ? item->columnCount() : 0;
}

QVariant QStandardItemModel::data(const QModelIndex &index, int role) const
{
   Q_D(const QStandardItemModel);
   QStandardItem *item = d->itemFromIndex(index);

   return item ? item->data(role) : QVariant();
}


Qt::ItemFlags QStandardItemModel::flags(const QModelIndex &index) const
{
   Q_D(const QStandardItemModel);
   if (!d->indexValid(index)) {
      return d->root->flags();
   }

   QStandardItem *item = d->itemFromIndex(index);
   if (item) {
      return item->flags();
   }

   return Qt::ItemIsSelectable
      | Qt::ItemIsEnabled
      | Qt::ItemIsEditable
      | Qt::ItemIsDragEnabled
      | Qt::ItemIsDropEnabled;
}

bool QStandardItemModel::hasChildren(const QModelIndex &parent) const
{
   Q_D(const QStandardItemModel);
   QStandardItem *item = d->itemFromIndex(parent);

   return item ? item->hasChildren() : false;
}

QVariant QStandardItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_D(const QStandardItemModel);

   if ((section < 0) || ((orientation == Qt::Horizontal) && (section >= columnCount()))
            || ((orientation == Qt::Vertical) && (section >= rowCount()))) {
      return QVariant();
   }

   QStandardItem *headerItem = nullptr;
   if (orientation == Qt::Horizontal) {
      headerItem = d->columnHeaderItems.at(section);
   } else if (orientation == Qt::Vertical) {
      headerItem = d->rowHeaderItems.at(section);
   }

   return headerItem ? headerItem->data(role)
      : QAbstractItemModel::headerData(section, orientation, role);
}

Qt::DropActions QStandardItemModel::supportedDropActions () const
{
   return Qt::CopyAction | Qt::MoveAction;
}

QModelIndex QStandardItemModel::index(int row, int column, const QModelIndex &parent) const
{
   Q_D(const QStandardItemModel);
   QStandardItem *parentItem = d->itemFromIndex(parent);

   if ((parentItem == nullptr) || (row < 0) || (column < 0)
            || (row >= parentItem->rowCount()) || (column >= parentItem->columnCount())) {
      return QModelIndex();
   }

   return createIndex(row, column, parentItem);
}

bool QStandardItemModel::insertColumns(int column, int count, const QModelIndex &parent)
{
   Q_D(QStandardItemModel);

   QStandardItem *item = parent.isValid() ? itemFromIndex(parent) : d->root.data();

   if (item == nullptr) {
      return false;
   }

   return item->d_func()->insertColumns(column, count, QList<QStandardItem *>());
}

bool QStandardItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
   Q_D(QStandardItemModel);

   QStandardItem *item = parent.isValid() ? itemFromIndex(parent) : d->root.data();
   if (item == nullptr) {
      return false;
   }

   return item->d_func()->insertRows(row, count, QList<QStandardItem *>());
}

QMap<int, QVariant> QStandardItemModel::itemData(const QModelIndex &index) const
{
   Q_D(const QStandardItemModel);
   QStandardItem *item = d->itemFromIndex(index);

   return item ? item->d_func()->itemData() : QMap<int, QVariant>();
}

QModelIndex QStandardItemModel::parent(const QModelIndex &child) const
{
   Q_D(const QStandardItemModel);

   if (! d->indexValid(child)) {
      return QModelIndex();
   }

   QStandardItem *parentItem = static_cast<QStandardItem *>(child.internalPointer());
   return indexFromItem(parentItem);
}

bool QStandardItemModel::removeColumns(int column, int count, const QModelIndex &parent)
{
   Q_D(QStandardItemModel);

   QStandardItem *item = d->itemFromIndex(parent);

   if ((item == nullptr) || (count < 1) || (column < 0) || ((column + count) > item->columnCount())) {
      return false;
   }

   item->removeColumns(column, count);

   return true;
}

bool QStandardItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
   Q_D(QStandardItemModel);

   QStandardItem *item = d->itemFromIndex(parent);

   if ((item == nullptr) || (count < 1) || (row < 0) || ((row + count) > item->rowCount())) {
      return false;
   }

   item->removeRows(row, count);

   return true;
}

int QStandardItemModel::rowCount(const QModelIndex &parent) const
{
   Q_D(const QStandardItemModel);
   QStandardItem *item = d->itemFromIndex(parent);

   return item ? item->rowCount() : 0;
}

bool QStandardItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   if (! index.isValid()) {
      return false;
   }

   QStandardItem *item = itemFromIndex(index);

   if (item == nullptr) {
      return false;
   }

   item->setData(value, role);

   return true;
}

bool QStandardItemModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
   Q_D(QStandardItemModel);

   if ((section < 0)
      || ((orientation == Qt::Horizontal) && (section >= columnCount()))
      || ((orientation == Qt::Vertical) && (section >= rowCount()))) {
      return false;
   }

   QStandardItem *headerItem = nullptr;

   if (orientation == Qt::Horizontal) {
      headerItem = d->columnHeaderItems.at(section);

      if (headerItem == nullptr) {
         headerItem = d->createItem();
         headerItem->d_func()->setModel(this);
         d->columnHeaderItems.replace(section, headerItem);
      }

   } else if (orientation == Qt::Vertical) {
      headerItem = d->rowHeaderItems.at(section);

      if (headerItem == nullptr) {
         headerItem = d->createItem();
         headerItem->d_func()->setModel(this);
         d->rowHeaderItems.replace(section, headerItem);
      }
   }

   if (headerItem != nullptr) {
      headerItem->setData(value, role);
      return true;
   }

   return false;
}

bool QStandardItemModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
   QStandardItem *item = itemFromIndex(index);

   if (item == nullptr) {
      return false;
   }

   item->d_func()->setItemData(roles);

   return true;
}

void QStandardItemModel::sort(int column, Qt::SortOrder order)
{
   Q_D(QStandardItemModel);
   d->root->sortChildren(column, order);
}

QStringList QStandardItemModel::mimeTypes() const
{
   return QAbstractItemModel::mimeTypes() <<  QString("application/x-qstandarditemmodeldatalist");
}

QMimeData *QStandardItemModel::mimeData(const QModelIndexList &indexes) const
{
   QMimeData *data = QAbstractItemModel::mimeData(indexes);
   if (! data) {
      return nullptr;
   }

   QString format = "application/x-qstandarditemmodeldatalist";
   if (! mimeTypes().contains(format)) {
      return data;
   }

   QByteArray encoded;
   QDataStream stream(&encoded, QIODevice::WriteOnly);

   QSet<QStandardItem *> itemsSet;
   QStack<QStandardItem *> stack;
   itemsSet.reserve(indexes.count());
   stack.reserve(indexes.count());

   for (int i = 0; i < indexes.count(); ++i) {
      if (QStandardItem *item = itemFromIndex(indexes.at(i))) {
         itemsSet << item;
         stack.push(item);
      } else {
         qWarning("QStandardItemModel::mimeData() No item was associated with this index");
         return nullptr;
      }
   }

   // remove duplicate children
   {
      QSet<QStandardItem *> seen;

      while (!stack.isEmpty()) {
         QStandardItem *itm = stack.pop();

         if (seen.contains(itm)) {
            continue;
         }
         seen.insert(itm);

         const QVector<QStandardItem *> &childList = itm->d_func()->children;

         for (int i = 0; i < childList.count(); ++i) {
            QStandardItem *chi = childList.at(i);

            if (chi) {
               QSet<QStandardItem *>::iterator it = itemsSet.find(chi);
               if (it != itemsSet.end()) {
                  itemsSet.erase(it);
               }

               stack.push(chi);
            }
         }
      }
   }

   stack.reserve(itemsSet.count());
   for (QStandardItem *item : itemsSet) {
      stack.push(item);
   }

   // stream everything recursively
   while (!stack.isEmpty()) {
      QStandardItem *item = stack.pop();
      if (itemsSet.contains(item)) {
         //if the item is selection 'top-level', strem its position
         stream << item->row() << item->column();
      }

      stream << *item << item->columnCount() << item->d_ptr->children.count();
      stack += item->d_ptr->children;
   }

   data->setData(format, encoded);

   return data;
}

void QStandardItemModelPrivate::decodeDataRecursive(QDataStream &stream, QStandardItem *item)
{
   int colCount, childCount;
   stream >> *item;
   stream >> colCount >> childCount;
   item->setColumnCount(colCount);

   int childPos = childCount;

   while (childPos > 0) {
      childPos--;
      QStandardItem *child = createItem();
      decodeDataRecursive(stream, child);
      item->setChild( childPos / colCount, childPos % colCount, child);
   }
}

bool QStandardItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
   int row, int column, const QModelIndex &parent)
{
   Q_D(QStandardItemModel);

   // check if the action is supported
   if (!data || !(action == Qt::CopyAction || action == Qt::MoveAction)) {
      return false;
   }

   // check if the format is supported
   QString format = QLatin1String("application/x-qstandarditemmodeldatalist");
   if (!data->hasFormat(format)) {
      return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
   }

   if (row > rowCount(parent)) {
      row = rowCount(parent);
   }

   if (row == -1) {
      row = rowCount(parent);
   }

   if (column == -1) {
      column = 0;
   }

   // decode and insert
   QByteArray encoded = data->data(format);
   QDataStream stream(&encoded, QIODevice::ReadOnly);

   //code based on QAbstractItemModel::decodeData
   // adapted to work with QStandardItem
   int top = INT_MAX;
   int left = INT_MAX;
   int bottom = 0;
   int right = 0;
   QVector<int> rows, columns;
   QVector<QStandardItem *> items;

   while (! stream.atEnd()) {
      int r, c;
      QStandardItem *item = d->createItem();
      stream >> r >> c;
      d->decodeDataRecursive(stream, item);

      rows.append(r);
      columns.append(c);
      items.append(item);
      top = qMin(r, top);
      left = qMin(c, left);
      bottom = qMax(r, bottom);
      right = qMax(c, right);
   }

   // insert the dragged items into the table, use a bit array to avoid overwriting items,
   // since items from different tables can have the same row and column
   int dragRowCount = 0;
   int dragColumnCount = right - left + 1;

   // Compute the number of continuous rows upon insertion and modify the rows to match
   QVector<int> rowsToInsert(bottom + 1);
   for (int i = 0; i < rows.count(); ++i) {
      rowsToInsert[rows.at(i)] = 1;
   }

   for (int i = 0; i < rowsToInsert.count(); ++i) {
      if (rowsToInsert[i] == 1) {
         rowsToInsert[i] = dragRowCount;
         ++dragRowCount;
      }
   }

   for (int i = 0; i < rows.count(); ++i) {
      rows[i] = top + rowsToInsert[rows[i]];
   }

   QBitArray isWrittenTo(dragRowCount * dragColumnCount);

   // make space in the table for the dropped data
   int colCount = columnCount(parent);
   if (colCount < dragColumnCount + column) {
      insertColumns(colCount, dragColumnCount + column - colCount, parent);
      colCount = columnCount(parent);
   }
   insertRows(row, dragRowCount, parent);

   row    = qMax(0, row);
   column = qMax(0, column);

   QStandardItem *parentItem = itemFromIndex (parent);
   if (! parentItem) {
      parentItem = invisibleRootItem();
   }

   QVector<QPersistentModelIndex> newIndexes(items.size());

   // set the data in the table
   for (int j = 0; j < items.size(); ++j) {
      int relativeRow = rows.at(j) - top;
      int relativeColumn = columns.at(j) - left;
      int destinationRow = relativeRow + row;
      int destinationColumn = relativeColumn + column;
      int flat = (relativeRow * dragColumnCount) + relativeColumn;

      // if the item was already written to, or we just can't fit it in the table, create a new row
      if (destinationColumn >= colCount || isWrittenTo.testBit(flat)) {
         destinationColumn = qBound(column, destinationColumn, colCount - 1);
         destinationRow = row + dragRowCount;
         insertRows(row + dragRowCount, 1, parent);
         flat = (dragRowCount * dragColumnCount) + relativeColumn;
         isWrittenTo.resize(++dragRowCount * dragColumnCount);
      }

      if (! isWrittenTo.testBit(flat)) {
         newIndexes[j] = index(destinationRow, destinationColumn, parentItem->index());
         isWrittenTo.setBit(flat);
      }
   }

   for (int k = 0; k < newIndexes.size(); k++) {
      if (newIndexes.at(k).isValid()) {
         parentItem->setChild(newIndexes.at(k).row(), newIndexes.at(k).column(), items.at(k));
      } else {
         delete items.at(k);
      }
   }

   return true;
}

void QStandardItemModel::_q_emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
   Q_D(QStandardItemModel);
   d->_q_emitItemChanged(topLeft, bottomRight);
}

#endif // QT_NO_STANDARDITEMMODEL
