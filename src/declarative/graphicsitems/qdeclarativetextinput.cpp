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

#include <qdeclarativetextinput_p.h>
#include <qdeclarativetextinput_p_p.h>
#include <qdeclarativeglobal_p.h>
#include <qdeclarativeinfo.h>
#include <QValidator>
#include <QTextCursor>
#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QTextBoundaryFinder>
#include <QInputContext>
#include <qstyle.h>

#ifndef QT_NO_LINEEDIT

QT_BEGIN_NAMESPACE

/*!
    \qmlclass TextInput QDeclarativeTextInput
    \ingroup qml-basic-visual-elements
    \since 4.7
    \brief The TextInput item displays an editable line of text.
    \inherits Item

    The TextInput element displays a single line of editable plain text.

    TextInput is used to accept a line of text input. Input constraints
    can be placed on a TextInput item (for example, through a \l validator or \l inputMask),
    and setting \l echoMode to an appropriate value enables TextInput to be used for
    a password input field.

    On Mac OS X, the Up/Down key bindings for Home/End are explicitly disabled.
    If you want such bindings (on any platform), you will need to construct them in QML.

    \sa TextEdit, Text, {declarative/text/textselection}{Text Selection example}
*/
QDeclarativeTextInput::QDeclarativeTextInput(QDeclarativeItem *parent)
   : QDeclarativeImplicitSizePaintedItem(*(new QDeclarativeTextInputPrivate), parent)
{
   Q_D(QDeclarativeTextInput);
   d->init();
}

QDeclarativeTextInput::~QDeclarativeTextInput()
{
}

/*!
    \qmlproperty string TextInput::text

    The text in the TextInput.
*/

QString QDeclarativeTextInput::text() const
{
   Q_D(const QDeclarativeTextInput);
   return d->control->text();
}

void QDeclarativeTextInput::setText(const QString &s)
{
   Q_D(QDeclarativeTextInput);
   if (s == text()) {
      return;
   }
   d->control->setText(s);
}

/*!
    \qmlproperty string TextInput::font.family

    Sets the family name of the font.

    The family name is case insensitive and may optionally include a foundry name, e.g. "Helvetica [Cronyx]".
    If the family is available from more than one foundry and the foundry isn't specified, an arbitrary foundry is chosen.
    If the family isn't available a family will be set using the font matching algorithm.
*/

/*!
    \qmlproperty bool TextInput::font.bold

    Sets whether the font weight is bold.
*/

/*!
    \qmlproperty enumeration TextInput::font.weight

    Sets the font's weight.

    The weight can be one of:
    \list
    \o Font.Light
    \o Font.Normal - the default
    \o Font.DemiBold
    \o Font.Bold
    \o Font.Black
    \endlist

    \qml
    TextInput { text: "Hello"; font.weight: Font.DemiBold }
    \endqml
*/

/*!
    \qmlproperty bool TextInput::font.italic

    Sets whether the font has an italic style.
*/

/*!
    \qmlproperty bool TextInput::font.underline

    Sets whether the text is underlined.
*/

/*!
    \qmlproperty bool TextInput::font.strikeout

    Sets whether the font has a strikeout style.
*/

/*!
    \qmlproperty real TextInput::font.pointSize

    Sets the font size in points. The point size must be greater than zero.
*/

/*!
    \qmlproperty int TextInput::font.pixelSize

    Sets the font size in pixels.

    Using this function makes the font device dependent.
    Use \c pointSize to set the size of the font in a device independent manner.
*/

/*!
    \qmlproperty real TextInput::font.letterSpacing

    Sets the letter spacing for the font.

    Letter spacing changes the default spacing between individual letters in the font.
    A positive value increases the letter spacing by the corresponding pixels; a negative value decreases the spacing.
*/

/*!
    \qmlproperty real TextInput::font.wordSpacing

    Sets the word spacing for the font.

    Word spacing changes the default spacing between individual words.
    A positive value increases the word spacing by a corresponding amount of pixels,
    while a negative value decreases the inter-word spacing accordingly.
*/

/*!
    \qmlproperty enumeration TextInput::font.capitalization

    Sets the capitalization for the text.

    \list
    \o Font.MixedCase - This is the normal text rendering option where no capitalization change is applied.
    \o Font.AllUppercase - This alters the text to be rendered in all uppercase type.
    \o Font.AllLowercase	 - This alters the text to be rendered in all lowercase type.
    \o Font.SmallCaps -	This alters the text to be rendered in small-caps type.
    \o Font.Capitalize - This alters the text to be rendered with the first character of each word as an uppercase character.
    \endlist

    \qml
    TextInput { text: "Hello"; font.capitalization: Font.AllLowercase }
    \endqml
*/

QFont QDeclarativeTextInput::font() const
{
   Q_D(const QDeclarativeTextInput);
   return d->sourceFont;
}

void QDeclarativeTextInput::setFont(const QFont &font)
{
   Q_D(QDeclarativeTextInput);
   if (d->sourceFont == font) {
      return;
   }

   d->sourceFont = font;
   QFont oldFont = d->font;
   d->font = font;
   if (d->font.pointSizeF() != -1) {
      // 0.5pt resolution
      qreal size = qRound(d->font.pointSizeF() * 2.0);
      d->font.setPointSizeF(size / 2.0);
   }

   if (oldFont != d->font) {
      d->control->setFont(d->font);
      updateSize();
      updateCursorRectangle();
      if (d->cursorItem) {
         d->cursorItem->setHeight(QFontMetrics(d->font).height());
      }
   }
   emit fontChanged(d->sourceFont);
}

/*!
    \qmlproperty color TextInput::color

    The text color.
*/
QColor QDeclarativeTextInput::color() const
{
   Q_D(const QDeclarativeTextInput);
   return d->color;
}

void QDeclarativeTextInput::setColor(const QColor &c)
{
   Q_D(QDeclarativeTextInput);
   if (c != d->color) {
      d->color = c;
      clearCache();
      update();
      emit colorChanged(c);
   }
}


/*!
    \qmlproperty color TextInput::selectionColor

    The text highlight color, used behind selections.
*/
QColor QDeclarativeTextInput::selectionColor() const
{
   Q_D(const QDeclarativeTextInput);
   return d->selectionColor;
}

void QDeclarativeTextInput::setSelectionColor(const QColor &color)
{
   Q_D(QDeclarativeTextInput);
   if (d->selectionColor == color) {
      return;
   }

   d->selectionColor = color;
   QPalette p = d->control->palette();
   p.setColor(QPalette::Highlight, d->selectionColor);
   d->control->setPalette(p);
   if (d->control->hasSelectedText()) {
      clearCache();
      update();
   }
   emit selectionColorChanged(color);
}

/*!
    \qmlproperty color TextInput::selectedTextColor

    The highlighted text color, used in selections.
*/
QColor QDeclarativeTextInput::selectedTextColor() const
{
   Q_D(const QDeclarativeTextInput);
   return d->selectedTextColor;
}

void QDeclarativeTextInput::setSelectedTextColor(const QColor &color)
{
   Q_D(QDeclarativeTextInput);
   if (d->selectedTextColor == color) {
      return;
   }

   d->selectedTextColor = color;
   QPalette p = d->control->palette();
   p.setColor(QPalette::HighlightedText, d->selectedTextColor);
   d->control->setPalette(p);
   if (d->control->hasSelectedText()) {
      clearCache();
      update();
   }
   emit selectedTextColorChanged(color);
}

