/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
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
#include <qmime.h>
#include <qdrag.h>
#include <qclipboard.h>
#include <qmenu.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qtextdocumentlayout_p.h>
#include <qtextdocument.h>
#include <qtextdocument_p.h>
#include <qtextlist.h>
#include <qtextcontrol_p.h>

#include <qtextformat.h>
#include <qdatetime.h>
#include <qapplication.h>
#include <limits.h>
#include <qtexttable.h>
#include <qvariant.h>
#include <qinputcontext.h>
#endif

QT_BEGIN_NAMESPACE

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
   : control(0),
     autoFormatting(QTextEdit::AutoNone), tabChangesFocus(false),
     lineWrap(QTextEdit::WidgetWidth), lineWrapColumnOrWidth(0),
     wordWrap(QTextOption::WrapAtWordBoundaryOrAnywhere), clickCausedFocus(0)
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

   QObject::connect(control, SIGNAL(microFocusChanged()),                 q, SLOT(updateMicroFocus()));
   QObject::connect(control, SIGNAL(documentSizeChanged(const QSizeF &)), q, SLOT(_q_adjustScrollbars()));
   QObject::connect(control, SIGNAL(updateRequest(const QRectF &)),       q, SLOT(_q_repaintContents(const QRectF &)));
   QObject::connect(control, SIGNAL(visibilityRequest(const QRectF &)),   q, SLOT(_q_ensureVisible(const QRectF &)));

   QObject::connect(control, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)),
                    q, SLOT(_q_currentCharFormatChanged(const QTextCharFormat &)));

   QObject::connect(control, SIGNAL(textChanged()),           q, SLOT(textChanged()));
   QObject::connect(control, SIGNAL(undoAvailable(bool)),     q, SLOT(undoAvailable(bool)));
   QObject::connect(control, SIGNAL(redoAvailable(bool)),     q, SLOT(redoAvailable(bool)));
   QObject::connect(control, SIGNAL(copyAvailable(bool)),     q, SLOT(copyAvailable(bool)));
   QObject::connect(control, SIGNAL(selectionChanged()),      q, SLOT(selectionChanged()));
   QObject::connect(control, SIGNAL(cursorPositionChanged()), q, SLOT(cursorPositionChanged()));
   QObject::connect(control, SIGNAL(textChanged()),           q, SLOT(updateMicroFocus()));

   QTextDocument *doc = control->document();
   // set a null page size initially to avoid any relayouting until the textedit
   // is shown. relayoutDocument() will take care of setting the page size to the
   // viewport dimensions later.
   doc->setPageSize(QSize(0, 0));
   doc->documentLayout()->setPaintDevice(viewport);
   doc->setDefaultFont(q->font());
   doc->setUndoRedoEnabled(false); // flush undo buffer.
   doc->setUndoRedoEnabled(true);

   if (!html.isEmpty()) {
      control->setHtml(html);
   }

   hbar->setSingleStep(20);
   vbar->setSingleStep(20);

   viewport->setBackgroundRole(QPalette::Base);
   q->setAcceptDrops(true);
   q->setFocusPolicy(Qt::WheelFocus);
   q->setAttribute(Qt::WA_KeyCompression);
   q->setAttribute(Qt::WA_InputMethodEnabled);

#ifndef QT_NO_CURSOR
   viewport->setCursor(Qt::IBeamCursor);
#endif

#ifdef Q_OS_WIN
   setSingleFingerPanEnabled(true);
#endif
}

