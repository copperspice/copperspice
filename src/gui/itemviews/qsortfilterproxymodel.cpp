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

#include <qsortfilterproxymodel.h>

#ifndef QT_NO_SORTFILTERPROXYMODEL

#include <qalgorithms.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qitemselectionmodel.h>
#include <qpair.h>
#include <qsize.h>
#include <qstringlist.h>

#include <qabstractitemmodel_p.h>
#include <qabstractproxymodel_p.h>

#include <algorithm>

typedef QList<QPair<QModelIndex, QPersistentModelIndex>> QModelIndexPairList;

static inline QSet<int> qVectorToSet(const QVector<int> &vector)
{
   QSet<int> set;
   set.reserve(vector.size());

   for (int i = 0; i < vector.size(); ++i) {
      set << vector.at(i);
   }

   return set;
}

class QSortFilterProxyModelLessThan
{
 public:
   inline QSortFilterProxyModelLessThan(int column, const QModelIndex &parent,
      const QAbstractItemModel *source, const QSortFilterProxyModel *proxy)
         : sort_column(column), source_parent(parent), source_model(source), proxy_model(proxy)
   {
   }

   inline bool operator()(int r1, int r2) const {
      QModelIndex i1 = source_model->index(r1, sort_column, source_parent);
      QModelIndex i2 = source_model->index(r2, sort_column, source_parent);
      return proxy_model->lessThan(i1, i2);
   }

 private:
   int sort_column;
   QModelIndex source_parent;
   const QAbstractItemModel *source_model;
   const QSortFilterProxyModel *proxy_model;
};

class QSortFilterProxyModelGreaterThan
{
 public:
   inline QSortFilterProxyModelGreaterThan(int column, const QModelIndex &parent,
      const QAbstractItemModel *source, const QSortFilterProxyModel *proxy)
      : sort_column(column), source_parent(parent), source_model(source), proxy_model(proxy)
   {
   }

   inline bool operator()(int r1, int r2) const {
      QModelIndex i1 = source_model->index(r1, sort_column, source_parent);
      QModelIndex i2 = source_model->index(r2, sort_column, source_parent);
      return proxy_model->lessThan(i2, i1);
   }

 private:
   int sort_column;
   QModelIndex source_parent;
   const QAbstractItemModel *source_model;
   const QSortFilterProxyModel *proxy_model;
};


// this struct is used to store which rows are removed
// between a call to rowsAboutToBeRemoved and rowsRemoved
// it avoids re-adding rows to the mapping that are currently being removed

struct QRowsRemoval {
   QRowsRemoval(const QModelIndex &parent_source, int start, int end)
      : parent_source(parent_source), start(start), end(end)
   {
   }

   QRowsRemoval() : start(-1), end(-1)
   {
   }

   bool contains(QModelIndex parent, int row) {
      do {
         if (parent == parent_source) {
            return row >= start && row <= end;
         }

         row = parent.row();
         parent = parent.parent();

      } while (row >= 0);

      return false;
   }

 private:
   QModelIndex parent_source;
   int start;
   int end;
};

class QSortFilterProxyModelPrivate : public QAbstractProxyModelPrivate
{
   Q_DECLARE_PUBLIC(QSortFilterProxyModel)

 public:
   struct Mapping {
      QVector<int> source_rows;
      QVector<int> source_columns;
      QVector<int> proxy_rows;
      QVector<int> proxy_columns;
      QVector<QModelIndex> mapped_children;
      QHash<QModelIndex, Mapping *>::const_iterator map_iter;
   };

   mutable QHash<QModelIndex, Mapping *> source_index_mapping;

   int source_sort_column;
   int proxy_sort_column;
   Qt::SortOrder sort_order;
   Qt::CaseSensitivity sort_casesensitivity;
   int sort_role;
   bool sort_localeaware;

   int filter_column;

   QRegularExpression filter_regexp;
   int filter_role;

   bool dynamic_sortfilter;
   QRowsRemoval itemsBeingRemoved;

   QModelIndexPairList saved_persistent_indexes;

   QHash<QModelIndex, Mapping *>::const_iterator create_mapping(const QModelIndex &sourceParent) const;

   QModelIndex proxy_to_source(const QModelIndex &proxyIndex) const;
   QModelIndex source_to_proxy(const QModelIndex &sourceIndex) const;
   bool can_create_mapping(const QModelIndex &sourceParent) const;

   void remove_from_mapping(const QModelIndex &sourceParent);

   inline QHash<QModelIndex, Mapping *>::const_iterator index_to_iterator(
      const QModelIndex &proxy_index) const {
      Q_ASSERT(proxy_index.isValid());
      Q_ASSERT(proxy_index.model() == q_func());
      const void *p = proxy_index.internalPointer();
      Q_ASSERT(p);

      QHash<QModelIndex, Mapping *>::const_iterator it = static_cast<const Mapping *>(p)->map_iter;
      Q_ASSERT(it != source_index_mapping.constEnd());
      Q_ASSERT(it.value());

      return it;
   }

   inline QModelIndex create_index(int row, int column,
      QHash<QModelIndex, Mapping *>::const_iterator it) const {
      return q_func()->createIndex(row, column, *it);
   }

   void _q_sourceDataChanged(const QModelIndex &source_top_left,
      const QModelIndex &source_bottom_right, const QVector<int> &roles);

   void _q_sourceHeaderDataChanged(Qt::Orientation orientation, int start, int end);

   void _q_sourceAboutToBeReset();
   void _q_sourceReset();

   void _q_sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents,
      QAbstractItemModel::LayoutChangeHint hint);

   void _q_sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents,
      QAbstractItemModel::LayoutChangeHint hint);

   void _q_sourceRowsAboutToBeInserted(const QModelIndex &sourceParent, int start, int end);
   void _q_sourceRowsInserted(const QModelIndex &sourceParent, int start, int end);
   void _q_sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end);
   void _q_sourceRowsRemoved(const QModelIndex &sourceParent, int start, int end);

   void _q_sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
      const QModelIndex &destinationParent, int destinationStart);

   void _q_sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
      const QModelIndex &destinationParent, int destinationStart);

   void _q_sourceColumnsAboutToBeInserted(const QModelIndex &sourceRowsAboutToBeInserted, int start, int end);
   void _q_sourceColumnsInserted(const QModelIndex &sourceRowsAboutToBeInserted, int start, int end);
   void _q_sourceColumnsAboutToBeRemoved(const QModelIndex &sourceRowsAboutToBeInserted, int start, int end);
   void _q_sourceColumnsRemoved(const QModelIndex &sourceRowsAboutToBeInserted, int start, int end);

   void _q_sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
      const QModelIndex &destinationParent, int destinationStart);

   void _q_sourceColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
      const QModelIndex &destinationParent, int destinationStart);

   void _q_clearMapping();

   void sort();
   bool update_source_sort_column();

   void sort_source_rows(QVector<int> &source_rows, const QModelIndex &sourceParent) const;

   QVector<QPair<int, QVector<int >>> proxy_intervals_for_source_items_to_add(
      const QVector<int> &proxy_to_source, const QVector<int> &source_items,
      const QModelIndex &sourceParent, Qt::Orientation orient) const;

   QVector<QPair<int, int >> proxy_intervals_for_source_items(
      const QVector<int> &source_to_proxy, const QVector<int> &source_items) const;

   void insert_source_items(QVector<int> &source_to_proxy, QVector<int> &proxy_to_source,
      const QVector<int> &source_items, const QModelIndex &sourceParent,
      Qt::Orientation orient, bool emit_signal = true);

   void remove_source_items(QVector<int> &source_to_proxy, QVector<int> &proxy_to_source,
      const QVector<int> &source_items, const QModelIndex &sourceParent,
      Qt::Orientation orient, bool emit_signal = true);

   void remove_proxy_interval(QVector<int> &source_to_proxy, QVector<int> &proxy_to_source,
      int proxy_start, int proxy_end, const QModelIndex &proxy_parent,
      Qt::Orientation orient, bool emit_signal = true);

   void build_source_to_proxy_mapping(
      const QVector<int> &proxy_to_source, QVector<int> &source_to_proxy) const;

   void source_items_inserted(const QModelIndex &, int start, int end, Qt::Orientation orient);

   void source_items_about_to_be_removed(const QModelIndex &sourceParent,
      int start, int end, Qt::Orientation orient);

   void source_items_removed(const QModelIndex &sourceParent,
      int start, int end, Qt::Orientation orient);

   void proxy_item_range(const QVector<int> &source_to_proxy, const QVector<int> &source_items,
      int &proxy_low, int &proxy_high) const;

   QModelIndexPairList store_persistent_indexes();
   void update_persistent_indexes(const QModelIndexPairList &source_indexes);

   void filter_about_to_be_changed(const QModelIndex &sourceParent = QModelIndex());
   void filter_changed(const QModelIndex &sourceParent = QModelIndex());

   QSet<int> handle_filter_changed(QVector<int> &source_to_proxy, QVector<int> &proxy_to_source,
      const QModelIndex &sourceParent, Qt::Orientation orient);

   void updateChildrenMapping(const QModelIndex &sourceParent, Mapping *parent_mapping,
      Qt::Orientation orient, int start, int end, int delta_item_count, bool remove);

   void _q_sourceModelDestroyed() override;
};

typedef QHash<QModelIndex, QSortFilterProxyModelPrivate::Mapping *> IndexMap;

void QSortFilterProxyModelPrivate::_q_sourceModelDestroyed()
{
   QAbstractProxyModelPrivate::_q_sourceModelDestroyed();
   qDeleteAll(source_index_mapping);
   source_index_mapping.clear();
}

void QSortFilterProxyModelPrivate::remove_from_mapping(const QModelIndex &sourceParent)
{
   if (Mapping *m = source_index_mapping.take(sourceParent)) {
      for (int i = 0; i < m->mapped_children.size(); ++i) {
         remove_from_mapping(m->mapped_children.at(i));
      }

      delete m;
   }
}

void QSortFilterProxyModelPrivate::_q_clearMapping()
{
   // store the persistent indexes
   QModelIndexPairList source_indexes = store_persistent_indexes();

   qDeleteAll(source_index_mapping);
   source_index_mapping.clear();
   if (dynamic_sortfilter && update_source_sort_column()) {
      //update_source_sort_column might have created wrong mapping so we have to clear it again
      qDeleteAll(source_index_mapping);
      source_index_mapping.clear();
   }

   // update the persistent indexes
   update_persistent_indexes(source_indexes);
}

