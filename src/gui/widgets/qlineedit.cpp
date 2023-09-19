/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qlineedit.h>
#include <qlineedit_p.h>

#ifndef QT_NO_LINEEDIT

#include <qaction.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qdrag.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qfontmetrics.h>
#include <qstylehints.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpointer.h>
#include <qstringlist.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtimer.h>
#include <qvalidator.h>
#include <qvariant.h>
#include <qvector.h>
#include <qwhatsthis.h>
#include <qdebug.h>
#include <qtextedit.h>
#include <qtextedit_p.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

#include <qabstractitemview.h>
#include <qstylesheetstyle_p.h>

#ifndef QT_NO_SHORTCUT
#include <qapplication_p.h>
#include <qshortcutmap_p.h>
#include <qkeysequence.h>

#define ACCEL_KEY(k)   (! qApp->d_func()->shortcutMap.hasShortcutForKeySequence(k) ?  \
                        QLatin1Char('\t') + QKeySequence(k).toString(QKeySequence::NativeText) : QString())
#else
#define ACCEL_KEY(k)   QString()
#endif

#include <limits.h>

#ifdef DrawText
#undef DrawText
#endif



void QLineEdit::initStyleOption(QStyleOptionFrame *option) const
{
   if (!option) {
      return;
   }

   Q_D(const QLineEdit);
   option->initFrom(this);
   option->rect = contentsRect();
   option->lineWidth = d->frame ? style()->pixelMetric(QStyle::PM_DefaultFrameWidth, option, this)
      : 0;
   option->midLineWidth = 0;
   option->state |= QStyle::State_Sunken;
   if (d->control->isReadOnly()) {
      option->state |= QStyle::State_ReadOnly;
   }

#ifdef QT_KEYPAD_NAVIGATION
   if (hasEditFocus()) {
      option->state |= QStyle::State_HasEditFocus;
   }
#endif

   option->features = QStyleOptionFrame::None;
}


QLineEdit::QLineEdit(QWidget *parent)
   : QWidget(*new QLineEditPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QLineEdit);
   d->init(QString());
}


QLineEdit::QLineEdit(const QString &contents, QWidget *parent)
   : QWidget(*new QLineEditPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QLineEdit);
   d->init(contents);
}

QLineEdit::~QLineEdit()
{
}

QString QLineEdit::text() const
{
   Q_D(const QLineEdit);
   return d->control->text();
}

void QLineEdit::setText(const QString &text)
{
   Q_D(QLineEdit);
   d->control->setText(text);
}

QString QLineEdit::placeholderText() const
{
   Q_D(const QLineEdit);
   return d->placeholderText;
}

void QLineEdit::setPlaceholderText(const QString &placeholderText)
{
   Q_D(QLineEdit);

   if (d->placeholderText != placeholderText) {
      d->placeholderText = placeholderText;

      if (d->shouldShowPlaceholderText()) {
         update();
      }
   }
}

QString QLineEdit::displayText() const
{
   Q_D(const QLineEdit);
   return d->control->displayText();
}

int QLineEdit::maxLength() const
{
   Q_D(const QLineEdit);
   return d->control->maxLength();
}

void QLineEdit::setMaxLength(int maxLength)
{
   Q_D(QLineEdit);
   d->control->setMaxLength(maxLength);
}

bool QLineEdit::hasFrame() const
{
   Q_D(const QLineEdit);
   return d->frame;
}

void QLineEdit::addAction(QAction *action, ActionPosition position)
{
   Q_D(QLineEdit);
   QWidget::addAction(action);
   d->addAction(action, nullptr, position);
}

QAction *QLineEdit::addAction(const QIcon &icon, ActionPosition position)
{
   QAction *result = new QAction(icon, QString(), this);
   addAction(result, position);

   return result;
}

static const QString clearButtonAction = QString("_cs_clearActionEnabled");

void QLineEdit::setClearButtonEnabled(bool enable)
{
   Q_D(QLineEdit);

   if (enable == isClearButtonEnabled()) {
      return;
   }

   if (enable) {
      QAction *clearAction = new QAction(d->clearButtonIcon(), QString(), this);
      clearAction->setEnabled(! isReadOnly());
      clearAction->setObjectName(clearButtonAction);

      d->addAction(clearAction, nullptr, QLineEdit::TrailingPosition,
                  QLineEditPrivate::SideWidgetClearButton | QLineEditPrivate::SideWidgetFadeInWithText);

   } else {
      QAction *clearAction = findChild<QAction *>(clearButtonAction);

      Q_ASSERT(clearAction);
      d->removeAction(clearAction);

      delete clearAction;
   }
}

bool QLineEdit::isClearButtonEnabled() const
{
   return findChild<QAction *>(clearButtonAction);
}

void QLineEdit::setFrame(bool enable)
{
   Q_D(QLineEdit);

   d->frame = enable;
   update();
   updateGeometry();
}

QLineEdit::EchoMode QLineEdit::echoMode() const
{
   Q_D(const QLineEdit);
   return (EchoMode) d->control->echoMode();
}

void QLineEdit::setEchoMode(EchoMode mode)
{
   Q_D(QLineEdit);

   if (mode == (EchoMode)d->control->echoMode()) {
      return;
   }

   Qt::InputMethodHints imHints = inputMethodHints();
   if (mode == Password || mode == NoEcho) {
      imHints |= Qt::ImhHiddenText;
   } else {
      imHints &= ~Qt::ImhHiddenText;
   }
   if (mode != Normal) {
      imHints |= (Qt::ImhNoAutoUppercase | Qt::ImhNoPredictiveText | Qt::ImhSensitiveData);
   } else {
      imHints &= ~(Qt::ImhNoAutoUppercase | Qt::ImhNoPredictiveText | Qt::ImhSensitiveData);
   }
   setInputMethodHints(imHints);
   d->control->setEchoMode(mode);
   update();

}

#ifndef QT_NO_VALIDATOR

const QValidator *QLineEdit::validator() const
{
   Q_D(const QLineEdit);
   return d->control->validator();
}

void QLineEdit::setValidator(const QValidator *v)
{
   Q_D(QLineEdit);
   d->control->setValidator(v);
}
#endif

#ifndef QT_NO_COMPLETER

