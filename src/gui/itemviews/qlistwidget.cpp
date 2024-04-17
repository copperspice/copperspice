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

#include <qlistwidget.h>

#ifndef QT_NO_LISTWIDGET

#include <qitemdelegate.h>

#include <qlistview_p.h>
#include <qlistwidget_p.h>
#include <qwidgetitemdata_p.h>

#include <algorithm>

// emerald - workaround for VC++ 6.0 linker bug
typedef bool(*LessThan)(const QPair<QListWidgetItem *, int> &, const QPair<QListWidgetItem *, int> &);

class QListWidgetMimeData : public QMimeData
{
   GUI_CS_OBJECT(QListWidgetMimeData)

 public:
   QList<QListWidgetItem *> items;
};

QListModel::QListModel(QListWidget *parent)
   : QAbstractListModel(parent)
{
}

QListModel::~QListModel()
{
   clear();
}

void QListModel::clear()
{
   beginResetModel();

   for (int i = 0; i < items.count(); ++i) {
      if (items.at(i)) {
         items.at(i)->d->theid = -1;
         items.at(i)->view = nullptr;
         delete items.at(i);
      }
   }
   items.clear();
   endResetModel();
}

QListWidgetItem *QListModel::at(int row) const
{
   return items.value(row);
}

void QListModel::remove(QListWidgetItem *item)
{
   if (!item) {
      return;
   }

   int row = items.indexOf(item); // ### use index(item) - it is faster

   Q_ASSERT(row != -1);
   beginRemoveRows(QModelIndex(), row, row);
   items.at(row)->d->theid = -1;
   items.at(row)->view = nullptr;
   items.removeAt(row);
   endRemoveRows();
}

void QListModel::insert(int row, QListWidgetItem *item)
{
   if (! item) {
      return;
   }

   item->view = qobject_cast<QListWidget *>(QObject::parent());
   if (item->view && item->view->isSortingEnabled()) {
      // sorted insertion
      QList<QListWidgetItem *>::iterator it;
      it = sortedInsertionIterator(items.begin(), items.end(), item->view->sortOrder(), item);
      row = qMax(it - items.begin(), 0);

   } else {
      if (row < 0) {
         row = 0;
      } else if (row > items.count()) {
         row = items.count();
      }
   }

   beginInsertRows(QModelIndex(), row, row);
   items.insert(row, item);
   item->d->theid = row;
   endInsertRows();
}

void QListModel::insert(int row, const QStringList &labels)
{
   const int count = labels.count();
   if (count <= 0) {
      return;
   }
   QListWidget *view = qobject_cast<QListWidget *>(QObject::parent());
   if (view && view->isSortingEnabled()) {
      // sorted insertion
      for (int i = 0; i < count; ++i) {
         QListWidgetItem *item = new QListWidgetItem(labels.at(i));
         insert(row, item);
      }
   } else {
      if (row < 0) {
         row = 0;
      } else if (row > items.count()) {
         row = items.count();
      }
      beginInsertRows(QModelIndex(), row, row + count - 1);
      for (int i = 0; i < count; ++i) {
         QListWidgetItem *item = new QListWidgetItem(labels.at(i));
         item->d->theid = row;
         item->view = qobject_cast<QListWidget *>(QObject::parent());
         items.insert(row++, item);
      }
      endInsertRows();
   }
}

QListWidgetItem *QListModel::take(int row)
{
   if (row < 0 || row >= items.count()) {
      return nullptr;
   }

   beginRemoveRows(QModelIndex(), row, row);
   items.at(row)->d->theid = -1;
   items.at(row)->view     = nullptr;
   QListWidgetItem *item   = items.takeAt(row);
   endRemoveRows();

   return item;
}

void QListModel::move(int srcRow, int dstRow)
{
   if (srcRow == dstRow
      || srcRow < 0 || srcRow >= items.count()
      || dstRow < 0 || dstRow > items.count()) {
      return;
   }

   if (!beginMoveRows(QModelIndex(), srcRow, srcRow, QModelIndex(), dstRow)) {
      return;
   }
   if (srcRow < dstRow) {
      --dstRow;
   }
   items.move(srcRow, dstRow);
   endMoveRows();
}

