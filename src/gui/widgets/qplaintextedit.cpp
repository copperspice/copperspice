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

#include <qplaintextedit_p.h>

#include <qaccessible.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qdrag.h>
#include <qevent.h>
#include <qfont.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qregularexpression.h>
#include <qstyle.h>
#include <qtextformat.h>
#include <qtextlist.h>
#include <qtexttable.h>
#include <qtimer.h>
#include <qvariant.h>

#include <qtextdocumentlayout_p.h>
#include <qabstracttextdocumentlayout_p.h>
#include <qtextdocument.h>
#include <qtextdocument_p.h>

#include <limits.h>

#ifndef QT_NO_TEXTEDIT

static inline bool shouldEnableInputMethod(QPlainTextEdit *plaintextedit)
{
   return !plaintextedit->isReadOnly();
}

class QPlainTextDocumentLayoutPrivate : public QAbstractTextDocumentLayoutPrivate
{
   Q_DECLARE_PUBLIC(QPlainTextDocumentLayout)

 public:
   QPlainTextDocumentLayoutPrivate() {
      mainViewPrivate = nullptr;
      width = 0;
      maximumWidth = 0;
      maximumWidthBlockNumber = 0;
      blockCount = 1;
      blockUpdate = blockDocumentSizeChanged = false;
      cursorWidth = 1;
      textLayoutFlags = 0;
   }

   qreal width;
   qreal maximumWidth;
   int maximumWidthBlockNumber;
   int blockCount;
   QPlainTextEditPrivate *mainViewPrivate;
   bool blockUpdate;
   bool blockDocumentSizeChanged;
   int cursorWidth;
   int textLayoutFlags;

   void layoutBlock(const QTextBlock &block);
   qreal blockWidth(const QTextBlock &block);

   void relayout();
};

QPlainTextDocumentLayout::QPlainTextDocumentLayout(QTextDocument *document)
   : QAbstractTextDocumentLayout(* new QPlainTextDocumentLayoutPrivate, document)
{
}

QPlainTextDocumentLayout::~QPlainTextDocumentLayout()
{
}

void QPlainTextDocumentLayout::draw(QPainter *, const PaintContext &)
{
}

int QPlainTextDocumentLayout::hitTest(const QPointF &, Qt::HitTestAccuracy ) const
{
   //  used from QAbstractTextDocumentLayout::anchorAt()
   //  is not implementable in a plain text document layout, because the
   //  layout depends on the top block and top line which depends on the view
   return -1;
}

int QPlainTextDocumentLayout::pageCount() const
{
   return 1;
}

QSizeF QPlainTextDocumentLayout::documentSize() const
{
   Q_D(const QPlainTextDocumentLayout);
   return QSizeF(d->maximumWidth, document()->lineCount());
}

QRectF QPlainTextDocumentLayout::frameBoundingRect(QTextFrame *) const
{
   Q_D(const QPlainTextDocumentLayout);
   return QRectF(0, 0, qMax(d->width, d->maximumWidth), qreal(INT_MAX));
}

QRectF QPlainTextDocumentLayout::blockBoundingRect(const QTextBlock &block) const
{
   if (! block.isValid()) {
      return QRectF();
   }

   QTextLayout *tl = block.layout();
   if (!tl->lineCount()) {
      const_cast<QPlainTextDocumentLayout *>(this)->layoutBlock(block);
   }

   QRectF br;
   if (block.isVisible()) {
      br = QRectF(QPointF(0, 0), tl->boundingRect().bottomRight());
      if (tl->lineCount() == 1) {
         br.setWidth(qMax(br.width(), tl->lineAt(0).naturalTextWidth()));
      }
      qreal margin = document()->documentMargin();
      br.adjust(0, 0, margin, 0);
      if (!block.next().isValid()) {
         br.adjust(0, 0, 0, margin);
      }
   }

   return br;
}

void QPlainTextDocumentLayout::ensureBlockLayout(const QTextBlock &block) const
{
   if (!block.isValid()) {
      return;
   }
   QTextLayout *tl = block.layout();
   if (!tl->lineCount()) {
      const_cast<QPlainTextDocumentLayout *>(this)->layoutBlock(block);
   }
}

void QPlainTextDocumentLayout::setCursorWidth(int width)
{
   Q_D(QPlainTextDocumentLayout);
   d->cursorWidth = width;
}

int QPlainTextDocumentLayout::cursorWidth() const
{
   Q_D(const QPlainTextDocumentLayout);
   return d->cursorWidth;
}

QPlainTextDocumentLayoutPrivate *QPlainTextDocumentLayout::priv() const
{
   Q_D(const QPlainTextDocumentLayout);
   return const_cast<QPlainTextDocumentLayoutPrivate *>(d);
}

void QPlainTextDocumentLayout::requestUpdate()
{
   emit update(QRectF(0., -document()->documentMargin(), 1000000000., 1000000000.));
}

void QPlainTextDocumentLayout::setTextWidth(qreal newWidth)
{
   Q_D(QPlainTextDocumentLayout);
   d->width = d->maximumWidth = newWidth;
   d->relayout();
}

qreal QPlainTextDocumentLayout::textWidth() const
{
   Q_D(const QPlainTextDocumentLayout);
   return d->width;
}

void QPlainTextDocumentLayoutPrivate::relayout()
{
   Q_Q(QPlainTextDocumentLayout);

   QTextBlock block = q->document()->firstBlock();
   while (block.isValid()) {
      block.layout()->clearLayout();
      block.setLineCount(block.isVisible() ? 1 : 0);
      block = block.next();
   }

   emit q->update();
}

void QPlainTextDocumentLayout::documentChanged(int from, int charsRemoved, int charsAdded)
{
   Q_D(QPlainTextDocumentLayout);

   QTextDocument *doc = document();
   int newBlockCount = doc->blockCount();
   int charsChanged = charsRemoved + charsAdded;

   QTextBlock changeStartBlock = doc->findBlock(from);
   QTextBlock changeEndBlock = doc->findBlock(qMax(0, from + charsChanged - 1));

   if (changeStartBlock == changeEndBlock && newBlockCount == d->blockCount) {
      QTextBlock block = changeStartBlock;

      if (block.isValid() && block.length()) {
         QRectF oldBr = blockBoundingRect(block);
         layoutBlock(block);

         QRectF newBr = blockBoundingRect(block);
         if (newBr.height() == oldBr.height()) {
            if (!d->blockUpdate) {
               emit updateBlock(block);
            }
            return;
         }
      }
   } else {
      QTextBlock block = changeStartBlock;

      do {
         block.clearLayout();
         if (block == changeEndBlock) {
            break;
         }
         block = block.next();

      } while (block.isValid());
   }

   if (newBlockCount != d->blockCount) {

      int changeEnd = changeEndBlock.blockNumber();
      int blockDiff = newBlockCount - d->blockCount;
      int oldChangeEnd = changeEnd - blockDiff;

      if (d->maximumWidthBlockNumber > oldChangeEnd) {
         d->maximumWidthBlockNumber += blockDiff;
      }

      d->blockCount = newBlockCount;
      if (d->blockCount == 1) {
         d->maximumWidth = blockWidth(doc->firstBlock());
      }

      if (!d->blockDocumentSizeChanged) {
         emit documentSizeChanged(documentSize());
      }

      if (blockDiff == 1 && changeEnd == newBlockCount - 1 ) {
         if (!d->blockUpdate) {
            QTextBlock b = changeStartBlock;
            for (;;) {
               emit updateBlock(b);
               if (b == changeEndBlock) {
                  break;
               }
               b = b.next();
            }
         }
         return;
      }
   }

   if (!d->blockUpdate) {
      emit update(QRectF(0., -doc->documentMargin(), 1000000000., 1000000000.));   // optimization potential
   }
}

void QPlainTextDocumentLayout::layoutBlock(const QTextBlock &block)
{
   Q_D(QPlainTextDocumentLayout);
   QTextDocument *doc = document();
   qreal margin = doc->documentMargin();
   qreal blockMaximumWidth = 0;

   qreal height = 0;
   QTextLayout *tl = block.layout();
   QTextOption option = doc->defaultTextOption();
   tl->setTextOption(option);

   int extraMargin = 0;
   if (option.flags() & QTextOption::AddSpaceForLineAndParagraphSeparators) {
      QFontMetrics fm(block.charFormat().font());
      extraMargin += fm.width(QChar(0x21B5));
   }
   tl->beginLayout();
   qreal availableWidth = d->width;
   if (availableWidth <= 0) {
      availableWidth = qreal(INT_MAX); // similar to text edit with pageSize.width == 0
   }
   availableWidth -= 2 * margin + extraMargin;

   while (true) {
      QTextLine line = tl->createLine();
      if (!line.isValid()) {
         break;
      }

      line.setLeadingIncluded(true);
      line.setLineWidth(availableWidth);
      line.setPosition(QPointF(margin, height));
      height += line.height();
      blockMaximumWidth = qMax(blockMaximumWidth, line.naturalTextWidth() + 2 * margin);
   }
   tl->endLayout();

   int previousLineCount = doc->lineCount();
   const_cast<QTextBlock &>(block).setLineCount(block.isVisible() ? tl->lineCount() : 0);
   int lineCount = doc->lineCount();

   bool emitDocumentSizeChanged = previousLineCount != lineCount;
   if (blockMaximumWidth > d->maximumWidth) {
      // new longest line
      d->maximumWidth = blockMaximumWidth;
      d->maximumWidthBlockNumber = block.blockNumber();
      emitDocumentSizeChanged = true;
   } else if (block.blockNumber() == d->maximumWidthBlockNumber && blockMaximumWidth < d->maximumWidth) {
      // longest line shrinking
      QTextBlock b = doc->firstBlock();
      d->maximumWidth = 0;
      QTextBlock maximumBlock;
      while (b.isValid()) {
         qreal blockMaximumWidth = blockWidth(b);
         if (blockMaximumWidth > d->maximumWidth) {
            d->maximumWidth = blockMaximumWidth;
            maximumBlock = b;
         }
         b = b.next();
      }
      if (maximumBlock.isValid()) {
         d->maximumWidthBlockNumber = maximumBlock.blockNumber();
         emitDocumentSizeChanged = true;
      }
   }
   if (emitDocumentSizeChanged && !d->blockDocumentSizeChanged) {
      emit documentSizeChanged(documentSize());
   }
}

