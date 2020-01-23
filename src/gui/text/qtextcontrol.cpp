/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qtextcontrol_p.h>
#include <qtextcontrol_p_p.h>

#ifndef QT_NO_TEXTCONTROL

#include <qaccessible.h>
#include <qapplication.h>
#include <qbuffer.h>
#include <qclipboard.h>
#include <qdatetime.h>
#include <qdesktopservices.h>
#include <qdebug.h>
#include <qdrag.h>
#include <qevent.h>
#include <qfont.h>
#include <qgraphicssceneevent.h>
#include <qinputmethod.h>
#include <qlineedit.h>
#include <qmenu.h>
#include <qpagedpaintdevice.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qstylehints.h>
#include <qstyleoption.h>
#include <qtextdocument.h>
#include <qtimer.h>
#include <qtextformat.h>
#include <qtextlist.h>
#include <qtextdocumentwriter.h>
#include <qtooltip.h>
#include <qtexttable.h>
#include <qvariant.h>
#include <qurl.h>

#include <qtextdocumentlayout_p.h>
#include <qabstracttextdocumentlayout_p.h>
#include <qtextedit_p.h>
#include <qtextdocument_p.h>
#include <qtextcursor_p.h>

#include <limits.h>

#ifndef QT_NO_SHORTCUT
#include <qkeysequence.h>

#include <qapplication_p.h>
#include <qshortcutmap_p.h>

#define ACCEL_KEY(k)   (! qApp->d_func()->shortcutMap.hasShortcutForKeySequence(k) ?  \
                        QLatin1Char('\t') + QKeySequence(k).toString(QKeySequence::NativeText) : QString())
#else
#define ACCEL_KEY(k)   QString()
#endif

#include <algorithm>



// could go into QTextCursor...
static QTextLine currentTextLine(const QTextCursor &cursor)
{
   const QTextBlock block = cursor.block();
   if (! block.isValid()) {
      return QTextLine();
   }

   const QTextLayout *layout = block.layout();
   if (!layout) {
      return QTextLine();
   }

   const int relativePos = cursor.position() - block.position();
   return layout->lineForTextPosition(relativePos);
}

QTextControlPrivate::QTextControlPrivate()
   : doc(0), cursorOn(false), cursorIsFocusIndicator(false),

#ifndef Q_OS_ANDROID
     interactionFlags(Qt::TextEditorInteraction),
#else
     interactionFlags(Qt::TextEditable | Qt::TextSelectableByKeyboard),
#endif

     dragEnabled(true),

#ifndef QT_NO_DRAGANDDROP
     mousePressed(false), mightStartDrag(false),
#endif

     lastSelectionPosition(0),
     lastSelectionAnchor(0),
     ignoreAutomaticScrollbarAdjustement(false),
     overwriteMode(false),
     acceptRichText(true),
     preeditCursor(0), hideCursor(false),
     hasFocus(false),
#ifdef QT_KEYPAD_NAVIGATION
     hasEditFocus(false),
#endif
     isEnabled(true),
     hadSelectionOnMousePress(false),
     ignoreUnusedNavigationEvents(false),
     openExternalLinks(false),
     wordSelectionEnabled(false)
{}

bool QTextControlPrivate::cursorMoveKeyEvent(QKeyEvent *e)
{
#ifdef QT_NO_SHORTCUT
   Q_UNUSED(e);
#endif

   Q_Q(QTextControl);

   if (cursor.isNull()) {
      return false;
   }

   const QTextCursor oldSelection = cursor;
   const int oldCursorPos = cursor.position();

   QTextCursor::MoveMode mode = QTextCursor::MoveAnchor;
   QTextCursor::MoveOperation op = QTextCursor::NoMove;


#ifndef QT_NO_SHORTCUT

   if (e == QKeySequence::MoveToNextChar) {
      op = QTextCursor::Right;
   } else if (e == QKeySequence::MoveToPreviousChar) {
      op = QTextCursor::Left;
   } else if (e == QKeySequence::SelectNextChar) {
      op = QTextCursor::Right;
      mode = QTextCursor::KeepAnchor;
   } else if (e == QKeySequence::SelectPreviousChar) {
      op = QTextCursor::Left;
      mode = QTextCursor::KeepAnchor;
   } else if (e == QKeySequence::SelectNextWord) {
      op = QTextCursor::WordRight;
      mode = QTextCursor::KeepAnchor;
   } else if (e == QKeySequence::SelectPreviousWord) {
      op = QTextCursor::WordLeft;
      mode = QTextCursor::KeepAnchor;
   } else if (e == QKeySequence::SelectStartOfLine) {
      op = QTextCursor::StartOfLine;
      mode = QTextCursor::KeepAnchor;
   } else if (e == QKeySequence::SelectEndOfLine) {
      op = QTextCursor::EndOfLine;
      mode = QTextCursor::KeepAnchor;
   } else if (e == QKeySequence::SelectStartOfBlock) {
      op = QTextCursor::StartOfBlock;
      mode = QTextCursor::KeepAnchor;
   } else if (e == QKeySequence::SelectEndOfBlock) {
      op = QTextCursor::EndOfBlock;
      mode = QTextCursor::KeepAnchor;
   } else if (e == QKeySequence::SelectStartOfDocument) {
      op = QTextCursor::Start;
      mode = QTextCursor::KeepAnchor;
   } else if (e == QKeySequence::SelectEndOfDocument) {
      op = QTextCursor::End;
      mode = QTextCursor::KeepAnchor;
   } else if (e == QKeySequence::SelectPreviousLine) {
      op = QTextCursor::Up;
      mode = QTextCursor::KeepAnchor;
   } else if (e == QKeySequence::SelectNextLine) {
      op = QTextCursor::Down;
      mode = QTextCursor::KeepAnchor;
      {
         QTextBlock block = cursor.block();
         QTextLine line = currentTextLine(cursor);
         if (!block.next().isValid()
            && line.isValid()
            && line.lineNumber() == block.layout()->lineCount() - 1) {
            op = QTextCursor::End;
         }
      }
   } else if (e == QKeySequence::MoveToNextWord) {
      op = QTextCursor::WordRight;
   } else if (e == QKeySequence::MoveToPreviousWord) {
      op = QTextCursor::WordLeft;
   } else if (e == QKeySequence::MoveToEndOfBlock) {
      op = QTextCursor::EndOfBlock;
   } else if (e == QKeySequence::MoveToStartOfBlock) {
      op = QTextCursor::StartOfBlock;
   } else if (e == QKeySequence::MoveToNextLine) {
      op = QTextCursor::Down;
   } else if (e == QKeySequence::MoveToPreviousLine) {
      op = QTextCursor::Up;
   } else if (e == QKeySequence::MoveToStartOfLine) {
      op = QTextCursor::StartOfLine;
   } else if (e == QKeySequence::MoveToEndOfLine) {
      op = QTextCursor::EndOfLine;
   } else if (e == QKeySequence::MoveToStartOfDocument) {
      op = QTextCursor::Start;

   } else if (e == QKeySequence::MoveToEndOfDocument) {
      op = QTextCursor::End;
#endif // QT_NO_SHORTCUT

   } else {
      return false;

   }

   // Except for pageup and pagedown, Mac OS X has very different behavior, we don't do it all, but
   // here's the breakdown:
   // Shift still works as an anchor, but only one of the other keys can be down Ctrl (Command),
   // Alt (Option), or Meta (Control).
   // Command/Control + Left/Right -- Move to left or right of the line
   //                 + Up/Down -- Move to top bottom of the file. (Control doesn't move the cursor)
   // Option + Left/Right -- Move one word Left/right.
   //        + Up/Down  -- Begin/End of Paragraph.
   // Home/End Top/Bottom of file. (usually don't move the cursor, but will select)

   bool visualNavigation = cursor.visualNavigation();
   cursor.setVisualNavigation(true);
   const bool moved = cursor.movePosition(op, mode);
   cursor.setVisualNavigation(visualNavigation);
   q->ensureCursorVisible();

   bool ignoreNavigationEvents = ignoreUnusedNavigationEvents;
   bool isNavigationEvent = e->key() == Qt::Key_Up || e->key() == Qt::Key_Down;

#ifdef QT_KEYPAD_NAVIGATION
   ignoreNavigationEvents = ignoreNavigationEvents || QApplication::keypadNavigationEnabled();
   isNavigationEvent = isNavigationEvent ||
      (QApplication::navigationMode() == Qt::NavigationModeKeypadDirectional
         && (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right));
#else
   isNavigationEvent = isNavigationEvent || e->key() == Qt::Key_Left || e->key() == Qt::Key_Right;
#endif

   if (moved) {
      if (cursor.position() != oldCursorPos) {
         emit q->cursorPositionChanged();
      }
      emit q->microFocusChanged();
   } else if (ignoreNavigationEvents && isNavigationEvent && oldSelection.anchor() == cursor.anchor()) {
      return false;
   }

   selectionChanged(/*forceEmitSelectionChanged =*/(mode == QTextCursor::KeepAnchor));
   repaintOldAndNewSelection(oldSelection);

   return true;
}

void QTextControlPrivate::updateCurrentCharFormat()
{
   Q_Q(QTextControl);

   QTextCharFormat fmt = cursor.charFormat();
   if (fmt == lastCharFormat) {
      return;
   }

   lastCharFormat = fmt;

   emit q->currentCharFormatChanged(fmt);
   emit q->microFocusChanged();
}

void QTextControlPrivate::indent()
{
   QTextBlockFormat blockFmt = cursor.blockFormat();

   QTextList *list = cursor.currentList();
   if (!list) {
      QTextBlockFormat modifier;
      modifier.setIndent(blockFmt.indent() + 1);
      cursor.mergeBlockFormat(modifier);
   } else {
      QTextListFormat format = list->format();
      format.setIndent(format.indent() + 1);

      if (list->itemNumber(cursor.block()) == 1) {
         list->setFormat(format);
      } else {
         cursor.createList(format);
      }
   }
}

void QTextControlPrivate::outdent()
{
   QTextBlockFormat blockFmt = cursor.blockFormat();

   QTextList *list = cursor.currentList();

   if (!list) {
      QTextBlockFormat modifier;
      modifier.setIndent(blockFmt.indent() - 1);
      cursor.mergeBlockFormat(modifier);
   } else {
      QTextListFormat listFmt = list->format();
      listFmt.setIndent(listFmt.indent() - 1);
      list->setFormat(listFmt);
   }
}

void QTextControlPrivate::gotoNextTableCell()
{
   QTextTable *table = cursor.currentTable();
   QTextTableCell cell = table->cellAt(cursor);

   int newColumn = cell.column() + cell.columnSpan();
   int newRow = cell.row();

   if (newColumn >= table->columns()) {
      newColumn = 0;
      ++newRow;
      if (newRow >= table->rows()) {
         table->insertRows(table->rows(), 1);
      }
   }

   cell = table->cellAt(newRow, newColumn);
   cursor = cell.firstCursorPosition();
}

void QTextControlPrivate::gotoPreviousTableCell()
{
   QTextTable *table = cursor.currentTable();
   QTextTableCell cell = table->cellAt(cursor);

   int newColumn = cell.column() - 1;
   int newRow = cell.row();

   if (newColumn < 0) {
      newColumn = table->columns() - 1;
      --newRow;
      if (newRow < 0) {
         return;
      }
   }

   cell = table->cellAt(newRow, newColumn);
   cursor = cell.firstCursorPosition();
}

void QTextControlPrivate::createAutoBulletList()
{
   cursor.beginEditBlock();

   QTextBlockFormat blockFmt = cursor.blockFormat();

   QTextListFormat listFmt;
   listFmt.setStyle(QTextListFormat::ListDisc);
   listFmt.setIndent(blockFmt.indent() + 1);

   blockFmt.setIndent(0);
   cursor.setBlockFormat(blockFmt);

   cursor.createList(listFmt);

   cursor.endEditBlock();
}

void QTextControlPrivate::init(Qt::TextFormat format, const QString &text, QTextDocument *document)
{
   Q_Q(QTextControl);
   setContent(format, text, document);

   doc->setUndoRedoEnabled(interactionFlags & Qt::TextEditable);
   q->setCursorWidth(-1);
}

void QTextControlPrivate::setContent(Qt::TextFormat format, const QString &text, QTextDocument *document)
{
   Q_Q(QTextControl);

   // for use when called from setPlainText. may want to re-use the currently set char format then.
   const QTextCharFormat charFormatForInsertion = cursor.charFormat();

   bool clearDocument = true;
   if (! doc) {
      if (document) {
         doc = document;
         clearDocument = false;
      } else {
         palette = QApplication::palette("QTextControl");
         doc = new QTextDocument(q);
      }

      _q_documentLayoutChanged();
      cursor = QTextCursor(doc);

      // #### doc->documentLayout()->setPaintDevice(viewport);

      QObject::connect(doc, &QTextDocument::contentsChanged,       q, &QTextControl::_q_updateCurrentCharFormatAndSelection);
      QObject::connect(doc, &QTextDocument::cursorPositionChanged, q, &QTextControl::_q_emitCursorPosChanged);
      QObject::connect(doc, &QTextDocument::documentLayoutChanged, q, &QTextControl::_q_documentLayoutChanged);

      // convenience signal forwards
      QObject::connect(doc, &QTextDocument::undoAvailable,       q, &QTextControl::undoAvailable);
      QObject::connect(doc, &QTextDocument::redoAvailable,       q, &QTextControl::redoAvailable);
      QObject::connect(doc, &QTextDocument::modificationChanged, q, &QTextControl::modificationChanged);
      QObject::connect(doc, &QTextDocument::blockCountChanged,   q, &QTextControl::blockCountChanged);
   }

   bool previousUndoRedoState = doc->isUndoRedoEnabled();
   if (! document) {
      doc->setUndoRedoEnabled(false);
   }

   // avoid multiple textChanged() signals being emitted
   QObject::disconnect(doc, &QTextDocument::contentsChanged, q, &QTextControl::textChanged);

   if (! text.isEmpty()) {
      // clear 'our' cursor for insertion to prevent the emission of the cursorPositionChanged() signal.
      // instead we emit it only once at the end instead of at the end of the document after
      // loading and when positioning the cursor again to the start of the document
      cursor = QTextCursor();

      if (format == Qt::PlainText) {
         QTextCursor formatCursor(doc);

         // put the setPlainText and the setCharFormat into one edit block, so that the syntax
         // highlight triggers only /once/ for the entire document, not twice

         formatCursor.beginEditBlock();
         doc->setPlainText(text);
         doc->setUndoRedoEnabled(false);
         formatCursor.select(QTextCursor::Document);
         formatCursor.setCharFormat(charFormatForInsertion);
         formatCursor.endEditBlock();

      } else {
#ifndef QT_NO_TEXTHTMLPARSER
         doc->setHtml(text);
#else
         doc->setPlainText(text);
#endif
         doc->setUndoRedoEnabled(false);
      }

      cursor = QTextCursor(doc);

   } else if (clearDocument) {
      doc->clear();
   }
   cursor.setCharFormat(charFormatForInsertion);

   QObject::connect(doc, &QTextDocument::contentsChanged, q, &QTextControl::textChanged);
   emit q->textChanged();

   if (! document) {
      doc->setUndoRedoEnabled(previousUndoRedoState);
   }

   _q_updateCurrentCharFormatAndSelection();
   if (!document) {
      doc->setModified(false);
   }

   q->ensureCursorVisible();
   emit q->cursorPositionChanged();
   QObject::connect(doc, &QTextDocument::contentsChange, q, &QTextControl::_q_contentsChanged, Qt::UniqueConnection);
}