/*!
    \qmlproperty enumeration TextInput::horizontalAlignment

    Sets the horizontal alignment of the text within the TextInput item's
    width and height. By default, the text alignment follows the natural alignment
    of the text, for example text that is read from left to right will be aligned to
    the left.

    TextInput does not have vertical alignment, as the natural height is
    exactly the height of the single line of text. If you set the height
    manually to something larger, TextInput will always be top aligned
    vertically. You can use anchors to align it however you want within
    another item.

    The valid values for \c horizontalAlignment are \c TextInput.AlignLeft, \c TextInput.AlignRight and
    \c TextInput.AlignHCenter.

    When using the attached property \l {LayoutMirroring::enabled} to mirror application
    layouts, the horizontal alignment of text will also be mirrored. However, the property
    \c horizontalAlignment will remain unchanged. To query the effective horizontal alignment
    of TextInput, use the property \l {LayoutMirroring::enabled}.
*/
QDeclarativeTextInput::HAlignment QDeclarativeTextInput::hAlign() const
{
   Q_D(const QDeclarativeTextInput);
   return d->hAlign;
}

void QDeclarativeTextInput::setHAlign(HAlignment align)
{
   Q_D(QDeclarativeTextInput);
   bool forceAlign = d->hAlignImplicit && d->effectiveLayoutMirror;
   d->hAlignImplicit = false;
   if (d->setHAlign(align, forceAlign) && isComponentComplete()) {
      updateCursorRectangle();
   }
}

void QDeclarativeTextInput::resetHAlign()
{
   Q_D(QDeclarativeTextInput);
   d->hAlignImplicit = true;
   if (d->determineHorizontalAlignment() && isComponentComplete()) {
      updateCursorRectangle();
   }
}

QDeclarativeTextInput::HAlignment QDeclarativeTextInput::effectiveHAlign() const
{
   Q_D(const QDeclarativeTextInput);
   QDeclarativeTextInput::HAlignment effectiveAlignment = d->hAlign;
   if (!d->hAlignImplicit && d->effectiveLayoutMirror) {
      switch (d->hAlign) {
         case QDeclarativeTextInput::AlignLeft:
            effectiveAlignment = QDeclarativeTextInput::AlignRight;
            break;
         case QDeclarativeTextInput::AlignRight:
            effectiveAlignment = QDeclarativeTextInput::AlignLeft;
            break;
         default:
            break;
      }
   }
   return effectiveAlignment;
}

bool QDeclarativeTextInputPrivate::setHAlign(QDeclarativeTextInput::HAlignment alignment, bool forceAlign)
{
   Q_Q(QDeclarativeTextInput);
   if ((hAlign != alignment || forceAlign) && alignment <= QDeclarativeTextInput::AlignHCenter) { // justify not supported
      hAlign = alignment;
      emit q->horizontalAlignmentChanged(alignment);
      return true;
   }
   return false;
}

bool QDeclarativeTextInputPrivate::determineHorizontalAlignment()
{
   if (hAlignImplicit) {
      // if no explicit alignment has been set, follow the natural layout direction of the text
      QString text = control->text();
      if (text.isEmpty()) {
         text = control->preeditAreaText();
      }
      bool isRightToLeft = text.isEmpty()
                           ? QApplication::keyboardInputDirection() == Qt::RightToLeft
                           : text.isRightToLeft();
      return setHAlign(isRightToLeft ? QDeclarativeTextInput::AlignRight : QDeclarativeTextInput::AlignLeft);
   }
   return false;
}

void QDeclarativeTextInputPrivate::mirrorChange()
{
   Q_Q(QDeclarativeTextInput);
   if (q->isComponentComplete()) {
      if (!hAlignImplicit && (hAlign == QDeclarativeTextInput::AlignRight || hAlign == QDeclarativeTextInput::AlignLeft)) {
         q->updateCursorRectangle();
         updateHorizontalScroll();
      }
   }
}

/*!
    \qmlproperty bool TextInput::readOnly

    Sets whether user input can modify the contents of the TextInput.

    If readOnly is set to true, then user input will not affect the text
    property. Any bindings or attempts to set the text property will still
    work.
*/

bool QDeclarativeTextInput::isReadOnly() const
{
   Q_D(const QDeclarativeTextInput);
   return d->control->isReadOnly();
}

void QDeclarativeTextInput::setReadOnly(bool ro)
{
   Q_D(QDeclarativeTextInput);
   if (d->control->isReadOnly() == ro) {
      return;
   }

   setFlag(QGraphicsItem::ItemAcceptsInputMethod, !ro);
   d->control->setReadOnly(ro);

   emit readOnlyChanged(ro);
}

/*!
    \qmlproperty int TextInput::maximumLength
    The maximum permitted length of the text in the TextInput.

    If the text is too long, it is truncated at the limit.

    By default, this property contains a value of 32767.
*/
int QDeclarativeTextInput::maxLength() const
{
   Q_D(const QDeclarativeTextInput);
   return d->control->maxLength();
}

void QDeclarativeTextInput::setMaxLength(int ml)
{
   Q_D(QDeclarativeTextInput);
   if (d->control->maxLength() == ml) {
      return;
   }

   d->control->setMaxLength(ml);

   emit maximumLengthChanged(ml);
}

/*!
    \qmlproperty bool TextInput::cursorVisible
    Set to true when the TextInput shows a cursor.

    This property is set and unset when the TextInput gets active focus, so that other
    properties can be bound to whether the cursor is currently showing. As it
    gets set and unset automatically, when you set the value yourself you must
    keep in mind that your value may be overwritten.

    It can be set directly in script, for example if a KeyProxy might
    forward keys to it and you desire it to look active when this happens
    (but without actually giving it active focus).

    It should not be set directly on the element, like in the below QML,
    as the specified value will be overridden an lost on focus changes.

    \code
    TextInput {
        text: "Text"
        cursorVisible: false
    }
    \endcode

    In the above snippet the cursor will still become visible when the
    TextInput gains active focus.
*/
bool QDeclarativeTextInput::isCursorVisible() const
{
   Q_D(const QDeclarativeTextInput);
   return d->cursorVisible;
}

void QDeclarativeTextInput::setCursorVisible(bool on)
{
   Q_D(QDeclarativeTextInput);
   if (d->cursorVisible == on) {
      return;
   }
   d->cursorVisible = on;
   d->control->setCursorBlinkPeriod(on ? QApplication::cursorFlashTime() : 0);
   QRect r = d->control->cursorRect();
   if (d->control->inputMask().isEmpty()) {
      updateRect(r);
   } else {
      updateRect();
   }
   emit cursorVisibleChanged(d->cursorVisible);
}

/*!
    \qmlproperty int TextInput::cursorPosition
    The position of the cursor in the TextInput.
*/
int QDeclarativeTextInput::cursorPosition() const
{
   Q_D(const QDeclarativeTextInput);
   return d->control->cursor();
}
void QDeclarativeTextInput::setCursorPosition(int cp)
{
   Q_D(QDeclarativeTextInput);
   if (cp < 0 || cp > d->control->text().length()) {
      return;
   }
   d->control->moveCursor(cp);
}

/*!
  Returns a Rect which encompasses the cursor, but which may be larger than is
  required. Ignores custom cursor delegates.
*/
QRect QDeclarativeTextInput::cursorRectangle() const
{
   Q_D(const QDeclarativeTextInput);
   QRect r = d->control->cursorRect();
   // Scroll and make consistent with TextEdit
   // QLineControl inexplicably adds 1 to the height and horizontal padding
   // for unicode direction markers.
   r.adjust(5 - d->hscroll, 0, -4 - d->hscroll, -1);
   return r;
}

/*!
    \qmlproperty int TextInput::selectionStart

    The cursor position before the first character in the current selection.

    This property is read-only. To change the selection, use select(start,end),
    selectAll(), or selectWord().

    \sa selectionEnd, cursorPosition, selectedText
*/
int QDeclarativeTextInput::selectionStart() const
{
   Q_D(const QDeclarativeTextInput);
   return d->lastSelectionStart;
}

