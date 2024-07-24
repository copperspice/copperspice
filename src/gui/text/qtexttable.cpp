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

#include <qtexttable.h>

#include <qdebug.h>
#include <qtextcursor.h>
#include <qtextformat.h>
#include <qvarlengtharray.h>

#include <qtextcursor_p.h>
#include <qtexttable_p.h>

#include <algorithm>
#include <stdlib.h>

void QTextTableCell::setFormat(const QTextCharFormat &format)
{
   QTextCharFormat fmt = format;
   fmt.clearProperty(QTextFormat::ObjectIndex);
   fmt.setObjectType(QTextFormat::TableCellObject);
   QTextDocumentPrivate *p = table->docHandle();
   QTextDocumentPrivate::FragmentIterator frag(&p->fragmentMap(), fragment);

   QTextFormatCollection *c = p->formatCollection();
   QTextCharFormat oldFormat = c->charFormat(frag->format);
   fmt.setTableCellRowSpan(oldFormat.tableCellRowSpan());
   fmt.setTableCellColumnSpan(oldFormat.tableCellColumnSpan());

   p->setCharFormat(frag.position(), 1, fmt, QTextDocumentPrivate::SetFormatAndPreserveObjectIndices);
}

QTextCharFormat QTextTableCell::format() const
{
   QTextDocumentPrivate *p = table->docHandle();
   QTextFormatCollection *c = p->formatCollection();

   QTextCharFormat fmt = c->charFormat(tableCellFormatIndex());
   fmt.setObjectType(QTextFormat::TableCellObject);
   return fmt;
}

int QTextTableCell::tableCellFormatIndex() const
{
   QTextDocumentPrivate *p = table->docHandle();
   return QTextDocumentPrivate::FragmentIterator(&p->fragmentMap(), fragment)->format;
}

int QTextTableCell::row() const
{
   const QTextTablePrivate *tp = table->d_func();
   if (tp->dirty) {
      tp->update();
   }

   int idx = tp->findCellIndex(fragment);
   if (idx == -1) {
      return idx;
   }
   return tp->cellIndices.at(idx) / tp->nCols;
}

int QTextTableCell::column() const
{
   const QTextTablePrivate *tp = table->d_func();
   if (tp->dirty) {
      tp->update();
   }

   int idx = tp->findCellIndex(fragment);
   if (idx == -1) {
      return idx;
   }
   return tp->cellIndices.at(idx) % tp->nCols;
}

int QTextTableCell::rowSpan() const
{
   return format().tableCellRowSpan();
}

int QTextTableCell::columnSpan() const
{
   return format().tableCellColumnSpan();
}

QTextCursor QTextTableCell::firstCursorPosition() const
{
   return QTextCursorPrivate::fromPosition(table->d_func()->pieceTable, firstPosition());
}

QTextCursor QTextTableCell::lastCursorPosition() const
{
   return QTextCursorPrivate::fromPosition(table->d_func()->pieceTable, lastPosition());
}

int QTextTableCell::firstPosition() const
{
   QTextDocumentPrivate *p = table->docHandle();
   return p->fragmentMap().position(fragment) + 1;
}

int QTextTableCell::lastPosition() const
{
   QTextDocumentPrivate *p = table->docHandle();
   const QTextTablePrivate *td = table->d_func();
   int index = table->d_func()->findCellIndex(fragment);
   int f;

   if (index != -1) {
      f = td->cells.value(index + 1, td->fragment_end);
   } else {
      f = td->fragment_end;
   }

   return p->fragmentMap().position(f);
}

QTextFrame::iterator QTextTableCell::begin() const
{
   QTextDocumentPrivate *p = table->docHandle();
   int b = p->blockMap().findNode(firstPosition());
   int e = p->blockMap().findNode(lastPosition() + 1);
   return QTextFrame::iterator(const_cast<QTextTable *>(table), b, b, e);
}

