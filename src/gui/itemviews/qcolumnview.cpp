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

#include <qglobal.h>

#ifndef QT_NO_COLUMNVIEW

#include <qcolumnview.h>
#include <qcolumnview_p.h>
#include <qcolumnviewgrip_p.h>

#include <qlistview.h>
#include <qabstractitemdelegate.h>
#include <qscrollbar.h>
#include <qpainter.h>
#include <qdebug.h>

#define ANIMATION_DURATION_MSEC 150

QColumnView::QColumnView(QWidget *parent)
   :  QAbstractItemView(*new QColumnViewPrivate, parent)
{
   Q_D(QColumnView);
   d->initialize();
}

QColumnView::QColumnView(QColumnViewPrivate &dd, QWidget *parent)
   :  QAbstractItemView(dd, parent)
{
   Q_D(QColumnView);
   d->initialize();
}

void QColumnViewPrivate::initialize()
{
   Q_Q(QColumnView);
   q->setTextElideMode(Qt::ElideMiddle);

#ifndef QT_NO_ANIMATION
   QObject::connect(&currentAnimation, &QPropertyAnimation::finished, q, &QColumnView::_q_changeCurrentColumn);

   currentAnimation.setDuration(ANIMATION_DURATION_MSEC);
   currentAnimation.setTargetObject(hbar);
   currentAnimation.setPropertyName("value");
   currentAnimation.setEasingCurve(QEasingCurve::InOutQuad);
#endif

   delete itemDelegate;
   q->setItemDelegate(new QColumnViewDelegate(q));
}

QColumnView::~QColumnView()
{
}

void QColumnView::setResizeGripsVisible(bool visible)
{
   Q_D(QColumnView);
   if (d->showResizeGrips == visible) {
      return;
   }

   d->showResizeGrips = visible;

   for (int i = 0; i < d->columns.count(); ++i) {
      QAbstractItemView *view = d->columns[i];

      if (visible) {
         QColumnViewGrip *grip = new QColumnViewGrip(view);
         view->setCornerWidget(grip);
         connect(grip, &QColumnViewGrip::gripMoved, this, &QColumnView::_q_gripMoved);

      } else {
         QWidget *widget = view->cornerWidget();
         view->setCornerWidget(nullptr);
         widget->deleteLater();
      }
   }
}

bool QColumnView::resizeGripsVisible() const
{
   Q_D(const QColumnView);
   return d->showResizeGrips;
}

void QColumnView::setModel(QAbstractItemModel *model)
{
   Q_D(QColumnView);
   if (model == d->model) {
      return;
   }
   d->closeColumns();
   QAbstractItemView::setModel(model);
}

void QColumnView::setRootIndex(const QModelIndex &index)
{
   Q_D(QColumnView);
   if (!model()) {
      return;
   }

   d->closeColumns();
   Q_ASSERT(d->columns.count() == 0);

   QAbstractItemView *view = d->createColumn(index, true);
   if (view->selectionModel()) {
      view->selectionModel()->deleteLater();
   }
   if (view->model()) {
      view->setSelectionModel(selectionModel());
   }

   QAbstractItemView::setRootIndex(index);
   d->updateScrollbars();
}

bool QColumnView::isIndexHidden(const QModelIndex &index) const
{
   (void) index;
   return false;
}

QModelIndex QColumnView::indexAt(const QPoint &point) const
{
   Q_D(const QColumnView);
   for (int i = 0; i < d->columns.size(); ++i) {
      QPoint topLeft = d->columns.at(i)->frameGeometry().topLeft();
      QPoint adjustedPoint(point.x() - topLeft.x(), point.y() - topLeft.y());
      QModelIndex index = d->columns.at(i)->indexAt(adjustedPoint);
      if (index.isValid()) {
         return index;
      }
   }
   return QModelIndex();
}

QRect QColumnView::visualRect(const QModelIndex &index) const
{
   if (!index.isValid()) {
      return QRect();
   }

   Q_D(const QColumnView);
   for (int i = 0; i < d->columns.size(); ++i) {
      QRect rect = d->columns.at(i)->visualRect(index);
      if (!rect.isNull()) {
         rect.translate(d->columns.at(i)->frameGeometry().topLeft());
         return rect;
      }
   }
   return QRect();
}

