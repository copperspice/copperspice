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

#include <qitemselectionmodel.h>
#include <qitemselectionmodel_p.h>
#include <qdebug.h>

#include <algorithm>

#ifndef QT_NO_ITEMVIEWS

bool QItemSelectionRange::intersects(const QItemSelectionRange &other) const
{
   return (isValid() && other.isValid()
         && parent() == other.parent()
         && model() == other.model()
         && ((top() <= other.top() && bottom() >= other.top())
            || (top() >= other.top() && top() <= other.bottom()))
         && ((left() <= other.left() && right() >= other.left())
            || (left() >= other.left() && left() <= other.right())));
}

QItemSelectionRange QItemSelectionRange::intersected(const QItemSelectionRange &other) const
{
   if (model() == other.model() && parent() == other.parent()) {
      QModelIndex topLeft = model()->index(qMax(top(), other.top()),
            qMax(left(), other.left()), other.parent());

      QModelIndex bottomRight = model()->index(qMin(bottom(), other.bottom()),
            qMin(right(), other.right()), other.parent());

      return QItemSelectionRange(topLeft, bottomRight);
   }

   return QItemSelectionRange();
}

static void rowLengthsFromRange(const QItemSelectionRange &range, QVector<QPair<QPersistentModelIndex, uint>> &result)
{
   if (range.isValid() && range.model()) {
      const QModelIndex topLeft = range.topLeft();
      const int bottom = range.bottom();
      const uint width = range.width();
      const int column = topLeft.column();
      for (int row = topLeft.row(); row <= bottom; ++row) {
         // We don't need to keep track of ItemIsSelectable and ItemIsEnabled here. That is
         // required in indexesFromRange() because that method is called from public API
         // which requires the limitation.
         result.push_back(qMakePair(QPersistentModelIndex(topLeft.sibling(row, column)), width));
      }
   }
}

template <typename ModelIndexContainer>
static void indexesFromRange(const QItemSelectionRange &range, ModelIndexContainer &result)
{
   if (range.isValid() && range.model()) {
      const QModelIndex topLeft = range.topLeft();
      const int bottom = range.bottom();
      const int right  = range.right();

      for (int row = topLeft.row(); row <= bottom; ++row) {
         const QModelIndex columnLeader = topLeft.sibling(row, topLeft.column());

         for (int column = topLeft.column(); column <= right; ++column) {
            QModelIndex index = columnLeader.sibling(row, column);
            Qt::ItemFlags flags = range.model()->flags(index);
            if ((flags & Qt::ItemIsSelectable) && (flags & Qt::ItemIsEnabled)) {
               result.push_back(index);
            }
         }
      }
   }
}

bool QItemSelectionRange::isEmpty() const
{
   if (!isValid() || !model()) {
      return true;
   }

   for (int column = left(); column <= right(); ++column) {
      for (int row = top(); row <= bottom(); ++row) {
         QModelIndex index = model()->index(row, column, parent());
         Qt::ItemFlags flags = model()->flags(index);
         if ((flags & Qt::ItemIsSelectable) && (flags & Qt::ItemIsEnabled)) {
            return false;
         }
      }
   }
   return true;
}

QModelIndexList QItemSelectionRange::indexes() const
{
   QModelIndexList result;
   indexesFromRange(*this, result);

   return result;
}

QItemSelection::QItemSelection(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
   select(topLeft, bottomRight);
}

void QItemSelection::select(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
   if (! topLeft.isValid() || !bottomRight.isValid()) {
      return;
   }

   if ((topLeft.model() != bottomRight.model()) || topLeft.parent() != bottomRight.parent()) {
      qWarning("QItemSelection::select() Unable to make a selection from a different model or with different parents");
      return;
   }

   if (topLeft.row() > bottomRight.row() || topLeft.column() > bottomRight.column()) {
      int top = qMin(topLeft.row(), bottomRight.row());
      int bottom = qMax(topLeft.row(), bottomRight.row());
      int left = qMin(topLeft.column(), bottomRight.column());
      int right = qMax(topLeft.column(), bottomRight.column());
      QModelIndex tl = topLeft.sibling(top, left);
      QModelIndex br = bottomRight.sibling(bottom, right);
      append(QItemSelectionRange(tl, br));
      return;
   }

   append(QItemSelectionRange(topLeft, bottomRight));
}

bool QItemSelection::contains(const QModelIndex &index) const
{
   if (index.flags() & Qt::ItemIsSelectable) {
      QList<QItemSelectionRange>::const_iterator it = begin();
      for (; it != end(); ++it)
         if ((*it).contains(index)) {
            return true;
         }
   }
   return false;
}

QModelIndexList QItemSelection::indexes() const
{
   QModelIndexList result;
   QList<QItemSelectionRange>::const_iterator it = begin();
   for (; it != end(); ++it) {
      indexesFromRange(*it, result);
   }
   return result;
}

static QVector<QPersistentModelIndex> qSelectionPersistentindexes(const QItemSelection &sel)
{
   QVector<QPersistentModelIndex> result;
   QList<QItemSelectionRange>::const_iterator it = sel.constBegin();

   for (; it != sel.constEnd(); ++it) {
      indexesFromRange(*it, result);
   }

   return result;
}

static QVector<QPair<QPersistentModelIndex, uint>> qSelectionPersistentRowLengths(const QItemSelection &sel)
{
   QVector<QPair<QPersistentModelIndex, uint>> result;

   for (const QItemSelectionRange &range : sel) {
      rowLengthsFromRange(range, result);
   }

   return result;
}