IndexMap::const_iterator QSortFilterProxyModelPrivate::create_mapping(const QModelIndex &sourceParent) const
{
   Q_Q(const QSortFilterProxyModel);

   IndexMap::const_iterator it = source_index_mapping.constFind(sourceParent);
   if (it != source_index_mapping.constEnd()) {
      // mapped already
      return it;
   }

   Mapping *m = new Mapping;

   int source_rows = model->rowCount(sourceParent);
   m->source_rows.reserve(source_rows);

   for (int i = 0; i < source_rows; ++i) {
      if (q->filterAcceptsRow(i, sourceParent)) {
         m->source_rows.append(i);
      }
   }
   int source_cols = model->columnCount(sourceParent);
   m->source_columns.reserve(source_cols);

   for (int i = 0; i < source_cols; ++i) {
      if (q->filterAcceptsColumn(i, sourceParent)) {
         m->source_columns.append(i);
      }
   }

   sort_source_rows(m->source_rows, sourceParent);
   m->proxy_rows.resize(source_rows);
   build_source_to_proxy_mapping(m->source_rows, m->proxy_rows);
   m->proxy_columns.resize(source_cols);
   build_source_to_proxy_mapping(m->source_columns, m->proxy_columns);

   it = IndexMap::const_iterator(source_index_mapping.insert(sourceParent, m));
   m->map_iter = it;

   if (sourceParent.isValid()) {
      QModelIndex source_grand_parent = sourceParent.parent();
      IndexMap::const_iterator it2 = create_mapping(source_grand_parent);
      Q_ASSERT(it2 != source_index_mapping.constEnd());
      it2.value()->mapped_children.append(sourceParent);
   }

   Q_ASSERT(it != source_index_mapping.constEnd());
   Q_ASSERT(it.value());

   return it;
}

QModelIndex QSortFilterProxyModelPrivate::proxy_to_source(const QModelIndex &proxy_index) const
{
   if (! proxy_index.isValid()) {
      //  may want to be able to set a root index later
      return QModelIndex();
   }

   if (proxy_index.model() != q_func()) {
      qWarning("QSortFilterProxyModel:QListView::setBatchSize() Index from a different model passed to mapToSource");
      return QModelIndex();
   }

   IndexMap::const_iterator it = index_to_iterator(proxy_index);
   Mapping *m = it.value();

   if ((proxy_index.row() >= m->source_rows.size()) || (proxy_index.column() >= m->source_columns.size())) {
      return QModelIndex();
   }

   int source_row = m->source_rows.at(proxy_index.row());
   int source_col = m->source_columns.at(proxy_index.column());

   return model->index(source_row, source_col, it.key());
}

QModelIndex QSortFilterProxyModelPrivate::source_to_proxy(const QModelIndex &source_index) const
{
   if (! source_index.isValid()) {
      // may want to be able to set a root index later
      return QModelIndex();
   }

   if (source_index.model() != model) {
      qWarning("QSortFilterProxyModel::source_to_proxy() Index from a different model passed to mapFromSource");

      return QModelIndex();
   }

   QModelIndex sourceParent    = source_index.parent();
   IndexMap::const_iterator it = create_mapping(sourceParent);
   Mapping *m = it.value();

   if ((source_index.row() >= m->proxy_rows.size()) || (source_index.column() >= m->proxy_columns.size())) {
      return QModelIndex();
   }

   int proxy_row = m->proxy_rows.at(source_index.row());
   int proxy_column = m->proxy_columns.at(source_index.column());

   if (proxy_row == -1 || proxy_column == -1) {
      return QModelIndex();
   }

   return create_index(proxy_row, proxy_column, it);
}

bool QSortFilterProxyModelPrivate::can_create_mapping(const QModelIndex &sourceParent) const
{
   if (sourceParent.isValid()) {
      QModelIndex source_grand_parent = sourceParent.parent();
      IndexMap::const_iterator it = source_index_mapping.constFind(source_grand_parent);

      if (it == source_index_mapping.constEnd()) {
         // do not have mapping for the grand parent
         return false;
      }

      Mapping *gm = it.value();

      if (gm->proxy_rows.at(sourceParent.row()) == -1 ||
         gm->proxy_columns.at(sourceParent.column()) == -1) {
         // Don't care, since parent is filtered
         return false;
      }
   }

   return true;
}

void QSortFilterProxyModelPrivate::sort()
{
   Q_Q(QSortFilterProxyModel);

   emit q->layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
   QModelIndexPairList source_indexes = store_persistent_indexes();
   IndexMap::const_iterator it = source_index_mapping.constBegin();

   for (; it != source_index_mapping.constEnd(); ++it) {
      QModelIndex sourceParent = it.key();

      Mapping *m = it.value();
      sort_source_rows(m->source_rows, sourceParent);
      build_source_to_proxy_mapping(m->source_rows, m->proxy_rows);
   }

   update_persistent_indexes(source_indexes);
   emit q->layoutChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
}

bool QSortFilterProxyModelPrivate::update_source_sort_column()
{
   int old_source_sort_column = source_sort_column;

   if (proxy_sort_column == -1) {
      source_sort_column = -1;

   } else {
      // cannot use index mapping here because in case of a still-empty
      // proxy model there's no valid proxy index we could map to source.
      // So always use the root mapping directly instead.

      Mapping *m = create_mapping(QModelIndex()).value();

      if (proxy_sort_column < m->source_columns.size()) {
         source_sort_column = m->source_columns.at(proxy_sort_column);
      } else {
         source_sort_column = -1;
      }
   }

   return old_source_sort_column != source_sort_column;
}

void QSortFilterProxyModelPrivate::sort_source_rows(QVector<int> &source_rows, const QModelIndex &sourceParent) const
{
   Q_Q(const QSortFilterProxyModel);

   if (source_sort_column >= 0) {
      if (sort_order == Qt::AscendingOrder) {
         QSortFilterProxyModelLessThan lt(source_sort_column, sourceParent, model, q);
         std::stable_sort(source_rows.begin(), source_rows.end(), lt);
      } else {
         QSortFilterProxyModelGreaterThan gt(source_sort_column, sourceParent, model, q);
         std::stable_sort(source_rows.begin(), source_rows.end(), gt);
      }

   } else {
      // restore the source model order
      std::stable_sort(source_rows.begin(), source_rows.end());
   }
}

QVector<QPair<int, int >> QSortFilterProxyModelPrivate::proxy_intervals_for_source_items(
      const QVector<int> &source_to_proxy, const QVector<int> &source_items) const
{
   QVector<QPair<int, int>> proxy_intervals;

   if (source_items.isEmpty()) {
      return proxy_intervals;
   }

   int source_items_index = 0;
   while (source_items_index < source_items.size()) {
      int first_proxy_item = source_to_proxy.at(source_items.at(source_items_index));
      Q_ASSERT(first_proxy_item != -1);

      int last_proxy_item = first_proxy_item;
      ++source_items_index;

      // Find end of interval
      while ((source_items_index < source_items.size())
         && (source_to_proxy.at(source_items.at(source_items_index)) == last_proxy_item + 1)) {
         ++last_proxy_item;
         ++source_items_index;
      }

      // Add interval to result
      proxy_intervals.append(QPair<int, int>(first_proxy_item, last_proxy_item));
   }

   std::stable_sort(proxy_intervals.begin(), proxy_intervals.end());
   return proxy_intervals;
}

void QSortFilterProxyModelPrivate::remove_source_items(QVector<int> &source_to_proxy,
      QVector<int> &proxy_to_source, const QVector<int> &source_items,
      const QModelIndex &sourceParent, Qt::Orientation orient, bool emit_signal)
{
   Q_Q(QSortFilterProxyModel);

   QModelIndex proxy_parent = q->mapFromSource(sourceParent);

   if (! proxy_parent.isValid() && sourceParent.isValid()) {
      return;   // nothing to do (already removed)
   }

   QVector<QPair<int, int>> proxy_intervals;
   proxy_intervals = proxy_intervals_for_source_items(source_to_proxy, source_items);

   for (int i = proxy_intervals.size() - 1; i >= 0; --i) {
      QPair<int, int> interval = proxy_intervals.at(i);
      int proxy_start = interval.first;
      int proxy_end = interval.second;
      remove_proxy_interval(source_to_proxy, proxy_to_source, proxy_start, proxy_end,
         proxy_parent, orient, emit_signal);
   }
}

void QSortFilterProxyModelPrivate::remove_proxy_interval(QVector<int> &source_to_proxy,
      QVector<int> &proxy_to_source, int proxy_start, int proxy_end,
      const QModelIndex &proxy_parent, Qt::Orientation orient, bool emit_signal)
{
   Q_Q(QSortFilterProxyModel);

   if (emit_signal) {
      if (orient == Qt::Vertical) {
         q->beginRemoveRows(proxy_parent, proxy_start, proxy_end);
      } else {
         q->beginRemoveColumns(proxy_parent, proxy_start, proxy_end);
      }
   }

   // Remove items from proxy-to-source mapping
   proxy_to_source.remove(proxy_start, proxy_end - proxy_start + 1);

   build_source_to_proxy_mapping(proxy_to_source, source_to_proxy);

   if (emit_signal) {
      if (orient == Qt::Vertical) {
         q->endRemoveRows();
      } else {
         q->endRemoveColumns();
      }
   }
}

QVector<QPair<int, QVector<int >>> QSortFilterProxyModelPrivate::proxy_intervals_for_source_items_to_add(
   const QVector<int> &proxy_to_source, const QVector<int> &source_items,
   const QModelIndex &sourceParent, Qt::Orientation orient) const
{
   Q_Q(const QSortFilterProxyModel);

   QVector<QPair<int, QVector<int>>> proxy_intervals;

   if (source_items.isEmpty()) {
      return proxy_intervals;
   }

   int proxy_low  = 0;
   int proxy_item = 0;
   int source_items_index = 0;

   QVector<int> source_items_in_interval;
   bool compare = (orient == Qt::Vertical && source_sort_column >= 0 && dynamic_sortfilter);

   while (source_items_index < source_items.size()) {
      source_items_in_interval.clear();
      int first_new_source_item = source_items.at(source_items_index);
      source_items_in_interval.append(first_new_source_item);
      ++source_items_index;

      // Find proxy item at which insertion should be started
      int proxy_high = proxy_to_source.size() - 1;
      QModelIndex i1 = compare ? model->index(first_new_source_item, source_sort_column, sourceParent) : QModelIndex();

      while (proxy_low <= proxy_high) {
         proxy_item = (proxy_low + proxy_high) / 2;

         if (compare) {
            QModelIndex i2 = model->index(proxy_to_source.at(proxy_item), source_sort_column, sourceParent);

            if ((sort_order == Qt::AscendingOrder) ? q->lessThan(i1, i2) : q->lessThan(i2, i1)) {
               proxy_high = proxy_item - 1;
            } else {
               proxy_low = proxy_item + 1;
            }

         } else {
            if (first_new_source_item < proxy_to_source.at(proxy_item)) {
               proxy_high = proxy_item - 1;
            } else {
               proxy_low = proxy_item + 1;
            }
         }
      }
      proxy_item = proxy_low;

      // Find the sequence of new source items that should be inserted here
      if (proxy_item >= proxy_to_source.size()) {
         for ( ; source_items_index < source_items.size(); ++source_items_index) {
            source_items_in_interval.append(source_items.at(source_items_index));
         }

      } else {
         i1 = compare ? model->index(proxy_to_source.at(proxy_item), source_sort_column, sourceParent) : QModelIndex();

         for ( ; source_items_index < source_items.size(); ++source_items_index) {
            int new_source_item = source_items.at(source_items_index);

            if (compare) {
               QModelIndex i2 = model->index(new_source_item, source_sort_column, sourceParent);

               if ((sort_order == Qt::AscendingOrder) ? q->lessThan(i1, i2) : q->lessThan(i2, i1)) {
                  break;
               }
            } else {
               if (proxy_to_source.at(proxy_item) < new_source_item) {
                  break;
               }
            }
            source_items_in_interval.append(new_source_item);
         }
      }

      // Add interval to result
      proxy_intervals.append(QPair<int, QVector<int>>(proxy_item, source_items_in_interval));
   }

   return proxy_intervals;
}

