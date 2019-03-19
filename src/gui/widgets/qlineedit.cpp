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

#ifndef QT_NO_IM
#include <qinputcontext.h>
#include <qlist.h>
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

QT_BEGIN_NAMESPACE

#ifdef Q_OS_MAC
extern void qt_mac_secure_keyboard(bool);
#endif

/*!
    Initialize \a option with the values from this QLineEdit. This method
    is useful for subclasses when they need a QStyleOptionFrame or QStyleOptionFrameV2, but don't want
    to fill in all the information themselves. This function will check the version
    of the QStyleOptionFrame and fill in the additional values for a
    QStyleOptionFrameV2.

    \sa QStyleOption::initFrom()
*/
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
   if (QStyleOptionFrameV2 *optionV2 = qstyleoption_cast<QStyleOptionFrameV2 *>(option)) {
      optionV2->features = QStyleOptionFrameV2::None;
   }
}

QLineEdit::QLineEdit(QWidget *parent)
   : QWidget(*new QLineEditPrivate, parent, 0)
{
   Q_D(QLineEdit);
   d->init(QString());
}


QLineEdit::QLineEdit(const QString &contents, QWidget *parent)
   : QWidget(*new QLineEditPrivate, parent, 0)
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

/*!
    \since 4.7

    \property QLineEdit::placeholderText
    \brief the line edit's placeholder text

    Setting this property makes the line edit display a grayed-out
    placeholder text as long as the text() is empty and the widget doesn't
    have focus.

    By default, this property contains an empty string.

    \sa text()
*/
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
      if (!hasFocus()) {
         update();
      }
   }
}

/*!
    \property QLineEdit::displayText
    \brief the displayed text

    If \l echoMode is \l Normal this returns the same as text(); if
    \l EchoMode is \l Password or \l PasswordEchoOnEdit it returns a string of asterisks
    text().length() characters long, e.g. "******"; if \l EchoMode is
    \l NoEcho returns an empty string, "".

    By default, this property contains an empty string.

    \sa setEchoMode() text() EchoMode
*/

QString QLineEdit::displayText() const
{
   Q_D(const QLineEdit);
   return d->control->displayText();
}


/*!
    \property QLineEdit::maxLength
    \brief the maximum permitted length of the text

    If the text is too long, it is truncated at the limit.

    If truncation occurs any selected text will be unselected, the
    cursor position is set to 0 and the first part of the string is
    shown.

    If the line edit has an input mask, the mask defines the maximum
    string length.

    By default, this property contains a value of 32767.

    \sa inputMask
*/

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

/*!
    \property QLineEdit::frame
    \brief whether the line edit draws itself with a frame

    If enabled (the default) the line edit draws itself inside a
    frame, otherwise the line edit draws itself without any frame.
*/
bool QLineEdit::hasFrame() const
{
   Q_D(const QLineEdit);
   return d->frame;
}


void QLineEdit::setFrame(bool enable)
{
   Q_D(QLineEdit);
   d->frame = enable;
   update();
   updateGeometry();
}


/*!
    \enum QLineEdit::EchoMode

    This enum type describes how a line edit should display its
    contents.

    \value Normal   Display characters as they are entered. This is the
                    default.
    \value NoEcho   Do not display anything. This may be appropriate
                    for passwords where even the length of the
                    password should be kept secret.
    \value Password  Display asterisks instead of the characters
                    actually entered.
    \value PasswordEchoOnEdit Display characters as they are entered
                    while editing otherwise display asterisks.

    \sa setEchoMode() echoMode()
*/


/*!
    \property QLineEdit::echoMode
    \brief the line edit's echo mode

    The echo mode determines how the text entered in the line edit is
    displayed (or echoed) to the user.

    The most common setting is \l Normal, in which the text entered by the
    user is displayed verbatim, but QLineEdit also supports modes that allow
    the entered text to be suppressed or obscured: these include \l NoEcho,
    \l Password and \l PasswordEchoOnEdit.

    The widget's display and the ability to copy or drag the text is
    affected by this setting.

    By default, this property is set to \l Normal.

    \sa EchoMode displayText()
*/

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
      imHints |= (Qt::ImhNoAutoUppercase | Qt::ImhNoPredictiveText);
   } else {
      imHints &= ~(Qt::ImhNoAutoUppercase | Qt::ImhNoPredictiveText);
   }
   setInputMethodHints(imHints);
   d->control->setEchoMode(mode);
   update();