void QLineEdit::setCompleter(QCompleter *c)
{
   Q_D(QLineEdit);

   if (c == d->control->completer()) {
      return;
   }

   if (d->control->completer()) {
      disconnect(d->control->completer(), QString(), this, QString());
      d->control->completer()->setWidget(nullptr);

      if (d->control->completer()->parent() == this) {
         delete d->control->completer();
      }
   }

   d->control->setCompleter(c);

   if (! c) {
      return;
   }

   if (c->widget() == nullptr) {
      c->setWidget(this);
   }

   if (hasFocus()) {
      QObject::connect(d->control->completer(), cs_mp_cast<const QString &>(&QCompleter::activated),
            this, &QLineEdit::setText);

      QObject::connect(d->control->completer(), cs_mp_cast<const QString &>(&QCompleter::highlighted),
            this, &QLineEdit::_q_completionHighlighted);
   }
}

QCompleter *QLineEdit::completer() const
{
   Q_D(const QLineEdit);
   return d->control->completer();
}

#endif // QT_NO_COMPLETER


QSize QLineEdit::sizeHint() const
{
   Q_D(const QLineEdit);
   ensurePolished();
   QFontMetrics fm(font());

   int h = qMax(fm.height(), 14) + 2 * d->verticalMargin
      + d->topTextMargin + d->bottomTextMargin
      + d->topmargin + d->bottommargin;

   int w = fm.width(QLatin1Char('x')) * 17 + 2 * d->horizontalMargin
      + d->effectiveLeftTextMargin() + d->effectiveRightTextMargin()
      + d->leftmargin + d->rightmargin; // "some"

   QStyleOptionFrame opt;
   initStyleOption(&opt);

   return (style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(w, h).
            expandedTo(QApplication::globalStrut()), this));
}


/*!
    Returns a minimum size for the line edit.

    The width returned is enough for at least one character.
*/

QSize QLineEdit::minimumSizeHint() const
{
   Q_D(const QLineEdit);
   ensurePolished();
   QFontMetrics fm = fontMetrics();
   int h = fm.height() + qMax(2 * d->verticalMargin, fm.leading())
      + d->topTextMargin + d->bottomTextMargin
      + d->topmargin + d->bottommargin;
   int w = fm.maxWidth()
      + d->effectiveLeftTextMargin() + d->effectiveRightTextMargin()
      + d->leftmargin + d->rightmargin;
   QStyleOptionFrame opt;
   initStyleOption(&opt);
   return (style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(w, h).
            expandedTo(QApplication::globalStrut()), this));
}


/*!
    \property QLineEdit::cursorPosition
    \brief the current cursor position for this line edit

    Setting the cursor position causes a repaint when appropriate.

    By default, this property contains a value of 0.
*/

int QLineEdit::cursorPosition() const
{
   Q_D(const QLineEdit);
   return d->control->cursorPosition();
}

void QLineEdit::setCursorPosition(int pos)
{
   Q_D(QLineEdit);
   d->control->setCursorPosition(pos);
}

/*!
    Returns the cursor position under the point \a pos.
*/
// ### What should this do if the point is outside of contentsRect? Currently returns 0.
int QLineEdit::cursorPositionAt(const QPoint &pos)
{
   Q_D(QLineEdit);
   return d->xToPos(pos.x());
}

/*!
    \property QLineEdit::alignment
    \brief the alignment of the line edit

    Both horizontal and vertical alignment is allowed here, Qt::AlignJustify
    will map to Qt::AlignLeft.

    By default, this property contains a combination of Qt::AlignLeft and Qt::AlignVCenter.

    \sa Qt::Alignment
*/

Qt::Alignment QLineEdit::alignment() const
{
   Q_D(const QLineEdit);
   return QFlag(d->alignment);
}

void QLineEdit::setAlignment(Qt::Alignment alignment)
{
   Q_D(QLineEdit);
   d->alignment = alignment;
   update();
}


/*!
    Moves the cursor forward \a steps characters. If \a mark is true
    each character moved over is added to the selection; if \a mark is
    false the selection is cleared.

    \sa cursorBackward()
*/

void QLineEdit::cursorForward(bool mark, int steps)
{
   Q_D(QLineEdit);
   d->control->cursorForward(mark, steps);
}


/*!
    Moves the cursor back \a steps characters. If \a mark is true each
    character moved over is added to the selection; if \a mark is
    false the selection is cleared.

    \sa cursorForward()
*/
void QLineEdit::cursorBackward(bool mark, int steps)
{
   cursorForward(mark, -steps);
}

/*!
    Moves the cursor one word forward. If \a mark is true, the word is
    also selected.

    \sa cursorWordBackward()
*/
void QLineEdit::cursorWordForward(bool mark)
{
   Q_D(QLineEdit);
   d->control->cursorWordForward(mark);
}

/*!
    Moves the cursor one word backward. If \a mark is true, the word
    is also selected.

    \sa cursorWordForward()
*/

void QLineEdit::cursorWordBackward(bool mark)
{
   Q_D(QLineEdit);
   d->control->cursorWordBackward(mark);
}


/*!
    If no text is selected, deletes the character to the left of the
    text cursor and moves the cursor one position to the left. If any
    text is selected, the cursor is moved to the beginning of the
    selected text and the selected text is deleted.

    \sa del()
*/
void QLineEdit::backspace()
{
   Q_D(QLineEdit);
   d->control->backspace();
}

/*!
    If no text is selected, deletes the character to the right of the
    text cursor. If any text is selected, the cursor is moved to the
    beginning of the selected text and the selected text is deleted.

    \sa backspace()
*/

void QLineEdit::del()
{
   Q_D(QLineEdit);
   d->control->del();
}

/*!
    Moves the text cursor to the beginning of the line unless it is
    already there. If \a mark is true, text is selected towards the
    first position; otherwise, any selected text is unselected if the
    cursor is moved.

    \sa end()
*/

void QLineEdit::home(bool mark)
{
   Q_D(QLineEdit);
   d->control->home(mark);
}

/*!
    Moves the text cursor to the end of the line unless it is already
    there. If \a mark is true, text is selected towards the last
    position; otherwise, any selected text is unselected if the cursor
    is moved.

    \sa home()
*/