void QItemSelection::cs_internal_merge(const QItemSelection &other, uint commandInt)
{
   QItemSelectionModel::SelectionFlags command = static_cast<QItemSelectionModel::SelectionFlags>(commandInt);

   if (other.isEmpty() ||
      ! (command & QItemSelectionModel::Select ||
         command & QItemSelectionModel::Deselect ||
         command & QItemSelectionModel::Toggle)) {
      return;
   }

   QItemSelection newSelection = other;

   // Collect intersections
   QItemSelection intersections;
   QItemSelection::iterator it = newSelection.begin();

   while (it != newSelection.end()) {
      if (!(*it).isValid()) {
         it = newSelection.erase(it);
         continue;
      }

      for (int t = 0; t < count(); ++t) {
         if ((*it).intersects(at(t))) {
            intersections.append(at(t).intersected(*it));
         }
      }
      ++it;
   }

   //  Split the old (and new) ranges using the intersections
   for (int i = 0; i < intersections.count(); ++i) { // for each intersection
      for (int t = 0; t < count();) { // splitt each old range
         if (at(t).intersects(intersections.at(i))) {
            split(at(t), intersections.at(i), this);
            removeAt(t);
         } else {
            ++t;
         }
      }

      // only split newSelection if Toggle is specified
      for (int n = 0; (command & QItemSelectionModel::Toggle) && n < newSelection.count();) {
         if (newSelection.at(n).intersects(intersections.at(i))) {
            split(newSelection.at(n), intersections.at(i), &newSelection);
            newSelection.removeAt(n);
         } else {
            ++n;
         }
      }
   }

   // do not add newSelection for Deselect
   if (!(command & QItemSelectionModel::Deselect)) {
      operator+=(newSelection);
   }
}

void QItemSelection::split(const QItemSelectionRange &range,
   const QItemSelectionRange &other, QItemSelection *result)
{
   if (range.parent() != other.parent() || range.model() != other.model()) {
      return;
   }

   QModelIndex parent = other.parent();
   int top    = range.top();
   int left   = range.left();
   int bottom = range.bottom();
   int right  = range.right();

   int other_top    = other.top();
   int other_left   = other.left();
   int other_bottom = other.bottom();
   int other_right  = other.right();

   const QAbstractItemModel *model = range.model();
   Q_ASSERT(model);

   if (other_top > top) {
      QModelIndex tl = model->index(top, left, parent);
      QModelIndex br = model->index(other_top - 1, right, parent);
      result->append(QItemSelectionRange(tl, br));
      top = other_top;
   }
   if (other_bottom < bottom) {
      QModelIndex tl = model->index(other_bottom + 1, left, parent);
      QModelIndex br = model->index(bottom, right, parent);
      result->append(QItemSelectionRange(tl, br));
      bottom = other_bottom;
   }
   if (other_left > left) {
      QModelIndex tl = model->index(top, left, parent);
      QModelIndex br = model->index(bottom, other_left - 1, parent);
      result->append(QItemSelectionRange(tl, br));
      left = other_left;
   }
   if (other_right < right) {
      QModelIndex tl = model->index(top, other_right + 1, parent);
      QModelIndex br = model->index(bottom, right, parent);
      result->append(QItemSelectionRange(tl, br));
      right = other_right;
   }
}

void QItemSelectionModelPrivate::initModel(QAbstractItemModel *model)
{
   Q_Q(QItemSelectionModel);

   if (this->model) {
      QObject::disconnect(model, &QAbstractItemModel::rowsAboutToBeRemoved,     q, &QItemSelectionModel::_q_rowsAboutToBeRemoved);
      QObject::disconnect(model, &QAbstractItemModel::columnsAboutToBeRemoved,  q, &QItemSelectionModel::_q_columnsAboutToBeRemoved);
      QObject::disconnect(model, &QAbstractItemModel::rowsAboutToBeInserted,    q, &QItemSelectionModel::_q_rowsAboutToBeInserted);
      QObject::disconnect(model, &QAbstractItemModel::columnsAboutToBeInserted, q, &QItemSelectionModel::_q_columnsAboutToBeInserted);
      QObject::disconnect(model, &QAbstractItemModel::rowsAboutToBeMoved,       q, &QItemSelectionModel::_q_layoutAboutToBeChanged);
      QObject::disconnect(model, &QAbstractItemModel::columnsAboutToBeMoved,    q, &QItemSelectionModel::_q_layoutAboutToBeChanged);
      QObject::disconnect(model, &QAbstractItemModel::rowsMoved,                q, &QItemSelectionModel::_q_layoutChanged);
      QObject::disconnect(model, &QAbstractItemModel::columnsMoved,             q, &QItemSelectionModel::_q_layoutChanged);
      QObject::disconnect(model, &QAbstractItemModel::layoutAboutToBeChanged,   q, &QItemSelectionModel::_q_layoutAboutToBeChanged);
      QObject::disconnect(model, &QAbstractItemModel::layoutChanged,            q, &QItemSelectionModel::_q_layoutChanged);
      QObject::disconnect(model, &QAbstractItemModel::modelReset,               q, &QItemSelectionModel::reset);
   }

   this->model = model;

   if (model) {
      QObject::connect(model, &QAbstractItemModel::rowsAboutToBeRemoved,     q, &QItemSelectionModel::_q_rowsAboutToBeRemoved);
      QObject::connect(model, &QAbstractItemModel::columnsAboutToBeRemoved,  q, &QItemSelectionModel::_q_columnsAboutToBeRemoved);
      QObject::connect(model, &QAbstractItemModel::rowsAboutToBeInserted,    q, &QItemSelectionModel::_q_rowsAboutToBeInserted);
      QObject::connect(model, &QAbstractItemModel::columnsAboutToBeInserted, q, &QItemSelectionModel::_q_columnsAboutToBeInserted);
      QObject::connect(model, &QAbstractItemModel::rowsAboutToBeMoved,       q, &QItemSelectionModel::_q_layoutAboutToBeChanged);
      QObject::connect(model, &QAbstractItemModel::columnsAboutToBeMoved,    q, &QItemSelectionModel::_q_layoutAboutToBeChanged);
      QObject::connect(model, &QAbstractItemModel::rowsMoved,                q, &QItemSelectionModel::_q_layoutChanged);
      QObject::connect(model, &QAbstractItemModel::columnsMoved,             q, &QItemSelectionModel::_q_layoutChanged);
      QObject::connect(model, &QAbstractItemModel::layoutAboutToBeChanged,   q, &QItemSelectionModel::_q_layoutAboutToBeChanged);
      QObject::connect(model, &QAbstractItemModel::layoutChanged,            q, &QItemSelectionModel::_q_layoutChanged);
      QObject::connect(model, &QAbstractItemModel::modelReset,               q, &QItemSelectionModel::reset);
   }
}

