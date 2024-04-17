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

#include <qtextedit_p.h>

#include <qlineedit.h>
#include <qtextbrowser.h>

#ifndef QT_NO_TEXTEDIT

#include <qfont.h>
#include <qpainter.h>
#include <qevent.h>
#include <qdebug.h>
#include <qdrag.h>
#include <qclipboard.h>
#include <qmenu.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qtextformat.h>
#include <qdatetime.h>
#include <qapplication.h>
#include <limits.h>
#include <qtexttable.h>
#include <qvariant.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

#include <qtextdocumentlayout_p.h>
#include <qtextdocument.h>
#include <qtextdocument_p.h>
#include <qtextlist.h>
#include <qtextcontrol_p.h>

#endif

#ifndef QT_NO_TEXTEDIT
static inline bool shouldEnableInputMethod(QTextEdit *textedit)
{
   return !textedit->isReadOnly();
}

class QTextEditControl : public QTextControl
{
 public:
   inline QTextEditControl(QObject *parent) : QTextControl(parent) {}

   QMimeData *createMimeDataFromSelection() const override {
      QTextEdit *ed = qobject_cast<QTextEdit *>(parent());
      if (!ed) {
         return QTextControl::createMimeDataFromSelection();
      }

      return ed->createMimeDataFromSelection();
   }

   bool canInsertFromMimeData(const QMimeData *source) const override {
      QTextEdit *ed = qobject_cast<QTextEdit *>(parent());
      if (!ed) {
         return QTextControl::canInsertFromMimeData(source);
      }
      return ed->canInsertFromMimeData(source);
   }

   void insertFromMimeData(const QMimeData *source) override {
      QTextEdit *ed = qobject_cast<QTextEdit *>(parent());
      if (!ed) {
         QTextControl::insertFromMimeData(source);
      } else {
         ed->insertFromMimeData(source);
      }
   }
};

QTextEditPrivate::QTextEditPrivate()
   : control(nullptr), autoFormatting(QTextEdit::AutoNone), tabChangesFocus(false),
     lineWrap(QTextEdit::WidgetWidth), lineWrapColumnOrWidth(0),
     wordWrap(QTextOption::WrapAtWordBoundaryOrAnywhere), clickCausedFocus(0),
     textFormat(Qt::AutoText)
{
   ignoreAutomaticScrollbarAdjustment = false;
   preferRichText = false;
   showCursorOnInitialShow = true;
   inDrag = false;
}

void QTextEditPrivate::createAutoBulletList()
{
   QTextCursor cursor = control->textCursor();
   cursor.beginEditBlock();

   QTextBlockFormat blockFmt = cursor.blockFormat();

   QTextListFormat listFmt;
   listFmt.setStyle(QTextListFormat::ListDisc);
   listFmt.setIndent(blockFmt.indent() + 1);

   blockFmt.setIndent(0);
   cursor.setBlockFormat(blockFmt);

   cursor.createList(listFmt);

   cursor.endEditBlock();
   control->setTextCursor(cursor);
}

void QTextEditPrivate::init(const QString &html)
{
   Q_Q(QTextEdit);

   control = new QTextEditControl(q);
   control->setPalette(q->palette());

   QObject::connect(control, &QTextEditControl::microFocusChanged,     q, &QTextEdit::updateMicroFocus);
   QObject::connect(control, &QTextEditControl::documentSizeChanged,   q, &QTextEdit::_q_adjustScrollbars);
   QObject::connect(control, &QTextEditControl::updateRequest,         q, &QTextEdit::_q_repaintContents);
   QObject::connect(control, &QTextEditControl::visibilityRequest,     q, &QTextEdit::_q_ensureVisible);

   QObject::connect(control, &QTextEditControl::currentCharFormatChanged, q, &QTextEdit::_q_currentCharFormatChanged);

   QObject::connect(control, &QTextEditControl::textChanged,           q, &QTextEdit::textChanged);
   QObject::connect(control, &QTextEditControl::undoAvailable,         q, &QTextEdit::undoAvailable);
   QObject::connect(control, &QTextEditControl::redoAvailable,         q, &QTextEdit::redoAvailable);
   QObject::connect(control, &QTextEditControl::copyAvailable,         q, &QTextEdit::copyAvailable);
   QObject::connect(control, &QTextEditControl::selectionChanged,      q, &QTextEdit::selectionChanged);
   QObject::connect(control, &QTextEditControl::cursorPositionChanged, q, &QTextEdit::_q_cursorPositionChanged);
   QObject::connect(control, &QTextEditControl::textChanged,           q, &QTextEdit::updateMicroFocus);

   QTextDocument *doc = control->document();

   // set a null page size initially to avoid any relayouting until the textedit is shown
   // relayoutDocument() will take care of setting the page size to the viewport dimensions later.
   doc->setPageSize(QSize(0, 0));
   doc->documentLayout()->setPaintDevice(viewport);
   doc->setDefaultFont(q->font());
   doc->setUndoRedoEnabled(false); // flush undo buffer
   doc->setUndoRedoEnabled(true);

   if (! html.isEmpty()) {
      control->setHtml(html);
   }

   hbar->setSingleStep(20);
   vbar->setSingleStep(20);

   viewport->setBackgroundRole(QPalette::Base);
   q->setAcceptDrops(true);
   q->setFocusPolicy(Qt::StrongFocus);
   q->setAttribute(Qt::WA_KeyCompression);
   q->setAttribute(Qt::WA_InputMethodEnabled);
   q->setInputMethodHints(Qt::ImhMultiLine);

#ifndef QT_NO_CURSOR
   viewport->setCursor(Qt::IBeamCursor);
#endif
}