#ifdef Q_OS_MAC
   if (hasFocus()) {
      qt_mac_secure_keyboard(mode == Password || mode == NoEcho);
   }
#endif
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
#endif // QT_NO_VALIDATOR

#ifndef QT_NO_COMPLETER

void QLineEdit::setCompleter(QCompleter *c)
{
   Q_D(QLineEdit);

   if (c == d->control->completer()) {
      return;
   }

   if (d->control->completer()) {
      disconnect(d->control->completer(), 0, this, 0);
      d->control->completer()->setWidget(0);

      if (d->control->completer()->parent() == this) {
         delete d->control->completer();
      }
   }

   d->control->setCompleter(c);

   if (!c) {
      return;
   }

   if (c->widget() == 0) {
      c->setWidget(this);
   }

   if (hasFocus()) {
      QObject::connect(d->control->completer(), SIGNAL(activated(const QString &)),   this, SLOT(setText(const QString &)));
      QObject::connect(d->control->completer(), SIGNAL(highlighted(const QString &)), this,
                       SLOT(_q_completionHighlighted(const QString &)));
   }
}

/*!
    \since 4.2

    Returns the current QCompleter that provides completions.
*/
QCompleter *QLineEdit::completer() const
{
   Q_D(const QLineEdit);
   return d->control->completer();
}

#endif // QT_NO_COMPLETER

/*!
    Returns a recommended size for the widget.

    The width returned, in pixels, is usually enough for about 15 to
    20 characters.
*/

QSize QLineEdit::sizeHint() const
{
   Q_D(const QLineEdit);
   ensurePolished();
   QFontMetrics fm(font());
   int h = qMax(fm.height(), 14) + 2 * d->verticalMargin
           + d->topTextMargin + d->bottomTextMargin
           + d->topmargin + d->bottommargin;
   int w = fm.width(QLatin1Char('x')) * 17 + 2 * d->horizontalMargin
           + d->leftTextMargin + d->rightTextMargin
           + d->leftmargin + d->rightmargin; // "some"
   QStyleOptionFrameV2 opt;
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
           + d->topmargin + d->bottommargin;
   int w = fm.maxWidth() + d->leftmargin + d->rightmargin;
   QStyleOptionFrameV2 opt;
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
      QStyleOptionFrameV2 opt;
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

/*!
    \property QLineEdit::inputMask
    \brief The validation input mask

    If no mask is set, inputMask() returns an empty string.

    Sets the QLineEdit's validation mask. Validators can be used
    instead of, or in conjunction with masks; see setValidator().

    Unset the mask and return to normal QLineEdit operation by passing
    an empty string ("") or just calling setInputMask() with no
    arguments.

    The table below shows the characters that can be used in an input mask.
    A space character, the default character for a blank, is needed for cases
    where a character is \e{permitted but not required}.

    \table
    \header \i Character \i Meaning
    \row \i \c A \i ASCII alphabetic character required. A-Z, a-z.
    \row \i \c a \i ASCII alphabetic character permitted but not required.
    \row \i \c N \i ASCII alphanumeric character required. A-Z, a-z, 0-9.
    \row \i \c n \i ASCII alphanumeric character permitted but not required.
    \row \i \c X \i Any character required.
    \row \i \c x \i Any character permitted but not required.
    \row \i \c 9 \i ASCII digit required. 0-9.
    \row \i \c 0 \i ASCII digit permitted but not required.
    \row \i \c D \i ASCII digit required. 1-9.
    \row \i \c d \i ASCII digit permitted but not required (1-9).
    \row \i \c # \i ASCII digit or plus/minus sign permitted but not required.
    \row \i \c H \i Hexadecimal character required. A-F, a-f, 0-9.
    \row \i \c h \i Hexadecimal character permitted but not required.
    \row \i \c B \i Binary character required. 0-1.
    \row \i \c b \i Binary character permitted but not required.
    \row \i \c > \i All following alphabetic characters are uppercased.
    \row \i \c < \i All following alphabetic characters are lowercased.
    \row \i \c ! \i Switch off case conversion.
    \row \i \tt{\\} \i Use \tt{\\} to escape the special
                           characters listed above to use them as
                           separators.
    \endtable

    The mask consists of a string of mask characters and separators,
    optionally followed by a semicolon and the character used for
    blanks. The blank characters are always removed from the text
    after editing.

    Examples:
    \table
    \header \i Mask \i Notes
    \row \i \c 000.000.000.000;_ \i IP address; blanks are \c{_}.
    \row \i \c HH:HH:HH:HH:HH:HH;_ \i MAC address
    \row \i \c 0000-00-00 \i ISO Date; blanks are \c space
    \row \i \c >AAAAA-AAAAA-AAAAA-AAAAA-AAAAA;# \i License number;
    blanks are \c - and all (alphabetic) characters are converted to
    uppercase.
    \endtable

    To get range control (e.g., for an IP address) use masks together
    with \link setValidator() validators\endlink.

    \sa maxLength
*/
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


/*!
    Deletes any selected text, inserts \a newText, and validates the
    result. If it is valid, it sets it as the new contents of the line
    edit.

    \sa setText(), clear()
*/
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
   resetInputContext();
   d->control->clear();
}