void QTextControlPrivate::startDrag()
{
#ifndef QT_NO_DRAGANDDROP
   Q_Q(QTextControl);

   mousePressed = false;
   if (!contextWidget) {
      return;
   }
   QMimeData *data = q->createMimeDataFromSelection();

   QDrag *drag = new QDrag(contextWidget);
   drag->setMimeData(data);

   Qt::DropActions actions = Qt::CopyAction;
   Qt::DropAction action;

   if (interactionFlags & Qt::TextEditable) {
      actions |= Qt::MoveAction;
      action = drag->exec(actions, Qt::MoveAction);
   } else {
      action = drag->exec(actions, Qt::CopyAction);
   }

   if (action == Qt::MoveAction && drag->target() != contextWidget) {
      cursor.removeSelectedText();
   }
#endif
}

void QTextControlPrivate::setCursorPosition(const QPointF &pos)
{
   Q_Q(QTextControl);
   const int cursorPos = q->hitTest(pos, Qt::FuzzyHit);
   if (cursorPos == -1) {
      return;
   }
   cursor.setPosition(cursorPos);
}

void QTextControlPrivate::setCursorPosition(int pos, QTextCursor::MoveMode mode)
{
   cursor.setPosition(pos, mode);

   if (mode != QTextCursor::KeepAnchor) {
      selectedWordOnDoubleClick = QTextCursor();
      selectedBlockOnTrippleClick = QTextCursor();
   }
}

void QTextControlPrivate::repaintCursor()
{
   Q_Q(QTextControl);
   emit q->updateRequest(cursorRectPlusUnicodeDirectionMarkers(cursor));
}

void QTextControlPrivate::repaintOldAndNewSelection(const QTextCursor &oldSelection)
{
   Q_Q(QTextControl);

   if (cursor.hasSelection() && oldSelection.hasSelection() && cursor.currentFrame() == oldSelection.currentFrame()
      && !cursor.hasComplexSelection() && !oldSelection.hasComplexSelection() && cursor.anchor() == oldSelection.anchor() ) {

      QTextCursor differenceSelection(doc);
      differenceSelection.setPosition(oldSelection.position());
      differenceSelection.setPosition(cursor.position(), QTextCursor::KeepAnchor);
      emit q->updateRequest(q->selectionRect(differenceSelection));

   } else {
      if (! oldSelection.isNull()) {
         emit q->updateRequest(q->selectionRect(oldSelection) | cursorRectPlusUnicodeDirectionMarkers(oldSelection));
      }

      emit q->updateRequest(q->selectionRect() | cursorRectPlusUnicodeDirectionMarkers(cursor));
   }
}

void QTextControlPrivate::selectionChanged(bool forceEmitSelectionChanged /*=false*/)
{
   Q_Q(QTextControl);

   if (forceEmitSelectionChanged) {
      emit q->selectionChanged();

#ifndef QT_NO_ACCESSIBILITY
      if (q->parent() && q->parent()->isWidgetType()) {
         QAccessibleTextSelectionEvent ev(q->parent(), cursor.anchor(), cursor.position());
         QAccessible::updateAccessibility(&ev);
      }
#endif
   }

   if (cursor.position() == lastSelectionPosition
      && cursor.anchor() == lastSelectionAnchor) {
      return;
   }

   bool selectionStateChange = (cursor.hasSelection()
         != (lastSelectionPosition != lastSelectionAnchor));
   if (selectionStateChange) {
      emit q->copyAvailable(cursor.hasSelection());
   }

   if (!forceEmitSelectionChanged
      && (selectionStateChange
         || (cursor.hasSelection()
            && (cursor.position() != lastSelectionPosition
               || cursor.anchor() != lastSelectionAnchor)))) {
      emit q->selectionChanged();
#ifndef QT_NO_ACCESSIBILITY
      if (q->parent() && q->parent()->isWidgetType()) {
         QAccessibleTextSelectionEvent ev(q->parent(), cursor.anchor(), cursor.position());
         QAccessible::updateAccessibility(&ev);
      }
#endif
   }

   emit q->microFocusChanged();
   lastSelectionPosition = cursor.position();
   lastSelectionAnchor = cursor.anchor();
}

void QTextControlPrivate::_q_updateCurrentCharFormatAndSelection()
{
   updateCurrentCharFormat();
   selectionChanged();
}

#ifndef QT_NO_CLIPBOARD
void QTextControlPrivate::setClipboardSelection()
{
   QClipboard *clipboard = QApplication::clipboard();
   if (!cursor.hasSelection() || !clipboard->supportsSelection()) {
      return;
   }
   Q_Q(QTextControl);
   QMimeData *data = q->createMimeDataFromSelection();
   clipboard->setMimeData(data, QClipboard::Selection);
}
#endif

void QTextControlPrivate::_q_emitCursorPosChanged(const QTextCursor &someCursor)
{
   Q_Q(QTextControl);
   if (someCursor.isCopyOf(cursor)) {
      emit q->cursorPositionChanged();
      emit q->microFocusChanged();
   }
}

void QTextControlPrivate::_q_contentsChanged(int from, int charsRemoved, int charsAdded)
{
#ifndef QT_NO_ACCESSIBILITY
   Q_Q(QTextControl);

   if (QAccessible::isActive() && q->parent() && q->parent()->isWidgetType()) {
      QTextCursor tmp(doc);
      tmp.setPosition(from);

      // when setting a new text document the length is off
      // QTBUG-32583 - characterCount is off by 1 requires the -1
      tmp.setPosition(qMin(doc->characterCount() - 1, from + charsAdded), QTextCursor::KeepAnchor);
      QString newText = tmp.selectedText();

      // always report the right number of removed chars, but in lack of the real string use spaces
      QString oldText = QString(charsRemoved, QLatin1Char(' '));

      QAccessibleEvent *ev = 0;
      if (charsRemoved == 0) {
         ev = new QAccessibleTextInsertEvent(q->parent(), from, newText);
      } else if (charsAdded == 0) {
         ev = new QAccessibleTextRemoveEvent(q->parent(), from, oldText);
      } else {
         ev = new QAccessibleTextUpdateEvent(q->parent(), from, oldText, newText);
      }

      QAccessible::updateAccessibility(ev);
      delete ev;
   }

#endif
}

void QTextControlPrivate::_q_documentLayoutChanged()
{
   Q_Q(QTextControl);

   QAbstractTextDocumentLayout *layout = doc->documentLayout();

   QObject::connect(layout, &QAbstractTextDocumentLayout::update,              q, &QTextControl::updateRequest);
   QObject::connect(layout, &QAbstractTextDocumentLayout::updateBlock,         q, &QTextControl::_q_updateBlock);
   QObject::connect(layout, &QAbstractTextDocumentLayout::documentSizeChanged, q, &QTextControl::documentSizeChanged);

}

void QTextControlPrivate::setBlinkingCursorEnabled(bool enable)
{
   Q_Q(QTextControl);

   if (enable && QApplication::cursorFlashTime() > 0) {
      cursorBlinkTimer.start(QApplication::cursorFlashTime() / 2, q);
   } else {
      cursorBlinkTimer.stop();
   }

   cursorOn = enable;

   repaintCursor();
}

void QTextControlPrivate::extendWordwiseSelection(int suggestedNewPosition, qreal mouseXPosition)
{
   Q_Q(QTextControl);

   // if inside the initial selected word keep that
   if (suggestedNewPosition >= selectedWordOnDoubleClick.selectionStart()
      && suggestedNewPosition <= selectedWordOnDoubleClick.selectionEnd()) {
      q->setTextCursor(selectedWordOnDoubleClick);
      return;
   }

   QTextCursor curs = selectedWordOnDoubleClick;
   curs.setPosition(suggestedNewPosition, QTextCursor::KeepAnchor);

   if (!curs.movePosition(QTextCursor::StartOfWord)) {
      return;
   }
   const int wordStartPos = curs.position();

   const int blockPos = curs.block().position();
   const QPointF blockCoordinates = q->blockBoundingRect(curs.block()).topLeft();

   QTextLine line = currentTextLine(curs);
   if (!line.isValid()) {
      return;
   }

   const qreal wordStartX = line.cursorToX(curs.position() - blockPos) + blockCoordinates.x();

   if (!curs.movePosition(QTextCursor::EndOfWord)) {
      return;
   }
   const int wordEndPos = curs.position();

   const QTextLine otherLine = currentTextLine(curs);
   if (otherLine.textStart() != line.textStart()
      || wordEndPos == wordStartPos) {
      return;
   }

   const qreal wordEndX = line.cursorToX(curs.position() - blockPos) + blockCoordinates.x();

   if (!wordSelectionEnabled && (mouseXPosition < wordStartX || mouseXPosition > wordEndX)) {
      return;
   }

   if (wordSelectionEnabled) {
      if (suggestedNewPosition < selectedWordOnDoubleClick.position()) {
         cursor.setPosition(selectedWordOnDoubleClick.selectionEnd());
         setCursorPosition(wordStartPos, QTextCursor::KeepAnchor);
      } else {
         cursor.setPosition(selectedWordOnDoubleClick.selectionStart());
         setCursorPosition(wordEndPos, QTextCursor::KeepAnchor);
      }
   } else {
      // keep the already selected word even when moving to the left
      // (#39164)
      if (suggestedNewPosition < selectedWordOnDoubleClick.position()) {
         cursor.setPosition(selectedWordOnDoubleClick.selectionEnd());
      } else {
         cursor.setPosition(selectedWordOnDoubleClick.selectionStart());
      }

      const qreal differenceToStart = mouseXPosition - wordStartX;
      const qreal differenceToEnd = wordEndX - mouseXPosition;

      if (differenceToStart < differenceToEnd) {
         setCursorPosition(wordStartPos, QTextCursor::KeepAnchor);
      } else {
         setCursorPosition(wordEndPos, QTextCursor::KeepAnchor);
      }
   }

   if (interactionFlags & Qt::TextSelectableByMouse) {
#ifndef QT_NO_CLIPBOARD
      setClipboardSelection();
#endif
      selectionChanged(true);
   }
}

void QTextControlPrivate::extendBlockwiseSelection(int suggestedNewPosition)
{
   Q_Q(QTextControl);

   // if inside the initial selected line keep that
   if (suggestedNewPosition >= selectedBlockOnTrippleClick.selectionStart()
      && suggestedNewPosition <= selectedBlockOnTrippleClick.selectionEnd()) {
      q->setTextCursor(selectedBlockOnTrippleClick);
      return;
   }

   if (suggestedNewPosition < selectedBlockOnTrippleClick.position()) {
      cursor.setPosition(selectedBlockOnTrippleClick.selectionEnd());
      cursor.setPosition(suggestedNewPosition, QTextCursor::KeepAnchor);
      cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
   } else {
      cursor.setPosition(selectedBlockOnTrippleClick.selectionStart());
      cursor.setPosition(suggestedNewPosition, QTextCursor::KeepAnchor);
      cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
      cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
   }

   if (interactionFlags & Qt::TextSelectableByMouse) {
#ifndef QT_NO_CLIPBOARD
      setClipboardSelection();
#endif
      selectionChanged(true);
   }
}

void QTextControlPrivate::_q_deleteSelected()
{
   if (!(interactionFlags & Qt::TextEditable) || !cursor.hasSelection()) {
      return;
   }
   cursor.removeSelectedText();
}

void QTextControl::undo()
{
   Q_D(QTextControl);
   d->repaintSelection();
   const int oldCursorPos = d->cursor.position();
   d->doc->undo(&d->cursor);
   if (d->cursor.position() != oldCursorPos) {
      emit cursorPositionChanged();
   }
   emit microFocusChanged();
   ensureCursorVisible();
}

void QTextControl::redo()
{
   Q_D(QTextControl);

   d->repaintSelection();
   const int oldCursorPos = d->cursor.position();
   d->doc->redo(&d->cursor);

   if (d->cursor.position() != oldCursorPos) {
      emit cursorPositionChanged();
   }

   emit microFocusChanged();
   ensureCursorVisible();
}

QTextControl::QTextControl(QObject *parent)
   : QInputControl(QInputControl::TextEdit, parent), d_ptr(new QTextControlPrivate)
{
   d_ptr->q_ptr = this;

   Q_D(QTextControl);
   d->init();
}

QTextControl::QTextControl(const QString &text, QObject *parent)
   : QInputControl(QInputControl::TextEdit, parent), d_ptr(new QTextControlPrivate)
{
   d_ptr->q_ptr = this;

   Q_D(QTextControl);
   d->init(Qt::RichText, text);
}

