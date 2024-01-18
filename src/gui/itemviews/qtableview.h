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

#ifndef QTABLEVIEW_H
#define QTABLEVIEW_H

#include <qabstractitemview.h>

#ifndef QT_NO_TABLEVIEW

class QHeaderView;
class QTableViewPrivate;

class Q_GUI_EXPORT QTableView : public QAbstractItemView
{
   GUI_CS_OBJECT(QTableView)

   GUI_CS_PROPERTY_READ(showGrid, showGrid)
   GUI_CS_PROPERTY_WRITE(showGrid, setShowGrid)

   GUI_CS_PROPERTY_READ(gridStyle, gridStyle)
   GUI_CS_PROPERTY_WRITE(gridStyle, setGridStyle)

   GUI_CS_PROPERTY_READ(sortingEnabled, isSortingEnabled)
   GUI_CS_PROPERTY_WRITE(sortingEnabled, setSortingEnabled)

   GUI_CS_PROPERTY_READ(wordWrap, wordWrap)
   GUI_CS_PROPERTY_WRITE(wordWrap, setWordWrap)

   GUI_CS_PROPERTY_READ(cornerButtonEnabled, isCornerButtonEnabled)
   GUI_CS_PROPERTY_WRITE(cornerButtonEnabled, setCornerButtonEnabled)

 public:
   explicit QTableView(QWidget *parent = nullptr);

   QTableView(const QTableView &) = delete;
   QTableView &operator=(const QTableView &) = delete;

   ~QTableView();

   void setModel(QAbstractItemModel *model) override;
   void setRootIndex(const QModelIndex &index) override;
   void setSelectionModel(QItemSelectionModel *selectionModel) override;
   void doItemsLayout() override;

   QHeaderView *horizontalHeader() const;
   QHeaderView *verticalHeader() const;
   void setHorizontalHeader(QHeaderView *header);
   void setVerticalHeader(QHeaderView *header);

   int rowViewportPosition(int row) const;
   int rowAt(int y) const;

   void setRowHeight(int row, int height);
   int rowHeight(int row) const;

   int columnViewportPosition(int column) const;
   int columnAt(int x) const;

   void setColumnWidth(int column, int width);
   int columnWidth(int column) const;

   bool isRowHidden(int row) const;
   void setRowHidden(int row, bool hide);

   bool isColumnHidden(int column) const;
   void setColumnHidden(int column, bool hide);

   void setSortingEnabled(bool enable);
   bool isSortingEnabled() const;

   bool showGrid() const;

   Qt::PenStyle gridStyle() const;
   void setGridStyle(Qt::PenStyle style);

   void setWordWrap(bool on);
   bool wordWrap() const;

   void setCornerButtonEnabled(bool enable);
   bool isCornerButtonEnabled() const;

   QRect visualRect(const QModelIndex &index) const override;
   void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
   QModelIndex indexAt(const QPoint &pos) const override;

   void setSpan(int row, int column, int rowSpanCount, int columnSpanCount);
   int rowSpan(int row, int column) const;
   int columnSpan(int row, int column) const;
   void clearSpans();

   void sortByColumn(int column, Qt::SortOrder order);

   GUI_CS_SLOT_1(Public, void selectRow(int row))
   GUI_CS_SLOT_2(selectRow)

   GUI_CS_SLOT_1(Public, void selectColumn(int column))
   GUI_CS_SLOT_2(selectColumn)

   GUI_CS_SLOT_1(Public, void hideRow(int row))
   GUI_CS_SLOT_2(hideRow)

   GUI_CS_SLOT_1(Public, void hideColumn(int column))
   GUI_CS_SLOT_2(hideColumn)

   GUI_CS_SLOT_1(Public, void showRow(int row))
   GUI_CS_SLOT_2(showRow)

   GUI_CS_SLOT_1(Public, void showColumn(int column))
   GUI_CS_SLOT_2(showColumn)

   GUI_CS_SLOT_1(Public, void resizeRowToContents(int row))
   GUI_CS_SLOT_2(resizeRowToContents)

   GUI_CS_SLOT_1(Public, void resizeRowsToContents())
   GUI_CS_SLOT_2(resizeRowsToContents)

   GUI_CS_SLOT_1(Public, void resizeColumnToContents(int column))
   GUI_CS_SLOT_2(resizeColumnToContents)

   GUI_CS_SLOT_1(Public, void resizeColumnsToContents())
   GUI_CS_SLOT_2(resizeColumnsToContents)

   GUI_CS_SLOT_1(Public, void sortByColumn(int column))
   GUI_CS_SLOT_OVERLOAD(sortByColumn, (int))

   GUI_CS_SLOT_1(Public, void setShowGrid(bool show))
   GUI_CS_SLOT_2(setShowGrid)

 protected:
   GUI_CS_SLOT_1(Protected, void rowMoved(int row, int oldIndex, int newIndex))
   GUI_CS_SLOT_2(rowMoved)
   GUI_CS_SLOT_1(Protected, void columnMoved(int column, int oldIndex, int newIndex))
   GUI_CS_SLOT_2(columnMoved)
   GUI_CS_SLOT_1(Protected, void rowResized(int row, int oldHeight, int newHeight))
   GUI_CS_SLOT_2(rowResized)
   GUI_CS_SLOT_1(Protected, void columnResized(int column, int oldWidth, int newWidth))
   GUI_CS_SLOT_2(columnResized)
   GUI_CS_SLOT_1(Protected, void rowCountChanged(int oldCount, int newCount))
   GUI_CS_SLOT_2(rowCountChanged)
   GUI_CS_SLOT_1(Protected, void columnCountChanged(int oldCount, int newCount))
   GUI_CS_SLOT_2(columnCountChanged)

   QTableView(QTableViewPrivate &, QWidget *parent);
   void scrollContentsBy(int dx, int dy) override;

   QStyleOptionViewItem viewOptions() const override;
   void paintEvent(QPaintEvent *event) override;

   void timerEvent(QTimerEvent *event) override;

   int horizontalOffset() const override;
   int verticalOffset() const override;
   QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;

   void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) override;
   QRegion visualRegionForSelection(const QItemSelection &selection) const override;
   QModelIndexList selectedIndexes() const override;

   void updateGeometries() override;

   QSize viewportSizeHint() const override;
   int sizeHintForRow(int row) const override;
   int sizeHintForColumn(int column) const override;

   void verticalScrollbarAction(int action) override;
   void horizontalScrollbarAction(int action) override;

   bool isIndexHidden(const QModelIndex &index) const override;

   void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
   void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

 private:
   friend class QAccessibleItemView;
   int visualIndex(const QModelIndex &index) const;

   Q_DECLARE_PRIVATE(QTableView)

   GUI_CS_SLOT_1(Private, void _q_selectRow(int row))
   GUI_CS_SLOT_2(_q_selectRow)

   GUI_CS_SLOT_1(Private, void _q_selectColumn(int column))
   GUI_CS_SLOT_2(_q_selectColumn)

   GUI_CS_SLOT_1(Private, void _q_updateSpanInsertedRows(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_updateSpanInsertedRows)

   GUI_CS_SLOT_1(Private, void _q_updateSpanInsertedColumns(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_updateSpanInsertedColumns)

   GUI_CS_SLOT_1(Private, void _q_updateSpanRemovedRows(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_updateSpanRemovedRows)

   GUI_CS_SLOT_1(Private, void _q_updateSpanRemovedColumns(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_updateSpanRemovedColumns)
};

#endif // QT_NO_TABLEVIEW

#endif // QTABLEVIEW_H