qreal QPlainTextDocumentLayout::blockWidth(const QTextBlock &block)
{
   QTextLayout *layout = block.layout();
   if (!layout->lineCount()) {
      return 0;   // only for layouted blocks
   }
   qreal blockWidth = 0;
   for (int i = 0; i < layout->lineCount(); ++i) {
      QTextLine line = layout->lineAt(i);
      blockWidth = qMax(line.naturalTextWidth() + 8, blockWidth);
   }
   return blockWidth;
}

QPlainTextEditControl::QPlainTextEditControl(QPlainTextEdit *parent)
   : QTextControl(parent), textEdit(parent), topBlock(0)
{
   setAcceptRichText(false);
}

void QPlainTextEditPrivate::_q_cursorPositionChanged()
{
   pageUpDownLastCursorYIsValid = false;

   Q_Q(QPlainTextEdit);

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleTextCursorEvent ev(q, q->textCursor().position());
   QAccessible::updateAccessibility(&ev);
#endif

   emit q->cursorPositionChanged();
}

void QPlainTextEditPrivate::_q_verticalScrollbarActionTriggered(int action)
{
   if (action == QAbstractSlider::SliderPageStepAdd) {
      pageUpDown(QTextCursor::Down, QTextCursor::MoveAnchor, false);
   } else if (action == QAbstractSlider::SliderPageStepSub) {
      pageUpDown(QTextCursor::Up, QTextCursor::MoveAnchor, false);
   }
}

QMimeData *QPlainTextEditControl::createMimeDataFromSelection() const
{
   QPlainTextEdit *ed = qobject_cast<QPlainTextEdit *>(parent());
   if (!ed) {
      return QTextControl::createMimeDataFromSelection();
   }
   return ed->createMimeDataFromSelection();
}

bool QPlainTextEditControl::canInsertFromMimeData(const QMimeData *source) const
{
   QPlainTextEdit *ed = qobject_cast<QPlainTextEdit *>(parent());

   if (!ed) {
      return QTextControl::canInsertFromMimeData(source);
   }
   return ed->canInsertFromMimeData(source);
}

void QPlainTextEditControl::insertFromMimeData(const QMimeData *source)
{
   QPlainTextEdit *ed = qobject_cast<QPlainTextEdit *>(parent());

   if (!ed) {
      QTextControl::insertFromMimeData(source);
   } else {
      ed->insertFromMimeData(source);
   }
}

qreal QPlainTextEditPrivate::verticalOffset(int topBlock, int topLine) const
{
   qreal offset = 0;
   QTextDocument *doc = control->document();

   if (topLine) {
      QTextBlock currentBlock = doc->findBlockByNumber(topBlock);
      QPlainTextDocumentLayout *documentLayout = qobject_cast<QPlainTextDocumentLayout *>(doc->documentLayout());
      Q_ASSERT(documentLayout);

      // emerald - investigate if there is a side effect
      QRectF r = documentLayout->blockBoundingRect(currentBlock);
      (void) r;

      QTextLayout *layout = currentBlock.layout();

      if (layout && topLine <= layout->lineCount()) {
         QTextLine line = layout->lineAt(topLine - 1);
         const QRectF lr = line.naturalTextRect();
         offset = lr.bottom();
      }
   }

   if (topBlock == 0 && topLine == 0) {
      offset -= doc->documentMargin();   // top margin
   }

   return offset;
}

qreal QPlainTextEditPrivate::verticalOffset() const
{
   return verticalOffset(control->topBlock, topLine) + topLineFracture;
}

QTextBlock QPlainTextEditControl::firstVisibleBlock() const
{
   return document()->findBlockByNumber(topBlock);
}

int QPlainTextEditControl::hitTest(const QPointF &point, Qt::HitTestAccuracy ) const
{
   int currentBlockNumber = topBlock;
   QTextBlock currentBlock = document()->findBlockByNumber(currentBlockNumber);

   if (!currentBlock.isValid()) {
      return -1;
   }

   QPlainTextDocumentLayout *documentLayout = qobject_cast<QPlainTextDocumentLayout *>(document()->documentLayout());
   Q_ASSERT(documentLayout);

   QPointF offset;
   QRectF r = documentLayout->blockBoundingRect(currentBlock);
   while (currentBlock.next().isValid() && r.bottom() + offset.y() <= point.y()) {
      offset.ry() += r.height();
      currentBlock = currentBlock.next();
      ++currentBlockNumber;
      r = documentLayout->blockBoundingRect(currentBlock);
   }
   while (currentBlock.previous().isValid() && r.top() + offset.y() > point.y()) {
      offset.ry() -= r.height();
      currentBlock = currentBlock.previous();
      --currentBlockNumber;
      r = documentLayout->blockBoundingRect(currentBlock);
   }

   if (!currentBlock.isValid()) {
      return -1;
   }

   QTextLayout *layout = currentBlock.layout();
   int off = 0;
   QPointF pos = point - offset;

   for (int i = 0; i < layout->lineCount(); ++i) {
      QTextLine line = layout->lineAt(i);
      const QRectF lr = line.naturalTextRect();
      if (lr.top() > pos.y()) {
         off = qMin(off, line.textStart());
      } else if (lr.bottom() <= pos.y()) {
         off = qMax(off, line.textStart() + line.textLength());
      } else {
         off = line.xToCursor(pos.x(), overwriteMode() ?
               QTextLine::CursorOnCharacter : QTextLine::CursorBetweenCharacters);
         break;
      }
   }

   return currentBlock.position() + off;
}

QRectF QPlainTextEditControl::blockBoundingRect(const QTextBlock &block) const
{
   int currentBlockNumber = topBlock;
   int blockNumber = block.blockNumber();
   QTextBlock currentBlock = document()->findBlockByNumber(currentBlockNumber);

   if (!currentBlock.isValid()) {
      return QRectF();
   }

   Q_ASSERT(currentBlock.blockNumber() == currentBlockNumber);
   QTextDocument *doc = document();
   QPlainTextDocumentLayout *documentLayout = qobject_cast<QPlainTextDocumentLayout *>(doc->documentLayout());
   Q_ASSERT(documentLayout);

   QPointF offset;
   if (!block.isValid()) {
      return QRectF();
   }
   QRectF r = documentLayout->blockBoundingRect(currentBlock);
   int maxVerticalOffset = r.height();
   while (currentBlockNumber < blockNumber && offset.y() - maxVerticalOffset <= 2 * textEdit->viewport()->height()) {
      offset.ry() += r.height();
      currentBlock = currentBlock.next();
      ++currentBlockNumber;
      if (!currentBlock.isVisible()) {
         currentBlock = doc->findBlockByLineNumber(currentBlock.firstLineNumber());
         currentBlockNumber = currentBlock.blockNumber();
      }
      r = documentLayout->blockBoundingRect(currentBlock);
   }
   while (currentBlockNumber > blockNumber && offset.y() + maxVerticalOffset >= -textEdit->viewport()->height()) {
      currentBlock = currentBlock.previous();
      --currentBlockNumber;
      while (!currentBlock.isVisible()) {
         currentBlock = currentBlock.previous();
         --currentBlockNumber;
      }
      if (!currentBlock.isValid()) {
         break;
      }

      r = documentLayout->blockBoundingRect(currentBlock);
      offset.ry() -= r.height();
   }

   if (currentBlockNumber != blockNumber) {
      // fallback for blocks out of reach. Give it some geometry at
      // least, and ensure the layout is up to date.
      r = documentLayout->blockBoundingRect(block);
      if (currentBlockNumber > blockNumber) {
         offset.ry() -= r.height();
      }
   }
   r.translate(offset);
   return r;
}

QString QPlainTextEditControl::anchorAt(const QPointF &pos) const
{
   return textEdit->anchorAt(pos.toPoint());
}

void QPlainTextEditPrivate::setTopLine(int visualTopLine, int dx)
{
   QTextDocument *doc = control->document();
   QTextBlock block = doc->findBlockByLineNumber(visualTopLine);
   int blockNumber = block.blockNumber();
   int lineNumber = visualTopLine - block.firstLineNumber();
   setTopBlock(blockNumber, lineNumber, dx);
}