QTextFrame::iterator QTextTableCell::end() const
{
   QTextDocumentPrivate *p = table->docHandle();
   int b = p->blockMap().findNode(firstPosition());
   int e = p->blockMap().findNode(lastPosition() + 1);
   return QTextFrame::iterator(const_cast<QTextTable *>(table), e, b, e);
}

QTextTablePrivate::~QTextTablePrivate()
{
   if (grid) {
      free(grid);
   }
}

QTextTable *QTextTablePrivate::createTable(QTextDocumentPrivate *pieceTable, int pos,
      int rows, int cols, const QTextTableFormat &tableFormat)
{
   QTextTableFormat fmt = tableFormat;
   fmt.setColumns(cols);

   QTextTable *table = qobject_cast<QTextTable *>(pieceTable->createObject(fmt));
   Q_ASSERT(table);

   pieceTable->beginEditBlock();

   // add block after table
   QTextCharFormat charFmt;
   charFmt.setObjectIndex(table->objectIndex());
   charFmt.setObjectType(QTextFormat::TableCellObject);

   int charIdx = pieceTable->formatCollection()->indexForFormat(charFmt);
   int cellIdx = pieceTable->formatCollection()->indexForFormat(QTextBlockFormat());

   QTextTablePrivate *d = table->d_func();
   d->blockFragmentUpdates = true;

   d->fragment_start = pieceTable->insertBlock(QTextBeginningOfFrame, pos, cellIdx, charIdx);
   d->cells.append(d->fragment_start);
   ++pos;

   for (int i = 1; i < rows * cols; ++i) {
      d->cells.append(pieceTable->insertBlock(QTextBeginningOfFrame, pos, cellIdx, charIdx));
      ++pos;
   }

   d->fragment_end = pieceTable->insertBlock(QTextEndOfFrame, pos, cellIdx, charIdx);
   ++pos;

   d->blockFragmentUpdates = false;
   d->dirty = true;

   pieceTable->endEditBlock();

   return table;
}

struct QFragmentFindHelper {
   QFragmentFindHelper(int _pos, const QTextDocumentPrivate::FragmentMap &map)
      : pos(_pos), fragmentMap(map)
   { }

   uint pos;
   const QTextDocumentPrivate::FragmentMap &fragmentMap;
};

static inline bool operator<(int fragment, const QFragmentFindHelper &helper)
{
   return helper.fragmentMap.position(fragment) < helper.pos;
}

static inline bool operator<(const QFragmentFindHelper &helper, int fragment)
{
   return helper.pos < helper.fragmentMap.position(fragment);
}

int QTextTablePrivate::findCellIndex(int fragment) const
{
   QFragmentFindHelper helper(pieceTable->fragmentMap().position(fragment), pieceTable->fragmentMap());

   QList<int>::const_iterator it = std::lower_bound(cells.constBegin(), cells.constEnd(), helper);

   if ((it == cells.constEnd()) || (helper < *it)) {
      return -1;
   }

   return it - cells.constBegin();
}

void QTextTablePrivate::fragmentAdded(QChar type, uint fragment)
{
   dirty = true;

   if (blockFragmentUpdates) {
      return;
   }

   if (type == QTextBeginningOfFrame) {
      Q_ASSERT(cells.indexOf(fragment) == -1);
      const uint pos = pieceTable->fragmentMap().position(fragment);

      QFragmentFindHelper helper(pos, pieceTable->fragmentMap());
      QList<int>::iterator it = std::lower_bound(cells.begin(), cells.end(), helper);
      cells.insert(it, fragment);

      if (!fragment_start || pos < pieceTable->fragmentMap().position(fragment_start)) {
         fragment_start = fragment;
      }
      return;
   }

   QTextFramePrivate::fragmentAdded(type, fragment);
}