void QColumnView::scrollContentsBy(int dx, int dy)
{
   Q_D(QColumnView);

   if (d->columns.isEmpty() || dx == 0) {
      return;
   }

   dx = isRightToLeft() ? -dx : dx;
   for (int i = 0; i < d->columns.count(); ++i) {
      d->columns.at(i)->move(d->columns.at(i)->x() + dx, 0);
   }
   d->offset += dx;
   QAbstractItemView::scrollContentsBy(dx, dy);
}

void QColumnView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
   Q_D(QColumnView);
   (void) hint;

   if (!index.isValid() || d->columns.isEmpty()) {
      return;
   }

#ifndef QT_NO_ANIMATION
   if (d->currentAnimation.state() == QPropertyAnimation::Running) {
      return;
   }

   d->currentAnimation.stop();
#endif

   // Fill up what is needed to get to index
   d->closeColumns(index, true);

   QModelIndex indexParent = index.parent();
   // Find the left edge of the column that contains index
   int currentColumn = 0;
   int leftEdge = 0;
   while (currentColumn < d->columns.size()) {
      if (indexParent == d->columns.at(currentColumn)->rootIndex()) {
         break;
      }
      leftEdge += d->columns.at(currentColumn)->width();
      ++currentColumn;
   }

   // Don't let us scroll above the root index
   if (currentColumn == d->columns.size()) {
      return;
   }

   int indexColumn = currentColumn;
   // Find the width of what we want to show (i.e. the right edge)
   int visibleWidth = d->columns.at(currentColumn)->width();
   // We want to always try to show two columns
   if (currentColumn + 1 < d->columns.size()) {
      ++currentColumn;
      visibleWidth += d->columns.at(currentColumn)->width();
   }

   int rightEdge = leftEdge + visibleWidth;
   if (isRightToLeft()) {
      leftEdge = viewport()->width() - leftEdge;
      rightEdge = leftEdge - visibleWidth;
      qSwap(rightEdge, leftEdge);
   }

   // If it is already visible don't animate
   if (leftEdge > -horizontalOffset()
      && rightEdge <= ( -horizontalOffset() + viewport()->size().width())) {
      d->columns.at(indexColumn)->scrollTo(index);
      d->_q_changeCurrentColumn();
      return;
   }

   int newScrollbarValue = 0;
   if (isRightToLeft()) {
      if (leftEdge < 0) {
         // scroll to the right
         newScrollbarValue = viewport()->size().width() - leftEdge;
      } else {
         // scroll to the left
         newScrollbarValue = rightEdge + horizontalOffset();
      }
   } else {
      if (leftEdge > -horizontalOffset()) {
         // scroll to the right
         newScrollbarValue = rightEdge - viewport()->size().width();
      } else {
         // scroll to the left
         newScrollbarValue = leftEdge;
      }
   }

#ifndef QT_NO_ANIMATION
   if (style()->styleHint(QStyle::SH_Widget_Animate, nullptr, this)) {
      d->currentAnimation.setEndValue(newScrollbarValue);
      d->currentAnimation.start();
   } else
#endif

   {
      horizontalScrollBar()->setValue(newScrollbarValue);
   }
}

QModelIndex QColumnView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
   // the child views which have focus get to deal with this first and if
   // they don't accept it then it comes up this view and we only grip left/right
   (void) modifiers;

   if (!model()) {
      return QModelIndex();
   }

   QModelIndex current = currentIndex();
   if (isRightToLeft()) {
      if (cursorAction == MoveLeft) {
         cursorAction = MoveRight;
      } else if (cursorAction == MoveRight) {
         cursorAction = MoveLeft;
      }
   }
   switch (cursorAction) {
      case MoveLeft:
         if (current.parent().isValid() && current.parent() != rootIndex()) {
            return (current.parent());
         } else {
            return current;
         }


      case MoveRight:
         if (model()->hasChildren(current)) {
            return model()->index(0, 0, current);
         } else {
            return current.sibling(current.row() + 1, current.column());
         }


      default:
         break;
   }

   return QModelIndex();
}

void QColumnView::resizeEvent(QResizeEvent *event)
{
   Q_D(QColumnView);

   d->doLayout();
   d->updateScrollbars();

   if (!isRightToLeft()) {
      int diff = event->oldSize().width() - event->size().width();
      if (diff < 0 && horizontalScrollBar()->isVisible()
         && horizontalScrollBar()->value() == horizontalScrollBar()->maximum()) {
         horizontalScrollBar()->setMaximum(horizontalScrollBar()->maximum() + diff);
      }
   }

   QAbstractItemView::resizeEvent(event);
}