void QTextEditPrivate::_q_repaintContents(const QRectF &contentsRect)
{
   if (!contentsRect.isValid()) {
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

/*!
    \internal
*/
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

void QTextEdit::_q_currentCharFormatChanged(const QTextCharFormat &un_named_arg1)
{
   Q_D(QTextEdit);
   d->_q_currentCharFormatChanged(un_named_arg1);
}

void QTextEdit::_q_adjustScrollbars()
{
   Q_D(QTextEdit);
   d->_q_adjustScrollbars();
}

void QTextEdit::_q_ensureVisible(const QRectF &un_named_arg1)
{
   Q_D(QTextEdit);
   d->_q_ensureVisible(un_named_arg1);
}

/*!
    Returns the point size of the font of the current format.

    \sa setFontFamily() setCurrentFont() setFontPointSize()
*/
qreal QTextEdit::fontPointSize() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().fontPointSize();
}

/*!
    Returns the font family of the current format.

    \sa setFontFamily() setCurrentFont() setFontPointSize()
*/
QString QTextEdit::fontFamily() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().fontFamily();
}

/*!
    Returns the font weight of the current format.

    \sa setFontWeight() setCurrentFont() setFontPointSize() QFont::Weight
*/
int QTextEdit::fontWeight() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().fontWeight();
}

/*!
    Returns true if the font of the current format is underlined; otherwise returns
    false.

    \sa setFontUnderline()
*/
bool QTextEdit::fontUnderline() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().fontUnderline();
}

/*!
    Returns true if the font of the current format is italic; otherwise returns
    false.

    \sa setFontItalic()
*/
bool QTextEdit::fontItalic() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().fontItalic();
}

/*!
    Returns the text color of the current format.

    \sa setTextColor()
*/
QColor QTextEdit::textColor() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().foreground().color();
}

/*!
    \since 4.4

    Returns the text background color of the current format.

    \sa setTextBackgroundColor()
*/
QColor QTextEdit::textBackgroundColor() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().background().color();
}

/*!
    Returns the font of the current format.

    \sa setCurrentFont() setFontFamily() setFontPointSize()
*/
QFont QTextEdit::currentFont() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().charFormat().font();
}

/*!
    Sets the alignment of the current paragraph to \a a. Valid
    alignments are Qt::AlignLeft, Qt::AlignRight,
    Qt::AlignJustify and Qt::AlignCenter (which centers
    horizontally).
*/
void QTextEdit::setAlignment(Qt::Alignment a)
{
   Q_D(QTextEdit);
   QTextBlockFormat fmt;
   fmt.setAlignment(a);
   QTextCursor cursor = d->control->textCursor();
   cursor.mergeBlockFormat(fmt);
   d->control->setTextCursor(cursor);
}

/*!
    Returns the alignment of the current paragraph.

    \sa setAlignment()
*/
Qt::Alignment QTextEdit::alignment() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor().blockFormat().alignment();
}

/*!
    Makes \a document the new document of the text editor.

    \note The editor \e{does not take ownership of the document} unless it
    is the document's parent object. The parent object of the provided document
    remains the owner of the object.

    The editor does not delete the current document, even if it is a child of the editor.

    \sa document()
*/
void QTextEdit::setDocument(QTextDocument *document)
{
   Q_D(QTextEdit);
   d->control->setDocument(document);
   d->updateDefaultTextOption();
   d->relayoutDocument();
}

/*!
    Returns a pointer to the underlying document.

    \sa setDocument()
*/
QTextDocument *QTextEdit::document() const
{
   Q_D(const QTextEdit);
   return d->control->document();
}

/*!
    Sets the visible \a cursor.
*/
void QTextEdit::setTextCursor(const QTextCursor &cursor)
{
   Q_D(QTextEdit);
   d->control->setTextCursor(cursor);
}

/*!
    Returns a copy of the QTextCursor that represents the currently visible cursor.
    Note that changes on the returned cursor do not affect QTextEdit's cursor; use
    setTextCursor() to update the visible cursor.
 */
QTextCursor QTextEdit::textCursor() const
{
   Q_D(const QTextEdit);
   return d->control->textCursor();
}

/*!
    Sets the font family of the current format to \a fontFamily.

    \sa fontFamily() setCurrentFont()
*/
void QTextEdit::setFontFamily(const QString &fontFamily)
{
   QTextCharFormat fmt;
   fmt.setFontFamily(fontFamily);
   mergeCurrentCharFormat(fmt);
}

