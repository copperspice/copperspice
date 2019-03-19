/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qtextcursor.h>
#include <qtextcursor_p.h>
#include <qglobal.h>
#include <qtextdocumentfragment.h>
#include <qtextdocumentfragment_p.h>
#include <qtextlist.h>
#include <qtexttable.h>
#include <qtexttable_p.h>
#include <qtextengine_p.h>
#include <qabstracttextdocumentlayout.h>

#include <qtextlayout.h>
#include <qdebug.h>

enum {
   AdjustPrev = 0x1,
   AdjustUp   = 0x3,
   AdjustNext = 0x4,
   AdjustDown = 0x12
};

QTextCursorPrivate::QTextCursorPrivate(QTextDocumentPrivate *p)
   : priv(p), x(0), position(0), anchor(0), adjusted_anchor(0),
     currentCharFormat(-1), visualNavigation(false), keepPositionOnInsert(false),
     changed(false)
{
   priv->addCursor(this);
}

QTextCursorPrivate::QTextCursorPrivate(const QTextCursorPrivate &rhs)
   : QSharedData(rhs)
{
   position = rhs.position;
   anchor = rhs.anchor;
   adjusted_anchor = rhs.adjusted_anchor;
   priv = rhs.priv;
   x = rhs.x;
   currentCharFormat = rhs.currentCharFormat;
   visualNavigation = rhs.visualNavigation;
   keepPositionOnInsert = rhs.keepPositionOnInsert;
   changed = rhs.changed;
   priv->addCursor(this);
}

QTextCursorPrivate::~QTextCursorPrivate()
{
   if (priv) {
      priv->removeCursor(this);
   }
}

QTextCursorPrivate::AdjustResult QTextCursorPrivate::adjustPosition(int positionOfChange, int charsAddedOrRemoved,
      QTextUndoCommand::Operation op)
{
   QTextCursorPrivate::AdjustResult result = QTextCursorPrivate::CursorMoved;
   // not(!) <= , so that inserting text adjusts the cursor correctly
   if (position < positionOfChange
         || (position == positionOfChange
             && (op == QTextUndoCommand::KeepCursor
                 || keepPositionOnInsert)
            )
      ) {
      result = CursorUnchanged;
   } else {
      if (charsAddedOrRemoved < 0 && position < positionOfChange - charsAddedOrRemoved) {
         position = positionOfChange;
      } else {
         position += charsAddedOrRemoved;
      }

      currentCharFormat = -1;
   }

   if (anchor >= positionOfChange
         && (anchor != positionOfChange || op != QTextUndoCommand::KeepCursor)) {
      if (charsAddedOrRemoved < 0 && anchor < positionOfChange - charsAddedOrRemoved) {
         anchor = positionOfChange;
      } else {
         anchor += charsAddedOrRemoved;
      }
   }

   if (adjusted_anchor >= positionOfChange
         && (adjusted_anchor != positionOfChange || op != QTextUndoCommand::KeepCursor)) {
      if (charsAddedOrRemoved < 0 && adjusted_anchor < positionOfChange - charsAddedOrRemoved) {
         adjusted_anchor = positionOfChange;
      } else {
         adjusted_anchor += charsAddedOrRemoved;
      }
   }

   return result;
}

void QTextCursorPrivate::setX()
{
   if (priv->isInEditBlock()) {
      x = -1; // mark dirty
      return;
   }

   QTextBlock block = this->block();
   const QTextLayout *layout = blockLayout(block);
   int pos = position - block.position();

   QTextLine line = layout->lineForTextPosition(pos);
   if (line.isValid()) {
      x = line.cursorToX(pos);
   } else {
      x = -1;   // delayed init.  Makes movePosition() call setX later on again.
   }
}

void QTextCursorPrivate::remove()
{
   if (anchor == position) {
      return;
   }
   currentCharFormat = -1;
   int pos1 = position;
   int pos2 = adjusted_anchor;
   QTextUndoCommand::Operation op = QTextUndoCommand::KeepCursor;

   if (pos1 > pos2) {
      pos1 = adjusted_anchor;
      pos2 = position;
      op = QTextUndoCommand::MoveCursor;
   }

   // deleting inside table? -> delete only content
   QTextTable *table = complexSelectionTable();

   if (table) {
      priv->beginEditBlock();
      int startRow, startCol, numRows, numCols;
      selectedTableCells(&startRow, &numRows, &startCol, &numCols);
      clearCells(table, startRow, startCol, numRows, numCols, op);
      adjusted_anchor = anchor = position;
      priv->endEditBlock();

   } else {
      priv->remove(pos1, pos2 - pos1, op);
      adjusted_anchor = anchor = position;
      priv->finishEdit();
   }

}

void QTextCursorPrivate::clearCells(QTextTable *table, int startRow, int startCol, int numRows, int numCols,
                                    QTextUndoCommand::Operation op)
{
   priv->beginEditBlock();

   for (int row = startRow; row < startRow + numRows; ++row) {

      for (int col = startCol; col < startCol + numCols; ++col) {
         QTextTableCell cell = table->cellAt(row, col);
         const int startPos = cell.firstPosition();
         const int endPos = cell.lastPosition();
         Q_ASSERT(startPos <= endPos);
         priv->remove(startPos, endPos - startPos, op);
      }
   }

   priv->endEditBlock();
}

bool QTextCursorPrivate::canDelete(int pos) const
{
   QTextDocumentPrivate::FragmentIterator fit = priv->find(pos);
   QTextCharFormat fmt = priv->formatCollection()->charFormat((*fit)->format);
   return (fmt.objectIndex() == -1 || fmt.objectType() == QTextFormat::ImageObject);
}

void QTextCursorPrivate::insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat)
{
   QTextFormatCollection *formats = priv->formatCollection();
   int idx = formats->indexForFormat(format);
   Q_ASSERT(formats->format(idx).isBlockFormat());

   priv->insertBlock(position, idx, formats->indexForFormat(charFormat));
   currentCharFormat = -1;
}

void QTextCursorPrivate::adjustCursor(QTextCursor::MoveOperation m)
{
   adjusted_anchor = anchor;
   if (position == anchor) {
      return;
   }

   QTextFrame *f_position = priv->frameAt(position);
   QTextFrame *f_anchor   = priv->frameAt(adjusted_anchor);

   if (f_position != f_anchor) {
      // find common parent frame
      QList<QTextFrame *> positionChain;
      QList<QTextFrame *> anchorChain;
      QTextFrame *f = f_position;

      while (f) {
         positionChain.prepend(f);
         f = f->parentFrame();
      }
      f = f_anchor;

      while (f) {
         anchorChain.prepend(f);
         f = f->parentFrame();
      }
      Q_ASSERT(positionChain.at(0) == anchorChain.at(0));

      int i = 1;
      int l = qMin(positionChain.size(), anchorChain.size());

      for (; i < l; ++i) {
         if (positionChain.at(i) != anchorChain.at(i)) {
            break;
         }
      }

      if (m <= QTextCursor::WordLeft) {
         if (i < positionChain.size()) {
            position = positionChain.at(i)->firstPosition() - 1;
         }
      } else {
         if (i < positionChain.size()) {
            position = positionChain.at(i)->lastPosition() + 1;
         }
      }
      if (position < adjusted_anchor) {
         if (i < anchorChain.size()) {
            adjusted_anchor = anchorChain.at(i)->lastPosition() + 1;
         }
      } else {
         if (i < anchorChain.size()) {
            adjusted_anchor = anchorChain.at(i)->firstPosition() - 1;
         }
      }

      f_position = positionChain.at(i - 1);
   }

   // same frame, either need to adjust to cell boundaries or return
   QTextTable *table = qobject_cast<QTextTable *>(f_position);
   if (!table) {
      return;
   }

   QTextTableCell c_position = table->cellAt(position);
   QTextTableCell c_anchor = table->cellAt(adjusted_anchor);

   if (c_position != c_anchor) {
      bool before;
      int col_position = c_position.column();
      int col_anchor = c_anchor.column();

      if (col_position == col_anchor) {
         before = c_position.row() < c_anchor.row();
      } else {
         before = col_position < col_anchor;
      }

      // adjust to cell boundaries
      if (m <= QTextCursor::WordLeft) {
         position = c_position.firstPosition();
         if (!before) {
            --position;
         }
      } else {
         position = c_position.lastPosition();
         if (before) {
            ++position;
         }
      }
      if (position < adjusted_anchor) {
         adjusted_anchor = c_anchor.lastPosition();
      } else {
         adjusted_anchor = c_anchor.firstPosition();
      }
   }

   currentCharFormat = -1;
}