QTextControl::QTextControl(QTextDocument *doc, QObject *parent)
   : QInputControl(QInputControl::TextEdit, parent), d_ptr(new QTextControlPrivate)
{
   d_ptr->q_ptr = this;

   Q_D(QTextControl);
   d->init(Qt::RichText, QString(), doc);
}

QTextControl::~QTextControl()
{
}

void QTextControl::setDocument(QTextDocument *document)
{
   Q_D(QTextControl);

   if (d->doc == document) {
      return;
   }

   d->doc->disconnect(this);
   d->doc->documentLayout()->disconnect(this);
   d->doc->documentLayout()->setPaintDevice(0);

   if (d->doc->parent() == this) {
      delete d->doc;
   }

   d->doc = 0;
   d->setContent(Qt::RichText, QString(), document);
}

QTextDocument *QTextControl::document() const
{
   Q_D(const QTextControl);
   return d->doc;
}

void QTextControl::setTextCursor(const QTextCursor &cursor)
{
   Q_D(QTextControl);

   d->cursorIsFocusIndicator = false;
   const bool posChanged = cursor.position() != d->cursor.position();
   const QTextCursor oldSelection = d->cursor;
   d->cursor = cursor;

   d->cursorOn = d->hasFocus && (d->interactionFlags & (Qt::TextSelectableByKeyboard | Qt::TextEditable));

   d->_q_updateCurrentCharFormatAndSelection();
   ensureCursorVisible();
   d->repaintOldAndNewSelection(oldSelection);
   if (posChanged) {
      emit cursorPositionChanged();
   }
}

QTextCursor QTextControl::textCursor() const
{
   Q_D(const QTextControl);
   return d->cursor;
}

#ifndef QT_NO_CLIPBOARD

void QTextControl::cut()
{
   Q_D(QTextControl);
   if (!(d->interactionFlags & Qt::TextEditable) || !d->cursor.hasSelection()) {
      return;
   }
   copy();
   d->cursor.removeSelectedText();
}

void QTextControl::copy()
{
   Q_D(QTextControl);

   if (!d->cursor.hasSelection()) {
      return;
   }

   QMimeData *data = createMimeDataFromSelection();
   QApplication::clipboard()->setMimeData(data);
}

void QTextControl::paste(QClipboard::Mode mode)
{
   const QMimeData *md = QApplication::clipboard()->mimeData(mode);

   if (md) {
      insertFromMimeData(md);
   }
}
#endif

void QTextControl::clear()
{
   Q_D(QTextControl);
   // clears and sets empty content
   d->extraSelections.clear();
   d->setContent();
}

void QTextControl::selectAll()
{
   Q_D(QTextControl);

   const int selectionLength = qAbs(d->cursor.position() - d->cursor.anchor());
   d->cursor.select(QTextCursor::Document);
   d->selectionChanged(selectionLength != qAbs(d->cursor.position() - d->cursor.anchor()));
   d->cursorIsFocusIndicator = false;
   emit updateRequest();
}

void QTextControl::processEvent(QEvent *e, const QPointF &coordinateOffset, QWidget *contextWidget)
{
   QMatrix m;
   m.translate(coordinateOffset.x(), coordinateOffset.y());
   processEvent(e, m, contextWidget);
}

void QTextControl::processEvent(QEvent *e, const QMatrix &matrix, QWidget *contextWidget)
{
   Q_D(QTextControl);
   if (d->interactionFlags == Qt::NoTextInteraction) {
      e->ignore();
      return;
   }

   d->contextWidget = contextWidget;

   if (!d->contextWidget) {
      switch (e->type()) {
#ifndef QT_NO_GRAPHICSVIEW
         case QEvent::GraphicsSceneMouseMove:
         case QEvent::GraphicsSceneMousePress:
         case QEvent::GraphicsSceneMouseRelease:
         case QEvent::GraphicsSceneMouseDoubleClick:
         case QEvent::GraphicsSceneContextMenu:
         case QEvent::GraphicsSceneHoverEnter:
         case QEvent::GraphicsSceneHoverMove:
         case QEvent::GraphicsSceneHoverLeave:
         case QEvent::GraphicsSceneHelp:
         case QEvent::GraphicsSceneDragEnter:
         case QEvent::GraphicsSceneDragMove:
         case QEvent::GraphicsSceneDragLeave:
         case QEvent::GraphicsSceneDrop: {
            QGraphicsSceneEvent *ev = static_cast<QGraphicsSceneEvent *>(e);
            d->contextWidget = ev->widget();
            break;
         }
#endif // QT_NO_GRAPHICSVIEW
         default:
            break;
      };
   }

   switch (e->type()) {
      case QEvent::KeyPress:
         d->keyPressEvent(static_cast<QKeyEvent *>(e));
         break;
      case QEvent::MouseButtonPress: {
         QMouseEvent *ev = static_cast<QMouseEvent *>(e);
         d->mousePressEvent(ev, ev->button(), matrix.map(ev->pos()), ev->modifiers(),
            ev->buttons(), ev->globalPos());
         break;
      }
      case QEvent::MouseMove: {
         QMouseEvent *ev = static_cast<QMouseEvent *>(e);
         d->mouseMoveEvent(ev, ev->button(), matrix.map(ev->pos()), ev->modifiers(),
            ev->buttons(), ev->globalPos());
         break;
      }
      case QEvent::MouseButtonRelease: {
         QMouseEvent *ev = static_cast<QMouseEvent *>(e);
         d->mouseReleaseEvent(ev, ev->button(), matrix.map(ev->pos()), ev->modifiers(),
            ev->buttons(), ev->globalPos());
         break;
      }
      case QEvent::MouseButtonDblClick: {
         QMouseEvent *ev = static_cast<QMouseEvent *>(e);
         d->mouseDoubleClickEvent(ev, ev->button(), matrix.map(ev->pos()), ev->modifiers(),
            ev->buttons(), ev->globalPos());
         break;
      }
      case QEvent::InputMethod:
         d->inputMethodEvent(static_cast<QInputMethodEvent *>(e));
         break;
#ifndef QT_NO_CONTEXTMENU
      case QEvent::ContextMenu: {
         QContextMenuEvent *ev = static_cast<QContextMenuEvent *>(e);
         d->contextMenuEvent(ev->globalPos(), matrix.map(ev->pos()), contextWidget);
         break;
      }
#endif // QT_NO_CONTEXTMENU
      case QEvent::FocusIn:
      case QEvent::FocusOut:
         d->focusEvent(static_cast<QFocusEvent *>(e));
         break;

      case QEvent::EnabledChange:
         d->isEnabled = e->isAccepted();
         break;

#ifndef QT_NO_TOOLTIP
      case QEvent::ToolTip: {
         QHelpEvent *ev = static_cast<QHelpEvent *>(e);
         d->showToolTip(ev->globalPos(), matrix.map(ev->pos()), contextWidget);
         break;
      }
#endif // QT_NO_TOOLTIP

#ifndef QT_NO_DRAGANDDROP
      case QEvent::DragEnter: {
         QDragEnterEvent *ev = static_cast<QDragEnterEvent *>(e);
         if (d->dragEnterEvent(e, ev->mimeData())) {
            ev->acceptProposedAction();
         }
         break;
      }
      case QEvent::DragLeave:
         d->dragLeaveEvent();
         break;
      case QEvent::DragMove: {
         QDragMoveEvent *ev = static_cast<QDragMoveEvent *>(e);
         if (d->dragMoveEvent(e, ev->mimeData(), matrix.map(ev->pos()))) {
            ev->acceptProposedAction();
         }
         break;
      }
      case QEvent::Drop: {
         QDropEvent *ev = static_cast<QDropEvent *>(e);
         if (d->dropEvent(ev->mimeData(), matrix.map(ev->pos()), ev->dropAction(), ev->source())) {
            ev->acceptProposedAction();
         }
         break;
      }
#endif

#ifndef QT_NO_GRAPHICSVIEW
      case QEvent::GraphicsSceneMousePress: {
         QGraphicsSceneMouseEvent *ev = static_cast<QGraphicsSceneMouseEvent *>(e);
         d->mousePressEvent(ev, ev->button(), matrix.map(ev->pos()), ev->modifiers(), ev->buttons(),
            ev->screenPos());
         break;
      }
      case QEvent::GraphicsSceneMouseMove: {
         QGraphicsSceneMouseEvent *ev = static_cast<QGraphicsSceneMouseEvent *>(e);
         d->mouseMoveEvent(ev, ev->button(), matrix.map(ev->pos()), ev->modifiers(), ev->buttons(),
            ev->screenPos());
         break;
      }
      case QEvent::GraphicsSceneMouseRelease: {
         QGraphicsSceneMouseEvent *ev = static_cast<QGraphicsSceneMouseEvent *>(e);
         d->mouseReleaseEvent(ev, ev->button(), matrix.map(ev->pos()), ev->modifiers(), ev->buttons(),
            ev->screenPos());
         break;
      }
      case QEvent::GraphicsSceneMouseDoubleClick: {
         QGraphicsSceneMouseEvent *ev = static_cast<QGraphicsSceneMouseEvent *>(e);
         d->mouseDoubleClickEvent(ev, ev->button(), matrix.map(ev->pos()), ev->modifiers(), ev->buttons(),
            ev->screenPos());
         break;
      }
      case QEvent::GraphicsSceneContextMenu: {
         QGraphicsSceneContextMenuEvent *ev = static_cast<QGraphicsSceneContextMenuEvent *>(e);
         d->contextMenuEvent(ev->screenPos(), matrix.map(ev->pos()), contextWidget);
         break;
      }

      case QEvent::GraphicsSceneHoverMove: {
         QGraphicsSceneHoverEvent *ev = static_cast<QGraphicsSceneHoverEvent *>(e);
         d->mouseMoveEvent(ev, Qt::NoButton, matrix.map(ev->pos()), ev->modifiers(), Qt::NoButton,
            ev->screenPos());
         break;
      }

      case QEvent::GraphicsSceneDragEnter: {
         QGraphicsSceneDragDropEvent *ev = static_cast<QGraphicsSceneDragDropEvent *>(e);
         if (d->dragEnterEvent(e, ev->mimeData())) {
            ev->acceptProposedAction();
         }
         break;
      }
      case QEvent::GraphicsSceneDragLeave:
         d->dragLeaveEvent();
         break;
      case QEvent::GraphicsSceneDragMove: {
         QGraphicsSceneDragDropEvent *ev = static_cast<QGraphicsSceneDragDropEvent *>(e);
         if (d->dragMoveEvent(e, ev->mimeData(), matrix.map(ev->pos()))) {
            ev->acceptProposedAction();
         }
         break;
      }
      case QEvent::GraphicsSceneDrop: {
         QGraphicsSceneDragDropEvent *ev = static_cast<QGraphicsSceneDragDropEvent *>(e);
         if (d->dropEvent(ev->mimeData(), matrix.map(ev->pos()), ev->dropAction(), ev->source())) {
            ev->accept();
         }
         break;
      }
#endif // QT_NO_GRAPHICSVIEW
#ifdef QT_KEYPAD_NAVIGATION
      case QEvent::EnterEditFocus:
      case QEvent::LeaveEditFocus:
         if (QApplication::keypadNavigationEnabled()) {
            d->editFocusEvent(e);
         }
         break;
#endif
      case QEvent::ShortcutOverride:
         if (d->interactionFlags & Qt::TextEditable) {
            QKeyEvent *ke = static_cast<QKeyEvent *>(e);
            if (ke->modifiers() == Qt::NoModifier
               || ke->modifiers() == Qt::ShiftModifier
               || ke->modifiers() == Qt::KeypadModifier) {
               if (ke->key() < Qt::Key_Escape) {
                  ke->accept();
               } else {
                  switch (ke->key()) {
                     case Qt::Key_Return:
                     case Qt::Key_Enter:
                     case Qt::Key_Delete:
                     case Qt::Key_Home:
                     case Qt::Key_End:
                     case Qt::Key_Backspace:
                     case Qt::Key_Left:
                     case Qt::Key_Right:
                     case Qt::Key_Up:
                     case Qt::Key_Down:
                     case Qt::Key_Tab:
                        ke->accept();
                     default:
                        break;
                  }
               }
#ifndef QT_NO_SHORTCUT
            } else if (ke == QKeySequence::Copy
               || ke == QKeySequence::Paste
               || ke == QKeySequence::Cut
               || ke == QKeySequence::Redo
               || ke == QKeySequence::Undo
               || ke == QKeySequence::MoveToNextWord
               || ke == QKeySequence::MoveToPreviousWord
               || ke == QKeySequence::MoveToStartOfDocument
               || ke == QKeySequence::MoveToEndOfDocument
               || ke == QKeySequence::SelectNextWord
               || ke == QKeySequence::SelectPreviousWord
               || ke == QKeySequence::SelectStartOfLine
               || ke == QKeySequence::SelectEndOfLine
               || ke == QKeySequence::SelectStartOfBlock
               || ke == QKeySequence::SelectEndOfBlock
               || ke == QKeySequence::SelectStartOfDocument
               || ke == QKeySequence::SelectEndOfDocument
               || ke == QKeySequence::SelectAll
            ) {
               ke->accept();
#endif
            }
         }
         break;
      default:
         break;
   }
}

bool QTextControl::event(QEvent *e)
{
   return QObject::event(e);
}

void QTextControl::timerEvent(QTimerEvent *e)
{
   Q_D(QTextControl);
   if (e->timerId() == d->cursorBlinkTimer.timerId()) {
      d->cursorOn = !d->cursorOn;

      if (d->cursor.hasSelection())
         d->cursorOn &= (QApplication::style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected)
               != 0);

      d->repaintCursor();
   } else if (e->timerId() == d->trippleClickTimer.timerId()) {
      d->trippleClickTimer.stop();
   }
}

void QTextControl::setPlainText(const QString &text)
{
   Q_D(QTextControl);
   d->setContent(Qt::PlainText, text);
}

