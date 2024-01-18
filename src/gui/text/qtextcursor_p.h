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

#ifndef QTEXTCURSOR_P_H
#define QTEXTCURSOR_P_H

#include <qtextcursor.h>
#include <qtextdocument.h>
#include <qtextdocument_p.h>
#include <qtextformat_p.h>
#include <qtextobject.h>

class QTextCursorPrivate : public QSharedData
{
 public:
   QTextCursorPrivate(QTextDocumentPrivate *p);
   QTextCursorPrivate(const QTextCursorPrivate &other);
   ~QTextCursorPrivate();

   enum AdjustResult { CursorMoved, CursorUnchanged };
   AdjustResult adjustPosition(int positionOfChange, int charsAddedOrRemoved, QTextUndoCommand::Operation op);

   void adjustCursor(QTextCursor::MoveOperation m);

   void remove();
   void clearCells(QTextTable *table, int startRow, int startCol, int numRows, int numCols,
      QTextUndoCommand::Operation op);
   inline bool setPosition(int newPosition) {
      Q_ASSERT(newPosition >= 0 && newPosition < priv->length());
      bool moved = position != newPosition;
      if (moved) {
         position = newPosition;
         currentCharFormat = -1;
      }
      return moved;
   }
   void setX();
   bool canDelete(int pos) const;

   void insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat);
   bool movePosition(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

   inline QTextBlock block() const {
      return QTextBlock(priv, priv->blockMap().findNode(position));
   }

   inline QTextBlockFormat blockFormat() const {
      return block().blockFormat();
   }

   QTextLayout *blockLayout(QTextBlock &block) const;

   QTextTable *complexSelectionTable() const;
   void selectedTableCells(int *firstRow, int *numRows, int *firstColumn, int *numColumns) const;

   void setBlockCharFormat(const QTextCharFormat &format, QTextDocumentPrivate::FormatChangeMode changeMode);
   void setBlockFormat(const QTextBlockFormat &format, QTextDocumentPrivate::FormatChangeMode changeMode);
   void setCharFormat(const QTextCharFormat &format, QTextDocumentPrivate::FormatChangeMode changeMode);

   void aboutToRemoveCell(int from, int to);
   static QTextCursor fromPosition(QTextDocumentPrivate *d, int pos) {
      return QTextCursor(d, pos);
   }

   QTextDocumentPrivate *priv;
   qreal x;
   int position;
   int anchor;
   int adjusted_anchor;
   int currentCharFormat;
   uint visualNavigation : 1;
   uint keepPositionOnInsert : 1;
   uint changed : 1;
};

#endif // QTEXTCURSOR_P_H