void QTextEditPrivate::_q_repaintContents(const QRectF &contentsRect)
{
   if (! contentsRect.isValid()) {
      viewport->update();
      return;
   }
   const int xOffset = horizontalOffset();
   const int yOffset = verticalOffset();
   const QRectF visibleRect(xOffset, yOffset, viewport->width(), viewport->height());

   QRect r = contentsRect.intersected(visibleRect).toAlignedRect();
   if (r.isEmpty()) {
      return;
   }

   r.translate(-xOffset, -yOffset);
   viewport->update(r);
}

void QTextEditPrivate::_q_cursorPositionChanged()
{
   Q_Q(QTextEdit);
   emit q->cursorPositionChanged();

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleTextCursorEvent event(q, q->textCursor().position());
   QAccessible::updateAccessibility(&event);
#endif
}

void QTextEditPrivate::pageUpDown(QTextCursor::MoveOperation op, QTextCursor::MoveMode moveMode)
{
   QTextCursor cursor = control->textCursor();
   bool moved = false;
   qreal lastY = control->cursorRect(cursor).top();
   qreal distance = 0;

   // move using movePosition to keep the cursor's x
   do {
      qreal y = control->cursorRect(cursor).top();
      distance += qAbs(y - lastY);
      lastY = y;
      moved = cursor.movePosition(op, moveMode);
   } while (moved && distance < viewport->height());

   if (moved) {
      if (op == QTextCursor::Up) {
         cursor.movePosition(QTextCursor::Down, moveMode);
         vbar->triggerAction(QAbstractSlider::SliderPageStepSub);
      } else {
         cursor.movePosition(QTextCursor::Up, moveMode);
         vbar->triggerAction(QAbstractSlider::SliderPageStepAdd);
      }
   }
   control->setTextCursor(cursor);
}

#ifndef QT_NO_SCROLLBAR
static QSize documentSize(QTextControl *control)
{
   QTextDocument *doc = control->document();
   QAbstractTextDocumentLayout *layout = doc->documentLayout();

   QSize docSize;

   if (QTextDocumentLayout *tlayout = qobject_cast<QTextDocumentLayout *>(layout)) {
      docSize = tlayout->dynamicDocumentSize().toSize();
      int percentageDone = tlayout->layoutStatus();
      // extrapolate height
      if (percentageDone > 0) {
         docSize.setHeight(docSize.height() * 100 / percentageDone);
      }

   } else {
      docSize = layout->documentSize().toSize();
   }

   return docSize;
}

void QTextEditPrivate::_q_adjustScrollbars()
{
   if (ignoreAutomaticScrollbarAdjustment) {
      return;
   }

   ignoreAutomaticScrollbarAdjustment = true; // avoid recursion, #106108

   QSize viewportSize = viewport->size();
   QSize docSize = documentSize(control);

   // due to the recursion guard we have to repeat this step a few times,
   // as adding/removing a scroll bar will cause the document or viewport
   // size to change
   // ideally we should loop until the viewport size and doc size stabilize,
   // but in corner cases they might fluctuate, so we need to limit the
   // number of iterations
   for (int i = 0; i < 4; ++i) {
      hbar->setRange(0, docSize.width() - viewportSize.width());
      hbar->setPageStep(viewportSize.width());

      vbar->setRange(0, docSize.height() - viewportSize.height());
      vbar->setPageStep(viewportSize.height());

      // if we are in left-to-right mode widening the document due to
      // lazy layouting does not require a repaint. If in right-to-left
      // the scroll bar has the value zero and it visually has the maximum
      // value (it is visually at the right), then widening the document
      // keeps it at value zero but visually adjusts it to the new maximum
      // on the right, hence we need an update.
      if (q_func()->isRightToLeft()) {
         viewport->update();
      }

      _q_showOrHideScrollBars();

      const QSize oldViewportSize = viewportSize;
      const QSize oldDocSize = docSize;

      // make sure the document is layouted if the viewport width changes
      viewportSize = viewport->size();
      if (viewportSize.width() != oldViewportSize.width()) {
         relayoutDocument();
      }

      docSize = documentSize(control);
      if (viewportSize == oldViewportSize && docSize == oldDocSize) {
         break;
      }
   }
   ignoreAutomaticScrollbarAdjustment = false;
}
#endif

// rect is in content coordinates
void QTextEditPrivate::_q_ensureVisible(const QRectF &_rect)
{
   const QRect rect = _rect.toRect();

   if ((vbar->isVisible() && vbar->maximum() < rect.bottom())
      || (hbar->isVisible() && hbar->maximum() < rect.right())) {
      _q_adjustScrollbars();
   }

   const int visibleWidth = viewport->width();
   const int visibleHeight = viewport->height();
   const bool rtl = q_func()->isRightToLeft();

   if (rect.x() < horizontalOffset()) {
      if (rtl) {
         hbar->setValue(hbar->maximum() - rect.x());
      } else {
         hbar->setValue(rect.x());
      }

   } else if (rect.x() + rect.width() > horizontalOffset() + visibleWidth) {
      if (rtl) {
         hbar->setValue(hbar->maximum() - (rect.x() + rect.width() - visibleWidth));
      } else {
         hbar->setValue(rect.x() + rect.width() - visibleWidth);
      }
   }

   if (rect.y() < verticalOffset()) {
      vbar->setValue(rect.y());
   } else if (rect.y() + rect.height() > verticalOffset() + visibleHeight) {
      vbar->setValue(rect.y() + rect.height() - visibleHeight);
   }
}