QItemSelection QItemSelectionModelPrivate::expandSelection(const QItemSelection &selection,
   QItemSelectionModel::SelectionFlags command) const
{
   if (selection.isEmpty() && !((command & QItemSelectionModel::Rows) ||
         (command & QItemSelectionModel::Columns))) {
      return selection;
   }

   QItemSelection expanded;

   if (command & QItemSelectionModel::Rows) {
      for (int i = 0; i < selection.count(); ++i) {
         QModelIndex parent = selection.at(i).parent();
         int colCount = model->columnCount(parent);
         QModelIndex tl = model->index(selection.at(i).top(), 0, parent);
         QModelIndex br = model->index(selection.at(i).bottom(), colCount - 1, parent);

         //we need to merge because the same row could have already been inserted
         expanded.merge(QItemSelection(tl, br), QItemSelectionModel::Select);
      }
   }

   if (command & QItemSelectionModel::Columns) {
      for (int i = 0; i < selection.count(); ++i) {
         QModelIndex parent = selection.at(i).parent();
         int rowCount = model->rowCount(parent);
         QModelIndex tl = model->index(0, selection.at(i).left(), parent);
         QModelIndex br = model->index(rowCount - 1, selection.at(i).right(), parent);
         //we need to merge because the same column could have already been inserted
         expanded.merge(QItemSelection(tl, br), QItemSelectionModel::Select);
      }
   }
   return expanded;
}

void QItemSelectionModelPrivate::_q_rowsAboutToBeRemoved(const QModelIndex &parent,
   int start, int end)
{
   Q_Q(QItemSelectionModel);
   finalize();

   // update current index
   if (currentIndex.isValid() && parent == currentIndex.parent()
      && currentIndex.row() >= start && currentIndex.row() <= end) {
      QModelIndex old = currentIndex;

      if (start > 0) {
         // there are rows left above the change
         currentIndex = model->index(start - 1, old.column(), parent);
      } else if (model && end < model->rowCount(parent) - 1) {
         // there are rows left below the change
         currentIndex = model->index(end + 1, old.column(), parent);
      } else {
         // there are no rows left in the table
         currentIndex = QModelIndex();
      }

      emit q->currentChanged(currentIndex, old);
      emit q->currentRowChanged(currentIndex, old);
      if (currentIndex.column() != old.column()) {
         emit q->currentColumnChanged(currentIndex, old);
      }
   }

   QItemSelection deselected;
   QItemSelection newParts;
   QItemSelection::iterator it = ranges.begin();

   while (it != ranges.end()) {
      if (it->topLeft().parent() != parent) {  // Check parents until reaching root or contained in range
         QModelIndex itParent = it->topLeft().parent();
         while (itParent.isValid() && itParent.parent() != parent) {
            itParent = itParent.parent();
         }

         if (itParent.isValid() && start <= itParent.row() && itParent.row() <= end) {
            deselected.append(*it);
            it = ranges.erase(it);
         } else {
            ++it;
         }
      } else if (start <= it->bottom() && it->bottom() <= end    // Full inclusion
         && start <= it->top() && it->top() <= end) {
         deselected.append(*it);
         it = ranges.erase(it);

      } else if (start <= it->top() && it->top() <= end) {      // Top intersection
         deselected.append(QItemSelectionRange(it->topLeft(), model->index(end, it->right(), it->parent())));
         *it = QItemSelectionRange(model->index(end + 1, it->left(), it->parent()), it->bottomRight());
         ++it;

      } else if (start <= it->bottom() && it->bottom() <= end) {    // Bottom intersection
         deselected.append(QItemSelectionRange(model->index(start, it->left(), it->parent()), it->bottomRight()));
         *it = QItemSelectionRange(it->topLeft(), model->index(start - 1, it->right(), it->parent()));
         ++it;

      } else if (it->top() < start && end < it->bottom()) { // Middle intersection
         // If the parent contains (1, 2, 3, 4, 5, 6, 7, 8) and [3, 4, 5, 6] is selected,
         // and [4, 5] is removed, we need to split [3, 4, 5, 6] into [3], [4, 5] and [6].
         // [4, 5] is appended to deselected, and [3] and [6] remain part of the selection
         // in ranges.
         const QItemSelectionRange removedRange(model->index(start, it->left(), it->parent()),
            model->index(end, it->right(), it->parent()));
         deselected.append(removedRange);
         QItemSelection::split(*it, removedRange, &newParts);
         it = ranges.erase(it);

      } else {
         ++it;
      }
   }
   ranges.append(newParts);

   if (!deselected.isEmpty()) {
      emit q->selectionChanged(QItemSelection(), deselected);
   }
}