void QPlainTextEditPrivate::setTopBlock(int blockNumber, int lineNumber, int dx)
{
   Q_Q(QPlainTextEdit);
   blockNumber = qMax(0, blockNumber);
   lineNumber = qMax(0, lineNumber);
   QTextDocument *doc = control->document();
   QTextBlock block = doc->findBlockByNumber(blockNumber);

   int newTopLine = block.firstLineNumber() + lineNumber;
   int maxTopLine = vbar->maximum();

   if (newTopLine > maxTopLine) {
      block = doc->findBlockByLineNumber(maxTopLine);
      blockNumber = block.blockNumber();
      lineNumber = maxTopLine - block.firstLineNumber();
   }

   bool vbarSignalsBlocked = vbar->blockSignals(true);
   vbar->setValue(newTopLine);
   vbar->blockSignals(vbarSignalsBlocked);

   if (! dx && blockNumber == control->topBlock && lineNumber == topLine) {
      return;
   }

   if (viewport->updatesEnabled() && viewport->isVisible()) {
      int dy = 0;
      if (doc->findBlockByNumber(control->topBlock).isValid()) {
         qreal realdy = -q->blockBoundingGeometry(block).y()
            + verticalOffset() - verticalOffset(blockNumber, lineNumber);
         dy = (int)realdy;
         topLineFracture = realdy - dy;
      }
      control->topBlock = blockNumber;
      topLine = lineNumber;

      bool vbarSignalsBlocked = vbar->blockSignals(true);
      vbar->setValue(block.firstLineNumber() + lineNumber);
      vbar->blockSignals(vbarSignalsBlocked);

      if (dx || dy) {
         viewport->scroll(q->isRightToLeft() ? -dx : dx, dy);
      } else {
         viewport->update();
         topLineFracture = 0;
      }
      emit q->updateRequest(viewport->rect(), dy);

   } else {
      control->topBlock = blockNumber;
      topLine = lineNumber;
      topLineFracture = 0;
   }
}

void QPlainTextEditPrivate::ensureVisible(int position, bool center, bool forceCenter)
{
   Q_Q(QPlainTextEdit);
   QRectF visible = QRectF(viewport->rect()).translated(-q->contentOffset());
   QTextBlock block = control->document()->findBlock(position);

   if (!block.isValid()) {
      return;
   }

   QRectF br = control->blockBoundingRect(block);
   if (!br.isValid()) {
      return;
   }

   QTextLine line = block.layout()->lineForTextPosition(position - block.position());
   Q_ASSERT(line.isValid());
   QRectF lr = line.naturalTextRect().translated(br.topLeft());

   if (lr.bottom() >= visible.bottom() || (center && lr.top() < visible.top()) || forceCenter) {

      qreal height = visible.height();
      if (center) {
         height /= 2;
      }

      qreal h = center ? line.naturalTextRect().center().y() : line.naturalTextRect().bottom();

      QTextBlock previousVisibleBlock = block;
      while (h < height && block.previous().isValid()) {
         previousVisibleBlock = block;
         do {
            block = block.previous();
         } while (!block.isVisible() && block.previous().isValid());
         h += q->blockBoundingRect(block).height();
      }

      int l = 0;
      int lineCount = block.layout()->lineCount();
      qreal voffset = verticalOffset(block.blockNumber(), 0);
      while (l < lineCount) {
         QRectF lineRect = block.layout()->lineAt(l).naturalTextRect();
         if (h - voffset - lineRect.top() <= height) {
            break;
         }
         ++l;
      }

      if (l >= lineCount) {
         block = previousVisibleBlock;
         l = 0;
      }
      setTopBlock(block.blockNumber(), l);
   } else if (lr.top() < visible.top()) {
      setTopBlock(block.blockNumber(), line.lineNumber());
   }
}

void QPlainTextEditPrivate::updateViewport()
{
   Q_Q(QPlainTextEdit);
   viewport->update();
   emit q->updateRequest(viewport->rect(), 0);
}

QPlainTextEditPrivate::QPlainTextEditPrivate()
   : control(nullptr), tabChangesFocus(false), lineWrap(QPlainTextEdit::WidgetWidth),
     wordWrap(QTextOption::WrapAtWordBoundaryOrAnywhere), clickCausedFocus(0), topLine(0), topLineFracture(0),
     pageUpDownLastCursorYIsValid(false)
{
   showCursorOnInitialShow = true;
   backgroundVisible       = false;
   centerOnScroll          = false;

   inDrag = false;
}

void QPlainTextEditPrivate::init(const QString &txt)
{
   Q_Q(QPlainTextEdit);
   control = new QPlainTextEditControl(q);

   QTextDocument *doc = new QTextDocument(control);
   QAbstractTextDocumentLayout *layout = new QPlainTextDocumentLayout(doc);
   doc->setDocumentLayout(layout);
   control->setDocument(doc);

   control->setPalette(q->palette());

   QObject::connect(vbar,    &QScrollBar::actionTriggered,  q, &QPlainTextEdit::_q_verticalScrollbarActionTriggered);

   QObject::connect(control, &QPlainTextEditControl::microFocusChanged,     q, &QPlainTextEdit::updateMicroFocus);
   QObject::connect(control, &QPlainTextEditControl::documentSizeChanged,   q, &QPlainTextEdit::_q_adjustScrollbars);
   QObject::connect(control, &QPlainTextEditControl::blockCountChanged,     q, &QPlainTextEdit::blockCountChanged);
   QObject::connect(control, &QPlainTextEditControl::updateRequest,         q, &QPlainTextEdit::_q_repaintContents);
   QObject::connect(control, &QPlainTextEditControl::modificationChanged,   q, &QPlainTextEdit::modificationChanged);

   QObject::connect(control, &QPlainTextEditControl::textChanged,           q, &QPlainTextEdit::textChanged);
   QObject::connect(control, &QPlainTextEditControl::textChanged,           q, &QPlainTextEdit::updateMicroFocus);

   QObject::connect(control, &QPlainTextEditControl::undoAvailable,         q, &QPlainTextEdit::undoAvailable);
   QObject::connect(control, &QPlainTextEditControl::redoAvailable,         q, &QPlainTextEdit::redoAvailable);
   QObject::connect(control, &QPlainTextEditControl::copyAvailable,         q, &QPlainTextEdit::copyAvailable);
   QObject::connect(control, &QPlainTextEditControl::selectionChanged,      q, &QPlainTextEdit::selectionChanged);
   QObject::connect(control, &QPlainTextEditControl::cursorPositionChanged, q, &QPlainTextEdit::_q_cursorPositionChanged);

   // set a null page size initially to avoid any relayouting until the textedit is shown
   // relayoutDocument() will take care of setting the page size to the viewport dimensions later.

   doc->setTextWidth(-1);
   doc->documentLayout()->setPaintDevice(viewport);
   doc->setDefaultFont(q->font());

   if (!txt.isEmpty()) {
      control->setPlainText(txt);
   }

   hbar->setSingleStep(20);
   vbar->setSingleStep(1);

   viewport->setBackgroundRole(QPalette::Base);
   q->setAcceptDrops(true);
   q->setFocusPolicy(Qt::StrongFocus);
   q->setAttribute(Qt::WA_KeyCompression);
   q->setAttribute(Qt::WA_InputMethodEnabled);
   q->setInputMethodHints(Qt::ImhMultiLine);

#ifndef QT_NO_CURSOR
   viewport->setCursor(Qt::IBeamCursor);
#endif

   originalOffsetY = 0;
}

void QPlainTextEditPrivate::_q_repaintContents(const QRectF &contentsRect)
{
   Q_Q(QPlainTextEdit);
   if (!contentsRect.isValid()) {
      updateViewport();
      return;
   }
   const int xOffset = horizontalOffset();
   const int yOffset = (int)verticalOffset();
   const QRect visibleRect(xOffset, yOffset, viewport->width(), viewport->height());

   QRect r = contentsRect.adjusted(-1, -1, 1, 1).intersected(visibleRect).toAlignedRect();
   if (r.isEmpty()) {
      return;
   }

   r.translate(-xOffset, -yOffset);
   viewport->update(r);
   emit q->updateRequest(r, 0);
}