/*!
    Sets the point size of the current format to \a s.

    Note that if \a s is zero or negative, the behavior of this
    function is not defined.

    \sa fontPointSize() setCurrentFont() setFontFamily()
*/
void QTextEdit::setFontPointSize(qreal s)
{
   QTextCharFormat fmt;
   fmt.setFontPointSize(s);
   mergeCurrentCharFormat(fmt);
}

/*!
    \fn void QTextEdit::setFontWeight(int weight)

    Sets the font weight of the current format to the given \a weight,
    where the value used is in the range defined by the QFont::Weight
    enum.

    \sa fontWeight(), setCurrentFont(), setFontFamily()
*/
void QTextEdit::setFontWeight(int w)
{
   QTextCharFormat fmt;
   fmt.setFontWeight(w);
   mergeCurrentCharFormat(fmt);
}

/*!
    If \a underline is true, sets the current format to underline;
    otherwise sets the current format to non-underline.

    \sa fontUnderline()
*/
void QTextEdit::setFontUnderline(bool underline)
{
   QTextCharFormat fmt;
   fmt.setFontUnderline(underline);
   mergeCurrentCharFormat(fmt);
}

/*!
    If \a italic is true, sets the current format to italic;
    otherwise sets the current format to non-italic.

    \sa fontItalic()
*/
void QTextEdit::setFontItalic(bool italic)
{
   QTextCharFormat fmt;
   fmt.setFontItalic(italic);
   mergeCurrentCharFormat(fmt);
}

/*!
    Sets the text color of the current format to \a c.

    \sa textColor()
*/
void QTextEdit::setTextColor(const QColor &c)
{
   QTextCharFormat fmt;
   fmt.setForeground(QBrush(c));
   mergeCurrentCharFormat(fmt);
}

/*!
    \since 4.4

    Sets the text background color of the current format to \a c.

    \sa textBackgroundColor()
*/
void QTextEdit::setTextBackgroundColor(const QColor &c)
{
   QTextCharFormat fmt;
   fmt.setBackground(QBrush(c));
   mergeCurrentCharFormat(fmt);
}

/*!
    Sets the font of the current format to \a f.

    \sa currentFont() setFontPointSize() setFontFamily()
*/
void QTextEdit::setCurrentFont(const QFont &f)
{
   QTextCharFormat fmt;
   fmt.setFont(f);
   mergeCurrentCharFormat(fmt);
}

/*!
    \since 4.2

    Undoes the last operation.

    If there is no operation to undo, i.e. there is no undo step in
    the undo/redo history, nothing happens.

    \sa redo()
*/
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

/*!
    \fn void QTextEdit::undo() const
    \fn void QTextEdit::redo() const
    \overload

    Use the non-const overload instead.
*/

/*!
    \fn void QTextEdit::redo()
    \since 4.2

    Redoes the last operation.

    If there is no operation to redo, i.e. there is no redo step in
    the undo/redo history, nothing happens.

    \sa undo()
*/

#ifndef QT_NO_CLIPBOARD
/*!
    Copies the selected text to the clipboard and deletes it from
    the text edit.

    If there is no selected text nothing happens.

    \sa copy() paste()
*/

void QTextEdit::cut()
{
   Q_D(QTextEdit);
   d->control->cut();
}

/*!
    Copies any selected text to the clipboard.

    \sa copyAvailable()
*/

void QTextEdit::copy()
{
   Q_D(QTextEdit);
   d->control->copy();
}

/*!
    Pastes the text from the clipboard into the text edit at the
    current cursor position.

    If there is no text in the clipboard nothing happens.

    To change the behavior of this function, i.e. to modify what
    QTextEdit can paste and how it is being pasted, reimplement the
    virtual canInsertFromMimeData() and insertFromMimeData()
    functions.

    \sa cut() copy()
*/

void QTextEdit::paste()
{
   Q_D(QTextEdit);
   d->control->paste();
}
#endif