int QListModel::rowCount(const QModelIndex &parent) const
{
   return parent.isValid() ? 0 : items.count();
}

QModelIndex QListModel::index(QListWidgetItem *item) const
{
   if (!item || !item->view || static_cast<const QListModel *>(item->view->model()) != this
      || items.isEmpty()) {
      return QModelIndex();
   }
   int row;
   const int theid = item->d->theid;
   if (theid >= 0 && theid < items.count() && items.at(theid) == item) {
      row = theid;
   } else { // we need to search for the item
      row = items.lastIndexOf(item);  // lastIndexOf is an optimization in favor of indexOf
      if (row == -1) { // not found
         return QModelIndex();
      }
      item->d->theid = row;
   }
   return createIndex(row, 0, item);
}

QModelIndex QListModel::index(int row, int column, const QModelIndex &parent) const
{
   if (hasIndex(row, column, parent)) {
      return createIndex(row, column, items.at(row));
   }
   return QModelIndex();
}

QVariant QListModel::data(const QModelIndex &index, int role) const
{
   if (!index.isValid() || index.row() >= items.count()) {
      return QVariant();
   }
   return items.at(index.row())->data(role);
}

bool QListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   if (!index.isValid() || index.row() >= items.count()) {
      return false;
   }
   items.at(index.row())->setData(role, value);
   return true;
}

QMap<int, QVariant> QListModel::itemData(const QModelIndex &index) const
{
   QMap<int, QVariant> roles;
   if (!index.isValid() || index.row() >= items.count()) {
      return roles;
   }
   QListWidgetItem *itm = items.at(index.row());
   for (int i = 0; i < itm->d->values.count(); ++i) {
      roles.insert(itm->d->values.at(i).role,
         itm->d->values.at(i).value);
   }
   return roles;
}

bool QListModel::insertRows(int row, int count, const QModelIndex &parent)
{
   if (count < 1 || row < 0 || row > rowCount() || parent.isValid()) {
      return false;
   }

   beginInsertRows(QModelIndex(), row, row + count - 1);
   QListWidget *view = qobject_cast<QListWidget *>(QObject::parent());
   QListWidgetItem *itm = nullptr;

   for (int r = row; r < row + count; ++r) {
      itm = new QListWidgetItem;
      itm->view = view;
      itm->d->theid = r;
      items.insert(r, itm);
   }

   endInsertRows();
   return true;
}

bool QListModel::removeRows(int row, int count, const QModelIndex &parent)
{
   if (count < 1 || row < 0 || (row + count) > rowCount() || parent.isValid()) {
      return false;
   }

   beginRemoveRows(QModelIndex(), row, row + count - 1);
   QListWidgetItem *itm = nullptr;

   for (int r = row; r < row + count; ++r) {
      itm = items.takeAt(row);
      itm->view = nullptr;
      itm->d->theid = -1;
      delete itm;
   }
   endRemoveRows();

   return true;
}

Qt::ItemFlags QListModel::flags(const QModelIndex &index) const
{
   if (!index.isValid() || index.row() >= items.count() || index.model() != this) {
      return Qt::ItemIsDropEnabled;   // we allow drops outside the items
   }
   return items.at(index.row())->flags();
}

void QListModel::sort(int column, Qt::SortOrder order)
{
   if (column != 0) {
      return;
   }

   emit layoutAboutToBeChanged();

   QVector < QPair<QListWidgetItem *, int>> sorting(items.count());
   for (int i = 0; i < items.count(); ++i) {
      QListWidgetItem *item = items.at(i);
      sorting[i].first = item;
      sorting[i].second = i;
   }

   LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
   std::sort(sorting.begin(), sorting.end(), compare);

   QModelIndexList fromIndexes;
   QModelIndexList toIndexes;

   for (int r = 0; r < sorting.count(); ++r) {
      QListWidgetItem *item = sorting.at(r).first;
      toIndexes.append(createIndex(r, 0, item));
      fromIndexes.append(createIndex(sorting.at(r).second, 0, sorting.at(r).first));
      items[r] = sorting.at(r).first;
   }
   changePersistentIndexList(fromIndexes, toIndexes);

   emit layoutChanged();
}