void QPlainTextEditPrivate::pageUpDown(QTextCursor::MoveOperation op, QTextCursor::MoveMode moveMode, bool moveCursor)
{
   Q_Q(QPlainTextEdit);

   QTextCursor cursor = control->textCursor();
   if (moveCursor) {
      ensureCursorVisible();
      if (!pageUpDownLastCursorYIsValid) {
         pageUpDownLastCursorY = control->cursorRect(cursor).top() - verticalOffset();
      }
   }

   qreal lastY = pageUpDownLastCursorY;

   if (op == QTextCursor::Down) {
      QRectF visible = QRectF(viewport->rect()).translated(-q->contentOffset());
      QTextBlock firstVisibleBlock = q->firstVisibleBlock();
      QTextBlock block = firstVisibleBlock;
      QRectF br = q->blockBoundingRect(block);
      qreal h = 0;
      int atEnd = false;

      while (h + br.height() <= visible.bottom()) {
         if (!block.next().isValid()) {
            atEnd = true;
            lastY = visible.bottom(); // set cursor to last line
            break;
         }
         h += br.height();
         block = block.next();
         br = q->blockBoundingRect(block);
      }

      if (!atEnd) {
         int line = 0;
         qreal diff = visible.bottom() - h;
         int lineCount = block.layout()->lineCount();
         while (line < lineCount - 1) {
            if (block.layout()->lineAt(line).naturalTextRect().bottom() > diff) {
               // the first line that did not completely fit the screen
               break;
            }
            ++line;
         }
         setTopBlock(block.blockNumber(), line);
      }

      if (moveCursor) {
         // move using movePosition to keep the cursor's x
         lastY += verticalOffset();
         bool moved = false;
         do {
            moved = cursor.movePosition(op, moveMode);
         } while (moved && control->cursorRect(cursor).top() < lastY);
      }

   } else if (op == QTextCursor::Up) {

      QRectF visible = QRectF(viewport->rect()).translated(-q->contentOffset());
      visible.translate(0, -visible.height()); // previous page
      QTextBlock block = q->firstVisibleBlock();
      qreal h = 0;
      while (h >= visible.top()) {
         if (!block.previous().isValid()) {
            if (control->topBlock == 0 && topLine == 0) {
               lastY = 0; // set cursor to first line
            }
            break;
         }
         block = block.previous();
         QRectF br = q->blockBoundingRect(block);
         h -= br.height();
      }

      int line = 0;
      if (block.isValid()) {
         qreal diff = visible.top() - h;
         int lineCount = block.layout()->lineCount();
         while (line < lineCount) {
            if (block.layout()->lineAt(line).naturalTextRect().top() >= diff) {
               break;
            }
            ++line;
         }
         if (line == lineCount) {
            if (block.next().isValid() && block.next() != q->firstVisibleBlock()) {
               block = block.next();
               line = 0;
            } else {
               --line;
            }
         }
      }
      setTopBlock(block.blockNumber(), line);

      if (moveCursor) {
         cursor.setVisualNavigation(true);
         // move using movePosition to keep the cursor's x
         lastY += verticalOffset();
         bool moved = false;
         do {
            moved = cursor.movePosition(op, moveMode);
         } while (moved && control->cursorRect(cursor).top() > lastY);
      }
   }

   if (moveCursor) {
      control->setTextCursor(cursor);
      pageUpDownLastCursorYIsValid = true;
   }
}

#ifndef QT_NO_SCROLLBAR

void QPlainTextEditPrivate::_q_adjustScrollbars()
{
   Q_Q(QPlainTextEdit);
   QTextDocument *doc = control->document();
   QPlainTextDocumentLayout *documentLayout = qobject_cast<QPlainTextDocumentLayout *>(doc->documentLayout());
   Q_ASSERT(documentLayout);
   bool documentSizeChangedBlocked = documentLayout->priv()->blockDocumentSizeChanged;
   documentLayout->priv()->blockDocumentSizeChanged = true;
   qreal margin = doc->documentMargin();

   int vmax = 0;

   int vSliderLength = 0;
   if (!centerOnScroll && q->isVisible()) {
      QTextBlock block = doc->lastBlock();
      const qreal visible = viewport->rect().height() - margin - 1;
      qreal y = 0;
      int visibleFromBottom = 0;

      while (block.isValid()) {
         if (!block.isVisible()) {
            block = block.previous();
            continue;
         }
         y += documentLayout->blockBoundingRect(block).height();

         QTextLayout *layout = block.layout();
         int layoutLineCount = layout->lineCount();
         if (y > visible) {
            int lineNumber = 0;
            while (lineNumber < layoutLineCount) {
               QTextLine line = layout->lineAt(lineNumber);
               const QRectF lr = line.naturalTextRect();
               if (lr.top() >= y - visible) {
                  break;
               }
               ++lineNumber;
            }
            if (lineNumber < layoutLineCount) {
               visibleFromBottom += (layoutLineCount - lineNumber);
            }
            break;

         }
         visibleFromBottom += layoutLineCount;
         block = block.previous();
      }
      vmax = qMax(0, doc->lineCount() - visibleFromBottom);
      vSliderLength = visibleFromBottom;

   } else {
      vmax = qMax(0, doc->lineCount() - 1);
      int lineSpacing = q->fontMetrics().lineSpacing();
      vSliderLength = lineSpacing != 0 ? viewport->height() / lineSpacing : 0;
   }

   QSizeF documentSize = documentLayout->documentSize();
   vbar->setRange(0, qMax(0, vmax));
   vbar->setPageStep(vSliderLength);
   int visualTopLine = vmax;

   QTextBlock firstVisibleBlock = q->firstVisibleBlock();
   if (firstVisibleBlock.isValid()) {
      visualTopLine = firstVisibleBlock.firstLineNumber() + topLine;
   }

   bool vbarSignalsBlocked = vbar->blockSignals(true);
   vbar->setValue(visualTopLine);
   vbar->blockSignals(vbarSignalsBlocked);

   hbar->setRange(0, (int)documentSize.width() - viewport->width());
   hbar->setPageStep(viewport->width());
   documentLayout->priv()->blockDocumentSizeChanged = documentSizeChangedBlocked;
   setTopLine(vbar->value());
}
#endif

void QPlainTextEditPrivate::ensureViewportLayouted()
{
}

QPlainTextEdit::QPlainTextEdit(QWidget *parent)
   : QAbstractScrollArea(*new QPlainTextEditPrivate, parent)
{
   Q_D(QPlainTextEdit);
   d->init();
}

QPlainTextEdit::QPlainTextEdit(QPlainTextEditPrivate &dd, QWidget *parent)
   : QAbstractScrollArea(dd, parent)
{
   Q_D(QPlainTextEdit);
   d->init();
}


QPlainTextEdit::QPlainTextEdit(const QString &text, QWidget *parent)
   : QAbstractScrollArea(*new QPlainTextEditPrivate, parent)
{
   Q_D(QPlainTextEdit);
   d->init(text);
}

QPlainTextEdit::~QPlainTextEdit()
{
   Q_D(QPlainTextEdit);

   if (d->documentLayoutPtr) {
      if (d->documentLayoutPtr->priv()->mainViewPrivate == d) {
         d->documentLayoutPtr->priv()->mainViewPrivate = nullptr;
      }
   }
}

void QPlainTextEdit::setDocument(QTextDocument *document)
{
   Q_D(QPlainTextEdit);
   QPlainTextDocumentLayout *documentLayout = nullptr;

   if (!document) {
      document = new QTextDocument(d->control);
      documentLayout = new QPlainTextDocumentLayout(document);
      document->setDocumentLayout(documentLayout);

   } else {
      documentLayout = qobject_cast<QPlainTextDocumentLayout *>(document->documentLayout());
      if (!documentLayout) {
         qWarning("QPlainTextEdit::setDocument() New document does not support QPlainTextDocumentLayout");
         return;
      }
   }

   d->control->setDocument(document);
   if (!documentLayout->priv()->mainViewPrivate) {
      documentLayout->priv()->mainViewPrivate = d;
   }

   d->documentLayoutPtr = documentLayout;
   d->updateDefaultTextOption();
   d->relayoutDocument();
   d->_q_adjustScrollbars();
}

QTextDocument *QPlainTextEdit::document() const
{
   Q_D(const QPlainTextEdit);
   return d->control->document();
}

void QPlainTextEdit::setPlaceholderText(const QString &placeholderText)
{
    Q_D(QPlainTextEdit);

    if (d->placeholderText != placeholderText) {
        d->placeholderText = placeholderText;

        if (d->control->document()->isEmpty()) {
           d->viewport->update();
        }
    }
}

QString QPlainTextEdit::placeholderText() const
{
    Q_D(const QPlainTextEdit);
    return d->placeholderText;
}

void QPlainTextEdit::setTextCursor(const QTextCursor &cursor)
{
   doSetTextCursor(cursor);
}
void QPlainTextEdit::doSetTextCursor(const QTextCursor &cursor)
{
   Q_D(QPlainTextEdit);
   d->control->setTextCursor(cursor);
}

QTextCursor QPlainTextEdit::textCursor() const
{
   Q_D(const QPlainTextEdit);
   return d->control->textCursor();
}

QString QPlainTextEdit::anchorAt(const QPoint &pos) const
{
   Q_D(const QPlainTextEdit);
   int cursorPos = d->control->hitTest(pos + QPointF(d->horizontalOffset(),
            d->verticalOffset()), Qt::ExactHit);

   if (cursorPos < 0) {
      return QString();
   }

   QTextDocumentPrivate *pieceTable = document()->docHandle();
   QTextDocumentPrivate::FragmentIterator it = pieceTable->find(cursorPos);
   QTextCharFormat fmt = pieceTable->formatCollection()->charFormat(it->format);
   return fmt.anchorHref();
}

void QPlainTextEdit::undo()
{
   Q_D(QPlainTextEdit);
   d->control->undo();
}

void QPlainTextEdit::redo()
{
   Q_D(QPlainTextEdit);
   d->control->redo();
}


#ifndef QT_NO_CLIPBOARD
void QPlainTextEdit::cut()
{
   Q_D(QPlainTextEdit);
   d->control->cut();
}

void QPlainTextEdit::copy()
{
   Q_D(QPlainTextEdit);
   d->control->copy();
}

void QPlainTextEdit::paste()
{
   Q_D(QPlainTextEdit);
   d->control->paste();
}
#endif


void QPlainTextEdit::clear()
{
   Q_D(QPlainTextEdit);
   // clears and sets empty content
   d->control->topBlock = d->topLine = d->topLineFracture = 0;
   d->control->clear();
}

void QPlainTextEdit::selectAll()
{
   Q_D(QPlainTextEdit);
   d->control->selectAll();
}