void QItemSelectionModelPrivate::_q_columnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
   Q_Q(QItemSelectionModel);

   // update current index
   if (currentIndex.isValid() && parent == currentIndex.parent()
      && currentIndex.column() >= start && currentIndex.column() <= end) {

      QModelIndex old = currentIndex;

      if (start > 0) {
         // there are columns to the left of the change
         currentIndex = model->index(old.row(), start - 1, parent);

      } else if (model && end < model->columnCount() - 1) {
         // there are columns to the right of the change
         currentIndex = model->index(old.row(), end + 1, parent);

      } else {
         // there are no columns left in the table
         currentIndex = QModelIndex();
      }
      emit q->currentChanged(currentIndex, old);
      if (currentIndex.row() != old.row()) {
         emit q->currentRowChanged(currentIndex, old);
      }
      emit q->currentColumnChanged(currentIndex, old);
   }

   // update selections
   QModelIndex tl = model->index(0, start, parent);
   QModelIndex br = model->index(model->rowCount(parent) - 1, end, parent);
   q->select(QItemSelection(tl, br), QItemSelectionModel::Deselect);
   finalize();
}

void QItemSelectionModelPrivate::_q_columnsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
   (void) end;

   finalize();
   QList<QItemSelectionRange> split;
   QList<QItemSelectionRange>::iterator it = ranges.begin();

   for (; it != ranges.end(); ) {
      if ((*it).isValid() && (*it).parent() == parent
         && (*it).left() < start && (*it).right() >= start) {
         QModelIndex bottomMiddle = model->index((*it).bottom(), start - 1, (*it).parent());
         QItemSelectionRange left((*it).topLeft(), bottomMiddle);
         QModelIndex topMiddle = model->index((*it).top(), start, (*it).parent());
         QItemSelectionRange right(topMiddle, (*it).bottomRight());
         it = ranges.erase(it);
         split.append(left);
         split.append(right);
      } else {
         ++it;
      }
   }
   ranges += split;
}

void QItemSelectionModelPrivate::_q_rowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
   (void) end;

   finalize();
   QList<QItemSelectionRange> split;
   QList<QItemSelectionRange>::iterator it = ranges.begin();
   for (; it != ranges.end(); ) {
      if ((*it).isValid() && (*it).parent() == parent
         && (*it).top() < start && (*it).bottom() >= start) {
         QModelIndex middleRight = model->index(start - 1, (*it).right(), (*it).parent());
         QItemSelectionRange top((*it).topLeft(), middleRight);
         QModelIndex middleLeft = model->index(start, (*it).left(), (*it).parent());
         QItemSelectionRange bottom(middleLeft, (*it).bottomRight());
         it = ranges.erase(it);
         split.append(top);
         split.append(bottom);
      } else {
         ++it;
      }
   }
   ranges += split;
}

void QItemSelectionModelPrivate::_q_layoutAboutToBeChanged(const QList<QPersistentModelIndex> &,
   QAbstractItemModel::LayoutChangeHint hint)
{
   savedPersistentIndexes.clear();
   savedPersistentCurrentIndexes.clear();
   savedPersistentRowLengths.clear();
   savedPersistentCurrentRowLengths.clear();

   // optimization for when all indexes are selected
   // (only if there is lots of items (1000) because this is not entirely correct)
   if (ranges.isEmpty() && currentSelection.count() == 1) {
      QItemSelectionRange range = currentSelection.first();
      QModelIndex parent = range.parent();
      tableRowCount = model->rowCount(parent);
      tableColCount = model->columnCount(parent);
      if (tableRowCount * tableColCount > 1000
         && range.top() == 0
         && range.left() == 0
         && range.bottom() == tableRowCount - 1
         && range.right() == tableColCount - 1) {
         tableSelected = true;
         tableParent = parent;
         return;
      }
   }
   tableSelected = false;

   if (hint == QAbstractItemModel::VerticalSortHint) {
      savedPersistentRowLengths = qSelectionPersistentRowLengths(ranges);
      savedPersistentCurrentRowLengths = qSelectionPersistentRowLengths(currentSelection);

   } else {
      savedPersistentIndexes = qSelectionPersistentindexes(ranges);
      savedPersistentCurrentIndexes = qSelectionPersistentindexes(currentSelection);
   }
}
static QItemSelection mergeRowLengths(const QVector<QPair<QPersistentModelIndex, uint>> &rowLengths)
{
   if (rowLengths.isEmpty()) {
      return QItemSelection();
   }
   QItemSelection result;
   int i = 0;
   while (i < rowLengths.count()) {
      const QPersistentModelIndex &tl = rowLengths.at(i).first;
      if (!tl.isValid()) {
         ++i;
         continue;
      }
      QPersistentModelIndex br = tl;
      const uint length = rowLengths.at(i).second;
      while (++i < rowLengths.count()) {
         const QPersistentModelIndex &next = rowLengths.at(i).first;
         if (!next.isValid()) {
            continue;
         }
         const uint nextLength = rowLengths.at(i).second;
         if ((nextLength == length)
            && (next.row() == br.row() + 1)
            && (next.parent() == br.parent())) {
            br = next;
         } else {
            break;
         }
      }
      result.append(QItemSelectionRange(tl, br.sibling(br.row(),  length - 1)));
   }
   return result;
}
/*!
    \internal

    Merges \a indexes into an item selection made up of ranges.
    Assumes that the indexes are sorted.
*/
static QItemSelection mergeIndexes(const QVector<QPersistentModelIndex> &indexes)
{
   QItemSelection colSpans;
   // merge columns
   int i = 0;
   while (i < indexes.count()) {
      const QPersistentModelIndex &tl = indexes.at(i);
      if (!tl.isValid()) {
         ++i;
         continue;
      }
      QPersistentModelIndex br = tl;
      QModelIndex brParent = br.parent();
      int brRow = br.row();
      int brColumn = br.column();
      while (++i < indexes.count()) {
         const QPersistentModelIndex &next = indexes.at(i);
         if (!next.isValid()) {
            continue;
         }
         const QModelIndex nextParent = next.parent();
         const int nextRow = next.row();
         const int nextColumn = next.column();
         if ((nextParent == brParent)
            && (nextRow == brRow)
            && (nextColumn == brColumn + 1)) {
            br = next;
            brParent = nextParent;
            brRow = nextRow;
            brColumn = nextColumn;
         } else {
            break;
         }
      }
      colSpans.append(QItemSelectionRange(tl, br));
   }
   // merge rows
   QItemSelection rowSpans;
   i = 0;
   while (i < colSpans.count()) {
      QModelIndex tl = colSpans.at(i).topLeft();
      QModelIndex br = colSpans.at(i).bottomRight();
      QModelIndex prevTl = tl;
      while (++i < colSpans.count()) {
         QModelIndex nextTl = colSpans.at(i).topLeft();
         QModelIndex nextBr = colSpans.at(i).bottomRight();

         if (nextTl.parent() != tl.parent()) {
            break;   // we can't merge selection ranges from different parents
         }

         if ((nextTl.column() == prevTl.column()) && (nextBr.column() == br.column())
            && (nextTl.row() == prevTl.row() + 1) && (nextBr.row() == br.row() + 1)) {
            br = nextBr;
            prevTl = nextTl;
         } else {
            break;
         }
      }
      rowSpans.append(QItemSelectionRange(tl, br));
   }
   return rowSpans;
}