void QTextTablePrivate::fragmentRemoved(QChar type, uint fragment)
{
   dirty = true;
   if (blockFragmentUpdates) {
      return;
   }

   if (type == QTextBeginningOfFrame) {
      Q_ASSERT(cells.indexOf(fragment) != -1);
      cells.removeAll(fragment);

      if (fragment_start == fragment && cells.size()) {
         fragment_start = cells.at(0);
      }

      if (fragment_start != fragment) {
         return;
      }
   }

   QTextFramePrivate::fragmentRemoved(type, fragment);
}

void QTextTablePrivate::update() const
{
   Q_Q(const QTextTable);

   nCols = q->format().columns();
   nRows = (cells.size() + nCols - 1) / nCols;

   grid = q_check_ptr((int *)realloc(grid, nRows * nCols * sizeof(int)));
   memset(grid, 0, nRows * nCols * sizeof(int));

   QTextDocumentPrivate *p = pieceTable;
   QTextFormatCollection *c = p->formatCollection();

   cellIndices.resize(cells.size());

   int cell = 0;
   for (int i = 0; i < cells.size(); ++i) {
      int fragment = cells.at(i);
      QTextCharFormat fmt = c->charFormat(QTextDocumentPrivate::FragmentIterator(&p->fragmentMap(), fragment)->format);
      int rowspan = fmt.tableCellRowSpan();
      int colspan = fmt.tableCellColumnSpan();

      // skip taken cells
      while (cell < nRows * nCols && grid[cell]) {
         ++cell;
      }

      int r = cell / nCols;
      int c = cell % nCols;
      cellIndices[i] = cell;

      if (r + rowspan > nRows) {
         grid = q_check_ptr((int *)realloc(grid, sizeof(int) * (r + rowspan) * nCols));
         memset(grid + (nRows * nCols), 0, sizeof(int) * (r + rowspan - nRows)*nCols);
         nRows = r + rowspan;
      }

      Q_ASSERT(c + colspan <= nCols);

      for (int ii = 0; ii < rowspan; ++ii) {
         for (int jj = 0; jj < colspan; ++jj) {
            Q_ASSERT(grid[(r + ii)*nCols + c + jj] == 0);
            grid[(r + ii)*nCols + c + jj] = fragment;
         }
      }
   }

   dirty = false;
}

QTextTable::QTextTable(QTextDocument *doc)
   : QTextFrame(*new QTextTablePrivate(doc), doc)
{
}

QTextTable::~QTextTable()
{
}

QTextTableCell QTextTable::cellAt(int row, int col) const
{
   Q_D(const QTextTable);
   if (d->dirty) {
      d->update();
   }

   if (row < 0 || row >= d->nRows || col < 0 || col >= d->nCols) {
      return QTextTableCell();
   }

   return QTextTableCell(this, d->grid[row * d->nCols + col]);
}

QTextTableCell QTextTable::cellAt(int position) const
{
   Q_D(const QTextTable);
   if (d->dirty) {
      d->update();
   }

   uint pos = (uint)position;
   const QTextDocumentPrivate::FragmentMap &map = d->pieceTable->fragmentMap();
   if (position < 0 || map.position(d->fragment_start) >= pos || map.position(d->fragment_end) < pos) {
      return QTextTableCell();
   }

   QFragmentFindHelper helper(position, map);
   QList<int>::const_iterator it = std::lower_bound(d->cells.begin(), d->cells.end(), helper);

   if (it != d->cells.begin()) {
      --it;
   }

   return QTextTableCell(this, *it);
}

QTextTableCell QTextTable::cellAt(const QTextCursor &c) const
{
   return cellAt(c.position());
}

void QTextTable::resize(int rows, int cols)
{
   Q_D(QTextTable);

   if (d->dirty) {
      d->update();
   }

   int nRows = this->rows();
   int nCols = this->columns();

   if (rows == nRows && cols == nCols) {
      return;
   }

   d->pieceTable->beginEditBlock();

   if (nCols < cols) {
      insertColumns(nCols, cols - nCols);
   } else if (nCols > cols) {
      removeColumns(cols, nCols - cols);
   }

   if (nRows < rows) {
      insertRows(nRows, rows - nRows);
   } else if (nRows > rows) {
      removeRows(rows, nRows - rows);
   }

   d->pieceTable->endEditBlock();
}