QTextEdit::QTextEdit(QWidget *parent)
   : QAbstractScrollArea(*new QTextEditPrivate, parent)
{
   Q_D(QTextEdit);
   d->init();
}

QTextEdit::QTextEdit(QTextEditPrivate &dd, QWidget *parent)
   : QAbstractScrollArea(dd, parent)
{
   Q_D(QTextEdit);
   d->init();
}

QTextEdit::QTextEdit(const QString &text, QWidget *parent)
   : QAbstractScrollArea(*new QTextEditPrivate, parent)
{
   Q_D(QTextEdit);
   d->init(text);
}

QTextEdit::~QTextEdit()
{
}

void QTextEdit::_q_repaintContents(const QRectF &r)
{
   Q_D(QTextEdit);
   d->_q_repaintContents(r);
}

void QTextEdit::_q_currentCharFormatChanged(const QTextCharFormat &textFormat)
{
   Q_D(QTextEdit);
   d->_q_currentCharFormatChanged(textFormat);
}

void QTextEdit::_q_adjustScrollbars()
{
   Q_D(QTextEdit);
   d->_q_adjustScrollbars();
}

void QTextEdit::_q_ensureVisible(const QRectF &rectF)
{
   Q_D(QTextEdit);
   d->_q_ensureVisible(rectF);
}

void QTextEdit::_q_cursorPositionChanged()
{
   Q_D(QTextEdit);
   d->_q_cursorPositionChanged();
}

qreal QTextEdit::fontPointSize() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().fontPointSize();
}

QString QTextEdit::fontFamily() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().fontFamily();
}

int QTextEdit::fontWeight() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().fontWeight();
}

bool QTextEdit::fontUnderline() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().fontUnderline();
}

bool QTextEdit::fontItalic() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().fontItalic();
}

QColor QTextEdit::textColor() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().foreground().color();
}

QColor QTextEdit::textBackgroundColor() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().background().color();
}

QFont QTextEdit::currentFont() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().font();
}

void QTextEdit::setAlignment(Qt::Alignment a)
{
   Q_D(QTextEdit);
   QTextBlockFormat fmt;
   fmt.setAlignment(a);
   QTextCursor cursor = d->control->textCursor();
   cursor.mergeBlockFormat(fmt);
   d->control->setTextCursor(cursor);
}

Qt::Alignment QTextEdit::alignment() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().blockFormat().alignment();
}

void QTextEdit::setDocument(QTextDocument *document)
{
   Q_D(QTextEdit);
   d->control->setDocument(document);
   d->updateDefaultTextOption();
   d->relayoutDocument();
}

QTextDocument *QTextEdit::document() const
{
   Q_D(const QTextEdit);
   return d->control->document();
}

QString QTextEdit::placeholderText() const
{
   Q_D(const QTextEdit);
   return d->placeholderText;
}

void QTextEdit::setPlaceholderText(const QString &placeholderText)
{
   Q_D(QTextEdit);

   if (d->placeholderText != placeholderText) {
      d->placeholderText = placeholderText;
      if (d->control->document()->isEmpty()) {
         d->viewport->update();
      }
   }
}

void QTextEdit::setTextCursor(const QTextCursor &cursor)
{
   doSetTextCursor(cursor);
}
void QTextEdit::doSetTextCursor(const QTextCursor &cursor)
{
   Q_D(QTextEdit);
   d->control->setTextCursor(cursor);
}

QTextCursor QTextEdit::textCursor() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor();
}

void QTextEdit::setFontFamily(const QString &fontFamily)
{
   QTextCharFormat fmt;
   fmt.setFontFamily(fontFamily);
   mergeCurrentCharFormat(fmt);
}

void QTextEdit::setFontPointSize(qreal s)
{
   QTextCharFormat fmt;
   fmt.setFontPointSize(s);
   mergeCurrentCharFormat(fmt);
}

void QTextEdit::setFontWeight(int w)
{
   QTextCharFormat fmt;
   fmt.setFontWeight(w);
   mergeCurrentCharFormat(fmt);
}

void QTextEdit::setFontUnderline(bool underline)
{
   QTextCharFormat fmt;
   fmt.setFontUnderline(underline);
   mergeCurrentCharFormat(fmt);
}

void QTextEdit::setFontItalic(bool italic)
{
   QTextCharFormat fmt;
   fmt.setFontItalic(italic);
   mergeCurrentCharFormat(fmt);
}

void QTextEdit::setTextColor(const QColor &c)
{
   QTextCharFormat fmt;
   fmt.setForeground(QBrush(c));
   mergeCurrentCharFormat(fmt);
}

void QTextEdit::setTextBackgroundColor(const QColor &c)
{
   QTextCharFormat fmt;
   fmt.setBackground(QBrush(c));
   mergeCurrentCharFormat(fmt);
}