void QListModel::ensureSorted(int column, Qt::SortOrder order, int start, int end)
{
   if (column != 0) {
      return;
   }

   int count = end - start + 1;
   QVector < QPair<QListWidgetItem *, int>> sorting(count);
   for (int i = 0; i < count; ++i) {
      sorting[i].first = items.at(start + i);
      sorting[i].second = start + i;
   }

   LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
   std::sort(sorting.begin(), sorting.end(), compare);

   QModelIndexList oldPersistentIndexes = persistentIndexList();
   QModelIndexList newPersistentIndexes = oldPersistentIndexes;
   QList<QListWidgetItem *> tmp = items;
   QList<QListWidgetItem *>::iterator lit = tmp.begin();
   bool changed = false;
   for (int i = 0; i < count; ++i) {
      int oldRow = sorting.at(i).second;
      int tmpitepos = lit - tmp.begin();

      QListWidgetItem *item = tmp.takeAt(oldRow);

      if (tmpitepos > tmp.size()) {
         --tmpitepos;
      }
      lit = tmp.begin() + tmpitepos;

      lit = sortedInsertionIterator(lit, tmp.end(), order, item);
      int newRow = qMax(lit - tmp.begin(), 0);
      lit = tmp.insert(lit, item);
      if (newRow != oldRow) {
         changed = true;
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
      emit layoutAboutToBeChanged();
      items = tmp;
      changePersistentIndexList(oldPersistentIndexes, newPersistentIndexes);
      emit layoutChanged();
   }
}

bool QListModel::itemLessThan(const QPair<QListWidgetItem *, int> &left,
   const QPair<QListWidgetItem *, int> &right)
{
   return (*left.first) < (*right.first);
}

bool QListModel::itemGreaterThan(const QPair<QListWidgetItem *, int> &left,
   const QPair<QListWidgetItem *, int> &right)
{
   return (*right.first) < (*left.first);
}

QList<QListWidgetItem *>::iterator QListModel::sortedInsertionIterator(
   const QList<QListWidgetItem *>::iterator &begin,
   const QList<QListWidgetItem *>::iterator &end,
   Qt::SortOrder order, QListWidgetItem *item)
{
   if (order == Qt::AscendingOrder) {
      return std::lower_bound(begin, end, item, QListModelLessThan());
   }

   return std::lower_bound(begin, end, item, QListModelGreaterThan());
}

void QListModel::itemChanged(QListWidgetItem *item)
{
   QModelIndex idx = index(item);
   emit dataChanged(idx, idx);
}

QStringList QListModel::mimeTypes() const
{
   const QListWidget *view = qobject_cast<const QListWidget *>(QObject::parent());
   return view->mimeTypes();
}

QMimeData *QListModel::internalMimeData()  const
{
   return QAbstractItemModel::mimeData(cachedIndexes);
}

QMimeData *QListModel::mimeData(const QModelIndexList &indexes) const
{
   QList<QListWidgetItem *> itemlist;
   for (int i = 0; i < indexes.count(); ++i) {
      itemlist << at(indexes.at(i).row());
   }
   const QListWidget *view = qobject_cast<const QListWidget *>(QObject::parent());

   cachedIndexes = indexes;
   QMimeData *mimeData = view->mimeData(itemlist);
   cachedIndexes.clear();
   return mimeData;
}

#ifndef QT_NO_DRAGANDDROP
bool QListModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
   int row, int column, const QModelIndex &index)
{
   (void) column;
   QListWidget *view = qobject_cast<QListWidget *>(QObject::parent());
   if (index.isValid()) {
      row = index.row();
   } else if (row == -1) {
      row = items.count();
   }

   return view->dropMimeData(row, data, action);
}