void QTextTable::insertRows(int pos, int num)
{
   Q_D(QTextTable);
   if (num <= 0) {
      return;
   }

   if (d->dirty) {
      d->update();
   }

   if (pos > d->nRows || pos < 0) {
      pos = d->nRows;
   }

   QTextDocumentPrivate *p  = d->pieceTable;
   QTextFormatCollection *c = p->formatCollection();
   p->beginEditBlock();

   int extended = 0;
   int insert_before = 0;

   if (pos > 0 && pos < d->nRows) {
      for (int i = 0; i < d->nCols; ++i) {
         int cell = d->grid[pos * d->nCols + i];

         if (cell == d->grid[(pos - 1)*d->nCols + i]) {
            // cell spans the insertion place, extend it
            QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);

            QTextCharFormat fmt = c->charFormat(it->format);
            fmt.setTableCellRowSpan(fmt.tableCellRowSpan() + num);

            p->setCharFormat(it.position(), 1, fmt);
            ++extended;

         } else if (! insert_before) {
            insert_before = cell;
         }
      }

   } else {
      insert_before = (pos == 0 ? d->grid[0] : d->fragment_end);
   }

   if (extended < d->nCols) {
      Q_ASSERT(insert_before);
      QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), insert_before);

      QTextCharFormat fmt = c->charFormat(it->format);
      fmt.setTableCellRowSpan(1);
      fmt.setTableCellColumnSpan(1);

      Q_ASSERT(fmt.objectIndex() == objectIndex());
      int pos = it.position();
      int cfmt = p->formatCollection()->indexForFormat(fmt);
      int bfmt = p->formatCollection()->indexForFormat(QTextBlockFormat());

      for (int i = 0; i < num * (d->nCols - extended); ++i) {
         p->insertBlock(QTextBeginningOfFrame, pos, bfmt, cfmt, QTextUndoCommand::MoveCursor);
      }
   }

   p->endEditBlock();
}

void QTextTable::insertColumns(int pos, int num)
{
   Q_D(QTextTable);

   if (num <= 0) {
      return;
   }

   if (d->dirty) {
      d->update();
   }

   if (pos > d->nCols || pos < 0) {
      pos = d->nCols;
   }

   QTextDocumentPrivate *p = d->pieceTable;
   QTextFormatCollection *c = p->formatCollection();
   p->beginEditBlock();

   QList<int> extendedSpans;
   for (int i = 0; i < d->nRows; ++i) {
      int cell;
      if (i == d->nRows - 1 && pos == d->nCols) {
         cell = d->fragment_end;
      } else {
         int logicalGridIndexBeforePosition = pos > 0
            ? d->findCellIndex(d->grid[i * d->nCols + pos - 1])
            : -1;

         // Search for the logical insertion point by skipping past cells which are not the first
         // cell in a rowspan. This means any cell for which the logical grid index is
         // less than the logical cell index of the cell before the insertion.
         int logicalGridIndex;
         int gridArrayOffset = i * d->nCols + pos;

         do {
            cell = d->grid[gridArrayOffset];
            logicalGridIndex = d->findCellIndex(cell);
            gridArrayOffset++;
         } while (logicalGridIndex < logicalGridIndexBeforePosition
            && gridArrayOffset < d->nRows * d->nCols);

         if (logicalGridIndex < logicalGridIndexBeforePosition
            && gridArrayOffset == d->nRows * d->nCols) {
            cell = d->fragment_end;
         }
      }

      if (pos > 0 && pos < d->nCols && cell == d->grid[i * d->nCols + pos - 1]) {
         // cell spans the insertion place, extend it
         if (!extendedSpans.contains(cell)) {
            QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
            QTextCharFormat fmt = c->charFormat(it->format);
            fmt.setTableCellColumnSpan(fmt.tableCellColumnSpan() + num);
            p->setCharFormat(it.position(), 1, fmt);
            d->dirty = true;
            extendedSpans << cell;
         }
      } else {
         /* If the next cell is spanned from the row above, we need to find the right position
         to insert to */
         if (i > 0 && pos < d->nCols && cell == d->grid[(i - 1) * d->nCols + pos]) {
            int gridIndex = i * d->nCols + pos;
            const int gridEnd = d->nRows * d->nCols - 1;
            while (gridIndex < gridEnd && cell == d->grid[gridIndex]) {
               ++gridIndex;
            }
            if (gridIndex == gridEnd) {
               cell = d->fragment_end;
            } else {
               cell = d->grid[gridIndex];
            }
         }

         QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
         QTextCharFormat fmt = c->charFormat(it->format);
         fmt.setTableCellRowSpan(1);
         fmt.setTableCellColumnSpan(1);

         Q_ASSERT(fmt.objectIndex() == objectIndex());

         int position = it.position();
         int cfmt = p->formatCollection()->indexForFormat(fmt);
         int bfmt = p->formatCollection()->indexForFormat(QTextBlockFormat());

         for (int i = 0; i < num; ++i) {
            p->insertBlock(QTextBeginningOfFrame, position, bfmt, cfmt, QTextUndoCommand::MoveCursor);
         }
      }
   }

   QTextTableFormat tfmt = format();
   tfmt.setColumns(tfmt.columns() + num);
   QVector<QTextLength> columnWidths = tfmt.columnWidthConstraints();

   if (! columnWidths.isEmpty()) {
      for (int i = num; i > 0; --i) {
         columnWidths.insert(pos, columnWidths[qMax(0, pos - 1)]);
      }
   }

   tfmt.setColumnWidthConstraints (columnWidths);
   QTextObject::setFormat(tfmt);

   p->endEditBlock();
}