/*!
    Undoes the last operation if undo is \link
    QLineEdit::undoAvailable available\endlink. Deselects any current
    selection, and updates the selection start to the current cursor
    position.
*/
void QLineEdit::undo()
{
   Q_D(QLineEdit);
   resetInputContext();
   d->control->undo();
}

/*!
    Redoes the last operation if redo is \link
    QLineEdit::redoAvailable available\endlink.
*/
void QLineEdit::redo()
{
   Q_D(QLineEdit);
   resetInputContext();
   d->control->redo();
}


/*!
    \property QLineEdit::readOnly
    \brief whether the line edit is read only.

    In read-only mode, the user can still copy the text to the
    clipboard, or drag and drop the text (if echoMode() is \l Normal),
    but cannot edit it.

    QLineEdit does not show a cursor in read-only mode.

    By default, this property is false.

    \sa setEnabled()
*/

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
      setAttribute(Qt::WA_MacShowFocusRect, !enable);
      setAttribute(Qt::WA_InputMethodEnabled, d->shouldEnableInputMethod());
#ifndef QT_NO_CURSOR
      setCursor(enable ? Qt::ArrowCursor : Qt::IBeamCursor);
#endif
      update();
   }
}


#ifndef QT_NO_CLIPBOARD
/*!
    Copies the selected text to the clipboard and deletes it, if there
    is any, and if echoMode() is \l Normal.

    If the current validator disallows deleting the selected text,
    cut() will copy without deleting.

    \sa copy() paste() setValidator()
*/

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

      if (timerId == d->dndTimer.timerId()) {
         d->drag();

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

   } else if (e->type() == QEvent::ShortcutOverride) {
      d->control->processEvent(e);

   } else if (e->type() == QEvent::KeyRelease) {
      d->control->setCursorBlinkPeriod(QApplication::cursorFlashTime());

   } else if (e->type() == QEvent::Show) {
      //In order to get the cursor blinking if QComboBox::setEditable is called when the combobox has focus
      if (hasFocus()) {
         d->control->setCursorBlinkPeriod(QApplication::cursorFlashTime());
         QStyleOptionFrameV2 opt;
         initStyleOption(&opt);

         if ((! hasSelectedText() && d->control->preeditAreaText().isEmpty())
               || style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected, &opt, this)) {
            d->setCursorVisible(true);
         }
      }

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
   int cursor = d->xToPos(e->pos().x());