void QLineEdit::end(bool mark)
{
   Q_D(QLineEdit);
   d->control->end(mark);
}


/*!
    \property QLineEdit::modified
    \brief whether the line edit's contents has been modified by the user

    The modified flag is never read by QLineEdit; it has a default value
    of false and is changed to true whenever the user changes the line
    edit's contents.

    This is useful for things that need to provide a default value but
    do not start out knowing what the default should be (perhaps it
    depends on other fields on the form). Start the line edit without
    the best default, and when the default is known, if modified()
    returns false (the user hasn't entered any text), insert the
    default value.

    Calling setText() resets the modified flag to false.
*/

bool QLineEdit::isModified() const
{
   Q_D(const QLineEdit);
   return d->control->isModified();
}

void QLineEdit::setModified(bool modified)
{
   Q_D(QLineEdit);
   d->control->setModified(modified);
}


/*!\fn QLineEdit::clearModified()

Use setModified(false) instead.

    \sa isModified()
*/


/*!
    \property QLineEdit::hasSelectedText
    \brief whether there is any text selected

    hasSelectedText() returns true if some or all of the text has been
    selected by the user; otherwise returns false.

    By default, this property is false.

    \sa selectedText()
*/


bool QLineEdit::hasSelectedText() const
{
   Q_D(const QLineEdit);
   return d->control->hasSelectedText();
}

/*!
    \property QLineEdit::selectedText
    \brief the selected text

    If there is no selected text this property's value is
    an empty string.

    By default, this property contains an empty string.

    \sa hasSelectedText()
*/

QString QLineEdit::selectedText() const
{
   Q_D(const QLineEdit);
   return d->control->selectedText();
}

/*!
    selectionStart() returns the index of the first selected character in the
    line edit or -1 if no text is selected.

    \sa selectedText()
*/

int QLineEdit::selectionStart() const
{
   Q_D(const QLineEdit);
   return d->control->selectionStart();
}


/*!
    Selects text from position \a start and for \a length characters.
    Negative lengths are allowed.

    \sa deselect() selectAll() selectedText()
*/

void QLineEdit::setSelection(int start, int length)
{
   Q_D(QLineEdit);
   if (start < 0 || start > (int)d->control->end()) {
      qWarning("QLineEdit::setSelection: Invalid start position (%d)", start);
      return;
   }

   d->control->setSelection(start, length);

   if (d->control->hasSelectedText()) {
      QStyleOptionFrame opt;
      initStyleOption(&opt);
      if (!style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected, &opt, this)) {
         d->setCursorVisible(false);
      }
   }
}


/*!
    \property QLineEdit::undoAvailable
    \brief whether undo is available

    Undo becomes available once the user has modified the text in the line edit.

    By default, this property is false.
*/

bool QLineEdit::isUndoAvailable() const
{
   Q_D(const QLineEdit);
   return d->control->isUndoAvailable();
}

/*!
    \property QLineEdit::redoAvailable
    \brief whether redo is available

    Redo becomes available once the user has performed one or more undo operations
    on text in the line edit.

    By default, this property is false.
*/

bool QLineEdit::isRedoAvailable() const
{
   Q_D(const QLineEdit);
   return d->control->isRedoAvailable();
}

/*!
    \property QLineEdit::dragEnabled
    \brief whether the lineedit starts a drag if the user presses and
    moves the mouse on some selected text

    Dragging is disabled by default.
*/

bool QLineEdit::dragEnabled() const
{
   Q_D(const QLineEdit);
   return d->dragEnabled;
}

void QLineEdit::setDragEnabled(bool b)
{
   Q_D(QLineEdit);
   d->dragEnabled = b;
}


/*!
  \property QLineEdit::cursorMoveStyle
  \brief the movement style of cursor in this line edit
  \since 4.8

  When this property is set to Qt::VisualMoveStyle, the line edit will use visual
  movement style. Pressing the left arrow key will always cause the cursor to move
  left, regardless of the text's writing direction. The same behavior applies to
  right arrow key.

  When the property is Qt::LogicalMoveStyle (the default), within a LTR text block,
  increase cursor position when pressing left arrow key, decrease cursor position
  when pressing the right arrow key. If the text block is right to left, the opposite
  behavior applies.
*/

/*!
    \since 4.8

    Returns the movement style for the cursor in the line edit.
*/
Qt::CursorMoveStyle QLineEdit::cursorMoveStyle() const
{
   Q_D(const QLineEdit);
   return d->control->cursorMoveStyle();
}

/*!
    \since 4.8

    Sets the movement style for the cursor in the line edit to the given
    \a style.
*/
void QLineEdit::setCursorMoveStyle(Qt::CursorMoveStyle style)
{
   Q_D(QLineEdit);
   d->control->setCursorMoveStyle(style);
}

/*!
    \property QLineEdit::acceptableInput
    \brief whether the input satisfies the inputMask and the
    validator.

    By default, this property is true.

    \sa setInputMask(), setValidator()
*/
bool QLineEdit::hasAcceptableInput() const
{
   Q_D(const QLineEdit);
   return d->control->hasAcceptableInput();
}

/*!
    Sets the margins around the text inside the frame to have the
    sizes \a left, \a top, \a right, and \a bottom.
    \since 4.5

    See also getTextMargins().
*/
void QLineEdit::setTextMargins(int left, int top, int right, int bottom)
{
   Q_D(QLineEdit);
   d->leftTextMargin = left;
   d->topTextMargin = top;
   d->rightTextMargin = right;
   d->bottomTextMargin = bottom;
   updateGeometry();
   update();
}

/*!
    \since 4.6
    Sets the \a margins around the text inside the frame.

    See also textMargins().
*/
void QLineEdit::setTextMargins(const QMargins &margins)
{
   setTextMargins(margins.left(), margins.top(), margins.right(), margins.bottom());
}

/*!
    Returns the widget's text margins for \a left, \a top, \a right, and \a bottom.
    \since 4.5

    \sa setTextMargins()
*/
void QLineEdit::getTextMargins(int *left, int *top, int *right, int *bottom) const
{
   Q_D(const QLineEdit);
   if (left) {
      *left = d->leftTextMargin;
   }
   if (top) {
      *top = d->topTextMargin;
   }
   if (right) {
      *right = d->rightTextMargin;
   }
   if (bottom) {
      *bottom = d->bottomTextMargin;
   }
}

