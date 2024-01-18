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

#ifndef QTREEVIEW_H
#define QTREEVIEW_H

#include <qabstractitemview.h>

#ifndef QT_NO_TREEVIEW

class QTreeViewPrivate;
class QHeaderView;

class Q_GUI_EXPORT QTreeView : public QAbstractItemView
{
   GUI_CS_OBJECT(QTreeView)

   GUI_CS_PROPERTY_READ(autoExpandDelay, autoExpandDelay)
   GUI_CS_PROPERTY_WRITE(autoExpandDelay, setAutoExpandDelay)

   GUI_CS_PROPERTY_READ(indentation, indentation)
   GUI_CS_PROPERTY_WRITE(indentation, setIndentation)
   GUI_CS_PROPERTY_READ(rootIsDecorated, rootIsDecorated)
   GUI_CS_PROPERTY_WRITE(rootIsDecorated, setRootIsDecorated)
   GUI_CS_PROPERTY_READ(uniformRowHeights, uniformRowHeights)
   GUI_CS_PROPERTY_WRITE(uniformRowHeights, setUniformRowHeights)
   GUI_CS_PROPERTY_READ(itemsExpandable, itemsExpandable)
   GUI_CS_PROPERTY_WRITE(itemsExpandable, setItemsExpandable)
   GUI_CS_PROPERTY_READ(sortingEnabled, isSortingEnabled)
   GUI_CS_PROPERTY_WRITE(sortingEnabled, setSortingEnabled)
   GUI_CS_PROPERTY_READ(animated, isAnimated)
   GUI_CS_PROPERTY_WRITE(animated, setAnimated)
   GUI_CS_PROPERTY_READ(allColumnsShowFocus, allColumnsShowFocus)
   GUI_CS_PROPERTY_WRITE(allColumnsShowFocus, setAllColumnsShowFocus)
   GUI_CS_PROPERTY_READ(wordWrap, wordWrap)
   GUI_CS_PROPERTY_WRITE(wordWrap, setWordWrap)
   GUI_CS_PROPERTY_READ(headerHidden, isHeaderHidden)
   GUI_CS_PROPERTY_WRITE(headerHidden, setHeaderHidden)

   GUI_CS_PROPERTY_READ(expandsOnDoubleClick, expandsOnDoubleClick)
   GUI_CS_PROPERTY_WRITE(expandsOnDoubleClick, setExpandsOnDoubleClick)

 public:
   explicit QTreeView(QWidget *parent = nullptr);

   QTreeView(const QTreeView &) = delete;
   QTreeView &operator=(const QTreeView &) = delete;

   ~QTreeView();

   void setModel(QAbstractItemModel *model) override;
   void setRootIndex(const QModelIndex &index) override;
   void setSelectionModel(QItemSelectionModel *selectionModel) override;

   QHeaderView *header() const;
   void setHeader(QHeaderView *header);

   int autoExpandDelay() const;
   void setAutoExpandDelay(int delay);

   int indentation() const;
   void setIndentation(int i);
   void resetIndentation();

   bool rootIsDecorated() const;
   void setRootIsDecorated(bool show);

   bool uniformRowHeights() const;
   void setUniformRowHeights(bool uniform);

   bool itemsExpandable() const;
   void setItemsExpandable(bool enable);

   bool expandsOnDoubleClick() const;
   void setExpandsOnDoubleClick(bool enable);

   int columnViewportPosition(int column) const;
   int columnWidth(int column) const;
   void setColumnWidth(int column, int width);
   int columnAt(int x) const;

   bool isColumnHidden(int column) const;
   void setColumnHidden(int column, bool hide);

   bool isHeaderHidden() const;
   void setHeaderHidden(bool hide);

   bool isRowHidden(int row, const QModelIndex &parent) const;
   void setRowHidden(int row, const QModelIndex &parent, bool hide);

   bool isFirstColumnSpanned(int row, const QModelIndex &parent) const;
   void setFirstColumnSpanned(int row, const QModelIndex &parent, bool span);

   bool isExpanded(const QModelIndex &index) const;
   void setExpanded(const QModelIndex &index, bool expanded);

   void setSortingEnabled(bool enable);
   bool isSortingEnabled() const;

   void setAnimated(bool enable);
   bool isAnimated() const;

   void setAllColumnsShowFocus(bool enable);
   bool allColumnsShowFocus() const;

   void setWordWrap(bool on);
   bool wordWrap() const;

   void setTreePosition(int logicalIndex);
   int treePosition() const;
   void keyboardSearch(const QString &search) override;