#ifndef QT_NO_DRAGANDDROP
   if (!mark && d->dragEnabled && d->control->echoMode() == Normal &&
         e->button() == Qt::LeftButton && d->control->inSelection(e->pos().x())) {
      d->dndPos = e->pos();
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
   if (d->sendMouseEventToInputContext(e)) {
      return;
   }

   if (e->buttons() & Qt::LeftButton) {
#ifndef QT_NO_DRAGANDDROP
      if (d->dndTimer.isActive()) {
         if ((d->dndPos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
            d->drag();
         }
      } else
#endif
      {
         d->control->moveCursor(d->xToPos(e->pos().x()), true);
      }
   }
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
   if (d->sendMouseEventToInputContext(e)) {
      return;
   }
   if (e->button() == Qt::LeftButton) {
      d->control->selectWordAtPos(d->xToPos(e->pos().x()));
      d->tripleClickTimer.start(QApplication::doubleClickInterval(), this);
      d->tripleClick = e->pos();
   }
}

/*!
    \fn void  QLineEdit::returnPressed()

    This signal is emitted when the Return or Enter key is pressed.
    Note that if there is a validator() or inputMask() set on the line
    edit, the returnPressed() signal will only be emitted if the input
    follows the inputMask() and the validator() returns
    QValidator::Acceptable.
*/

/*!
    \fn void  QLineEdit::editingFinished()

    This signal is emitted when the Return or Enter key is pressed or
    the line edit loses focus. Note that if there is a validator() or
    inputMask() set on the line edit and enter/return is pressed, the
    editingFinished() signal will only be emitted if the input follows
    the inputMask() and the validator() returns QValidator::Acceptable.
*/

/*!
    Converts the given key press \a event into a line edit action.

    If Return or Enter is pressed and the current text is valid (or
    can be \link QValidator::fixup() made valid\endlink by the
    validator), the signal returnPressed() is emitted.

    The default key bindings are listed in the class's detailed
    description.
*/

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

/*!
  \since 4.4

  Returns a rectangle that includes the lineedit cursor.
*/
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
      case Qt::ImMicroFocus:
         return d->cursorRect();
      case Qt::ImFont:
         return font();
      case Qt::ImCursorPosition:
         return QVariant(d->control->cursor());
      case Qt::ImSurroundingText:
         return QVariant(text());
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
         return QVariant();
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
   if (!QApplication::keypadNavigationEnabled() || (hasEditFocus() && ( e->reason() == Qt::PopupFocusReason
                                                                      ))) {
#endif

      d->control->setCursorBlinkPeriod(QApplication::cursorFlashTime());
      QStyleOptionFrameV2 opt;
      initStyleOption(&opt);

      if ((!hasSelectedText() && d->control->preeditAreaText().isEmpty())
            || style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected, &opt, this)) {
         d->setCursorVisible(true);
      }

#ifdef Q_OS_MAC
      if (d->control->echoMode() == Password || d->control->echoMode() == NoEcho) {
         qt_mac_secure_keyboard(true);
      }
#endif

#ifdef QT_KEYPAD_NAVIGATION
      d->control->setCancelText(d->control->text());
   }
#endif

#ifndef QT_NO_COMPLETER
   if (d->control->completer()) {
      d->control->completer()->setWidget(this);

      QObject::connect(d->control->completer(), SIGNAL(activated(const QString &)),   this, SLOT(setText(const QString &)));
      QObject::connect(d->control->completer(), SIGNAL(highlighted(const QString &)), this,
                       SLOT(_q_completionHighlighted(const QString &)));
   }
#endif
   update();
}

/*!\reimp
*/