void QTextCursorPrivate::aboutToRemoveCell(int from, int to)
{
   Q_ASSERT(from <= to);
   if (position == anchor
) {
      return;
   }

   QTextTable *t = qobject_cast<QTextTable *>(priv->frameAt(position));
   if (!t) {
      return;
   }
   QTextTableCell removedCellFrom = t->cellAt(from);
   QTextTableCell removedCellEnd = t->cellAt(to);
   if (! removedCellFrom.isValid() || !removedCellEnd.isValid()) {
      return;
   }

   int curFrom = position;
   int curTo = adjusted_anchor;
   if (curTo < curFrom) {
      qSwap(curFrom, curTo);
   }

   QTextTableCell cellStart = t->cellAt(curFrom);
   QTextTableCell cellEnd = t->cellAt(curTo);

   if (cellStart.row() >= removedCellFrom.row() && cellEnd.row() <= removedCellEnd.row()
         && cellStart.column() >= removedCellFrom.column()
         && cellEnd.column() <= removedCellEnd.column()) {

      // selection is completely removed
      // find a new position, as close as possible to where we were.
      QTextTableCell cell;

      if (removedCellFrom.row() == 0 && removedCellEnd.row() == t->rows() - 1) { // removed n columns
         cell = t->cellAt(cellStart.row(), removedCellEnd.column() + 1);
      } else if (removedCellFrom.column() == 0 && removedCellEnd.column() == t->columns() - 1) { // removed n rows
         cell = t->cellAt(removedCellEnd.row() + 1, cellStart.column());
      }

      int newPosition;
      if (cell.isValid()) {
         newPosition = cell.firstPosition();
      } else {
         newPosition = t->lastPosition() + 1;
      }

      setPosition(newPosition);
      anchor = newPosition;
      adjusted_anchor = newPosition;
      x = 0;
   } else if (cellStart.row() >= removedCellFrom.row() && cellStart.row() <= removedCellEnd.row()
              && cellEnd.row() > removedCellEnd.row()) {
      int newPosition = t->cellAt(removedCellEnd.row() + 1, cellStart.column()).firstPosition();
      if (position < anchor) {
         position = newPosition;
      } else {
         anchor = adjusted_anchor = newPosition;
      }
   } else if (cellStart.column() >= removedCellFrom.column() && cellStart.column() <= removedCellEnd.column()
              && cellEnd.column() > removedCellEnd.column()) {
      int newPosition = t->cellAt(cellStart.row(), removedCellEnd.column() + 1).firstPosition();
      if (position < anchor) {
         position = newPosition;
      } else {
         anchor = adjusted_anchor = newPosition;
      }
   }
}