void QColumnViewPrivate::updateScrollbars()
{
   Q_Q(QColumnView);

#ifndef QT_NO_ANIMATION
   if (currentAnimation.state() == QPropertyAnimation::Running) {
      return;
   }
#endif

   // find the total horizontal length of the laid out columns
   int horizontalLength = 0;
   if (!columns.isEmpty()) {
      horizontalLength = (columns.last()->x() + columns.last()->width()) - columns.first()->x();
      if (horizontalLength <= 0) { // reverse mode
         horizontalLength = (columns.first()->x() + columns.first()->width()) - columns.last()->x();
      }
   }

   QSize viewportSize = viewport->size();

   if (horizontalLength < viewportSize.width() && hbar->value() == 0) {
      hbar->setRange(0, 0);
   } else {
      int visibleLength = qMin(horizontalLength + q->horizontalOffset(), viewportSize.width());
      int hiddenLength = horizontalLength - visibleLength;
      if (hiddenLength != hbar->maximum()) {
         hbar->setRange(0, hiddenLength);
      }
   }

   if (!columns.isEmpty()) {
      int pageStepSize = columns.at(0)->width();
      if (pageStepSize != hbar->pageStep()) {
         hbar->setPageStep(pageStepSize);
      }
   }

   bool visible = (hbar->maximum() > 0);

   if (visible != hbar->isVisible()) {
      hbar->setVisible(visible);
   }
}

int QColumnView::horizontalOffset() const
{
   Q_D(const QColumnView);
   return d->offset;
}

int QColumnView::verticalOffset() const
{
   return 0;
}

QRegion QColumnView::visualRegionForSelection(const QItemSelection &selection) const
{
   int ranges = selection.count();

   if (ranges == 0) {
      return QRect();
   }

   // Note that we use the top and bottom functions of the selection range
   // since the data is stored in rows.
   int firstRow = selection.at(0).top();
   int lastRow = selection.at(0).top();
   for (int i = 0; i < ranges; ++i) {
      firstRow = qMin(firstRow, selection.at(i).top());
      lastRow = qMax(lastRow, selection.at(i).bottom());
   }

   QModelIndex firstIdx = model()->index(qMin(firstRow, lastRow), 0, rootIndex());
   QModelIndex lastIdx = model()->index(qMax(firstRow, lastRow), 0, rootIndex());

   if (firstIdx == lastIdx) {
      return visualRect(firstIdx);
   }

   QRegion firstRegion = visualRect(firstIdx);
   QRegion lastRegion = visualRect(lastIdx);
   return firstRegion.united(lastRegion);
}

void QColumnView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
   (void) rect;
   (void) command;
}

void QColumnView::setSelectionModel(QItemSelectionModel *newSelectionModel)
{
   Q_D(const QColumnView);

   for (int i = 0; i < d->columns.size(); ++i) {
      if (d->columns.at(i)->selectionModel() == selectionModel()) {
         d->columns.at(i)->setSelectionModel(newSelectionModel);
         break;
      }
   }

   QAbstractItemView::setSelectionModel(newSelectionModel);
}

QSize QColumnView::sizeHint() const
{
   Q_D(const QColumnView);
   QSize sizeHint;
   for (int i = 0; i < d->columns.size(); ++i) {
      sizeHint += d->columns.at(i)->sizeHint();
   }
   return sizeHint.expandedTo(QAbstractItemView::sizeHint());
}

void QColumnViewPrivate::_q_gripMoved(int offset)
{
   Q_Q(QColumnView);

   QObject *grip = q->sender();
   Q_ASSERT(grip);

   if (q->isRightToLeft()) {
      offset = -1 * offset;
   }

   bool found = false;
   for (int i = 0; i < columns.size(); ++i) {
      if (!found && columns.at(i)->cornerWidget() == grip) {
         found = true;
         columnSizes[i] = columns.at(i)->width();
         if (q->isRightToLeft()) {
            columns.at(i)->move(columns.at(i)->x() + offset, 0);
         }
         continue;
      }
      if (!found) {
         continue;
      }

      int currentX = columns.at(i)->x();
      columns.at(i)->move(currentX + offset, 0);
   }

   updateScrollbars();
}