void QTextControl::setHtml(const QString &text)
{
   Q_D(QTextControl);
   d->setContent(Qt::RichText, text);
}

void QTextControlPrivate::keyPressEvent(QKeyEvent *e)
{
   Q_Q(QTextControl);
#ifndef QT_NO_SHORTCUT
   if (e == QKeySequence::SelectAll) {
      e->accept();
      q->selectAll();
      return;
   }
#ifndef QT_NO_CLIPBOARD
   else if (e == QKeySequence::Copy) {
      e->accept();
      q->copy();
      return;
   }
#endif
#endif // QT_NO_SHORTCUT

   if (interactionFlags & Qt::TextSelectableByKeyboard
      && cursorMoveKeyEvent(e)) {
      goto accept;
   }

   if (interactionFlags & Qt::LinksAccessibleByKeyboard) {
      if ((e->key() == Qt::Key_Return
            || e->key() == Qt::Key_Enter
#ifdef QT_KEYPAD_NAVIGATION
            || e->key() == Qt::Key_Select
#endif
         )
         && cursor.hasSelection()) {

         e->accept();
         activateLinkUnderCursor();
         return;
      }
   }

   if (!(interactionFlags & Qt::TextEditable)) {
      e->ignore();
      return;
   }

   if (e->key() == Qt::Key_Direction_L || e->key() == Qt::Key_Direction_R) {
      QTextBlockFormat fmt;
      fmt.setLayoutDirection((e->key() == Qt::Key_Direction_L) ? Qt::LeftToRight : Qt::RightToLeft);
      cursor.mergeBlockFormat(fmt);
      goto accept;
   }

   // schedule a repaint of the region of the cursor, as when we move it we
   // want to make sure the old cursor disappears (not noticeable when moving
   // only a few pixels but noticeable when jumping between cells in tables for
   // example)
   repaintSelection();

   if (e->key() == Qt::Key_Backspace && !(e->modifiers() & ~Qt::ShiftModifier)) {
      QTextBlockFormat blockFmt = cursor.blockFormat();
      QTextList *list = cursor.currentList();
      if (list && cursor.atBlockStart() && !cursor.hasSelection()) {
         list->remove(cursor.block());
      } else if (cursor.atBlockStart() && blockFmt.indent() > 0) {
         blockFmt.setIndent(blockFmt.indent() - 1);
         cursor.setBlockFormat(blockFmt);
      } else {
         QTextCursor localCursor = cursor;
         localCursor.deletePreviousChar();
      }
      goto accept;
   }
#ifndef QT_NO_SHORTCUT
   else if (e == QKeySequence::InsertParagraphSeparator) {
      cursor.insertBlock();
      e->accept();
      goto accept;
   } else if (e == QKeySequence::InsertLineSeparator) {
      cursor.insertText(QString(QChar::LineSeparator));
      e->accept();
      goto accept;
   }
#endif
   if (false) {
   }
#ifndef QT_NO_SHORTCUT
   else if (e == QKeySequence::Undo) {
      q->undo();
   } else if (e == QKeySequence::Redo) {
      q->redo();
   }
#ifndef QT_NO_CLIPBOARD
   else if (e == QKeySequence::Cut) {
      q->cut();
   } else if (e == QKeySequence::Paste) {
      QClipboard::Mode mode = QClipboard::Clipboard;

      if (QGuiApplication::clipboard()->supportsSelection()) {
         if (e->modifiers() == (Qt::CTRL | Qt::SHIFT) && e->key() == Qt::Key_Insert) {
            mode = QClipboard::Selection;
         }
      }

      q->paste(mode);
   }
#endif
   else if (e == QKeySequence::Delete) {
      QTextCursor localCursor = cursor;
      localCursor.deleteChar();

   } else if (e == QKeySequence::Backspace) {
      QTextCursor localCursor = cursor;
      localCursor.deletePreviousChar();

   } else if (e == QKeySequence::DeleteEndOfWord) {
      if (!cursor.hasSelection()) {
         cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
      }
      cursor.removeSelectedText();

   } else if (e == QKeySequence::DeleteStartOfWord) {
      if (!cursor.hasSelection()) {
         cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
      }
      cursor.removeSelectedText();

   } else if (e == QKeySequence::DeleteEndOfLine) {
      QTextBlock block = cursor.block();

      if (cursor.position() == block.position() + block.length() - 2) {
         cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
      } else {
         cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
      }
      cursor.removeSelectedText();
   }
#endif // QT_NO_SHORTCUT

   else {
      goto process;
   }
   goto accept;

process: {
      if (q->isAcceptableInput(e)) {
         if (overwriteMode
            // no need to call deleteChar() if we have a selection, insertText
            // does it already
            && !cursor.hasSelection()
            && !cursor.atBlockEnd()) {

            cursor.deleteChar();
         }

         cursor.insertText(e->text());
         selectionChanged();

      } else {
         e->ignore();
         return;
      }
   }

accept:
   e->accept();
   cursorOn = true;

   q->ensureCursorVisible();

   updateCurrentCharFormat();
}

QVariant QTextControl::loadResource(int type, const QUrl &name)
{
#ifdef QT_NO_TEXTEDIT
   Q_UNUSED(type);
   Q_UNUSED(name);
#else
   if (QTextEdit *textEdit = qobject_cast<QTextEdit *>(parent())) {
      QUrl resolvedName = textEdit->d_func()->resolveUrl(name);
      return textEdit->loadResource(type, resolvedName);
   }
#endif
   return QVariant();
}

void QTextControlPrivate::_q_updateBlock(const QTextBlock &block)
{
   Q_Q(QTextControl);

   QRectF br = q->blockBoundingRect(block);
   br.setRight(qreal(INT_MAX)); // the block might have shrunk
   emit q->updateRequest(br);
}

QRectF QTextControlPrivate::rectForPosition(int position) const
{
   Q_Q(const QTextControl);

   const QTextBlock block = doc->findBlock(position);
   if (!block.isValid()) {
      return QRectF();
   }

   const QAbstractTextDocumentLayout *docLayout = doc->documentLayout();
   const QTextLayout *layout = block.layout();
   const QPointF layoutPos = q->blockBoundingRect(block).topLeft();
   int relativePos = position - block.position();

   if (preeditCursor != 0) {
      int preeditPos = layout->preeditAreaPosition();
      if (relativePos == preeditPos) {
         relativePos += preeditCursor;
      } else if (relativePos > preeditPos) {
         relativePos += layout->preeditAreaText().length();
      }
   }

   QTextLine line = layout->lineForTextPosition(relativePos);

   int cursorWidth;
   {
      bool ok = false;
#ifndef QT_NO_PROPERTIES
      cursorWidth = docLayout->property("cursorWidth").toInt(&ok);
#endif
      if (!ok) {
         cursorWidth = 1;
      }
   }

   QRectF r;

   if (line.isValid()) {
      qreal x = line.cursorToX(relativePos);
      qreal w = 0;
      if (overwriteMode) {
         if (relativePos < line.textLength() - line.textStart()) {
            w = line.cursorToX(relativePos + 1) - x;
         } else {
            w = QFontMetrics(block.layout()->font()).width(QLatin1Char(' '));   // in sync with QTextLine::draw()
         }
      }
      r = QRectF(layoutPos.x() + x, layoutPos.y() + line.y(),
            cursorWidth + w, line.height());
   } else {
      r = QRectF(layoutPos.x(), layoutPos.y(), cursorWidth, 10); // #### correct height
   }

   return r;
}

namespace {

struct QTextFrameComparator {


   bool operator()(QTextFrame *frame, int position) {
      return frame->firstPosition() < position;
   }
   bool operator()(int position, QTextFrame *frame) {
      return position < frame->firstPosition();
   }
};

} // namespace


static QRectF boundingRectOfFloatsInSelection(const QTextCursor &cursor)
{
   QRectF r;
   QTextFrame *frame = cursor.currentFrame();
   const QList<QTextFrame *> children = frame->childFrames();

   const QList<QTextFrame *>::const_iterator firstFrame = std::lower_bound(children.constBegin(), children.constEnd(),
         cursor.selectionStart(), QTextFrameComparator());

   const QList<QTextFrame *>::const_iterator lastFrame = std::upper_bound(children.constBegin(), children.constEnd(),
         cursor.selectionEnd(), QTextFrameComparator());

   for (QList<QTextFrame *>::const_iterator it = firstFrame; it != lastFrame; ++it) {
      if ((*it)->frameFormat().position() != QTextFrameFormat::InFlow) {
         r |= frame->document()->documentLayout()->frameBoundingRect(*it);
      }
   }
   return r;
}

QRectF QTextControl::selectionRect(const QTextCursor &cursor) const
{
   Q_D(const QTextControl);

   QRectF r = d->rectForPosition(cursor.selectionStart());

   if (cursor.hasComplexSelection() && cursor.currentTable()) {
      QTextTable *table = cursor.currentTable();

      r = d->doc->documentLayout()->frameBoundingRect(table);
      /*
      int firstRow, numRows, firstColumn, numColumns;
      cursor.selectedTableCells(&firstRow, &numRows, &firstColumn, &numColumns);

      const QTextTableCell firstCell = table->cellAt(firstRow, firstColumn);
      const QTextTableCell lastCell = table->cellAt(firstRow + numRows - 1, firstColumn + numColumns - 1);

      const QAbstractTextDocumentLayout * const layout = doc->documentLayout();

      QRectF tableSelRect = layout->blockBoundingRect(firstCell.firstCursorPosition().block());

      for (int col = firstColumn; col < firstColumn + numColumns; ++col) {
          const QTextTableCell cell = table->cellAt(firstRow, col);
          const qreal y = layout->blockBoundingRect(cell.firstCursorPosition().block()).top();

          tableSelRect.setTop(qMin(tableSelRect.top(), y));
      }

      for (int row = firstRow; row < firstRow + numRows; ++row) {
          const QTextTableCell cell = table->cellAt(row, firstColumn);
          const qreal x = layout->blockBoundingRect(cell.firstCursorPosition().block()).left();

          tableSelRect.setLeft(qMin(tableSelRect.left(), x));
      }

      for (int col = firstColumn; col < firstColumn + numColumns; ++col) {
          const QTextTableCell cell = table->cellAt(firstRow + numRows - 1, col);
          const qreal y = layout->blockBoundingRect(cell.lastCursorPosition().block()).bottom();

          tableSelRect.setBottom(qMax(tableSelRect.bottom(), y));
      }

      for (int row = firstRow; row < firstRow + numRows; ++row) {
          const QTextTableCell cell = table->cellAt(row, firstColumn + numColumns - 1);
          const qreal x = layout->blockBoundingRect(cell.lastCursorPosition().block()).right();

          tableSelRect.setRight(qMax(tableSelRect.right(), x));
      }

      r = tableSelRect.toRect();
      */

   } else if (cursor.hasSelection()) {
      const int position = cursor.selectionStart();
      const int anchor = cursor.selectionEnd();
      const QTextBlock posBlock = d->doc->findBlock(position);
      const QTextBlock anchorBlock = d->doc->findBlock(anchor);

      if (posBlock == anchorBlock && posBlock.isValid() && posBlock.layout()->lineCount()) {
         const QTextLine posLine = posBlock.layout()->lineForTextPosition(position - posBlock.position());
         const QTextLine anchorLine = anchorBlock.layout()->lineForTextPosition(anchor - anchorBlock.position());

         const int firstLine = qMin(posLine.lineNumber(), anchorLine.lineNumber());
         const int lastLine = qMax(posLine.lineNumber(), anchorLine.lineNumber());
         const QTextLayout *layout = posBlock.layout();
         r = QRectF();
         for (int i = firstLine; i <= lastLine; ++i) {
            r |= layout->lineAt(i).rect();
            r |= layout->lineAt(i).naturalTextRect(); // might be bigger in the case of wrap not enabled
         }
         r.translate(blockBoundingRect(posBlock).topLeft());
      } else {
         QRectF anchorRect = d->rectForPosition(cursor.selectionEnd());

         r |= anchorRect;
         r |= boundingRectOfFloatsInSelection(cursor);
         QRectF frameRect(d->doc->documentLayout()->frameBoundingRect(cursor.currentFrame()));
         r.setLeft(frameRect.left());
         r.setRight(frameRect.right());
      }

      if (r.isValid()) {
         r.adjust(-1, -1, 1, 1);
      }
   }

   return r;
}

QRectF QTextControl::selectionRect() const
{
   Q_D(const QTextControl);
   return selectionRect(d->cursor);
}

