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

#ifndef QTEXTCURSOR_H
#define QTEXTCURSOR_H

#include <qstring.h>
#include <qshareddata.h>
#include <qtextformat.h>

class QTextDocument;
class QTextCursorPrivate;
class QTextDocumentFragment;
class QTextCharFormat;
class QTextBlockFormat;
class QTextListFormat;
class QTextTableFormat;
class QTextFrameFormat;
class QTextImageFormat;
class QTextDocumentPrivate;
class QTextList;
class QTextTable;
class QTextFrame;
class QTextBlock;

class Q_GUI_EXPORT QTextCursor
{
 public:
   QTextCursor();

   explicit QTextCursor(QTextDocument *document);
   QTextCursor(QTextDocumentPrivate *p, int pos);
   explicit QTextCursor(QTextFrame *frame);
   explicit QTextCursor(const QTextBlock &block);
   explicit QTextCursor(QTextCursorPrivate *d);
   QTextCursor(const QTextCursor &other);

   ~QTextCursor();

   QTextCursor &operator=(const QTextCursor &other);

   QTextCursor &operator=(QTextCursor &&other) {
      swap(other);
      return *this;
   }

   void swap(QTextCursor &other) {
      qSwap(d, other.d);
   }

   bool isNull() const;

   enum MoveMode {
      MoveAnchor,
      KeepAnchor
   };

   void setPosition(int pos, MoveMode mode = MoveAnchor);
   int position() const;
   int positionInBlock() const;

   int anchor() const;

   void insertText(const QString &text);
   void insertText(const QString &text, const QTextCharFormat &format);

   enum MoveOperation {
      NoMove,

      Start,
      Up,
      StartOfLine,
      StartOfBlock,
      StartOfWord,
      PreviousBlock,
      PreviousCharacter,
      PreviousWord,
      Left,
      WordLeft,

      End,
      Down,
      EndOfLine,
      EndOfWord,
      EndOfBlock,
      NextBlock,
      NextCharacter,
      NextWord,
      Right,
      WordRight,

      NextCell,
      PreviousCell,
      NextRow,
      PreviousRow
   };

   bool movePosition(MoveOperation operation, MoveMode mode = MoveAnchor, int n = 1);

   bool visualNavigation() const;
   void setVisualNavigation(bool b);

   void setVerticalMovementX(int x);
   int verticalMovementX() const;

   void setKeepPositionOnInsert(bool b);
   bool keepPositionOnInsert() const;

   void deleteChar();
   void deletePreviousChar();

   enum SelectionType {
      WordUnderCursor,
      LineUnderCursor,
      BlockUnderCursor,
      Document
   };
   void select(SelectionType selection);

   bool hasSelection() const;
   bool hasComplexSelection() const;
   void removeSelectedText();
   void clearSelection();
   int selectionStart() const;
   int selectionEnd() const;

   QString selectedText() const;
   QTextDocumentFragment selection() const;
   void selectedTableCells(int *firstRow, int *numRows, int *firstColumn, int *numColumns) const;

   QTextBlock block() const;

   QTextCharFormat charFormat() const;
   void setCharFormat(const QTextCharFormat &format);
   void mergeCharFormat(const QTextCharFormat &modifier);

   QTextBlockFormat blockFormat() const;
   void setBlockFormat(const QTextBlockFormat &format);
   void mergeBlockFormat(const QTextBlockFormat &modifier);

   QTextCharFormat blockCharFormat() const;
   void setBlockCharFormat(const QTextCharFormat &format);
   void mergeBlockCharFormat(const QTextCharFormat &modifier);

   bool atBlockStart() const;
   bool atBlockEnd() const;
   bool atStart() const;
   bool atEnd() const;

   void insertBlock();
   void insertBlock(const QTextBlockFormat &format);
   void insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat);

   QTextList *insertList(const QTextListFormat &format);
   QTextList *insertList(QTextListFormat::Style style);

   QTextList *createList(const QTextListFormat &format);
   QTextList *createList(QTextListFormat::Style style);
   QTextList *currentList() const;

   QTextTable *insertTable(int rows, int cols, const QTextTableFormat &format);
   QTextTable *insertTable(int rows, int cols);
   QTextTable *currentTable() const;

   QTextFrame *insertFrame(const QTextFrameFormat &format);
   QTextFrame *currentFrame() const;

   void insertFragment(const QTextDocumentFragment &fragment);

#ifndef QT_NO_TEXTHTMLPARSER
   void insertHtml(const QString &html);
#endif

   void insertImage(const QTextImageFormat &format, QTextFrameFormat::Position alignment);
   void insertImage(const QTextImageFormat &format);
   void insertImage(const QString &name);
   void insertImage(const QImage &image, const QString &name = QString());

   void beginEditBlock();
   void joinPreviousEditBlock();
   void endEditBlock();

   bool operator!=(const QTextCursor &other) const;
   bool operator<(const QTextCursor &other)  const;
   bool operator<=(const QTextCursor &other) const;
   bool operator==(const QTextCursor &other) const;
   bool operator>=(const QTextCursor &other) const;
   bool operator>(const QTextCursor &other)  const;

   bool isCopyOf(const QTextCursor &other) const;

   int blockNumber() const;
   int columnNumber() const;

   QTextDocument *document() const;

 private:
   QSharedDataPointer<QTextCursorPrivate> d;
   friend class QTextCursorPrivate;
   friend class QTextDocumentPrivate;
   friend class QTextDocumentFragmentPrivate;
   friend class QTextCopyHelper;
   friend class QTextControlPrivate;
};

CS_DECLARE_METATYPE(QTextCursor)

#endif