void QSortFilterProxyModelPrivate::insert_source_items(QVector<int> &source_to_proxy,
      QVector<int> &proxy_to_source, const QVector<int> &source_items,
      const QModelIndex &sourceParent, Qt::Orientation orient, bool emit_signal)
{
   Q_Q(QSortFilterProxyModel);

   QModelIndex proxy_parent = q->mapFromSource(sourceParent);

   if (! proxy_parent.isValid() && sourceParent.isValid()) {
      return;   // nothing to do (sourceParent is not mapped)
   }

   QVector<QPair<int, QVector<int>>> proxy_intervals;
   proxy_intervals = proxy_intervals_for_source_items_to_add(
         proxy_to_source, source_items, sourceParent, orient);

   for (int i = proxy_intervals.size() - 1; i >= 0; --i) {
      QPair<int, QVector<int>> interval = proxy_intervals.at(i);
      int proxy_start = interval.first;
      QVector<int> source_items = interval.second;
      int proxy_end = proxy_start + source_items.size() - 1;

      if (emit_signal) {
         if (orient == Qt::Vertical) {
            q->beginInsertRows(proxy_parent, proxy_start, proxy_end);
         } else {
            q->beginInsertColumns(proxy_parent, proxy_start, proxy_end);
         }
      }

      for (int i = 0; i < source_items.size(); ++i) {
         proxy_to_source.insert(proxy_start + i, source_items.at(i));
      }

      build_source_to_proxy_mapping(proxy_to_source, source_to_proxy);

      if (emit_signal) {
         if (orient == Qt::Vertical) {
            q->endInsertRows();
         } else {
            q->endInsertColumns();
         }
      }
   }
}

void QSortFilterProxyModelPrivate::source_items_inserted(const QModelIndex &sourceParent, int start, int end,
      Qt::Orientation orient)
{
   Q_Q(QSortFilterProxyModel);

   if ((start < 0) || (end < 0)) {
      return;
   }

   IndexMap::const_iterator it = source_index_mapping.constFind(sourceParent);

   if (it == source_index_mapping.constEnd()) {
      if (!can_create_mapping(sourceParent)) {
         return;
      }

      it = create_mapping(sourceParent);
      Mapping *m = it.value();
      QModelIndex proxy_parent = q->mapFromSource(sourceParent);

      if (m->source_rows.count() > 0) {
         q->beginInsertRows(proxy_parent, 0, m->source_rows.count() - 1);
         q->endInsertRows();
      }

      if (m->source_columns.count() > 0) {
         q->beginInsertColumns(proxy_parent, 0, m->source_columns.count() - 1);
         q->endInsertColumns();
      }

      return;
   }

   Mapping *m = it.value();
   QVector<int> &source_to_proxy = (orient == Qt::Vertical) ? m->proxy_rows  : m->proxy_columns;
   QVector<int> &proxy_to_source = (orient == Qt::Vertical) ? m->source_rows : m->source_columns;

   int delta_item_count = end - start + 1;
   int old_item_count   = source_to_proxy.size();

   updateChildrenMapping(sourceParent, m, orient, start, end, delta_item_count, false);

   // Expand source-to-proxy mapping to account for new items
   if (start < 0 || start > source_to_proxy.size()) {
      qWarning("QSortFilterProxyModel::source_items_inserted() Inserted rows rejected by source model");
      remove_from_mapping(sourceParent);
      return;
   }

   source_to_proxy.insert(start, delta_item_count, -1);

   if (start < old_item_count) {
      // Adjust existing "stale" indexes in proxy-to-source mapping
      int proxy_count = proxy_to_source.size();

      for (int proxy_item = 0; proxy_item < proxy_count; ++proxy_item) {
         int source_item = proxy_to_source.at(proxy_item);
         if (source_item >= start) {
            proxy_to_source.replace(proxy_item, source_item + delta_item_count);
         }
      }
      build_source_to_proxy_mapping(proxy_to_source, source_to_proxy);
   }

   // Figure out which items to add to mapping based on filter
   QVector<int> source_items;

   for (int i = start; i <= end; ++i) {
      bool ok;

      if (orient == Qt::Vertical) {
         ok = q->filterAcceptsRow(i, sourceParent);
      } else {
         ok = q->filterAcceptsColumn(i, sourceParent);
      }

      if (ok) {
         source_items.append(i);
      }
   }

   if (model->rowCount(sourceParent) == delta_item_count) {
      // Items were inserted where there were none before.
      // If it was new rows make sure to create mappings for columns so that a
      // valid mapping can be retrieved later and vice-versa.

      QVector<int> &orthogonal_proxy_to_source = (orient == Qt::Horizontal) ? m->source_rows : m->source_columns;
      QVector<int> &orthogonal_source_to_proxy = (orient == Qt::Horizontal) ? m->proxy_rows : m->proxy_columns;

      if (orthogonal_source_to_proxy.isEmpty()) {
         const int ortho_end = (orient == Qt::Horizontal) ? model->rowCount(sourceParent) : model->columnCount(sourceParent);

         orthogonal_source_to_proxy.resize(ortho_end);

         for (int ortho_item = 0; ortho_item < ortho_end; ++ortho_item) {
            if ((orient == Qt::Horizontal) ? q->filterAcceptsRow(ortho_item, sourceParent)
               : q->filterAcceptsColumn(ortho_item, sourceParent)) {
               orthogonal_proxy_to_source.append(ortho_item);
            }
         }

         if (orient == Qt::Horizontal) {
            // reacting to columnsInserted, just inserted new rows, sort now
            sort_source_rows(orthogonal_proxy_to_source, sourceParent);
         }

         build_source_to_proxy_mapping(orthogonal_proxy_to_source, orthogonal_source_to_proxy);
      }
   }

   // Sort and insert the items
   if (orient == Qt::Vertical) {
      // Only sort rows
      sort_source_rows(source_items, sourceParent);
   }

   insert_source_items(source_to_proxy, proxy_to_source, source_items, sourceParent, orient);
}

void QSortFilterProxyModelPrivate::source_items_about_to_be_removed(
   const QModelIndex &sourceParent, int start, int end, Qt::Orientation orient)
{
   if ((start < 0) || (end < 0)) {
      return;
   }

   IndexMap::const_iterator it = source_index_mapping.constFind(sourceParent);
   if (it == source_index_mapping.constEnd()) {
      // do not have mapping for this index
      return;
   }

   Mapping *m = it.value();
   QVector<int> &source_to_proxy = (orient == Qt::Vertical) ? m->proxy_rows : m->proxy_columns;
   QVector<int> &proxy_to_source = (orient == Qt::Vertical) ? m->source_rows : m->source_columns;

   // figure out which items to remove
   QVector<int> source_items_to_remove;
   int proxy_count = proxy_to_source.size();

   for (int proxy_item = 0; proxy_item < proxy_count; ++proxy_item) {
      int source_item = proxy_to_source.at(proxy_item);

      if ((source_item >= start) && (source_item <= end)) {
         source_items_to_remove.append(source_item);
      }
   }

   remove_source_items(source_to_proxy, proxy_to_source, source_items_to_remove,
      sourceParent, orient);
}

void QSortFilterProxyModelPrivate::source_items_removed(
   const QModelIndex &sourceParent, int start, int end, Qt::Orientation orient)
{
   if ((start < 0) || (end < 0)) {
      return;
   }

   IndexMap::const_iterator it = source_index_mapping.constFind(sourceParent);
   if (it == source_index_mapping.constEnd()) {
      // no mapping for this index
      return;
   }

   Mapping *m = it.value();
   QVector<int> &source_to_proxy = (orient == Qt::Vertical) ? m->proxy_rows  : m->proxy_columns;
   QVector<int> &proxy_to_source = (orient == Qt::Vertical) ? m->source_rows : m->source_columns;

   if (end >= source_to_proxy.size()) {
      end = source_to_proxy.size() - 1;
   }

   // Shrink the source-to-proxy mapping to reflect the new item count
   int delta_item_count = end - start + 1;
   source_to_proxy.remove(start, delta_item_count);

   int proxy_count = proxy_to_source.size();
   if (proxy_count > source_to_proxy.size()) {
      // mapping is in an inconsistent state -- redo the whole mapping
      qWarning("QSortFilterProxyModel::source_items_removed() Inconsistent changes reported by source model");

      Q_Q(QSortFilterProxyModel);
      q->beginResetModel();
      remove_from_mapping(sourceParent);
      q->endResetModel();

      return;
   }

   // Adjust "stale" indexes in proxy-to-source mapping
   for (int proxy_item = 0; proxy_item < proxy_count; ++proxy_item) {
      int source_item = proxy_to_source.at(proxy_item);
      if (source_item >= start) {
         Q_ASSERT(source_item - delta_item_count >= 0);
         proxy_to_source.replace(proxy_item, source_item - delta_item_count);
      }
   }
   build_source_to_proxy_mapping(proxy_to_source, source_to_proxy);

   updateChildrenMapping(sourceParent, m, orient, start, end, delta_item_count, true);

}