void QTextControlPrivate::mousePressEvent(QEvent *e, Qt::MouseButton button, const QPointF &pos,
   Qt::KeyboardModifiers modifiers,
   Qt::MouseButtons buttons, const QPoint &globalPos)
{
   Q_Q(QTextControl);
   mousePressPos = pos.toPoint();
#ifndef QT_NO_DRAGANDDROP
   mightStartDrag = false;
#endif

   if (sendMouseEventToInputContext(
         e, QEvent::MouseButtonPress, button, pos, modifiers, buttons, globalPos)) {
      return;
   }

   if (interactionFlags & Qt::LinksAccessibleByMouse) {
      anchorOnMousePress = q->anchorAt(pos);

      if (cursorIsFocusIndicator) {
         cursorIsFocusIndicator = false;
         repaintSelection();
         cursor.clearSelection();
      }
   }
   if (!(button & Qt::LeftButton) ||
      !((interactionFlags & Qt::TextSelectableByMouse) || (interactionFlags & Qt::TextEditable))) {
      e->ignore();
      return;
   }

   cursorIsFocusIndicator = false;
   const QTextCursor oldSelection = cursor;
   const int oldCursorPos = cursor.position();

   mousePressed = (interactionFlags & Qt::TextSelectableByMouse);
   commitPreedit();


   if (trippleClickTimer.isActive()
      && ((pos - trippleClickPoint).toPoint().manhattanLength() < QApplication::startDragDistance())) {

      cursor.movePosition(QTextCursor::StartOfBlock);
      cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
      cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
      selectedBlockOnTrippleClick = cursor;

      anchorOnMousePress = QString();

      trippleClickTimer.stop();
   } else {
      int cursorPos = q->hitTest(pos, Qt::FuzzyHit);
      if (cursorPos == -1) {
         e->ignore();
         return;
      }

      if (modifiers == Qt::ShiftModifier && (interactionFlags & Qt::TextSelectableByMouse)) {
         if (wordSelectionEnabled && !selectedWordOnDoubleClick.hasSelection()) {
            selectedWordOnDoubleClick = cursor;
            selectedWordOnDoubleClick.select(QTextCursor::WordUnderCursor);
         }

         if (selectedBlockOnTrippleClick.hasSelection()) {
            extendBlockwiseSelection(cursorPos);
         } else if (selectedWordOnDoubleClick.hasSelection()) {
            extendWordwiseSelection(cursorPos, pos.x());
         } else if (!wordSelectionEnabled) {
            setCursorPosition(cursorPos, QTextCursor::KeepAnchor);
         }
      } else {

         if (dragEnabled
            && cursor.hasSelection()
            && !cursorIsFocusIndicator
            && cursorPos >= cursor.selectionStart()
            && cursorPos <= cursor.selectionEnd()
            && q->hitTest(pos, Qt::ExactHit) != -1) {

#ifndef QT_NO_DRAGANDDROP
            mightStartDrag = true;
#endif
            return;
         }

         setCursorPosition(cursorPos);
      }
   }

   if (interactionFlags & Qt::TextEditable) {
      q->ensureCursorVisible();
      if (cursor.position() != oldCursorPos) {
         emit q->cursorPositionChanged();
      }
      _q_updateCurrentCharFormatAndSelection();
   } else {
      if (cursor.position() != oldCursorPos) {
         emit q->cursorPositionChanged();
         emit q->microFocusChanged();
      }
      selectionChanged();
   }
   repaintOldAndNewSelection(oldSelection);
   hadSelectionOnMousePress = cursor.hasSelection();
}

void QTextControlPrivate::mouseMoveEvent(QEvent *e, Qt::MouseButton button, const QPointF &mousePos,
   Qt::KeyboardModifiers modifiers,
   Qt::MouseButtons buttons, const QPoint &globalPos)
{
   Q_Q(QTextControl);

   if (interactionFlags & Qt::LinksAccessibleByMouse) {
      QString anchor = q->anchorAt(mousePos);
      if (anchor != highlightedAnchor) {
         highlightedAnchor = anchor;
         emit q->linkHovered(anchor);
      }
   }

   if (buttons & Qt::LeftButton) {
      const bool editable = interactionFlags & Qt::TextEditable;

      if (! (mousePressed
            || editable
            || mightStartDrag
            || selectedWordOnDoubleClick.hasSelection()
            || selectedBlockOnTrippleClick.hasSelection())) {
         return;
      }

      const QTextCursor oldSelection = cursor;
      const int oldCursorPos = cursor.position();

      if (mightStartDrag) {
         if ((mousePos.toPoint() - mousePressPos).manhattanLength() > QApplication::startDragDistance()) {
            startDrag();
         }
         return;
      }

      const qreal mouseX = qreal(mousePos.x());

      int newCursorPos = q->hitTest(mousePos, Qt::FuzzyHit);
      if (isPreediting()) {
         // note: oldCursorPos not including preedit
         int selectionStartPos = q->hitTest(mousePressPos, Qt::FuzzyHit);

         if (newCursorPos != selectionStartPos) {
            commitPreedit();
            // commit invalidates positions
            newCursorPos = q->hitTest(mousePos, Qt::FuzzyHit);
            selectionStartPos = q->hitTest(mousePressPos, Qt::FuzzyHit);
            setCursorPosition(selectionStartPos);
         }
      }
      if (newCursorPos == -1) {
         return;
      }

      if (mousePressed && wordSelectionEnabled && !selectedWordOnDoubleClick.hasSelection()) {
         selectedWordOnDoubleClick = cursor;
         selectedWordOnDoubleClick.select(QTextCursor::WordUnderCursor);
      }

      if (selectedBlockOnTrippleClick.hasSelection()) {
         extendBlockwiseSelection(newCursorPos);

      } else if (selectedWordOnDoubleClick.hasSelection()) {
         extendWordwiseSelection(newCursorPos, mouseX);

      }  else if (mousePressed && !isPreediting()) {
         setCursorPosition(newCursorPos, QTextCursor::KeepAnchor);
      }

      if (interactionFlags & Qt::TextEditable) {
         // don't call ensureVisible for the visible cursor to avoid jumping
         // scrollbars. the autoscrolling ensures smooth scrolling if necessary.
         //q->ensureCursorVisible();
         if (cursor.position() != oldCursorPos) {
            emit q->cursorPositionChanged();
         }

         _q_updateCurrentCharFormatAndSelection();

#ifndef QT_NO_IM
         if (contextWidget) {
            QGuiApplication::inputMethod()->update(Qt::ImQueryInput);
         }
#endif

      } else {
         //emit q->visibilityRequest(QRectF(mousePos, QSizeF(1, 1)));
         if (cursor.position() != oldCursorPos) {
            emit q->cursorPositionChanged();
            emit q->microFocusChanged();
         }
      }
      selectionChanged(true);
      repaintOldAndNewSelection(oldSelection);

   }
   sendMouseEventToInputContext(e, QEvent::MouseMove, button, mousePos, modifiers, buttons, globalPos);
}

void QTextControlPrivate::mouseReleaseEvent(QEvent *e, Qt::MouseButton button, const QPointF &pos,
   Qt::KeyboardModifiers modifiers, Qt::MouseButtons buttons, const QPoint &globalPos)
{
   Q_Q(QTextControl);

   const QTextCursor oldSelection = cursor;
   if (sendMouseEventToInputContext(
         e, QEvent::MouseButtonRelease, button, pos, modifiers, buttons, globalPos)) {
      repaintOldAndNewSelection(oldSelection);
      return;
   }

   const int oldCursorPos = cursor.position();

#ifndef QT_NO_DRAGANDDROP
   if (mightStartDrag && (button & Qt::LeftButton)) {
      mousePressed = false;
      setCursorPosition(pos);
      cursor.clearSelection();
      selectionChanged();
   }
#endif
   if (mousePressed) {
      mousePressed = false;

#ifndef QT_NO_CLIPBOARD
      setClipboardSelection();
      selectionChanged(true);

   } else if (button == Qt::MiddleButton && (interactionFlags & Qt::TextEditable)
         && QApplication::clipboard()->supportsSelection()) {

      setCursorPosition(pos);
      const QMimeData *md = QApplication::clipboard()->mimeData(QClipboard::Selection);

      if (md) {
         q->insertFromMimeData(md);
      }
#endif
   }

   repaintOldAndNewSelection(oldSelection);

   if (cursor.position() != oldCursorPos) {
      emit q->cursorPositionChanged();
      emit q->microFocusChanged();
   }

   if (interactionFlags & Qt::LinksAccessibleByMouse) {
      if (!(button & Qt::LeftButton)) {
         return;
      }

      const QString anchor = q->anchorAt(pos);

      if (anchor.isEmpty()) {
         return;
      }

      if (!cursor.hasSelection()
         || (anchor == anchorOnMousePress && hadSelectionOnMousePress)) {

         const int anchorPos = q->hitTest(pos, Qt::ExactHit);
         if (anchorPos != -1) {
            cursor.setPosition(anchorPos);

            QString anchor = anchorOnMousePress;
            anchorOnMousePress = QString();
            activateLinkUnderCursor(anchor);
         }
      }
   }
}

void QTextControlPrivate::mouseDoubleClickEvent(QEvent *e, Qt::MouseButton button, const QPointF &pos,
   Qt::KeyboardModifiers modifiers,
   Qt::MouseButtons buttons, const QPoint &globalPos)
{
   Q_Q(QTextControl);

   if (button == Qt::LeftButton
      && (interactionFlags & Qt::TextSelectableByMouse)) {

#ifndef QT_NO_DRAGANDDROP
      mightStartDrag = false;
#endif

      commitPreedit();
      const QTextCursor oldSelection = cursor;
      setCursorPosition(pos);
      QTextLine line = currentTextLine(cursor);
      bool doEmit = false;
      if (line.isValid() && line.textLength()) {
         cursor.select(QTextCursor::WordUnderCursor);
         doEmit = true;
      }
      repaintOldAndNewSelection(oldSelection);

      cursorIsFocusIndicator = false;
      selectedWordOnDoubleClick = cursor;

      trippleClickPoint = pos;
      trippleClickTimer.start(QApplication::doubleClickInterval(), q);

      if (doEmit) {
         selectionChanged();
#ifndef QT_NO_CLIPBOARD
         setClipboardSelection();
#endif
         emit q->cursorPositionChanged();
      }

   } else if (!sendMouseEventToInputContext(e, QEvent::MouseButtonDblClick, button, pos,
         modifiers, buttons, globalPos)) {
      e->ignore();
   }
}

bool QTextControlPrivate::sendMouseEventToInputContext(
   QEvent *e, QEvent::Type eventType, Qt::MouseButton button, const QPointF &pos,
   Qt::KeyboardModifiers modifiers, Qt::MouseButtons buttons, const QPoint &globalPos)
{
#if ! defined(QT_NO_IM)
   Q_Q(QTextControl);

   if (isPreediting()) {

      QTextLayout *layout = cursor.block().layout();

      int cursorPos = q->hitTest(pos, Qt::FuzzyHit) - cursor.position();

      if (cursorPos < 0 || cursorPos > layout->preeditAreaText().length()) {
         cursorPos = -1;
      }

      // don't send move events outside the preedit area
      if (cursorPos >= 0) {
         if (eventType == QEvent::MouseButtonRelease) {
            QGuiApplication::inputMethod()->invokeAction(QInputMethod::Click, cursorPos);
         }

         e->setAccepted(true);
         return true;
      }
   }
#endif
   return false;
}

void QTextControlPrivate::contextMenuEvent(const QPoint &screenPos, const QPointF &docPos, QWidget *contextWidget)
{
#ifndef QT_NO_CONTEXTMENU

   Q_Q(QTextControl);

   if (!hasFocus) {
      return;
   }
   QMenu *menu = q->createStandardContextMenu(docPos, contextWidget);
   if (!menu) {
      return;
   }
   menu->setAttribute(Qt::WA_DeleteOnClose);
   menu->popup(screenPos);
#endif
}

bool QTextControlPrivate::dragEnterEvent(QEvent *e, const QMimeData *mimeData)
{
   Q_Q(QTextControl);

   if (!(interactionFlags & Qt::TextEditable) || ! q->canInsertFromMimeData(mimeData)) {
      e->ignore();
      return false;
   }

   dndFeedbackCursor = QTextCursor();

   return true; // accept proposed action
}

void QTextControlPrivate::dragLeaveEvent()
{
   Q_Q(QTextControl);

   const QRectF crect = q->cursorRect(dndFeedbackCursor);
   dndFeedbackCursor = QTextCursor();

   if (crect.isValid()) {
      emit q->updateRequest(crect);
   }
}

bool QTextControlPrivate::dragMoveEvent(QEvent *e, const QMimeData *mimeData, const QPointF &pos)
{
   Q_Q(QTextControl);
   if (!(interactionFlags & Qt::TextEditable) || !q->canInsertFromMimeData(mimeData)) {
      e->ignore();
      return false;
   }

   const int cursorPos = q->hitTest(pos, Qt::FuzzyHit);
   if (cursorPos != -1) {
      QRectF crect = q->cursorRect(dndFeedbackCursor);
      if (crect.isValid()) {
         emit q->updateRequest(crect);
      }

      dndFeedbackCursor = cursor;
      dndFeedbackCursor.setPosition(cursorPos);

      crect = q->cursorRect(dndFeedbackCursor);
      emit q->updateRequest(crect);
   }

   return true; // accept proposed action
}

bool QTextControlPrivate::dropEvent(const QMimeData *mimeData, const QPointF &pos, Qt::DropAction dropAction,
   QObject *source)
{
   Q_Q(QTextControl);
   dndFeedbackCursor = QTextCursor();

   if (!(interactionFlags & Qt::TextEditable) || !q->canInsertFromMimeData(mimeData)) {
      return false;
   }

   repaintSelection();

   QTextCursor insertionCursor = q->cursorForPosition(pos);
   insertionCursor.beginEditBlock();

   if (dropAction == Qt::MoveAction && source == contextWidget) {
      cursor.removeSelectedText();
   }

   cursor = insertionCursor;
   q->insertFromMimeData(mimeData);
   insertionCursor.endEditBlock();
   q->ensureCursorVisible();

   return true; // accept proposed action
}

void QTextControlPrivate::inputMethodEvent(QInputMethodEvent *e)
{
   Q_Q(QTextControl);
   if (!(interactionFlags & Qt::TextEditable) || cursor.isNull()) {
      e->ignore();
      return;
   }
   bool isGettingInput = !e->commitString().isEmpty()
      || e->preeditString() != cursor.block().layout()->preeditAreaText()
      || e->replacementLength() > 0;

   cursor.beginEditBlock();
   if (isGettingInput) {
      cursor.removeSelectedText();
   }

   // insert commit string
   if (!e->commitString().isEmpty() || e->replacementLength()) {
      QTextCursor c = cursor;
      c.setPosition(c.position() + e->replacementStart());
      c.setPosition(c.position() + e->replacementLength(), QTextCursor::KeepAnchor);
      c.insertText(e->commitString());
   }

   for (int i = 0; i < e->attributes().size(); ++i) {
      const QInputMethodEvent::Attribute &a = e->attributes().at(i);
      if (a.type == QInputMethodEvent::Selection) {
         QTextCursor oldCursor = cursor;
         int blockStart = a.start + cursor.block().position();
         cursor.setPosition(blockStart, QTextCursor::MoveAnchor);
         cursor.setPosition(blockStart + a.length, QTextCursor::KeepAnchor);
         q->ensureCursorVisible();
         repaintOldAndNewSelection(oldCursor);
      }
   }

   QTextBlock block = cursor.block();
   QTextLayout *layout = block.layout();
   if (isGettingInput) {
      layout->setPreeditArea(cursor.position() - block.position(), e->preeditString());
   }

   QVector<QTextLayout::FormatRange> overrides;
   overrides.reserve(e->attributes().size());

   const int oldPreeditCursor = preeditCursor;
   preeditCursor = e->preeditString().length();
   hideCursor = false;

   for (int i = 0; i < e->attributes().size(); ++i) {
      const QInputMethodEvent::Attribute &a = e->attributes().at(i);
      if (a.type == QInputMethodEvent::Cursor) {
         preeditCursor = a.start;
         hideCursor = !a.length;

      } else if (a.type == QInputMethodEvent::TextFormat) {
         QTextCharFormat f = qvariant_cast<QTextFormat>(a.value).toCharFormat();

         if (f.isValid()) {
            QTextLayout::FormatRange o;
            o.start = a.start + cursor.position() - block.position();
            o.length = a.length;
            o.format = f;
            overrides.append(o);
         }
      }
   }

   layout->setFormats(overrides);
   cursor.endEditBlock();
   if (cursor.d) {
      cursor.d->setX();
   }
   if (oldPreeditCursor != preeditCursor) {
      emit q->microFocusChanged();
   }
}