void QTextEdit::setCurrentFont(const QFont &f)
{
   QTextCharFormat fmt;
   fmt.setFont(f);
   mergeCurrentCharFormat(fmt);
}

void QTextEdit::undo()
{
   Q_D(QTextEdit);
   d->control->undo();
}

void QTextEdit::redo()
{
   Q_D(QTextEdit);
   d->control->redo();
}

#ifndef QT_NO_CLIPBOARD
void QTextEdit::cut()
{
   Q_D(QTextEdit);
   d->control->cut();
}

void QTextEdit::copy()
{
   Q_D(QTextEdit);
   d->control->copy();
}

void QTextEdit::paste()
{
   Q_D(QTextEdit);
   d->control->paste();
}
#endif

void QTextEdit::clear()
{
   Q_D(QTextEdit);

   // clears and sets empty content
   d->control->clear();
}

void QTextEdit::selectAll()
{
   Q_D(QTextEdit);
   d->control->selectAll();
}

bool QTextEdit::event(QEvent *e)
{
   Q_D(QTextEdit);

#ifndef QT_NO_CONTEXTMENU
   if (e->type() == QEvent::ContextMenu && static_cast<QContextMenuEvent *>(e)->reason() == QContextMenuEvent::Keyboard) {
      ensureCursorVisible();

      const QPoint cursorPos = cursorRect().center();
      QContextMenuEvent ce(QContextMenuEvent::Keyboard, cursorPos, d->viewport->mapToGlobal(cursorPos));
      ce.setAccepted(e->isAccepted());

      const bool result = QAbstractScrollArea::event(&ce);
      e->setAccepted(ce.isAccepted());

      return result;

   } else if (e->type() == QEvent::ShortcutOverride || e->type() == QEvent::ToolTip) {
      d->sendControlEvent(e);
   }
#endif

#ifdef QT_KEYPAD_NAVIGATION
   if (e->type() == QEvent::EnterEditFocus || e->type() == QEvent::LeaveEditFocus) {
      if (QApplication::keypadNavigationEnabled()) {
         d->sendControlEvent(e);
      }
   }
#endif

   return QAbstractScrollArea::event(e);
}