static bool qt_PersistentModelIndexLessThan(const QPersistentModelIndex &i1, const QPersistentModelIndex &i2)
{
   const QModelIndex parent1 = i1.parent();
   const QModelIndex parent2 = i2.parent();
   return parent1 == parent2 ? i1 < i2 : parent1 < parent2;
}

void QItemSelectionModelPrivate::_q_layoutChanged(const QList<QPersistentModelIndex> &, QAbstractItemModel::LayoutChangeHint hint)
{
   // special case for when all indexes are selected
   if (tableSelected && tableColCount == model->columnCount(tableParent)
      && tableRowCount == model->rowCount(tableParent)) {
      ranges.clear();
      currentSelection.clear();
      int bottom = tableRowCount - 1;
      int right = tableColCount - 1;
      QModelIndex tl = model->index(0, 0, tableParent);
      QModelIndex br = model->index(bottom, right, tableParent);
      currentSelection << QItemSelectionRange(tl, br);
      tableParent = QModelIndex();
      tableSelected = false;
      return;
   }

   if ((hint != QAbstractItemModel::VerticalSortHint && savedPersistentCurrentIndexes.isEmpty() && savedPersistentIndexes.isEmpty())
      || (hint == QAbstractItemModel::VerticalSortHint && savedPersistentRowLengths.isEmpty() &&
         savedPersistentCurrentRowLengths.isEmpty())) {
      // either the selection was actually empty, or we
      // didn't get the layoutAboutToBeChanged() signal
      return;
   }
   // clear the "old" selection
   ranges.clear();
   currentSelection.clear();

   if (hint != QAbstractItemModel::VerticalSortHint) {
      // sort the "new" selection, as preparation for merging
      std::stable_sort(savedPersistentIndexes.begin(), savedPersistentIndexes.end(),
         qt_PersistentModelIndexLessThan);

      std::stable_sort(savedPersistentCurrentIndexes.begin(), savedPersistentCurrentIndexes.end(),
         qt_PersistentModelIndexLessThan);

      // update the selection by merging the individual indexes
      ranges = mergeIndexes(savedPersistentIndexes);
      currentSelection = mergeIndexes(savedPersistentCurrentIndexes);

      // release the persistent indexes
      savedPersistentIndexes.clear();
      savedPersistentCurrentIndexes.clear();
   } else {
      std::stable_sort(savedPersistentRowLengths.begin(), savedPersistentRowLengths.end());
      std::stable_sort(savedPersistentCurrentRowLengths.begin(), savedPersistentCurrentRowLengths.end());
      ranges = mergeRowLengths(savedPersistentRowLengths);
      currentSelection = mergeRowLengths(savedPersistentCurrentRowLengths);
      savedPersistentRowLengths.clear();
      savedPersistentCurrentRowLengths.clear();
   }
}

QItemSelectionModel::QItemSelectionModel(QAbstractItemModel *model)
   : QObject(model), d_ptr(new QItemSelectionModelPrivate)
{
   d_ptr->q_ptr = this;
   d_func()->initModel(model);
}

QItemSelectionModel::QItemSelectionModel(QAbstractItemModel *model, QObject *parent)
   : QObject(parent), d_ptr(new QItemSelectionModelPrivate)
{
   d_ptr->q_ptr = this;
   d_func()->initModel(model);
}

QItemSelectionModel::QItemSelectionModel(QItemSelectionModelPrivate &dd, QAbstractItemModel *model)
   : QObject(model), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   dd.initModel(model);
}

QItemSelectionModel::~QItemSelectionModel()
{
}

void QItemSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
   QItemSelection selection(index, index);
   select(selection, command);
}

void QItemSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
   Q_D(QItemSelectionModel);

   if (! d->model) {
      qWarning("QItemSelectionModel::select() No model is set, no items will be selected");
      return;
   }

   if (command == NoUpdate) {
      return;
   }

   // store old selection
   QItemSelection sel = selection;

   // If d->ranges is non-empty when the source model is reset the persistent indexes
   // it contains will be invalid. We can't clear them in a modelReset slot because that might already
   // be too late if another model observer is connected to the same modelReset slot and is invoked first
   // it might call select() on this selection model before any such QItemSelectionModelPrivate::_q_modelReset() slot
   // is invoked, so it would not be cleared yet. We clear it invalid ranges in it here.

   QItemSelection::iterator it = d->ranges.begin();

   while (it != d->ranges.end()) {
      if (!it->isValid()) {
         it = d->ranges.erase(it);
      } else {
         ++it;
      }
   }

   QItemSelection old = d->ranges;
   old.merge(d->currentSelection, d->currentCommand);

   // expand selection according to SelectionBehavior
   if (command & Rows || command & Columns) {
      sel = d->expandSelection(sel, command);
   }

   // clear ranges and currentSelection
   if (command & Clear) {
      d->ranges.clear();
      d->currentSelection.clear();
   }

   // merge and clear currentSelection if Current was not set (ie. start new currentSelection)
   if (!(command & Current)) {
      d->finalize();
   }

   // update currentSelection
   if (command & Toggle || command & Select || command & Deselect) {
      d->currentCommand = command;
      d->currentSelection = sel;
   }

   // generate new selection, compare with old and emit selectionChanged()
   QItemSelection newSelection = d->ranges;
   newSelection.merge(d->currentSelection, d->currentCommand);
   emitSelectionChanged(newSelection, old);
}

void QItemSelectionModel::clear()
{
   clearSelection();
   clearCurrentIndex();
}
void QItemSelectionModel::clearCurrentIndex()
{
   Q_D(QItemSelectionModel);
   QModelIndex previous = d->currentIndex;
   d->currentIndex = QModelIndex();
   if (previous.isValid()) {
      emit currentChanged(d->currentIndex, previous);
      emit currentRowChanged(d->currentIndex, previous);
      emit currentColumnChanged(d->currentIndex, previous);
   }
}

void QItemSelectionModel::reset()
{
   bool block = blockSignals(true);
   clear();
   blockSignals(block);
}

void QItemSelectionModel::clearSelection()
{
   Q_D(QItemSelectionModel);

   if (d->ranges.count() == 0 && d->currentSelection.count() == 0) {
      return;
   }

   select(QItemSelection(), Clear);
}

void QItemSelectionModel::setCurrentIndex(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
   Q_D(QItemSelectionModel);

   if (!d->model) {
      qWarning("QItemSelectionModel::setCurrentIndex() Unable to set the current index without a model");
      return;
   }

   if (index == d->currentIndex) {
      if (command != NoUpdate) {
         select(index, command);   // select item
      }
      return;
   }

   QPersistentModelIndex previous = d->currentIndex;
   d->currentIndex = index; // set current before emitting selection changed below
   if (command != NoUpdate) {
      select(d->currentIndex, command);   // select item
   }

   emit currentChanged(d->currentIndex, previous);

   if (d->currentIndex.row() != previous.row() ||
      d->currentIndex.parent() != previous.parent()) {
      emit currentRowChanged(d->currentIndex, previous);
   }

   if (d->currentIndex.column() != previous.column() ||
      d->currentIndex.parent() != previous.parent()) {
      emit currentColumnChanged(d->currentIndex, previous);
   }
}

QModelIndex QItemSelectionModel::currentIndex() const
{
   return static_cast<QModelIndex>(d_func()->currentIndex);
}

bool QItemSelectionModel::isSelected(const QModelIndex &index) const
{
   Q_D(const QItemSelectionModel);
   if (d->model != index.model() || !index.isValid()) {
      return false;
   }

   bool selected = false;
   //  search model ranges
   QList<QItemSelectionRange>::const_iterator it = d->ranges.begin();
   for (; it != d->ranges.end(); ++it) {
      if ((*it).isValid() && (*it).contains(index)) {
         selected = true;
         break;
      }
   }

   // check  currentSelection
   if (d->currentSelection.count()) {
      if ((d->currentCommand & Deselect) && selected) {
         selected = !d->currentSelection.contains(index);
      } else if (d->currentCommand & Toggle) {
         selected ^= d->currentSelection.contains(index);
      } else if ((d->currentCommand & Select) && !selected) {
         selected = d->currentSelection.contains(index);
      }
   }

   if (selected) {
      Qt::ItemFlags flags = d->model->flags(index);
      return (flags & Qt::ItemIsSelectable);
   }

   return false;
}

bool QItemSelectionModel::isRowSelected(int row, const QModelIndex &parent) const
{
   Q_D(const QItemSelectionModel);

   if (!d->model) {
      return false;
   }

   if (parent.isValid() && d->model != parent.model()) {
      return false;
   }

   // return false if row exist in currentSelection (Deselect)
   if (d->currentCommand & Deselect && d->currentSelection.count()) {
      for (int i = 0; i < d->currentSelection.count(); ++i) {
         if (d->currentSelection.at(i).parent() == parent &&
            row >= d->currentSelection.at(i).top() &&
            row <= d->currentSelection.at(i).bottom()) {
            return false;
         }
      }
   }
   // return false if ranges in both currentSelection and ranges
   // intersect and have the same row contained
   if (d->currentCommand & Toggle && d->currentSelection.count()) {
      for (int i = 0; i < d->currentSelection.count(); ++i)
         if (d->currentSelection.at(i).top() <= row &&
            d->currentSelection.at(i).bottom() >= row)
            for (int j = 0; j < d->ranges.count(); ++j)
               if (d->ranges.at(j).top() <= row && d->ranges.at(j).bottom() >= row
                  && d->currentSelection.at(i).intersected(d->ranges.at(j)).isValid()) {
                  return false;
               }
   }
   // add ranges and currentSelection and check through them all
   QList<QItemSelectionRange>::const_iterator it;
   QList<QItemSelectionRange> joined = d->ranges;
   if (d->currentSelection.count()) {
      joined += d->currentSelection;
   }
   int colCount = d->model->columnCount(parent);
   for (int column = 0; column < colCount; ++column) {
      for (it = joined.constBegin(); it != joined.constEnd(); ++it) {
         if ((*it).contains(row, column, parent)) {
            bool selectable = false;
            for (int i = column; !selectable && i <= (*it).right(); ++i) {
               Qt::ItemFlags flags = d->model->index(row, i, parent).flags();
               selectable = flags & Qt::ItemIsSelectable;
            }
            if (selectable) {
               column = qMax(column, (*it).right());
               break;
            }
         }
      }
      if (it == joined.constEnd()) {
         return false;
      }
   }
   return colCount > 0; // no columns means no selected items
}