bool QTextCursorPrivate::movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode)
{
   currentCharFormat = -1;
   bool adjustX = true;
   QTextBlock blockIt = block();
   bool visualMovement = priv->defaultCursorMoveStyle == Qt::VisualMoveStyle;

   if (!blockIt.isValid()) {
      return false;
   }

   if (blockIt.textDirection() == Qt::RightToLeft) {
      if (op == QTextCursor::WordLeft) {
         op = QTextCursor::NextWord;
      } else if (op == QTextCursor::WordRight) {
         op = QTextCursor::PreviousWord;
      }

      if (!visualMovement) {
         if (op == QTextCursor::Left) {
            op = QTextCursor::NextCharacter;
         } else if (op == QTextCursor::Right) {
            op = QTextCursor::PreviousCharacter;
         }
      }
   }

   const QTextLayout *layout = blockLayout(blockIt);
   int relativePos = position - blockIt.position();
   QTextLine line;
   if (!priv->isInEditBlock()) {
      line = layout->lineForTextPosition(relativePos);
   }

   Q_ASSERT(priv->frameAt(position) == priv->frameAt(adjusted_anchor));

   int newPosition = position;

   if (x == -1 && !priv->isInEditBlock() && (op == QTextCursor::Up || op == QTextCursor::Down)) {
      setX();
   }

   switch (op) {
      case QTextCursor::NoMove:
         return true;

      case QTextCursor::Start:
         newPosition = 0;
         break;
      case QTextCursor::StartOfLine: {
         newPosition = blockIt.position();
         if (line.isValid()) {
            newPosition += line.textStart();
         }

         break;
      }
      case QTextCursor::StartOfBlock: {
         newPosition = blockIt.position();
         break;
      }
      case QTextCursor::PreviousBlock: {
         if (blockIt == priv->blocksBegin()) {
            return false;
         }
         blockIt = blockIt.previous();

         newPosition = blockIt.position();
         break;
      }
      case QTextCursor::PreviousCharacter:
#ifdef Q_OS_MAC
         if (mode == QTextCursor::MoveAnchor && position != adjusted_anchor) {
            newPosition = qMin(position, adjusted_anchor);
         } else
#endif
            newPosition = priv->previousCursorPosition(position, QTextLayout::SkipCharacters);
         break;
      case QTextCursor::Left:
#ifdef Q_OS_MAC
         if (mode == QTextCursor::MoveAnchor && position != adjusted_anchor)
            newPosition = visualMovement ? qMax(position, adjusted_anchor)
                          : qMin(position, adjusted_anchor);
         else
#endif
            newPosition = visualMovement ? priv->leftCursorPosition(position)
                          : priv->previousCursorPosition(position, QTextLayout::SkipCharacters);
         break;
      case QTextCursor::StartOfWord: {
         if (relativePos == 0) {
            break;
         }

         // skip if already at word start
         QTextEngine *engine = layout->engine();
         engine->attributes();
         if ((relativePos == blockIt.length() - 1)
               && (engine->atSpace(relativePos - 1) || engine->atWordSeparator(relativePos - 1))) {
            return false;
         }

         if (relativePos < blockIt.length() - 1) {
            ++position;
         }

         // FALL THROUGH!
      }
      case QTextCursor::PreviousWord:
      case QTextCursor::WordLeft:
         newPosition = priv->previousCursorPosition(position, QTextLayout::SkipWords);
         break;
      case QTextCursor::Up: {
         int i = line.lineNumber() - 1;
         if (i == -1) {
            if (blockIt == priv->blocksBegin()) {
               return false;
            }
            int blockPosition = blockIt.position();
            QTextTable *table = qobject_cast<QTextTable *>(priv->frameAt(blockPosition));
            if (table) {
               QTextTableCell cell = table->cellAt(blockPosition);
               if (cell.firstPosition() == blockPosition) {
                  int row = cell.row() - 1;
                  if (row >= 0) {
                     blockPosition = table->cellAt(row, cell.column()).lastPosition();
                  } else {
                     // move to line above the table
                     blockPosition = table->firstPosition() - 1;
                  }
                  blockIt = priv->blocksFind(blockPosition);
               } else {
                  blockIt = blockIt.previous();
               }
            } else {
               blockIt = blockIt.previous();
            }
            layout = blockLayout(blockIt);
            i = layout->lineCount() - 1;
         }
         if (layout->lineCount()) {
            QTextLine line = layout->lineAt(i);
            newPosition = line.xToCursor(x) + blockIt.position();
         } else {
            newPosition = blockIt.position();
         }
         adjustX = false;
         break;
      }

      case QTextCursor::End:
         newPosition = priv->length() - 1;
         break;
      case QTextCursor::EndOfLine: {
         if (!line.isValid() || line.textLength() == 0) {
            if (blockIt.length() >= 1)
               // position right before the block separator
            {
               newPosition = blockIt.position() + blockIt.length() - 1;
            }
            break;
         }
         newPosition = blockIt.position() + line.textStart() + line.textLength();
         if (newPosition >= priv->length()) {
            newPosition = priv->length() - 1;
         }
         if (line.lineNumber() < layout->lineCount() - 1) {
            const QString text = blockIt.text();
            // ###### this relies on spaces being the cause for linebreaks.
            // this doesn't work with japanese
            if (text.at(line.textStart() + line.textLength() - 1).isSpace()) {
               --newPosition;
            }
         }
         break;
      }
      case QTextCursor::EndOfWord: {
         QTextEngine *engine = layout->engine();
         engine->attributes();
         const int len = blockIt.length() - 1;
         if (relativePos >= len) {
            return false;
         }
         if (engine->atWordSeparator(relativePos)) {
            ++relativePos;
            while (relativePos < len && engine->atWordSeparator(relativePos)) {
               ++relativePos;
            }
         } else {
            while (relativePos < len && !engine->atSpace(relativePos) && !engine->atWordSeparator(relativePos)) {
               ++relativePos;
            }
         }
         newPosition = blockIt.position() + relativePos;
         break;
      }
      case QTextCursor::EndOfBlock:
         if (blockIt.length() >= 1)
            // position right before the block separator
         {
            newPosition = blockIt.position() + blockIt.length() - 1;
         }
         break;
      case QTextCursor::NextBlock: {
         blockIt = blockIt.next();
         if (!blockIt.isValid()) {
            return false;
         }

         newPosition = blockIt.position();
         break;
      }
      case QTextCursor::NextCharacter:
#ifdef Q_OS_MAC
         if (mode == QTextCursor::MoveAnchor && position != adjusted_anchor) {
            newPosition = qMax(position, adjusted_anchor);
         } else
#endif
            newPosition = priv->nextCursorPosition(position, QTextLayout::SkipCharacters);
         break;
      case QTextCursor::Right:
#ifdef Q_OS_MAC
         if (mode == QTextCursor::MoveAnchor && position != adjusted_anchor)
            newPosition = visualMovement ? qMin(position, adjusted_anchor)
                          : qMax(position, adjusted_anchor);
         else
#endif
            newPosition = visualMovement ? priv->rightCursorPosition(position)
                          : priv->nextCursorPosition(position, QTextLayout::SkipCharacters);
         break;
      case QTextCursor::NextWord:
      case QTextCursor::WordRight:
         newPosition = priv->nextCursorPosition(position, QTextLayout::SkipWords);
         break;

      case QTextCursor::Down: {
         int i = line.lineNumber() + 1;

         if (i >= layout->lineCount()) {
            int blockPosition = blockIt.position() + blockIt.length() - 1;
            QTextTable *table = qobject_cast<QTextTable *>(priv->frameAt(blockPosition));
            if (table) {
               QTextTableCell cell = table->cellAt(blockPosition);
               if (cell.lastPosition() == blockPosition) {
                  int row = cell.row() + cell.rowSpan();
                  if (row < table->rows()) {
                     blockPosition = table->cellAt(row, cell.column()).firstPosition();
                  } else {
                     // move to line below the table
                     blockPosition = table->lastPosition() + 1;
                  }
                  blockIt = priv->blocksFind(blockPosition);
               } else {
                  blockIt = blockIt.next();
               }
            } else {
               blockIt = blockIt.next();
            }

            if (blockIt == priv->blocksEnd()) {
               return false;
            }
            layout = blockLayout(blockIt);
            i = 0;
         }
         if (layout->lineCount()) {
            QTextLine line = layout->lineAt(i);
            newPosition = line.xToCursor(x) + blockIt.position();
         } else {
            newPosition = blockIt.position();
         }
         adjustX = false;
         break;
      }
      case QTextCursor::NextCell: // fall through
      case QTextCursor::PreviousCell: // fall through
      case QTextCursor::NextRow: // fall through
      case QTextCursor::PreviousRow: {
         QTextTable *table = qobject_cast<QTextTable *>(priv->frameAt(position));
         if (!table) {
            return false;
         }

         QTextTableCell cell = table->cellAt(position);
         Q_ASSERT(cell.isValid());
         int column = cell.column();
         int row = cell.row();
         const int currentRow = row;
         if (op == QTextCursor::NextCell || op == QTextCursor::NextRow) {
            do {
               column += cell.columnSpan();
               if (column >= table->columns()) {
                  column = 0;
                  ++row;
               }
               cell = table->cellAt(row, column);
               // note we also continue while we have not reached a cell thats not merged with one above us
            } while (cell.isValid()
                     && ((op == QTextCursor::NextRow && currentRow == cell.row())
                         || cell.row() < row));
         } else if (op == QTextCursor::PreviousCell || op == QTextCursor::PreviousRow) {
            do {
               --column;
               if (column < 0) {
                  column = table->columns() - 1;
                  --row;
               }
               cell = table->cellAt(row, column);
               // note we also continue while we have not reached a cell thats not merged with one above us
            } while (cell.isValid()
                     && ((op == QTextCursor::PreviousRow && currentRow == cell.row())
                         || cell.row() < row));
         }
         if (cell.isValid()) {
            newPosition = cell.firstPosition();
         }
         break;
      }
   }

   if (mode == QTextCursor::KeepAnchor) {
      QTextTable *table = qobject_cast<QTextTable *>(priv->frameAt(position));
      if (table && ((op >= QTextCursor::PreviousBlock && op <= QTextCursor::WordLeft)
                    || (op >= QTextCursor::NextBlock && op <= QTextCursor::WordRight))) {
         int oldColumn = table->cellAt(position).column();

         const QTextTableCell otherCell = table->cellAt(newPosition);
         if (!otherCell.isValid()) {
            return false;
         }

         int newColumn = otherCell.column();
         if ((oldColumn > newColumn && op >= QTextCursor::End)
               || (oldColumn < newColumn && op <= QTextCursor::WordLeft)) {
            return false;
         }
      }
   }

   const bool moved = setPosition(newPosition);

   if (mode == QTextCursor::MoveAnchor) {
      anchor = position;
      adjusted_anchor = position;
   } else {
      adjustCursor(op);
   }

   if (adjustX) {
      setX();
   }

   return moved;
}

QTextTable *QTextCursorPrivate::complexSelectionTable() const
{
   if (position == anchor) {
      return 0;
   }

   QTextTable *t = qobject_cast<QTextTable *>(priv->frameAt(position));
   if (t) {
      QTextTableCell cell_pos = t->cellAt(position);
      QTextTableCell cell_anchor = t->cellAt(adjusted_anchor);

      Q_ASSERT(cell_anchor.isValid());

      if (cell_pos == cell_anchor) {
         t = 0;
      }
   }
   return t;
}