   QRect visualRect(const QModelIndex &index) const override;
   void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
   QModelIndex indexAt(const QPoint &point) const override;
   QModelIndex indexAbove(const QModelIndex &index) const;
   QModelIndex indexBelow(const QModelIndex &index) const;

   void doItemsLayout() override;
   void reset() override;

   void sortByColumn(int column, Qt::SortOrder order);

   void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;
   void selectAll() override;

   GUI_CS_SIGNAL_1(Public, void expanded(const QModelIndex &index))
   GUI_CS_SIGNAL_2(expanded, index)

   GUI_CS_SIGNAL_1(Public, void collapsed(const QModelIndex &index))
   GUI_CS_SIGNAL_2(collapsed, index)

   GUI_CS_SLOT_1(Public, void hideColumn(int column))
   GUI_CS_SLOT_2(hideColumn)

   GUI_CS_SLOT_1(Public, void showColumn(int column))
   GUI_CS_SLOT_2(showColumn)

   GUI_CS_SLOT_1(Public, void expand(const QModelIndex &index))
   GUI_CS_SLOT_2(expand)

   GUI_CS_SLOT_1(Public, void collapse(const QModelIndex &index))
   GUI_CS_SLOT_2(collapse)

   GUI_CS_SLOT_1(Public, void resizeColumnToContents(int column))
   GUI_CS_SLOT_2(resizeColumnToContents)

   GUI_CS_SLOT_1(Public, void sortByColumn(int column))
   GUI_CS_SLOT_OVERLOAD(sortByColumn, (int))

   GUI_CS_SLOT_1(Public, void expandAll())
   GUI_CS_SLOT_2(expandAll)

   GUI_CS_SLOT_1(Public, void collapseAll())
   GUI_CS_SLOT_2(collapseAll)

   GUI_CS_SLOT_1(Public, void expandToDepth(int depth))
   GUI_CS_SLOT_2(expandToDepth)

 protected:
   QTreeView(QTreeViewPrivate &dd, QWidget *parent = nullptr);
   void scrollContentsBy(int dx, int dy) override;
   void rowsInserted(const QModelIndex &parent, int start, int end)override;
   void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;

   QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
   int horizontalOffset() const override;
   int verticalOffset() const override;

   void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;
   QRegion visualRegionForSelection(const QItemSelection &selection) const override;
   QModelIndexList selectedIndexes() const override;

   void timerEvent(QTimerEvent *event) override;
   void paintEvent(QPaintEvent *event) override;

   void drawTree(QPainter *painter, const QRegion &region) const;
   virtual void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   virtual void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;

   void mousePressEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mouseDoubleClickEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;

#ifndef QT_NO_DRAGANDDROP
   void dragMoveEvent(QDragMoveEvent *event) override;
#endif

   bool viewportEvent(QEvent *event) override;

   void updateGeometries() override;

   QSize viewportSizeHint() const override;

   int sizeHintForColumn(int column) const override;
   int indexRowSizeHint(const QModelIndex &index) const;
   int rowHeight(const QModelIndex &index) const;

   void horizontalScrollbarAction(int action) override;

   bool isIndexHidden(const QModelIndex &index) const override;
   void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
   void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

   GUI_CS_SLOT_1(Protected, void columnResized(int column, int oldSize, int newSize))
   GUI_CS_SLOT_2(columnResized)

   GUI_CS_SLOT_1(Protected, void columnCountChanged(int oldCount, int newCount))
   GUI_CS_SLOT_2(columnCountChanged)

   GUI_CS_SLOT_1(Protected, void columnMoved())
   GUI_CS_SLOT_2(columnMoved)

   GUI_CS_SLOT_1(Protected, void reexpand())
   GUI_CS_SLOT_2(reexpand)

   GUI_CS_SLOT_1(Protected, void rowsRemoved(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(rowsRemoved)

 private:
   Q_DECLARE_PRIVATE(QTreeView)

   int visualIndex(const QModelIndex &index) const;

#ifndef QT_NO_ANIMATION
   GUI_CS_SLOT_1(Private, void _q_endAnimatedOperation())
   GUI_CS_SLOT_2(_q_endAnimatedOperation)
#endif

   GUI_CS_SLOT_1(Private, void _q_modelAboutToBeReset())
   GUI_CS_SLOT_2(_q_modelAboutToBeReset)

   GUI_CS_SLOT_1(Private, void _q_sortIndicatorChanged(int column, Qt::SortOrder order))
   GUI_CS_SLOT_2(_q_sortIndicatorChanged)

   friend class QAccessibleTable;
   friend class QAccessibleTree;
   friend class QAccessibleTableCell;
};

#endif // QT_NO_TREEVIEW

#endif