/*!
    \qmlproperty int TextInput::selectionEnd

    The cursor position after the last character in the current selection.

    This property is read-only. To change the selection, use select(start,end),
    selectAll(), or selectWord().

    \sa selectionStart, cursorPosition, selectedText
*/
int QDeclarativeTextInput::selectionEnd() const
{
   Q_D(const QDeclarativeTextInput);
   return d->lastSelectionEnd;
}

/*!
    \qmlmethod void TextInput::select(int start, int end)

    Causes the text from \a start to \a end to be selected.

    If either start or end is out of range, the selection is not changed.

    After calling this, selectionStart will become the lesser
    and selectionEnd will become the greater (regardless of the order passed
    to this method).

    \sa selectionStart, selectionEnd
*/
void QDeclarativeTextInput::select(int start, int end)
{
   Q_D(QDeclarativeTextInput);
   if (start < 0 || end < 0 || start > d->control->text().length() || end > d->control->text().length()) {
      return;
   }
   d->control->setSelection(start, end - start);
}

/*!
    \qmlproperty string TextInput::selectedText

    This read-only property provides the text currently selected in the
    text input.

    It is equivalent to the following snippet, but is faster and easier
    to use.

    \js
    myTextInput.text.toString().substring(myTextInput.selectionStart,
        myTextInput.selectionEnd);
    \endjs
*/
QString QDeclarativeTextInput::selectedText() const
{
   Q_D(const QDeclarativeTextInput);
   return d->control->selectedText();
}

/*!
    \qmlproperty bool TextInput::activeFocusOnPress

    Whether the TextInput should gain active focus on a mouse press. By default this is
    set to true.
*/
bool QDeclarativeTextInput::focusOnPress() const
{
   Q_D(const QDeclarativeTextInput);
   return d->focusOnPress;
}

void QDeclarativeTextInput::setFocusOnPress(bool b)
{
   Q_D(QDeclarativeTextInput);
   if (d->focusOnPress == b) {
      return;
   }

   d->focusOnPress = b;

   emit activeFocusOnPressChanged(d->focusOnPress);
}

/*!
    \qmlproperty bool TextInput::autoScroll

    Whether the TextInput should scroll when the text is longer than the width. By default this is
    set to true.
*/
bool QDeclarativeTextInput::autoScroll() const
{
   Q_D(const QDeclarativeTextInput);
   return d->autoScroll;
}

void QDeclarativeTextInput::setAutoScroll(bool b)
{
   Q_D(QDeclarativeTextInput);
   if (d->autoScroll == b) {
      return;
   }

   d->autoScroll = b;
   //We need to repaint so that the scrolling is taking into account.
   updateSize(true);
   updateCursorRectangle();
   emit autoScrollChanged(d->autoScroll);
}

/*!
    \qmlclass IntValidator QIntValidator
    \ingroup qml-basic-visual-elements

    This element provides a validator for integer values.

    IntValidator uses the \l {QLocale::setDefault()}{default locale} to interpret the number and
    will accept locale specific digits, group separators, and positive and negative signs.  In
    addition, IntValidator is always guaranteed to accept a number formatted according to the "C"
    locale.
*/
/*!
    \qmlproperty int IntValidator::top

    This property holds the validator's highest acceptable value.
    By default, this property's value is derived from the highest signed integer available (typically 2147483647).
*/
/*!
    \qmlproperty int IntValidator::bottom

    This property holds the validator's lowest acceptable value.
    By default, this property's value is derived from the lowest signed integer available (typically -2147483647).
*/

/*!
    \qmlclass DoubleValidator QDoubleValidator
    \ingroup qml-basic-visual-elements

    This element provides a validator for non-integer numbers.
*/

/*!
    \qmlproperty real DoubleValidator::top

    This property holds the validator's maximum acceptable value.
    By default, this property contains a value of infinity.
*/
/*!
    \qmlproperty real DoubleValidator::bottom

    This property holds the validator's minimum acceptable value.
    By default, this property contains a value of -infinity.
*/
/*!
    \qmlproperty int DoubleValidator::decimals

    This property holds the validator's maximum number of digits after the decimal point.
    By default, this property contains a value of 1000.
*/
/*!
    \qmlproperty enumeration DoubleValidator::notation
    This property holds the notation of how a string can describe a number.

    The possible values for this property are:

    \list
    \o DoubleValidator.StandardNotation
    \o DoubleValidator.ScientificNotation (default)
    \endlist

    If this property is set to DoubleValidator.ScientificNotation, the written number may have an exponent part (e.g. 1.5E-2).
*/

/*!
    \qmlclass RegExpValidator QRegExpValidator
    \ingroup qml-basic-visual-elements

    This element provides a validator, which counts as valid any string which
    matches a specified regular expression.
*/
/*!
   \qmlproperty regExp RegExpValidator::regExp

   This property holds the regular expression used for validation.

   Note that this property should be a regular expression in JS syntax, e.g /a/ for the regular expression
   matching "a".

   By default, this property contains a regular expression with the pattern .* that matches any string.
*/

/*!
    \qmlproperty Validator TextInput::validator

    Allows you to set a validator on the TextInput. When a validator is set
    the TextInput will only accept input which leaves the text property in
    an acceptable or intermediate state. The accepted signal will only be sent
    if the text is in an acceptable state when enter is pressed.

    Currently supported validators are IntValidator, DoubleValidator and
    RegExpValidator. An example of using validators is shown below, which allows
    input of integers between 11 and 31 into the text input:

    \code
    import QtQuick 1.0
    TextInput{
        validator: IntValidator{bottom: 11; top: 31;}
        focus: true
    }
    \endcode

    \sa acceptableInput, inputMask
*/
#ifndef QT_NO_VALIDATOR
QValidator *QDeclarativeTextInput::validator() const
{
   Q_D(const QDeclarativeTextInput);
   //###const cast isn't good, but needed for property system?
   return const_cast<QValidator *>(d->control->validator());
}

void QDeclarativeTextInput::setValidator(QValidator *v)
{
   Q_D(QDeclarativeTextInput);
   if (d->control->validator() == v) {
      return;
   }

   d->control->setValidator(v);
   if (!d->control->hasAcceptableInput()) {
      d->oldValidity = false;
      emit acceptableInputChanged();
   }

   emit validatorChanged();
}
#endif // QT_NO_VALIDATOR

/*!
    \qmlproperty string TextInput::inputMask

    Allows you to set an input mask on the TextInput, restricting the allowable
    text inputs. See QLineEdit::inputMask for further details, as the exact
    same mask strings are used by TextInput.

    \sa acceptableInput, validator
*/
QString QDeclarativeTextInput::inputMask() const
{
   Q_D(const QDeclarativeTextInput);
   return d->control->inputMask();
}

void QDeclarativeTextInput::setInputMask(const QString &im)
{
   Q_D(QDeclarativeTextInput);
   if (d->control->inputMask() == im) {
      return;
   }

   d->control->setInputMask(im);
   emit inputMaskChanged(d->control->inputMask());
}

/*!
    \qmlproperty bool TextInput::acceptableInput

    This property is always true unless a validator or input mask has been set.
    If a validator or input mask has been set, this property will only be true
    if the current text is acceptable to the validator or input mask as a final
    string (not as an intermediate string).
*/
bool QDeclarativeTextInput::hasAcceptableInput() const
{
   Q_D(const QDeclarativeTextInput);
   return d->control->hasAcceptableInput();
}

/*!
    \qmlsignal TextInput::onAccepted()

    This handler is called when the Return or Enter key is pressed.
    Note that if there is a \l validator or \l inputMask set on the text
    input, the handler will only be emitted if the input is in an acceptable
    state.
*/

