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

#include <qsyntaxhighlighter.h>

#ifndef QT_NO_SYNTAXHIGHLIGHTER
#include <qtextdocument.h>
#include <qtextlayout.h>
#include <qpointer.h>
#include <qtextobject.h>
#include <qtextcursor.h>
#include <qdebug.h>
#include <qtimer.h>

#include <qtextdocument_p.h>

class QSyntaxHighlighterPrivate
{
   Q_DECLARE_PUBLIC(QSyntaxHighlighter)

 public:
   inline QSyntaxHighlighterPrivate()
      : rehighlightPending(false), inReformatBlocks(false) {
   }

   virtual ~QSyntaxHighlighterPrivate()
   { }

   QPointer<QTextDocument> doc;

   void _q_reformatBlocks(int from, int charsRemoved, int charsAdded);
   void reformatBlocks(int from, int charsRemoved, int charsAdded);
   void reformatBlock(const QTextBlock &block);

   inline void rehighlight(QTextCursor &cursor, QTextCursor::MoveOperation operation) {
      inReformatBlocks = true;
      cursor.beginEditBlock();
      int from = cursor.position();
      cursor.movePosition(operation);
      reformatBlocks(from, 0, cursor.position() - from);
      cursor.endEditBlock();
      inReformatBlocks = false;
   }

   inline void _q_delayedRehighlight() {
      if (! rehighlightPending) {
         return;
      }

      rehighlightPending = false;
      q_func()->rehighlight();
   }

   void applyFormatChanges();
   QVector<QTextCharFormat> formatChanges;
   QTextBlock currentBlock;
   bool rehighlightPending;
   bool inReformatBlocks;

 protected:
   QSyntaxHighlighter *q_ptr;
};

void QSyntaxHighlighterPrivate::applyFormatChanges()
{
   bool formatsChanged = false;

   QTextLayout *layout = currentBlock.layout();
   QVector<QTextLayout::FormatRange> ranges = layout->formats();

   const int preeditAreaStart = layout->preeditAreaPosition();
   const int preeditAreaLength = layout->preeditAreaText().length();

   if (preeditAreaLength != 0) {
      QVector<QTextLayout::FormatRange>::iterator it = ranges.begin();

      while (it != ranges.end()) {
         if (it->start >= preeditAreaStart
            && it->start + it->length <= preeditAreaStart + preeditAreaLength) {
            ++it;
         } else {
            it = ranges.erase(it);
            formatsChanged = true;
         }
      }
   } else if (!ranges.isEmpty()) {
      ranges.clear();
      formatsChanged = true;
   }

   int i = 0;
   while (i < formatChanges.count()) {
      QTextLayout::FormatRange r;

      while (i < formatChanges.count() && formatChanges.at(i) == r.format) {
         ++i;
      }

      if (i == formatChanges.count()) {
         break;
      }

      r.start = i;
      r.format = formatChanges.at(i);

      while (i < formatChanges.count() && formatChanges.at(i) == r.format) {
         ++i;
      }

      Q_ASSERT(i <= formatChanges.count());

      r.length = i - r.start;

      if (preeditAreaLength != 0) {
         if (r.start >= preeditAreaStart) {
            r.start += preeditAreaLength;
         } else if (r.start + r.length >= preeditAreaStart) {
            r.length += preeditAreaLength;
         }
      }

      ranges << r;
      formatsChanged = true;
   }

   if (formatsChanged) {
      layout->setFormats(ranges);
      doc->markContentsDirty(currentBlock.position(), currentBlock.length());
   }
}

void QSyntaxHighlighterPrivate::_q_reformatBlocks(int from, int charsRemoved, int charsAdded)
{
   if (! inReformatBlocks) {
      reformatBlocks(from, charsRemoved, charsAdded);
   }
}

void QSyntaxHighlighterPrivate::reformatBlocks(int from, int charsRemoved, int charsAdded)
{
   rehighlightPending = false;

   QTextBlock block = doc->findBlock(from);
   if (! block.isValid()) {
      return;
   }

   int endPosition;
   QTextBlock lastBlock = doc->findBlock(from + charsAdded + (charsRemoved > 0 ? 1 : 0));

   if (lastBlock.isValid()) {
      endPosition = lastBlock.position() + lastBlock.length();
   } else {
      endPosition = doc->docHandle()->length();
   }

   bool forceHighlightOfNextBlock = false;

   while (block.isValid() && (block.position() < endPosition || forceHighlightOfNextBlock)) {
      const int stateBeforeHighlight = block.userState();

      reformatBlock(block);
      forceHighlightOfNextBlock = (block.userState() != stateBeforeHighlight);

      block = block.next();
   }

   formatChanges.clear();
}

void QSyntaxHighlighterPrivate::reformatBlock(const QTextBlock &block)
{
   Q_Q(QSyntaxHighlighter);
   Q_ASSERT_X(! currentBlock.isValid(), "QSyntaxHighlighter::reformatBlock()", "reFormatBlock() called recursively");

   currentBlock = block;
   formatChanges.fill(QTextCharFormat(), block.length() - 1);

   q->highlightBlock(block.text());
   applyFormatChanges();

   currentBlock = QTextBlock();
}