void QSortFilterProxyModelPrivate::updateChildrenMapping(const QModelIndex &sourceParent,
      Mapping *parent_mapping, Qt::Orientation orient, int start, int end,
      int delta_item_count, bool remove)
{
   // see if any mapped children should be (re)moved
   QVector<QPair<QModelIndex, Mapping *>> moved_source_index_mappings;
   QVector<QModelIndex>::iterator it2 = parent_mapping->mapped_children.begin();

   for ( ; it2 != parent_mapping->mapped_children.end();) {
      const QModelIndex source_child_index = *it2;

      const int pos = (orient == Qt::Vertical) ? source_child_index.row() : source_child_index.column();

      if (pos < start) {
         // not affected
         ++it2;

      } else if (remove && pos <= end) {
         // in the removed interval
         it2 = parent_mapping->mapped_children.erase(it2);
         remove_from_mapping(source_child_index);

      } else {
         // below the removed items -- recompute the index
         QModelIndex new_index;
         const int newpos = remove ? pos - delta_item_count : pos + delta_item_count;

         if (orient == Qt::Vertical) {
            new_index = model->index(newpos, source_child_index.column(), sourceParent);

         } else {
            new_index = model->index(source_child_index.row(), newpos, sourceParent);
         }

         *it2 = new_index;
         ++it2;

         // update mapping
         Mapping *cm = source_index_mapping.take(source_child_index);
         Q_ASSERT(cm);

         // do not reinsert right away, because the new index might be identical with another, old index
         moved_source_index_mappings.append(QPair<QModelIndex, Mapping *>(new_index, cm));
      }
   }

   // reinsert moved into source

   for (auto &item : moved_source_index_mappings) {
      auto tmp_iter = source_index_mapping.insert(item);
      item.second->map_iter = tmp_iter;
   }
}

void QSortFilterProxyModelPrivate::proxy_item_range(const QVector<int> &source_to_proxy,
      const QVector<int> &source_items, int &proxy_low, int &proxy_high) const
{
   proxy_low  = INT_MAX;
   proxy_high = INT_MIN;

   for (int i = 0; i < source_items.count(); ++i) {
      int proxy_item = source_to_proxy.at(source_items.at(i));

      Q_ASSERT(proxy_item != -1);

      if (proxy_item < proxy_low) {
         proxy_low = proxy_item;
      }
      if (proxy_item > proxy_high) {
         proxy_high = proxy_item;
      }
   }
}

void QSortFilterProxyModelPrivate::build_source_to_proxy_mapping(
   const QVector<int> &proxy_to_source, QVector<int> &source_to_proxy) const
{
   source_to_proxy.fill(-1);
   int proxy_count = proxy_to_source.size();

   for (int i = 0; i < proxy_count; ++i) {
      source_to_proxy[proxy_to_source.at(i)] = i;
   }
}

QModelIndexPairList QSortFilterProxyModelPrivate::store_persistent_indexes()
{
   Q_Q(QSortFilterProxyModel);

   QModelIndexPairList source_indexes;

   for (QPersistentModelIndexData *data : persistent.m_indexes) {
      QModelIndex proxy_index = data->index;
      QModelIndex source_index = q->mapToSource(proxy_index);
      source_indexes.append(qMakePair(proxy_index, QPersistentModelIndex(source_index)));
   }

   return source_indexes;
}

void QSortFilterProxyModelPrivate::update_persistent_indexes(const QModelIndexPairList &source_indexes)
{
   Q_Q(QSortFilterProxyModel);

   QModelIndexList from, to;

   for (int i = 0; i < source_indexes.count(); ++i) {
      QModelIndex source_index = source_indexes.at(i).second;
      QModelIndex old_proxy_index = source_indexes.at(i).first;
      create_mapping(source_index.parent());
      QModelIndex proxy_index = q->mapFromSource(source_index);
      from << old_proxy_index;
      to << proxy_index;
   }

   q->changePersistentIndexList(from, to);
}

void QSortFilterProxyModelPrivate::filter_about_to_be_changed(const QModelIndex &sourceParent)
{
   if (! filter_regexp.pattern().isEmpty() &&
            source_index_mapping.constFind(sourceParent) == source_index_mapping.constEnd()) {
      create_mapping(sourceParent);
   }
}

void QSortFilterProxyModelPrivate::filter_changed(const QModelIndex &sourceParent)
{
   IndexMap::const_iterator it = source_index_mapping.constFind(sourceParent);
   if (it == source_index_mapping.constEnd()) {
      return;
   }

   Mapping *m = it.value();
   QSet<int> rows_removed = handle_filter_changed(m->proxy_rows, m->source_rows, sourceParent, Qt::Vertical);
   QSet<int> columns_removed = handle_filter_changed(m->proxy_columns, m->source_columns, sourceParent, Qt::Horizontal);

   // We need to iterate over a copy of m->mapped_children because otherwise it may be changed by other code, invalidating
   // the iterator it2.
   // The m->mapped_children vector can be appended to with indexes which are no longer filtered
   // out (in create_mapping) when this function recurses for child indexes.
   const QVector<QModelIndex> mappedChildren = m->mapped_children;
   QVector<int> indexesToRemove;
   for (int i = 0; i < mappedChildren.size(); ++i) {
      const QModelIndex source_child_index = mappedChildren.at(i);
      if (rows_removed.contains(source_child_index.row()) || columns_removed.contains(source_child_index.column())) {
         indexesToRemove.push_back(i);
         remove_from_mapping(source_child_index);
      } else {
         filter_changed(source_child_index);
      }
   }
   QVector<int>::const_iterator removeIt = indexesToRemove.constEnd();
   const QVector<int>::const_iterator removeBegin = indexesToRemove.constBegin();

   // We can't just remove these items from mappedChildren while iterating above and then
   // do something like m->mapped_children = mappedChildren, because mapped_children might
   // be appended to in create_mapping, and we would lose those new items.
   // Because they are always appended in create_mapping, we can still remove them by
   // position here.
   while (removeIt != removeBegin) {
      --removeIt;
      m->mapped_children.remove(*removeIt);
   }
}

QSet<int> QSortFilterProxyModelPrivate::handle_filter_changed(QVector<int> &source_to_proxy,
      QVector<int> &proxy_to_source, const QModelIndex &sourceParent, Qt::Orientation orient)
{
   Q_Q(QSortFilterProxyModel);

   // Figure out which mapped items to remove
   QVector<int> source_items_remove;

   for (int i = 0; i < proxy_to_source.count(); ++i) {
      const int source_item = proxy_to_source.at(i);

      bool ok;

      if (orient == Qt::Vertical) {
         ok = ! q->filterAcceptsRow(source_item, sourceParent);
      } else {
         ok = ! q->filterAcceptsColumn(source_item, sourceParent);
      }

      if (ok) {
         // source item does not satisfy the filter, so it must be removed
         source_items_remove.append(source_item);
      }
   }

   // Figure out which non-mapped items to insert
   QVector<int> source_items_insert;

   int source_count = source_to_proxy.size();

   for (int source_item = 0; source_item < source_count; ++source_item) {
      if (source_to_proxy.at(source_item) == -1) {
         bool ok;

         if (orient == Qt::Vertical) {
            ok = q->filterAcceptsRow(source_item, sourceParent);
         } else {
            ok = q->filterAcceptsColumn(source_item, sourceParent);
         }

         if (ok) {
            // source item satisfies the filter, so it must be added
            source_items_insert.append(source_item);
         }
      }
   }

   if (! source_items_remove.isEmpty() || ! source_items_insert.isEmpty()) {
      // Do item remove and insertion
      remove_source_items(source_to_proxy, proxy_to_source,
         source_items_remove, sourceParent, orient);

      if (orient == Qt::Vertical) {
         sort_source_rows(source_items_insert, sourceParent);
      }

      insert_source_items(source_to_proxy, proxy_to_source,
         source_items_insert, sourceParent, orient);
   }

   return qVectorToSet(source_items_remove);
}

void QSortFilterProxyModelPrivate::_q_sourceDataChanged(const QModelIndex &source_top_left,
   const QModelIndex &source_bottom_right, const QVector<int> &roles)
{
   Q_Q(QSortFilterProxyModel);

   if (! source_top_left.isValid() || ! source_bottom_right.isValid()) {
      return;
   }

   QModelIndex sourceParent = source_top_left.parent();
   IndexMap::const_iterator it = source_index_mapping.find(sourceParent);

   if (it == source_index_mapping.constEnd()) {
      // do not care since we do not have mapping for this index
      return;
   }

   Mapping *m = it.value();

   // Figure out how the source changes affect us
   QVector<int> source_rows_remove;
   QVector<int> source_rows_insert;
   QVector<int> source_rows_change;
   QVector<int> source_rows_resort;
   int end = qMin(source_bottom_right.row(), m->proxy_rows.count() - 1);

   for (int source_row = source_top_left.row(); source_row <= end; ++source_row) {
      if (dynamic_sortfilter) {
         if (m->proxy_rows.at(source_row) != -1) {
            if (! q->filterAcceptsRow(source_row, sourceParent)) {
               // This source row no longer satisfies the filter, so it must be removed
               source_rows_remove.append(source_row);
            } else if (source_sort_column >= source_top_left.column() && source_sort_column <= source_bottom_right.column()) {
               // This source row has changed in a way that may affect sorted order
               source_rows_resort.append(source_row);
            } else {
               // This row has simply changed, without affecting filtering nor sorting
               source_rows_change.append(source_row);
            }

         } else {
            if (!itemsBeingRemoved.contains(sourceParent, source_row) && q->filterAcceptsRow(source_row, sourceParent)) {
               // This source row now satisfies the filter, so it must be added
               source_rows_insert.append(source_row);
            }
         }

      } else {
         if (m->proxy_rows.at(source_row) != -1) {
            source_rows_change.append(source_row);
         }
      }
   }

   if (! source_rows_remove.isEmpty()) {
      remove_source_items(m->proxy_rows, m->source_rows,
         source_rows_remove, sourceParent, Qt::Vertical);

      QSet<int> source_rows_remove_set = qVectorToSet(source_rows_remove);
      QVector<QModelIndex>::iterator it = m->mapped_children.end();

      while (it != m->mapped_children.begin()) {
         --it;

         const QModelIndex source_child_index = *it;

         if (source_rows_remove_set.contains(source_child_index.row())) {
            it = m->mapped_children.erase(it);
            remove_from_mapping(source_child_index);
         }
      }
   }

   if (! source_rows_resort.isEmpty()) {
      // Re-sort the rows
      QList<QPersistentModelIndex> parents;
      parents << q->mapFromSource(sourceParent);
      emit q->layoutAboutToBeChanged(parents, QAbstractItemModel::VerticalSortHint);

      QModelIndexPairList source_indexes = store_persistent_indexes();
      remove_source_items(m->proxy_rows, m->source_rows, source_rows_resort,
         sourceParent, Qt::Vertical, false);

      sort_source_rows(source_rows_resort, sourceParent);
      insert_source_items(m->proxy_rows, m->source_rows, source_rows_resort,
         sourceParent, Qt::Vertical, false);

      update_persistent_indexes(source_indexes);
      emit q->layoutChanged(parents, QAbstractItemModel::VerticalSortHint);
      // Make sure we also emit dataChanged for the rows
      source_rows_change += source_rows_resort;
   }

   if (! source_rows_change.isEmpty()) {
      // Find the proxy row range
      int proxy_start_row;
      int proxy_end_row;
      proxy_item_range(m->proxy_rows, source_rows_change,
         proxy_start_row, proxy_end_row);
      // ### Find the proxy column range also
      if (proxy_end_row >= 0) {
         // the row was accepted, but some columns might still be filtered out
         int source_left_column = source_top_left.column();
         while (source_left_column < source_bottom_right.column()
            && m->proxy_columns.at(source_left_column) == -1) {
            ++source_left_column;
         }
         const QModelIndex proxy_top_left = create_index(
               proxy_start_row, m->proxy_columns.at(source_left_column), it);
         int source_right_column = source_bottom_right.column();
         while (source_right_column > source_top_left.column()
            && m->proxy_columns.at(source_right_column) == -1) {
            --source_right_column;
         }
         const QModelIndex proxy_bottom_right = create_index(
               proxy_end_row, m->proxy_columns.at(source_right_column), it);
         emit q->dataChanged(proxy_top_left, proxy_bottom_right, roles);
      }
   }

   if (! source_rows_insert.isEmpty()) {
      sort_source_rows(source_rows_insert, sourceParent);

      insert_source_items(m->proxy_rows, m->source_rows,
         source_rows_insert, sourceParent, Qt::Vertical);
   }
}