void QTextTable::appendRows(int count)
{
   insertRows(rows(), count);
}

void QTextTable::appendColumns(int count)
{
   insertColumns(columns(), count);
}

void QTextTable::removeRows(int pos, int num)
{
   Q_D(QTextTable);

   if (num <= 0 || pos < 0) {
      return;
   }

   if (d->dirty) {
      d->update();
   }

   if (pos >= d->nRows) {
      return;
   }

   if (pos + num > d->nRows) {
      num = d->nRows - pos;
   }

   QTextDocumentPrivate *p = d->pieceTable;
   QTextFormatCollection *collection = p->formatCollection();
   p->beginEditBlock();

   // delete whole table?
   if (pos == 0 && num == d->nRows) {
      const int pos = p->fragmentMap().position(d->fragment_start);
      p->remove(pos, p->fragmentMap().position(d->fragment_end) - pos + 1);
      p->endEditBlock();
      return;
   }

   p->aboutToRemoveCell(cellAt(pos, 0).firstPosition(), cellAt(pos + num - 1, d->nCols - 1).lastPosition());

   QList<int> touchedCells;
   for (int r = pos; r < pos + num; ++r) {
      for (int c = 0; c < d->nCols; ++c) {
         int cell = d->grid[r * d->nCols + c];
         if (touchedCells.contains(cell)) {
            continue;
         }
         touchedCells << cell;
         QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
         QTextCharFormat fmt = collection->charFormat(it->format);
         int span = fmt.tableCellRowSpan();
         if (span > 1) {
            fmt.setTableCellRowSpan(span - 1);
            p->setCharFormat(it.position(), 1, fmt);
         } else {
            // remove cell
            int index = d->cells.indexOf(cell) + 1;
            int f_end = index < d->cells.size() ? d->cells.at(index) : d->fragment_end;
            p->remove(it.position(), p->fragmentMap().position(f_end) - it.position());
         }
      }
   }

   p->endEditBlock();
}