/*!
    Deletes all the text in the text edit.

    Note that the undo/redo history is cleared by this function.

    \sa cut() setPlainText() setHtml()
*/
void QTextEdit::clear()
{
   Q_D(QTextEdit);
   // clears and sets empty content
   d->control->clear();
}


/*!
    Selects all text.

    \sa copy() cut() textCursor()
 */
void QTextEdit::selectAll()
{
   Q_D(QTextEdit);
   d->control->selectAll();
}

/*! \internal
*/
bool QTextEdit::event(QEvent *e)
{
   Q_D(QTextEdit);
#ifndef QT_NO_CONTEXTMENU
   if (e->type() == QEvent::ContextMenu
         && static_cast<QContextMenuEvent *>(e)->reason() == QContextMenuEvent::Keyboard) {
      Q_D(QTextEdit);
      ensureCursorVisible();
      const QPoint cursorPos = cursorRect().center();
      QContextMenuEvent ce(QContextMenuEvent::Keyboard, cursorPos, d->viewport->mapToGlobal(cursorPos));
      ce.setAccepted(e->isAccepted());
      const bool result = QAbstractScrollArea::event(&ce);
      e->setAccepted(ce.isAccepted());
      return result;
   } else if (e->type() == QEvent::ShortcutOverride
              || e->type() == QEvent::ToolTip) {
      d->sendControlEvent(e);
   }
#endif // QT_NO_CONTEXTMENU
#ifdef QT_KEYPAD_NAVIGATION
   if (e->type() == QEvent::EnterEditFocus || e->type() == QEvent::LeaveEditFocus) {
      if (QApplication::keypadNavigationEnabled()) {
         d->sendControlEvent(e);
      }
   }
#endif
   return QAbstractScrollArea::event(e);
}

/*! \internal
*/

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
         QMouseEvent ev(QEvent::MouseMove, pos, globalPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
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

/*!
    Changes the text of the text edit to the string \a text.
    Any previous text is removed.

    \a text is interpreted as plain text.

    Note that the undo/redo history is cleared by this function.

    \sa toPlainText()
*/

void QTextEdit::setPlainText(const QString &text)
{
   Q_D(QTextEdit);
   d->control->setPlainText(text);
   d->preferRichText = false;
}

/*!
    \fn QString QTextEdit::toPlainText() const

    Returns the text of the text edit as plain text.

    \sa QTextEdit::setPlainText()
 */


/*!
    \property QTextEdit::html

    This property provides an HTML interface to the text of the text edit.

    toHtml() returns the text of the text edit as html.

    setHtml() changes the text of the text edit.  Any previous text is
    removed and the undo/redo history is cleared. The input text is
    interpreted as rich text in html format.

    \note It is the responsibility of the caller to make sure that the
    text is correctly decoded when a QString containing HTML is created
    and passed to setHtml().

    By default, for a newly-created, empty document, this property contains
    text to describe an HTML 4.0 document with no body text.

    \sa {Supported HTML Subset}, plainText
*/

#ifndef QT_NO_TEXTHTMLPARSER
void QTextEdit::setHtml(const QString &text)
{
   Q_D(QTextEdit);
   d->control->setHtml(text);
   d->preferRichText = true;
}
#endif

/*! \reimp
*/
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
#endif // QT_NO_SHORTCUT

   {
      QTextCursor cursor = d->control->textCursor();
      const QString text = e->text();
      if (cursor.atBlockStart()
            && (d->autoFormatting & AutoBulletList)
            && (text.length() == 1)
            && (text.at(0) == QLatin1Char('-') || text.at(0) == QLatin1Char('*'))
            && (!cursor.currentList())) {

         d->createAutoBulletList();
         e->accept();
         return;
      }
   }

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

/*! \reimp
*/
void QTextEdit::keyReleaseEvent(QKeyEvent *e)
{
#ifdef QT_KEYPAD_NAVIGATION
   Q_D(QTextEdit);
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
         e->accept();
         return;
      }
   }
