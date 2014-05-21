/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QTABLEVIEW_H
#define QTABLEVIEW_H

#include <QtGui/qabstractitemview.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_TABLEVIEW

class QHeaderView;
class QTableViewPrivate;

class Q_GUI_EXPORT QTableView : public QAbstractItemView
{
    CS_OBJECT(QTableView)

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
    explicit QTableView(QWidget *parent = 0);
    ~QTableView();

    void setModel(QAbstractItemModel *model);
    void setRootIndex(const QModelIndex &index);
    void setSelectionModel(QItemSelectionModel *selectionModel);
    void doItemsLayout();

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

    QRect visualRect(const QModelIndex &index) const;
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
    QModelIndex indexAt(const QPoint &p) const;

    void setSpan(int row, int column, int rowSpan, int columnSpan);
    int rowSpan(int row, int column) const;
    int columnSpan(int row, int column) const;
    void clearSpans();

    void sortByColumn(int column, Qt::SortOrder order);

public :
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
    GUI_CS_SLOT_OVERLOAD(sortByColumn,(int)) 

    GUI_CS_SLOT_1(Public, void setShowGrid(bool show))
    GUI_CS_SLOT_2(setShowGrid) 

protected :
    GUI_CS_SLOT_1(Protected, void rowMoved(int row,int oldIndex,int newIndex))
    GUI_CS_SLOT_2(rowMoved) 
    GUI_CS_SLOT_1(Protected, void columnMoved(int column,int oldIndex,int newIndex))
    GUI_CS_SLOT_2(columnMoved) 
    GUI_CS_SLOT_1(Protected, void rowResized(int row,int oldHeight,int newHeight))
    GUI_CS_SLOT_2(rowResized) 
    GUI_CS_SLOT_1(Protected, void columnResized(int column,int oldWidth,int newWidth))
    GUI_CS_SLOT_2(columnResized) 
    GUI_CS_SLOT_1(Protected, void rowCountChanged(int oldCount,int newCount))
    GUI_CS_SLOT_2(rowCountChanged) 
    GUI_CS_SLOT_1(Protected, void columnCountChanged(int oldCount,int newCount))
    GUI_CS_SLOT_2(columnCountChanged) 

    QTableView(QTableViewPrivate &, QWidget *parent);
    void scrollContentsBy(int dx, int dy);

    QStyleOptionViewItem viewOptions() const;
    void paintEvent(QPaintEvent *e);

    void timerEvent(QTimerEvent *event);

    int horizontalOffset() const;
    int verticalOffset() const;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    QRegion visualRegionForSelection(const QItemSelection &selection) const;
    QModelIndexList selectedIndexes() const;

    void updateGeometries();

    int sizeHintForRow(int row) const;
    int sizeHintForColumn(int column) const;

    void verticalScrollbarAction(int action);
    void horizontalScrollbarAction(int action);

    bool isIndexHidden(const QModelIndex &index) const;

    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    friend class QAccessibleItemView;
    int visualIndex(const QModelIndex &index) const;

    Q_DECLARE_PRIVATE(QTableView)
    Q_DISABLE_COPY(QTableView)
   
    GUI_CS_SLOT_1(Private, void _q_selectRow(int un_named_arg1))
    GUI_CS_SLOT_2(_q_selectRow)

    GUI_CS_SLOT_1(Private, void _q_selectColumn(int un_named_arg1))
    GUI_CS_SLOT_2(_q_selectColumn)

    GUI_CS_SLOT_1(Private, void _q_updateSpanInsertedRows(const QModelIndex & un_named_arg1,int un_named_arg2,int un_named_arg3))
    GUI_CS_SLOT_2(_q_updateSpanInsertedRows)

    GUI_CS_SLOT_1(Private, void _q_updateSpanInsertedColumns(const QModelIndex & un_named_arg1,int un_named_arg2,int un_named_arg3))
    GUI_CS_SLOT_2(_q_updateSpanInsertedColumns)

    GUI_CS_SLOT_1(Private, void _q_updateSpanRemovedRows(const QModelIndex & un_named_arg1,int un_named_arg2,int un_named_arg3))
    GUI_CS_SLOT_2(_q_updateSpanRemovedRows)

    GUI_CS_SLOT_1(Private, void _q_updateSpanRemovedColumns(const QModelIndex & un_named_arg1,int un_named_arg2,int un_named_arg3))
    GUI_CS_SLOT_2(_q_updateSpanRemovedColumns)
};

#endif // QT_NO_TABLEVIEW

QT_END_NAMESPACE

#endif // QTABLEVIEW_H