void QSortFilterProxyModelPrivate::_q_sourceHeaderDataChanged(Qt::Orientation orientation,
   int start, int end)
{
   Q_ASSERT(start <= end);
   Q_Q(QSortFilterProxyModel);

   Mapping *m = create_mapping(QModelIndex()).value();

   const QVector<int> &source_to_proxy = (orientation == Qt::Vertical) ? m->proxy_rows : m->proxy_columns;

   QVector<int> proxy_positions;
   proxy_positions.reserve(end - start + 1);

   {
      Q_ASSERT(source_to_proxy.size() > end);
      QVector<int>::const_iterator it = source_to_proxy.constBegin() + start;
      const QVector<int>::const_iterator endIt = source_to_proxy.constBegin() + end + 1;
      for ( ; it != endIt; ++it) {
         if (*it != -1) {
            proxy_positions.push_back(*it);
         }
      }
   }

   std::sort(proxy_positions.begin(), proxy_positions.end());

   int last_index = 0;
   const int numItems = proxy_positions.size();

   while (last_index < numItems) {
      const int proxyStart = proxy_positions.at(last_index);
      int proxyEnd = proxyStart;
      ++last_index;

      for (int i = last_index; i < numItems; ++i) {
         if (proxy_positions.at(i) == proxyEnd + 1) {
            ++last_index;
            ++proxyEnd;
         } else {
            break;
         }
      }
      emit q->headerDataChanged(orientation, proxyStart, proxyEnd);
   }
}

void QSortFilterProxyModelPrivate::_q_sourceAboutToBeReset()
{
   Q_Q(QSortFilterProxyModel);
   q->beginResetModel();
}

void QSortFilterProxyModelPrivate::_q_sourceReset()
{
   Q_Q(QSortFilterProxyModel);
   invalidatePersistentIndexes();
   _q_clearMapping();

   // All internal structures are deleted in clear()
   q->endResetModel();
   update_source_sort_column();

   if (dynamic_sortfilter) {
      sort();
   }
}

void QSortFilterProxyModelPrivate::_q_sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents,
   QAbstractItemModel::LayoutChangeHint hint)
{
   (void) hint;

   Q_Q(QSortFilterProxyModel);

   saved_persistent_indexes.clear();
   QList<QPersistentModelIndex> parents;

   for (const QPersistentModelIndex &parent : sourceParents) {
      if (!parent.isValid()) {
         parents << QPersistentModelIndex();
         continue;
      }

      const QModelIndex mappedParent = q->mapFromSource(parent);

      // Might be filtered out
      if (mappedParent.isValid()) {
         parents << mappedParent;
      }
   }

   // All parents filtered out
   if (! sourceParents.isEmpty() && parents.isEmpty()) {
      return;
   }

   emit q->layoutAboutToBeChanged(parents);

   if (persistent.m_indexes.isEmpty()) {
      return;
   }

   saved_persistent_indexes = store_persistent_indexes();
}

void QSortFilterProxyModelPrivate::_q_sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents,
   QAbstractItemModel::LayoutChangeHint hint)
{
   (void) hint;

   Q_Q(QSortFilterProxyModel);

   qDeleteAll(source_index_mapping);
   source_index_mapping.clear();

   update_persistent_indexes(saved_persistent_indexes);
   saved_persistent_indexes.clear();

   if (dynamic_sortfilter && update_source_sort_column()) {
      //update_source_sort_column might have created wrong mapping so we have to clear it again
      qDeleteAll(source_index_mapping);
      source_index_mapping.clear();
   }

   QList<QPersistentModelIndex> parents;
   for (const QPersistentModelIndex &parent : sourceParents) {
      if (!parent.isValid()) {
         parents << QPersistentModelIndex();
         continue;
      }
      const QModelIndex mappedParent = q->mapFromSource(parent);
      if (mappedParent.isValid()) {
         parents << mappedParent;
      }
   }

   if (! sourceParents.isEmpty() && parents.isEmpty()) {
      return;
   }

   emit q->layoutChanged(parents);
}

void QSortFilterProxyModelPrivate::_q_sourceRowsAboutToBeInserted(const QModelIndex &sourceParent, int start, int end)
{
   (void) start;
   (void) end;

   // Force the creation of a mapping now, even if its empty
   // We need it because the proxy can be acessed at the moment it emits rowsAboutToBeInserted in insert_source_items

   if (can_create_mapping(sourceParent)) {
      create_mapping(sourceParent);
   }
}

void QSortFilterProxyModelPrivate::_q_sourceRowsInserted(const QModelIndex &sourceParent, int start, int end)
{
   source_items_inserted(sourceParent, start, end, Qt::Vertical);

   if (update_source_sort_column() && dynamic_sortfilter) {
      // previous call to update_source_sort_column may fail if the model has no column.
      sort();   // now it should succeed. make sure to sort again
   }
}

void QSortFilterProxyModelPrivate::_q_sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end)
{
   itemsBeingRemoved = QRowsRemoval(sourceParent, start, end);
   source_items_about_to_be_removed(sourceParent, start, end, Qt::Vertical);
}

void QSortFilterProxyModelPrivate::_q_sourceRowsRemoved(const QModelIndex &sourceParent, int start, int end)
{
   itemsBeingRemoved = QRowsRemoval();
   source_items_removed(sourceParent, start, end, Qt::Vertical);
}

void QSortFilterProxyModelPrivate::_q_sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart,
   int sourceEnd, const QModelIndex &destinationParent, int destinationStart)
{
   (void) sourceStart;
   (void) sourceEnd;
   (void) destinationStart;

   Q_Q(QSortFilterProxyModel);

   // Because rows which are contiguous in the source model might not be contiguous
   // in the proxy due to sorting, the best thing we can do here is be specific about what
   // parents are having their children changed.
   // Optimize: Emit move signals if the proxy is not sorted. Will need to account for rows
   // being filtered out though.

   saved_persistent_indexes.clear();

   QList<QPersistentModelIndex> parents;
   parents << q->mapFromSource(sourceParent);

   if (sourceParent != destinationParent) {
      parents << q->mapFromSource(destinationParent);
   }

   emit q->layoutAboutToBeChanged(parents);

   if (persistent.m_indexes.isEmpty()) {
      return;
   }

   saved_persistent_indexes = store_persistent_indexes();
}

void QSortFilterProxyModelPrivate::_q_sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart,
   int sourceEnd, const QModelIndex &destinationParent, int destinationStart)
{
   (void) sourceStart;
   (void) sourceEnd;
   (void) destinationStart;

   Q_Q(QSortFilterProxyModel);

   // Optimize: We only need to clear and update the persistent indexes which are children of
   // sourceParent or destinationParent
   qDeleteAll(source_index_mapping);
   source_index_mapping.clear();

   update_persistent_indexes(saved_persistent_indexes);
   saved_persistent_indexes.clear();

   if (dynamic_sortfilter && update_source_sort_column()) {
      //update_source_sort_column might have created wrong mapping so we have to clear it again
      qDeleteAll(source_index_mapping);
      source_index_mapping.clear();
   }

   QList<QPersistentModelIndex> parents;
   parents << q->mapFromSource(sourceParent);

   if (sourceParent != destinationParent) {
      parents << q->mapFromSource(destinationParent);
   }

   emit q->layoutChanged(parents);
}

void QSortFilterProxyModelPrivate::_q_sourceColumnsAboutToBeInserted(const QModelIndex &sourceParent, int start, int end)
{
   (void) start;
   (void) end;

   // Force the creation of a mapping now, even if its empty.
   // We need it because the proxy can be acessed at the moment it emits columnsAboutToBeInserted in insert_source_items

   if (can_create_mapping(sourceParent)) {
      create_mapping(sourceParent);
   }
}

void QSortFilterProxyModelPrivate::_q_sourceColumnsInserted(const QModelIndex &sourceParent, int start, int end)
{
   Q_Q(const QSortFilterProxyModel);

   source_items_inserted(sourceParent, start, end, Qt::Horizontal);

   if (sourceParent.isValid()) {
      return;   // sort according to the root column only
   }

   if (source_sort_column == -1) {
      // update the source_sort_column depending on the proxy_sort_column
      if (update_source_sort_column() && dynamic_sortfilter) {
         sort();
      }

   } else {
      if (start <= source_sort_column) {
         source_sort_column += end - start + 1;
      }

      proxy_sort_column = q->mapFromSource(model->index(0, source_sort_column, sourceParent)).column();
   }
}

void QSortFilterProxyModelPrivate::_q_sourceColumnsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end)
{
   source_items_about_to_be_removed(sourceParent, start, end, Qt::Horizontal);
}