QVariant QTextControl::inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const
{
   Q_D(const QTextControl);

   QTextBlock block = d->cursor.block();

   switch (property) {
      case Qt::ImCursorRectangle:
         return cursorRect();
      case Qt::ImFont:
         return QVariant(d->cursor.charFormat().font());
      case Qt::ImCursorPosition:
         return QVariant(d->cursor.position() - block.position());
      case Qt::ImSurroundingText:
         return QVariant(block.text());
      case Qt::ImCurrentSelection:
         return QVariant(d->cursor.selectedText());
      case Qt::ImMaximumTextLength:
         return QVariant(); // No limit.
      case Qt::ImAnchorPosition:
         return QVariant(d->cursor.anchor() - block.position());
      case Qt::ImAbsolutePosition:
         return QVariant(d->cursor.position());
      case Qt::ImTextAfterCursor: {
         int maxLength = argument.isValid() ? argument.toInt() : 1024;
         QTextCursor tmpCursor = d->cursor;
         int localPos = d->cursor.position() - block.position();
         QString result = block.text().mid(localPos);
         while (result.length() < maxLength) {
            int currentBlock = tmpCursor.blockNumber();
            tmpCursor.movePosition(QTextCursor::NextBlock);
            if (tmpCursor.blockNumber() == currentBlock) {
               break;
            }
            result += QLatin1Char('\n') + tmpCursor.block().text();
         }
         return QVariant(result);
      }
      case Qt::ImTextBeforeCursor: {
         int maxLength = argument.isValid() ? argument.toInt() : 1024;
         QTextCursor tmpCursor = d->cursor;
         int localPos = d->cursor.position() - block.position();
         int numBlocks = 0;
         int resultLen = localPos;
         while (resultLen < maxLength) {
            int currentBlock = tmpCursor.blockNumber();
            tmpCursor.movePosition(QTextCursor::PreviousBlock);
            if (tmpCursor.blockNumber() == currentBlock) {
               break;
            }
            numBlocks++;
            resultLen += tmpCursor.block().length();
         }
         QString result;
         while (numBlocks) {
            result += tmpCursor.block().text() + QLatin1Char('\n');
            tmpCursor.movePosition(QTextCursor::NextBlock);
            --numBlocks;
         }
         result += block.text().mid(0, localPos);
         return QVariant(result);
      }
      default:
         return QVariant();
   }
}

void QTextControl::setFocus(bool focus, Qt::FocusReason reason)
{
   QFocusEvent ev(focus ? QEvent::FocusIn : QEvent::FocusOut,
      reason);
   processEvent(&ev);
}

void QTextControlPrivate::focusEvent(QFocusEvent *e)
{
   Q_Q(QTextControl);

   emit q->updateRequest(q->selectionRect());

   if (e->gotFocus()) {

#ifdef QT_KEYPAD_NAVIGATION
      if (! QApplication::keypadNavigationEnabled() || (hasEditFocus && (e->reason() == Qt::PopupFocusReason))) {

#endif
         cursorOn = (interactionFlags & (Qt::TextSelectableByKeyboard | Qt::TextEditable));
         if (interactionFlags & Qt::TextEditable) {
            setBlinkingCursorEnabled(true);
         }
#ifdef QT_KEYPAD_NAVIGATION
      }
#endif

   } else {
      setBlinkingCursorEnabled(false);

      if (cursorIsFocusIndicator
         && e->reason() != Qt::ActiveWindowFocusReason
         && e->reason() != Qt::PopupFocusReason
         && cursor.hasSelection()) {
         cursor.clearSelection();
      }
   }
   hasFocus = e->gotFocus();
}

QString QTextControlPrivate::anchorForCursor(const QTextCursor &anchorCursor) const
{
   if (anchorCursor.hasSelection()) {
      QTextCursor cursor = anchorCursor;
      if (cursor.selectionStart() != cursor.position()) {
         cursor.setPosition(cursor.selectionStart());
      }

      cursor.movePosition(QTextCursor::NextCharacter);
      QTextCharFormat fmt = cursor.charFormat();

      if (fmt.isAnchor() && fmt.hasProperty(QTextFormat::AnchorHref)) {
         return fmt.stringProperty(QTextFormat::AnchorHref);
      }
   }
   return QString();
}

#ifdef QT_KEYPAD_NAVIGATION
void QTextControlPrivate::editFocusEvent(QEvent *e)
{
   Q_Q(QTextControl);

   if (QApplication::keypadNavigationEnabled()) {
      if (e->type() == QEvent::EnterEditFocus && interactionFlags & Qt::TextEditable) {
         const QTextCursor oldSelection = cursor;
         const int oldCursorPos = cursor.position();
         const bool moved = cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
         q->ensureCursorVisible();
         if (moved) {
            if (cursor.position() != oldCursorPos) {
               emit q->cursorPositionChanged();
            }
            emit q->microFocusChanged();
         }
         selectionChanged();
         repaintOldAndNewSelection(oldSelection);

         setBlinkingCursorEnabled(true);
      } else {
         setBlinkingCursorEnabled(false);
      }
   }

   hasEditFocus = (e->type() == QEvent::EnterEditFocus);
}
#endif

#ifndef QT_NO_CONTEXTMENU

static inline void setActionIcon(QAction *action, const QString &name)
{
   const QIcon icon = QIcon::fromTheme(name);

   if (!icon.isNull()) {
      action->setIcon(icon);
   }
}

QMenu *QTextControl::createStandardContextMenu(const QPointF &pos, QWidget *parent)
{
   Q_D(QTextControl);

   const bool showTextSelectionActions = d->interactionFlags & (Qt::TextEditable | Qt::TextSelectableByKeyboard |
         Qt::TextSelectableByMouse);

   d->linkToCopy = QString();
   if (! pos.isNull()) {
      d->linkToCopy = anchorAt(pos);
   }

   if (d->linkToCopy.isEmpty() && !showTextSelectionActions) {
      return nullptr;
   }

   QMenu *menu = new QMenu(parent);
   QAction *a;

   if (d->interactionFlags & Qt::TextEditable) {
      a = menu->addAction(tr("&Undo") + ACCEL_KEY(QKeySequence::Undo), this, SLOT(undo()));
      a->setEnabled(d->doc->isUndoAvailable());
      a->setObjectName("edit-undo");
      setActionIcon(a, "edit-undo");

      a = menu->addAction(tr("&Redo") + ACCEL_KEY(QKeySequence::Redo), this, SLOT(redo()));
      a->setEnabled(d->doc->isRedoAvailable());
      a->setObjectName("edit-redo");
      setActionIcon(a, "edit-redo");

      menu->addSeparator();

#ifndef QT_NO_CLIPBOARD
      a = menu->addAction(tr("Cu&t") + ACCEL_KEY(QKeySequence::Cut), this, SLOT(cut()));
      a->setEnabled(d->cursor.hasSelection());
      a->setObjectName("edit-cut");
      setActionIcon(a, "edit-cut");
#endif
   }

#ifndef QT_NO_CLIPBOARD
   if (showTextSelectionActions) {
      a = menu->addAction(tr("&Copy") + ACCEL_KEY(QKeySequence::Copy), this, SLOT(copy()));
      a->setEnabled(d->cursor.hasSelection());
      a->setObjectName("edit-copy");
      setActionIcon(a, "edit-copy");
   }

   if ((d->interactionFlags & Qt::LinksAccessibleByKeyboard)
      || (d->interactionFlags & Qt::LinksAccessibleByMouse)) {

      a = menu->addAction(tr("Copy &Link Location"), this, SLOT(_q_copyLink()));
      a->setEnabled(!d->linkToCopy.isEmpty());
      a->setObjectName("link-copy");
   }
#endif

   if (d->interactionFlags & Qt::TextEditable) {
#ifndef QT_NO_CLIPBOARD
      a = menu->addAction(tr("&Paste") + ACCEL_KEY(QKeySequence::Paste), this, SLOT(paste()));
      a->setEnabled(canPaste());
      a->setObjectName("edit-paste");
      setActionIcon(a, "edit-paste");
#endif

      a = menu->addAction(tr("Delete"), this, SLOT(_q_deleteSelected()));
      a->setEnabled(d->cursor.hasSelection());
      a->setObjectName("edit-delete");
      setActionIcon(a, "edit-delete");
   }


   if (showTextSelectionActions) {
      menu->addSeparator();
      a = menu->addAction(tr("Select All") + ACCEL_KEY(QKeySequence::SelectAll), this, SLOT(selectAll()));
      a->setEnabled(!d->doc->isEmpty());
      a->setObjectName("select-all");
   }

   if ((d->interactionFlags & Qt::TextEditable) && QGuiApplication::styleHints()->useRtlExtensions()) {

      menu->addSeparator();
      QUnicodeControlCharacterMenu *ctrlCharacterMenu = new QUnicodeControlCharacterMenu(this, menu);
      menu->addMenu(ctrlCharacterMenu);
   }

   return menu;
}
#endif // QT_NO_CONTEXTMENU

QTextCursor QTextControl::cursorForPosition(const QPointF &pos) const
{
   Q_D(const QTextControl);
   int cursorPos = hitTest(pos, Qt::FuzzyHit);
   if (cursorPos == -1) {
      cursorPos = 0;
   }
   QTextCursor c(d->doc);
   c.setPosition(cursorPos);
   return c;
}

QRectF QTextControl::cursorRect(const QTextCursor &cursor) const
{
   Q_D(const QTextControl);

   if (cursor.isNull()) {
      return QRectF();
   }

   return d->rectForPosition(cursor.position());
}

QRectF QTextControl::cursorRect() const
{
   Q_D(const QTextControl);
   return cursorRect(d->cursor);
}

QRectF QTextControlPrivate::cursorRectPlusUnicodeDirectionMarkers(const QTextCursor &cursor) const
{
   if (cursor.isNull()) {
      return QRectF();
   }

   return rectForPosition(cursor.position()).adjusted(-4, 0, 4, 0);
}

QString QTextControl::anchorAt(const QPointF &pos) const
{
   Q_D(const QTextControl);
   return d->doc->documentLayout()->anchorAt(pos);
}

QString QTextControl::anchorAtCursor() const
{
   Q_D(const QTextControl);

   return d->anchorForCursor(d->cursor);
}

bool QTextControl::overwriteMode() const
{
   Q_D(const QTextControl);
   return d->overwriteMode;
}

void QTextControl::setOverwriteMode(bool overwrite)
{
   Q_D(QTextControl);
   d->overwriteMode = overwrite;
}

int QTextControl::cursorWidth() const
{
#ifndef QT_NO_PROPERTIES
   Q_D(const QTextControl);

   return d->doc->documentLayout()->property("cursorWidth").toInt();
#else
   return 1;
#endif
}

void QTextControl::setCursorWidth(int width)
{
   Q_D(QTextControl);
#ifdef QT_NO_PROPERTIES
   Q_UNUSED(width);
#else
   if (width == -1) {
      width = QApplication::style()->pixelMetric(QStyle::PM_TextCursorWidth);
   }

   d->doc->documentLayout()->setProperty("cursorWidth", width);
#endif

   d->repaintCursor();
}

bool QTextControl::acceptRichText() const
{
   Q_D(const QTextControl);
   return d->acceptRichText;
}

void QTextControl::setAcceptRichText(bool accept)
{
   Q_D(QTextControl);
   d->acceptRichText = accept;
}

#ifndef QT_NO_TEXTEDIT

void QTextControl::setExtraSelections(const QList<QTextEdit::ExtraSelection> &selections)
{
   Q_D(QTextControl);

   QMultiHash<int, int> hash;

   for (int i = 0; i < d->extraSelections.count(); ++i) {
      const QAbstractTextDocumentLayout::Selection &esel = d->extraSelections.at(i);
      hash.insertMulti(esel.cursor.anchor(), i);
   }

   for (int i = 0; i < selections.count(); ++i) {
      const QTextEdit::ExtraSelection &sel = selections.at(i);

      int key   = sel.cursor.anchor();
      auto iter = hash.find(key);

      bool okToAdd = true;

      while (iter != hash.end() && iter.key() == key) {
         // check each potetial match
         const QAbstractTextDocumentLayout::Selection &esel = d->extraSelections.at(iter.value());

         if (esel.cursor.position() == sel.cursor.position() && esel.format == sel.format) {
            iter = hash.erase(iter);
            okToAdd = false;

         } else {
            ++iter;
         }
      }

      if (okToAdd) {
         // new selection
         QRectF r = selectionRect(sel.cursor);

         if (sel.format.boolProperty(QTextFormat::FullWidthSelection)) {
            r.setLeft(0);
            r.setWidth(qreal(INT_MAX));
         }

         emit updateRequest(r);
      }
   }

   for (auto &item : hash) {
      const QAbstractTextDocumentLayout::Selection &esel = d->extraSelections.at(item);
      QRectF r = selectionRect(esel.cursor);

      if (esel.format.boolProperty(QTextFormat::FullWidthSelection)) {
         r.setLeft(0);
         r.setWidth(qreal(INT_MAX));
      }
      emit updateRequest(r);
   }

   d->extraSelections.resize(selections.count());

   for (int i = 0; i < selections.count(); ++i) {
      d->extraSelections[i].cursor = selections.at(i).cursor;
      d->extraSelections[i].format = selections.at(i).format;
   }
}