void QTextCursorPrivate::selectedTableCells(int *firstRow, int *numRows, int *firstColumn, int *numColumns) const
{
   *firstRow = -1;
   *firstColumn = -1;
   *numRows = -1;
   *numColumns = -1;

   if (position == anchor) {
      return;
   }

   QTextTable *t = qobject_cast<QTextTable *>(priv->frameAt(position));
   if (!t) {
      return;
   }

   QTextTableCell cell_pos = t->cellAt(position);
   QTextTableCell cell_anchor = t->cellAt(adjusted_anchor);

   Q_ASSERT(cell_anchor.isValid());

   if (cell_pos == cell_anchor) {
      return;
   }

   *firstRow = qMin(cell_pos.row(), cell_anchor.row());
   *firstColumn = qMin(cell_pos.column(), cell_anchor.column());
   *numRows = qMax(cell_pos.row() + cell_pos.rowSpan(), cell_anchor.row() + cell_anchor.rowSpan()) - *firstRow;
   *numColumns = qMax(cell_pos.column() + cell_pos.columnSpan(),
                      cell_anchor.column() + cell_anchor.columnSpan()) - *firstColumn;
}

static void setBlockCharFormatHelper(QTextDocumentPrivate *priv, int pos1, int pos2,
                                     const QTextCharFormat &format, QTextDocumentPrivate::FormatChangeMode changeMode)
{
   QTextBlock it  = priv->blocksFind(pos1);
   QTextBlock end = priv->blocksFind(pos2);

   if (end.isValid()) {
      end = end.next();
   }

   for (; it != end; it = it.next()) {
      priv->setCharFormat(it.position() - 1, 1, format, changeMode);
   }
}

void QTextCursorPrivate::setBlockCharFormat(const QTextCharFormat &_format,
      QTextDocumentPrivate::FormatChangeMode changeMode)
{
   priv->beginEditBlock();

   QTextCharFormat format = _format;
   format.clearProperty(QTextFormat::ObjectIndex);

   QTextTable *table = complexSelectionTable();
   if (table) {
      int row_start, col_start, num_rows, num_cols;
      selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);

      Q_ASSERT(row_start != -1);
      for (int r = row_start; r < row_start + num_rows; ++r) {
         for (int c = col_start; c < col_start + num_cols; ++c) {
            QTextTableCell cell = table->cellAt(r, c);
            int rspan = cell.rowSpan();
            int cspan = cell.columnSpan();
            if (rspan != 1) {
               int cr = cell.row();
               if (cr != r) {
                  continue;
               }
            }
            if (cspan != 1) {
               int cc = cell.column();
               if (cc != c) {
                  continue;
               }
            }

            int pos1 = cell.firstPosition();
            int pos2 = cell.lastPosition();
            setBlockCharFormatHelper(priv, pos1, pos2, format, changeMode);
         }
      }
   } else {
      int pos1 = position;
      int pos2 = adjusted_anchor;
      if (pos1 > pos2) {
         pos1 = adjusted_anchor;
         pos2 = position;
      }

      setBlockCharFormatHelper(priv, pos1, pos2, format, changeMode);
   }
   priv->endEditBlock();
}


void QTextCursorPrivate::setBlockFormat(const QTextBlockFormat &format,
                                        QTextDocumentPrivate::FormatChangeMode changeMode)
{
   QTextTable *table = complexSelectionTable();
   if (table) {
      priv->beginEditBlock();
      int row_start, col_start, num_rows, num_cols;
      selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);

      Q_ASSERT(row_start != -1);
      for (int r = row_start; r < row_start + num_rows; ++r) {
         for (int c = col_start; c < col_start + num_cols; ++c) {
            QTextTableCell cell = table->cellAt(r, c);
            int rspan = cell.rowSpan();
            int cspan = cell.columnSpan();
            if (rspan != 1) {
               int cr = cell.row();
               if (cr != r) {
                  continue;
               }
            }
            if (cspan != 1) {
               int cc = cell.column();
               if (cc != c) {
                  continue;
               }
            }

            int pos1 = cell.firstPosition();
            int pos2 = cell.lastPosition();
            priv->setBlockFormat(priv->blocksFind(pos1), priv->blocksFind(pos2), format, changeMode);
         }
      }
      priv->endEditBlock();
   } else {
      int pos1 = position;
      int pos2 = adjusted_anchor;
      if (pos1 > pos2) {
         pos1 = adjusted_anchor;
         pos2 = position;
      }

      priv->setBlockFormat(priv->blocksFind(pos1), priv->blocksFind(pos2), format, changeMode);
   }
}

void QTextCursorPrivate::setCharFormat(const QTextCharFormat &_format,
                                       QTextDocumentPrivate::FormatChangeMode changeMode)
{
   Q_ASSERT(position != anchor);

   QTextCharFormat format = _format;
   format.clearProperty(QTextFormat::ObjectIndex);

   QTextTable *table = complexSelectionTable();
   if (table) {
      priv->beginEditBlock();
      int row_start, col_start, num_rows, num_cols;
      selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);

      Q_ASSERT(row_start != -1);
      for (int r = row_start; r < row_start + num_rows; ++r) {
         for (int c = col_start; c < col_start + num_cols; ++c) {
            QTextTableCell cell = table->cellAt(r, c);
            int rspan = cell.rowSpan();
            int cspan = cell.columnSpan();
            if (rspan != 1) {
               int cr = cell.row();
               if (cr != r) {
                  continue;
               }
            }
            if (cspan != 1) {
               int cc = cell.column();
               if (cc != c) {
                  continue;
               }
            }

            int pos1 = cell.firstPosition();
            int pos2 = cell.lastPosition();
            priv->setCharFormat(pos1, pos2 - pos1, format, changeMode);
         }
      }
      priv->endEditBlock();
   } else {
      int pos1 = position;
      int pos2 = adjusted_anchor;
      if (pos1 > pos2) {
         pos1 = adjusted_anchor;
         pos2 = position;
      }

      priv->setCharFormat(pos1, pos2 - pos1, format, changeMode);
   }
}


QTextLayout *QTextCursorPrivate::blockLayout(QTextBlock &block) const
{
   QTextLayout *tl = block.layout();
   if (!tl->lineCount() && priv->layout()) {
      priv->layout()->blockBoundingRect(block);
   }
   return tl;
}

QTextCursor::QTextCursor()
   : d(0)
{
}

/*!
    Constructs a cursor pointing to the beginning of the \a document.
 */
QTextCursor::QTextCursor(QTextDocument *document)
   : d(new QTextCursorPrivate(document->docHandle()))
{
}

/*!
    Constructs a cursor pointing to the beginning of the \a frame.
*/
QTextCursor::QTextCursor(QTextFrame *frame)
   : d(new QTextCursorPrivate(frame->document()->docHandle()))
{
   d->adjusted_anchor = d->anchor = d->position = frame->firstPosition();
}


/*!
    Constructs a cursor pointing to the beginning of the \a block.
*/
QTextCursor::QTextCursor(const QTextBlock &block)
   : d(new QTextCursorPrivate(block.docHandle()))
{
   d->adjusted_anchor = d->anchor = d->position = block.position();
}


/*!
  \internal
 */
QTextCursor::QTextCursor(QTextDocumentPrivate *p, int pos)
   : d(new QTextCursorPrivate(p))
{
   d->adjusted_anchor = d->anchor = d->position = pos;

   d->setX();
}

/*!
    \internal
*/
QTextCursor::QTextCursor(QTextCursorPrivate *d)
{
   Q_ASSERT(d);
   this->d = d;
}

/*!
    Constructs a new cursor that is a copy of \a cursor.
 */
QTextCursor::QTextCursor(const QTextCursor &cursor)
{
   d = cursor.d;
}

/*!
    Makes a copy of \a cursor and assigns it to this QTextCursor. Note
    that QTextCursor is an \l{Implicitly Shared Classes}{implicitly
    shared} class.

 */
QTextCursor &QTextCursor::operator=(const QTextCursor &cursor)
{
   d = cursor.d;
   return *this;
}

/*!
    Destroys the QTextCursor.
 */
QTextCursor::~QTextCursor()
{
}

/*!
    Returns true if the cursor is null; otherwise returns false. A null
    cursor is created by the default constructor.
 */