void QSortFilterProxyModelPrivate::_q_sourceColumnsRemoved(const QModelIndex &sourceParent, int start, int end)
{
   Q_Q(const QSortFilterProxyModel);

   source_items_removed(sourceParent, start, end, Qt::Horizontal);

   if (sourceParent.isValid()) {
      return;   // sort according to the root column only
   }

   if (start <= source_sort_column) {
      if (end < source_sort_column) {
         source_sort_column -= end - start + 1;
      } else {
         source_sort_column = -1;
      }
   }

   proxy_sort_column = q->mapFromSource(model->index(0, source_sort_column, sourceParent)).column();
}

void QSortFilterProxyModelPrivate::_q_sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int , int,
      const QModelIndex &destinationParent, int)
{
   Q_Q(QSortFilterProxyModel);

   saved_persistent_indexes.clear();

   QList<QPersistentModelIndex> parents;
   parents << q->mapFromSource(sourceParent);

   if (sourceParent != destinationParent) {
      parents << q->mapFromSource(destinationParent);
   }

   emit q->layoutAboutToBeChanged(parents);

   if (persistent.m_indexes.isEmpty()) {
      return;
   }

   saved_persistent_indexes = store_persistent_indexes();
}

void QSortFilterProxyModelPrivate::_q_sourceColumnsMoved(const QModelIndex &sourceParent, int, int,
      const QModelIndex &destinationParent, int)
{
   Q_Q(QSortFilterProxyModel);

   qDeleteAll(source_index_mapping);
   source_index_mapping.clear();
   update_persistent_indexes(saved_persistent_indexes);
   saved_persistent_indexes.clear();

   if (dynamic_sortfilter && update_source_sort_column()) {
      qDeleteAll(source_index_mapping);
      source_index_mapping.clear();
   }

   QList<QPersistentModelIndex> parents;
   parents << q->mapFromSource(sourceParent);

   if (sourceParent != destinationParent) {
      parents << q->mapFromSource(destinationParent);
   }

   emit q->layoutChanged(parents);
}

QSortFilterProxyModel::QSortFilterProxyModel(QObject *parent)
   : QAbstractProxyModel(*new QSortFilterProxyModelPrivate, parent)
{
   Q_D(QSortFilterProxyModel);
   d->proxy_sort_column = d->source_sort_column = -1;
   d->sort_order = Qt::AscendingOrder;
   d->sort_casesensitivity = Qt::CaseSensitive;
   d->sort_role = Qt::DisplayRole;
   d->sort_localeaware = false;
   d->filter_column = 0;
   d->filter_role = Qt::DisplayRole;
   d->dynamic_sortfilter = true;

   connect(this, &QSortFilterProxyModel::modelReset, this, &QSortFilterProxyModel::_q_clearMapping);
}

QSortFilterProxyModel::~QSortFilterProxyModel()
{
   Q_D(QSortFilterProxyModel);
   qDeleteAll(d->source_index_mapping);
   d->source_index_mapping.clear();
}

void QSortFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
   Q_D(QSortFilterProxyModel);

   beginResetModel();

   disconnect(d->model, &QAbstractItemModel::dataChanged,              this, &QSortFilterProxyModel::_q_sourceDataChanged);
   disconnect(d->model, &QAbstractItemModel::headerDataChanged,        this, &QSortFilterProxyModel::_q_sourceHeaderDataChanged);

   disconnect(d->model, &QAbstractItemModel::rowsAboutToBeInserted,    this, &QSortFilterProxyModel::_q_sourceRowsAboutToBeInserted);
   disconnect(d->model, &QAbstractItemModel::rowsInserted,             this, &QSortFilterProxyModel::_q_sourceRowsInserted);
   disconnect(d->model, &QAbstractItemModel::columnsAboutToBeInserted, this, &QSortFilterProxyModel::_q_sourceColumnsAboutToBeInserted);
   disconnect(d->model, &QAbstractItemModel::columnsInserted,          this, &QSortFilterProxyModel::_q_sourceColumnsInserted);

   disconnect(d->model, &QAbstractItemModel::rowsAboutToBeRemoved,     this, &QSortFilterProxyModel::_q_sourceRowsAboutToBeRemoved);
   disconnect(d->model, &QAbstractItemModel::rowsRemoved,              this, &QSortFilterProxyModel::_q_sourceRowsRemoved);
   disconnect(d->model, &QAbstractItemModel::columnsAboutToBeRemoved,  this, &QSortFilterProxyModel::_q_sourceColumnsAboutToBeRemoved);
   disconnect(d->model, &QAbstractItemModel::columnsRemoved,           this, &QSortFilterProxyModel::_q_sourceColumnsRemoved);

   disconnect(d->model, &QAbstractItemModel::rowsAboutToBeMoved,       this, &QSortFilterProxyModel::_q_sourceRowsAboutToBeMoved);
   disconnect(d->model, &QAbstractItemModel::rowsMoved,                this, &QSortFilterProxyModel::_q_sourceRowsMoved);
   disconnect(d->model, &QAbstractItemModel::columnsAboutToBeMoved,    this, &QSortFilterProxyModel::_q_sourceColumnsAboutToBeMoved);
   disconnect(d->model, &QAbstractItemModel::columnsMoved,             this, &QSortFilterProxyModel::_q_sourceColumnsMoved);

   disconnect(d->model, &QAbstractItemModel::layoutAboutToBeChanged,   this, &QSortFilterProxyModel::_q_sourceLayoutAboutToBeChanged);
   disconnect(d->model, &QAbstractItemModel::layoutChanged,            this, &QSortFilterProxyModel::_q_sourceLayoutChanged);
   disconnect(d->model, &QAbstractItemModel::modelAboutToBeReset,      this, &QSortFilterProxyModel::_q_sourceAboutToBeReset);
   disconnect(d->model, &QAbstractItemModel::modelReset,               this, &QSortFilterProxyModel::_q_sourceReset);

   QAbstractProxyModel::setSourceModel(sourceModel);

   connect(d->model, &QAbstractItemModel::dataChanged,              this, &QSortFilterProxyModel::_q_sourceDataChanged);
   connect(d->model, &QAbstractItemModel::headerDataChanged,        this, &QSortFilterProxyModel::_q_sourceHeaderDataChanged);

   connect(d->model, &QAbstractItemModel::rowsAboutToBeInserted,    this, &QSortFilterProxyModel::_q_sourceRowsAboutToBeInserted);
   connect(d->model, &QAbstractItemModel::rowsInserted,             this, &QSortFilterProxyModel::_q_sourceRowsInserted);
   connect(d->model, &QAbstractItemModel::columnsAboutToBeInserted, this, &QSortFilterProxyModel::_q_sourceColumnsAboutToBeInserted);
   connect(d->model, &QAbstractItemModel::columnsInserted,          this, &QSortFilterProxyModel::_q_sourceColumnsInserted);

   connect(d->model, &QAbstractItemModel::rowsAboutToBeRemoved,     this, &QSortFilterProxyModel::_q_sourceRowsAboutToBeRemoved);
   connect(d->model, &QAbstractItemModel::rowsRemoved,              this, &QSortFilterProxyModel::_q_sourceRowsRemoved);
   connect(d->model, &QAbstractItemModel::columnsAboutToBeRemoved,  this, &QSortFilterProxyModel::_q_sourceColumnsAboutToBeRemoved);
   connect(d->model, &QAbstractItemModel::columnsRemoved,           this, &QSortFilterProxyModel::_q_sourceColumnsRemoved);

   connect(d->model, &QAbstractItemModel::rowsAboutToBeMoved,       this, &QSortFilterProxyModel::_q_sourceRowsAboutToBeMoved);
   connect(d->model, &QAbstractItemModel::rowsMoved,                this, &QSortFilterProxyModel::_q_sourceRowsMoved);
   connect(d->model, &QAbstractItemModel::columnsAboutToBeMoved,    this, &QSortFilterProxyModel::_q_sourceColumnsAboutToBeMoved);
   connect(d->model, &QAbstractItemModel::columnsMoved,             this, &QSortFilterProxyModel::_q_sourceColumnsMoved);

   connect(d->model, &QAbstractItemModel::layoutAboutToBeChanged,   this, &QSortFilterProxyModel::_q_sourceLayoutAboutToBeChanged);
   connect(d->model, &QAbstractItemModel::layoutChanged,            this, &QSortFilterProxyModel::_q_sourceLayoutChanged);
   connect(d->model, &QAbstractItemModel::modelAboutToBeReset,      this, &QSortFilterProxyModel::_q_sourceAboutToBeReset);
   connect(d->model, &QAbstractItemModel::modelReset,               this, &QSortFilterProxyModel::_q_sourceReset);

   d->_q_clearMapping();
   endResetModel();

   if (d->update_source_sort_column() && d->dynamic_sortfilter) {
      d->sort();
   }
}

QModelIndex QSortFilterProxyModel::index(int row, int column, const QModelIndex &parent) const
{
   Q_D(const QSortFilterProxyModel);

   if (row < 0 || column < 0) {
      return QModelIndex();
   }

   QModelIndex sourceParent = mapToSource(parent); // parent is already mapped at this point
   IndexMap::const_iterator it = d->create_mapping(sourceParent); // but make sure that the children are mapped

   if (it.value()->source_rows.count() <= row || it.value()->source_columns.count() <= column) {
      return QModelIndex();
   }

   return d->create_index(row, column, it);
}

QModelIndex QSortFilterProxyModel::parent(const QModelIndex &child) const
{
   Q_D(const QSortFilterProxyModel);

   if (! d->indexValid(child)) {
      return QModelIndex();
   }

   IndexMap::const_iterator it = d->index_to_iterator(child);

   Q_ASSERT(it != d->source_index_mapping.constEnd());

   QModelIndex sourceParent = it.key();
   QModelIndex proxy_parent = mapFromSource(sourceParent);

   return proxy_parent;
}

QModelIndex QSortFilterProxyModel::sibling(int row, int column, const QModelIndex &idx) const
{
   Q_D(const QSortFilterProxyModel);

   if (! d->indexValid(idx)) {
      return QModelIndex();
   }

   const IndexMap::const_iterator it = d->index_to_iterator(idx);
   if (it.value()->source_rows.count() <= row || it.value()->source_columns.count() <= column) {
      return QModelIndex();
   }

   return d->create_index(row, column, it);
}

int QSortFilterProxyModel::rowCount(const QModelIndex &parent) const
{
   Q_D(const QSortFilterProxyModel);

   QModelIndex sourceParent = mapToSource(parent);

   if (parent.isValid() && ! sourceParent.isValid()) {
      return 0;
   }

   IndexMap::const_iterator it = d->create_mapping(sourceParent);

   return it.value()->source_rows.count();
}