void QTextTable::removeColumns(int pos, int num)
{
   Q_D(QTextTable);

   if (num <= 0 || pos < 0) {
      return;
   }

   if (d->dirty) {
      d->update();
   }

   if (pos >= d->nCols) {
      return;
   }

   if (pos + num > d->nCols) {
      pos = d->nCols - num;
   }

   QTextDocumentPrivate *p = d->pieceTable;
   QTextFormatCollection *collection = p->formatCollection();
   p->beginEditBlock();

   // delete whole table?
   if (pos == 0 && num == d->nCols) {
      const int pos = p->fragmentMap().position(d->fragment_start);
      p->remove(pos, p->fragmentMap().position(d->fragment_end) - pos + 1);
      p->endEditBlock();
      return;
   }

   p->aboutToRemoveCell(cellAt(0, pos).firstPosition(), cellAt(d->nRows - 1, pos + num - 1).lastPosition());

   QList<int> touchedCells;
   for (int r = 0; r < d->nRows; ++r) {
      for (int c = pos; c < pos + num; ++c) {
         int cell = d->grid[r * d->nCols + c];
         QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), cell);
         QTextCharFormat fmt = collection->charFormat(it->format);
         int span = fmt.tableCellColumnSpan();
         if (touchedCells.contains(cell) && span <= 1) {
            continue;
         }
         touchedCells << cell;

         if (span > 1) {
            fmt.setTableCellColumnSpan(span - 1);
            p->setCharFormat(it.position(), 1, fmt);
         } else {
            // remove cell
            int index = d->cells.indexOf(cell) + 1;
            int f_end = index < d->cells.size() ? d->cells.at(index) : d->fragment_end;
            p->remove(it.position(), p->fragmentMap().position(f_end) - it.position());
         }
      }
   }

   QTextTableFormat tfmt = format();
   tfmt.setColumns(tfmt.columns() - num);
   QVector<QTextLength> columnWidths = tfmt.columnWidthConstraints();
   if (columnWidths.count() > pos) {
      columnWidths.remove(pos, num);
      tfmt.setColumnWidthConstraints (columnWidths);
   }
   QTextObject::setFormat(tfmt);

   p->endEditBlock();
}