void QDeclarativeTextInputPrivate::updateInputMethodHints()
{
   Q_Q(QDeclarativeTextInput);
   Qt::InputMethodHints hints = inputMethodHints;
   uint echo = control->echoMode();
   if (echo == QDeclarativeTextInput::Password || echo == QDeclarativeTextInput::NoEcho) {
      hints |= Qt::ImhHiddenText;
   } else if (echo == QDeclarativeTextInput::PasswordEchoOnEdit) {
      hints &= ~Qt::ImhHiddenText;
   }
   if (echo != QDeclarativeTextInput::Normal) {
      hints |= (Qt::ImhNoAutoUppercase | Qt::ImhNoPredictiveText);
   }
   q->setInputMethodHints(hints);
}

/*!
    \qmlproperty enumeration TextInput::echoMode

    Specifies how the text should be displayed in the TextInput.
    \list
    \o TextInput.Normal - Displays the text as it is. (Default)
    \o TextInput.Password - Displays asterixes instead of characters.
    \o TextInput.NoEcho - Displays nothing.
    \o TextInput.PasswordEchoOnEdit - Displays characters as they are entered
    while editing, otherwise displays asterisks.
    \endlist
*/
QDeclarativeTextInput::EchoMode QDeclarativeTextInput::echoMode() const
{
   Q_D(const QDeclarativeTextInput);
   return (QDeclarativeTextInput::EchoMode)d->control->echoMode();
}

void QDeclarativeTextInput::setEchoMode(QDeclarativeTextInput::EchoMode echo)
{
   Q_D(QDeclarativeTextInput);
   if (echoMode() == echo) {
      return;
   }
   d->control->setEchoMode((uint)echo);
   d->updateInputMethodHints();
   q_textChanged();
   emit echoModeChanged(echoMode());
}

Qt::InputMethodHints QDeclarativeTextInput::imHints() const
{
   Q_D(const QDeclarativeTextInput);
   return d->inputMethodHints;
}

void QDeclarativeTextInput::setIMHints(Qt::InputMethodHints hints)
{
   Q_D(QDeclarativeTextInput);
   if (d->inputMethodHints == hints) {
      return;
   }
   d->inputMethodHints = hints;
   d->updateInputMethodHints();
}

/*!
    \qmlproperty Component TextInput::cursorDelegate
    The delegate for the cursor in the TextInput.

    If you set a cursorDelegate for a TextInput, this delegate will be used for
    drawing the cursor instead of the standard cursor. An instance of the
    delegate will be created and managed by the TextInput when a cursor is
    needed, and the x property of delegate instance will be set so as
    to be one pixel before the top left of the current character.

    Note that the root item of the delegate component must be a QDeclarativeItem or
    QDeclarativeItem derived item.
*/
QDeclarativeComponent *QDeclarativeTextInput::cursorDelegate() const
{
   Q_D(const QDeclarativeTextInput);
   return d->cursorComponent;
}

void QDeclarativeTextInput::setCursorDelegate(QDeclarativeComponent *c)
{
   Q_D(QDeclarativeTextInput);
   if (d->cursorComponent == c) {
      return;
   }

   d->cursorComponent = c;
   if (!c) {
      //note that the components are owned by something else
      delete d->cursorItem;
   } else {
      d->startCreatingCursor();
   }

   emit cursorDelegateChanged();
}

void QDeclarativeTextInputPrivate::startCreatingCursor()
{
   Q_Q(QDeclarativeTextInput);
   if (cursorComponent->isReady()) {
      q->createCursor();
   } else if (cursorComponent->isLoading()) {
      q->connect(cursorComponent, SIGNAL(statusChanged(int)),
                 q, SLOT(createCursor()));
   } else {//isError
      qmlInfo(q, cursorComponent->errors()) << QDeclarativeTextInput::tr("Could not load cursor delegate");
   }
}

void QDeclarativeTextInput::createCursor()
{
   Q_D(QDeclarativeTextInput);
   if (d->cursorComponent->isError()) {
      qmlInfo(this, d->cursorComponent->errors()) << tr("Could not load cursor delegate");
      return;
   }

   if (!d->cursorComponent->isReady()) {
      return;
   }

   if (d->cursorItem) {
      delete d->cursorItem;
   }
   d->cursorItem = qobject_cast<QDeclarativeItem *>(d->cursorComponent->create());
   if (!d->cursorItem) {
      qmlInfo(this, d->cursorComponent->errors()) << tr("Could not instantiate cursor delegate");
      return;
   }

   QDeclarative_setParent_noEvent(d->cursorItem, this);
   d->cursorItem->setParentItem(this);
   d->cursorItem->setX(d->control->cursorToX());
   d->cursorItem->setHeight(d->control->height() -
                            1); // -1 to counter QLineControl's +1 which is not consistent with Text.
}

/*!
    \qmlmethod rect TextInput::positionToRectangle(int pos)

    This function takes a character position and returns the rectangle that the
    cursor would occupy, if it was placed at that character position.

    This is similar to setting the cursorPosition, and then querying the cursor
    rectangle, but the cursorPosition is not changed.
*/
QRectF QDeclarativeTextInput::positionToRectangle(int pos) const
{
   Q_D(const QDeclarativeTextInput);
   if (pos > d->control->cursorPosition()) {
      pos += d->control->preeditAreaText().length();
   }
   return QRectF(d->control->cursorToX(pos) - d->hscroll,
                 0.0,
                 d->control->cursorWidth(),
                 cursorRectangle().height());
}

int QDeclarativeTextInput::positionAt(int x) const
{
   return positionAt(x, CursorBetweenCharacters);
}

/*!
    \qmlmethod int TextInput::positionAt(int x, CursorPosition position = CursorBetweenCharacters)
    \since QtQuick 1.1

    This function returns the character position at
    x pixels from the left of the textInput. Position 0 is before the
    first character, position 1 is after the first character but before the second,
    and so on until position text.length, which is after all characters.

    This means that for all x values before the first character this function returns 0,
    and for all x values after the last character this function returns text.length.

    The cursor position type specifies how the cursor position should be resolved.

    \list
    \o TextInput.CursorBetweenCharacters - Returns the position between characters that is nearest x.
    \o TextInput.CursorOnCharacter - Returns the position before the character that is nearest x.
    \endlist
*/
int QDeclarativeTextInput::positionAt(int x, CursorPosition position) const
{
   Q_D(const QDeclarativeTextInput);
   int pos = d->control->xToPos(x + d->hscroll, QTextLine::CursorPosition(position));
   const int cursor = d->control->cursor();
   if (pos > cursor) {
      const int preeditLength = d->control->preeditAreaText().length();
      pos = pos > cursor + preeditLength
            ? pos - preeditLength
            : cursor;
   }
   return pos;
}

void QDeclarativeTextInputPrivate::focusChanged(bool hasFocus)
{
   Q_Q(QDeclarativeTextInput);
   focused = hasFocus;
   q->setCursorVisible(hasFocus && scene && scene->hasFocus());
   if (!hasFocus && control->passwordEchoEditing()) {
      control->updatePasswordEchoEditing(false);   //QLineControl sets it on key events, but doesn't deal with focus events
   }
   if (!hasFocus) {
      control->deselect();
   }
   QDeclarativeItemPrivate::focusChanged(hasFocus);
}

void QDeclarativeTextInput::keyPressEvent(QKeyEvent *ev)
{
   Q_D(QDeclarativeTextInput);
   keyPressPreHandler(ev);
   if (ev->isAccepted()) {
      return;
   }

   // Don't allow MacOSX up/down support, and we don't allow a completer.
   bool ignore = (ev->key() == Qt::Key_Up || ev->key() == Qt::Key_Down) && ev->modifiers() == Qt::NoModifier;
   if (!ignore && (d->lastSelectionStart == d->lastSelectionEnd) && (ev->key() == Qt::Key_Right ||
         ev->key() == Qt::Key_Left)) {
      // Ignore when moving off the end unless there is a selection,
      // because then moving will do something (deselect).
      int cursorPosition = d->control->cursor();
      if (cursorPosition == 0) {
         ignore = ev->key() == (d->control->layoutDirection() == Qt::LeftToRight ? Qt::Key_Left : Qt::Key_Right);
      }
      if (!ignore && cursorPosition == d->control->text().length()) {
         ignore = ev->key() == (d->control->layoutDirection() == Qt::LeftToRight ? Qt::Key_Right : Qt::Key_Left);
      }
   }
   if (ignore) {
      ev->ignore();
   } else {
      d->control->processKeyEvent(ev);
   }
   if (!ev->isAccepted()) {
      QDeclarativePaintedItem::keyPressEvent(ev);
   }
}