int QSortFilterProxyModel::columnCount(const QModelIndex &parent) const
{
   Q_D(const QSortFilterProxyModel);

   QModelIndex sourceParent = mapToSource(parent);

   if (parent.isValid() && ! sourceParent.isValid()) {
      return 0;
   }

   IndexMap::const_iterator it = d->create_mapping(sourceParent);

   return it.value()->source_columns.count();
}

bool QSortFilterProxyModel::hasChildren(const QModelIndex &parent) const
{
   Q_D(const QSortFilterProxyModel);

   QModelIndex sourceParent = mapToSource(parent);

   if (parent.isValid() && ! sourceParent.isValid()) {
      return false;
   }

   if (! d->model->hasChildren(sourceParent)) {
      return false;
   }

   if (d->model->canFetchMore(sourceParent)) {
      return true;   // assume we might have children that can be fetched
   }

   QSortFilterProxyModelPrivate::Mapping *m = d->create_mapping(sourceParent).value();
   return m->source_rows.count() != 0 && m->source_columns.count() != 0;
}

QVariant QSortFilterProxyModel::data(const QModelIndex &index, int role) const
{
   Q_D(const QSortFilterProxyModel);

   QModelIndex source_index = mapToSource(index);

   if (index.isValid() && ! source_index.isValid()) {
      return QVariant();
   }

   return d->model->data(source_index, role);
}

bool QSortFilterProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   Q_D(QSortFilterProxyModel);

   QModelIndex source_index = mapToSource(index);

   if (index.isValid() && ! source_index.isValid()) {
      return false;
   }

   return d->model->setData(source_index, value, role);
}

QVariant QSortFilterProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_D(const QSortFilterProxyModel);

   IndexMap::const_iterator it = d->create_mapping(QModelIndex());

   if (it.value()->source_rows.count() * it.value()->source_columns.count() > 0) {
      return QAbstractProxyModel::headerData(section, orientation, role);
   }

   int source_section;

   if (orientation == Qt::Vertical) {
      if (section < 0 || section >= it.value()->source_rows.count()) {
         return QVariant();
      }
      source_section = it.value()->source_rows.at(section);

   } else {
      if (section < 0 || section >= it.value()->source_columns.count()) {
         return QVariant();
      }
      source_section = it.value()->source_columns.at(section);
   }

   return d->model->headerData(source_section, orientation, role);
}

bool QSortFilterProxyModel::setHeaderData(int section, Qt::Orientation orientation,
   const QVariant &value, int role)
{
   Q_D(QSortFilterProxyModel);

   IndexMap::const_iterator it = d->create_mapping(QModelIndex());

   if (it.value()->source_rows.count() * it.value()->source_columns.count() > 0) {
      return QAbstractProxyModel::setHeaderData(section, orientation, value, role);
   }

   int source_section;

   if (orientation == Qt::Vertical) {
      if (section < 0 || section >= it.value()->source_rows.count()) {
         return false;
      }
      source_section = it.value()->source_rows.at(section);

   } else {
      if (section < 0 || section >= it.value()->source_columns.count()) {
         return false;
      }
      source_section = it.value()->source_columns.at(section);
   }

   return d->model->setHeaderData(source_section, orientation, value, role);
}

QMimeData *QSortFilterProxyModel::mimeData(const QModelIndexList &indexes) const
{
   Q_D(const QSortFilterProxyModel);

   QModelIndexList source_indexes;

   for (int i = 0; i < indexes.count(); ++i) {
      source_indexes << mapToSource(indexes.at(i));
   }

   return d->model->mimeData(source_indexes);
}

QStringList QSortFilterProxyModel::mimeTypes() const
{
   Q_D(const QSortFilterProxyModel);
   return d->model->mimeTypes();
}

Qt::DropActions QSortFilterProxyModel::supportedDropActions() const
{
   Q_D(const QSortFilterProxyModel);
   return d->model->supportedDropActions();
}

bool QSortFilterProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
   int row, int column, const QModelIndex &parent)
{
   return QAbstractProxyModel::dropMimeData(data, action, row, column, parent);
}

bool QSortFilterProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
   Q_D(QSortFilterProxyModel);

   if (row < 0 || count <= 0) {
      return false;
   }

   QModelIndex sourceParent = mapToSource(parent);
   if (parent.isValid() && ! sourceParent.isValid()) {
      return false;
   }

   QSortFilterProxyModelPrivate::Mapping *m = d->create_mapping(sourceParent).value();
   if (row > m->source_rows.count()) {
      return false;
   }

   int source_row = (row >= m->source_rows.count()
         ? m->source_rows.count() : m->source_rows.at(row));

   return d->model->insertRows(source_row, count, sourceParent);
}

bool QSortFilterProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
   Q_D(QSortFilterProxyModel);

   if (column < 0 || count <= 0) {
      return false;
   }

   QModelIndex sourceParent = mapToSource(parent);
   if (parent.isValid() && ! sourceParent.isValid()) {
      return false;
   }

   QSortFilterProxyModelPrivate::Mapping *m = d->create_mapping(sourceParent).value();

   if (column > m->source_columns.count()) {
      return false;
   }

   int source_column = (column >= m->source_columns.count()
         ? m->source_columns.count() : m->source_columns.at(column));

   return d->model->insertColumns(source_column, count, sourceParent);
}

bool QSortFilterProxyModel::removeRows(int row, int count, const QModelIndex &parent)
{
   Q_D(QSortFilterProxyModel);

   if (row < 0 || count <= 0) {
      return false;
   }

   QModelIndex sourceParent = mapToSource(parent);

   if (parent.isValid() && ! sourceParent.isValid()) {
      return false;
   }

   QSortFilterProxyModelPrivate::Mapping *m = d->create_mapping(sourceParent).value();

   if (row + count > m->source_rows.count()) {
      return false;
   }

   if ((count == 1) || ((d->source_sort_column < 0) && (m->proxy_rows.count() == m->source_rows.count()))) {
      int source_row = m->source_rows.at(row);
      return d->model->removeRows(source_row, count, sourceParent);
   }

   // remove corresponding source intervals
   // if this proves to be slow, switch to single-row removal

   QVector<int> rows;
   rows.reserve(count);

   for (int i = row; i < row + count; ++i) {
      rows.append(m->source_rows.at(i));
   }

   std::sort(rows.begin(), rows.end());

   int pos = rows.count() - 1;
   bool ok = true;

   while (pos >= 0) {
      const int source_end = rows.at(pos--);
      int source_start = source_end;

      while ((pos >= 0) && (rows.at(pos) == (source_start - 1))) {
         --source_start;
         --pos;
      }

      ok = ok && d->model->removeRows(source_start, source_end - source_start + 1, sourceParent);
   }

   return ok;
}

bool QSortFilterProxyModel::removeColumns(int column, int count, const QModelIndex &parent)
{
   Q_D(QSortFilterProxyModel);

   if (column < 0 || count <= 0) {
      return false;
   }

   QModelIndex sourceParent = mapToSource(parent);
   if (parent.isValid() && ! sourceParent.isValid()) {
      return false;
   }

   QSortFilterProxyModelPrivate::Mapping *m = d->create_mapping(sourceParent).value();
   if (column + count > m->source_columns.count()) {
      return false;
   }

   if ((count == 1) || (m->proxy_columns.count() == m->source_columns.count())) {
      int source_column = m->source_columns.at(column);
      return d->model->removeColumns(source_column, count, sourceParent);
   }

   // remove corresponding source intervals
   QVector<int> columns;
   columns.reserve(count);

   for (int i = column; i < column + count; ++i) {
      columns.append(m->source_columns.at(i));
   }

   int pos = columns.count() - 1;
   bool ok = true;
   while (pos >= 0) {
      const int source_end = columns.at(pos--);
      int source_start = source_end;

      while ((pos >= 0) && (columns.at(pos) == (source_start - 1))) {
         --source_start;
         --pos;
      }

      ok = ok && d->model->removeColumns(source_start, source_end - source_start + 1, sourceParent);
   }

   return ok;
}

void QSortFilterProxyModel::fetchMore(const QModelIndex &parent)
{
   Q_D(QSortFilterProxyModel);

   QModelIndex sourceParent;

   if (d->indexValid(parent)) {
      sourceParent = mapToSource(parent);
   }

   d->model->fetchMore(sourceParent);
}

bool QSortFilterProxyModel::canFetchMore(const QModelIndex &parent) const
{
   Q_D(const QSortFilterProxyModel);

   QModelIndex sourceParent;

   if (d->indexValid(parent)) {
      sourceParent = mapToSource(parent);
   }

   return d->model->canFetchMore(sourceParent);
}

Qt::ItemFlags QSortFilterProxyModel::flags(const QModelIndex &index) const
{
   Q_D(const QSortFilterProxyModel);

   QModelIndex source_index;

   if (d->indexValid(index)) {
      source_index = mapToSource(index);
   }

   return d->model->flags(source_index);
}

QModelIndex QSortFilterProxyModel::buddy(const QModelIndex &index) const
{
   Q_D(const QSortFilterProxyModel);

   if (!d->indexValid(index)) {
      return QModelIndex();
   }

   QModelIndex source_index = mapToSource(index);
   QModelIndex source_buddy = d->model->buddy(source_index);

   if (source_index == source_buddy) {
      return index;
   }

   return mapFromSource(source_buddy);
}

QModelIndexList QSortFilterProxyModel::match(const QModelIndex &start, int role,
         const QVariant &value, int hits, Qt::MatchFlags flags) const
{
   return QAbstractProxyModel::match(start, role, value, hits, flags);
}

QSize QSortFilterProxyModel::span(const QModelIndex &index) const
{
   Q_D(const QSortFilterProxyModel);

   QModelIndex source_index = mapToSource(index);

   if (index.isValid() && ! source_index.isValid()) {
      return QSize();
   }

   return d->model->span(source_index);
}

void QSortFilterProxyModel::sort(int column, Qt::SortOrder order)
{
   Q_D(QSortFilterProxyModel);

   if (d->dynamic_sortfilter && d->proxy_sort_column == column && d->sort_order == order) {
      return;
   }

   d->sort_order = order;
   d->proxy_sort_column = column;
   d->update_source_sort_column();
   d->sort();
}

int QSortFilterProxyModel::sortColumn() const
{
   Q_D(const QSortFilterProxyModel);
   return d->proxy_sort_column;
}

Qt::SortOrder QSortFilterProxyModel::sortOrder() const
{
   Q_D(const QSortFilterProxyModel);
   return d->sort_order;
}

QRegularExpression QSortFilterProxyModel::filterRegExp() const
{
   Q_D(const QSortFilterProxyModel);
   return d->filter_regexp;
}