void QTextTable::mergeCells(int row, int column, int numRows, int numCols)
{
   Q_D(QTextTable);

   if (d->dirty) {
      d->update();
   }

   QTextDocumentPrivate *p = d->pieceTable;
   QTextFormatCollection *fc = p->formatCollection();

   const QTextTableCell cell = cellAt(row, column);
   if (!cell.isValid() || row != cell.row() || column != cell.column()) {
      return;
   }

   QTextCharFormat fmt = cell.format();
   const int rowSpan = fmt.tableCellRowSpan();
   const int colSpan = fmt.tableCellColumnSpan();

   numRows = qMin(numRows, rows() - cell.row());
   numCols = qMin(numCols, columns() - cell.column());

   // nothing to merge?
   if (numRows < rowSpan || numCols < colSpan) {
      return;
   }

   // check the edges of the merge rect to make sure no cell spans the edge
   for (int r = row; r < row + numRows; ++r) {
      if (cellAt(r, column) == cellAt(r, column - 1)) {
         return;
      }
      if (cellAt(r, column + numCols) == cellAt(r, column + numCols - 1)) {
         return;
      }
   }

   for (int c = column; c < column + numCols; ++c) {
      if (cellAt(row, c) == cellAt(row - 1, c)) {
         return;
      }
      if (cellAt(row + numRows, c) == cellAt(row + numRows - 1, c)) {
         return;
      }
   }

   p->beginEditBlock();

   const int origCellPosition = cell.firstPosition() - 1;

   const int cellFragment = d->grid[row * d->nCols + column];

   // find the position at which to insert the contents of the merged cells
   QFragmentFindHelper helper(origCellPosition, p->fragmentMap());

   QList<int>::iterator it = std::lower_bound(d->cells.begin(), d->cells.end(), helper);

   Q_ASSERT(it != d->cells.end());
   Q_ASSERT(! (helper < *it));
   Q_ASSERT(*it == cellFragment);

   const int insertCellIndex = it - d->cells.begin();
   int insertFragment = d->cells.value(insertCellIndex + 1, d->fragment_end);
   uint insertPos = p->fragmentMap().position(insertFragment);

   d->blockFragmentUpdates = true;

   bool rowHasText = cell.firstCursorPosition().block().length();
   bool needsParagraph = rowHasText && colSpan == numCols;

   // find all cells that will be erased by the merge
   for (int r = row; r < row + numRows; ++r) {
      int firstColumn = r < row + rowSpan ? column + colSpan : column;

      // don't recompute the cell index for the first row
      int firstCellIndex = r == row ? insertCellIndex + 1 : -1;
      int cellIndex = firstCellIndex;

      for (int c = firstColumn; c < column + numCols; ++c) {
         const int fragment = d->grid[r * d->nCols + c];

         // already handled?
         if (fragment == cellFragment) {
            continue;
         }

         QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), fragment);
         uint pos = it.position();

         if (firstCellIndex == -1) {
            QFragmentFindHelper helper(pos, p->fragmentMap());
            QList<int>::iterator it = std::lower_bound(d->cells.begin(), d->cells.end(), helper);

            Q_ASSERT(it != d->cells.end());
            Q_ASSERT(! (helper < *it));
            Q_ASSERT(*it == fragment);

            firstCellIndex = cellIndex = it - d->cells.begin();
         }

         ++cellIndex;

         QTextCharFormat fmt = fc->charFormat(it->format);

         const int cellRowSpan = fmt.tableCellRowSpan();
         const int cellColSpan = fmt.tableCellColumnSpan();

         // update the grid for this cell
         for (int i = r; i < r + cellRowSpan; ++i)
            for (int j = c; j < c + cellColSpan; ++j) {
               d->grid[i * d->nCols + j] = cellFragment;
            }

         // erase the cell marker
         p->remove(pos, 1);

         const int nextFragment = d->cells.value(cellIndex, d->fragment_end);
         const uint nextPos = p->fragmentMap().position(nextFragment);

         Q_ASSERT(nextPos >= pos);

         // merge the contents of the cell (if not empty)
         if (nextPos > pos) {
            if (needsParagraph) {
               needsParagraph = false;
               QTextCursorPrivate::fromPosition(p, insertPos++).insertBlock();
               p->move(pos + 1, insertPos, nextPos - pos);
            } else if (rowHasText) {
               QTextCursorPrivate::fromPosition(p, insertPos++).insertText(QLatin1String(" "));
               p->move(pos + 1, insertPos, nextPos - pos);
            } else {
               p->move(pos, insertPos, nextPos - pos);
            }

            insertPos += nextPos - pos;
            rowHasText = true;
         }
      }

      if (rowHasText) {
         needsParagraph = true;
         rowHasText = false;
      }

      // erase cells from last row
      if (firstCellIndex >= 0) {
         d->cellIndices.remove(firstCellIndex, cellIndex - firstCellIndex);
         d->cells.erase(d->cells.begin() + firstCellIndex, d->cells.begin() + cellIndex);
      }
   }

   d->fragment_start = d->cells.first();

   fmt.setTableCellRowSpan(numRows);
   fmt.setTableCellColumnSpan(numCols);
   p->setCharFormat(origCellPosition, 1, fmt);

   d->blockFragmentUpdates = false;
   d->dirty = false;

   p->endEditBlock();
}

void QTextTable::mergeCells(const QTextCursor &cursor)
{
   if (!cursor.hasComplexSelection()) {
      return;
   }

   int firstRow, numRows, firstColumn, numColumns;
   cursor.selectedTableCells(&firstRow, &numRows, &firstColumn, &numColumns);
   mergeCells(firstRow, firstColumn, numRows, numColumns);
}