Qt::DropActions QListModel::supportedDropActions() const
{
   const QListWidget *view = qobject_cast<const QListWidget *>(QObject::parent());
   return view->supportedDropActions();
}
#endif // QT_NO_DRAGANDDROP

QListWidgetItem::QListWidgetItem(QListWidget *view, int type)
   : rtti(type), view(view), d(new QListWidgetItemPrivate(this)),
     itemFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled)
{
   if (QListModel *model = (view ? qobject_cast<QListModel *>(view->model()) : nullptr)) {
      model->insert(model->rowCount(), this);
   }
}

QListWidgetItem::QListWidgetItem(const QString &text, QListWidget *view, int type)
   : rtti(type), view(nullptr), d(new QListWidgetItemPrivate(this)),
     itemFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled)
{
   setData(Qt::DisplayRole, text);
   this->view = view;

   if (QListModel *model = (view ? qobject_cast<QListModel *>(view->model()) : nullptr)) {
      model->insert(model->rowCount(), this);
   }
}

QListWidgetItem::QListWidgetItem(const QIcon &icon, const QString &text, QListWidget *view, int type)
   : rtti(type), view(nullptr), d(new QListWidgetItemPrivate(this)),
     itemFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled)
{
   setData(Qt::DisplayRole, text);
   setData(Qt::DecorationRole, icon);
   this->view = view;

   if (QListModel *model = (view ? qobject_cast<QListModel *>(view->model()) : nullptr)) {
      model->insert(model->rowCount(), this);
   }
}

QListWidgetItem::~QListWidgetItem()
{
   if (QListModel *model = (view ? qobject_cast<QListModel *>(view->model()) : nullptr)) {
      model->remove(this);
   }
   delete d;
}

QListWidgetItem *QListWidgetItem::clone() const
{
   return new QListWidgetItem(*this);
}

void QListWidgetItem::setData(int role, const QVariant &value)
{
   bool found = false;
   role = (role == Qt::EditRole ? Qt::DisplayRole : role);
   for (int i = 0; i < d->values.count(); ++i) {
      if (d->values.at(i).role == role) {
         if (d->values.at(i).value == value) {
            return;
         }
         d->values[i].value = value;
         found = true;
         break;
      }
   }
   if (!found) {
      d->values.append(QWidgetItemData(role, value));
   }
   if (QListModel *model = (view ? qobject_cast<QListModel *>(view->model()) : nullptr)) {
      model->itemChanged(this);
   }
}

QVariant QListWidgetItem::data(int role) const
{
   role = (role == Qt::EditRole ? Qt::DisplayRole : role);

   for (int i = 0; i < d->values.count(); ++i) {
      if (d->values.at(i).role == role) {
         return d->values.at(i).value;
      }
   }

   return QVariant();
}

bool QListWidgetItem::operator<(const QListWidgetItem &other) const
{
   const QVariant v1 = data(Qt::DisplayRole), v2 = other.data(Qt::DisplayRole);
   return QAbstractItemModelPrivate::variantLessThan(v1, v2);
}

void QListWidgetItem::read(QDataStream &in)
{
   in >> d->values;
}

void QListWidgetItem::write(QDataStream &out) const
{
   out << d->values;
}
QListWidgetItem::QListWidgetItem(const QListWidgetItem &other)
   : rtti(Type), view(nullptr), d(new QListWidgetItemPrivate(this)), itemFlags(other.itemFlags)
{
   d->values = other.d->values;
}

QListWidgetItem &QListWidgetItem::operator=(const QListWidgetItem &other)
{
   d->values = other.d->values;
   itemFlags = other.itemFlags;
   return *this;
}

QDataStream &operator<<(QDataStream &out, const QListWidgetItem &item)
{
   item.write(out);
   return out;
}

QDataStream &operator>>(QDataStream &in, QListWidgetItem &item)
{
   item.read(in);
   return in;
}

void QListWidgetItem::setFlags(Qt::ItemFlags aflags)
{
   itemFlags = aflags;

   if (QListModel *model = (view ? qobject_cast<QListModel *>(view->model()) : nullptr)) {
      model->itemChanged(this);
   }
}