/*!
    \since 4.6
    Returns the widget's text margins.

    \sa setTextMargins()
*/
QMargins QLineEdit::textMargins() const
{
   Q_D(const QLineEdit);
   return QMargins(d->leftTextMargin, d->topTextMargin, d->rightTextMargin, d->bottomTextMargin);
}

QString QLineEdit::inputMask() const
{
   Q_D(const QLineEdit);
   return d->control->inputMask();
}

void QLineEdit::setInputMask(const QString &inputMask)
{
   Q_D(QLineEdit);
   d->control->setInputMask(inputMask);
}

/*!
    Selects all the text (i.e. highlights it) and moves the cursor to
    the end. This is useful when a default value has been inserted
    because if the user types before clicking on the widget, the
    selected text will be deleted.

    \sa setSelection() deselect()
*/

void QLineEdit::selectAll()
{
   Q_D(QLineEdit);
   d->control->selectAll();
}

/*!
    Deselects any selected text.

    \sa setSelection() selectAll()
*/

void QLineEdit::deselect()
{
   Q_D(QLineEdit);
   d->control->deselect();
}

void QLineEdit::insert(const QString &newText)
{
   //     q->resetInputContext(); //#### FIX ME IN QT
   Q_D(QLineEdit);
   d->control->insert(newText);
}

/*!
    Clears the contents of the line edit.

    \sa setText(), insert()
*/
void QLineEdit::clear()
{
   Q_D(QLineEdit);

   d->resetInputMethod();
   d->control->clear();
}


void QLineEdit::undo()
{
   Q_D(QLineEdit);
   d->resetInputMethod();
   d->control->undo();
}


void QLineEdit::redo()
{
   Q_D(QLineEdit);
   d->resetInputMethod();
   d->control->redo();
}



bool QLineEdit::isReadOnly() const
{
   Q_D(const QLineEdit);
   return d->control->isReadOnly();
}

void QLineEdit::setReadOnly(bool enable)
{
   Q_D(QLineEdit);
   if (d->control->isReadOnly() != enable) {
      d->control->setReadOnly(enable);
      d->setClearButtonEnabled(!enable);
      setAttribute(Qt::WA_MacShowFocusRect, !enable);
      setAttribute(Qt::WA_InputMethodEnabled, d->shouldEnableInputMethod());
#ifndef QT_NO_CURSOR
      setCursor(enable ? Qt::ArrowCursor : Qt::IBeamCursor);
#endif
      QEvent event(QEvent::ReadOnlyChange);
      QCoreApplication::sendEvent(this, &event);
      update();
   }
}


#ifndef QT_NO_CLIPBOARD


void QLineEdit::cut()
{
   if (hasSelectedText()) {
      copy();
      del();
   }
}
void QLineEdit::copy() const
{
   Q_D(const QLineEdit);
   d->control->copy();
}

void QLineEdit::paste()
{
   Q_D(QLineEdit);
   d->control->paste();
}

#endif // ! QT_NO_CLIPBOARD

bool QLineEdit::event(QEvent *e)
{
   Q_D(QLineEdit);

   if (e->type() == QEvent::Timer) {
      // should be timerEvent, is here for binary compatibility
      int timerId = ((QTimerEvent *)e)->timerId();

      if (false) {

#ifndef QT_NO_DRAGANDDROP
      } else if (timerId == d->dndTimer.timerId()) {
         d->drag();
#endif

      } else if (timerId == d->tripleClickTimer.timerId()) {
         d->tripleClickTimer.stop();

      }

   } else if (e->type() == QEvent::ContextMenu) {

#ifndef QT_NO_IM
      if (d->control->composeMode()) {
         return true;
      }
#endif

   } else if (e->type() == QEvent::WindowActivate) {
      QTimer::singleShot(0, this, SLOT(_q_handleWindowActivate()));

#ifndef QT_NO_SHORTCUT
   } else if (e->type() == QEvent::ShortcutOverride) {
      QKeyEvent *ke = static_cast<QKeyEvent *>(e);
      d->control->processShortcutOverrideEvent(ke);
#endif

   } else if (e->type() == QEvent::KeyRelease) {
      d->control->setCursorBlinkPeriod(QApplication::cursorFlashTime());

   } else if (e->type() == QEvent::Show) {
      //In order to get the cursor blinking if QComboBox::setEditable is called when the combobox has focus
      if (hasFocus()) {
         d->control->setCursorBlinkPeriod(QApplication::cursorFlashTime());
         QStyleOptionFrame opt;
         initStyleOption(&opt);

         if ((! hasSelectedText() && d->control->preeditAreaText().isEmpty())
            || style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected, &opt, this)) {
            d->setCursorVisible(true);
         }
      }

   } else if (e->type() == QEvent::ActionRemoved) {
      d->removeAction(static_cast<QActionEvent *>(e)->action());

   } else if (e->type() == QEvent::Resize) {
      d->positionSideWidgets();
   }

#ifdef QT_KEYPAD_NAVIGATION
   if (QApplication::keypadNavigationEnabled()) {
      if (e->type() == QEvent::EnterEditFocus) {
         end(false);
         d->setCursorVisible(true);
         d->control->setCursorBlinkPeriod(QApplication::cursorFlashTime());

      } else if (e->type() == QEvent::LeaveEditFocus) {
         d->setCursorVisible(false);
         d->control->setCursorBlinkPeriod(0);

         if (d->control->hasAcceptableInput() || d->control->fixup()) {
            emit editingFinished();
         }
      }
   }
#endif

   return QWidget::event(e);
}

/*! \reimp
*/
void QLineEdit::mousePressEvent(QMouseEvent *e)
{
   Q_D(QLineEdit);
   d->mousePressPos = e->pos();
   if (d->sendMouseEventToInputContext(e)) {
      return;
   }
   if (e->button() == Qt::RightButton) {
      return;
   }
#ifdef QT_KEYPAD_NAVIGATION
   if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
      setEditFocus(true);
      // Get the completion list to pop up.
      if (d->control->completer()) {
         d->control->completer()->complete();
      }
   }