// internal
bool QPlainTextEdit::event(QEvent *e)
{
   Q_D(QPlainTextEdit);

#ifndef QT_NO_CONTEXTMENU
   if (e->type() == QEvent::ContextMenu
      && static_cast<QContextMenuEvent *>(e)->reason() == QContextMenuEvent::Keyboard) {
      ensureCursorVisible();
      const QPoint cursorPos = cursorRect().center();
      QContextMenuEvent ce(QContextMenuEvent::Keyboard, cursorPos, d->viewport->mapToGlobal(cursorPos));
      ce.setAccepted(e->isAccepted());
      const bool result = QAbstractScrollArea::event(&ce);
      e->setAccepted(ce.isAccepted());
      return result;
   }
#endif

   if (e->type() == QEvent::ShortcutOverride
      || e->type() == QEvent::ToolTip) {
      d->sendControlEvent(e);
   }

#ifdef QT_KEYPAD_NAVIGATION
   else if (e->type() == QEvent::EnterEditFocus || e->type() == QEvent::LeaveEditFocus) {
      if (QApplication::keypadNavigationEnabled()) {
         d->sendControlEvent(e);
      }
   }
#endif

#ifndef QT_NO_GESTURES
   else if (e->type() == QEvent::Gesture) {
      QGestureEvent *ge = static_cast<QGestureEvent *>(e);
      QPanGesture *g = static_cast<QPanGesture *>(ge->gesture(Qt::PanGesture));
      if (g) {
         QScrollBar *hBar = horizontalScrollBar();
         QScrollBar *vBar = verticalScrollBar();

         if (g->state() == Qt::GestureStarted) {
            d->originalOffsetY = vBar->value();
         }

         QPointF offset = g->offset();
         if (!offset.isNull()) {
            if (QApplication::isRightToLeft()) {
               offset.rx() *= -1;
            }

            // QPlainTextEdit scrolls by lines only in vertical direction
            QFontMetrics fm(document()->defaultFont());
            int lineHeight = fm.height();
            int newX = hBar->value() - g->delta().x();
            int newY = d->originalOffsetY - offset.y() / lineHeight;
            hBar->setValue(newX);
            vBar->setValue(newY);
         }
      }
      return true;
   }
#endif

   return QAbstractScrollArea::event(e);
}