QList<QTextEdit::ExtraSelection> QTextControl::extraSelections() const
{
   Q_D(const QTextControl);
   QList<QTextEdit::ExtraSelection> selections;
   for (int i = 0; i < d->extraSelections.count(); ++i) {
      QTextEdit::ExtraSelection sel;
      const QAbstractTextDocumentLayout::Selection &tmp = d->extraSelections.at(i);

      sel.cursor = tmp.cursor;
      sel.format = tmp.format;
      selections.append(sel);
   }
   return selections;
}

#endif // QT_NO_TEXTEDIT

void QTextControl::setTextWidth(qreal width)
{
   Q_D(QTextControl);
   d->doc->setTextWidth(width);
}

qreal QTextControl::textWidth() const
{
   Q_D(const QTextControl);
   return d->doc->textWidth();
}

QSizeF QTextControl::size() const
{
   Q_D(const QTextControl);
   return d->doc->size();
}

void QTextControl::setOpenExternalLinks(bool open)
{
   Q_D(QTextControl);
   d->openExternalLinks = open;
}

bool QTextControl::openExternalLinks() const
{
   Q_D(const QTextControl);
   return d->openExternalLinks;
}

bool QTextControl::ignoreUnusedNavigationEvents() const
{
   Q_D(const QTextControl);
   return d->ignoreUnusedNavigationEvents;
}

void QTextControl::setIgnoreUnusedNavigationEvents(bool ignore)
{
   Q_D(QTextControl);
   d->ignoreUnusedNavigationEvents = ignore;
}

void QTextControl::moveCursor(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode)
{
   Q_D(QTextControl);
   const QTextCursor oldSelection = d->cursor;
   const bool moved = d->cursor.movePosition(op, mode);
   d->_q_updateCurrentCharFormatAndSelection();
   ensureCursorVisible();
   d->repaintOldAndNewSelection(oldSelection);
   if (moved) {
      emit cursorPositionChanged();
   }
}

bool QTextControl::canPaste() const
{
#ifndef QT_NO_CLIPBOARD
   Q_D(const QTextControl);

   if (d->interactionFlags & Qt::TextEditable) {
      const QMimeData *md = QApplication::clipboard()->mimeData();
      return md && canInsertFromMimeData(md);
   }
#endif
   return false;
}

void QTextControl::setCursorIsFocusIndicator(bool b)
{
   Q_D(QTextControl);
   d->cursorIsFocusIndicator = b;
   d->repaintCursor();
}

bool QTextControl::cursorIsFocusIndicator() const
{
   Q_D(const QTextControl);
   return d->cursorIsFocusIndicator;
}


void QTextControl::setDragEnabled(bool enabled)
{
   Q_D(QTextControl);
   d->dragEnabled = enabled;
}

bool QTextControl::isDragEnabled() const
{
   Q_D(const QTextControl);
   return d->dragEnabled;
}

void QTextControl::setWordSelectionEnabled(bool enabled)
{
   Q_D(QTextControl);
   d->wordSelectionEnabled = enabled;
}

bool QTextControl::isWordSelectionEnabled() const
{
   Q_D(const QTextControl);
   return d->wordSelectionEnabled;
}

bool QTextControl::isPreediting()
{
   return d_func()->isPreediting();
}
#ifndef QT_NO_PRINTER
void QTextControl::print(QPagedPaintDevice *printer) const
{
   Q_D(const QTextControl);

   if (! printer) {
      return;
   }

   QTextDocument *tempDoc   = nullptr;
   const QTextDocument *doc = d->doc;

   if (printer->printSelectionOnly()) {
      if (! d->cursor.hasSelection()) {
         return;
      }

      tempDoc = new QTextDocument(const_cast<QTextDocument *>(doc));
      tempDoc->setMetaInformation(QTextDocument::DocumentTitle, doc->metaInformation(QTextDocument::DocumentTitle));
      tempDoc->setPageSize(doc->pageSize());
      tempDoc->setDefaultFont(doc->defaultFont());
      tempDoc->setUseDesignMetrics(doc->useDesignMetrics());

      QTextCursor(tempDoc).insertFragment(d->cursor.selection());
      doc = tempDoc;

      // copy the custom object handlers
      doc->documentLayout()->d_func()->handlers = d->doc->documentLayout()->d_func()->handlers;
   }

   doc->print(printer);
   delete tempDoc;

}
#endif // QT_NO_PRINTER

QMimeData *QTextControl::createMimeDataFromSelection() const
{
   Q_D(const QTextControl);
   const QTextDocumentFragment fragment(d->cursor);
   return new QTextEditMimeData(fragment);
}

bool QTextControl::canInsertFromMimeData(const QMimeData *source) const
{
   Q_D(const QTextControl);

   if (d->acceptRichText) {

      return (source->hasText() && ! source->text().isEmpty())
         || source->hasHtml()
         || source->hasFormat("application/x-qrichtext")
         || source->hasFormat("application/x-qt-richtext");

   } else {
      return source->hasText() && !source->text().isEmpty();
   }
}

void QTextControl::insertFromMimeData(const QMimeData *source)
{
   Q_D(QTextControl);

   if (! (d->interactionFlags & Qt::TextEditable) || !source) {
      return;
   }

   bool hasData = false;
   QTextDocumentFragment fragment;

#ifndef QT_NO_TEXTHTMLPARSER

   if (source->hasFormat("application/x-qrichtext") && d->acceptRichText) {
      // x-qrichtext is always UTF-8

      QString richtext = QString::fromUtf8(source->data("application/x-qrichtext"));
      richtext.prepend("<meta name=\"qrichtext\" content=\"1\" />");
      fragment = QTextDocumentFragment::fromHtml(richtext, d->doc);
      hasData = true;

   } else if (source->hasHtml() && d->acceptRichText) {
      fragment = QTextDocumentFragment::fromHtml(source->html(), d->doc);
      hasData = true;

   } else {
      QString text = source->text();

      if (! text.isEmpty()) {
         fragment = QTextDocumentFragment::fromPlainText(text);
         hasData = true;
      }
   }

#else
   fragment = QTextDocumentFragment::fromPlainText(source->text());

#endif

   if (hasData) {
      d->cursor.insertFragment(fragment);
   }

   ensureCursorVisible();
}

bool QTextControl::findNextPrevAnchor(const QTextCursor &startCursor, bool next, QTextCursor &newAnchor)
{
   Q_D(QTextControl);

   int anchorStart = -1;
   QString anchorHref;
   int anchorEnd = -1;

   if (next) {
      const int startPos = startCursor.selectionEnd();

      QTextBlock block = d->doc->findBlock(startPos);
      QTextBlock::iterator it = block.begin();

      while (!it.atEnd() && it.fragment().position() < startPos) {
         ++it;
      }

      while (block.isValid()) {
         anchorStart = -1;

         // find next anchor
         for (; !it.atEnd(); ++it) {
            const QTextFragment fragment = it.fragment();
            const QTextCharFormat fmt = fragment.charFormat();

            if (fmt.isAnchor() && fmt.hasProperty(QTextFormat::AnchorHref)) {
               anchorStart = fragment.position();
               anchorHref = fmt.anchorHref();
               break;
            }
         }

         if (anchorStart != -1) {
            anchorEnd = -1;

            // find next non-anchor fragment
            for (; !it.atEnd(); ++it) {
               const QTextFragment fragment = it.fragment();
               const QTextCharFormat fmt = fragment.charFormat();

               if (!fmt.isAnchor() || fmt.anchorHref() != anchorHref) {
                  anchorEnd = fragment.position();
                  break;
               }
            }

            if (anchorEnd == -1) {
               anchorEnd = block.position() + block.length() - 1;
            }

            // make found selection
            break;
         }

         block = block.next();
         it = block.begin();
      }
   } else {
      int startPos = startCursor.selectionStart();
      if (startPos > 0) {
         --startPos;
      }

      QTextBlock block = d->doc->findBlock(startPos);
      QTextBlock::iterator blockStart = block.begin();
      QTextBlock::iterator it = block.end();

      if (startPos == block.position()) {
         it = block.begin();
      } else {
         do {
            if (it == blockStart) {
               it = QTextBlock::iterator();
               block = QTextBlock();
            } else {
               --it;
            }
         } while (!it.atEnd() && it.fragment().position() + it.fragment().length() - 1 > startPos);
      }

      while (block.isValid()) {
         anchorStart = -1;

         if (!it.atEnd()) {
            do {
               const QTextFragment fragment = it.fragment();
               const QTextCharFormat fmt = fragment.charFormat();

               if (fmt.isAnchor() && fmt.hasProperty(QTextFormat::AnchorHref)) {
                  anchorStart = fragment.position() + fragment.length();
                  anchorHref = fmt.anchorHref();
                  break;
               }

               if (it == blockStart) {
                  it = QTextBlock::iterator();
               } else {
                  --it;
               }
            } while (!it.atEnd());
         }

         if (anchorStart != -1 && !it.atEnd()) {
            anchorEnd = -1;

            do {
               const QTextFragment fragment = it.fragment();
               const QTextCharFormat fmt = fragment.charFormat();

               if (!fmt.isAnchor() || fmt.anchorHref() != anchorHref) {
                  anchorEnd = fragment.position() + fragment.length();
                  break;
               }

               if (it == blockStart) {
                  it = QTextBlock::iterator();
               } else {
                  --it;
               }
            } while (!it.atEnd());

            if (anchorEnd == -1) {
               anchorEnd = qMax(0, block.position());
            }

            break;
         }

         block = block.previous();
         it = block.end();
         if (it != block.begin()) {
            --it;
         }
         blockStart = block.begin();
      }

   }

   if (anchorStart != -1 && anchorEnd != -1) {
      newAnchor = d->cursor;
      newAnchor.setPosition(anchorStart);
      newAnchor.setPosition(anchorEnd, QTextCursor::KeepAnchor);
      return true;
   }

   return false;
}

void QTextControlPrivate::activateLinkUnderCursor(QString href)
{
   QTextCursor oldCursor = cursor;

   if (href.isEmpty()) {
      QTextCursor tmp = cursor;
      if (tmp.selectionStart() != tmp.position()) {
         tmp.setPosition(tmp.selectionStart());
      }
      tmp.movePosition(QTextCursor::NextCharacter);
      href = tmp.charFormat().anchorHref();
   }
   if (href.isEmpty()) {
      return;
   }

   if (!cursor.hasSelection()) {
      QTextBlock block = cursor.block();
      const int cursorPos = cursor.position();

      QTextBlock::iterator it = block.begin();
      QTextBlock::iterator linkFragment;

      for (; !it.atEnd(); ++it) {
         QTextFragment fragment = it.fragment();
         const int fragmentPos = fragment.position();
         if (fragmentPos <= cursorPos &&
            fragmentPos + fragment.length() > cursorPos) {
            linkFragment = it;
            break;
         }
      }

      if (!linkFragment.atEnd()) {
         it = linkFragment;
         cursor.setPosition(it.fragment().position());
         if (it != block.begin()) {
            do {
               --it;
               QTextFragment fragment = it.fragment();
               if (fragment.charFormat().anchorHref() != href) {
                  break;
               }
               cursor.setPosition(fragment.position());
            } while (it != block.begin());
         }

         for (it = linkFragment; !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            if (fragment.charFormat().anchorHref() != href) {
               break;
            }
            cursor.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
         }
      }
   }

   if (hasFocus) {
      cursorIsFocusIndicator = true;
   } else {
      cursorIsFocusIndicator = false;
      cursor.clearSelection();
   }
   repaintOldAndNewSelection(oldCursor);

#ifndef QT_NO_DESKTOPSERVICES
   if (openExternalLinks) {
      QDesktopServices::openUrl(QUrl(href));
   } else
#endif
      emit q_func()->linkActivated(href);
}

#ifndef QT_NO_TOOLTIP
void QTextControlPrivate::showToolTip(const QPoint &globalPos, const QPointF &pos, QWidget *contextWidget)
{
   const QString toolTip = q_func()->cursorForPosition(pos).charFormat().toolTip();
   if (toolTip.isEmpty()) {
      return;
   }
   QToolTip::showText(globalPos, toolTip, contextWidget);
}
#endif // QT_NO_TOOLTIP


bool QTextControlPrivate::isPreediting() const
{
   QTextLayout *layout = cursor.block().layout();
   if (layout && !layout->preeditAreaText().isEmpty()) {
      return true;
   }

   return false;
}

void QTextControlPrivate::commitPreedit()
{
   if (!isPreediting()) {
      return;
   }

   QGuiApplication::inputMethod()->commit();

   if (!isPreediting()) {
      return;
   }

   cursor.beginEditBlock();
   preeditCursor = 0;
   QTextBlock block = cursor.block();
   QTextLayout *layout = block.layout();
   layout->setPreeditArea(-1, QString());
   layout->clearFormats();
   cursor.endEditBlock();
}
bool QTextControl::setFocusToNextOrPreviousAnchor(bool next)
{
   Q_D(QTextControl);

   if (!(d->interactionFlags & Qt::LinksAccessibleByKeyboard)) {
      return false;
   }

   QRectF crect = selectionRect();
   emit updateRequest(crect);

   // If we don't have a current anchor, we start from the start/end
   if (!d->cursor.hasSelection()) {
      d->cursor = QTextCursor(d->doc);
      if (next) {
         d->cursor.movePosition(QTextCursor::Start);
      } else {
         d->cursor.movePosition(QTextCursor::End);
      }
   }

   QTextCursor newAnchor;
   if (findNextPrevAnchor(d->cursor, next, newAnchor)) {
      d->cursor = newAnchor;
      d->cursorIsFocusIndicator = true;
   } else {
      d->cursor.clearSelection();
   }

   if (d->cursor.hasSelection()) {
      crect = selectionRect();
      emit updateRequest(crect);
      emit visibilityRequest(crect);
      return true;
   } else {
      return false;
   }
}