#endif
   if (d->tripleClickTimer.isActive() && (e->pos() - d->tripleClick).manhattanLength() <
      QApplication::startDragDistance()) {
      selectAll();
      return;
   }
   bool mark = e->modifiers() & Qt::ShiftModifier;

#ifdef Q_OS_ANDROID
   mark = mark && (d->imHints & Qt::ImhNoPredictiveText);
#endif

   int cursor = d->xToPos(e->pos().x());

#ifndef QT_NO_DRAGANDDROP

   if (!mark && d->dragEnabled && d->control->echoMode() == Normal &&
      e->button() == Qt::LeftButton && d->inSelection(e->pos().x())) {
      if (!d->dndTimer.isActive()) {
         d->dndTimer.start(QApplication::startDragTime(), this);
      }

   } else
#endif
   {
      d->control->moveCursor(cursor, mark);
   }
}

/*! \reimp
*/
void QLineEdit::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QLineEdit);


   if (e->buttons() & Qt::LeftButton) {
#ifndef QT_NO_DRAGANDDROP
      if (d->dndTimer.isActive()) {
         if ((d->mousePressPos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
            d->drag();
         }
      } else
#endif
      {
#ifndef Q_OS_ANDROID
         const bool select = true;
#else
         const bool select = (d->imHints & Qt::ImhNoPredictiveText);
#endif
#ifndef QT_NO_IM
         if (d->control->composeMode() && select) {
            int startPos = d->xToPos(d->mousePressPos.x());
            int currentPos = d->xToPos(e->pos().x());
            if (startPos != currentPos) {
               d->control->setSelection(startPos, currentPos - startPos);
            }

         } else
#endif
         {
            d->control->moveCursor(d->xToPos(e->pos().x()), select);
         }
      }
   }

   d->sendMouseEventToInputContext(e);
}

/*! \reimp
*/
void QLineEdit::mouseReleaseEvent(QMouseEvent *e)
{
   Q_D(QLineEdit);
   if (d->sendMouseEventToInputContext(e)) {
      return;
   }
#ifndef QT_NO_DRAGANDDROP
   if (e->button() == Qt::LeftButton) {
      if (d->dndTimer.isActive()) {
         d->dndTimer.stop();
         deselect();
         return;
      }
   }
#endif
#ifndef QT_NO_CLIPBOARD
   if (QApplication::clipboard()->supportsSelection()) {
      if (e->button() == Qt::LeftButton) {
         d->control->copy(QClipboard::Selection);
      } else if (!d->control->isReadOnly() && e->button() == Qt::MiddleButton) {
         deselect();
         insert(QApplication::clipboard()->text(QClipboard::Selection));
      }
   }
#endif

   if (!isReadOnly() && rect().contains(e->pos())) {
      d->handleSoftwareInputPanel(e->button(), d->clickCausedFocus);
   }
   d->clickCausedFocus = 0;
}

/*! \reimp
*/
void QLineEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
   Q_D(QLineEdit);

   if (e->button() == Qt::LeftButton) {
      int position = d->xToPos(e->pos().x());

      // exit composition mode
#ifndef QT_NO_IM
      if (d->control->composeMode()) {
         int preeditPos = d->control->cursor();
         int posInPreedit = position - d->control->cursor();
         int preeditLength = d->control->preeditAreaText().length();
         bool positionOnPreedit = false;

         if (posInPreedit >= 0 && posInPreedit <= preeditLength) {
            positionOnPreedit = true;
         }

         int textLength = d->control->end();
         d->control->commitPreedit();
         int sizeChange = d->control->end() - textLength;

         if (positionOnPreedit) {
            if (sizeChange == 0) {
               position = -1;   // cancel selection, word disappeared
            } else
               // ensure not selecting after preedit if event happened there
            {
               position = qBound(preeditPos, position, preeditPos + sizeChange);
            }
         } else if (position > preeditPos) {
            // adjust positions after former preedit by how much text changed
            position += (sizeChange - preeditLength);
         }
      }
#endif

      if (position >= 0) {
         d->control->selectWordAtPos(position);
      }

      d->tripleClickTimer.start(QApplication::doubleClickInterval(), this);
      d->tripleClick = e->pos();
   } else {
      d->sendMouseEventToInputContext(e);
   }
}



void QLineEdit::keyPressEvent(QKeyEvent *event)
{
   Q_D(QLineEdit);
#ifdef QT_KEYPAD_NAVIGATION
   bool select = false;
   switch (event->key()) {
      case Qt::Key_Select:
         if (QApplication::keypadNavigationEnabled()) {
            if (hasEditFocus()) {
               setEditFocus(false);
               if (d->control->completer() && d->control->completer()->popup()->isVisible()) {
                  d->control->completer()->popup()->hide();
               }
               select = true;
            }
         }
         break;
      case Qt::Key_Back:
      case Qt::Key_No:
         if (!QApplication::keypadNavigationEnabled() || !hasEditFocus()) {
            event->ignore();
            return;
         }
         break;
      default:
         if (QApplication::keypadNavigationEnabled()) {
            if (!hasEditFocus() && !(event->modifiers() & Qt::ControlModifier)) {
               if (!event->text().isEmpty() && event->text().at(0).isPrint()
                  && !isReadOnly()) {
                  setEditFocus(true);
               } else {
                  event->ignore();
                  return;
               }
            }
         }
   }



   if (QApplication::keypadNavigationEnabled() && !select && !hasEditFocus()) {
      setEditFocus(true);
      if (event->key() == Qt::Key_Select) {
         return;   // Just start. No action.
      }
   }
#endif
   d->control->processKeyEvent(event);
   if (event->isAccepted()) {
      if (layoutDirection() != d->control->layoutDirection()) {
         setLayoutDirection(d->control->layoutDirection());
      }
      d->control->setCursorBlinkPeriod(0);
   }
}


QRect QLineEdit::cursorRect() const
{
   Q_D(const QLineEdit);
   return d->cursorRect();
}

/*! \reimp
 */