// internal
void QPlainTextEdit::timerEvent(QTimerEvent *e)
{
   Q_D(QPlainTextEdit);

   if (e->timerId() == d->autoScrollTimer.timerId()) {
      QRect visible = d->viewport->rect();
      QPoint pos;

      if (d->inDrag) {
         pos = d->autoScrollDragPos;
         visible.adjust(qMin(visible.width() / 3, 20), qMin(visible.height() / 3, 20),
            -qMin(visible.width() / 3, 20), -qMin(visible.height() / 3, 20));

      } else {
         const QPoint globalPos = QCursor::pos();
         pos = d->viewport->mapFromGlobal(globalPos);

         QMouseEvent ev(QEvent::MouseMove, pos, d->viewport->mapTo(d->viewport->topLevelWidget(), pos), globalPos,
            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
         mouseMoveEvent(&ev);
      }

      int deltaY = qMax(pos.y() - visible.top(), visible.bottom() - pos.y()) - visible.height();
      int deltaX = qMax(pos.x() - visible.left(), visible.right() - pos.x()) - visible.width();
      int delta = qMax(deltaX, deltaY);

      if (delta >= 0) {
         if (delta < 7) {
            delta = 7;
         }
         int timeout = 4900 / (delta * delta);
         d->autoScrollTimer.start(timeout, this);

         if (deltaY > 0)
            d->vbar->triggerAction(pos.y() < visible.center().y() ?
               QAbstractSlider::SliderSingleStepSub
               : QAbstractSlider::SliderSingleStepAdd);
         if (deltaX > 0)
            d->hbar->triggerAction(pos.x() < visible.center().x() ?
               QAbstractSlider::SliderSingleStepSub
               : QAbstractSlider::SliderSingleStepAdd);
      }
   }
#ifdef QT_KEYPAD_NAVIGATION
   else if (e->timerId() == d->deleteAllTimer.timerId()) {
      d->deleteAllTimer.stop();
      clear();
   }
#endif
}

void QPlainTextEdit::setPlainText(const QString &text)
{
   Q_D(QPlainTextEdit);
   d->control->setPlainText(text);
}

void QPlainTextEdit::keyPressEvent(QKeyEvent *e)
{
   Q_D(QPlainTextEdit);

#ifdef QT_KEYPAD_NAVIGATION
   switch (e->key()) {
      case Qt::Key_Select:
         if (QApplication::keypadNavigationEnabled()) {
            if (!(d->control->textInteractionFlags() & Qt::LinksAccessibleByKeyboard)) {
               setEditFocus(!hasEditFocus());
            } else {
               if (!hasEditFocus()) {
                  setEditFocus(true);
               } else {
                  QTextCursor cursor = d->control->textCursor();
                  QTextCharFormat charFmt = cursor.charFormat();
                  if (!cursor.hasSelection() || charFmt.anchorHref().isEmpty()) {
                     setEditFocus(false);
                  }
               }
            }
         }
         break;
      case Qt::Key_Back:
      case Qt::Key_No:
         if (!QApplication::keypadNavigationEnabled()
            || (QApplication::keypadNavigationEnabled() && !hasEditFocus())) {
            e->ignore();
            return;
         }
         break;
      default:
         if (QApplication::keypadNavigationEnabled()) {
            if (!hasEditFocus() && !(e->modifiers() & Qt::ControlModifier)) {
               if (e->text()[0].isPrint()) {
                  setEditFocus(true);
                  clear();
               } else {
                  e->ignore();
                  return;
               }
            }
         }
         break;
   }
#endif

#ifndef QT_NO_SHORTCUT

   Qt::TextInteractionFlags tif = d->control->textInteractionFlags();

   if (tif & Qt::TextSelectableByKeyboard) {
      if (e == QKeySequence::SelectPreviousPage) {
         e->accept();
         d->pageUpDown(QTextCursor::Up, QTextCursor::KeepAnchor);
         return;
      } else if (e == QKeySequence::SelectNextPage) {
         e->accept();
         d->pageUpDown(QTextCursor::Down, QTextCursor::KeepAnchor);
         return;
      }
   }
   if (tif & (Qt::TextSelectableByKeyboard | Qt::TextEditable)) {
      if (e == QKeySequence::MoveToPreviousPage) {
         e->accept();
         d->pageUpDown(QTextCursor::Up, QTextCursor::MoveAnchor);
         return;
      } else if (e == QKeySequence::MoveToNextPage) {
         e->accept();
         d->pageUpDown(QTextCursor::Down, QTextCursor::MoveAnchor);
         return;
      }
   }

   if (!(tif & Qt::TextEditable)) {
      switch (e->key()) {
         case Qt::Key_Space:
            e->accept();
            if (e->modifiers() & Qt::ShiftModifier) {
               d->vbar->triggerAction(QAbstractSlider::SliderPageStepSub);
            } else {
               d->vbar->triggerAction(QAbstractSlider::SliderPageStepAdd);
            }
            break;

         default:
            d->sendControlEvent(e);
            if (!e->isAccepted() && e->modifiers() == Qt::NoModifier) {
               if (e->key() == Qt::Key_Home) {
                  d->vbar->triggerAction(QAbstractSlider::SliderToMinimum);
                  e->accept();
               } else if (e->key() == Qt::Key_End) {
                  d->vbar->triggerAction(QAbstractSlider::SliderToMaximum);
                  e->accept();
               }
            }
            if (!e->isAccepted()) {
               QAbstractScrollArea::keyPressEvent(e);
            }
      }
      return;
   }
#endif // QT_NO_SHORTCUT

   d->sendControlEvent(e);

#ifdef QT_KEYPAD_NAVIGATION
   if (!e->isAccepted()) {
      switch (e->key()) {
         case Qt::Key_Up:
         case Qt::Key_Down:
            if (QApplication::keypadNavigationEnabled()) {
               // Cursor position didn't change, so we want to leave
               // these keys to change focus.
               e->ignore();
               return;
            }
            break;
         case Qt::Key_Left:
         case Qt::Key_Right:
            if (QApplication::keypadNavigationEnabled()
               && QApplication::navigationMode() == Qt::NavigationModeKeypadDirectional) {
               // Same as for Key_Up and Key_Down.
               e->ignore();
               return;
            }
            break;
         case Qt::Key_Back:
            if (!e->isAutoRepeat()) {
               if (QApplication::keypadNavigationEnabled()) {
                  if (document()->isEmpty()) {
                     setEditFocus(false);
                     e->accept();
                  } else if (!d->deleteAllTimer.isActive()) {
                     e->accept();
                     d->deleteAllTimer.start(750, this);
                  }
               } else {
                  e->ignore();
                  return;
               }
            }
            break;
         default:
            break;
      }
   }
#endif
}

void QPlainTextEdit::keyReleaseEvent(QKeyEvent *e)
{
#ifdef QT_KEYPAD_NAVIGATION
   Q_D(QPlainTextEdit);

   if (QApplication::keypadNavigationEnabled()) {
      if (!e->isAutoRepeat() && e->key() == Qt::Key_Back
         && d->deleteAllTimer.isActive()) {
         d->deleteAllTimer.stop();
         QTextCursor cursor = d->control->textCursor();
         QTextBlockFormat blockFmt = cursor.blockFormat();

         QTextList *list = cursor.currentList();
         if (list && cursor.atBlockStart()) {
            list->remove(cursor.block());
         } else if (cursor.atBlockStart() && blockFmt.indent() > 0) {
            blockFmt.setIndent(blockFmt.indent() - 1);
            cursor.setBlockFormat(blockFmt);
         } else {
            cursor.deletePreviousChar();
         }
         setTextCursor(cursor);
      }
   }

#else
   QWidget::keyReleaseEvent(e);
#endif
}

QVariant QPlainTextEdit::loadResource(int type, const QUrl &name)
{
   (void) type;
   (void) name;

   return QVariant();
}

void QPlainTextEdit::resizeEvent(QResizeEvent *e)
{
   Q_D(QPlainTextEdit);
   if (e->oldSize().width() != e->size().width()) {
      d->relayoutDocument();
   }
   d->_q_adjustScrollbars();
}

void QPlainTextEditPrivate::relayoutDocument()
{
   QTextDocument *doc = control->document();
   QPlainTextDocumentLayout *documentLayout = qobject_cast<QPlainTextDocumentLayout *>(doc->documentLayout());
   Q_ASSERT(documentLayout);
   documentLayoutPtr = documentLayout;

   int width = viewport->width();

   if (documentLayout->priv()->mainViewPrivate == nullptr || documentLayout->priv()->mainViewPrivate == this
               || width > documentLayout->textWidth()) {
      documentLayout->priv()->mainViewPrivate = this;
      documentLayout->setTextWidth(width);
   }
}

static void fillBackground(QPainter *p, const QRectF &rect, QBrush brush, QRectF gradientRect = QRectF())
{
   p->save();
   if (brush.style() >= Qt::LinearGradientPattern && brush.style() <= Qt::ConicalGradientPattern) {
      if (!gradientRect.isNull()) {
         QTransform m = QTransform::fromTranslate(gradientRect.left(), gradientRect.top());
         m.scale(gradientRect.width(), gradientRect.height());
         brush.setTransform(m);
         const_cast<QGradient *>(brush.gradient())->setCoordinateMode(QGradient::LogicalMode);
      }
   } else {
      p->setBrushOrigin(rect.topLeft());
   }
   p->fillRect(rect, brush);
   p->restore();
}

void QPlainTextEdit::paintEvent(QPaintEvent *e)
{
   QPainter painter(viewport());
   Q_ASSERT(qobject_cast<QPlainTextDocumentLayout *>(document()->documentLayout()));

   QPointF offset(contentOffset());

   QRect er = e->rect();
   QRect viewportRect = viewport()->rect();

   bool editable = !isReadOnly();

   QTextBlock block = firstVisibleBlock();
   qreal maximumWidth = document()->documentLayout()->documentSize().width();

   // Set a brush origin so that the WaveUnderline knows where the wave started
   painter.setBrushOrigin(offset);

   // keep right margin clean from full-width selection
   int maxX = offset.x() + qMax((qreal)viewportRect.width(), maximumWidth) - document()->documentMargin();
   er.setRight(qMin(er.right(), maxX));
   painter.setClipRect(er);

   QAbstractTextDocumentLayout::PaintContext context = getPaintContext();

   while (block.isValid()) {

      QRectF r = blockBoundingRect(block).translated(offset);
      QTextLayout *layout = block.layout();

      if (!block.isVisible()) {
         offset.ry() += r.height();
         block = block.next();
         continue;
      }

      if (r.bottom() >= er.top() && r.top() <= er.bottom()) {

         QTextBlockFormat blockFormat = block.blockFormat();

         QBrush bg = blockFormat.background();
         if (bg != Qt::NoBrush) {
            QRectF contentsRect = r;
            contentsRect.setWidth(qMax(r.width(), maximumWidth));
            fillBackground(&painter, contentsRect, bg);
         }

         QVector<QTextLayout::FormatRange> selections;
         int blpos = block.position();
         int bllen = block.length();

         for (int i = 0; i < context.selections.size(); ++i) {
            const QAbstractTextDocumentLayout::Selection &range = context.selections.at(i);
            const int selStart = range.cursor.selectionStart() - blpos;
            const int selEnd = range.cursor.selectionEnd() - blpos;
            if (selStart < bllen && selEnd > 0
               && selEnd > selStart) {
               QTextLayout::FormatRange o;
               o.start = selStart;
               o.length = selEnd - selStart;
               o.format = range.format;
               selections.append(o);
            } else if (!range.cursor.hasSelection() && range.format.hasProperty(QTextFormat::FullWidthSelection)
               && block.contains(range.cursor.position())) {
               // for full width selections we don't require an actual selection, just
               // a position to specify the line. that's more convenience in usage.
               QTextLayout::FormatRange o;
               QTextLine l = layout->lineForTextPosition(range.cursor.position() - blpos);
               o.start = l.textStart();
               o.length = l.textLength();

               if (o.start + o.length == bllen - 1) {
                  ++o.length;   // include newline
               }

               o.format = range.format;
               selections.append(o);
            }
         }

         bool drawCursor = ((editable || (textInteractionFlags() & Qt::TextSelectableByKeyboard))
               && context.cursorPosition >= blpos
               && context.cursorPosition < blpos + bllen);

         bool drawCursorAsBlock = drawCursor && overwriteMode() ;

         if (drawCursorAsBlock) {
            if (context.cursorPosition == blpos + bllen - 1) {
               drawCursorAsBlock = false;
            } else {
               QTextLayout::FormatRange o;
               o.start = context.cursorPosition - blpos;
               o.length = 1;
               o.format.setForeground(palette().base());
               o.format.setBackground(palette().text());
               selections.append(o);
            }
         }


         if (!placeholderText().isEmpty() && document()->isEmpty()) {
            Q_D(QPlainTextEdit);
            QColor col = d->control->palette().text().color();
            col.setAlpha(128);
            painter.setPen(col);
            const int margin = int(document()->documentMargin());
            painter.drawText(r.adjusted(margin, 0, 0, 0), Qt::AlignTop | Qt::TextWordWrap, placeholderText());
         } else {
            layout->draw(&painter, offset, selections, er);
         }

         if ((drawCursor && !drawCursorAsBlock)
            || (editable && context.cursorPosition < -1
               && !layout->preeditAreaText().isEmpty())) {
            int cpos = context.cursorPosition;
            if (cpos < -1) {
               cpos = layout->preeditAreaPosition() - (cpos + 2);
            } else {
               cpos -= blpos;
            }
            layout->drawCursor(&painter, offset, cpos, cursorWidth());
         }
      }

      offset.ry() += r.height();
      if (offset.y() > viewportRect.height()) {
         break;
      }
      block = block.next();
   }

   if (backgroundVisible() && !block.isValid() && offset.y() <= er.bottom()
      && (centerOnScroll() || verticalScrollBar()->maximum() == verticalScrollBar()->minimum())) {
      painter.fillRect(QRect(QPoint((int)er.left(), (int)offset.y()), er.bottomRight()), palette().background());
   }
}

void QPlainTextEditPrivate::updateDefaultTextOption()
{
   QTextDocument *doc = control->document();

   QTextOption opt = doc->defaultTextOption();
   QTextOption::WrapMode oldWrapMode = opt.wrapMode();

   if (lineWrap == QPlainTextEdit::NoWrap) {
      opt.setWrapMode(QTextOption::NoWrap);
   } else {
      opt.setWrapMode(wordWrap);
   }

   if (opt.wrapMode() != oldWrapMode) {
      doc->setDefaultTextOption(opt);
   }
}

void QPlainTextEdit::mousePressEvent(QMouseEvent *e)
{
   Q_D(QPlainTextEdit);

#ifdef QT_KEYPAD_NAVIGATION
   if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
      setEditFocus(true);
   }
#endif
   d->sendControlEvent(e);
}

void QPlainTextEdit::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QPlainTextEdit);

   d->inDrag = false; // paranoia
   const QPoint pos = e->pos();
   d->sendControlEvent(e);
   if (!(e->buttons() & Qt::LeftButton)) {
      return;
   }

   if (e->source() == Qt::MouseEventNotSynthesized) {
      const QRect visible = d->viewport->rect();

      if (visible.contains(pos)) {
         d->autoScrollTimer.stop();
      } else if (!d->autoScrollTimer.isActive()) {
         d->autoScrollTimer.start(100, this);
      }
   }
}

void QPlainTextEdit::mouseReleaseEvent(QMouseEvent *e)
{
   Q_D(QPlainTextEdit);
   d->sendControlEvent(e);

   if (e->source() == Qt::MouseEventNotSynthesized && d->autoScrollTimer.isActive()) {
      d->autoScrollTimer.stop();
      d->ensureCursorVisible();
   }

   if (!isReadOnly() && rect().contains(e->pos())) {
      d->handleSoftwareInputPanel(e->button(), d->clickCausedFocus);
   }
   d->clickCausedFocus = 0;
}

void QPlainTextEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
   Q_D(QPlainTextEdit);
   d->sendControlEvent(e);
}

bool QPlainTextEdit::focusNextPrevChild(bool next)
{
   Q_D(const QPlainTextEdit);
   if (! d->tabChangesFocus && d->control->textInteractionFlags() & Qt::TextEditable) {
      return false;
   }

   return QAbstractScrollArea::focusNextPrevChild(next);
}

