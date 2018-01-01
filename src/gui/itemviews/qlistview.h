/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QLISTVIEW_H
#define QLISTVIEW_H

#include <QtGui/qabstractitemview.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_LISTVIEW

class QListViewPrivate;

class Q_GUI_EXPORT QListView : public QAbstractItemView
{
   GUI_CS_OBJECT(QListView)

   GUI_CS_ENUM(Movement)
   GUI_CS_ENUM(Flow)
   GUI_CS_ENUM(ResizeMode)
   GUI_CS_ENUM(LayoutMode)
   GUI_CS_ENUM(ViewMode)

   GUI_CS_PROPERTY_READ(movement, movement)
   GUI_CS_PROPERTY_WRITE(movement, setMovement)
   GUI_CS_PROPERTY_READ(flow, flow)
   GUI_CS_PROPERTY_WRITE(flow, setFlow)
   GUI_CS_PROPERTY_READ(isWrapping, isWrapping)
   GUI_CS_PROPERTY_WRITE(isWrapping, setWrapping)
   GUI_CS_PROPERTY_READ(resizeMode, resizeMode)
   GUI_CS_PROPERTY_WRITE(resizeMode, setResizeMode)
   GUI_CS_PROPERTY_READ(layoutMode, layoutMode)
   GUI_CS_PROPERTY_WRITE(layoutMode, setLayoutMode)
   GUI_CS_PROPERTY_READ(spacing, spacing)
   GUI_CS_PROPERTY_WRITE(spacing, setSpacing)
   GUI_CS_PROPERTY_READ(gridSize, gridSize)
   GUI_CS_PROPERTY_WRITE(gridSize, setGridSize)
   GUI_CS_PROPERTY_READ(viewMode, viewMode)
   GUI_CS_PROPERTY_WRITE(viewMode, setViewMode)
   GUI_CS_PROPERTY_READ(modelColumn, modelColumn)
   GUI_CS_PROPERTY_WRITE(modelColumn, setModelColumn)
   GUI_CS_PROPERTY_READ(uniformItemSizes, uniformItemSizes)
   GUI_CS_PROPERTY_WRITE(uniformItemSizes, setUniformItemSizes)
   GUI_CS_PROPERTY_READ(batchSize, batchSize)
   GUI_CS_PROPERTY_WRITE(batchSize, setBatchSize)
   GUI_CS_PROPERTY_READ(wordWrap, wordWrap)
   GUI_CS_PROPERTY_WRITE(wordWrap, setWordWrap)
   GUI_CS_PROPERTY_READ(selectionRectVisible, isSelectionRectVisible)
   GUI_CS_PROPERTY_WRITE(selectionRectVisible, setSelectionRectVisible)

 public:
   enum Movement { Static, Free, Snap };
   enum Flow { LeftToRight, TopToBottom };
   enum ResizeMode { Fixed, Adjust };
   enum LayoutMode { SinglePass, Batched };
   enum ViewMode { ListMode, IconMode };

   explicit QListView(QWidget *parent = nullptr);
   ~QListView();

   void setMovement(Movement movement);
   Movement movement() const;

   void setFlow(Flow flow);
   Flow flow() const;

   void setWrapping(bool enable);
   bool isWrapping() const;

   void setResizeMode(ResizeMode mode);
   ResizeMode resizeMode() const;

   void setLayoutMode(LayoutMode mode);
   LayoutMode layoutMode() const;

   void setSpacing(int space);
   int spacing() const;

   void setBatchSize(int batchSize);
   int batchSize() const;

   void setGridSize(const QSize &size);
   QSize gridSize() const;

   void setViewMode(ViewMode mode);
   ViewMode viewMode() const;

   void clearPropertyFlags();

   bool isRowHidden(int row) const;
   void setRowHidden(int row, bool hide);

   void setModelColumn(int column);
   int modelColumn() const;

   void setUniformItemSizes(bool enable);
   bool uniformItemSizes() const;

   void setWordWrap(bool on);
   bool wordWrap() const;

   void setSelectionRectVisible(bool show);
   bool isSelectionRectVisible() const;

   QRect visualRect(const QModelIndex &index) const override;
   void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
   QModelIndex indexAt(const QPoint &p) const override;

   void doItemsLayout() override;
   void reset() override;
   void setRootIndex(const QModelIndex &index) override;

   GUI_CS_SIGNAL_1(Public, void indexesMoved(const QModelIndexList &indexes))
   GUI_CS_SIGNAL_2(indexesMoved, indexes)

 protected:
   QListView(QListViewPrivate &, QWidget *parent = nullptr);

   bool event(QEvent *e) override;

   void scrollContentsBy(int dx, int dy) override;

   void resizeContents(int width, int height);
   QSize contentsSize() const;

   void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) override;
   void rowsInserted(const QModelIndex &parent, int start, int end) override;
   void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;

   void mouseMoveEvent(QMouseEvent *e) override;
   void mouseReleaseEvent(QMouseEvent *e) override;

   void timerEvent(QTimerEvent *e) override;
   void resizeEvent(QResizeEvent *e) override;

#ifndef QT_NO_DRAGANDDROP
   void dragMoveEvent(QDragMoveEvent *e) override;
   void dragLeaveEvent(QDragLeaveEvent *e) override;
   void dropEvent(QDropEvent *e) override;
   void startDrag(Qt::DropActions supportedActions) override;

   void internalDrop(QDropEvent *e);
   void internalDrag(Qt::DropActions supportedActions);
#endif

   QStyleOptionViewItem viewOptions() const override;
   void paintEvent(QPaintEvent *e) override;

   int horizontalOffset() const override;
   int verticalOffset() const override;
   QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
   QRect rectForIndex(const QModelIndex &index) const;
   void setPositionForIndex(const QPoint &position, const QModelIndex &index);

   void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;
   QRegion visualRegionForSelection(const QItemSelection &selection) const override;
   QModelIndexList selectedIndexes() const override;

   void updateGeometries() override;

   bool isIndexHidden(const QModelIndex &index) const override;

   void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
   void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

 private:
   friend class QAccessibleItemView;
   int visualIndex(const QModelIndex &index) const;

   Q_DECLARE_PRIVATE(QListView)
   Q_DISABLE_COPY(QListView)
};

#endif // QT_NO_LISTVIEW

QT_END_NAMESPACE

#endif // QLISTVIEW_H