void QLineEdit::inputMethodEvent(QInputMethodEvent *e)
{
   Q_D(QLineEdit);
   if (d->control->isReadOnly()) {
      e->ignore();
      return;
   }

   if (echoMode() == PasswordEchoOnEdit && !d->control->passwordEchoEditing()) {
      // Clear the edit and reset to normal echo mode while entering input
      // method data; the echo mode switches back when the edit loses focus.
      // ### changes a public property, resets current content.
      d->updatePasswordEchoEditing(true);
      clear();
   }

#ifdef QT_KEYPAD_NAVIGATION
   // Focus in if currently in navigation focus on the widget
   // Only focus in on preedits, to allow input methods to
   // commit text as they focus out without interfering with focus
   if (QApplication::keypadNavigationEnabled()
      && hasFocus() && !hasEditFocus()
      && !e->preeditString().isEmpty()) {
      setEditFocus(true);
   }
#endif

   d->control->processInputMethodEvent(e);

#ifndef QT_NO_COMPLETER
   if (!e->commitString().isEmpty()) {
      d->control->complete(Qt::Key_unknown);
   }
#endif
}

/*!\reimp
*/
QVariant QLineEdit::inputMethodQuery(Qt::InputMethodQuery property) const
{
   Q_D(const QLineEdit);
   switch (property) {
      case Qt::ImCursorRectangle:
         return d->cursorRect();
      case Qt::ImFont:
         return font();
      case Qt::ImCursorPosition:
         return QVariant(d->control->cursor());
      case Qt::ImSurroundingText:
         return QVariant(d->control->text());
      case Qt::ImCurrentSelection:
         return QVariant(selectedText());
      case Qt::ImMaximumTextLength:
         return QVariant(maxLength());
      case Qt::ImAnchorPosition:
         if (d->control->selectionStart() == d->control->selectionEnd()) {
            return QVariant(d->control->cursor());
         } else if (d->control->selectionStart() == d->control->cursor()) {
            return QVariant(d->control->selectionEnd());
         } else {
            return QVariant(d->control->selectionStart());
         }
      default:
         return QWidget::inputMethodQuery(property);
   }
}

/*!\reimp
*/

void QLineEdit::focusInEvent(QFocusEvent *e)
{
   Q_D(QLineEdit);
   if (e->reason() == Qt::TabFocusReason ||
      e->reason() == Qt::BacktabFocusReason  ||
      e->reason() == Qt::ShortcutFocusReason) {
      if (!d->control->inputMask().isEmpty()) {
         d->control->moveCursor(d->control->nextMaskBlank(0));
      } else if (!d->control->hasSelectedText()) {
         selectAll();
      }
   } else if (e->reason() == Qt::MouseFocusReason) {
      d->clickCausedFocus = 1;
   }

#ifdef QT_KEYPAD_NAVIGATION
   if (! QApplication::keypadNavigationEnabled() || (hasEditFocus() && ( e->reason() == Qt::PopupFocusReason))) {
#endif

      d->control->setCursorBlinkPeriod(QApplication::cursorFlashTime());
      QStyleOptionFrame opt;
      initStyleOption(&opt);

      if ((! hasSelectedText() && d->control->preeditAreaText().isEmpty())
               || style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected, &opt, this)) {
         d->setCursorVisible(true);
      }

#ifdef QT_KEYPAD_NAVIGATION
      d->control->setCancelText(d->control->text());
   }
#endif

#ifndef QT_NO_COMPLETER
   if (d->control->completer()) {
      d->control->completer()->setWidget(this);

      QObject::connect(d->control->completer(), cs_mp_cast<const QString &>(&QCompleter::activated),
            this, &QLineEdit::setText);

      QObject::connect(d->control->completer(), cs_mp_cast<const QString &>(&QCompleter::highlighted),
            this, &QLineEdit::_q_completionHighlighted);
   }

#endif
   update();
}

void QLineEdit::focusOutEvent(QFocusEvent *e)
{
   Q_D(QLineEdit);

   if (d->control->passwordEchoEditing()) {
      // Reset the echomode back to PasswordEchoOnEdit when the widget loses focus
      d->updatePasswordEchoEditing(false);
   }

   Qt::FocusReason reason = e->reason();
   if (reason != Qt::ActiveWindowFocusReason &&
      reason != Qt::PopupFocusReason) {
      deselect();
   }

   d->setCursorVisible(false);
   d->control->setCursorBlinkPeriod(0);

#ifdef QT_KEYPAD_NAVIGATION
   // editingFinished() is already emitted on LeaveEditFocus
   if (! QApplication::keypadNavigationEnabled())
#endif

      if (reason != Qt::PopupFocusReason
            || ! (QApplication::activePopupWidget() && QApplication::activePopupWidget()->parentWidget() == this)) {
         if (hasAcceptableInput() || d->control->fixup()) {
            emit editingFinished();
         }
      }

#ifdef QT_KEYPAD_NAVIGATION
   d->control->setCancelText(QString());
#endif

#ifndef QT_NO_COMPLETER
   if (d->control->completer()) {
      QObject::disconnect(d->control->completer(), QString(), this, QString());
   }
#endif

   QWidget::focusOutEvent(e);
}

