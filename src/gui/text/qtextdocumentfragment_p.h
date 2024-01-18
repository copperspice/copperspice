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

#ifndef QTEXTDOCUMENTFRAGMENT_P_H
#define QTEXTDOCUMENTFRAGMENT_P_H

#include <qtextdocument.h>
#include <qtexthtmlparser_p.h>
#include <qtextdocument_p.h>
#include <qtexttable.h>
#include <qatomic.h>
#include <qlist.h>
#include <qmap.h>
#include <qpointer.h>
#include <qvarlengtharray.h>
#include <qdatastream.h>

class QTextDocumentFragmentPrivate;

class QTextCopyHelper
{
 public:
   QTextCopyHelper(const QTextCursor &_source, const QTextCursor &_destination, bool forceCharFormat = false,
      const QTextCharFormat &fmt = QTextCharFormat());

   void copy();

 private:
   void appendFragments(int pos, int endPos);
   int appendFragment(int pos, int endPos, int objectIndex = -1);
   int convertFormatIndex(const QTextFormat &oldFormat, int objectIndexToSet = -1);

   inline int convertFormatIndex(int oldFormatIndex, int objectIndexToSet = -1) {
      return convertFormatIndex(src->formatCollection()->format(oldFormatIndex), objectIndexToSet);
   }

   inline QTextFormat convertFormat(const QTextFormat &fmt) {
      return dst->formatCollection()->format(convertFormatIndex(fmt));
   }

   int insertPos;

   bool forceCharFormat;
   int primaryCharFormatIndex;

   QTextCursor cursor;
   QTextDocumentPrivate *dst;
   QTextDocumentPrivate *src;
   QTextFormatCollection &formatCollection;
   const QString originalText;

   QMap<int, int> objectIndexMap;
};

class QTextDocumentFragmentPrivate
{
 public:
   QTextDocumentFragmentPrivate(const QTextCursor &cursor = QTextCursor());

   QTextDocumentFragmentPrivate(const QTextDocumentFragmentPrivate &) = delete;
   QTextDocumentFragmentPrivate &operator=(const QTextDocumentFragmentPrivate &) = delete;

   ~QTextDocumentFragmentPrivate() {
      delete doc;
   }

   void insert(QTextCursor &cursor) const;

   QAtomicInt ref;
   QTextDocument *doc;

   uint importedFromPlainText : 1;
};

#ifndef QT_NO_TEXTHTMLPARSER

class QTextHtmlImporter : public QTextHtmlParser
{
   struct Table;

 public:
   enum ImportMode {
      ImportToFragment,
      ImportToDocument
   };

   QTextHtmlImporter(QTextDocument *_doc, const QString &html,
      ImportMode mode,
      const QTextDocument *resourceProvider = nullptr);

   void import();

 private:
   bool closeTag();

   Table scanTable(int tableNodeIdx);

   enum ProcessNodeResult { ContinueWithNextNode, ContinueWithCurrentNode, ContinueWithNextSibling };

   void appendBlock(const QTextBlockFormat &format, QTextCharFormat charFmt = QTextCharFormat());
   bool appendNodeText();

   ProcessNodeResult processBlockNode();
   ProcessNodeResult processSpecialNodes();

   struct List {
      inline List() : listNode(0) {}
      QTextListFormat format;
      int listNode;
      QPointer<QTextList> list;
   };

   QVector<List> lists;
   int indent;

   // insert a named anchor the next time we emit a char format,
   // either in a block or in regular text
   QStringList namedAnchors;

   struct TableCellIterator {
      inline TableCellIterator(QTextTable *t = nullptr)
         : table(t), row(0), column(0)
      {
      }

      inline TableCellIterator &operator++() {
         if (atEnd()) {
            return *this;
         }
         do {
            const QTextTableCell cell = table->cellAt(row, column);
            if (!cell.isValid()) {
               break;
            }
            column += cell.columnSpan();
            if (column >= table->columns()) {
               column = 0;
               ++row;
            }
         } while (row < table->rows() && table->cellAt(row, column).row() != row);

         return *this;
      }

      inline bool atEnd() const {
         return table == nullptr || row >= table->rows();
      }

      QTextTableCell cell() const {
         return table->cellAt(row, column);
      }

      QTextTable *table;
      int row;
      int column;
   };

   friend struct Table;
   struct Table {
      Table() : isTextFrame(false), rows(0), columns(0), currentRow(0), lastIndent(0) {}
      QPointer<QTextFrame> frame;
      bool isTextFrame;
      int rows;
      int columns;
      int currentRow; // ... for buggy html (see html_skipCell testcase)
      TableCellIterator currentCell;
      int lastIndent;
   };
   QVector<Table> tables;

   struct RowColSpanInfo {
      int row, col;
      int rowSpan, colSpan;
   };

   enum WhiteSpace {
      RemoveWhiteSpace,
      CollapseWhiteSpace,
      PreserveWhiteSpace
   };

   WhiteSpace compressNextWhitespace;

   QTextDocument *doc;
   QTextCursor cursor;
   QTextHtmlParserNode::WhiteSpaceMode wsm;
   ImportMode importMode;
   bool hasBlock;
   bool forceBlockMerging;
   bool blockTagClosed;
   int currentNodeIdx;
   const QTextHtmlParserNode *currentNode;
};

#endif // QT_NO_TEXTHTMLPARSER

#endif // QTEXTDOCUMENTFRAGMENT_P_H
