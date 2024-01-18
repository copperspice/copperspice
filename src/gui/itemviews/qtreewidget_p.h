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

#ifndef QTREEWIDGET_P_H
#define QTREEWIDGET_P_H

#include <qabstractitemmodel.h>

#include <qpair.h>
#include <qbasictimer.h>
#include <qtreewidget.h>
#include <qtreeview_p.h>
#include <qheaderview.h>

#include <qabstractitemmodel_p.h>

#ifndef QT_NO_TREEWIDGET

class QTreeWidgetItem;
class QTreeWidgetItemIterator;
class QTreeModelPrivate;

class QTreeModel : public QAbstractItemModel
{
   GUI_CS_OBJECT(QTreeModel)

   friend class QTreeWidget;
   friend class QTreeWidgetPrivate;
   friend class QTreeWidgetItem;
   friend class QTreeWidgetItemPrivate;
   friend class QTreeWidgetItemIterator;
   friend class QTreeWidgetItemIteratorPrivate;

 public:
   explicit QTreeModel(int columns = 0, QTreeWidget *parent = nullptr);
   ~QTreeModel();

   inline QTreeWidget *view() const {
      return qobject_cast<QTreeWidget *>(QObject::parent());
   }

   void clear();
   void setColumnCount(int columns);

   QTreeWidgetItem *item(const QModelIndex &index) const;
   void itemChanged(QTreeWidgetItem *item);

   QModelIndex index(const QTreeWidgetItem *item, int column) const;
   QModelIndex index(int row, int column, const QModelIndex &parent) const override;
   QModelIndex parent(const QModelIndex &child) const override;
   int rowCount(const QModelIndex &parent) const override;
   int columnCount(const QModelIndex &parent = QModelIndex()) const override;
   bool hasChildren(const QModelIndex &parent) const override;

   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
   bool setData(const QModelIndex &index, const QVariant &value, int role) override;

   QMap<int, QVariant> itemData(const QModelIndex &index) const override;

   QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
   bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) override;

   Qt::ItemFlags flags(const QModelIndex &index) const override;

   void sort(int column, Qt::SortOrder order) override;
   void ensureSorted(int column, Qt::SortOrder order, int start, int end, const QModelIndex &parent);
   static bool itemLessThan(const QPair<QTreeWidgetItem *, int> &left, const QPair<QTreeWidgetItem *, int> &right);

   static bool itemGreaterThan(const QPair<QTreeWidgetItem *, int> &left, const QPair<QTreeWidgetItem *, int> &right);

   static QList<QTreeWidgetItem *>::iterator sortedInsertionIterator(
      const QList<QTreeWidgetItem *>::iterator &begin,
      const QList<QTreeWidgetItem *>::iterator &end,
      Qt::SortOrder order, QTreeWidgetItem *item);

   bool insertRows(int row, int count, const QModelIndex &) override;
   bool insertColumns(int column, int count, const QModelIndex &) override;

   bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

   // dnd
   QStringList mimeTypes() const override;
   QMimeData *mimeData(const QModelIndexList &indexes) const override;
   bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
   Qt::DropActions supportedDropActions() const override;

   QMimeData *internalMimeData() const;

   inline QModelIndex createIndexFromItem(int row, int col, QTreeWidgetItem *item) const {
      return createIndex(row, col, item);
   }

   struct SkipSorting {
      const QTreeModel *const model;
      const bool previous;
      SkipSorting(const QTreeModel *m) : model(m), previous(model->skipPendingSort) {
         model->skipPendingSort = true;
      }

      ~SkipSorting() {
         model->skipPendingSort = previous;
      }
   };
   friend struct SkipSorting;

 protected:
   QTreeModel(QTreeModelPrivate &, QTreeWidget *parent = nullptr);
   void emitDataChanged(QTreeWidgetItem *item, int column);
   void beginInsertItems(QTreeWidgetItem *parent, int row, int count);
   void endInsertItems();
   void beginRemoveItems(QTreeWidgetItem *parent, int row, int count);
   void endRemoveItems();
   void sortItems(QList<QTreeWidgetItem *> *items, int column, Qt::SortOrder order);
   void timerEvent(QTimerEvent *) override;

 private:
   QTreeWidgetItem *rootItem;
   QTreeWidgetItem *headerItem;

   mutable QModelIndexList cachedIndexes;
   QList<QTreeWidgetItemIterator *> iterators;

   mutable QBasicTimer sortPendingTimer;
   mutable bool skipPendingSort; //while doing internal operation we don't care about sorting
   bool inline executePendingSort() const;

   bool isChanging() const;

   Q_DECLARE_PRIVATE(QTreeModel)

};



class QTreeModelPrivate : public QAbstractItemModelPrivate
{
   Q_DECLARE_PUBLIC(QTreeModel)
};

class QTreeWidgetItemPrivate
{
 public:
   QTreeWidgetItemPrivate(QTreeWidgetItem *item)
      : q(item), disabled(false), selected(false), rowGuess(-1), policy(QTreeWidgetItem::DontShowIndicatorWhenChildless) {}
   void propagateDisabled(QTreeWidgetItem *item);
   void sortChildren(int column, Qt::SortOrder order, bool climb);
   QTreeWidgetItem *q;
   QVariantList display;
   uint disabled : 1;
   uint selected : 1;
   int rowGuess;
   QTreeWidgetItem::ChildIndicatorPolicy policy;
};


inline bool QTreeModel::executePendingSort() const
{
   if (!skipPendingSort && sortPendingTimer.isActive() && !isChanging()) {
      sortPendingTimer.stop();
      int column = view()->header()->sortIndicatorSection();
      Qt::SortOrder order = view()->header()->sortIndicatorOrder();
      QTreeModel *that = const_cast<QTreeModel *>(this);
      that->sort(column, order);
      return true;
   }
   return false;
}

class QTreeWidgetPrivate : public QTreeViewPrivate
{
   friend class QTreeModel;
   Q_DECLARE_PUBLIC(QTreeWidget)

 public:
   QTreeWidgetPrivate() : QTreeViewPrivate(), explicitSortColumn(-1) {}

   inline QTreeModel *treeModel() const {
      return qobject_cast<QTreeModel *>(model);
   }

   inline QModelIndex index(const QTreeWidgetItem *item, int column = 0) const {
      return treeModel()->index(item, column);
   }

   inline QTreeWidgetItem *item(const QModelIndex &index) const {
      return treeModel()->item(index);
   }
   void _q_emitItemPressed(const QModelIndex &index);
   void _q_emitItemClicked(const QModelIndex &index);
   void _q_emitItemDoubleClicked(const QModelIndex &index);
   void _q_emitItemActivated(const QModelIndex &index);
   void _q_emitItemEntered(const QModelIndex &index);
   void _q_emitItemChanged(const QModelIndex &index);
   void _q_emitItemExpanded(const QModelIndex &index);
   void _q_emitItemCollapsed(const QModelIndex &index);
   void _q_emitCurrentItemChanged(const QModelIndex &previous, const QModelIndex &index);
   void _q_sort();
   void _q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
   void _q_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

   // used by QTreeWidgetItem::sortChildren to make sure the column argument is used
   int explicitSortColumn;
};

#endif // QT_NO_TREEWIDGET

#endif // QTREEWIDGET_P_H