bool QItemSelectionModel::isColumnSelected(int column, const QModelIndex &parent) const
{
   Q_D(const QItemSelectionModel);

   if (!d->model) {
      return false;
   }

   if (parent.isValid() && d->model != parent.model()) {
      return false;
   }

   // return false if column exist in currentSelection (Deselect)
   if (d->currentCommand & Deselect && d->currentSelection.count()) {
      for (int i = 0; i < d->currentSelection.count(); ++i) {
         if (d->currentSelection.at(i).parent() == parent &&
            column >= d->currentSelection.at(i).left() &&
            column <= d->currentSelection.at(i).right()) {
            return false;
         }
      }
   }
   // return false if ranges in both currentSelection and the selection model
   // intersect and have the same column contained
   if (d->currentCommand & Toggle && d->currentSelection.count()) {
      for (int i = 0; i < d->currentSelection.count(); ++i) {
         if (d->currentSelection.at(i).left() <= column &&
            d->currentSelection.at(i).right() >= column) {
            for (int j = 0; j < d->ranges.count(); ++j) {
               if (d->ranges.at(j).left() <= column && d->ranges.at(j).right() >= column
                  && d->currentSelection.at(i).intersected(d->ranges.at(j)).isValid()) {
                  return false;
               }
            }
         }
      }
   }
   // add ranges and currentSelection and check through them all
   QList<QItemSelectionRange>::const_iterator it;
   QList<QItemSelectionRange> joined = d->ranges;
   if (d->currentSelection.count()) {
      joined += d->currentSelection;
   }
   int rowCount = d->model->rowCount(parent);
   for (int row = 0; row < rowCount; ++row) {
      for (it = joined.constBegin(); it != joined.constEnd(); ++it) {
         if ((*it).contains(row, column, parent)) {
            Qt::ItemFlags flags = d->model->index(row, column, parent).flags();
            if ((flags & Qt::ItemIsSelectable) && (flags & Qt::ItemIsEnabled)) {
               row = qMax(row, (*it).bottom());
               break;
            }
         }
      }
      if (it == joined.constEnd()) {
         return false;
      }
   }
   return rowCount > 0; // no rows means no selected items
}

bool QItemSelectionModel::rowIntersectsSelection(int row, const QModelIndex &parent) const
{
   Q_D(const QItemSelectionModel);
   if (!d->model) {
      return false;
   }
   if (parent.isValid() && d->model != parent.model()) {
      return false;
   }

   QItemSelection sel = d->ranges;
   sel.merge(d->currentSelection, d->currentCommand);
   for (int i = 0; i < sel.count(); ++i) {
      QItemSelectionRange range = sel.at(i);
      if (range.parent() != parent) {
         return false;
      }
      int top = range.top();
      int bottom = range.bottom();
      int left = range.left();
      int right = range.right();
      if (top <= row && bottom >= row) {
         for (int j = left; j <= right; j++) {
            const Qt::ItemFlags flags = d->model->index(row, j, parent).flags();
            if ((flags & Qt::ItemIsSelectable) && (flags & Qt::ItemIsEnabled)) {
               return true;
            }
         }
      }
   }

   return false;
}

bool QItemSelectionModel::columnIntersectsSelection(int column, const QModelIndex &parent) const
{
   Q_D(const QItemSelectionModel);
   if (!d->model) {
      return false;
   }
   if (parent.isValid() && d->model != parent.model()) {
      return false;
   }

   QItemSelection sel = d->ranges;
   sel.merge(d->currentSelection, d->currentCommand);
   for (int i = 0; i < sel.count(); ++i) {
      int left = sel.at(i).left();
      int right = sel.at(i).right();
      int top =  sel.at(i).top();
      int bottom =  sel.at(i).bottom();
      if (left <= column && right >= column) {
         for (int j = top; j <= bottom; j++) {
            const Qt::ItemFlags flags = d->model->index(j, column, parent).flags();
            if ((flags & Qt::ItemIsSelectable) && (flags & Qt::ItemIsEnabled)) {
               return true;
            }
         }
      }
   }

   return false;
}

bool QItemSelectionModel::hasSelection() const
{
   Q_D(const QItemSelectionModel);
   if (d->currentCommand & (Toggle | Deselect)) {
      QItemSelection sel = d->ranges;
      sel.merge(d->currentSelection, d->currentCommand);
      return !sel.isEmpty();
   } else {
      return !(d->ranges.isEmpty() && d->currentSelection.isEmpty());
   }
}

QModelIndexList QItemSelectionModel::selectedIndexes() const
{
   Q_D(const QItemSelectionModel);
   QItemSelection selected = d->ranges;
   selected.merge(d->currentSelection, d->currentCommand);
   return selected.indexes();
}