void QDeclarativeTextInput::inputMethodEvent(QInputMethodEvent *ev)
{
   Q_D(QDeclarativeTextInput);
   ev->ignore();
   const bool wasComposing = d->control->preeditAreaText().length() > 0;
   inputMethodPreHandler(ev);
   if (!ev->isAccepted()) {
      if (d->control->isReadOnly()) {
         ev->ignore();
      } else {
         d->control->processInputMethodEvent(ev);
      }
   }
   if (!ev->isAccepted()) {
      QDeclarativePaintedItem::inputMethodEvent(ev);
   }

   if (wasComposing != (d->control->preeditAreaText().length() > 0)) {
      emit inputMethodComposingChanged();
   }
}

/*!
\overload
Handles the given mouse \a event.
*/
void QDeclarativeTextInput::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativeTextInput);
   if (d->sendMouseEventToInputContext(event, QEvent::MouseButtonDblClick)) {
      return;
   }
   if (d->selectByMouse) {
      int cursor = d->xToPos(event->pos().x());
      d->control->selectWordAtPos(cursor);
      event->setAccepted(true);
   } else {
      QDeclarativePaintedItem::mouseDoubleClickEvent(event);
   }
}

void QDeclarativeTextInput::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativeTextInput);
   if (d->sendMouseEventToInputContext(event, QEvent::MouseButtonPress)) {
      return;
   }
   if (d->focusOnPress) {
      bool hadActiveFocus = hasActiveFocus();
      forceActiveFocus();
      if (d->showInputPanelOnFocus) {
         if (hasActiveFocus() && hadActiveFocus && !isReadOnly()) {
            // re-open input panel on press if already focused
            openSoftwareInputPanel();
         }
      } else { // show input panel on click
         if (hasActiveFocus() && !hadActiveFocus) {
            d->clickCausedFocus = true;
         }
      }
   }
   if (d->selectByMouse) {
      setKeepMouseGrab(false);
      d->selectPressed = true;
      d->pressPos = event->pos();
   }
   bool mark = (event->modifiers() & Qt::ShiftModifier) && d->selectByMouse;
   int cursor = d->xToPos(event->pos().x());
   d->control->moveCursor(cursor, mark);
   event->setAccepted(true);
}

void QDeclarativeTextInput::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativeTextInput);
   if (d->sendMouseEventToInputContext(event, QEvent::MouseMove)) {
      return;
   }
   if (d->selectPressed) {
      if (qAbs(int(event->pos().x() - d->pressPos.x())) > QApplication::startDragDistance()) {
         setKeepMouseGrab(true);
      }
      moveCursorSelection(d->xToPos(event->pos().x()), d->mouseSelectionMode);
      event->setAccepted(true);
   } else {
      QDeclarativePaintedItem::mouseMoveEvent(event);
   }
}

/*!
\overload
Handles the given mouse \a event.
*/
void QDeclarativeTextInput::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativeTextInput);
   if (d->sendMouseEventToInputContext(event, QEvent::MouseButtonRelease)) {
      return;
   }
   if (d->selectPressed) {
      d->selectPressed = false;
      setKeepMouseGrab(false);
   }
   if (!d->showInputPanelOnFocus) { // input panel on click
      if (d->focusOnPress && !isReadOnly() && boundingRect().contains(event->pos())) {
         if (QGraphicsView *view = qobject_cast<QGraphicsView *>(qApp->focusWidget())) {
            if (view->scene() && view->scene() == scene()) {
               qt_widget_private(view)->handleSoftwareInputPanel(event->button(), d->clickCausedFocus);
            }
         }
      }
   }
   d->clickCausedFocus = false;
   d->control->processEvent(event);
   if (!event->isAccepted()) {
      QDeclarativePaintedItem::mouseReleaseEvent(event);
   }
}

bool QDeclarativeTextInputPrivate::sendMouseEventToInputContext(
   QGraphicsSceneMouseEvent *event, QEvent::Type eventType)
{
#if !defined QT_NO_IM
   Q_Q(QDeclarativeTextInput);

   QWidget *widget = event->widget();
   // event->widget() is null, if this is delayed event from QDeclarativeFlickable.
   if (!widget && qApp) {
      QGraphicsView *view = qobject_cast<QGraphicsView *>(qApp->focusWidget());
      if (view && view->scene() && view->scene() == q->scene()) {
         widget = view->viewport();
      }
   }

   if (widget && control->composeMode()) {
      int tmp_cursor = xToPos(event->pos().x());
      int mousePos = tmp_cursor - control->cursor();
      if (mousePos < 0 || mousePos > control->preeditAreaText().length()) {
         mousePos = -1;
         // don't send move events outside the preedit area
         if (eventType == QEvent::MouseMove) {
            return true;
         }
      }

      QInputContext *qic = widget->inputContext();
      if (qic) {
         QMouseEvent mouseEvent(
            eventType,
            widget->mapFromGlobal(event->screenPos()),
            event->screenPos(),
            event->button(),
            event->buttons(),
            event->modifiers());
         // may be causing reset() in some input methods
         qic->mouseHandler(mousePos, &mouseEvent);
         event->setAccepted(mouseEvent.isAccepted());
      }
      if (!control->preeditAreaText().isEmpty()) {
         return true;
      }
   }
#else
   Q_UNUSED(event);
   Q_UNUSED(eventType)
#endif

   return false;
}

bool QDeclarativeTextInput::sceneEvent(QEvent *event)
{
   Q_D(QDeclarativeTextInput);
   bool rv = QDeclarativeItem::sceneEvent(event);
   if (event->type() == QEvent::UngrabMouse) {
      d->selectPressed = false;
      setKeepMouseGrab(false);
   }
   return rv;
}

bool QDeclarativeTextInput::event(QEvent *ev)
{
   Q_D(QDeclarativeTextInput);
   //Anything we don't deal with ourselves, pass to the control
   bool handled = false;
   switch (ev->type()) {
      case QEvent::KeyPress:
      case QEvent::KeyRelease://###Should the control be doing anything with release?
      case QEvent::InputMethod:
      case QEvent::GraphicsSceneMousePress:
      case QEvent::GraphicsSceneMouseMove:
      case QEvent::GraphicsSceneMouseRelease:
      case QEvent::GraphicsSceneMouseDoubleClick:
         break;
      default:
         handled = d->control->processEvent(ev);
   }
   if (!handled) {
      handled = QDeclarativePaintedItem::event(ev);
   }
   return handled;
}

void QDeclarativeTextInput::geometryChanged(const QRectF &newGeometry,
      const QRectF &oldGeometry)
{
   if (newGeometry.width() != oldGeometry.width()) {
      updateSize();
      updateCursorRectangle();
   }
   QDeclarativePaintedItem::geometryChanged(newGeometry, oldGeometry);
}

int QDeclarativeTextInputPrivate::calculateTextWidth()
{
   return qRound(control->naturalTextWidth());
}