#endif
   e->ignore();
}

/*!
    Loads the resource specified by the given \a type and \a name.

    This function is an extension of QTextDocument::loadResource().

    \sa QTextDocument::loadResource()
*/
QVariant QTextEdit::loadResource(int type, const QUrl &name)
{
   Q_UNUSED(type);
   Q_UNUSED(name);
   return QVariant();
}

/*! \reimp
*/
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
}

/*! \fn void QTextEdit::paintEvent(QPaintEvent *event)

This event handler can be reimplemented in a subclass to receive paint events passed in \a event.
It is usually unnecessary to reimplement this function in a subclass of QTextEdit.

\warning The underlying text document must not be modified from within a reimplementation
of this function.
*/
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

/*! \reimp
*/
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

/*! \reimp
*/
void QTextEdit::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QTextEdit);
   d->inDrag = false; // paranoia
   const QPoint pos = e->pos();
   d->sendControlEvent(e);
   if (!(e->buttons() & Qt::LeftButton)) {
      return;
   }
   QRect visible = d->viewport->rect();
   if (visible.contains(pos)) {
      d->autoScrollTimer.stop();
   } else if (!d->autoScrollTimer.isActive()) {
      d->autoScrollTimer.start(100, this);
   }
}

/*! \reimp
*/
void QTextEdit::mouseReleaseEvent(QMouseEvent *e)
{
   Q_D(QTextEdit);
   d->sendControlEvent(e);
   if (d->autoScrollTimer.isActive()) {
      d->autoScrollTimer.stop();
      ensureCursorVisible();
   }
   if (!isReadOnly() && rect().contains(e->pos())) {
      d->handleSoftwareInputPanel(e->button(), d->clickCausedFocus);
   }
   d->clickCausedFocus = 0;
}

/*! \reimp
*/
void QTextEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
   Q_D(QTextEdit);
   d->sendControlEvent(e);
}

/*! \reimp
*/
bool QTextEdit::focusNextPrevChild(bool next)
{
   Q_D(const QTextEdit);
   if (!d->tabChangesFocus && d->control->textInteractionFlags() & Qt::TextEditable) {
      return false;
   }
   return QAbstractScrollArea::focusNextPrevChild(next);
}

#ifndef QT_NO_CONTEXTMENU
/*!
  \fn void QTextEdit::contextMenuEvent(QContextMenuEvent *event)

  Shows the standard context menu created with createStandardContextMenu().

  If you do not want the text edit to have a context menu, you can set
  its \l contextMenuPolicy to Qt::NoContextMenu. If you want to
  customize the context menu, reimplement this function. If you want
  to extend the standard context menu, reimplement this function, call
  createStandardContextMenu() and extend the menu returned.

  Information about the event is passed in the \a event object.

  \snippet doc/src/snippets/code/src_gui_widgets_qtextedit.cpp 0
*/
void QTextEdit::contextMenuEvent(QContextMenuEvent *e)
{
   Q_D(QTextEdit);
   d->sendControlEvent(e);
}
#endif // QT_NO_CONTEXTMENU

#ifndef QT_NO_DRAGANDDROP
/*! \reimp
*/
void QTextEdit::dragEnterEvent(QDragEnterEvent *e)
{
   Q_D(QTextEdit);
   d->inDrag = true;
   d->sendControlEvent(e);
}

/*! \reimp
*/
void QTextEdit::dragLeaveEvent(QDragLeaveEvent *e)
{
   Q_D(QTextEdit);
   d->inDrag = false;
   d->autoScrollTimer.stop();
   d->sendControlEvent(e);
}

/*! \reimp
*/
void QTextEdit::dragMoveEvent(QDragMoveEvent *e)
{
   Q_D(QTextEdit);
   d->autoScrollDragPos = e->pos();
   if (!d->autoScrollTimer.isActive()) {
      d->autoScrollTimer.start(100, this);
   }
   d->sendControlEvent(e);
}