void QColumnViewPrivate::closeColumns(const QModelIndex &parent, bool build)
{
   if (columns.isEmpty()) {
      return;
   }

   bool clearAll = !parent.isValid();
   bool passThroughRoot = false;

   QList<QModelIndex> dirsToAppend;

   // Find the last column that matches the parent's tree
   int currentColumn = -1;
   QModelIndex parentIndex = parent;

   while (currentColumn == -1 && parentIndex.isValid()) {
      if (columns.isEmpty()) {
         break;
      }

      parentIndex = parentIndex.parent();
      if (root == parentIndex) {
         passThroughRoot = true;
      }

      if (!parentIndex.isValid()) {
         break;
      }

      for (int i = columns.size() - 1; i >= 0; --i) {
         if (columns.at(i)->rootIndex() == parentIndex) {
            currentColumn = i;
            break;
         }
      }
      if (currentColumn == -1) {
         dirsToAppend.append(parentIndex);
      }
   }

   // Someone wants to go to an index that can be reached without changing
   // the root index, don't allow them
   if (!clearAll && !passThroughRoot && currentColumn == -1) {
      return;
   }

   if (currentColumn == -1 && parent.isValid()) {
      currentColumn = 0;
   }

   // Optimization so we don't go deleting and then creating the same thing
   bool alreadyExists = false;
   if (build && columns.size() > currentColumn + 1) {
      bool viewingParent = (columns.at(currentColumn + 1)->rootIndex() == parent);
      bool viewingChild = (!model->hasChildren(parent)
            && !columns.at(currentColumn + 1)->rootIndex().isValid());
      if (viewingParent || viewingChild) {
         currentColumn++;
         alreadyExists = true;
      }
   }

   // Delete columns that don't match our path
   for (int i = columns.size() - 1; i > currentColumn; --i) {
      QAbstractItemView *notShownAnymore = columns.at(i);
      columns.removeAt(i);
      notShownAnymore->setVisible(false);
      if (notShownAnymore != previewColumn) {
         notShownAnymore->deleteLater();
      }
   }

   if (columns.isEmpty()) {
      offset = 0;
      updateScrollbars();
   }

   // Now fill in missing columns
   while (!dirsToAppend.isEmpty()) {
      QAbstractItemView *newView = createColumn(dirsToAppend.takeLast(), true);
      if (!dirsToAppend.isEmpty()) {
         newView->setCurrentIndex(dirsToAppend.last());
      }
   }

   if (build && !alreadyExists) {
      createColumn(parent, false);
   }
}

void QColumnViewPrivate::_q_clicked(const QModelIndex &index)
{
   Q_Q(QColumnView);
   QModelIndex parent = index.parent();
   QAbstractItemView *columnClicked = nullptr;

   for (int column = 0; column < columns.count(); ++column) {
      if (columns.at(column)->rootIndex() == parent) {
         columnClicked = columns[column];
         break;
      }
   }

   if (q->selectionModel() && columnClicked) {
      QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Current;
      if (columnClicked->selectionModel()->isSelected(index)) {
         flags |= QItemSelectionModel::Select;
      }
      q->selectionModel()->setCurrentIndex(index, flags);
   }
}

QAbstractItemView *QColumnViewPrivate::createColumn(const QModelIndex &index, bool show)
{
   Q_Q(QColumnView);
   QAbstractItemView *view = nullptr;

   if (model->hasChildren(index)) {
      view = q->createColumn(index);
      q->connect(view, &QAbstractItemView::clicked, q, &QColumnView::_q_clicked);

   } else {
      if (! previewColumn) {
         setPreviewWidget(new QWidget(q));
      }

      view = previewColumn;
      view->setMinimumWidth(qMax(view->minimumWidth(), previewWidget->minimumWidth()));
   }

   q->connect(view, &QAbstractItemView::activated,     q, &QColumnView::activated);
   q->connect(view, &QAbstractItemView::clicked,       q, &QColumnView::clicked);
   q->connect(view, &QAbstractItemView::doubleClicked, q, &QColumnView::doubleClicked);
   q->connect(view, &QAbstractItemView::entered,       q, &QColumnView::entered);
   q->connect(view, &QAbstractItemView::pressed,       q, &QColumnView::pressed);

   view->setFocusPolicy(Qt::NoFocus);
   view->setParent(viewport);
   Q_ASSERT(view);

   // Setup corner grip
   if (showResizeGrips) {
      QColumnViewGrip *grip = new QColumnViewGrip(view);
      view->setCornerWidget(grip);
      q->connect(grip, &QColumnViewGrip::gripMoved, q, &QColumnView::_q_gripMoved);
   }

   if (columnSizes.count() > columns.count()) {
      view->setGeometry(0, 0, columnSizes.at(columns.count()), viewport->height());

   } else {
      int initialWidth = view->sizeHint().width();
      if (q->isRightToLeft()) {
         view->setGeometry(viewport->width() - initialWidth, 0, initialWidth, viewport->height());
      } else {
         view->setGeometry(0, 0, initialWidth, viewport->height());
      }

      columnSizes.resize(qMax(columnSizes.count(), columns.count() + 1));
      columnSizes[columns.count()] = initialWidth;
   }
   if (!columns.isEmpty() && columns.last()->isHidden()) {
      columns.last()->setVisible(true);
   }

   columns.append(view);
   doLayout();
   updateScrollbars();
   if (show && view->isHidden()) {
      view->setVisible(true);
   }
   return view;
}