void QLineEdit::focusOutEvent(QFocusEvent *e)
{
   Q_D(QLineEdit);
   if (d->control->passwordEchoEditing()) {
      // Reset the echomode back to PasswordEchoOnEdit when the widget loses
      // focus.
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
   if (!QApplication::keypadNavigationEnabled())
#endif
      if (reason != Qt::PopupFocusReason
            || !(QApplication::activePopupWidget() && QApplication::activePopupWidget()->parentWidget() == this)) {
         if (hasAcceptableInput() || d->control->fixup()) {
            emit editingFinished();
         }
      }

#ifdef Q_OS_MAC
   if (d->control->echoMode() == Password || d->control->echoMode() == NoEcho) {
      qt_mac_secure_keyboard(false);
   }
#endif

#ifdef QT_KEYPAD_NAVIGATION
   d->control->setCancelText(QString());
#endif

#ifndef QT_NO_COMPLETER
   if (d->control->completer()) {
      QObject::disconnect(d->control->completer(), 0, this, 0);
   }
#endif
   update();
}

/*!\reimp
*/
void QLineEdit::paintEvent(QPaintEvent *)
{
   Q_D(QLineEdit);
   QPainter p(this);

   QRect r = rect();
   QPalette pal = palette();

   QStyleOptionFrameV2 panel;
   initStyleOption(&panel);
   style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);
   r = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
   r.setX(r.x() + d->leftTextMargin);
   r.setY(r.y() + d->topTextMargin);
   r.setRight(r.right() - d->rightTextMargin);
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

   int minLB = qMax(0, -fm.minLeftBearing());
   int minRB = qMax(0, -fm.minRightBearing());

   if (d->control->text().isEmpty()) {
      if (!hasFocus() && !d->placeholderText.isEmpty()) {
         QColor col = pal.text().color();
         col.setAlpha(128);
         QPen oldpen = p.pen();
         p.setPen(col);
         lineRect.adjust(minLB, 0, 0, 0);
         QString elidedText = fm.elidedText(d->placeholderText, Qt::ElideRight, lineRect.width());
         p.drawText(lineRect, va, elidedText);
         p.setPen(oldpen);
         return;
      }
   }

   int cix = qRound(d->control->cursorToX());

   // horizontal scrolling. d->hscroll is the left indent from the beginning
   // of the text line to the left edge of lineRect. we update this value
   // depending on the delta from the last paint event; in effect this means
   // the below code handles all scrolling based on the textline (widthUsed,
   // minLB, minRB), the line edit rect (lineRect) and the cursor position
   // (cix).
   int widthUsed = qRound(d->control->naturalTextWidth()) + 1 + minRB;
   if ((minLB + widthUsed) <=  lineRect.width()) {
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
      d->hscroll -= minLB;
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
/*!
    Shows the standard context menu created with
    createStandardContextMenu().

    If you do not want the line edit to have a context menu, you can set
    its \l contextMenuPolicy to Qt::NoContextMenu. If you want to
    customize the context menu, reimplement this function. If you want
    to extend the standard context menu, reimplement this function, call
    createStandardContextMenu() and extend the menu returned.

    \snippet doc/src/snippets/code/src_gui_widgets_qlineedit.cpp 0

    The \a event parameter is used to obtain the position where
    the mouse cursor was when the event was generated.

    \sa setContextMenuPolicy()
*/
void QLineEdit::contextMenuEvent(QContextMenuEvent *event)
{
   if (QMenu *menu = createStandardContextMenu()) {
      menu->setAttribute(Qt::WA_DeleteOnClose);
      menu->popup(event->globalPos());
   }
}

#if defined(Q_OS_WIN) || defined(Q_WS_X11)
extern bool qt_use_rtl_extensions;
#endif

/*!  This function creates the standard context menu which is shown
        when the user clicks on the line edit with the right mouse
        button. It is called from the default contextMenuEvent() handler.
        The popup menu's ownership is transferred to the caller.
*/

QMenu *QLineEdit::createStandardContextMenu()
{
   Q_D(QLineEdit);
   QMenu *popup = new QMenu(this);
   popup->setObjectName(QLatin1String("qt_edit_menu"));
   QAction *action = 0;

   if (!isReadOnly()) {
      action = popup->addAction(QLineEdit::tr("&Undo") + ACCEL_KEY(QKeySequence::Undo));
      action->setEnabled(d->control->isUndoAvailable());
      connect(action, SIGNAL(triggered()), this, SLOT(undo()));

      action = popup->addAction(QLineEdit::tr("&Redo") + ACCEL_KEY(QKeySequence::Redo));
      action->setEnabled(d->control->isRedoAvailable());
      connect(action, SIGNAL(triggered()), this, SLOT(redo()));

      popup->addSeparator();
   }

#ifndef QT_NO_CLIPBOARD
   if (!isReadOnly()) {
      action = popup->addAction(QLineEdit::tr("Cu&t") + ACCEL_KEY(QKeySequence::Cut));
      action->setEnabled(!d->control->isReadOnly() && d->control->hasSelectedText()
                         && d->control->echoMode() == QLineEdit::Normal);
      connect(action, SIGNAL(triggered()), this, SLOT(cut()));
   }

   action = popup->addAction(QLineEdit::tr("&Copy") + ACCEL_KEY(QKeySequence::Copy));
   action->setEnabled(d->control->hasSelectedText() && d->control->echoMode() == QLineEdit::Normal);
   connect(action, SIGNAL(triggered()), this, SLOT(copy()));

   if (!isReadOnly()) {
      action = popup->addAction(QLineEdit::tr("&Paste") + ACCEL_KEY(QKeySequence::Paste));
      action->setEnabled(!d->control->isReadOnly() && !QApplication::clipboard()->text().isEmpty());
      connect(action, SIGNAL(triggered()), this, SLOT(paste()));
   }
#endif

   if (!isReadOnly()) {
      action = popup->addAction(QLineEdit::tr("Delete"));
      action->setEnabled(!d->control->isReadOnly() && !d->control->text().isEmpty() && d->control->hasSelectedText());
      connect(action, SIGNAL(triggered()), d->control, SLOT(_q_deleteSelected()));
   }

   if (!popup->isEmpty()) {
      popup->addSeparator();
   }

   action = popup->addAction(QLineEdit::tr("Select All") + ACCEL_KEY(QKeySequence::SelectAll));
   action->setEnabled(!d->control->text().isEmpty() && !d->control->allSelected());
   d->selectAllAction = action;
   connect(action, SIGNAL(triggered()), this, SLOT(selectAll()));

#if !defined(QT_NO_IM)
   QInputContext *qic = inputContext();

   if (qic) {
      QList<QAction *> imActions = qic->actions();
      for (int i = 0; i < imActions.size(); ++i) {
         popup->addAction(imActions.at(i));
      }
   }
#endif

#if defined(Q_OS_WIN) || defined(Q_WS_X11)
   if (!d->control->isReadOnly() && qt_use_rtl_extensions) {
#else
   if (!d->control->isReadOnly()) {
#endif
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
         QStyleOptionFrameV2 opt;
         initStyleOption(&opt);
         d->control->setPasswordCharacter(style()->styleHint(QStyle::SH_LineEdit_PasswordCharacter, &opt, this));
      }
      update();
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

void QLineEdit::_q_textEdited(const QString &un_named_arg1)
{
   Q_D(QLineEdit);
   d->_q_textEdited(un_named_arg1);
}

void QLineEdit::_q_cursorPositionChanged(int un_named_arg1, int un_named_arg2)
{
   Q_D(QLineEdit);
   d->_q_cursorPositionChanged(un_named_arg1, un_named_arg2);
}

#ifndef QT_NO_COMPLETER
void QLineEdit::_q_completionHighlighted(const QString &un_named_arg1)
{
   Q_D(QLineEdit);
   d->_q_completionHighlighted(un_named_arg1);
}
#endif

#ifdef QT_KEYPAD_NAVIGATION
void QLineEdit::_q_editFocusChange(bool un_named_arg1)
{
   Q_D(QLineEdit);
   d->_q_editFocusChange(un_named_arg1);
}
#endif

void QLineEdit::_q_selectionChanged()
{
   Q_D(QLineEdit);
   d->_q_selectionChanged();
}

void QLineEdit::_q_updateNeeded(const QRect &un_named_arg1)
{
   Q_D(QLineEdit);
   d->_q_updateNeeded(un_named_arg1);
}

QT_END_NAMESPACE

#endif // QT_NO_LINEEDIT