QModelIndexList QItemSelectionModel::selectedRows(int column) const
{
   QModelIndexList indexes;

   //the QSet contains pairs of parent modelIndex and row number
   QSet< QPair<QModelIndex, int>> rowsSeen;

   const QItemSelection ranges = selection();
   for (int i = 0; i < ranges.count(); ++i) {
      const QItemSelectionRange &range = ranges.at(i);
      QModelIndex parent = range.parent();
      for (int row = range.top(); row <= range.bottom(); row++) {
         QPair<QModelIndex, int> rowDef = qMakePair(parent, row);
         if (!rowsSeen.contains(rowDef)) {
            rowsSeen << rowDef;
            if (isRowSelected(row, parent)) {
               indexes.append(model()->index(row, column, parent));
            }
         }
      }
   }

   return indexes;
}

QModelIndexList QItemSelectionModel::selectedColumns(int row) const
{
   QModelIndexList indexes;

   //the QSet contains pairs of parent modelIndex and column number
   QSet< QPair<QModelIndex, int>> columnsSeen;

   const QItemSelection ranges = selection();
   for (int i = 0; i < ranges.count(); ++i) {
      const QItemSelectionRange &range = ranges.at(i);
      QModelIndex parent = range.parent();
      for (int column = range.left(); column <= range.right(); column++) {
         QPair<QModelIndex, int> columnDef = qMakePair(parent, column);
         if (!columnsSeen.contains(columnDef)) {
            columnsSeen << columnDef;
            if (isColumnSelected(column, parent)) {
               indexes.append(model()->index(row, column, parent));
            }
         }
      }
   }

   return indexes;
}

QItemSelection QItemSelectionModel::selection() const
{
   Q_D(const QItemSelectionModel);
   QItemSelection selected = d->ranges;
   selected.merge(d->currentSelection, d->currentCommand);

   int i = 0;

   // make sure we have no invalid ranges
   // should probably be handled more generic somewhere else

   while (i < selected.count()) {
      if (selected.at(i).isValid()) {
         ++i;
      } else {
         (selected.removeAt(i));
      }
   }

   return selected;
}

const QAbstractItemModel *QItemSelectionModel::model() const
{
   return d_func()->model;
}

void QItemSelectionModel::setModel(QAbstractItemModel *model)
{
   Q_D(QItemSelectionModel);
   if (d->model == model) {
      return;
   }
   d->initModel(model);
   emit modelChanged(model);
}

void QItemSelectionModel::emitSelectionChanged(const QItemSelection &newSelection,
   const QItemSelection &oldSelection)
{
   // if both selections are empty or equal we return
   if ((oldSelection.isEmpty() && newSelection.isEmpty()) ||
      oldSelection == newSelection) {
      return;
   }

   // if either selection is empty we do not need to compare
   if (oldSelection.isEmpty() || newSelection.isEmpty()) {
      emit selectionChanged(newSelection, oldSelection);
      return;
   }

   QItemSelection deselected = oldSelection;
   QItemSelection selected = newSelection;

   // remove equal ranges
   bool advance;
   for (int o = 0; o < deselected.count(); ++o) {
      advance = true;
      for (int s = 0; s < selected.count() && o < deselected.count();) {
         if (deselected.at(o) == selected.at(s)) {
            deselected.removeAt(o);
            selected.removeAt(s);
            advance = false;
         } else {
            ++s;
         }
      }
      if (advance) {
         ++o;
      }
   }

   // find intersections
   QItemSelection intersections;
   for (int o = 0; o < deselected.count(); ++o) {
      for (int s = 0; s < selected.count(); ++s) {
         if (deselected.at(o).intersects(selected.at(s))) {
            intersections.append(deselected.at(o).intersected(selected.at(s)));
         }
      }
   }

   // compare remaining ranges with intersections and split them to find deselected and selected
   for (int i = 0; i < intersections.count(); ++i) {
      // split deselected
      for (int o = 0; o < deselected.count();) {
         if (deselected.at(o).intersects(intersections.at(i))) {
            QItemSelection::split(deselected.at(o), intersections.at(i), &deselected);
            deselected.removeAt(o);
         } else {
            ++o;
         }
      }

      // split selected
      for (int s = 0; s < selected.count();) {
         if (selected.at(s).intersects(intersections.at(i))) {
            QItemSelection::split(selected.at(s), intersections.at(i), &selected);
            selected.removeAt(s);
         } else {
            ++s;
         }
      }
   }

   if (!selected.isEmpty() || !deselected.isEmpty()) {
      emit selectionChanged(selected, deselected);
   }
}

QDebug operator<<(QDebug debug, const QItemSelectionRange &range)
{
   QDebugStateSaver saver(debug);
   debug.nospace();

   debug << "QItemSelectionRange(" << range.topLeft() << ", " << range.bottomRight() << ')';

   return debug;
}

void QItemSelectionModel::_q_columnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
   Q_D(QItemSelectionModel);
   d->_q_columnsAboutToBeRemoved(parent, start, end);
}

void QItemSelectionModel::_q_rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
   Q_D(QItemSelectionModel);
   d->_q_rowsAboutToBeRemoved(parent, start, end);
}

void QItemSelectionModel::_q_columnsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
   Q_D(QItemSelectionModel);
   d->_q_columnsAboutToBeInserted(parent, start, end);
}

void QItemSelectionModel::_q_rowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
   Q_D(QItemSelectionModel);
   d->_q_rowsAboutToBeInserted(parent, start, end);
}

void QItemSelectionModel::_q_layoutAboutToBeChanged()
{
   Q_D(QItemSelectionModel);
   d->_q_layoutAboutToBeChanged();
}

void QItemSelectionModel::_q_layoutChanged()
{
   Q_D(QItemSelectionModel);
   d->_q_layoutChanged();
}

#endif // QT_NO_ITEMVIEWS