QAbstractItemView *QColumnView::createColumn(const QModelIndex &index)
{
   QListView *view = new QListView(viewport());

   initializeColumn(view);

   view->setRootIndex(index);
   if (model()->canFetchMore(index)) {
      model()->fetchMore(index);
   }

   return view;
}

void QColumnView::initializeColumn(QAbstractItemView *column) const
{
   Q_D(const QColumnView);

   column->setFrameShape(QFrame::NoFrame);
   column->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   column->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
   column->setMinimumWidth(100);
   column->setAttribute(Qt::WA_MacShowFocusRect, false);

#ifndef QT_NO_DRAGANDDROP
   column->setDragDropMode(dragDropMode());
   column->setDragDropOverwriteMode(dragDropOverwriteMode());
   column->setDropIndicatorShown(showDropIndicator());
#endif
   column->setAlternatingRowColors(alternatingRowColors());
   column->setAutoScroll(hasAutoScroll());
   column->setEditTriggers(editTriggers());
   column->setHorizontalScrollMode(horizontalScrollMode());
   column->setIconSize(iconSize());
   column->setSelectionBehavior(selectionBehavior());
   column->setSelectionMode(selectionMode());
   column->setTabKeyNavigation(tabKeyNavigation());
   column->setTextElideMode(textElideMode());
   column->setVerticalScrollMode(verticalScrollMode());

   column->setModel(model());

   // Copy the custom delegate per row
   QMapIterator<int, QPointer<QAbstractItemDelegate>> i(d->rowDelegates);
   while (i.hasNext()) {
      i.next();
      column->setItemDelegateForRow(i.key(), i.value());
   }

   // set the delegate to be the columnview delegate
   QAbstractItemDelegate *delegate = column->itemDelegate();
   column->setItemDelegate(d->itemDelegate);
   delete delegate;
}

QWidget *QColumnView::previewWidget() const
{
   Q_D(const QColumnView);
   return d->previewWidget;
}

void QColumnView::setPreviewWidget(QWidget *widget)
{
   Q_D(QColumnView);
   d->setPreviewWidget(widget);
}

void QColumnViewPrivate::setPreviewWidget(QWidget *widget)
{
   Q_Q(QColumnView);

   if (previewColumn) {
      if (!columns.isEmpty() && columns.last() == previewColumn) {
         columns.removeLast();
      }
      previewColumn->deleteLater();
   }

   QColumnViewPreviewColumn *column = new QColumnViewPreviewColumn(q);

   column->setPreviewWidget(widget);
   previewColumn = column;
   previewColumn->hide();
   previewColumn->setFrameShape(QFrame::NoFrame);
   previewColumn->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
   previewColumn->setSelectionMode(QAbstractItemView::NoSelection);
   previewColumn->setMinimumWidth(qMax(previewColumn->verticalScrollBar()->width(),
         previewColumn->minimumWidth()));
   previewWidget = widget;
   previewWidget->setParent(previewColumn->viewport());
}

void QColumnView::setColumnWidths(const QList<int> &list)
{
   Q_D(QColumnView);

   int i = 0;
   const int listCount = list.count();
   const int count = qMin(listCount, d->columns.count());

   for (; i < count; ++i) {
      d->columns.at(i)->resize(list.at(i), d->columns.at(i)->height());
      d->columnSizes[i] = list.at(i);
   }
   d->columnSizes.reserve(listCount);
   for (; i < listCount; ++i) {
      d->columnSizes.append(list.at(i));
   }
}