QSyntaxHighlighter::QSyntaxHighlighter(QObject *parent)
   : QObject(parent), d_ptr(new QSyntaxHighlighterPrivate)
{
   d_ptr->q_ptr = this;

   if (parent->inherits("QTextEdit")) {
      QTextDocument *doc = parent->property("document").value<QTextDocument *>();

      if (doc) {
         setDocument(doc);
      }
   }
}


QSyntaxHighlighter::QSyntaxHighlighter(QTextDocument *parent)
   : QObject(parent), d_ptr(new QSyntaxHighlighterPrivate)
{
   d_ptr->q_ptr = this;
   setDocument(parent);
}

QSyntaxHighlighter::~QSyntaxHighlighter()
{
   setDocument(nullptr);
}

void QSyntaxHighlighter::setDocument(QTextDocument *doc)
{
   Q_D(QSyntaxHighlighter);

   if (d->doc) {
      disconnect(d->doc.data(), &QTextDocument::contentsChange, this, &QSyntaxHighlighter::_q_reformatBlocks);

      QTextCursor cursor(d->doc);
      cursor.beginEditBlock();

      for (QTextBlock blk = d->doc->begin(); blk.isValid(); blk = blk.next()) {
         blk.layout()->clearFormats();
      }

      cursor.endEditBlock();
   }

   d->doc = doc;
   if (d->doc) {
      connect(d->doc.data(), &QTextDocument::contentsChange, this, &QSyntaxHighlighter::_q_reformatBlocks);
      d->rehighlightPending = true;
      QTimer::singleShot(0, this, SLOT(_q_delayedRehighlight()));
   }
}

QTextDocument *QSyntaxHighlighter::document() const
{
   Q_D(const QSyntaxHighlighter);
   return d->doc;
}

void QSyntaxHighlighter::rehighlight()
{
   Q_D(QSyntaxHighlighter);
   if (!d->doc) {
      return;
   }

   QTextCursor cursor(d->doc);
   d->rehighlight(cursor, QTextCursor::End);
}

void QSyntaxHighlighter::rehighlightBlock(const QTextBlock &block)
{
   Q_D(QSyntaxHighlighter);
   if (!d->doc || !block.isValid() || block.document() != d->doc) {
      return;
   }

   const bool rehighlightPending = d->rehighlightPending;

   QTextCursor cursor(block);
   d->rehighlight(cursor, QTextCursor::EndOfBlock);

   if (rehighlightPending) {
      d->rehighlightPending = rehighlightPending;
   }
}


void QSyntaxHighlighter::setFormat(int start, int count, const QTextCharFormat &format)
{
   Q_D(QSyntaxHighlighter);

   if (start < 0 || start >= d->formatChanges.count()) {
      return;
   }

   const int end = qMin(start + count, d->formatChanges.count());
   for (int i = start; i < end; ++i) {
      d->formatChanges[i] = format;
   }
}

void QSyntaxHighlighter::setFormat(int start, int count, const QColor &color)
{
   QTextCharFormat format;
   format.setForeground(color);
   setFormat(start, count, format);
}

void QSyntaxHighlighter::setFormat(int start, int count, const QFont &font)
{
   QTextCharFormat format;
   format.setFont(font);
   setFormat(start, count, format);
}

QTextCharFormat QSyntaxHighlighter::format(int pos) const
{
   Q_D(const QSyntaxHighlighter);
   if (pos < 0 || pos >= d->formatChanges.count()) {
      return QTextCharFormat();
   }
   return d->formatChanges.at(pos);
}

int QSyntaxHighlighter::previousBlockState() const
{
   Q_D(const QSyntaxHighlighter);
   if (!d->currentBlock.isValid()) {
      return -1;
   }

   const QTextBlock previous = d->currentBlock.previous();
   if (!previous.isValid()) {
      return -1;
   }

   return previous.userState();
}

/*!
    Returns the state of the current text block. If no value is set,
    the returned value is -1.
*/
int QSyntaxHighlighter::currentBlockState() const
{
   Q_D(const QSyntaxHighlighter);
   if (!d->currentBlock.isValid()) {
      return -1;
   }

   return d->currentBlock.userState();
}

void QSyntaxHighlighter::setCurrentBlockState(int newState)
{
   Q_D(QSyntaxHighlighter);
   if (!d->currentBlock.isValid()) {
      return;
   }

   d->currentBlock.setUserState(newState);
}

void QSyntaxHighlighter::setCurrentBlockUserData(QTextBlockUserData *data)
{
   Q_D(QSyntaxHighlighter);
   if (!d->currentBlock.isValid()) {
      return;
   }

   d->currentBlock.setUserData(data);
}

QTextBlockUserData *QSyntaxHighlighter::currentBlockUserData() const
{
   Q_D(const QSyntaxHighlighter);
   if (!d->currentBlock.isValid()) {
      return nullptr;
   }

   return d->currentBlock.userData();
}

QTextBlock QSyntaxHighlighter::currentBlock() const
{
   Q_D(const QSyntaxHighlighter);
   return d->currentBlock;
}

void QSyntaxHighlighter::_q_reformatBlocks(int from, int charsRemoved, int charsAdded)
{
   Q_D(QSyntaxHighlighter);
   d->_q_reformatBlocks(from, charsRemoved, charsAdded);
}

void QSyntaxHighlighter::_q_delayedRehighlight()
{
   Q_D(QSyntaxHighlighter);
   d->_q_delayedRehighlight();
}

#endif // QT_NO_SYNTAXHIGHLIGHTER