void QLineEdit::paintEvent(QPaintEvent *)
{
   Q_D(QLineEdit);
   QPainter p(this);

   QPalette pal = palette();

   QStyleOptionFrame panel;
   initStyleOption(&panel);
   style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);

   QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
   r.setX(r.x() + d->effectiveLeftTextMargin());
   r.setY(r.y() + d->topTextMargin);
   r.setRight(r.right() - d->effectiveRightTextMargin());
   r.setBottom(r.bottom() - d->bottomTextMargin);
   p.setClipRect(r);

   QFontMetrics fm = fontMetrics();
   Qt::Alignment va = QStyle::visualAlignment(d->control->layoutDirection(), QFlag(d->alignment));

   switch (va & Qt::AlignVertical_Mask) {
      case Qt::AlignBottom:
         d->vscroll = r.y() + r.height() - fm.height() - d->verticalMargin;
         break;

      case Qt::AlignTop:
         d->vscroll = r.y() + d->verticalMargin;
         break;

      default:
         //center
         d->vscroll = r.y() + (r.height() - fm.height() + 1) / 2;
         break;
   }

   QRect lineRect(r.x() + d->horizontalMargin, d->vscroll, r.width() - 2 * d->horizontalMargin, fm.height());

   if (d->shouldShowPlaceholderText()) {
      if (!d->placeholderText.isEmpty()) {
         QColor col = pal.text().color();
         col.setAlpha(128);
         QPen oldpen = p.pen();
         p.setPen(col);

         QString elidedText = fm.elidedText(d->placeholderText, Qt::ElideRight, lineRect.width());
         p.drawText(lineRect, va, elidedText);
         p.setPen(oldpen);

      }
   }

   int cix = qRound(d->control->cursorToX());

   // horizontal scrolling. d->hscroll is the left indent from the beginning
   // of the text line to the left edge of lineRect. we update this value
   // depending on the delta from the last paint event; in effect this means
   // the below code handles all scrolling based on the textline (widthUsed,
   // minLB, minRB), the line edit rect (lineRect) and the cursor position
   // (cix).
   int widthUsed = qRound(d->control->naturalTextWidth()) + 1;
   if (widthUsed <= lineRect.width()) {
      // text fits in lineRect; use hscroll for alignment
      switch (va & ~(Qt::AlignAbsolute | Qt::AlignVertical_Mask)) {
         case Qt::AlignRight:
            d->hscroll = widthUsed - lineRect.width() + 1;
            break;
         case Qt::AlignHCenter:
            d->hscroll = (widthUsed - lineRect.width()) / 2;
            break;
         default:
            // Left
            d->hscroll = 0;
            break;
      }

   } else if (cix - d->hscroll >= lineRect.width()) {
      // text doesn't fit, cursor is to the right of lineRect (scroll right)
      d->hscroll = cix - lineRect.width() + 1;
   } else if (cix - d->hscroll < 0 && d->hscroll < widthUsed) {
      // text doesn't fit, cursor is to the left of lineRect (scroll left)
      d->hscroll = cix;
   } else if (widthUsed - d->hscroll < lineRect.width()) {
      // text doesn't fit, text document is to the left of lineRect; align
      // right
      d->hscroll = widthUsed - lineRect.width() + 1;
   } else {
      //in case the text is bigger than the lineedit, the hscroll can never be negative
      d->hscroll = qMax(0, d->hscroll);
   }

   // the y offset is there to keep the baseline constant in case we have script changes in the text.
   QPoint topLeft = lineRect.topLeft() - QPoint(d->hscroll, d->control->ascent() - fm.ascent());

   // draw text, selections and cursors
#ifndef QT_NO_STYLE_STYLESHEET
   if (QStyleSheetStyle *cssStyle = qobject_cast<QStyleSheetStyle *>(style())) {
      cssStyle->styleSheetPalette(this, &panel, &pal);
   }
#endif
   p.setPen(pal.text().color());

   int flags = QLineControl::DrawText;

#ifdef QT_KEYPAD_NAVIGATION
   if (!QApplication::keypadNavigationEnabled() || hasEditFocus())
#endif
      if (d->control->hasSelectedText() || (d->cursorVisible && !d->control->inputMask().isEmpty() &&
            !d->control->isReadOnly())) {
         flags |= QLineControl::DrawSelections;
         // Palette only used for selections/mask and may not be in sync
         if (d->control->palette() != pal
            || d->control->palette().currentColorGroup() != pal.currentColorGroup()) {
            d->control->setPalette(pal);
         }
      }

   // Asian users see an IM selection text as cursor on candidate
   // selection phase of input method, so the ordinary cursor should be
   // invisible if we have a preedit string.
   if (d->cursorVisible && !d->control->isReadOnly()) {
      flags |= QLineControl::DrawCursor;
   }

   d->control->setCursorWidth(style()->pixelMetric(QStyle::PM_TextCursorWidth));
   d->control->draw(&p, topLeft, r, flags);

}


#ifndef QT_NO_DRAGANDDROP
/*!\reimp
*/
void QLineEdit::dragMoveEvent(QDragMoveEvent *e)
{
   Q_D(QLineEdit);
   if (!d->control->isReadOnly() && e->mimeData()->hasFormat(QLatin1String("text/plain"))) {
      e->acceptProposedAction();
      d->control->moveCursor(d->xToPos(e->pos().x()), false);
      d->cursorVisible = true;
      update();
   }
}

/*!\reimp */
void QLineEdit::dragEnterEvent(QDragEnterEvent *e)
{
   QLineEdit::dragMoveEvent(e);
}

/*!\reimp */
void QLineEdit::dragLeaveEvent(QDragLeaveEvent *)
{
   Q_D(QLineEdit);
   if (d->cursorVisible) {
      d->cursorVisible = false;
      update();
   }
}

/*!\reimp */
void QLineEdit::dropEvent(QDropEvent *e)
{
   Q_D(QLineEdit);
   QString str = e->mimeData()->text();

   if (! str.isEmpty() && !d->control->isReadOnly()) {
      if (e->source() == this && e->dropAction() == Qt::CopyAction) {
         deselect();
      }
      int cursorPos = d->xToPos(e->pos().x());
      int selStart = cursorPos;
      int oldSelStart = d->control->selectionStart();
      int oldSelEnd = d->control->selectionEnd();
      d->control->moveCursor(cursorPos, false);
      d->cursorVisible = false;
      e->acceptProposedAction();
      insert(str);
      if (e->source() == this) {
         if (e->dropAction() == Qt::MoveAction) {
            if (selStart > oldSelStart && selStart <= oldSelEnd) {
               setSelection(oldSelStart, str.length());
            } else if (selStart > oldSelEnd) {
               setSelection(selStart - str.length(), str.length());
            } else {
               setSelection(selStart, str.length());
            }
         } else {
            setSelection(selStart, str.length());
         }
      }
   } else {
      e->ignore();
      update();
   }
}

#endif // QT_NO_DRAGANDDROP

#ifndef QT_NO_CONTEXTMENU
void QLineEdit::contextMenuEvent(QContextMenuEvent *event)
{
   if (QMenu *menu = createStandardContextMenu()) {
      menu->setAttribute(Qt::WA_DeleteOnClose);
      menu->popup(event->globalPos());
   }
}