#ifndef QT_NO_CONTEXTMENU
void QPlainTextEdit::contextMenuEvent(QContextMenuEvent *e)
{
   Q_D(QPlainTextEdit);
   d->sendControlEvent(e);
}
#endif // QT_NO_CONTEXTMENU

#ifndef QT_NO_DRAGANDDROP

void QPlainTextEdit::dragEnterEvent(QDragEnterEvent *e)
{
   Q_D(QPlainTextEdit);
   d->inDrag = true;
   d->sendControlEvent(e);
}

void QPlainTextEdit::dragLeaveEvent(QDragLeaveEvent *e)
{
   Q_D(QPlainTextEdit);
   d->inDrag = false;
   d->autoScrollTimer.stop();
   d->sendControlEvent(e);
}

void QPlainTextEdit::dragMoveEvent(QDragMoveEvent *e)
{
   Q_D(QPlainTextEdit);
   d->autoScrollDragPos = e->pos();

   if (! d->autoScrollTimer.isActive()) {
      d->autoScrollTimer.start(100, this);
   }

   d->sendControlEvent(e);
}

void QPlainTextEdit::dropEvent(QDropEvent *e)
{
   Q_D(QPlainTextEdit);
   d->inDrag = false;
   d->autoScrollTimer.stop();
   d->sendControlEvent(e);
}

#endif // QT_NO_DRAGANDDROP

void QPlainTextEdit::inputMethodEvent(QInputMethodEvent *e)
{
   Q_D(QPlainTextEdit);

#ifdef QT_KEYPAD_NAVIGATION
   if (d->control->textInteractionFlags() & Qt::TextEditable
         && QApplication::keypadNavigationEnabled() && ! hasEditFocus()) {
      setEditFocus(true);
      selectAll();    // so text is replaced rather than appended to
   }
#endif

   d->sendControlEvent(e);
   ensureCursorVisible();
}

void QPlainTextEdit::scrollContentsBy(int dx, int)
{
   Q_D(QPlainTextEdit);
   d->setTopLine(d->vbar->value(), dx);
}

QVariant QPlainTextEdit::inputMethodQuery(Qt::InputMethodQuery property) const
{
   return inputMethodQuery(property, QVariant());
}

QVariant QPlainTextEdit::inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const
{
   Q_D(const QPlainTextEdit);

   if (query == Qt::ImHints) {
      return QWidget::inputMethodQuery(query);
   }

   const QVariant v = d->control->inputMethodQuery(query, argument);
   const QPointF offset = contentOffset();

   switch (v.type()) {
      case QVariant::RectF:
         return v.toRectF().translated(offset);

      case QVariant::PointF:
         return v.toPointF() + offset;

      case QVariant::Rect:
         return v.toRect().translated(offset.toPoint());

      case QVariant::Point:
         return v.toPoint() + offset.toPoint();

      default:
         break;
   }

   return v;
}

void QPlainTextEdit::focusInEvent(QFocusEvent *e)
{
   Q_D(QPlainTextEdit);

   if (e->reason() == Qt::MouseFocusReason) {
      d->clickCausedFocus = 1;
   }

   QAbstractScrollArea::focusInEvent(e);
   d->sendControlEvent(e);
}

void QPlainTextEdit::focusOutEvent(QFocusEvent *e)
{
   Q_D(QPlainTextEdit);
   QAbstractScrollArea::focusOutEvent(e);
   d->sendControlEvent(e);
}

void QPlainTextEdit::showEvent(QShowEvent *)
{
   Q_D(QPlainTextEdit);
   if (d->showCursorOnInitialShow) {
      d->showCursorOnInitialShow = false;
      ensureCursorVisible();
   }
}

void QPlainTextEdit::changeEvent(QEvent *e)
{
   Q_D(QPlainTextEdit);
   QAbstractScrollArea::changeEvent(e);

   if (e->type() == QEvent::ApplicationFontChange || e->type() == QEvent::FontChange) {
      d->control->document()->setDefaultFont(font());

   }  else if (e->type() == QEvent::ActivationChange) {
      if (!isActiveWindow()) {
         d->autoScrollTimer.stop();
      }

   } else if (e->type() == QEvent::EnabledChange) {
      e->setAccepted(isEnabled());
      d->sendControlEvent(e);

   } else if (e->type() == QEvent::PaletteChange) {
      d->control->setPalette(palette());

   } else if (e->type() == QEvent::LayoutDirectionChange) {
      d->sendControlEvent(e);
   }
}

#ifndef QT_NO_WHEELEVENT
void QPlainTextEdit::wheelEvent(QWheelEvent *e)
{
   Q_D(QPlainTextEdit);

   if (!(d->control->textInteractionFlags() & Qt::TextEditable)) {
      if (e->modifiers() & Qt::ControlModifier) {
         float delta = e->angleDelta().y() / 120.f;
         zoomInF(delta);
         return;
      }
   }
   QAbstractScrollArea::wheelEvent(e);
   updateMicroFocus();
}
#endif

void QPlainTextEdit::zoomIn(int range)
{
   zoomInF(range);
}

void QPlainTextEdit::zoomOut(int range)
{
   zoomInF(-range);
}

void QPlainTextEdit::zoomInF(float range)
{
   if (range == 0.f) {
      return;
   }

   QFont f = font();
   const float newSize = f.pointSizeF() + range;

   if (newSize <= 0) {
      return;
   }

   f.setPointSizeF(newSize);
   setFont(f);
}

#ifndef QT_NO_CONTEXTMENU

QMenu *QPlainTextEdit::createStandardContextMenu()
{
   Q_D(QPlainTextEdit);
   return d->control->createStandardContextMenu(QPointF(), this);
}

QMenu *QPlainTextEdit::createStandardContextMenu(const QPoint &position)
{
   Q_D(QPlainTextEdit);
   return d->control->createStandardContextMenu(position, this);
}
#endif

QTextCursor QPlainTextEdit::cursorForPosition(const QPoint &pos) const
{
   Q_D(const QPlainTextEdit);
   return d->control->cursorForPosition(d->mapToContents(pos));
}

QRect QPlainTextEdit::cursorRect(const QTextCursor &cursor) const
{
   Q_D(const QPlainTextEdit);
   if (cursor.isNull()) {
      return QRect();
   }

   QRect r = d->control->cursorRect(cursor).toRect();
   r.translate(-d->horizontalOffset(), -(int)d->verticalOffset());
   return r;
}

QRect QPlainTextEdit::cursorRect() const
{
   Q_D(const QPlainTextEdit);
   QRect r = d->control->cursorRect().toRect();
   r.translate(-d->horizontalOffset(), -(int)d->verticalOffset());
   return r;
}

bool QPlainTextEdit::overwriteMode() const
{
   Q_D(const QPlainTextEdit);
   return d->control->overwriteMode();
}

void QPlainTextEdit::setOverwriteMode(bool overwrite)
{
   Q_D(QPlainTextEdit);
   d->control->setOverwriteMode(overwrite);
}

int QPlainTextEdit::tabStopWidth() const
{
   Q_D(const QPlainTextEdit);
   return qRound(d->control->document()->defaultTextOption().tabStop());
}

void QPlainTextEdit::setTabStopWidth(int width)
{
   Q_D(QPlainTextEdit);
   QTextOption opt = d->control->document()->defaultTextOption();
   if (opt.tabStop() == width || width < 0) {
      return;
   }
   opt.setTabStop(width);
   d->control->document()->setDefaultTextOption(opt);
}

int QPlainTextEdit::cursorWidth() const
{
   Q_D(const QPlainTextEdit);
   return d->control->cursorWidth();
}

void QPlainTextEdit::setCursorWidth(int width)
{
   Q_D(QPlainTextEdit);
   d->control->setCursorWidth(width);
}

void QPlainTextEdit::setExtraSelections(const QList<QTextEdit::ExtraSelection> &selections)
{
   Q_D(QPlainTextEdit);
   d->control->setExtraSelections(selections);
}

QList<QTextEdit::ExtraSelection> QPlainTextEdit::extraSelections() const
{
   Q_D(const QPlainTextEdit);
   return d->control->extraSelections();
}

QMimeData *QPlainTextEdit::createMimeDataFromSelection() const
{
   Q_D(const QPlainTextEdit);
   return d->control->QTextControl::createMimeDataFromSelection();
}

bool QPlainTextEdit::canInsertFromMimeData(const QMimeData *source) const
{
   Q_D(const QPlainTextEdit);
   return d->control->QTextControl::canInsertFromMimeData(source);
}

void QPlainTextEdit::insertFromMimeData(const QMimeData *source)
{
   Q_D(QPlainTextEdit);
   d->control->QTextControl::insertFromMimeData(source);
}

bool QPlainTextEdit::isReadOnly() const
{
   Q_D(const QPlainTextEdit);
   return !(d->control->textInteractionFlags() & Qt::TextEditable);
}

void QPlainTextEdit::setReadOnly(bool ro)
{
   Q_D(QPlainTextEdit);

   Qt::TextInteractionFlags flags = Qt::NoTextInteraction;

   if (ro) {
      flags = Qt::TextSelectableByMouse;
   } else {
      flags = Qt::TextEditorInteraction;
   }

   setAttribute(Qt::WA_InputMethodEnabled, shouldEnableInputMethod(this));
   d->control->setTextInteractionFlags(flags);
   QEvent event(QEvent::ReadOnlyChange);
   QApplication::sendEvent(this, &event);
}