QList<int> QColumnView::columnWidths() const
{
   Q_D(const QColumnView);
   QList<int> list;
   for (int i = 0; i < d->columns.count(); ++i) {
      list.append(d->columnSizes.at(i));
   }
   return list;
}

void QColumnView::rowsInserted(const QModelIndex &parent, int start, int end)
{
   QAbstractItemView::rowsInserted(parent, start, end);
   d_func()->checkColumnCreation(parent);
}

void QColumnView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
   Q_D(QColumnView);

   if (!current.isValid()) {
      QAbstractItemView::currentChanged(current, previous);
      return;
   }

   QModelIndex currentParent = current.parent();
   // optimize for just moving up/down in a list where the child view doesn't change
   if (currentParent == previous.parent()
      && model()->hasChildren(current) && model()->hasChildren(previous)) {
      for (int i = 0; i < d->columns.size(); ++i) {
         if (currentParent == d->columns.at(i)->rootIndex()) {
            if (d->columns.size() > i + 1) {
               QAbstractItemView::currentChanged(current, previous);
               return;
            }
            break;
         }
      }
   }

   // Scrolling to the right we need to have an empty spot
   bool found = false;
   if (currentParent == previous) {
      for (int i = 0; i < d->columns.size(); ++i) {
         if (currentParent == d->columns.at(i)->rootIndex()) {
            found = true;
            if (d->columns.size() < i + 2) {
               d->createColumn(current, false);
            }
            break;
         }
      }
   }
   if (!found) {
      d->closeColumns(current, true);
   }

   if (!model()->hasChildren(current)) {
      emit updatePreviewWidget(current);
   }

   QAbstractItemView::currentChanged(current, previous);
}

void QColumnViewPrivate::_q_changeCurrentColumn()
{
   Q_Q(QColumnView);
   if (columns.isEmpty()) {
      return;
   }

   QModelIndex current = q->currentIndex();
   if (!current.isValid()) {
      return;
   }

   // We might have scrolled far to the left so we need to close all of the children
   closeColumns(current, true);

   // Set up the "current" column with focus
   int currentColumn = qMax(0, columns.size() - 2);
   QAbstractItemView *parentColumn = columns.at(currentColumn);
   if (q->hasFocus()) {
      parentColumn->setFocus(Qt::OtherFocusReason);
   }
   q->setFocusProxy(parentColumn);

   // find the column that is our current selection model and give it a new one.
   for (int i = 0; i < columns.size(); ++i) {
      if (columns.at(i)->selectionModel() == q->selectionModel()) {
         QItemSelectionModel *replacementSelectionModel =
            new QItemSelectionModel(parentColumn->model());
         replacementSelectionModel->setCurrentIndex(
            q->selectionModel()->currentIndex(), QItemSelectionModel::Current);
         replacementSelectionModel->select(
            q->selectionModel()->selection(), QItemSelectionModel::Select);
         QAbstractItemView *view = columns.at(i);
         view->setSelectionModel(replacementSelectionModel);
         view->setFocusPolicy(Qt::NoFocus);
         if (columns.size() > i + 1) {
            const QModelIndex newRootIndex = columns.at(i + 1)->rootIndex();
            if (newRootIndex.isValid()) {
               view->setCurrentIndex(newRootIndex);
            }
         }
         break;
      }
   }
   parentColumn->selectionModel()->deleteLater();
   parentColumn->setFocusPolicy(Qt::StrongFocus);
   parentColumn->setSelectionModel(q->selectionModel());
   // We want the parent selection to stay highlighted (but dimmed depending upon the color theme)
   if (currentColumn > 0) {
      parentColumn = columns.at(currentColumn - 1);
      if (parentColumn->currentIndex() != current.parent()) {
         parentColumn->setCurrentIndex(current.parent());
      }
   }

   if (columns.last()->isHidden()) {
      columns.last()->setVisible(true);
   }
   if (columns.last()->selectionModel()) {
      columns.last()->selectionModel()->clear();
   }
   updateScrollbars();
}