void QTextEdit::timerEvent(QTimerEvent *e)
{
   Q_D(QTextEdit);

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

         QMouseEvent ev(QEvent::MouseMove, pos, mapTo(topLevelWidget(), pos), globalPos,
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

void QTextEdit::setPlainText(const QString &text)
{
   Q_D(QTextEdit);
   d->control->setPlainText(text);
   d->preferRichText = false;
}

QString QTextEdit::toPlainText() const
{
   Q_D(const QTextEdit);
   return d->control->toPlainText();
}

#ifndef QT_NO_TEXTHTMLPARSER
void QTextEdit::setHtml(const QString &text)
{
   Q_D(QTextEdit);
   d->control->setHtml(text);
   d->preferRichText = true;
}

QString QTextEdit::toHtml() const
{
   Q_D(const QTextEdit);
   return d->control->toHtml();
}
#endif

void QTextEdit::keyPressEvent(QKeyEvent *e)
{
   Q_D(QTextEdit);

#ifdef QT_KEYPAD_NAVIGATION
   switch (e->key()) {
      case Qt::Key_Select:
         if (QApplication::keypadNavigationEnabled()) {
            // code assumes linksaccessible + editable isn't meaningful
            if (d->control->textInteractionFlags() & Qt::TextEditable) {
               setEditFocus(!hasEditFocus());
            } else {
               if (!hasEditFocus()) {
                  setEditFocus(true);
               } else {
                  QTextCursor cursor = d->control->textCursor();
                  QTextCharFormat charFmt = cursor.charFormat();
                  if (!(d->control->textInteractionFlags() & Qt::LinksAccessibleByKeyboard)
                     || !cursor.hasSelection() || charFmt.anchorHref().isEmpty()) {
                     e->accept();
                     return;
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
#endif

   {
      QTextCursor cursor = d->control->textCursor();
      const QString text = e->text();

      if (cursor.atBlockStart() && (d->autoFormatting & AutoBulletList && text.length() == 1)) {
         QChar ch = text.first();

         if ((ch == '-' || ch == '*') && (! cursor.currentList())) {
            d->createAutoBulletList();
            e->accept();
            return;
         }
      }
   }

   d->sendControlEvent(e);

#ifdef QT_KEYPAD_NAVIGATION
   if (! e->isAccepted()) {
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
         case Qt::Key_Back:
            if (!e->isAutoRepeat()) {
               if (QApplication::keypadNavigationEnabled()) {
                  if (document()->isEmpty() || !(d->control->textInteractionFlags() & Qt::TextEditable)) {
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

void QTextEdit::keyReleaseEvent(QKeyEvent *e)
{
#ifdef QT_KEYPAD_NAVIGATION
   Q_D(QTextEdit);

   if (QApplication::keypadNavigationEnabled()) {
      if (!e->isAutoRepeat() && e->key() == Qt::Key_Back && d->deleteAllTimer.isActive()) {
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
         e->accept();
         return;
      }
   }
#endif

   e->ignore();
}

QVariant QTextEdit::loadResource(int type, const QUrl &name)
{
   (void) type;
   (void) name;

   return QVariant();
}

void QTextEdit::resizeEvent(QResizeEvent *e)
{
   Q_D(QTextEdit);

   if (d->lineWrap == NoWrap) {
      QTextDocument *doc = d->control->document();
      QVariant alignmentProperty = doc->documentLayout()->property("contentHasAlignment");

      if (!doc->pageSize().isNull()
         && alignmentProperty.type() == QVariant::Bool
         && !alignmentProperty.toBool()) {

         d->_q_adjustScrollbars();
         return;
      }
   }

   if (d->lineWrap != FixedPixelWidth
      && e->oldSize().width() != e->size().width()) {
      d->relayoutDocument();
   } else {
      d->_q_adjustScrollbars();
   }
}

void QTextEditPrivate::relayoutDocument()
{
   QTextDocument *doc = control->document();
   QAbstractTextDocumentLayout *layout = doc->documentLayout();

   if (QTextDocumentLayout *tlayout = qobject_cast<QTextDocumentLayout *>(layout)) {
      if (lineWrap == QTextEdit::FixedColumnWidth) {
         tlayout->setFixedColumnWidth(lineWrapColumnOrWidth);
      } else {
         tlayout->setFixedColumnWidth(-1);
      }
   }

   QTextDocumentLayout *tlayout = qobject_cast<QTextDocumentLayout *>(layout);
   QSize lastUsedSize;
   if (tlayout) {
      lastUsedSize = tlayout->dynamicDocumentSize().toSize();
   } else {
      lastUsedSize = layout->documentSize().toSize();
   }

   // ignore calls to _q_adjustScrollbars caused by an emission of the
   // usedSizeChanged() signal in the layout, as we're calling it
   // later on our own anyway (or deliberately not) .
   const bool oldIgnoreScrollbarAdjustment = ignoreAutomaticScrollbarAdjustment;
   ignoreAutomaticScrollbarAdjustment = true;

   int width = viewport->width();
   if (lineWrap == QTextEdit::FixedPixelWidth) {
      width = lineWrapColumnOrWidth;
   } else if (lineWrap == QTextEdit::NoWrap) {
      QVariant alignmentProperty = doc->documentLayout()->property("contentHasAlignment");
      if (alignmentProperty.type() == QVariant::Bool && !alignmentProperty.toBool()) {

         width = 0;
      }
   }

   doc->setPageSize(QSize(width, -1));
   if (tlayout) {
      tlayout->ensureLayouted(verticalOffset() + viewport->height());
   }

   ignoreAutomaticScrollbarAdjustment = oldIgnoreScrollbarAdjustment;

   QSize usedSize;
   if (tlayout) {
      usedSize = tlayout->dynamicDocumentSize().toSize();
   } else {
      usedSize = layout->documentSize().toSize();
   }

   // this is an obscure situation in the layout that can happen:
   // if a character at the end of a line is the tallest one and therefore
   // influencing the total height of the line and the line right below it
   // is always taller though, then it can happen that if due to line breaking
   // that tall character wraps into the lower line the document not only shrinks
   // horizontally (causing the character to wrap in the first place) but also
   // vertically, because the original line is now smaller and the one below kept
   // its size. So a layout with less width _can_ take up less vertical space, too.
   // If the wider case causes a vertical scroll bar to appear and the narrower one
   // (narrower because the vertical scroll bar takes up horizontal space)) to disappear
   // again then we have an endless loop, as _q_adjustScrollBars sets new ranges on the
   // scroll bars, the QAbstractScrollArea will find out about it and try to show/hide
   // the scroll bars again. That's why we try to detect this case here and break out.
   //
   // (if you change this please also check the layoutingLoop() testcase in
   // QTextEdit's autotests)
   if (lastUsedSize.isValid()
      && !vbar->isHidden()
      && viewport->width() < lastUsedSize.width()
      && usedSize.height() < lastUsedSize.height()
      && usedSize.height() <= viewport->height()) {
      return;
   }

   _q_adjustScrollbars();
}

void QTextEditPrivate::paint(QPainter *p, QPaintEvent *e)
{
   const int xOffset = horizontalOffset();
   const int yOffset = verticalOffset();

   QRect r = e->rect();
   p->translate(-xOffset, -yOffset);
   r.translate(xOffset, yOffset);

   QTextDocument *doc = control->document();
   QTextDocumentLayout *layout = qobject_cast<QTextDocumentLayout *>(doc->documentLayout());

   // the layout might need to expand the root frame to
   // the viewport if NoWrap is set
   if (layout) {
      layout->setViewport(viewport->rect());
   }

   control->drawContents(p, r, q_func());

   if (layout) {
      layout->setViewport(QRect());
   }

   if (!placeholderText.isEmpty() && doc->isEmpty() && !control->isPreediting()) {
      QColor col = control->palette().text().color();
      col.setAlpha(128);
      p->setPen(col);
      const int margin = int(doc->documentMargin());
      p->drawText(viewport->rect().adjusted(margin, margin, -margin, -margin), Qt::AlignTop | Qt::TextWordWrap, placeholderText);
   }
}


void QTextEdit::paintEvent(QPaintEvent *e)
{
   Q_D(QTextEdit);
   QPainter p(d->viewport);
   d->paint(&p, e);
}

void QTextEditPrivate::_q_currentCharFormatChanged(const QTextCharFormat &fmt)
{
   Q_Q(QTextEdit);
   emit q->currentCharFormatChanged(fmt);
}

void QTextEditPrivate::updateDefaultTextOption()
{
   QTextDocument *doc = control->document();

   QTextOption opt = doc->defaultTextOption();
   QTextOption::WrapMode oldWrapMode = opt.wrapMode();

   if (lineWrap == QTextEdit::NoWrap) {
      opt.setWrapMode(QTextOption::NoWrap);
   } else {
      opt.setWrapMode(wordWrap);
   }

   if (opt.wrapMode() != oldWrapMode) {
      doc->setDefaultTextOption(opt);
   }
}

void QTextEdit::mousePressEvent(QMouseEvent *e)
{
   Q_D(QTextEdit);

#ifdef QT_KEYPAD_NAVIGATION
   if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
      setEditFocus(true);
   }
#endif
   d->sendControlEvent(e);
}

void QTextEdit::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QTextEdit);

   d->inDrag = false;
   const QPoint pos = e->pos();
   d->sendControlEvent(e);

   if (! (e->buttons() & Qt::LeftButton)) {
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

void QTextEdit::mouseReleaseEvent(QMouseEvent *e)
{
   Q_D(QTextEdit);
   d->sendControlEvent(e);

   if (e->source() == Qt::MouseEventNotSynthesized && d->autoScrollTimer.isActive()) {
      d->autoScrollTimer.stop();
      ensureCursorVisible();
   }

   if (!isReadOnly() && rect().contains(e->pos())) {
      d->handleSoftwareInputPanel(e->button(), d->clickCausedFocus);
   }
   d->clickCausedFocus = 0;
}

void QTextEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
   Q_D(QTextEdit);
   d->sendControlEvent(e);
}

bool QTextEdit::focusNextPrevChild(bool next)
{
   Q_D(const QTextEdit);
   if (!d->tabChangesFocus && d->control->textInteractionFlags() & Qt::TextEditable) {
      return false;
   }
   return QAbstractScrollArea::focusNextPrevChild(next);
}

#ifndef QT_NO_CONTEXTMENU

void QTextEdit::contextMenuEvent(QContextMenuEvent *e)
{
   Q_D(QTextEdit);
   d->sendControlEvent(e);
}
#endif

#ifndef QT_NO_DRAGANDDROP

void QTextEdit::dragEnterEvent(QDragEnterEvent *e)
{
   Q_D(QTextEdit);
   d->inDrag = true;
   d->sendControlEvent(e);
}

void QTextEdit::dragLeaveEvent(QDragLeaveEvent *e)
{
   Q_D(QTextEdit);
   d->inDrag = false;
   d->autoScrollTimer.stop();
   d->sendControlEvent(e);
}

void QTextEdit::dragMoveEvent(QDragMoveEvent *e)
{
   Q_D(QTextEdit);
   d->autoScrollDragPos = e->pos();
   if (!d->autoScrollTimer.isActive()) {
      d->autoScrollTimer.start(100, this);
   }
   d->sendControlEvent(e);
}

void QTextEdit::dropEvent(QDropEvent *e)
{
   Q_D(QTextEdit);
   d->inDrag = false;
   d->autoScrollTimer.stop();
   d->sendControlEvent(e);
}

#endif

void QTextEdit::inputMethodEvent(QInputMethodEvent *e)
{
   Q_D(QTextEdit);

#ifdef QT_KEYPAD_NAVIGATION
   if (d->control->textInteractionFlags() & Qt::TextEditable
      && QApplication::keypadNavigationEnabled()
      && !hasEditFocus()) {
      setEditFocus(true);
   }
#endif

   d->sendControlEvent(e);
   ensureCursorVisible();
}

void QTextEdit::scrollContentsBy(int dx, int dy)
{
   Q_D(QTextEdit);

   if (isRightToLeft()) {
      dx = -dx;
   }

   d->viewport->scroll(dx, dy);
}

QVariant QTextEdit::inputMethodQuery(Qt::InputMethodQuery property) const
{
   return inputMethodQuery(property, QVariant());
}

QVariant QTextEdit::inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const
{
   Q_D(const QTextEdit);

   if (query == Qt::ImHints) {
      return QWidget::inputMethodQuery(query);
   }

   const QVariant v = d->control->inputMethodQuery(query, argument);
   const QPointF offset(-d->horizontalOffset(), -d->verticalOffset());

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

void QTextEdit::focusInEvent(QFocusEvent *e)
{
   Q_D(QTextEdit);
   if (e->reason() == Qt::MouseFocusReason) {
      d->clickCausedFocus = 1;
   }
   QAbstractScrollArea::focusInEvent(e);
   d->sendControlEvent(e);
}

void QTextEdit::focusOutEvent(QFocusEvent *e)
{
   Q_D(QTextEdit);
   QAbstractScrollArea::focusOutEvent(e);
   d->sendControlEvent(e);
}

void QTextEdit::showEvent(QShowEvent *)
{
   Q_D(QTextEdit);
   if (!d->anchorToScrollToWhenVisible.isEmpty()) {
      scrollToAnchor(d->anchorToScrollToWhenVisible);
      d->anchorToScrollToWhenVisible.clear();
      d->showCursorOnInitialShow = false;
   } else if (d->showCursorOnInitialShow) {
      d->showCursorOnInitialShow = false;
      ensureCursorVisible();
   }
}

void QTextEdit::changeEvent(QEvent *e)
{
   Q_D(QTextEdit);
   QAbstractScrollArea::changeEvent(e);

   if (e->type() == QEvent::ApplicationFontChange || e->type() == QEvent::FontChange) {
      d->control->document()->setDefaultFont(font());

   } else if (e->type() == QEvent::ActivationChange) {
      if (!isActiveWindow()) {
         d->autoScrollTimer.stop();
      }

   } else if (e->type() == QEvent::EnabledChange) {
      e->setAccepted(isEnabled());
      d->control->setPalette(palette());
      d->sendControlEvent(e);

   } else if (e->type() == QEvent::PaletteChange) {
      d->control->setPalette(palette());

   } else if (e->type() == QEvent::LayoutDirectionChange) {
      d->sendControlEvent(e);
   }
}

#ifndef QT_NO_WHEELEVENT
void QTextEdit::wheelEvent(QWheelEvent *e)
{
   Q_D(QTextEdit);
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

#ifndef QT_NO_CONTEXTMENU
QMenu *QTextEdit::createStandardContextMenu()
{
   Q_D(QTextEdit);
   return d->control->createStandardContextMenu(QPointF(), this);
}

QMenu *QTextEdit::createStandardContextMenu(const QPoint &position)
{
   Q_D(QTextEdit);
   return d->control->createStandardContextMenu(position, this);
}
#endif

QTextCursor QTextEdit::cursorForPosition(const QPoint &pos) const
{
   Q_D(const QTextEdit);
   return d->control->cursorForPosition(d->mapToContents(pos));
}

QRect QTextEdit::cursorRect(const QTextCursor &cursor) const
{
   Q_D(const QTextEdit);
   if (cursor.isNull()) {
      return QRect();
   }

   QRect r = d->control->cursorRect(cursor).toRect();
   r.translate(-d->horizontalOffset(), -d->verticalOffset());
   return r;
}

QRect QTextEdit::cursorRect() const
{
   Q_D(const QTextEdit);
   QRect r = d->control->cursorRect().toRect();
   r.translate(-d->horizontalOffset(), -d->verticalOffset());
   return r;
}

QString QTextEdit::anchorAt(const QPoint &pos) const
{
   Q_D(const QTextEdit);
   return d->control->anchorAt(d->mapToContents(pos));
}

bool QTextEdit::overwriteMode() const
{
   Q_D(const QTextEdit);
   return d->control->overwriteMode();
}

void QTextEdit::setOverwriteMode(bool overwrite)
{
   Q_D(QTextEdit);
   d->control->setOverwriteMode(overwrite);
}

int QTextEdit::tabStopWidth() const
{
   Q_D(const QTextEdit);
   return qRound(d->control->document()->defaultTextOption().tabStop());
}

void QTextEdit::setTabStopWidth(int width)
{
   Q_D(QTextEdit);
   QTextOption opt = d->control->document()->defaultTextOption();
   if (opt.tabStop() == width || width < 0) {
      return;
   }
   opt.setTabStop(width);
   d->control->document()->setDefaultTextOption(opt);
}

int QTextEdit::cursorWidth() const
{
   Q_D(const QTextEdit);
   return d->control->cursorWidth();
}

void QTextEdit::setCursorWidth(int width)
{
   Q_D(QTextEdit);
   d->control->setCursorWidth(width);
}

bool QTextEdit::acceptRichText() const
{
   Q_D(const QTextEdit);
   return d->control->acceptRichText();
}

void QTextEdit::setAcceptRichText(bool accept)
{
   Q_D(QTextEdit);
   d->control->setAcceptRichText(accept);
}

void QTextEdit::setExtraSelections(const QList<ExtraSelection> &selections)
{
   Q_D(QTextEdit);
   d->control->setExtraSelections(selections);
}

QList<QTextEdit::ExtraSelection> QTextEdit::extraSelections() const
{
   Q_D(const QTextEdit);
   return d->control->extraSelections();
}

QMimeData *QTextEdit::createMimeDataFromSelection() const
{
   Q_D(const QTextEdit);
   return d->control->QTextControl::createMimeDataFromSelection();
}

bool QTextEdit::canInsertFromMimeData(const QMimeData *source) const
{
   Q_D(const QTextEdit);
   return d->control->QTextControl::canInsertFromMimeData(source);
}

void QTextEdit::insertFromMimeData(const QMimeData *source)
{
   Q_D(QTextEdit);
   d->control->QTextControl::insertFromMimeData(source);
}

bool QTextEdit::isReadOnly() const
{
   Q_D(const QTextEdit);
   return !(d->control->textInteractionFlags() & Qt::TextEditable);
}

void QTextEdit::setReadOnly(bool ro)
{
   Q_D(QTextEdit);
   Qt::TextInteractionFlags flags = Qt::NoTextInteraction;
   if (ro) {
      flags = Qt::TextSelectableByMouse;
#ifndef QT_NO_TEXTBROWSER
      if (qobject_cast<QTextBrowser *>(this)) {
         flags |= Qt::TextBrowserInteraction;
      }
#endif
   } else {
      flags = Qt::TextEditorInteraction;
   }
   d->control->setTextInteractionFlags(flags);
   setAttribute(Qt::WA_InputMethodEnabled, shouldEnableInputMethod(this));
   QEvent event(QEvent::ReadOnlyChange);
   QApplication::sendEvent(this, &event);
}

void QTextEdit::setTextInteractionFlags(Qt::TextInteractionFlags flags)
{
   Q_D(QTextEdit);
   d->control->setTextInteractionFlags(flags);
}

Qt::TextInteractionFlags QTextEdit::textInteractionFlags() const
{
   Q_D(const QTextEdit);
   return d->control->textInteractionFlags();
}

void QTextEdit::mergeCurrentCharFormat(const QTextCharFormat &modifier)
{
   Q_D(QTextEdit);
   d->control->mergeCurrentCharFormat(modifier);
}

void QTextEdit::setCurrentCharFormat(const QTextCharFormat &format)
{
   Q_D(QTextEdit);
   d->control->setCurrentCharFormat(format);
}

QTextCharFormat QTextEdit::currentCharFormat() const
{
   Q_D(const QTextEdit);
   return d->control->currentCharFormat();
}

QTextEdit::AutoFormatting QTextEdit::autoFormatting() const
{
   Q_D(const QTextEdit);
   return d->autoFormatting;
}

void QTextEdit::setAutoFormatting(AutoFormatting features)
{
   Q_D(QTextEdit);
   d->autoFormatting = features;
}

void QTextEdit::insertPlainText(const QString &text)
{
   Q_D(QTextEdit);
   d->control->insertPlainText(text);
}

#ifndef QT_NO_TEXTHTMLPARSER
void QTextEdit::insertHtml(const QString &text)
{
   Q_D(QTextEdit);
   d->control->insertHtml(text);
}
#endif

void QTextEdit::scrollToAnchor(const QString &name)
{
   Q_D(QTextEdit);
   if (name.isEmpty()) {
      return;
   }

   if (!isVisible()) {
      d->anchorToScrollToWhenVisible = name;
      return;
   }

   QPointF p = d->control->anchorPosition(name);
   const int newPosition = qRound(p.y());
   if ( d->vbar->maximum() < newPosition ) {
      d->_q_adjustScrollbars();
   }

   d->vbar->setValue(newPosition);
}

void QTextEdit::zoomIn(int range)
{
   zoomInF(range);
}

void QTextEdit::zoomOut(int range)
{
   zoomInF(-range);
}

void QTextEdit::zoomInF(float range)
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

void QTextEdit::moveCursor(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode)
{
   Q_D(QTextEdit);
   d->control->moveCursor(operation, mode);
}

bool QTextEdit::canPaste() const
{
   Q_D(const QTextEdit);
   return d->control->canPaste();
}

#ifndef QT_NO_PRINTER

void QTextEdit::print(QPagedPaintDevice *printer) const
{
   Q_D(const QTextEdit);
   d->control->print(printer);
}
#endif

bool QTextEdit::tabChangesFocus() const
{
   Q_D(const QTextEdit);
   return d->tabChangesFocus;
}

void QTextEdit::setTabChangesFocus(bool b)
{
   Q_D(QTextEdit);
   d->tabChangesFocus = b;
}

QTextEdit::LineWrapMode QTextEdit::lineWrapMode() const
{
   Q_D(const QTextEdit);
   return d->lineWrap;
}

void QTextEdit::setLineWrapMode(LineWrapMode wrap)
{
   Q_D(QTextEdit);
   if (d->lineWrap == wrap) {
      return;
   }
   d->lineWrap = wrap;
   d->updateDefaultTextOption();
   d->relayoutDocument();
}

int QTextEdit::lineWrapColumnOrWidth() const
{
   Q_D(const QTextEdit);
   return d->lineWrapColumnOrWidth;
}

void QTextEdit::setLineWrapColumnOrWidth(int w)
{
   Q_D(QTextEdit);
   d->lineWrapColumnOrWidth = w;
   d->relayoutDocument();
}

QTextOption::WrapMode QTextEdit::wordWrapMode() const
{
   Q_D(const QTextEdit);
   return d->wordWrap;
}

void QTextEdit::setWordWrapMode(QTextOption::WrapMode mode)
{
   Q_D(QTextEdit);
   if (mode == d->wordWrap) {
      return;
   }
   d->wordWrap = mode;
   d->updateDefaultTextOption();
}

bool QTextEdit::find(const QString &exp, QTextDocument::FindFlags options)
{
   Q_D(QTextEdit);
   return d->control->find(exp, options);
}

bool QTextEdit::find(const QRegularExpression &exp, QTextDocument::FindFlags options)
{
   Q_D(QTextEdit);
   return d->control->find(exp, options);
}

void QTextEdit::setText(const QString &text)
{
   Q_D(QTextEdit);
   Qt::TextFormat format = d->textFormat;

   if (d->textFormat == Qt::AutoText) {
      format = Qt::mightBeRichText(text) ? Qt::RichText : Qt::PlainText;
   }

#ifndef QT_NO_TEXTHTMLPARSER
   if (format == Qt::RichText) {
      setHtml(text);
   } else
#endif
      setPlainText(text);
}

void QTextEdit::append(const QString &text)
{
   Q_D(QTextEdit);
   const bool atBottom = isReadOnly() ?  d->verticalOffset() >= d->vbar->maximum()
      : d->control->textCursor().atEnd();

   d->control->append(text);

   if (atBottom) {
      d->vbar->setValue(d->vbar->maximum());
   }
}

void QTextEdit::ensureCursorVisible()
{
   Q_D(QTextEdit);
   d->control->ensureCursorVisible();
}

#endif