/*! \reimp
*/
void QTextEdit::dropEvent(QDropEvent *e)
{
   Q_D(QTextEdit);
   d->inDrag = false;
   d->autoScrollTimer.stop();
   d->sendControlEvent(e);
}

#endif // QT_NO_DRAGANDDROP

/*! \reimp
 */
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

/*!\reimp
*/
void QTextEdit::scrollContentsBy(int dx, int dy)
{
   Q_D(QTextEdit);
   if (isRightToLeft()) {
      dx = -dx;
   }
   d->viewport->scroll(dx, dy);
}

/*!\reimp
*/
QVariant QTextEdit::inputMethodQuery(Qt::InputMethodQuery property) const
{
   Q_D(const QTextEdit);
   QVariant v = d->control->inputMethodQuery(property);
   const QPoint offset(-d->horizontalOffset(), -d->verticalOffset());
   if (v.type() == QVariant::RectF) {
      v = v.toRectF().toRect().translated(offset);
   } else if (v.type() == QVariant::PointF) {
      v = v.toPointF().toPoint() + offset;
   } else if (v.type() == QVariant::Rect) {
      v = v.toRect().translated(offset);
   } else if (v.type() == QVariant::Point) {
      v = v.toPoint() + offset;
   }
   return v;
}

/*! \reimp
*/
void QTextEdit::focusInEvent(QFocusEvent *e)
{
   Q_D(QTextEdit);
   if (e->reason() == Qt::MouseFocusReason) {
      d->clickCausedFocus = 1;
   }
   QAbstractScrollArea::focusInEvent(e);
   d->sendControlEvent(e);
}

/*! \reimp
*/
void QTextEdit::focusOutEvent(QFocusEvent *e)
{
   Q_D(QTextEdit);
   QAbstractScrollArea::focusOutEvent(e);
   d->sendControlEvent(e);
}

/*! \reimp
*/
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