static inline void setActionIcon(QAction *action, const QString &name)
{
   const QIcon icon = QIcon::fromTheme(name);

   if (!icon.isNull()) {
      action->setIcon(icon);
   }
}

QMenu *QLineEdit::createStandardContextMenu()
{
   Q_D(QLineEdit);

   QMenu *popup = new QMenu(this);
   popup->setObjectName("qt_edit_menu");
   QAction *action = nullptr;

   if (!isReadOnly()) {
      action = popup->addAction(QLineEdit::tr("&Undo") + ACCEL_KEY(QKeySequence::Undo));
      action->setEnabled(d->control->isUndoAvailable());

      setActionIcon(action, "edit-undo");
      connect(action, &QAction::triggered, this, &QLineEdit::undo);

      action = popup->addAction(QLineEdit::tr("&Redo") + ACCEL_KEY(QKeySequence::Redo));
      action->setEnabled(d->control->isRedoAvailable());
      setActionIcon(action, "edit-redo");
      connect(action, &QAction::triggered, this, &QLineEdit::redo);

      popup->addSeparator();
   }

#ifndef QT_NO_CLIPBOARD
   if (! isReadOnly()) {
      action = popup->addAction(QLineEdit::tr("Cu&t") + ACCEL_KEY(QKeySequence::Cut));
      action->setEnabled(!d->control->isReadOnly() && d->control->hasSelectedText()
                  && d->control->echoMode() == QLineEdit::Normal);

      setActionIcon(action, "edit-cut");
      connect(action, &QAction::triggered, this, &QLineEdit::cut);
   }

   action = popup->addAction(QLineEdit::tr("&Copy") + ACCEL_KEY(QKeySequence::Copy));
   action->setEnabled(d->control->hasSelectedText() && d->control->echoMode() == QLineEdit::Normal);
   setActionIcon(action, "edit-copy");
   connect(action, &QAction::triggered, this, &QLineEdit::copy);

   if (! isReadOnly()) {
      action = popup->addAction(QLineEdit::tr("&Paste") + ACCEL_KEY(QKeySequence::Paste));
      action->setEnabled(! d->control->isReadOnly() && ! QApplication::clipboard()->text().isEmpty());

      setActionIcon(action, "edit-paste");
      connect(action, &QAction::triggered, this, &QLineEdit::paste);
   }
#endif

   if (! isReadOnly()) {
      action = popup->addAction(QLineEdit::tr("Delete"));
      action->setEnabled(! d->control->isReadOnly() && ! d->control->text().isEmpty() && d->control->hasSelectedText());

      setActionIcon(action, "edit-delete");
      connect(action, &QAction::triggered, d->control, &QLineControl::_q_deleteSelected);
   }

   if (! popup->isEmpty()) {
      popup->addSeparator();
   }

   action = popup->addAction(QLineEdit::tr("Select All") + ACCEL_KEY(QKeySequence::SelectAll));
   action->setEnabled(!d->control->text().isEmpty() && ! d->control->allSelected());
   d->selectAllAction = action;
   connect(action, &QAction::triggered, this, &QLineEdit::selectAll);


   if (! d->control->isReadOnly() && QGuiApplication::styleHints()->useRtlExtensions()) {

      popup->addSeparator();
      QUnicodeControlCharacterMenu *ctrlCharacterMenu = new QUnicodeControlCharacterMenu(this, popup);
      popup->addMenu(ctrlCharacterMenu);
   }
   return popup;
}
#endif // QT_NO_CONTEXTMENU

/*! \reimp */
void QLineEdit::changeEvent(QEvent *ev)
{
   Q_D(QLineEdit);
   switch (ev->type()) {
      case QEvent::ActivationChange:
         if (!palette().isEqual(QPalette::Active, QPalette::Inactive)) {
            update();
         }
         break;

      case QEvent::FontChange:
         d->control->setFont(font());
         break;

      case QEvent::StyleChange: {
         QStyleOptionFrame opt;
         initStyleOption(&opt);
         d->control->setPasswordCharacter(style()->styleHint(QStyle::SH_LineEdit_PasswordCharacter, &opt, this));
         d->control->setPasswordMaskDelay(style()->styleHint(QStyle::SH_LineEdit_PasswordMaskDelay, &opt, this));
       }

       update();
       break;

      case QEvent::LayoutDirectionChange:
         for (const QLineEditPrivate::SideWidgetEntry &e : d->trailingSideWidgets) {
            // Refresh icon to show arrow in right direction.

            if (e.flags & QLineEditPrivate::SideWidgetClearButton) {
               static_cast<QLineEditIconButton *>(e.widget)->setIcon(d->clearButtonIcon());
            }
         }

         d->positionSideWidgets();
         break;

      default:
         break;
   }
   QWidget::changeEvent(ev);
}

void QLineEdit::_q_handleWindowActivate()
{
   Q_D(QLineEdit);
   d->_q_handleWindowActivate();
}

void QLineEdit::_q_textEdited(const QString &newText)
{
   Q_D(QLineEdit);
   d->_q_textEdited(newText);
}

void QLineEdit::_q_cursorPositionChanged(int oldValue, int newValue)
{
   Q_D(QLineEdit);
   d->_q_cursorPositionChanged(oldValue, newValue);
}

#ifndef QT_NO_COMPLETER
void QLineEdit::_q_completionHighlighted(const QString &text)
{
   Q_D(QLineEdit);
   d->_q_completionHighlighted(text);
}
#endif

#ifdef QT_KEYPAD_NAVIGATION
void QLineEdit::_q_editFocusChange(bool isFocusChanged)
{
   Q_D(QLineEdit);
   d->_q_editFocusChange(isFocusChanged);
}
#endif

void QLineEdit::_q_selectionChanged()
{
   Q_D(QLineEdit);
   d->_q_selectionChanged();
}

void QLineEdit::_q_updateNeeded(const QRect &rect)
{
   Q_D(QLineEdit);
   d->_q_updateNeeded(rect);
}

void QLineEdit::_q_textChanged(const QString &newText)
{
   Q_D(QLineEdit);
   d->_q_textChanged(newText);
}

void QLineEdit::_q_clearButtonClicked()
{
   Q_D(QLineEdit);
   d->_q_clearButtonClicked();
}

#endif // QT_NO_LINEEDIT