void QTextTable::splitCell(int row, int column, int numRows, int numCols)
{
   Q_D(QTextTable);

   if (d->dirty) {
      d->update();
   }

   QTextDocumentPrivate *p = d->pieceTable;
   QTextFormatCollection *c = p->formatCollection();

   const QTextTableCell cell = cellAt(row, column);
   if (!cell.isValid()) {
      return;
   }
   row = cell.row();
   column = cell.column();

   QTextCharFormat fmt = cell.format();
   const int rowSpan = fmt.tableCellRowSpan();
   const int colSpan = fmt.tableCellColumnSpan();

   // nothing to split?
   if (numRows > rowSpan || numCols > colSpan) {
      return;
   }

   p->beginEditBlock();

   const int origCellPosition = cell.firstPosition() - 1;

   QVarLengthArray<int> rowPositions(rowSpan);

   rowPositions[0] = cell.lastPosition();

   for (int r = row + 1; r < row + rowSpan; ++r) {
      // find the cell before which to insert the new cell markers
      int gridIndex = r * d->nCols + column;

      QVector<int>::iterator it = std::upper_bound(d->cellIndices.begin(), d->cellIndices.end(), gridIndex);
      int cellIndex = it - d->cellIndices.begin();
      int fragment = d->cells.value(cellIndex, d->fragment_end);
      rowPositions[r - row] = p->fragmentMap().position(fragment);
   }

   fmt.setTableCellColumnSpan(1);
   fmt.setTableCellRowSpan(1);
   const int fmtIndex = c->indexForFormat(fmt);
   const int blockIndex = p->blockMap().find(cell.lastPosition())->format;

   int insertAdjustement = 0;
   for (int i = 0; i < numRows; ++i) {
      for (int c = 0; c < colSpan - numCols; ++c) {
         p->insertBlock(QTextBeginningOfFrame, rowPositions[i] + insertAdjustement + c, blockIndex, fmtIndex);
      }
      insertAdjustement += colSpan - numCols;
   }

   for (int i = numRows; i < rowSpan; ++i) {
      for (int c = 0; c < colSpan; ++c) {
         p->insertBlock(QTextBeginningOfFrame, rowPositions[i] + insertAdjustement + c, blockIndex, fmtIndex);
      }
      insertAdjustement += colSpan;
   }

   fmt.setTableCellRowSpan(numRows);
   fmt.setTableCellColumnSpan(numCols);
   p->setCharFormat(origCellPosition, 1, fmt);

   p->endEditBlock();
}

int QTextTable::rows() const
{
   Q_D(const QTextTable);
   if (d->dirty) {
      d->update();
   }

   return d->nRows;
}

int QTextTable::columns() const
{
   Q_D(const QTextTable);

   if (d->dirty) {
      d->update();
   }

   return d->nCols;
}

QTextCursor QTextTable::rowStart(const QTextCursor &c) const
{
   Q_D(const QTextTable);
   QTextTableCell cell = cellAt(c);
   if (!cell.isValid()) {
      return QTextCursor();
   }

   int row = cell.row();
   QTextDocumentPrivate *p = d->pieceTable;
   QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), d->grid[row * d->nCols]);

   return QTextCursorPrivate::fromPosition(p, it.position());
}

QTextCursor QTextTable::rowEnd(const QTextCursor &c) const
{
   Q_D(const QTextTable);
   QTextTableCell cell = cellAt(c);

   if (! cell.isValid()) {
      return QTextCursor();
   }

   int row = cell.row() + 1;
   int fragment = row < d->nRows ? d->grid[row * d->nCols] : d->fragment_end;
   QTextDocumentPrivate *p = d->pieceTable;
   QTextDocumentPrivate::FragmentIterator it(&p->fragmentMap(), fragment);

   return QTextCursorPrivate::fromPosition(p, it.position() - 1);
}

void QTextTable::setFormat(const QTextTableFormat &format)
{
   QTextTableFormat fmt = format;
   // don't try to change the number of table columns from here
   fmt.setColumns(columns());
   QTextObject::setFormat(fmt);
}