bool QTextCursor::isNull() const
{
   return !d || !d->priv;
}

/*!
    Moves the cursor to the absolute position in the document specified by
    \a pos using a \c MoveMode specified by \a m. The cursor is positioned
    between characters.

    \sa position() movePosition() anchor()
*/
void QTextCursor::setPosition(int pos, MoveMode m)
{
   if (!d || !d->priv) {
      return;
   }

   if (pos < 0 || pos >= d->priv->length()) {
      qWarning("QTextCursor::setPosition: Position '%d' out of range", pos);
      return;
   }

   d->setPosition(pos);
   if (m == MoveAnchor) {
      d->anchor = pos;
      d->adjusted_anchor = pos;
   } else { // keep anchor
      QTextCursor::MoveOperation op;
      if (pos < d->anchor) {
         op = QTextCursor::Left;
      } else {
         op = QTextCursor::Right;
      }
      d->adjustCursor(op);
   }
   d->setX();
}

/*!
    Returns the absolute position of the cursor within the document.
    The cursor is positioned between characters.

    \sa setPosition() movePosition() anchor() positionInBlock()
*/
int QTextCursor::position() const
{
   if (!d || !d->priv) {
      return -1;
   }
   return d->position;
}

/*!
    \since 4.7
    Returns the relative position of the cursor within the block.
    The cursor is positioned between characters.

    This is equivalent to \c{ position() - block().position()}.

    \sa position()
*/
int QTextCursor::positionInBlock() const
{
   if (!d || !d->priv) {
      return 0;
   }
   return d->position - d->block().position();
}

/*!
    Returns the anchor position; this is the same as position() unless
    there is a selection in which case position() marks one end of the
    selection and anchor() marks the other end. Just like the cursor
    position, the anchor position is between characters.

    \sa position() setPosition() movePosition() selectionStart() selectionEnd()
*/
int QTextCursor::anchor() const
{
   if (!d || !d->priv) {
      return -1;
   }
   return d->anchor;
}

bool QTextCursor::movePosition(MoveOperation op, MoveMode mode, int n)
{
   if (!d || !d->priv) {
      return false;
   }

   switch (op) {
      case Start:
      case StartOfLine:
      case End:
      case EndOfLine:
         n = 1;
         break;
      default:
         break;
   }

   int previousPosition = d->position;
   for (; n > 0; --n) {
      if (!d->movePosition(op, mode)) {
         return false;
      }
   }

   if (d->visualNavigation && !d->block().isVisible()) {
      QTextBlock b = d->block();
      if (previousPosition < d->position) {
         while (!b.next().isVisible()) {
            b = b.next();
         }
         d->setPosition(b.position() + b.length() - 1);
      } else {
         while (!b.previous().isVisible()) {
            b = b.previous();
         }
         d->setPosition(b.position());
      }
      if (mode == QTextCursor::MoveAnchor) {
         d->anchor = d->position;
      }
      while (d->movePosition(op, mode)
             && !d->block().isVisible())
         ;

   }
   return true;
}

bool QTextCursor::visualNavigation() const
{
   return d ? d->visualNavigation : false;
}

void QTextCursor::setVisualNavigation(bool b)
{
   if (d) {
      d->visualNavigation = b;
   }
}

void QTextCursor::setVerticalMovementX(int x)
{
   if (d) {
      d->x = x;
   }
}

int QTextCursor::verticalMovementX() const
{
   return d ? d->x : -1;
}

bool QTextCursor::keepPositionOnInsert() const
{
   return d ? d->keepPositionOnInsert : false;
}

void QTextCursor::setKeepPositionOnInsert(bool b)
{
   if (d) {
      d->keepPositionOnInsert = b;
   }
}

void QTextCursor::insertText(const QString &text)
{
   QTextCharFormat fmt = charFormat();
   fmt.clearProperty(QTextFormat::ObjectType);
   insertText(text, fmt);
}

void QTextCursor::insertText(const QString &text, const QTextCharFormat &_format)
{
   if (! d || ! d->priv) {
      return;
   }

   Q_ASSERT(_format.isValid());

   QTextCharFormat format = _format;
   format.clearProperty(QTextFormat::ObjectIndex);

   bool hasEditBlock = false;

   if (d->anchor != d->position) {
      hasEditBlock = true;
      d->priv->beginEditBlock();
      d->remove();
   }

   if (! text.isEmpty()) {
      QTextFormatCollection *formats = d->priv->formatCollection();
      int formatIdx = formats->indexForFormat(format);

      Q_ASSERT(formats->format(formatIdx).isCharFormat());

      QTextBlockFormat blockFmt = blockFormat();

      int textStart  = d->priv->text.length();
      int blockStart = 0;

      d->priv->text += text;

      int textEnd = d->priv->text.length();
      int i = 0;

      QString::const_iterator nextCh = text.constBegin() + 1;

      for (auto ch : text) {
         const int blockEnd = i;

         if (ch == '\r' && nextCh < text.constEnd() && *nextCh == '\n') {
            ++i;
            ++nextCh;

            continue;
         }

         if (ch == '\n' || ch == '\r' || ch == QChar::ParagraphSeparator || ch == QTextBeginningOfFrame
                  || ch == QTextEndOfFrame) {

            if (! hasEditBlock) {
               hasEditBlock = true;
               d->priv->beginEditBlock();
            }

            if (blockEnd > blockStart) {
               d->priv->insert(d->position, textStart + blockStart, blockEnd - blockStart, formatIdx);
            }

            d->insertBlock(blockFmt, format);
            blockStart = i + 1;
         }

         ++i;
         ++nextCh;
      }

      if (textStart + blockStart < textEnd) {
         d->priv->insert(d->position, textStart + blockStart, textEnd - textStart - blockStart, formatIdx);
      }
   }

   if (hasEditBlock) {
      d->priv->endEditBlock();
   }

   d->setX();
}

void QTextCursor::deleteChar()
{
   if (!d || !d->priv) {
      return;
   }

   if (d->position != d->anchor) {
      removeSelectedText();
      return;
   }

   if (!d->canDelete(d->position)) {
      return;
   }

   d->adjusted_anchor = d->anchor = d->priv->nextCursorPosition(d->anchor, QTextLayout::SkipCharacters);
   d->remove();
   d->setX();
}

void QTextCursor::deletePreviousChar()
{
   if (! d || !d->priv) {
      return;
   }

   if (d->position != d->anchor) {
      removeSelectedText();
      return;
   }

   if (d->anchor < 1 || !d->canDelete(d->anchor - 1)) {
      return;
   }
   d->anchor--;

   QTextDocumentPrivate::FragmentIterator fragIt = d->priv->find(d->anchor);
   const QTextFragmentData *const frag = fragIt.value();

   int fpos = fragIt.position();

   d->adjusted_anchor = d->anchor;
   d->remove();
   d->setX();
}

/*!
    Selects text in the document according to the given \a selection.
*/
void QTextCursor::select(SelectionType selection)
{
   if (!d || !d->priv) {
      return;
   }

   clearSelection();

   const QTextBlock block = d->block();

   switch (selection) {
      case LineUnderCursor:
         movePosition(StartOfLine);
         movePosition(EndOfLine, KeepAnchor);
         break;

      case WordUnderCursor:
         movePosition(StartOfWord);
         movePosition(EndOfWord, KeepAnchor);
         break;
      case BlockUnderCursor:
         if (block.length() == 1) { // no content
            break;
         }
         movePosition(StartOfBlock);
         // also select the paragraph separator
         if (movePosition(PreviousBlock)) {
            movePosition(EndOfBlock);
            movePosition(NextBlock, KeepAnchor);
         }
         movePosition(EndOfBlock, KeepAnchor);
         break;

      case Document:
         movePosition(Start);
         movePosition(End, KeepAnchor);
         break;
   }
}