void QDeclarativeTextInputPrivate::updateHorizontalScroll()
{
   Q_Q(QDeclarativeTextInput);
   const int preeditLength = control->preeditAreaText().length();
   int cix = qRound(control->cursorToX(control->cursor() + preeditLength));
   QRect br(q->boundingRect().toRect());
   int widthUsed = calculateTextWidth();

   QDeclarativeTextInput::HAlignment effectiveHAlign = q->effectiveHAlign();
   if (autoScroll) {
      if (widthUsed <=  br.width()) {
         // text fits in br; use hscroll for alignment
         switch (effectiveHAlign & ~(Qt::AlignAbsolute | Qt::AlignVertical_Mask)) {
            case Qt::AlignRight:
               hscroll = widthUsed - br.width() - 1;
               break;
            case Qt::AlignHCenter:
               hscroll = (widthUsed - br.width()) / 2;
               break;
            default:
               // Left
               hscroll = 0;
               break;
         }
      } else if (cix - hscroll >= br.width()) {
         // text doesn't fit, cursor is to the right of br (scroll right)
         hscroll = cix - br.width() + 1;
      } else if (cix - hscroll < 0 && hscroll < widthUsed) {
         // text doesn't fit, cursor is to the left of br (scroll left)
         hscroll = cix;
      } else if (widthUsed - hscroll < br.width()) {
         // text doesn't fit, text document is to the left of br; align
         // right
         hscroll = widthUsed - br.width() + 1;
      }
      if (preeditLength > 0) {
         // check to ensure long pre-edit text doesn't push the cursor
         // off to the left
         cix = qRound(control->cursorToX(
                         control->cursor() + qMax(0, control->preeditCursor() - 1)));
         if (cix < hscroll) {
            hscroll = cix;
         }
      }
   } else {
      switch (effectiveHAlign) {
         case QDeclarativeTextInput::AlignRight:
            hscroll = q->width() - widthUsed;
            break;
         case QDeclarativeTextInput::AlignHCenter:
            hscroll = (q->width() - widthUsed) / 2;
            break;
         default:
            // Left
            hscroll = 0;
            break;
      }
   }
}

void QDeclarativeTextInput::drawContents(QPainter *p, const QRect &r)
{
   Q_D(QDeclarativeTextInput);
   p->setRenderHint(QPainter::TextAntialiasing, true);
   p->save();
   p->setPen(QPen(d->color));
   int flags = QLineControl::DrawText;
   if (!isReadOnly() && d->cursorVisible && !d->cursorItem) {
      flags |= QLineControl::DrawCursor;
   }
   if (d->control->hasSelectedText()) {
      flags |= QLineControl::DrawSelections;
   }
   QPoint offset = QPoint(0, 0);
   QFontMetrics fm = QFontMetrics(d->font);
   QRect br(boundingRect().toRect());
   if (d->autoScroll) {
      // the y offset is there to keep the baseline constant in case we have script changes in the text.
      offset = br.topLeft() - QPoint(d->hscroll, d->control->ascent() - fm.ascent());
   } else {
      offset = QPoint(d->hscroll, 0);
   }
   d->control->draw(p, offset, r, flags);
   p->restore();
}