void QPlainTextEdit::setTextInteractionFlags(Qt::TextInteractionFlags flags)
{
   Q_D(QPlainTextEdit);
   d->control->setTextInteractionFlags(flags);
}

Qt::TextInteractionFlags QPlainTextEdit::textInteractionFlags() const
{
   Q_D(const QPlainTextEdit);
   return d->control->textInteractionFlags();
}

void QPlainTextEdit::mergeCurrentCharFormat(const QTextCharFormat &modifier)
{
   Q_D(QPlainTextEdit);
   d->control->mergeCurrentCharFormat(modifier);
}

void QPlainTextEdit::setCurrentCharFormat(const QTextCharFormat &format)
{
   Q_D(QPlainTextEdit);
   d->control->setCurrentCharFormat(format);
}

QTextCharFormat QPlainTextEdit::currentCharFormat() const
{
   Q_D(const QPlainTextEdit);
   return d->control->currentCharFormat();
}

void QPlainTextEdit::insertPlainText(const QString &text)
{
   Q_D(QPlainTextEdit);
   d->control->insertPlainText(text);
}

void QPlainTextEdit::moveCursor(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode)
{
   Q_D(QPlainTextEdit);
   d->control->moveCursor(operation, mode);
}

bool QPlainTextEdit::canPaste() const
{
   Q_D(const QPlainTextEdit);
   return d->control->canPaste();
}

#ifndef QT_NO_PRINTER
void QPlainTextEdit::print(QPagedPaintDevice *printer) const
{
   Q_D(const QPlainTextEdit);
   d->control->print(printer);
}
#endif

bool QPlainTextEdit::tabChangesFocus() const
{
   Q_D(const QPlainTextEdit);
   return d->tabChangesFocus;
}

void QPlainTextEdit::setTabChangesFocus(bool b)
{
   Q_D(QPlainTextEdit);
   d->tabChangesFocus = b;
}

QPlainTextEdit::LineWrapMode QPlainTextEdit::lineWrapMode() const
{
   Q_D(const QPlainTextEdit);
   return d->lineWrap;
}

void QPlainTextEdit::setLineWrapMode(LineWrapMode wrap)
{
   Q_D(QPlainTextEdit);

   if (d->lineWrap == wrap) {
      return;
   }

   d->lineWrap = wrap;
   d->updateDefaultTextOption();
   d->relayoutDocument();
   d->_q_adjustScrollbars();
   ensureCursorVisible();
}

QTextOption::WrapMode QPlainTextEdit::wordWrapMode() const
{
   Q_D(const QPlainTextEdit);
   return d->wordWrap;
}

void QPlainTextEdit::setWordWrapMode(QTextOption::WrapMode mode)
{
   Q_D(QPlainTextEdit);
   if (mode == d->wordWrap) {
      return;
   }

   d->wordWrap = mode;
   d->updateDefaultTextOption();
}

bool QPlainTextEdit::backgroundVisible() const
{
   Q_D(const QPlainTextEdit);
   return d->backgroundVisible;
}

void QPlainTextEdit::setBackgroundVisible(bool visible)
{
   Q_D(QPlainTextEdit);
   if (visible == d->backgroundVisible) {
      return;
   }

   d->backgroundVisible = visible;
   d->updateViewport();
}

bool QPlainTextEdit::centerOnScroll() const
{
   Q_D(const QPlainTextEdit);
   return d->centerOnScroll;
}

void QPlainTextEdit::setCenterOnScroll(bool enabled)
{
   Q_D(QPlainTextEdit);
   if (enabled == d->centerOnScroll) {
      return;
   }
   d->centerOnScroll = enabled;
}

bool QPlainTextEdit::find(const QString &exp, QTextDocument::FindFlags options)
{
   Q_D(QPlainTextEdit);
   return d->control->find(exp, options);
}

bool QPlainTextEdit::find(const QRegularExpression &exp, QTextDocument::FindFlags options)
{
   Q_D(QPlainTextEdit);
   return d->control->find(exp, options);
}

void QPlainTextEditPrivate::append(const QString &text, Qt::TextFormat format)
{
   Q_Q(QPlainTextEdit);

   QTextDocument *document = control->document();
   QPlainTextDocumentLayout *documentLayout = qobject_cast<QPlainTextDocumentLayout *>(document->documentLayout());
   Q_ASSERT(documentLayout);

   int maximumBlockCount = document->maximumBlockCount();
   if (maximumBlockCount) {
      document->setMaximumBlockCount(0);
   }

   const bool atBottom =  q->isVisible()
      && (control->blockBoundingRect(document->lastBlock()).bottom() - verticalOffset()
         <= viewport->rect().bottom());

   if (! q->isVisible()) {
      showCursorOnInitialShow = true;
   }

   bool documentSizeChangedBlocked = documentLayout->priv()->blockDocumentSizeChanged;
   documentLayout->priv()->blockDocumentSizeChanged = true;

   if (format == Qt::RichText) {
      control->appendHtml(text);
   } else if (format == Qt::PlainText) {
      control->appendPlainText(text);
   } else {
      control->append(text);
   }

   if (maximumBlockCount > 0) {
      if (document->blockCount() > maximumBlockCount) {
         bool blockUpdate = false;
         if (control->topBlock) {
            control->topBlock--;
            blockUpdate = true;
            emit q->updateRequest(viewport->rect(), 0);
         }

         bool updatesBlocked = documentLayout->priv()->blockUpdate;
         documentLayout->priv()->blockUpdate = blockUpdate;
         QTextCursor cursor(document);
         cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
         cursor.removeSelectedText();
         documentLayout->priv()->blockUpdate = updatesBlocked;
      }
      document->setMaximumBlockCount(maximumBlockCount);
   }

   documentLayout->priv()->blockDocumentSizeChanged = documentSizeChangedBlocked;
   _q_adjustScrollbars();

   if (atBottom) {
      const bool needScroll = ! centerOnScroll
         || control->blockBoundingRect(document->lastBlock()).bottom() - verticalOffset() > viewport->rect().bottom();

      if (needScroll) {
         vbar->setValue(vbar->maximum());
      }
   }
}

void QPlainTextEdit::appendPlainText(const QString &text)
{
   Q_D(QPlainTextEdit);
   d->append(text, Qt::PlainText);
}

void QPlainTextEdit::appendHtml(const QString &html)
{
   Q_D(QPlainTextEdit);
   d->append(html, Qt::RichText);
}

void QPlainTextEditPrivate::ensureCursorVisible(bool center)
{
   Q_Q(QPlainTextEdit);
   QRect visible = viewport->rect();
   QRect cr = q->cursorRect();
   if (cr.top() < visible.top() || cr.bottom() > visible.bottom()) {
      ensureVisible(control->textCursor().position(), center);
   }

   const bool rtl = q->isRightToLeft();
   if (cr.left() < visible.left() || cr.right() > visible.right()) {
      int x = cr.center().x() + horizontalOffset() - visible.width() / 2;
      hbar->setValue(rtl ? hbar->maximum() - x : x);
   }
}

void QPlainTextEdit::ensureCursorVisible()
{
   Q_D(QPlainTextEdit);
   d->ensureCursorVisible(d->centerOnScroll);
}

void QPlainTextEdit::centerCursor()
{
   Q_D(QPlainTextEdit);
   d->ensureVisible(textCursor().position(), true, true);
}

QTextBlock QPlainTextEdit::firstVisibleBlock() const
{
   Q_D(const QPlainTextEdit);
   return d->control->firstVisibleBlock();
}

QPointF QPlainTextEdit::contentOffset() const
{
   Q_D(const QPlainTextEdit);
   return QPointF(-d->horizontalOffset(), -d->verticalOffset());
}


QRectF QPlainTextEdit::blockBoundingGeometry(const QTextBlock &block) const
{
   Q_D(const QPlainTextEdit);
   return d->control->blockBoundingRect(block);
}

QRectF QPlainTextEdit::blockBoundingRect(const QTextBlock &block) const
{
   QPlainTextDocumentLayout *documentLayout = qobject_cast<QPlainTextDocumentLayout *>(document()->documentLayout());
   Q_ASSERT(documentLayout);
   return documentLayout->blockBoundingRect(block);
}

int QPlainTextEdit::blockCount() const
{
   return document()->blockCount();
}

QAbstractTextDocumentLayout::PaintContext QPlainTextEdit::getPaintContext() const
{
   Q_D(const QPlainTextEdit);
   return d->control->getPaintContext(d->viewport);
}

void QPlainTextEdit::_q_repaintContents(const QRectF &r)
{
   Q_D(QPlainTextEdit);
   d->_q_repaintContents(r);
}

void QPlainTextEdit::_q_adjustScrollbars()
{
   Q_D(QPlainTextEdit);
   d->_q_adjustScrollbars();
}

void QPlainTextEdit::_q_verticalScrollbarActionTriggered(int action)
{
   Q_D(QPlainTextEdit);
   d->_q_verticalScrollbarActionTriggered(action);
}

void QPlainTextEdit::_q_cursorPositionChanged()
{
   Q_D(QPlainTextEdit);
   d->_q_cursorPositionChanged();
}

#endif // QT_NO_TEXTEDIT