bool QTextCursor::hasSelection() const
{
   return !!d && d->position != d->anchor;
}

bool QTextCursor::hasComplexSelection() const
{
   if (!d) {
      return false;
   }

   return d->complexSelectionTable() != 0;
}

void QTextCursor::selectedTableCells(int *firstRow, int *numRows, int *firstColumn, int *numColumns) const
{
   *firstRow    = -1;
   *firstColumn = -1;
   *numRows     = -1;
   *numColumns  = -1;

   if (!d || d->position == d->anchor) {
      return;
   }

   d->selectedTableCells(firstRow, numRows, firstColumn, numColumns);
}

void QTextCursor::clearSelection()
{
   if (!d) {
      return;
   }
   d->adjusted_anchor = d->anchor = d->position;
   d->currentCharFormat = -1;
}

/*!
    If there is a selection, its content is deleted; otherwise does
    nothing.

    \sa hasSelection()
*/
void QTextCursor::removeSelectedText()
{
   if (!d || !d->priv || d->position == d->anchor) {
      return;
   }

   d->priv->beginEditBlock();
   d->remove();
   d->priv->endEditBlock();
   d->setX();
}

/*!
    Returns the start of the selection or position() if the
    cursor doesn't have a selection.

    \sa selectionEnd() position() anchor()
*/
int QTextCursor::selectionStart() const
{
   if (!d || !d->priv) {
      return -1;
   }
   return qMin(d->position, d->adjusted_anchor);
}

int QTextCursor::selectionEnd() const
{
   if (!d || !d->priv) {
      return -1;
   }
   return qMax(d->position, d->adjusted_anchor);
}

static void getText(QString &text, QTextDocumentPrivate *priv, const QString &docText, int pos, int end)
{
   while (pos < end) {
      QTextDocumentPrivate::FragmentIterator fragIt = priv->find(pos);
      const QTextFragmentData *const frag = fragIt.value();

      const int offsetInFragment = qMax(0, pos - fragIt.position());
      const int len = qMin(int(frag->size_array[0] - offsetInFragment), end - pos);

      text += docText.mid(frag->stringPosition + offsetInFragment, len);
      pos += len;
   }
}

QString QTextCursor::selectedText() const
{
   if (! d || !d->priv || d->position == d->anchor) {
      return QString();
   }

   const QString docText = d->priv->buffer();
   QString text;

   QTextTable *table = d->complexSelectionTable();
   if (table) {
      int row_start, col_start, num_rows, num_cols;
      selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);

      Q_ASSERT(row_start != -1);
      for (int r = row_start; r < row_start + num_rows; ++r) {

         for (int c = col_start; c < col_start + num_cols; ++c) {
            QTextTableCell cell = table->cellAt(r, c);
            int rspan = cell.rowSpan();
            int cspan = cell.columnSpan();

            if (rspan != 1) {
               int cr = cell.row();
               if (cr != r) {
                  continue;
               }
            }
            if (cspan != 1) {
               int cc = cell.column();
               if (cc != c) {
                  continue;
               }
            }

            getText(text, d->priv, docText, cell.firstPosition(), cell.lastPosition());
         }
      }
   } else {
      getText(text, d->priv, docText, selectionStart(), selectionEnd());
   }

   return text;
}

QTextDocumentFragment QTextCursor::selection() const
{
   return QTextDocumentFragment(*this);
}

/*!
    Returns the block that contains the cursor.
*/
QTextBlock QTextCursor::block() const
{
   if (!d || !d->priv) {
      return QTextBlock();
   }
   return d->block();
}

QTextBlockFormat QTextCursor::blockFormat() const
{
   if (!d || !d->priv) {
      return QTextBlockFormat();
   }

   return d->block().blockFormat();
}

void QTextCursor::setBlockFormat(const QTextBlockFormat &format)
{
   if (!d || !d->priv) {
      return;
   }

   d->setBlockFormat(format, QTextDocumentPrivate::SetFormat);
}

void QTextCursor::mergeBlockFormat(const QTextBlockFormat &modifier)
{
   if (!d || !d->priv) {
      return;
   }

   d->setBlockFormat(modifier, QTextDocumentPrivate::MergeFormat);
}

QTextCharFormat QTextCursor::blockCharFormat() const
{
   if (!d || !d->priv) {
      return QTextCharFormat();
   }

   return d->block().charFormat();
}

void QTextCursor::setBlockCharFormat(const QTextCharFormat &format)
{
   if (!d || !d->priv) {
      return;
   }

   d->setBlockCharFormat(format, QTextDocumentPrivate::SetFormatAndPreserveObjectIndices);
}

void QTextCursor::mergeBlockCharFormat(const QTextCharFormat &modifier)
{
   if (!d || !d->priv) {
      return;
   }

   d->setBlockCharFormat(modifier, QTextDocumentPrivate::MergeFormat);
}

QTextCharFormat QTextCursor::charFormat() const
{
   if (!d || !d->priv) {
      return QTextCharFormat();
   }

   int idx = d->currentCharFormat;
   if (idx == -1) {
      QTextBlock block = d->block();

      int pos;
      if (d->position == block.position()
            && block.length() > 1) {
         pos = d->position;
      } else {
         pos = d->position - 1;
      }

      if (pos == -1) {
         idx = d->priv->blockCharFormatIndex(d->priv->blockMap().firstNode());
      } else {
         Q_ASSERT(pos >= 0 && pos < d->priv->length());

         QTextDocumentPrivate::FragmentIterator it = d->priv->find(pos);
         Q_ASSERT(!it.atEnd());
         idx = it.value()->format;
      }
   }

   QTextCharFormat cfmt = d->priv->formatCollection()->charFormat(idx);
   cfmt.clearProperty(QTextFormat::ObjectIndex);

   Q_ASSERT(cfmt.isValid());
   return cfmt;
}

/*!
    Sets the cursor's current character format to the given \a
    format. If the cursor has a selection, the given \a format is
    applied to the current selection.

    \sa hasSelection(), mergeCharFormat()
*/
void QTextCursor::setCharFormat(const QTextCharFormat &format)
{
   if (!d || !d->priv) {
      return;
   }
   if (d->position == d->anchor) {
      d->currentCharFormat = d->priv->formatCollection()->indexForFormat(format);
      return;
   }
   d->setCharFormat(format, QTextDocumentPrivate::SetFormatAndPreserveObjectIndices);
}

/*!
    Merges the cursor's current character format with the properties
    described by format \a modifier. If the cursor has a selection,
    this function applies all the properties set in \a modifier to all
    the character formats that are part of the selection.

    \sa hasSelection(), setCharFormat()
*/
void QTextCursor::mergeCharFormat(const QTextCharFormat &modifier)
{
   if (!d || !d->priv) {
      return;
   }
   if (d->position == d->anchor) {
      QTextCharFormat format = charFormat();
      format.merge(modifier);
      d->currentCharFormat = d->priv->formatCollection()->indexForFormat(format);
      return;
   }

   d->setCharFormat(modifier, QTextDocumentPrivate::MergeFormat);
}

/*!
    Returns true if the cursor is at the start of a block; otherwise
    returns false.

    \sa atBlockEnd(), atStart()
*/
bool QTextCursor::atBlockStart() const
{
   if (!d || !d->priv) {
      return false;
   }

   return d->position == d->block().position();
}

/*!
    Returns true if the cursor is at the end of a block; otherwise
    returns false.

    \sa atBlockStart(), atEnd()
*/
bool QTextCursor::atBlockEnd() const
{
   if (!d || !d->priv) {
      return false;
   }

   return d->position == d->block().position() + d->block().length() - 1;
}

/*!
    Returns true if the cursor is at the start of the document;
    otherwise returns false.

    \sa atBlockStart(), atEnd()
*/
bool QTextCursor::atStart() const
{
   if (!d || !d->priv) {
      return false;
   }

   return d->position == 0;
}