/*!
\overload
Returns the value of the given \a property.
*/
QVariant QDeclarativeTextInput::inputMethodQuery(Qt::InputMethodQuery property) const
{
   Q_D(const QDeclarativeTextInput);
   switch (property) {
      case Qt::ImMicroFocus:
         return cursorRectangle();
      case Qt::ImFont:
         return font();
      case Qt::ImCursorPosition:
         return QVariant(d->control->cursor());
      case Qt::ImSurroundingText:
         if (d->control->echoMode() == PasswordEchoOnEdit && !d->control->passwordEchoEditing()) {
            return QVariant(displayText());
         } else {
            return QVariant(text());
         }
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

/*!
    \qmlmethod void TextInput::deselect()
    \since QtQuick 1.1

    Removes active text selection.
*/
void QDeclarativeTextInput::deselect()
{
   Q_D(QDeclarativeTextInput);
   d->control->deselect();
}

/*!
    \qmlmethod void TextInput::selectAll()

    Causes all text to be selected.
*/
void QDeclarativeTextInput::selectAll()
{
   Q_D(QDeclarativeTextInput);
   d->control->setSelection(0, d->control->text().length());
}

/*!
    \qmlmethod void TextInput::isRightToLeft(int start, int end)

    Returns true if the natural reading direction of the editor text
    found between positions \a start and \a end is right to left.
*/
bool QDeclarativeTextInput::isRightToLeft(int start, int end)
{
   Q_D(QDeclarativeTextInput);
   if (start > end) {
      qmlInfo(this) << "isRightToLeft(start, end) called with the end property being smaller than the start.";
      return false;
   } else {
      return d->control->text().mid(start, end - start).isRightToLeft();
   }
}

#ifndef QT_NO_CLIPBOARD
/*!
    \qmlmethod TextInput::cut()

    Moves the currently selected text to the system clipboard.
*/
void QDeclarativeTextInput::cut()
{
   Q_D(QDeclarativeTextInput);
   d->control->copy();
   d->control->del();
}

/*!
    \qmlmethod TextInput::copy()

    Copies the currently selected text to the system clipboard.
*/
void QDeclarativeTextInput::copy()
{
   Q_D(QDeclarativeTextInput);
   d->control->copy();
}

/*!
    \qmlmethod TextInput::paste()

    Replaces the currently selected text by the contents of the system clipboard.
*/
void QDeclarativeTextInput::paste()
{
   Q_D(QDeclarativeTextInput);
   if (!d->control->isReadOnly()) {
      d->control->paste();
   }
}
#endif // QT_NO_CLIPBOARD

/*!
    \qmlmethod void TextInput::selectWord()

    Causes the word closest to the current cursor position to be selected.
*/
void QDeclarativeTextInput::selectWord()
{
   Q_D(QDeclarativeTextInput);
   d->control->selectWordAtPos(d->control->cursor());
}

/*!
    \qmlproperty bool TextInput::smooth

    This property holds whether the text is smoothly scaled or transformed.

    Smooth filtering gives better visual quality, but is slower.  If
    the item is displayed at its natural size, this property has no visual or
    performance effect.

    \note Generally scaling artifacts are only visible if the item is stationary on
    the screen.  A common pattern when animating an item is to disable smooth
    filtering at the beginning of the animation and reenable it at the conclusion.
*/

/*!
   \qmlproperty string TextInput::passwordCharacter

   This is the character displayed when echoMode is set to Password or
   PasswordEchoOnEdit. By default it is an asterisk.

   If this property is set to a string with more than one character,
   the first character is used. If the string is empty, the value
   is ignored and the property is not set.
*/
QString QDeclarativeTextInput::passwordCharacter() const
{
   Q_D(const QDeclarativeTextInput);
   return QString(d->control->passwordCharacter());
}

void QDeclarativeTextInput::setPasswordCharacter(const QString &str)
{
   Q_D(QDeclarativeTextInput);
   if (str.length() < 1) {
      return;
   }
   d->control->setPasswordCharacter(str.constData()[0]);
   EchoMode echoMode_ = echoMode();
   if (echoMode_ == Password || echoMode_ == PasswordEchoOnEdit) {
      updateSize();
   }
   emit passwordCharacterChanged();
}

/*!
   \qmlproperty string TextInput::displayText

   This is the text displayed in the TextInput.

   If \l echoMode is set to TextInput::Normal, this holds the
   same value as the TextInput::text property. Otherwise,
   this property holds the text visible to the user, while
   the \l text property holds the actual entered text.
*/
QString QDeclarativeTextInput::displayText() const
{
   Q_D(const QDeclarativeTextInput);
   return d->control->displayText();
}

/*!
    \qmlproperty bool TextInput::selectByMouse

    Defaults to false.

    If true, the user can use the mouse to select text in some
    platform-specific way. Note that for some platforms this may
    not be an appropriate interaction (eg. may conflict with how
    the text needs to behave inside a Flickable.
*/
bool QDeclarativeTextInput::selectByMouse() const
{
   Q_D(const QDeclarativeTextInput);
   return d->selectByMouse;
}

void QDeclarativeTextInput::setSelectByMouse(bool on)
{
   Q_D(QDeclarativeTextInput);
   if (d->selectByMouse != on) {
      d->selectByMouse = on;
      emit selectByMouseChanged(on);
   }
}

/*!
    \qmlproperty enum TextInput::mouseSelectionMode
    \since QtQuick 1.1

    Specifies how text should be selected using a mouse.

    \list
    \o TextInput.SelectCharacters - The selection is updated with individual characters. (Default)
    \o TextInput.SelectWords - The selection is updated with whole words.
    \endlist

    This property only applies when \l selectByMouse is true.
*/

QDeclarativeTextInput::SelectionMode QDeclarativeTextInput::mouseSelectionMode() const
{
   Q_D(const QDeclarativeTextInput);
   return d->mouseSelectionMode;
}

void QDeclarativeTextInput::setMouseSelectionMode(SelectionMode mode)
{
   Q_D(QDeclarativeTextInput);
   if (d->mouseSelectionMode != mode) {
      d->mouseSelectionMode = mode;
      emit mouseSelectionModeChanged(mode);
   }
}

/*!
    \qmlproperty bool TextInput::canPaste
    \since QtQuick 1.1

    Returns true if the TextInput is writable and the content of the clipboard is
    suitable for pasting into the TextEdit.
*/
bool QDeclarativeTextInput::canPaste() const
{
   Q_D(const QDeclarativeTextInput);
   return d->canPaste;
}

void QDeclarativeTextInput::moveCursorSelection(int position)
{
   Q_D(QDeclarativeTextInput);
   d->control->moveCursor(position, true);
}

/*!
    \qmlmethod void TextInput::moveCursorSelection(int position, SelectionMode mode = TextInput.SelectCharacters)
    \since QtQuick 1.1

    Moves the cursor to \a position and updates the selection according to the optional \a mode
    parameter.  (To only move the cursor, set the \l cursorPosition property.)

    When this method is called it additionally sets either the
    selectionStart or the selectionEnd (whichever was at the previous cursor position)
    to the specified position. This allows you to easily extend and contract the selected
    text range.

    The selection mode specifies whether the selection is updated on a per character or a per word
    basis.  If not specified the selection mode will default to TextInput.SelectCharacters.

    \list
    \o TextEdit.SelectCharacters - Sets either the selectionStart or selectionEnd (whichever was at
    the previous cursor position) to the specified position.
    \o TextEdit.SelectWords - Sets the selectionStart and selectionEnd to include all
    words between the specified postion and the previous cursor position.  Words partially in the
    range are included.
    \endlist

    For example, take this sequence of calls:

    \code
        cursorPosition = 5
        moveCursorSelection(9, TextInput.SelectCharacters)
        moveCursorSelection(7, TextInput.SelectCharacters)
    \endcode

    This moves the cursor to position 5, extend the selection end from 5 to 9
    and then retract the selection end from 9 to 7, leaving the text from position 5 to 7
    selected (the 6th and 7th characters).

    The same sequence with TextInput.SelectWords will extend the selection start to a word boundary
    before or on position 5 and extend the selection end to a word boundary on or past position 9.
*/
void QDeclarativeTextInput::moveCursorSelection(int pos, SelectionMode mode)
{
   Q_D(QDeclarativeTextInput);

   if (mode == SelectCharacters) {
      d->control->moveCursor(pos, true);
   } else if (pos != d->control->cursor()) {
      const int cursor = d->control->cursor();
      int anchor;
      if (!d->control->hasSelectedText()) {
         anchor = d->control->cursor();
      } else if (d->control->selectionStart() == d->control->cursor()) {
         anchor = d->control->selectionEnd();
      } else {
         anchor = d->control->selectionStart();
      }

      if (anchor < pos || (anchor == pos && cursor < pos)) {
         const QString text = d->control->text();
         QTextBoundaryFinder finder(QTextBoundaryFinder::Word, text);
         finder.setPosition(anchor);

         const QTextBoundaryFinder::BoundaryReasons reasons = finder.boundaryReasons();
         if (anchor < text.length() && (!(reasons & QTextBoundaryFinder::StartWord)
                                        || ((reasons & QTextBoundaryFinder::EndWord) && anchor > cursor))) {
            finder.toPreviousBoundary();
         }
         anchor = finder.position() != -1 ? finder.position() : 0;

         finder.setPosition(pos);
         if (pos > 0 && !finder.boundaryReasons()) {
            finder.toNextBoundary();
         }
         const int cursor = finder.position() != -1 ? finder.position() : text.length();

         d->control->setSelection(anchor, cursor - anchor);
      } else if (anchor > pos || (anchor == pos && cursor > pos)) {
         const QString text = d->control->text();
         QTextBoundaryFinder finder(QTextBoundaryFinder::Word, text);
         finder.setPosition(anchor);

         const QTextBoundaryFinder::BoundaryReasons reasons = finder.boundaryReasons();
         if (anchor > 0 && (!(reasons & QTextBoundaryFinder::EndWord)
                            || ((reasons & QTextBoundaryFinder::StartWord) && anchor < cursor))) {
            finder.toNextBoundary();
         }
         anchor = finder.position() != -1 ? finder.position() : text.length();

         finder.setPosition(pos);
         if (pos < text.length() && !finder.boundaryReasons()) {
            finder.toPreviousBoundary();
         }
         const int cursor = finder.position() != -1 ? finder.position() : 0;

         d->control->setSelection(anchor, cursor - anchor);
      }
   }
}

/*!
    \qmlmethod void TextInput::openSoftwareInputPanel()

    Opens software input panels like virtual keyboards for typing, useful for
    customizing when you want the input keyboard to be shown and hidden in
    your application.

    By default the opening of input panels follows the platform style. On Symbian^1 and
    Symbian^3 -based devices the panels are opened by clicking TextInput. On other platforms
    the panels are automatically opened when TextInput element gains active focus. Input panels are
    always closed if no editor has active focus.

  . You can disable the automatic behavior by setting the property \c activeFocusOnPress to false
    and use functions openSoftwareInputPanel() and closeSoftwareInputPanel() to implement
    the behavior you want.

    Only relevant on platforms, which provide virtual keyboards.

    \qml
        import QtQuick 1.0
        TextInput {
            id: textInput
            text: "Hello world!"
            activeFocusOnPress: false
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (!textInput.activeFocus) {
                        textInput.forceActiveFocus()
                        textInput.openSoftwareInputPanel();
                    } else {
                        textInput.focus = false;
                    }
                }
                onPressAndHold: textInput.closeSoftwareInputPanel();
            }
        }
    \endqml
*/
void QDeclarativeTextInput::openSoftwareInputPanel()
{
   QEvent event(QEvent::RequestSoftwareInputPanel);
   if (qApp) {
      if (QGraphicsView *view = qobject_cast<QGraphicsView *>(qApp->focusWidget())) {
         if (view->scene() && view->scene() == scene()) {
            QApplication::sendEvent(view, &event);
         }
      }
   }
}

/*!
    \qmlmethod void TextInput::closeSoftwareInputPanel()

    Closes a software input panel like a virtual keyboard shown on the screen, useful
    for customizing when you want the input keyboard to be shown and hidden in
    your application.

    By default the opening of input panels follows the platform style. On Symbian^1 and
    Symbian^3 -based devices the panels are opened by clicking TextInput. On other platforms
    the panels are automatically opened when TextInput element gains active focus. Input panels are
    always closed if no editor has active focus.

  . You can disable the automatic behavior by setting the property \c activeFocusOnPress to false
    and use functions openSoftwareInputPanel() and closeSoftwareInputPanel() to implement
    the behavior you want.

    Only relevant on platforms, which provide virtual keyboards.

    \qml
        import QtQuick 1.0
        TextInput {
            id: textInput
            text: "Hello world!"
            activeFocusOnPress: false
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (!textInput.activeFocus) {
                        textInput.forceActiveFocus();
                        textInput.openSoftwareInputPanel();
                    } else {
                        textInput.focus = false;
                    }
                }
                onPressAndHold: textInput.closeSoftwareInputPanel();
            }
        }
    \endqml
*/
void QDeclarativeTextInput::closeSoftwareInputPanel()
{
   QEvent event(QEvent::CloseSoftwareInputPanel);
   if (qApp) {
      QEvent event(QEvent::CloseSoftwareInputPanel);
      if (QGraphicsView *view = qobject_cast<QGraphicsView *>(qApp->focusWidget())) {
         if (view->scene() && view->scene() == scene()) {
            QApplication::sendEvent(view, &event);
         }
      }
   }
}

void QDeclarativeTextInput::focusInEvent(QFocusEvent *event)
{
   Q_D(const QDeclarativeTextInput);
   if (d->showInputPanelOnFocus) {
      if (d->focusOnPress && !isReadOnly()) {
         openSoftwareInputPanel();
      }
   }
   QDeclarativePaintedItem::focusInEvent(event);
}

/*!
    \qmlproperty bool TextInput::inputMethodComposing

    \since QtQuick 1.1

    This property holds whether the TextInput has partial text input from an
    input method.

    While it is composing an input method may rely on mouse or key events from
    the TextInput to edit or commit the partial text.  This property can be
    used to determine when to disable events handlers that may interfere with
    the correct operation of an input method.
*/
bool QDeclarativeTextInput::isInputMethodComposing() const
{
   Q_D(const QDeclarativeTextInput);
   return d->control->preeditAreaText().length() > 0;
}

void QDeclarativeTextInputPrivate::init()
{
   Q_Q(QDeclarativeTextInput);
   control->setParent(q);
   control->setCursorWidth(1);
   control->setPasswordCharacter(QLatin1Char('*'));
   q->setSmooth(smooth);
   q->setAcceptedMouseButtons(Qt::LeftButton);
   q->setFlag(QGraphicsItem::ItemHasNoContents, false);
   q->setFlag(QGraphicsItem::ItemAcceptsInputMethod);
   q->connect(control, SIGNAL(cursorPositionChanged(int, int)),
              q, SLOT(cursorPosChanged()));
   q->connect(control, SIGNAL(selectionChanged()),
              q, SLOT(selectionChanged()));
   q->connect(control, SIGNAL(textChanged(QString)),
              q, SLOT(q_textChanged()));
   q->connect(control, SIGNAL(accepted()),
              q, SIGNAL(accepted()));
   q->connect(control, SIGNAL(updateNeeded(QRect)),
              q, SLOT(updateRect(QRect)));
#ifndef QT_NO_CLIPBOARD
   q->connect(q, SIGNAL(readOnlyChanged(bool)),
              q, SLOT(q_canPasteChanged()));
   q->connect(QApplication::clipboard(), SIGNAL(dataChanged()),
              q, SLOT(q_canPasteChanged()));
   canPaste = !control->isReadOnly() && QApplication::clipboard()->text().length() != 0;
#endif // QT_NO_CLIPBOARD
   q->connect(control, SIGNAL(updateMicroFocus()),
              q, SLOT(updateCursorRectangle()));
   q->connect(control, SIGNAL(displayTextChanged(QString)),
              q, SLOT(updateRect()));
   q->updateSize();
   oldValidity = control->hasAcceptableInput();
   lastSelectionStart = 0;
   lastSelectionEnd = 0;
   QPalette p = control->palette();
   selectedTextColor = p.color(QPalette::HighlightedText);
   selectionColor = p.color(QPalette::Highlight);
   determineHorizontalAlignment();
}

void QDeclarativeTextInput::cursorPosChanged()
{
   Q_D(QDeclarativeTextInput);
   updateCursorRectangle();
   emit cursorPositionChanged();
   d->control->resetCursorBlinkTimer();

   if (!d->control->hasSelectedText()) {
      if (d->lastSelectionStart != d->control->cursor()) {
         d->lastSelectionStart = d->control->cursor();
         emit selectionStartChanged();
      }
      if (d->lastSelectionEnd != d->control->cursor()) {
         d->lastSelectionEnd = d->control->cursor();
         emit selectionEndChanged();
      }
   }
}

void QDeclarativeTextInput::updateCursorRectangle()
{
   Q_D(QDeclarativeTextInput);
   d->determineHorizontalAlignment();
   d->updateHorizontalScroll();
   updateRect();//TODO: Only update rect between pos's
   updateMicroFocus();
   emit cursorRectangleChanged();
   if (d->cursorItem) {
      d->cursorItem->setX(d->control->cursorToX() - d->hscroll);
   }
}

void QDeclarativeTextInput::selectionChanged()
{
   Q_D(QDeclarativeTextInput);
   updateRect();//TODO: Only update rect in selection
   emit selectedTextChanged();

   if (d->lastSelectionStart != d->control->selectionStart()) {
      d->lastSelectionStart = d->control->selectionStart();
      if (d->lastSelectionStart == -1) {
         d->lastSelectionStart = d->control->cursor();
      }
      emit selectionStartChanged();
   }
   if (d->lastSelectionEnd != d->control->selectionEnd()) {
      d->lastSelectionEnd = d->control->selectionEnd();
      if (d->lastSelectionEnd == -1) {
         d->lastSelectionEnd = d->control->cursor();
      }
      emit selectionEndChanged();
   }
}

void QDeclarativeTextInput::q_textChanged()
{
   Q_D(QDeclarativeTextInput);
   emit textChanged();
   emit displayTextChanged();
   updateSize();
   d->determineHorizontalAlignment();
   d->updateHorizontalScroll();
   updateMicroFocus();
   if (hasAcceptableInput() != d->oldValidity) {
      d->oldValidity = hasAcceptableInput();
      emit acceptableInputChanged();
   }
}

void QDeclarativeTextInput::updateRect(const QRect &r)
{
   Q_D(QDeclarativeTextInput);
   if (r == QRect()) {
      clearCache();
   } else {
      dirtyCache(QRect(r.x() - d->hscroll, r.y(), r.width(), r.height()));
   }
   update();
}

QRectF QDeclarativeTextInput::boundingRect() const
{
   Q_D(const QDeclarativeTextInput);
   QRectF r = QDeclarativePaintedItem::boundingRect();

   int cursorWidth = d->cursorItem ? d->cursorItem->width() : d->control->cursorWidth();

   // Could include font max left/right bearings to either side of rectangle.

   r.setRight(r.right() + cursorWidth);
   return r;
}

void QDeclarativeTextInput::updateSize(bool needsRedraw)
{
   Q_D(QDeclarativeTextInput);
   int w = width();
   int h = height();
   setImplicitHeight(d->control->height() - 1); // -1 to counter QLineControl's +1 which is not consistent with Text.
   setImplicitWidth(d->calculateTextWidth());
   setContentsSize(QSize(width(), height()));//Repaints if changed
   if (w == width() && h == height() && needsRedraw) {
      clearCache();
      update();
   }
}

void QDeclarativeTextInput::q_canPasteChanged()
{
   Q_D(QDeclarativeTextInput);
   bool old = d->canPaste;
#ifndef QT_NO_CLIPBOARD
   d->canPaste = !d->control->isReadOnly() && QApplication::clipboard()->text().length() != 0;
#endif
   if (d->canPaste != old) {
      emit canPasteChanged();
   }
}

QT_END_NAMESPACE

#endif // QT_NO_LINEEDIT