void QListWidgetPrivate::setup()
{
   Q_Q(QListWidget);
   q->QListView::setModel(new QListModel(q));

   // view signals
   QObject::connect(q,     &QListWidget::pressed,         q, &QListWidget::_q_emitItemPressed);
   QObject::connect(q,     &QListWidget::clicked,         q, &QListWidget::_q_emitItemClicked);
   QObject::connect(q,     &QListWidget::doubleClicked,   q, &QListWidget::_q_emitItemDoubleClicked);
   QObject::connect(q,     &QListWidget::activated,       q, &QListWidget::_q_emitItemActivated);
   QObject::connect(q,     &QListWidget::entered,         q, &QListWidget::_q_emitItemEntered);
   QObject::connect(model, &QListModel::dataChanged,      q, &QListWidget::_q_emitItemChanged);

   QObject::connect(q->selectionModel(), &QItemSelectionModel::currentChanged,   q, &QListWidget::_q_emitCurrentItemChanged);
   QObject::connect(q->selectionModel(), &QItemSelectionModel::selectionChanged, q, &QListWidget::itemSelectionChanged);

   QObject::connect(model, &QListModel::dataChanged,     q, &QListWidget::_q_dataChanged);
   QObject::connect(model, &QListModel::columnsRemoved,  q, &QListWidget::_q_sort);
}

void QListWidgetPrivate::_q_emitItemPressed(const QModelIndex &index)
{
   Q_Q(QListWidget);
   emit q->itemPressed(listModel()->at(index.row()));
}

void QListWidgetPrivate::_q_emitItemClicked(const QModelIndex &index)
{
   Q_Q(QListWidget);
   emit q->itemClicked(listModel()->at(index.row()));
}

void QListWidgetPrivate::_q_emitItemDoubleClicked(const QModelIndex &index)
{
   Q_Q(QListWidget);
   emit q->itemDoubleClicked(listModel()->at(index.row()));
}

void QListWidgetPrivate::_q_emitItemActivated(const QModelIndex &index)
{
   Q_Q(QListWidget);
   emit q->itemActivated(listModel()->at(index.row()));
}

void QListWidgetPrivate::_q_emitItemEntered(const QModelIndex &index)
{
   Q_Q(QListWidget);
   emit q->itemEntered(listModel()->at(index.row()));
}

void QListWidgetPrivate::_q_emitItemChanged(const QModelIndex &index)
{
   Q_Q(QListWidget);
   emit q->itemChanged(listModel()->at(index.row()));
}

void QListWidgetPrivate::_q_emitCurrentItemChanged(const QModelIndex &current, const QModelIndex &previous)
{
   Q_Q(QListWidget);

   QPersistentModelIndex persistentCurrent = current;
   QListWidgetItem *currentItem = listModel()->at(persistentCurrent.row());
   emit q->currentItemChanged(currentItem, listModel()->at(previous.row()));

   //persistentCurrent is invalid if something changed the model in response
   //to the currentItemChanged signal emission and the item was removed
   if (!persistentCurrent.isValid()) {
      currentItem = nullptr;
   }

   emit q->currentTextChanged(currentItem ? currentItem->text() : QString());
   emit q->currentRowChanged(persistentCurrent.row());
}

void QListWidgetPrivate::_q_sort()
{
   if (sortingEnabled) {
      model->sort(0, sortOrder);
   }
}

void QListWidgetPrivate::_q_dataChanged(const QModelIndex &topLeft,
   const QModelIndex &bottomRight)
{
   if (sortingEnabled && topLeft.isValid() && bottomRight.isValid())
      listModel()->ensureSorted(topLeft.column(), sortOrder,
         topLeft.row(), bottomRight.row());
}

QListWidget::QListWidget(QWidget *parent)
   : QListView(*new QListWidgetPrivate(), parent)
{
   Q_D(QListWidget);
   d->setup();
}

QListWidget::~QListWidget()
{
}