void QColumnView::selectAll()
{
   if (!model() || !selectionModel()) {
      return;
   }

   QModelIndexList indexList = selectionModel()->selectedIndexes();
   QModelIndex parent = rootIndex();
   QItemSelection selection;
   if (indexList.count() >= 1) {
      parent = indexList.at(0).parent();
   }
   if (indexList.count() == 1) {
      parent = indexList.at(0);
      if (!model()->hasChildren(parent)) {
         parent = parent.parent();
      } else {
         selection.append(QItemSelectionRange(parent, parent));
      }
   }

   QModelIndex tl = model()->index(0, 0, parent);
   QModelIndex br = model()->index(model()->rowCount(parent) - 1,
         model()->columnCount(parent) - 1,
         parent);
   selection.append(QItemSelectionRange(tl, br));
   selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

QColumnViewPrivate::QColumnViewPrivate()
   :  QAbstractItemViewPrivate(), showResizeGrips(true), offset(), previewWidget(nullptr), previewColumn(nullptr)
{
}

QColumnViewPrivate::~QColumnViewPrivate()
{
}

void QColumnViewPrivate::_q_columnsInserted(const QModelIndex &parent, int start, int end)
{
   QAbstractItemViewPrivate::_q_columnsInserted(parent, start, end);
   checkColumnCreation(parent);
}

void QColumnViewPrivate::checkColumnCreation(const QModelIndex &parent)
{
   if (parent == q_func()->currentIndex() && model->hasChildren(parent)) {
      //the parent has children and is the current
      //let's try to find out if there is already a mapping that is good
      for (int i = 0; i < columns.count(); ++i) {
         QAbstractItemView *view = columns.at(i);

         if (view->rootIndex() == parent) {
            if (view == previewColumn) {
               //let's recreate the parent
               closeColumns(parent, false);
               createColumn(parent, true /*show*/);
            }
            break;
         }
      }
   }
}

void QColumnViewPrivate::doLayout()
{
   Q_Q(QColumnView);
   if (!model || columns.isEmpty()) {
      return;
   }

   int viewportHeight = viewport->height();
   int x = columns.at(0)->x();

   if (q->isRightToLeft()) {
      x = viewport->width() + q->horizontalOffset();
      for (int i = 0; i < columns.size(); ++i) {
         QAbstractItemView *view = columns.at(i);
         x -= view->width();
         if (x != view->x() || viewportHeight != view->height()) {
            view->setGeometry(x, 0, view->width(), viewportHeight);
         }
      }

   } else {
      for (int i = 0; i < columns.size(); ++i) {
         QAbstractItemView *view = columns.at(i);
         int currentColumnWidth = view->width();
         if (x != view->x() || viewportHeight != view->height()) {
            view->setGeometry(x, 0, currentColumnWidth, viewportHeight);
         }
         x += currentColumnWidth;
      }
   }
}

void QColumnViewDelegate::paint(QPainter *painter,
      const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   drawBackground(painter, option, index );

   bool reverse = (option.direction == Qt::RightToLeft);
   int width = ((option.rect.height() * 2) / 3);
   // Modify the options to give us room to add an arrow
   QStyleOptionViewItem opt = option;

   if (reverse) {
      opt.rect.adjust(width, 0, 0, 0);
   } else {
      opt.rect.adjust(0, 0, -width, 0);
   }

   if (!(index.model()->flags(index) & Qt::ItemIsEnabled)) {
      opt.showDecorationSelected = true;
      opt.state |= QStyle::State_Selected;
   }

   QItemDelegate::paint(painter, opt, index);

   if (reverse) {
      opt.rect = QRect(option.rect.x(), option.rect.y(), width, option.rect.height());
   } else
      opt.rect = QRect(option.rect.x() + option.rect.width() - width, option.rect.y(),
            width, option.rect.height());

   // Draw >
   if (index.model()->hasChildren(index)) {
      const QWidget *view = opt.widget;
      QStyle *style = view ? view->style() : QApplication::style();
      style->drawPrimitive(QStyle::PE_IndicatorColumnViewArrow, &opt, painter, view);
   }
}

void QColumnView::_q_gripMoved(int offset)
{
   Q_D(QColumnView);
   d->_q_gripMoved(offset);
}

void QColumnView::_q_changeCurrentColumn()
{
   Q_D(QColumnView);
   d->_q_changeCurrentColumn();
}

void QColumnView::_q_clicked(const QModelIndex &index)
{
   Q_D(QColumnView);
   d->_q_clicked(index);
}

#endif // QT_NO_COLUMNVIEW
