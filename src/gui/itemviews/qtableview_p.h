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

#ifndef QTABLEVIEW_P_H
#define QTABLEVIEW_P_H

#include <qalgorithms.h>
#include <qdebug.h>
#include <qlist.h>
#include <qlinkedlist.h>
#include <qmap.h>
#include <qset.h>

#include <qabstractitemview_p.h>

#ifndef QT_NO_TABLEVIEW

class QSpanCollection
{
 public:
   struct Span {
      int m_top;
      int m_left;
      int m_bottom;
      int m_right;
      bool will_be_deleted;

      Span()
         : m_top(-1), m_left(-1), m_bottom(-1), m_right(-1), will_be_deleted(false)
      { }

      Span(int row, int column, int rowCount, int columnCount)
         : m_top(row), m_left(column), m_bottom(row + rowCount - 1), m_right(column + columnCount - 1),
           will_be_deleted(false)
      { }

      inline int top() const {
         return m_top;
      }
      inline int left() const {
         return m_left;
      }
      inline int bottom() const {
         return m_bottom;
      }
      inline int right() const {
         return m_right;
      }
      inline int height() const {
         return m_bottom - m_top + 1;
      }
      inline int width() const {
         return m_right - m_left + 1;
      }
   };

   ~QSpanCollection() {
      qDeleteAll(spans);
   }

   void addSpan(Span *span);
   void updateSpan(Span *span, int old_height);
   Span *spanAt(int x, int y) const;
   void clear();
   QList<Span *> spansInRect(int x, int y, int w, int h) const;

   void updateInsertedRows(int start, int end);
   void updateInsertedColumns(int start, int end);
   void updateRemovedRows(int start, int end);
   void updateRemovedColumns(int start, int end);

   typedef QLinkedList<Span *> SpanList;
   SpanList spans; //lists of all spans

 private:
   //the indexes are negative so the QMap::lowerBound do what i need.
   typedef QMap<int, Span *> SubIndex;
   typedef QMap<int, SubIndex> Index;
   Index index;

   bool cleanSpanSubIndex(SubIndex &subindex, int end, bool update = false);
};

class QTableViewPrivate : public QAbstractItemViewPrivate
{
   Q_DECLARE_PUBLIC(QTableView)

 public:
   QTableViewPrivate()
      : showGrid(true), gridStyle(Qt::SolidLine), rowSectionAnchor(-1), columnSectionAnchor(-1),
        columnResizeTimerID(0), rowResizeTimerID(0), horizontalHeader(nullptr), verticalHeader(nullptr),
        sortingEnabled(false), geometryRecursionBlock(false), visualCursor(QPoint())
   {
      wrapItemText = true;

#ifndef QT_NO_DRAGANDDROP
      overwrite = true;
#endif

   }
   void init();
   void trimHiddenSelections(QItemSelectionRange *range) const;

   inline bool isHidden(int row, int col) const {
      return verticalHeader->isSectionHidden(row) || horizontalHeader->isSectionHidden(col);
   }

   inline int visualRow(int logicalRow) const {
      return verticalHeader->visualIndex(logicalRow);
   }

   inline int visualColumn(int logicalCol) const {
      return horizontalHeader->visualIndex(logicalCol);
   }

   inline int logicalRow(int visualRow) const {
      return verticalHeader->logicalIndex(visualRow);
   }

   inline int logicalColumn(int visualCol) const {
      return horizontalHeader->logicalIndex(visualCol);
   }

   inline int accessibleTable2Index(const QModelIndex &index) const {
      const int vHeader = verticalHeader ? 1 : 0;

      return (index.row() + (horizontalHeader ? 1 : 0)) * (index.model()->columnCount() + vHeader)
         + index.column() + vHeader;
   }

   int sectionSpanEndLogical(const QHeaderView *header, int logical, int span) const;
   int sectionSpanSize(const QHeaderView *header, int logical, int span) const;
   bool spanContainsSection(const QHeaderView *header, int logical, int spanLogical, int span) const;

   void drawAndClipSpans(const QRegion &area, QPainter *painter, const QStyleOptionViewItem &option, QBitArray *drawn,
      int firstVisualRow, int lastVisualRow, int firstVisualColumn, int lastVisualColumn);

   void drawCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index);
   int widthHintForIndex(const QModelIndex &index, int hint, const QStyleOptionViewItem &option) const;
   int heightHintForIndex(const QModelIndex &index, int hint, QStyleOptionViewItem &option) const;

   bool showGrid;
   Qt::PenStyle gridStyle;
   int rowSectionAnchor;
   int columnSectionAnchor;
   int columnResizeTimerID;
   int rowResizeTimerID;
   QList<int> columnsToUpdate;
   QList<int> rowsToUpdate;
   QHeaderView *horizontalHeader;
   QHeaderView *verticalHeader;
   QWidget *cornerWidget;
   bool sortingEnabled;
   bool geometryRecursionBlock;
   QPoint visualCursor;  // (Row,column) cell coordinates to track through span navigation.

   QSpanCollection spans;

   void setSpan(int row, int column, int rowSpan, int columnSpan);
   QSpanCollection::Span span(int row, int column) const;

   inline int rowSpan(int row, int column) const {
      return span(row, column).height();
   }

   inline int columnSpan(int row, int column) const {
      return span(row, column).width();
   }

   inline bool hasSpans() const {
      return !spans.spans.isEmpty();
   }

   inline int rowSpanHeight(int row, int span) const {
      return sectionSpanSize(verticalHeader, row, span);
   }

   inline int columnSpanWidth(int column, int span) const {
      return sectionSpanSize(horizontalHeader, column, span);
   }

   inline int rowSpanEndLogical(int row, int span) const {
      return sectionSpanEndLogical(verticalHeader, row, span);
   }

   inline int columnSpanEndLogical(int column, int span) const {
      return sectionSpanEndLogical(horizontalHeader, column, span);
   }

   inline bool isRowHidden(int row) const {
      return verticalHeader->isSectionHidden(row);
   }

   inline bool isColumnHidden(int column) const {
      return horizontalHeader->isSectionHidden(column);
   }

   inline bool isCellEnabled(int row, int column) const {
      return isIndexEnabled(model->index(row, column, root));
   }

   inline bool isVisualRowHiddenOrDisabled(int row, int column) const {
      int r = logicalRow(row);
      int c = logicalColumn(column);
      return isRowHidden(r) || !isCellEnabled(r, c);
   }

   inline bool isVisualColumnHiddenOrDisabled(int row, int column) const {
      int r = logicalRow(row);
      int c = logicalColumn(column);
      return isColumnHidden(c) || !isCellEnabled(r, c);
   }

   QRect visualSpanRect(const QSpanCollection::Span &span) const;

   void _q_selectRow(int row);
   void _q_selectColumn(int column);

   void selectRow(int row, bool anchor);
   void selectColumn(int column, bool anchor);

   void _q_updateSpanInsertedRows(const QModelIndex &parent, int start, int end);
   void _q_updateSpanInsertedColumns(const QModelIndex &parent, int start, int end);
   void _q_updateSpanRemovedRows(const QModelIndex &parent, int start, int end);
   void _q_updateSpanRemovedColumns(const QModelIndex &parent, int start, int end);
};

#endif // QT_NO_TABLEVIEW

#endif // QTABLEVIEW_P_H