QListWidgetItem *QListWidget::item(int row) const
{
   Q_D(const QListWidget);

   if (row < 0 || row >= d->model->rowCount()) {
      return nullptr;
   }

   return d->listModel()->at(row);
}

int QListWidget::row(const QListWidgetItem *item) const
{
   Q_D(const QListWidget);
   return d->listModel()->index(const_cast<QListWidgetItem *>(item)).row();
}

void QListWidget::insertItem(int row, QListWidgetItem *item)
{
   Q_D(QListWidget);

   if (item && !item->view) {
      d->listModel()->insert(row, item);
   }
}

void QListWidget::insertItem(int row, const QString &label)
{
   Q_D(QListWidget);
   d->listModel()->insert(row, new QListWidgetItem(label));
}

void QListWidget::insertItems(int row, const QStringList &labels)
{
   Q_D(QListWidget);
   d->listModel()->insert(row, labels);
}

QListWidgetItem *QListWidget::takeItem(int row)
{
   Q_D(QListWidget);
   if (row < 0 || row >= d->model->rowCount()) {
      return nullptr;
   }

   return d->listModel()->take(row);
}

int QListWidget::count() const
{
   Q_D(const QListWidget);
   return d->model->rowCount();
}

QListWidgetItem *QListWidget::currentItem() const
{
   Q_D(const QListWidget);
   return d->listModel()->at(currentIndex().row());
}

void QListWidget::setCurrentItem(QListWidgetItem *item)
{
   setCurrentRow(row(item));
}

void QListWidget::setCurrentItem(QListWidgetItem *item, QItemSelectionModel::SelectionFlags command)
{
   setCurrentRow(row(item), command);
}

int QListWidget::currentRow() const
{
   return currentIndex().row();
}

void QListWidget::setCurrentRow(int row)
{
   Q_D(QListWidget);

   QModelIndex index = d->listModel()->index(row);

   if (d->selectionMode == SingleSelection) {
      selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
   } else if (d->selectionMode == NoSelection) {
      selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
   } else {
      selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
   }
}

void QListWidget::setCurrentRow(int row, QItemSelectionModel::SelectionFlags command)
{
   Q_D(QListWidget);
   d->selectionModel->setCurrentIndex(d->listModel()->index(row), command);
}

QListWidgetItem *QListWidget::itemAt(const QPoint &p) const
{
   Q_D(const QListWidget);
   return d->listModel()->at(indexAt(p).row());

}

QRect QListWidget::visualItemRect(const QListWidgetItem *item) const
{
   Q_D(const QListWidget);
   QModelIndex index = d->listModel()->index(const_cast<QListWidgetItem *>(item));
   return visualRect(index);
}

void QListWidget::sortItems(Qt::SortOrder order)
{
   Q_D(QListWidget);
   d->sortOrder = order;
   d->listModel()->sort(0, order);
}

void QListWidget::setSortingEnabled(bool enable)
{
   Q_D(QListWidget);
   d->sortingEnabled = enable;
}

bool QListWidget::isSortingEnabled() const
{
   Q_D(const QListWidget);
   return d->sortingEnabled;
}

Qt::SortOrder QListWidget::sortOrder() const
{
   Q_D(const QListWidget);
   return d->sortOrder;
}

void QListWidget::editItem(QListWidgetItem *item)
{
   Q_D(QListWidget);
   edit(d->listModel()->index(item));
}

void QListWidget::openPersistentEditor(QListWidgetItem *item)
{
   Q_D(QListWidget);
   QModelIndex index = d->listModel()->index(item);
   QAbstractItemView::openPersistentEditor(index);
}

void QListWidget::closePersistentEditor(QListWidgetItem *item)
{
   Q_D(QListWidget);
   QModelIndex index = d->listModel()->index(item);
   QAbstractItemView::closePersistentEditor(index);
}

QWidget *QListWidget::itemWidget(QListWidgetItem *item) const
{
   Q_D(const QListWidget);
   QModelIndex index = d->listModel()->index(item);
   return QAbstractItemView::indexWidget(index);
}