void QSortFilterProxyModel::setFilterRegExp(const QRegularExpression &regExp)
{
   Q_D(QSortFilterProxyModel);

   d->filter_about_to_be_changed();
   d->filter_regexp = regExp;
   d->filter_changed();
}

int QSortFilterProxyModel::filterKeyColumn() const
{
   Q_D(const QSortFilterProxyModel);
   return d->filter_column;
}

void QSortFilterProxyModel::setFilterKeyColumn(int column)
{
   Q_D(QSortFilterProxyModel);

   d->filter_about_to_be_changed();
   d->filter_column = column;
   d->filter_changed();
}

Qt::CaseSensitivity QSortFilterProxyModel::filterCaseSensitivity() const
{
   Q_D(const QSortFilterProxyModel);

   QPatternOptionFlags flags = d->filter_regexp.patternOptions();

   if (flags & QPatternOption::CaseInsensitiveOption) {
      return Qt::CaseInsensitive;
   } else {
      return Qt::CaseSensitive;
   }
}

void QSortFilterProxyModel::setFilterCaseSensitivity(Qt::CaseSensitivity cs)
{
   Q_D(QSortFilterProxyModel);

   QPatternOptionFlags flags = d->filter_regexp.patternOptions();
   QPatternOptionFlags newFlags = flags;

   if (cs == Qt::CaseSensitive) {
      newFlags = flags & ~QPatternOptionFlags(QPatternOption::CaseInsensitiveOption);

   } else {
      newFlags = flags | QPatternOption::CaseInsensitiveOption;

   }

   if (flags == newFlags) {
      return;
   }

   d->filter_about_to_be_changed();
   d->filter_regexp.setPatternOptions(flags);
   d->filter_changed();
}

Qt::CaseSensitivity QSortFilterProxyModel::sortCaseSensitivity() const
{
   Q_D(const QSortFilterProxyModel);
   return d->sort_casesensitivity;
}

void QSortFilterProxyModel::setSortCaseSensitivity(Qt::CaseSensitivity cs)
{
   Q_D(QSortFilterProxyModel);

   if (d->sort_casesensitivity == cs) {
      return;
   }

   d->sort_casesensitivity = cs;
   d->sort();
}

bool QSortFilterProxyModel::isSortLocaleAware() const
{
   Q_D(const QSortFilterProxyModel);
   return d->sort_localeaware;
}

void QSortFilterProxyModel::setSortLocaleAware(bool on)
{
   Q_D(QSortFilterProxyModel);

   if (d->sort_localeaware == on) {
      return;
   }

   d->sort_localeaware = on;
   d->sort();
}

void QSortFilterProxyModel::setFilterRegExp(const QString &pattern)
{
   Q_D(QSortFilterProxyModel);
   d->filter_about_to_be_changed();
   d->filter_regexp.setPatternOptions(QPatternOption::NoPatternOption);
   d->filter_regexp.setPattern(pattern);
   d->filter_changed();
}

void QSortFilterProxyModel::setFilterWildcard(const QString &pattern)
{
   Q_D(QSortFilterProxyModel);
   d->filter_about_to_be_changed();
   d->filter_regexp.setPatternOptions(QPatternOption::WildcardOption);
   d->filter_regexp.setPattern(pattern);
   d->filter_changed();
}

void QSortFilterProxyModel::setFilterFixedString(const QString &pattern)
{
   Q_D(QSortFilterProxyModel);
   d->filter_about_to_be_changed();
   d->filter_regexp.setPatternOptions(QPatternOption::NoPatternOption);
   d->filter_regexp.setPattern(pattern);
   d->filter_changed();
}

bool QSortFilterProxyModel::dynamicSortFilter() const
{
   Q_D(const QSortFilterProxyModel);
   return d->dynamic_sortfilter;
}

void QSortFilterProxyModel::setDynamicSortFilter(bool enable)
{
   Q_D(QSortFilterProxyModel);

   d->dynamic_sortfilter = enable;

   if (enable) {
      d->sort();
   }
}

int QSortFilterProxyModel::sortRole() const
{
   Q_D(const QSortFilterProxyModel);
   return d->sort_role;
}

void QSortFilterProxyModel::setSortRole(int role)
{
   Q_D(QSortFilterProxyModel);

   if (d->sort_role == role) {
      return;
   }

   d->sort_role = role;
   d->sort();
}

int QSortFilterProxyModel::filterRole() const
{
   Q_D(const QSortFilterProxyModel);
   return d->filter_role;
}

void QSortFilterProxyModel::setFilterRole(int role)
{
   Q_D(QSortFilterProxyModel);
   if (d->filter_role == role) {
      return;
   }

   d->filter_about_to_be_changed();
   d->filter_role = role;
   d->filter_changed();
}

void QSortFilterProxyModel::invalidate()
{
   Q_D(QSortFilterProxyModel);

   emit layoutAboutToBeChanged();
   d->_q_clearMapping();
   emit layoutChanged();
}

void QSortFilterProxyModel::invalidateFilter()
{
   Q_D(QSortFilterProxyModel);
   d->filter_changed();
}

bool QSortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
   Q_D(const QSortFilterProxyModel);

   QVariant l = (source_left.model() ? source_left.model()->data(source_left, d->sort_role) : QVariant());
   QVariant r = (source_right.model() ? source_right.model()->data(source_right, d->sort_role) : QVariant());

   // Duplicated in QStandardItem::operator<()
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
         if (d->sort_localeaware) {
            return l.toString().localeAwareCompare(r.toString()) < 0;
         } else {
            return l.toString().compare(r.toString(), d->sort_casesensitivity) < 0;
         }
   }
   return false;
}

bool QSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &sourceParent) const
{
   Q_D(const QSortFilterProxyModel);

   if (! d->filter_regexp.isValid()) {
      return true;
   }

   if (d->filter_column == -1) {
      int column_count = d->model->columnCount(sourceParent);

      for (int column = 0; column < column_count; ++column) {
         QModelIndex source_index = d->model->index(source_row, column, sourceParent);
         QString key = d->model->data(source_index, d->filter_role).toString();

         if (key.contains(d->filter_regexp)) {
            return true;
         }
      }

      return false;
   }

   QModelIndex source_index = d->model->index(source_row, d->filter_column, sourceParent);

   if (! source_index.isValid()) { // the column may not exist
      return true;
   }

   QString key = d->model->data(source_index, d->filter_role).toString();
   return key.contains(d->filter_regexp);
}

bool QSortFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &sourceParent) const
{
   (void) source_column;
   (void) sourceParent;

   return true;
}

QModelIndex QSortFilterProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
   Q_D(const QSortFilterProxyModel);
   return d->proxy_to_source(proxyIndex);
}

QModelIndex QSortFilterProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
   Q_D(const QSortFilterProxyModel);
   return d->source_to_proxy(sourceIndex);
}

QItemSelection QSortFilterProxyModel::mapSelectionToSource(const QItemSelection &proxySelection) const
{
   return QAbstractProxyModel::mapSelectionToSource(proxySelection);
}

QItemSelection QSortFilterProxyModel::mapSelectionFromSource(const QItemSelection &sourceSelection) const
{
   return QAbstractProxyModel::mapSelectionFromSource(sourceSelection);
}

void QSortFilterProxyModel::_q_sourceDataChanged(const QModelIndex &source_top_left,
   const QModelIndex &source_bottom_right, const QVector<int> &roles)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceDataChanged(source_top_left, source_bottom_right, roles);
}

void QSortFilterProxyModel::_q_sourceHeaderDataChanged(Qt::Orientation orientation, int start, int end)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceHeaderDataChanged(orientation, start, end);
}

void QSortFilterProxyModel::_q_sourceAboutToBeReset()
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceAboutToBeReset();
}

void QSortFilterProxyModel::_q_sourceReset()
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceReset();
}

void QSortFilterProxyModel::_q_sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents,
   QAbstractItemModel::LayoutChangeHint hint)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceLayoutAboutToBeChanged(sourceParents, hint);
}

void QSortFilterProxyModel::_q_sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents,
   QAbstractItemModel::LayoutChangeHint hint)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceLayoutChanged(sourceParents, hint);
}

void QSortFilterProxyModel::_q_sourceRowsAboutToBeInserted(const QModelIndex &sourceParent, int start, int end)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceRowsAboutToBeInserted(sourceParent, start, end);
}

void QSortFilterProxyModel::_q_sourceRowsInserted(const QModelIndex &sourceParent, int start, int end)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceRowsInserted(sourceParent, start, end);
}

void QSortFilterProxyModel::_q_sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceRowsAboutToBeRemoved(sourceParent, start, end);
}

void QSortFilterProxyModel::_q_sourceRowsRemoved(const QModelIndex &sourceParent, int start, int end)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceRowsRemoved(sourceParent, start, end);
}

void QSortFilterProxyModel::_q_sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart,
   int sourceEnd, const QModelIndex &destinationParent, int destinationStart)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceRowsAboutToBeMoved(sourceParent, sourceStart, sourceEnd, destinationParent, destinationStart);
}

void QSortFilterProxyModel::_q_sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart,
   int sourceEnd, const QModelIndex &destinationParent, int destinationStart)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceRowsMoved(sourceParent, sourceStart, sourceEnd, destinationParent, destinationStart);
}

void QSortFilterProxyModel::_q_sourceColumnsAboutToBeInserted(const QModelIndex &sourceParent, int start, int end)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceColumnsAboutToBeInserted(sourceParent, start, end);
}

void QSortFilterProxyModel::_q_sourceColumnsInserted(const QModelIndex &sourceParent, int start, int end)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceColumnsInserted(sourceParent, start, end);
}

void QSortFilterProxyModel::_q_sourceColumnsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceColumnsAboutToBeRemoved(sourceParent, start, end);
}

void QSortFilterProxyModel::_q_sourceColumnsRemoved(const QModelIndex &sourceParent, int start, int end)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceColumnsRemoved(sourceParent, start, end);
}

void QSortFilterProxyModel::_q_sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
         const QModelIndex &destinationParent, int destinationStart)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceColumnsAboutToBeMoved(sourceParent, sourceStart, sourceEnd, destinationParent, destinationStart);
}

void QSortFilterProxyModel::_q_sourceColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
         const QModelIndex &destinationParent, int destinationStart)
{
   Q_D(QSortFilterProxyModel);
   d->_q_sourceColumnsMoved(sourceParent, sourceStart, sourceEnd, destinationParent, destinationStart);
}

void QSortFilterProxyModel::_q_clearMapping()
{
   Q_D(QSortFilterProxyModel);
   d->_q_clearMapping();
}

#endif // QT_NO_SORTFILTERPROXYMODEL