/*! \reimp
*/
void QTextEdit::changeEvent(QEvent *e)
{
   Q_D(QTextEdit);
   QAbstractScrollArea::changeEvent(e);
   if (e->type() == QEvent::ApplicationFontChange
         || e->type() == QEvent::FontChange) {
      d->control->document()->setDefaultFont(font());
   }  else if (e->type() == QEvent::ActivationChange) {
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

/*! \reimp
*/
#ifndef QT_NO_WHEELEVENT
void QTextEdit::wheelEvent(QWheelEvent *e)
{
   Q_D(QTextEdit);
   if (!(d->control->textInteractionFlags() & Qt::TextEditable)) {
      if (e->modifiers() & Qt::ControlModifier) {
         const int delta = e->delta();
         if (delta < 0) {
            zoomOut();
         } else if (delta > 0) {
            zoomIn();
         }
         return;
      }
   }
   QAbstractScrollArea::wheelEvent(e);
   updateMicroFocus();
}
#endif

#ifndef QT_NO_CONTEXTMENU
/*!  This function creates the standard context menu which is shown
  when the user clicks on the text edit with the right mouse
  button. It is called from the default contextMenuEvent() handler.
  The popup menu's ownership is transferred to the caller.

  We recommend that you use the createStandardContextMenu(QPoint) version instead
  which will enable the actions that are sensitive to where the user clicked.
*/

QMenu *QTextEdit::createStandardContextMenu()
{
   Q_D(QTextEdit);
   return d->control->createStandardContextMenu(QPointF(), this);
}

/*!
  \since 4.4
  This function creates the standard context menu which is shown
  when the user clicks on the text edit with the right mouse
  button. It is called from the default contextMenuEvent() handler
  and it takes the \a position of where the mouse click was.
  This can enable actions that are sensitive to the position where the user clicked.
  The popup menu's ownership is transferred to the caller.
*/

QMenu *QTextEdit::createStandardContextMenu(const QPoint &position)
{
   Q_D(QTextEdit);
   return d->control->createStandardContextMenu(position, this);
}
#endif // QT_NO_CONTEXTMENU

/*!
  returns a QTextCursor at position \a pos (in viewport coordinates).
*/
QTextCursor QTextEdit::cursorForPosition(const QPoint &pos) const
{
   Q_D(const QTextEdit);
   return d->control->cursorForPosition(d->mapToContents(pos));
}

/*!
  returns a rectangle (in viewport coordinates) that includes the
  \a cursor.
 */
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

/*!
  returns a rectangle (in viewport coordinates) that includes the
  cursor of the text edit.
 */
QRect QTextEdit::cursorRect() const
{
   Q_D(const QTextEdit);
   QRect r = d->control->cursorRect().toRect();
   r.translate(-d->horizontalOffset(), -d->verticalOffset());
   return r;
}


/*!
    Returns the reference of the anchor at position \a pos, or an
    empty string if no anchor exists at that point.
*/
QString QTextEdit::anchorAt(const QPoint &pos) const
{
   Q_D(const QTextEdit);
   return d->control->anchorAt(d->mapToContents(pos));
}

/*!
   \property QTextEdit::overwriteMode
   \since 4.1
   \brief whether text entered by the user will overwrite existing text

   As with many text editors, the text editor widget can be configured
   to insert or overwrite existing text with new text entered by the user.

   If this property is true, existing text is overwritten, character-for-character
   by new text; otherwise, text is inserted at the cursor position, displacing
   existing text.

   By default, this property is false (new text does not overwrite existing text).
*/

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

/*!
    \property QTextEdit::tabStopWidth
    \brief the tab stop width in pixels
    \since 4.1

    By default, this property contains a value of 80 pixels.
*/

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

/*!
    \since 4.2
    \property QTextEdit::cursorWidth

    This property specifies the width of the cursor in pixels. The default value is 1.
*/
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

/*!
    \property QTextEdit::acceptRichText
    \brief whether the text edit accepts rich text insertions by the user
    \since 4.1

    When this propery is set to false text edit will accept only
    plain text input from the user. For example through clipboard or drag and drop.

    This property's default is true.
*/

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

/*!
    \class QTextEdit::ExtraSelection
    \since 4.2
    \brief The QTextEdit::ExtraSelection structure provides a way of specifying a
           character format for a given selection in a document
*/

/*!
    \variable QTextEdit::ExtraSelection::cursor
    A cursor that contains a selection in a QTextDocument
*/

/*!
    \variable QTextEdit::ExtraSelection::format
    A format that is used to specify a foreground or background brush/color
    for the selection.
*/

/*!
    \since 4.2
    This function allows temporarily marking certain regions in the document
    with a given color, specified as \a selections. This can be useful for
    example in a programming editor to mark a whole line of text with a given
    background color to indicate the existence of a breakpoint.

    \sa QTextEdit::ExtraSelection, extraSelections()
*/
void QTextEdit::setExtraSelections(const QList<ExtraSelection> &selections)
{
   Q_D(QTextEdit);
   d->control->setExtraSelections(selections);
}

/*!
    \since 4.2
    Returns previously set extra selections.

    \sa setExtraSelections()
*/
QList<QTextEdit::ExtraSelection> QTextEdit::extraSelections() const
{
   Q_D(const QTextEdit);
   return d->control->extraSelections();
}

/*!
    This function returns a new MIME data object to represent the contents
    of the text edit's current selection. It is called when the selection needs
    to be encapsulated into a new QMimeData object; for example, when a drag
    and drop operation is started, or when data is copyied to the clipboard.

    If you reimplement this function, note that the ownership of the returned
    QMimeData object is passed to the caller. The selection can be retrieved
    by using the textCursor() function.
*/
QMimeData *QTextEdit::createMimeDataFromSelection() const
{
   Q_D(const QTextEdit);
   return d->control->QTextControl::createMimeDataFromSelection();
}

/*!
    This function returns true if the contents of the MIME data object, specified
    by \a source, can be decoded and inserted into the document. It is called
    for example when during a drag operation the mouse enters this widget and it
    is necessary to determine whether it is possible to accept the drag and drop
    operation.

    Reimplement this function to enable drag and drop support for additional MIME types.
 */
bool QTextEdit::canInsertFromMimeData(const QMimeData *source) const
{
   Q_D(const QTextEdit);
   return d->control->QTextControl::canInsertFromMimeData(source);
}

/*!
    This function inserts the contents of the MIME data object, specified
    by \a source, into the text edit at the current cursor position. It is
    called whenever text is inserted as the result of a clipboard paste
    operation, or when the text edit accepts data from a drag and drop
    operation.

    Reimplement this function to enable drag and drop support for additional MIME types.
 */
void QTextEdit::insertFromMimeData(const QMimeData *source)
{
   Q_D(QTextEdit);
   d->control->QTextControl::insertFromMimeData(source);
}

/*!
    \property QTextEdit::readOnly
    \brief whether the text edit is read-only

    In a read-only text edit the user can only navigate through the
    text and select text; modifying the text is not possible.

    This property's default is false.
*/

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
}

/*!
    \property QTextEdit::textInteractionFlags
    \since 4.2

    Specifies how the widget should interact with user input.

    The default value depends on whether the QTextEdit is read-only
    or editable, and whether it is a QTextBrowser or not.
*/

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

/*!
    Merges the properties specified in \a modifier into the current character
    format by calling QTextCursor::mergeCharFormat on the editor's cursor.
    If the editor has a selection then the properties of \a modifier are
    directly applied to the selection.

    \sa QTextCursor::mergeCharFormat()
 */
void QTextEdit::mergeCurrentCharFormat(const QTextCharFormat &modifier)
{
   Q_D(QTextEdit);
   d->control->mergeCurrentCharFormat(modifier);
}

/*!
    Sets the char format that is be used when inserting new text to \a
    format by calling QTextCursor::setCharFormat() on the editor's
    cursor.  If the editor has a selection then the char format is
    directly applied to the selection.
 */
void QTextEdit::setCurrentCharFormat(const QTextCharFormat &format)
{
   Q_D(QTextEdit);
   d->control->setCurrentCharFormat(format);
}

/*!
    Returns the char format that is used when inserting new text.
 */
QTextCharFormat QTextEdit::currentCharFormat() const
{
   Q_D(const QTextEdit);
   return d->control->currentCharFormat();
}

/*!
    \property QTextEdit::autoFormatting
    \brief the enabled set of auto formatting features

    The value can be any combination of the values in the
    AutoFormattingFlag enum.  The default is AutoNone. Choose
    AutoAll to enable all automatic formatting.

    Currently, the only automatic formatting feature provided is
    AutoBulletList; future versions of Qt may offer more.
*/

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

/*!
    Convenience slot that inserts \a text at the current
    cursor position.

    It is equivalent to

    \snippet doc/src/snippets/code/src_gui_widgets_qtextedit.cpp 1
 */
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
#endif // QT_NO_TEXTHTMLPARSER

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
   QFont f = font();
   const int newSize = f.pointSize() + range;
   if (newSize <= 0) {
      return;
   }
   f.setPointSize(newSize);
   setFont(f);
}

void QTextEdit::zoomOut(int range)
{
   zoomIn(-range);
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

void QTextEdit::print(QPrinter *printer) const
{
   Q_D(const QTextEdit);
   d->control->print(printer);
}
#endif // QT _NO_PRINTER

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

void QTextEdit::setText(const QString &text)
{
   Qt::TextFormat format = Qt::mightBeRichText(text) ? Qt::RichText : Qt::PlainText;

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
   const bool atBottom = isReadOnly() ?  d->verticalOffset() >= d->vbar->maximum() :
                         d->control->textCursor().atEnd();
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

QT_END_NAMESPACE