bool QTextControl::setFocusToAnchor(const QTextCursor &newCursor)
{
   Q_D(QTextControl);

   if (!(d->interactionFlags & Qt::LinksAccessibleByKeyboard)) {
      return false;
   }

   // Verify that this is an anchor.
   const QString anchorHref = d->anchorForCursor(newCursor);
   if (anchorHref.isEmpty()) {
      return false;
   }

   // and process it
   QRectF crect = selectionRect();
   emit updateRequest(crect);

   d->cursor.setPosition(newCursor.selectionStart());
   d->cursor.setPosition(newCursor.selectionEnd(), QTextCursor::KeepAnchor);
   d->cursorIsFocusIndicator = true;

   crect = selectionRect();
   emit updateRequest(crect);
   emit visibilityRequest(crect);
   return true;
}

void QTextControl::setTextInteractionFlags(Qt::TextInteractionFlags flags)
{
   Q_D(QTextControl);
   if (flags == d->interactionFlags) {
      return;
   }
   d->interactionFlags = flags;

   if (d->hasFocus) {
      d->setBlinkingCursorEnabled(flags & Qt::TextEditable);
   }
}

Qt::TextInteractionFlags QTextControl::textInteractionFlags() const
{
   Q_D(const QTextControl);
   return d->interactionFlags;
}

void QTextControl::mergeCurrentCharFormat(const QTextCharFormat &modifier)
{
   Q_D(QTextControl);
   d->cursor.mergeCharFormat(modifier);
   d->updateCurrentCharFormat();
}

void QTextControl::setCurrentCharFormat(const QTextCharFormat &format)
{
   Q_D(QTextControl);
   d->cursor.setCharFormat(format);
   d->updateCurrentCharFormat();
}

QTextCharFormat QTextControl::currentCharFormat() const
{
   Q_D(const QTextControl);
   return d->cursor.charFormat();
}

void QTextControl::insertPlainText(const QString &text)
{
   Q_D(QTextControl);
   d->cursor.insertText(text);
}

#ifndef QT_NO_TEXTHTMLPARSER
void QTextControl::insertHtml(const QString &text)
{
   Q_D(QTextControl);
   d->cursor.insertHtml(text);
}
#endif // QT_NO_TEXTHTMLPARSER

QPointF QTextControl::anchorPosition(const QString &name) const
{
   Q_D(const QTextControl);
   if (name.isEmpty()) {
      return QPointF();
   }

   QRectF r;
   for (QTextBlock block = d->doc->begin(); block.isValid(); block = block.next()) {
      QTextCharFormat format = block.charFormat();
      if (format.isAnchor() && format.anchorNames().contains(name)) {
         r = d->rectForPosition(block.position());
         break;
      }

      for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
         QTextFragment fragment = it.fragment();
         format = fragment.charFormat();
         if (format.isAnchor() && format.anchorNames().contains(name)) {
            r = d->rectForPosition(fragment.position());
            block = QTextBlock();
            break;
         }
      }
   }
   if (!r.isValid()) {
      return QPointF();
   }
   return QPointF(0, r.top());
}

void QTextControl::adjustSize()
{
   Q_D(QTextControl);
   d->doc->adjustSize();
}

bool QTextControl::find(const QString &exp, QTextDocument::FindFlags options)
{
   Q_D(QTextControl);
   QTextCursor search = d->doc->find(exp, d->cursor, options);

   if (search.isNull()) {
      return false;
   }

   setTextCursor(search);
   return true;
}

bool QTextControl::find(const QRegularExpression &exp, QTextDocument::FindFlags options)
{
   Q_D(QTextControl);

   QTextCursor search = d->doc->find(exp, d->cursor, options);
   if (search.isNull()) {
      return false;
   }

   setTextCursor(search);
   return true;
}

QString QTextControl::toPlainText() const
{
   return document()->toPlainText();
}

#ifndef QT_NO_TEXTHTMLPARSER
QString QTextControl::toHtml() const
{
   return document()->toHtml();
}
#endif

void QTextControlPrivate::append(const QString &text, Qt::TextFormat format)
{
   QTextCursor tmp(doc);
   tmp.beginEditBlock();
   tmp.movePosition(QTextCursor::End);

   if (!doc->isEmpty()) {
      tmp.insertBlock(cursor.blockFormat(), cursor.charFormat());
   } else {
      tmp.setCharFormat(cursor.charFormat());
   }

   // preserve the char format
   QTextCharFormat oldCharFormat = cursor.charFormat();

#ifndef QT_NO_TEXTHTMLPARSER
   if (format == Qt::RichText || (format == Qt::AutoText && Qt::mightBeRichText(text))) {
      tmp.insertHtml(text);
   } else {
      tmp.insertText(text);
   }
#else
   tmp.insertText(text);
#endif

   if (!cursor.hasSelection()) {
      cursor.setCharFormat(oldCharFormat);
   }

   tmp.endEditBlock();
}

void QTextControl::append(const QString &text)
{
   Q_D(QTextControl);
   d->append(text, Qt::AutoText);
}

void QTextControl::appendHtml(const QString &html)
{
   Q_D(QTextControl);
   d->append(html, Qt::RichText);
}

void QTextControl::appendPlainText(const QString &text)
{
   Q_D(QTextControl);
   d->append(text, Qt::PlainText);
}


void QTextControl::ensureCursorVisible()
{
   Q_D(QTextControl);
   QRectF crect = d->rectForPosition(d->cursor.position()).adjusted(-5, 0, 5, 0);
   emit visibilityRequest(crect);
   emit microFocusChanged();
}

QPalette QTextControl::palette() const
{
   Q_D(const QTextControl);
   return d->palette;
}

void QTextControl::setPalette(const QPalette &pal)
{
   Q_D(QTextControl);
   d->palette = pal;
}

QAbstractTextDocumentLayout::PaintContext QTextControl::getPaintContext(QWidget *widget) const
{
   Q_D(const QTextControl);

   QAbstractTextDocumentLayout::PaintContext ctx;

   ctx.selections = d->extraSelections;
   ctx.palette = d->palette;
   if (d->cursorOn && d->isEnabled) {
      if (d->hideCursor) {
         ctx.cursorPosition = -1;
      } else if (d->preeditCursor != 0) {
         ctx.cursorPosition = - (d->preeditCursor + 2);
      } else {
         ctx.cursorPosition = d->cursor.position();
      }
   }

   if (!d->dndFeedbackCursor.isNull()) {
      ctx.cursorPosition = d->dndFeedbackCursor.position();
   }

#ifdef QT_KEYPAD_NAVIGATION
   if (!QApplication::keypadNavigationEnabled() || d->hasEditFocus)
#endif

      if (d->cursor.hasSelection()) {
         QAbstractTextDocumentLayout::Selection selection;
         selection.cursor = d->cursor;
         if (d->cursorIsFocusIndicator) {
            QStyleOption opt;
            opt.palette = ctx.palette;
            QStyleHintReturnVariant ret;
            QStyle *style = QApplication::style();
            if (widget) {
               style = widget->style();
            }
            style->styleHint(QStyle::SH_TextControl_FocusIndicatorTextCharFormat, &opt, widget, &ret);
            selection.format = qvariant_cast<QTextFormat>(ret.variant).toCharFormat();
         } else {
            QPalette::ColorGroup cg = d->hasFocus ? QPalette::Active : QPalette::Inactive;
            selection.format.setBackground(ctx.palette.brush(cg, QPalette::Highlight));
            selection.format.setForeground(ctx.palette.brush(cg, QPalette::HighlightedText));
            QStyleOption opt;
            QStyle *style = QApplication::style();
            if (widget) {
               opt.initFrom(widget);
               style = widget->style();
            }
            if (style->styleHint(QStyle::SH_RichText_FullWidthSelection, &opt, widget)) {
               selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            }
         }
         ctx.selections.append(selection);
      }

   return ctx;
}

void QTextControl::drawContents(QPainter *p, const QRectF &rect, QWidget *widget)
{
   Q_D(QTextControl);
   p->save();
   QAbstractTextDocumentLayout::PaintContext ctx = getPaintContext(widget);
   if (rect.isValid()) {
      p->setClipRect(rect, Qt::IntersectClip);
   }
   ctx.clip = rect;

   d->doc->documentLayout()->draw(p, ctx);
   p->restore();
}

void QTextControlPrivate::_q_copyLink()
{
#ifndef QT_NO_CLIPBOARD
   QMimeData *md = new QMimeData;
   md->setText(linkToCopy);
   QApplication::clipboard()->setMimeData(md);
#endif
}

int QTextControl::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
   Q_D(const QTextControl);
   return d->doc->documentLayout()->hitTest(point, accuracy);
}

QRectF QTextControl::blockBoundingRect(const QTextBlock &block) const
{
   Q_D(const QTextControl);
   return d->doc->documentLayout()->blockBoundingRect(block);
}

#ifndef QT_NO_CONTEXTMENU
#define NUM_CONTROL_CHARACTERS 14

const struct QUnicodeControlCharacter {
   const char *text;
   ushort character;
} qt_controlCharacters[NUM_CONTROL_CHARACTERS] = {
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "LRM Left-to-right mark"), 0x200e },
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "RLM Right-to-left mark"), 0x200f },
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "ZWJ Zero width joiner"), 0x200d },
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "ZWNJ Zero width non-joiner"), 0x200c },
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "ZWSP Zero width space"), 0x200b },
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "LRE Start of left-to-right embedding"), 0x202a },
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "RLE Start of right-to-left embedding"), 0x202b },
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "LRO Start of left-to-right override"), 0x202d },
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "RLO Start of right-to-left override"), 0x202e },
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "PDF Pop directional formatting"), 0x202c },
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "LRI Left-to-right isolate"), 0x2066 },
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "RLI Right-to-left isolate"), 0x2067 },
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "FSI First strong isolate"), 0x2068 },
   { QT_TRANSLATE_NOOP("QUnicodeControlCharacterMenu", "PDI Pop directional isolate"), 0x2069 }
};

QUnicodeControlCharacterMenu::QUnicodeControlCharacterMenu(QObject *_editWidget, QWidget *parent)
   : QMenu(parent), editWidget(_editWidget)
{
   setTitle(tr("Insert Unicode control character"));
   for (int i = 0; i < NUM_CONTROL_CHARACTERS; ++i) {
      addAction(tr(qt_controlCharacters[i].text), this, SLOT(menuActionTriggered()));
   }
}

void QUnicodeControlCharacterMenu::menuActionTriggered()
{
   QAction *a = qobject_cast<QAction *>(sender());
   int idx = actions().indexOf(a);
   if (idx < 0 || idx >= NUM_CONTROL_CHARACTERS) {
      return;
   }
   QChar c(qt_controlCharacters[idx].character);
   QString str(c);

#ifndef QT_NO_TEXTEDIT
   if (QTextEdit *edit = qobject_cast<QTextEdit *>(editWidget)) {
      edit->insertPlainText(str);
      return;
   }
#endif
   if (QTextControl *control = qobject_cast<QTextControl *>(editWidget)) {
      control->insertPlainText(str);
   }
#ifndef QT_NO_LINEEDIT
   if (QLineEdit *edit = qobject_cast<QLineEdit *>(editWidget)) {
      edit->insert(str);
      return;
   }
#endif
}
#endif // QT_NO_CONTEXTMENU

QStringList QTextEditMimeData::formats() const
{
   if (!fragment.isEmpty())
      return QStringList() << QString("text/plain") << QString("text/html")
#ifndef QT_NO_TEXTODFWRITER
         << QString::fromLatin1("application/vnd.oasis.opendocument.text")
#endif
         ;
   else {
      return QMimeData::formats();
   }
}

QVariant QTextEditMimeData::retrieveData(const QString &mimeType, QVariant::Type type) const
{
   if (!fragment.isEmpty()) {
      setup();
   }
   return QMimeData::retrieveData(mimeType, type);
}

void QTextEditMimeData::setup() const
{
   QTextEditMimeData *that = const_cast<QTextEditMimeData *>(this);

#ifndef QT_NO_TEXTHTMLPARSER
   that->setData(QLatin1String("text/html"), fragment.toHtml("utf-8").toUtf8());
#endif

#ifndef QT_NO_TEXTODFWRITER
   {
      QBuffer buffer;
      QTextDocumentWriter writer(&buffer, "ODF");
      writer.write(fragment);
      buffer.close();
      that->setData(QLatin1String("application/vnd.oasis.opendocument.text"), buffer.data());
   }
#endif

   that->setText(fragment.toPlainText());
   fragment = QTextDocumentFragment();
}

void QTextControl::_q_updateCurrentCharFormatAndSelection()
{
   Q_D(QTextControl);
   d->_q_updateCurrentCharFormatAndSelection();
}

void QTextControl::_q_emitCursorPosChanged(const QTextCursor &un_named_arg1)
{
   Q_D(QTextControl);
   d->_q_emitCursorPosChanged(un_named_arg1);
}

void QTextControl::_q_deleteSelected()
{
   Q_D(QTextControl);
   d->_q_deleteSelected();
}

void QTextControl::_q_copyLink()
{
   Q_D(QTextControl);
   d->_q_copyLink();
}

void QTextControl::_q_updateBlock(const QTextBlock &un_named_arg1)
{
   Q_D(QTextControl);
   d->_q_updateBlock(un_named_arg1);
}

void QTextControl::_q_documentLayoutChanged()
{
   Q_D(QTextControl);
   d->_q_documentLayoutChanged();
}

void QTextControl::_q_contentsChanged(int arg1, int arg2, int arg3)
{
   Q_D(QTextControl);
   d->_q_contentsChanged(arg1, arg2, arg3);
}

#endif // QT_NO_TEXTCONTROL