/*!
    \since 4.6

    Returns true if the cursor is at the end of the document;
    otherwise returns false.

    \sa atStart(), atBlockEnd()
*/
bool QTextCursor::atEnd() const
{
   if (!d || !d->priv) {
      return false;
   }

   return d->position == d->priv->length() - 1;
}

/*!
    Inserts a new empty block at the cursor position() with the
    current blockFormat() and charFormat().

    \sa setBlockFormat()
*/
void QTextCursor::insertBlock()
{
   insertBlock(blockFormat());
}

/*!
    \overload

    Inserts a new empty block at the cursor position() with block
    format \a format and the current charFormat() as block char format.

    \sa setBlockFormat()
*/
void QTextCursor::insertBlock(const QTextBlockFormat &format)
{
   QTextCharFormat charFmt = charFormat();
   charFmt.clearProperty(QTextFormat::ObjectType);
   insertBlock(format, charFmt);
}

/*!
    \fn void QTextCursor::insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat)
    \overload

    Inserts a new empty block at the cursor position() with block
    format \a format and \a charFormat as block char format.

    \sa setBlockFormat()
*/
void QTextCursor::insertBlock(const QTextBlockFormat &format, const QTextCharFormat &_charFormat)
{
   if (!d || !d->priv) {
      return;
   }

   QTextCharFormat charFormat = _charFormat;
   charFormat.clearProperty(QTextFormat::ObjectIndex);

   d->priv->beginEditBlock();
   d->remove();
   d->insertBlock(format, charFormat);
   d->priv->endEditBlock();
   d->setX();
}

/*!
    Inserts a new block at the current position and makes it the first
    list item of a newly created list with the given \a format. Returns
    the created list.

    \sa currentList() createList() insertBlock()
 */
QTextList *QTextCursor::insertList(const QTextListFormat &format)
{
   insertBlock();
   return createList(format);
}

/*!
    \overload

    Inserts a new block at the current position and makes it the first
    list item of a newly created list with the given \a style. Returns
    the created list.

    \sa currentList(), createList(), insertBlock()
 */
QTextList *QTextCursor::insertList(QTextListFormat::Style style)
{
   insertBlock();
   return createList(style);
}

/*!
    Creates and returns a new list with the given \a format, and makes the
    current paragraph the cursor is in the first list item.

    \sa insertList() currentList()
 */
QTextList *QTextCursor::createList(const QTextListFormat &format)
{
   if (!d || !d->priv) {
      return 0;
   }

   QTextList *list = static_cast<QTextList *>(d->priv->createObject(format));
   QTextBlockFormat modifier;
   modifier.setObjectIndex(list->objectIndex());
   mergeBlockFormat(modifier);
   return list;
}

/*!
    \overload

    Creates and returns a new list with the given \a style, making the
    cursor's current paragraph the first list item.

    The style to be used is defined by the QTextListFormat::Style enum.

    \sa insertList() currentList()
 */
QTextList *QTextCursor::createList(QTextListFormat::Style style)
{
   QTextListFormat fmt;
   fmt.setStyle(style);
   return createList(fmt);
}

/*!
    Returns the current list if the cursor position() is inside a
    block that is part of a list; otherwise returns 0.

    \sa insertList() createList()
 */
QTextList *QTextCursor::currentList() const
{
   if (!d || !d->priv) {
      return 0;
   }

   QTextBlockFormat b = blockFormat();
   QTextObject *o = d->priv->objectForFormat(b);
   return qobject_cast<QTextList *>(o);
}

/*!
    \fn QTextTable *QTextCursor::insertTable(int rows, int columns)

    \overload

    Creates a new table with the given number of \a rows and \a columns,
    inserts it at the current cursor position() in the document, and returns
    the table object. The cursor is moved to the beginning of the first cell.

    There must be at least one row and one column in the table.

    \sa currentTable()
 */
QTextTable *QTextCursor::insertTable(int rows, int cols)
{
   return insertTable(rows, cols, QTextTableFormat());
}

/*!
    \fn QTextTable *QTextCursor::insertTable(int rows, int columns, const QTextTableFormat &format)

    Creates a new table with the given number of \a rows and \a columns
    in the specified \a format, inserts it at the current cursor position()
    in the document, and returns the table object. The cursor is moved to
    the beginning of the first cell.

    There must be at least one row and one column in the table.

    \sa currentTable()
*/
QTextTable *QTextCursor::insertTable(int rows, int cols, const QTextTableFormat &format)
{
   if (!d || !d->priv || rows == 0 || cols == 0) {
      return 0;
   }

   int pos = d->position;
   QTextTable *t = QTextTablePrivate::createTable(d->priv, d->position, rows, cols, format);
   d->setPosition(pos + 1);
   // ##### what should we do if we have a selection?
   d->anchor = d->position;
   d->adjusted_anchor = d->anchor;
   return t;
}

/*!
    Returns a pointer to the current table if the cursor position()
    is inside a block that is part of a table; otherwise returns 0.

    \sa insertTable()
*/
QTextTable *QTextCursor::currentTable() const
{
   if (!d || !d->priv) {
      return 0;
   }

   QTextFrame *frame = d->priv->frameAt(d->position);
   while (frame) {
      QTextTable *table = qobject_cast<QTextTable *>(frame);
      if (table) {
         return table;
      }
      frame = frame->parentFrame();
   }
   return 0;
}

/*!
    Inserts a frame with the given \a format at the current cursor position(),
    moves the cursor position() inside the frame, and returns the frame.

    If the cursor holds a selection, the whole selection is moved inside the
    frame.

    \sa hasSelection()
*/
QTextFrame *QTextCursor::insertFrame(const QTextFrameFormat &format)
{
   if (!d || !d->priv) {
      return 0;
   }

   return d->priv->insertFrame(selectionStart(), selectionEnd(), format);
}

/*!
    Returns a pointer to the current frame. Returns 0 if the cursor is invalid.

    \sa insertFrame()
*/
QTextFrame *QTextCursor::currentFrame() const
{
   if (!d || !d->priv) {
      return 0;
   }

   return d->priv->frameAt(d->position);
}


/*!
    Inserts the text \a fragment at the current position().
*/
void QTextCursor::insertFragment(const QTextDocumentFragment &fragment)
{
   if (!d || !d->priv || fragment.isEmpty()) {
      return;
   }

   d->priv->beginEditBlock();
   d->remove();
   fragment.d->insert(*this);
   d->priv->endEditBlock();

   if (fragment.d && fragment.d->doc) {
      d->priv->mergeCachedResources(fragment.d->doc->docHandle());
   }
}

/*!
    \since 4.2
    Inserts the text \a html at the current position(). The text is interpreted as
    HTML.

    \note When using this function with a style sheet, the style sheet will
    only apply to the current block in the document. In order to apply a style
    sheet throughout a document, use QTextDocument::setDefaultStyleSheet()
    instead.
*/

#ifndef QT_NO_TEXTHTMLPARSER

void QTextCursor::insertHtml(const QString &html)
{
   if (!d || !d->priv) {
      return;
   }
   QTextDocumentFragment fragment = QTextDocumentFragment::fromHtml(html, d->priv->document());
   insertFragment(fragment);
}

#endif // QT_NO_TEXTHTMLPARSER

/*!
    \overload
    \since 4.2

    Inserts the image defined by the given \a format at the cursor's current position
    with the specified \a alignment.

    \sa position()
*/
void QTextCursor::insertImage(const QTextImageFormat &format, QTextFrameFormat::Position alignment)
{
   if (!d || !d->priv) {
      return;
   }

   QTextFrameFormat ffmt;
   ffmt.setPosition(alignment);
   QTextObject *obj = d->priv->createObject(ffmt);

   QTextImageFormat fmt = format;
   fmt.setObjectIndex(obj->objectIndex());

   d->priv->beginEditBlock();
   d->remove();
   const int idx = d->priv->formatCollection()->indexForFormat(fmt);
   d->priv->insert(d->position, QString(QChar(QChar::ObjectReplacementCharacter)), idx);
   d->priv->endEditBlock();
}