void QListWidget::setItemWidget(QListWidgetItem *item, QWidget *widget)
{
   Q_D(QListWidget);
   QModelIndex index = d->listModel()->index(item);
   QAbstractItemView::setIndexWidget(index, widget);
}

bool QListWidget::isItemSelected(const QListWidgetItem *item) const
{
   Q_D(const QListWidget);
   QModelIndex index = d->listModel()->index(const_cast<QListWidgetItem *>(item));
   return selectionModel()->isSelected(index);
}

void QListWidget::setItemSelected(const QListWidgetItem *item, bool select)
{
   Q_D(QListWidget);
   QModelIndex index = d->listModel()->index(const_cast<QListWidgetItem *>(item));

   if (d->selectionMode == SingleSelection) {
      selectionModel()->select(index, select
         ? QItemSelectionModel::ClearAndSelect : QItemSelectionModel::Deselect);

   } else if (d->selectionMode != NoSelection) {
      selectionModel()->select(index, select
         ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
   }

}

QList<QListWidgetItem *> QListWidget::selectedItems() const
{
   Q_D(const QListWidget);

   QModelIndexList indexes = selectionModel()->selectedIndexes();
   QList<QListWidgetItem *> items;

   for (int i = 0; i < indexes.count(); ++i) {
      items.append(d->listModel()->at(indexes.at(i).row()));
   }
   return items;
}

QList<QListWidgetItem *> QListWidget::findItems(const QString &text, Qt::MatchFlags flags) const
{
   Q_D(const QListWidget);

   QModelIndexList indexes = d->listModel()->match(model()->index(0, 0, QModelIndex()),
         Qt::DisplayRole, text, -1, flags);

   QList<QListWidgetItem *> items;
   for (int i = 0; i < indexes.size(); ++i) {
      items.append(d->listModel()->at(indexes.at(i).row()));
   }

   return items;
}

bool QListWidget::isItemHidden(const QListWidgetItem *item) const
{
   return isRowHidden(row(item));
}

void QListWidget::setItemHidden(const QListWidgetItem *item, bool hide)
{
   setRowHidden(row(item), hide);
}

void QListWidget::scrollToItem(const QListWidgetItem *item, QAbstractItemView::ScrollHint hint)
{
   Q_D(QListWidget);

   QModelIndex index = d->listModel()->index(const_cast<QListWidgetItem *>(item));
   QListView::scrollTo(index, hint);
}

void QListWidget::clear()
{
   Q_D(QListWidget);

   selectionModel()->clear();
   d->listModel()->clear();
}

QStringList QListWidget::mimeTypes() const
{
   return d_func()->listModel()->QAbstractListModel::mimeTypes();
}

QMimeData *QListWidget::mimeData(const QList<QListWidgetItem *> &items) const
{
   Q_D(const QListWidget);
   QModelIndexList &cachedIndexes = d->listModel()->cachedIndexes;
   if (cachedIndexes.isEmpty()) {

      for (QListWidgetItem *item : items) {
         cachedIndexes << indexFromItem(item);
      }
      QMimeData *result = d->listModel()->internalMimeData();
      cachedIndexes.clear();
      return result;
   }
   return d->listModel()->internalMimeData();
}

#ifndef QT_NO_DRAGANDDROP

bool QListWidget::dropMimeData(int index, const QMimeData *data, Qt::DropAction action)
{
   QModelIndex idx;

   int row    = index;
   int column = 0;

   if (dropIndicatorPosition() == QAbstractItemView::OnItem) {
      // QAbstractListModel::dropMimeData will overwrite on the index if row == -1 and column == -1
      idx = model()->index(row, column);
      row = -1;
      column = -1;
   }
   return d_func()->listModel()->QAbstractListModel::dropMimeData(data, action, row, column, idx);
}

void QListWidget::dropEvent(QDropEvent *event)
{
   Q_D(QListWidget);
   if (event->source() == this && d->movement != Static) {
      QListView::dropEvent(event);
      return;
   }

   if (event->source() == this && (event->dropAction() == Qt::MoveAction ||
         dragDropMode() == QAbstractItemView::InternalMove)) {
      QModelIndex topIndex;
      int col = -1;
      int row = -1;

      if (d->dropOn(event, &row, &col, &topIndex)) {
         QList<QModelIndex> selIndexes = selectedIndexes();
         QList<QPersistentModelIndex> persIndexes;
         for (int i = 0; i < selIndexes.count(); i++) {
            persIndexes.append(selIndexes.at(i));
         }

         if (persIndexes.contains(topIndex)) {
            return;
         }
         std::sort(persIndexes.begin(), persIndexes.end()); // The dropped items will remain in the same visual order.

         QPersistentModelIndex dropRow = model()->index(row, col, topIndex);

         int r = row == -1 ? count() : (dropRow.row() >= 0 ? dropRow.row() : row);
         for (int i = 0; i < persIndexes.count(); ++i) {
            const QPersistentModelIndex &pIndex = persIndexes.at(i);
            d->listModel()->move(pIndex.row(), r);
            r = pIndex.row() + 1;   // Dropped items are inserted contiguously and in the right order.
         }

         event->accept();
         // Don't want QAbstractItemView to delete it because it was "moved" we already did it
         event->setDropAction(Qt::CopyAction);
      }
   }

   QListView::dropEvent(event);
}

Qt::DropActions QListWidget::supportedDropActions() const
{
   Q_D(const QListWidget);
   return d->listModel()->QAbstractListModel::supportedDropActions() | Qt::MoveAction;
}
#endif // QT_NO_DRAGANDDROP

QList<QListWidgetItem *> QListWidget::items(const QMimeData *data) const
{
   const QListWidgetMimeData *lwd = qobject_cast<const QListWidgetMimeData *>(data);
   if (lwd) {
      return lwd->items;
   }
   return QList<QListWidgetItem *>();
}

QModelIndex QListWidget::indexFromItem(QListWidgetItem *item) const
{
   Q_D(const QListWidget);
   return d->listModel()->index(item);
}

QListWidgetItem *QListWidget::itemFromIndex(const QModelIndex &index) const
{
   Q_D(const QListWidget);
   if (d->isIndexValid(index)) {
      return d->listModel()->at(index.row());
   }
   return nullptr;
}

void QListWidget::setModel(QAbstractItemModel *)
{
   Q_ASSERT(!"QListWidget::setModel() - Changing the model of the QListWidget is not allowed.");
}

bool QListWidget::event(QEvent *e)
{
   return QListView::event(e);
}

void QListWidget::_q_emitItemPressed(const QModelIndex &index)
{
   Q_D(QListWidget);
   d->_q_emitItemPressed(index);
}

void QListWidget::_q_emitItemClicked(const QModelIndex &index)
{
   Q_D(QListWidget);
   d->_q_emitItemClicked(index);
}

void QListWidget::_q_emitItemDoubleClicked(const QModelIndex &index)
{
   Q_D(QListWidget);
   d->_q_emitItemDoubleClicked(index);
}

void QListWidget::_q_emitItemActivated(const QModelIndex &index)
{
   Q_D(QListWidget);
   d->_q_emitItemActivated(index);
}

void QListWidget::_q_emitItemEntered(const QModelIndex &index)
{
   Q_D(QListWidget);
   d->_q_emitItemEntered(index);
}

void QListWidget::_q_emitItemChanged(const QModelIndex &index)
{
   Q_D(QListWidget);
   d->_q_emitItemChanged(index);
}

void QListWidget::_q_emitCurrentItemChanged(const QModelIndex &previous, const QModelIndex &current)
{
   Q_D(QListWidget);
   d->_q_emitCurrentItemChanged(previous, current);
}

void QListWidget::_q_sort()
{
   Q_D(QListWidget);
   d->_q_sort();
}

void QListWidget::_q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
   Q_D(QListWidget);
   d->_q_dataChanged(topLeft, bottomRight);
}

#endif // QT_NO_LISTWIDGET