/*!
    Inserts the image defined by \a format at the current position().
*/
void QTextCursor::insertImage(const QTextImageFormat &format)
{
   insertText(QString(QChar::ObjectReplacementCharacter), format);
}

/*!
    \overload

    Convenience method for inserting the image with the given \a name at the
    current position().

    \snippet doc/src/snippets/code/src_gui_text_qtextcursor.cpp 1
*/
void QTextCursor::insertImage(const QString &name)
{
   QTextImageFormat format;
   format.setName(name);
   insertImage(format);
}

/*!
    \since 4.5
    \overload

    Convenience function for inserting the given \a image with an optional
    \a name at the current position().
*/
void QTextCursor::insertImage(const QImage &image, const QString &name)
{
   if (image.isNull()) {
      qWarning("QTextCursor::insertImage: attempt to add an invalid image");
      return;
   }
   QString imageName = name;
   if (name.isEmpty()) {
      imageName = QString::number(image.serialNumber());
   }
   d->priv->document()->addResource(QTextDocument::ImageResource, QUrl(imageName), image);
   QTextImageFormat format;
   format.setName(imageName);
   insertImage(format);
}

/*!
    \fn bool QTextCursor::operator!=(const QTextCursor &other) const

    Returns true if the \a other cursor is at a different position in
    the document as this cursor; otherwise returns false.
*/
bool QTextCursor::operator!=(const QTextCursor &rhs) const
{
   return !operator==(rhs);
}

/*!
    \fn bool QTextCursor::operator<(const QTextCursor &other) const

    Returns true if the \a other cursor is positioned later in the
    document than this cursor; otherwise returns false.
*/
bool QTextCursor::operator<(const QTextCursor &rhs) const
{
   if (!d) {
      return !!rhs.d;
   }

   if (!rhs.d) {
      return false;
   }

   Q_ASSERT_X(d->priv == rhs.d->priv, "QTextCursor::operator<", "cannot compare cursors attached to different documents");

   return d->position < rhs.d->position;
}

/*!
    \fn bool QTextCursor::operator<=(const QTextCursor &other) const

    Returns true if the \a other cursor is positioned later or at the
    same position in the document as this cursor; otherwise returns
    false.
*/
bool QTextCursor::operator<=(const QTextCursor &rhs) const
{
   if (!d) {
      return true;
   }

   if (!rhs.d) {
      return false;
   }

   Q_ASSERT_X(d->priv == rhs.d->priv, "QTextCursor::operator<=", "cannot compare cursors attached to different documents");

   return d->position <= rhs.d->position;
}

/*!
    \fn bool QTextCursor::operator==(const QTextCursor &other) const

    Returns true if the \a other cursor is at the same position in the
    document as this cursor; otherwise returns false.
*/
bool QTextCursor::operator==(const QTextCursor &rhs) const
{
   if (!d) {
      return !rhs.d;
   }

   if (!rhs.d) {
      return false;
   }

   return d->position == rhs.d->position && d->priv == rhs.d->priv;
}

/*!
    \fn bool QTextCursor::operator>=(const QTextCursor &other) const

    Returns true if the \a other cursor is positioned earlier or at the
    same position in the document as this cursor; otherwise returns
    false.
*/
bool QTextCursor::operator>=(const QTextCursor &rhs) const
{
   if (!d) {
      return false;
   }

   if (!rhs.d) {
      return true;
   }

   Q_ASSERT_X(d->priv == rhs.d->priv, "QTextCursor::operator>=", "cannot compare cursors attached to different documents");

   return d->position >= rhs.d->position;
}

/*!
    \fn bool QTextCursor::operator>(const QTextCursor &other) const

    Returns true if the \a other cursor is positioned earlier in the
    document than this cursor; otherwise returns false.
*/
bool QTextCursor::operator>(const QTextCursor &rhs) const
{
   if (!d) {
      return false;
   }

   if (!rhs.d) {
      return true;
   }

   Q_ASSERT_X(d->priv == rhs.d->priv, "QTextCursor::operator>", "cannot compare cursors attached to different documents");

   return d->position > rhs.d->position;
}

/*!
    Indicates the start of a block of editing operations on the
    document that should appear as a single operation from an
    undo/redo point of view.

    For example:

    \snippet doc/src/snippets/code/src_gui_text_qtextcursor.cpp 2

    The call to undo() will cause both insertions to be undone,
    causing both "World" and "Hello" to be removed.

    It is possible to nest calls to beginEditBlock and endEditBlock. The
    top-most pair will determine the scope of the undo/redo operation.

    \sa endEditBlock()
 */
void QTextCursor::beginEditBlock()
{
   if (!d || !d->priv) {
      return;
   }

   if (d->priv->editBlock == 0) { // we are the initial edit block, store current cursor position for undo
      d->priv->editBlockCursorPosition = d->position;
   }

   d->priv->beginEditBlock();
}

/*!
    Like beginEditBlock() indicates the start of a block of editing operations
    that should appear as a single operation for undo/redo. However unlike
    beginEditBlock() it does not start a new block but reverses the previous call to
    endEditBlock() and therefore makes following operations part of the previous edit block created.

    For example:

    \snippet doc/src/snippets/code/src_gui_text_qtextcursor.cpp 3

    The call to undo() will cause all three insertions to be undone.

    \sa beginEditBlock(), endEditBlock()
 */
void QTextCursor::joinPreviousEditBlock()
{
   if (!d || !d->priv) {
      return;
   }

   d->priv->joinPreviousEditBlock();
}

/*!
    Indicates the end of a block of editing operations on the document
    that should appear as a single operation from an undo/redo point
    of view.

    \sa beginEditBlock()
 */

void QTextCursor::endEditBlock()
{
   if (!d || !d->priv) {
      return;
   }

   d->priv->endEditBlock();
}

/*!
    Returns true if this cursor and \a other are copies of each other, i.e.
    one of them was created as a copy of the other and neither has moved since.
    This is much stricter than equality.

    \sa operator=() operator==()
*/
bool QTextCursor::isCopyOf(const QTextCursor &other) const
{
   return d == other.d;
}

/*!
    \since 4.2
    Returns the number of the block the cursor is in, or 0 if the cursor is invalid.

    Note that this function only makes sense in documents without complex objects such
    as tables or frames.
*/
int QTextCursor::blockNumber() const
{
   if (!d || !d->priv) {
      return 0;
   }

   return d->block().blockNumber();
}


/*!
    \since 4.2
    Returns the position of the cursor within its containing line.

    Note that this is the column number relative to a wrapped line,
    not relative to the block (i.e. the paragraph).

    You probably want to call positionInBlock() instead.

    \sa positionInBlock()
*/
int QTextCursor::columnNumber() const
{
   if (!d || !d->priv) {
      return 0;
   }

   QTextBlock block = d->block();
   if (!block.isValid()) {
      return 0;
   }

   const QTextLayout *layout = d->blockLayout(block);

   const int relativePos = d->position - block.position();

   if (layout->lineCount() == 0) {
      return relativePos;
   }

   QTextLine line = layout->lineForTextPosition(relativePos);
   if (!line.isValid()) {
      return 0;
   }
   return relativePos - line.textStart();
}

/*!
    \since 4.5
    Returns the document this cursor is associated with.
*/
QTextDocument *QTextCursor::document() const
{
   if (d->priv) {
      return d->priv->document();
   }
   return 0; // document went away
}
