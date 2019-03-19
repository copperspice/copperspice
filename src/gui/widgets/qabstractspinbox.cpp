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

#include <qplatformdefs.h>
#include <qabstractspinbox_p.h>
#include <qdatetime_p.h>
#include <qlineedit_p.h>
#include <qabstractspinbox.h>

#ifndef QT_NO_SPINBOX

#include <qapplication.h>
#include <qclipboard.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>
#include <qevent.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qstylepainter.h>
#include <qdebug.h>

#ifndef QT_NO_ACCESSIBILITY
# include <qaccessible.h>
#endif

#if defined(Q_WS_X11)
#include <limits.h>
#endif

//#define QABSTRACTSPINBOX_QSBDEBUG
#ifdef QABSTRACTSPINBOX_QSBDEBUG
#  define QASBDEBUG qDebug
#else
#  define QASBDEBUG if (false) qDebug
#endif


QAbstractSpinBox::QAbstractSpinBox(QWidget *parent)
   : QWidget(*new QAbstractSpinBoxPrivate, parent, 0)
{
   Q_D(QAbstractSpinBox);
   d->init();
}

/*!
    \internal
*/
QAbstractSpinBox::QAbstractSpinBox(QAbstractSpinBoxPrivate &dd, QWidget *parent)
   : QWidget(dd, parent, 0)
{
   Q_D(QAbstractSpinBox);
   d->init();
}

/*!
    Called when the QAbstractSpinBox is destroyed.
*/

QAbstractSpinBox::~QAbstractSpinBox()
{
}


QAbstractSpinBox::ButtonSymbols QAbstractSpinBox::buttonSymbols() const
{
   Q_D(const QAbstractSpinBox);
   return d->buttonSymbols;
}

void QAbstractSpinBox::setButtonSymbols(ButtonSymbols symbols)
{
   Q_D(QAbstractSpinBox);

   if (d->buttonSymbols != symbols) {
      d->buttonSymbols = symbols;
      d->updateEditFieldGeometry();
      update();
   }
}

QString QAbstractSpinBox::text() const
{
   return lineEdit()->displayText();
}

QString QAbstractSpinBox::specialValueText() const
{
   Q_D(const QAbstractSpinBox);
   return d->specialValueText;
}

void QAbstractSpinBox::setSpecialValueText(const QString &specialValueText)
{
   Q_D(QAbstractSpinBox);

   d->specialValueText = specialValueText;
   d->cachedSizeHint = QSize(); // minimumSizeHint doesn't care about specialValueText
   d->clearCache();
   d->updateEdit();
}


bool QAbstractSpinBox::wrapping() const
{
   Q_D(const QAbstractSpinBox);
   return d->wrapping;
}

void QAbstractSpinBox::setWrapping(bool wrapping)
{
   Q_D(QAbstractSpinBox);
   d->wrapping = wrapping;
}

bool QAbstractSpinBox::isReadOnly() const
{
   Q_D(const QAbstractSpinBox);
   return d->readOnly;
}

void QAbstractSpinBox::setReadOnly(bool enable)
{
   Q_D(QAbstractSpinBox);
   d->readOnly = enable;
   d->edit->setReadOnly(enable);
   update();
}

bool QAbstractSpinBox::keyboardTracking() const
{
   Q_D(const QAbstractSpinBox);
   return d->keyboardTracking;
}

void QAbstractSpinBox::setKeyboardTracking(bool enable)
{
   Q_D(QAbstractSpinBox);
   d->keyboardTracking = enable;
}

bool QAbstractSpinBox::hasFrame() const
{
   Q_D(const QAbstractSpinBox);
   return d->frame;
}


void QAbstractSpinBox::setFrame(bool enable)
{
   Q_D(QAbstractSpinBox);
   d->frame = enable;
   update();
   d->updateEditFieldGeometry();
}

void QAbstractSpinBox::setAccelerated(bool accelerate)
{
   Q_D(QAbstractSpinBox);
   d->accelerate = accelerate;

}
bool QAbstractSpinBox::isAccelerated() const
{
   Q_D(const QAbstractSpinBox);
   return d->accelerate;
}

void QAbstractSpinBox::setCorrectionMode(CorrectionMode correctionMode)
{
   Q_D(QAbstractSpinBox);
   d->correctionMode = correctionMode;

}
QAbstractSpinBox::CorrectionMode QAbstractSpinBox::correctionMode() const
{
   Q_D(const QAbstractSpinBox);
   return d->correctionMode;
}


bool QAbstractSpinBox::hasAcceptableInput() const
{
   Q_D(const QAbstractSpinBox);
   return d->edit->hasAcceptableInput();
}

Qt::Alignment QAbstractSpinBox::alignment() const
{
   Q_D(const QAbstractSpinBox);

   return (Qt::Alignment)d->edit->alignment();
}

void QAbstractSpinBox::setAlignment(Qt::Alignment flag)
{
   Q_D(QAbstractSpinBox);

   d->edit->setAlignment(flag);
}

/*!
    Selects all the text in the spinbox except the prefix and suffix.
*/

void QAbstractSpinBox::selectAll()
{
   Q_D(QAbstractSpinBox);


   if (!d->specialValue()) {
      const int tmp = d->edit->displayText().size() - d->suffix.size();
      d->edit->setSelection(tmp, -(tmp - d->prefix.size()));
   } else {
      d->edit->selectAll();
   }
}

/*!
    Clears the lineedit of all text but prefix and suffix.
*/

void QAbstractSpinBox::clear()
{
   Q_D(QAbstractSpinBox);

   d->edit->setText(d->prefix + d->suffix);
   d->edit->setCursorPosition(d->prefix.size());
   d->cleared = true;
}

/*!
    Virtual function that determines whether stepping up and down is
    legal at any given time.

    The up arrow will be painted as disabled unless (stepEnabled() &
    StepUpEnabled) != 0.

    The default implementation will return (StepUpEnabled|
    StepDownEnabled) if wrapping is turned on. Else it will return
    StepDownEnabled if value is > minimum() or'ed with StepUpEnabled if
    value < maximum().

    If you subclass QAbstractSpinBox you will need to reimplement this function.

    \sa QSpinBox::minimum(), QSpinBox::maximum(), wrapping()
*/


QAbstractSpinBox::StepEnabled QAbstractSpinBox::stepEnabled() const
{
   Q_D(const QAbstractSpinBox);
   if (d->readOnly || d->type == QVariant::Invalid) {
      return StepNone;
   }
   if (d->wrapping) {
      return StepEnabled(StepUpEnabled | StepDownEnabled);
   }
   StepEnabled ret = StepNone;
   if (d->variantCompare(d->value, d->maximum) < 0) {
      ret |= StepUpEnabled;
   }
   if (d->variantCompare(d->value, d->minimum) > 0) {
      ret |= StepDownEnabled;
   }
   return ret;
}

/*!
   This virtual function is called by the QAbstractSpinBox to
   determine whether \a input is valid. The \a pos parameter indicates
   the position in the string. Reimplemented in the various
   subclasses.
*/

QValidator::State QAbstractSpinBox::validate(QString & /* input */, int & /* pos */) const
{
   return QValidator::Acceptable;
}

/*!
   This virtual function is called by the QAbstractSpinBox if the
   \a input is not validated to QValidator::Acceptable when Return is
   pressed or interpretText() is called. It will try to change the
   text so it is valid. Reimplemented in the various subclasses.
*/

void QAbstractSpinBox::fixup(QString & /* input */) const
{
}

/*!
  Steps up by one linestep
  Calling this slot is analogous to calling stepBy(1);
  \sa stepBy(), stepDown()
*/

void QAbstractSpinBox::stepUp()
{
   stepBy(1);
}

/*!
  Steps down by one linestep
  Calling this slot is analogous to calling stepBy(-1);
  \sa stepBy(), stepUp()
*/

void QAbstractSpinBox::stepDown()
{
   stepBy(-1);
}
/*!
    Virtual function that is called whenever the user triggers a step.
    The \a steps parameter indicates how many steps were taken, e.g.
    Pressing Qt::Key_Down will trigger a call to stepBy(-1),
    whereas pressing Qt::Key_Prior will trigger a call to
    stepBy(10).

    If you subclass QAbstractSpinBox you must reimplement this
    function. Note that this function is called even if the resulting
    value will be outside the bounds of minimum and maximum. It's this
    function's job to handle these situations.
*/

void QAbstractSpinBox::stepBy(int steps)
{
   Q_D(QAbstractSpinBox);

   const QVariant old = d->value;
   QString tmp = d->edit->displayText();
   int cursorPos = d->edit->cursorPosition();
   bool dontstep = false;
   EmitPolicy e = EmitIfChanged;
   if (d->pendingEmit) {
      dontstep = validate(tmp, cursorPos) != QValidator::Acceptable;
      d->cleared = false;
      d->interpret(NeverEmit);
      if (d->value != old) {
         e = AlwaysEmit;
      }
   }
   if (!dontstep) {
      d->setValue(d->bound(d->value + (d->singleStep * steps), old, steps), e);
   } else if (e == AlwaysEmit) {
      d->emitSignals(e, old);
   }
   selectAll();
}

/*!
    This function returns a pointer to the line edit of the spin box.
*/

QLineEdit *QAbstractSpinBox::lineEdit() const
{
   Q_D(const QAbstractSpinBox);

   return d->edit;
}


/*!
    \fn void QAbstractSpinBox::setLineEdit(QLineEdit *lineEdit)

    Sets the line edit of the spinbox to be \a lineEdit instead of the
    current line edit widget. \a lineEdit can not be 0.

    QAbstractSpinBox takes ownership of the new lineEdit

    If QLineEdit::validator() for the \a lineEdit returns 0, the internal
    validator of the spinbox will be set on the line edit.
*/

void QAbstractSpinBox::setLineEdit(QLineEdit *lineEdit)
{
   Q_D(QAbstractSpinBox);

   if (!lineEdit) {
      Q_ASSERT(lineEdit);
      return;
   }
   delete d->edit;
   d->edit = lineEdit;
   if (!d->edit->validator()) {
      d->edit->setValidator(d->validator);
   }

   if (d->edit->parent() != this) {
      d->edit->setParent(this);
   }

   d->edit->setFrame(false);
   d->edit->setFocusProxy(this);
   d->edit->setAcceptDrops(false);

   if (d->type != QVariant::Invalid) {
      connect(d->edit, SIGNAL(textChanged(const QString &)), this, SLOT(_q_editorTextChanged(const QString &)));
      connect(d->edit, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(_q_editorCursorPositionChanged(int, int)));
   }

   d->updateEditFieldGeometry();
   d->edit->setContextMenuPolicy(Qt::NoContextMenu);

   if (isVisible()) {
      d->edit->show();
   }
   if (isVisible()) {
      d->updateEdit();
   }
}


/*!
    This function interprets the text of the spin box. If the value
    has changed since last interpretation it will emit signals.
*/

void QAbstractSpinBox::interpretText()
{
   Q_D(QAbstractSpinBox);
   d->interpret(EmitIfChanged);
}

/*
    Reimplemented in 4.6, so be careful.
 */
/*!
    \reimp
*/
QVariant QAbstractSpinBox::inputMethodQuery(Qt::InputMethodQuery query) const
{
   Q_D(const QAbstractSpinBox);
   return d->edit->inputMethodQuery(query);
}

/*!
    \reimp
*/

bool QAbstractSpinBox::event(QEvent *event)
{
   Q_D(QAbstractSpinBox);
   switch (event->type()) {
      case QEvent::FontChange:
      case QEvent::StyleChange:
         d->cachedSizeHint = d->cachedMinimumSizeHint = QSize();
         break;
      case QEvent::ApplicationLayoutDirectionChange:
      case QEvent::LayoutDirectionChange:
         d->updateEditFieldGeometry();
         break;
      case QEvent::HoverEnter:
      case QEvent::HoverLeave:
      case QEvent::HoverMove:
         if (const QHoverEvent *he = static_cast<const QHoverEvent *>(event)) {
            d->updateHoverControl(he->pos());
         }
         break;
      case QEvent::ShortcutOverride:
         if (d->edit->event(event)) {
            return true;
         }
         break;
#ifdef QT_KEYPAD_NAVIGATION
      case QEvent::EnterEditFocus:
      case QEvent::LeaveEditFocus:
         if (QApplication::keypadNavigationEnabled()) {
            const bool b = d->edit->event(event);
            d->edit->setSelection(d->edit->displayText().size() - d->suffix.size(), 0);
            if (event->type() == QEvent::LeaveEditFocus) {
               emit editingFinished();
            }
            if (b) {
               return true;
            }
         }
         break;
#endif
      case QEvent::InputMethod:
         return d->edit->event(event);
      default:
         break;
   }
   return QWidget::event(event);
}

/*!
    \reimp
*/

void QAbstractSpinBox::showEvent(QShowEvent *)
{
   Q_D(QAbstractSpinBox);
   d->reset();

   if (d->ignoreUpdateEdit) {
      d->ignoreUpdateEdit = false;
   } else {
      d->updateEdit();
   }
}

/*!
    \reimp
*/

void QAbstractSpinBox::changeEvent(QEvent *event)
{
   Q_D(QAbstractSpinBox);

   switch (event->type()) {
      case QEvent::StyleChange:
         d->spinClickTimerInterval = style()->styleHint(QStyle::SH_SpinBox_ClickAutoRepeatRate, 0, this);
         d->spinClickThresholdTimerInterval =
            style()->styleHint(QStyle::SH_SpinBox_ClickAutoRepeatThreshold, 0, this);
         d->reset();
         d->updateEditFieldGeometry();
         break;
      case QEvent::EnabledChange:
         if (!isEnabled()) {
            d->reset();
         }
         break;
      case QEvent::ActivationChange:
         if (!isActiveWindow()) {
            d->reset();
            if (d->pendingEmit) { // pendingEmit can be true even if it hasn't changed.
               d->interpret(EmitIfChanged);   // E.g. 10 to 10.0
            }
         }
         break;
      default:
         break;
   }
   QWidget::changeEvent(event);
}

/*!
    \reimp
*/

void QAbstractSpinBox::resizeEvent(QResizeEvent *event)
{
   Q_D(QAbstractSpinBox);
   QWidget::resizeEvent(event);

   d->updateEditFieldGeometry();
   update();
}

/*!
    \reimp
*/

QSize QAbstractSpinBox::sizeHint() const
{
   Q_D(const QAbstractSpinBox);
   if (d->cachedSizeHint.isEmpty()) {
      ensurePolished();

      const QFontMetrics fm(fontMetrics());
      int h = d->edit->sizeHint().height();
      int w = 0;
      QString s;
      s = d->prefix + d->textFromValue(d->minimum) + d->suffix + QLatin1Char(' ');
      s.truncate(18);
      w = qMax(w, fm.width(s));
      s = d->prefix + d->textFromValue(d->maximum) + d->suffix + QLatin1Char(' ');
      s.truncate(18);
      w = qMax(w, fm.width(s));
      if (d->specialValueText.size()) {
         s = d->specialValueText;
         w = qMax(w, fm.width(s));
      }
      w += 2; // cursor blinking space

      QStyleOptionSpinBox opt;
      initStyleOption(&opt);
      QSize hint(w, h);
      QSize extra(35, 6);
      opt.rect.setSize(hint + extra);
      extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                              QStyle::SC_SpinBoxEditField, this).size();
      // get closer to final result by repeating the calculation
      opt.rect.setSize(hint + extra);
      extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                              QStyle::SC_SpinBoxEditField, this).size();
      hint += extra;

      opt.rect = rect();
      d->cachedSizeHint = style()->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, this)
                          .expandedTo(QApplication::globalStrut());
   }
   return d->cachedSizeHint;
}

/*!
    \reimp
*/

QSize QAbstractSpinBox::minimumSizeHint() const
{
   Q_D(const QAbstractSpinBox);
   if (d->cachedMinimumSizeHint.isEmpty()) {
      ensurePolished();

      const QFontMetrics fm(fontMetrics());
      int h = d->edit->minimumSizeHint().height();
      int w = fm.width(QLatin1String("1000"));
      w += 2; // cursor blinking space

      QStyleOptionSpinBox opt;
      initStyleOption(&opt);
      QSize hint(w, h);
      QSize extra(35, 6);
      opt.rect.setSize(hint + extra);
      extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                              QStyle::SC_SpinBoxEditField, this).size();
      // get closer to final result by repeating the calculation
      opt.rect.setSize(hint + extra);
      extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                              QStyle::SC_SpinBoxEditField, this).size();
      hint += extra;

      opt.rect = rect();

      d->cachedMinimumSizeHint = style()->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, this)
                                 .expandedTo(QApplication::globalStrut());
   }
   return d->cachedMinimumSizeHint;
}

/*!
    \reimp
*/

void QAbstractSpinBox::paintEvent(QPaintEvent *)
{
   QStyleOptionSpinBox opt;
   initStyleOption(&opt);
   QStylePainter p(this);
   p.drawComplexControl(QStyle::CC_SpinBox, opt);
}

/*!
    \reimp

    This function handles keyboard input.

    The following keys are handled specifically:
    \table
    \row \i Enter/Return
         \i This will reinterpret the text and emit a signal even if the value has not changed
         since last time a signal was emitted.
    \row \i Up
         \i This will invoke stepBy(1)
    \row \i Down
         \i This will invoke stepBy(-1)
    \row \i Page up
         \i This will invoke stepBy(10)
    \row \i Page down
         \i This will invoke stepBy(-10)
    \endtable
*/


void QAbstractSpinBox::keyPressEvent(QKeyEvent *event)
{
   Q_D(QAbstractSpinBox);

   if (!event->text().isEmpty() && d->edit->cursorPosition() < d->prefix.size()) {
      d->edit->setCursorPosition(d->prefix.size());
   }

   int steps = 1;
   bool isPgUpOrDown = false;
   switch (event->key()) {
      case Qt::Key_PageUp:
      case Qt::Key_PageDown:
         steps *= 10;
         isPgUpOrDown = true;
      case Qt::Key_Up:
      case Qt::Key_Down: {
#ifdef QT_KEYPAD_NAVIGATION
         if (QApplication::keypadNavigationEnabled()) {
            // Reserve up/down for nav - use left/right for edit.
            if (!hasEditFocus() && (event->key() == Qt::Key_Up
                                    || event->key() == Qt::Key_Down)) {
               event->ignore();
               return;
            }
         }
#endif
         event->accept();
         const bool up = (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_Up);
         if (!(stepEnabled() & (up ? StepUpEnabled : StepDownEnabled))) {
            return;
         }
         if (!up) {
            steps *= -1;
         }
         if (style()->styleHint(QStyle::SH_SpinBox_AnimateButton, 0, this)) {
            d->buttonState = (Keyboard | (up ? Up : Down));
         }
         if (d->spinClickTimerId == -1) {
            stepBy(steps);
         }
         if (event->isAutoRepeat() && !isPgUpOrDown) {
            if (d->spinClickThresholdTimerId == -1 && d->spinClickTimerId == -1) {
               d->updateState(up, true);
            }
         }
#ifndef QT_NO_ACCESSIBILITY
         QAccessible::updateAccessibility(this, 0, QAccessible::ValueChanged);
#endif
         return;
      }
#ifdef QT_KEYPAD_NAVIGATION
      case Qt::Key_Left:
      case Qt::Key_Right:
         if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
            event->ignore();
            return;
         }
         break;
      case Qt::Key_Back:
         if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
            event->ignore();
            return;
         }
         break;
#endif
      case Qt::Key_Enter:
      case Qt::Key_Return:
         d->edit->d_func()->control->clearUndo();
         d->interpret(d->keyboardTracking ? AlwaysEmit : EmitIfChanged);
         selectAll();
         event->ignore();
         emit editingFinished();
         emit d->edit->returnPressed();
         return;

#ifdef QT_KEYPAD_NAVIGATION
      case Qt::Key_Select:
         if (QApplication::keypadNavigationEnabled()) {
            // Toggles between left/right moving cursor and inc/dec.
            setEditFocus(!hasEditFocus());
         }
         return;
#endif

#ifdef Q_WS_X11 // only X11
      case Qt::Key_U:
         if (event->modifiers() & Qt::ControlModifier) {
            event->accept();
            if (!isReadOnly()) {
               clear();
            }
            return;
         }
         break;
#endif

      case Qt::Key_End:
      case Qt::Key_Home:
         if (event->modifiers() & Qt::ShiftModifier) {
            int currentPos = d->edit->cursorPosition();
            const QString text = d->edit->displayText();
            if (event->key() == Qt::Key_End) {
               if ((currentPos == 0 && !d->prefix.isEmpty()) || text.size() - d->suffix.size() <= currentPos) {
                  break; // let lineedit handle this
               } else {
                  d->edit->setSelection(currentPos, text.size() - d->suffix.size() - currentPos);
               }
            } else {
               if ((currentPos == text.size() && !d->suffix.isEmpty()) || currentPos <= d->prefix.size()) {
                  break; // let lineedit handle this
               } else {
                  d->edit->setSelection(currentPos, d->prefix.size() - currentPos);
               }
            }
            event->accept();
            return;
         }
         break;

      default:
#ifndef QT_NO_SHORTCUT
         if (event == QKeySequence::SelectAll) {
            selectAll();
            event->accept();
            return;
         }
#endif
         break;
   }

   d->edit->event(event);
   if (!isVisible()) {
      d->ignoreUpdateEdit = true;
   }
}

/*!
    \reimp
*/

void QAbstractSpinBox::keyReleaseEvent(QKeyEvent *event)
{
   Q_D(QAbstractSpinBox);

   if (d->buttonState & Keyboard && !event->isAutoRepeat())  {
      d->reset();
   } else {
      d->edit->event(event);
   }
}

/*!
    \reimp
*/

#ifndef QT_NO_WHEELEVENT
void QAbstractSpinBox::wheelEvent(QWheelEvent *event)
{
   const int steps = (event->delta() > 0 ? 1 : -1);
   if (stepEnabled() & (steps > 0 ? StepUpEnabled : StepDownEnabled)) {
      stepBy(event->modifiers() & Qt::ControlModifier ? steps * 10 : steps);
   }
   event->accept();
}
#endif


/*!
    \reimp
*/
void QAbstractSpinBox::focusInEvent(QFocusEvent *event)
{
   Q_D(QAbstractSpinBox);

   d->edit->event(event);
   if (event->reason() == Qt::TabFocusReason || event->reason() == Qt::BacktabFocusReason) {
      selectAll();
   }
   QWidget::focusInEvent(event);
}

/*!
    \reimp
*/

void QAbstractSpinBox::focusOutEvent(QFocusEvent *event)
{
   Q_D(QAbstractSpinBox);

   if (d->pendingEmit) {
      d->interpret(EmitIfChanged);
   }

   d->reset();
   d->edit->event(event);
   d->updateEdit();
   QWidget::focusOutEvent(event);

#ifdef QT_KEYPAD_NAVIGATION
   // editingFinished() is already emitted on LeaveEditFocus
   if (!QApplication::keypadNavigationEnabled())
#endif
      emit editingFinished();
}

/*!
    \reimp
*/

void QAbstractSpinBox::closeEvent(QCloseEvent *event)
{
   Q_D(QAbstractSpinBox);

   d->reset();
   if (d->pendingEmit) {
      d->interpret(EmitIfChanged);
   }
   QWidget::closeEvent(event);
}

/*!
    \reimp
*/

void QAbstractSpinBox::hideEvent(QHideEvent *event)
{
   Q_D(QAbstractSpinBox);
   d->reset();
   if (d->pendingEmit) {
      d->interpret(EmitIfChanged);
   }
   QWidget::hideEvent(event);
}


/*!
    \internal

    Used when acceleration is turned on. We need to get the
    keyboard auto repeat rate from OS. This value is used as
    argument when starting acceleration related timers.

    Every platform should, either, use native calls to obtain
    the value or hard code some reasonable rate.

    Remember that time value should be given in msecs.
*/
static int getKeyboardAutoRepeatRate()
{
   int ret = 30;

#if defined(Q_OS_WIN)
   DWORD time;
   if (SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &time, 0) != FALSE) {
      ret = static_cast<int>(1000 / static_cast<int>(time));   // msecs
   }
#endif
   return ret; // msecs
}

/*!
    \reimp
*/

void QAbstractSpinBox::timerEvent(QTimerEvent *event)
{
   Q_D(QAbstractSpinBox);

   bool doStep = false;
   if (event->timerId() == d->spinClickThresholdTimerId) {
      killTimer(d->spinClickThresholdTimerId);
      d->spinClickThresholdTimerId = -1;
      d->effectiveSpinRepeatRate = d->buttonState & Keyboard
                                   ? getKeyboardAutoRepeatRate()
                                   : d->spinClickTimerInterval;
      d->spinClickTimerId = startTimer(d->effectiveSpinRepeatRate);
      doStep = true;
   } else if (event->timerId() == d->spinClickTimerId) {
      if (d->accelerate) {
         d->acceleration = d->acceleration + (int)(d->effectiveSpinRepeatRate * 0.05);
         if (d->effectiveSpinRepeatRate - d->acceleration >= 10) {
            killTimer(d->spinClickTimerId);
            d->spinClickTimerId = startTimer(d->effectiveSpinRepeatRate - d->acceleration);
         }
      }
      doStep = true;
   }

   if (doStep) {
      const StepEnabled st = stepEnabled();
      if (d->buttonState & Up) {
         if (!(st & StepUpEnabled)) {
            d->reset();
         } else {
            stepBy(1);
         }
      } else if (d->buttonState & Down) {
         if (!(st & StepDownEnabled)) {
            d->reset();
         } else {
            stepBy(-1);
         }
      }
      return;
   }
   QWidget::timerEvent(event);
   return;
}

/*!
    \reimp
*/

void QAbstractSpinBox::contextMenuEvent(QContextMenuEvent *event)
{
#ifdef QT_NO_CONTEXTMENU
   Q_UNUSED(event);
#else
   Q_D(QAbstractSpinBox);

   QPointer<QMenu> menu = d->edit->createStandardContextMenu();
   if (!menu) {
      return;
   }

   d->reset();

   QAction *selAll = new QAction(tr("&Select All"), menu);
   menu->insertAction(d->edit->d_func()->selectAllAction,
                      selAll);
   menu->removeAction(d->edit->d_func()->selectAllAction);
   menu->addSeparator();
   const uint se = stepEnabled();
   QAction *up = menu->addAction(tr("&Step up"));
   up->setEnabled(se & StepUpEnabled);
   QAction *down = menu->addAction(tr("Step &down"));
   down->setEnabled(se & StepDownEnabled);
   menu->addSeparator();

   const QPointer<QAbstractSpinBox> that = this;
   const QPoint pos = (event->reason() == QContextMenuEvent::Mouse)
                      ? event->globalPos() : mapToGlobal(QPoint(event->pos().x(), 0)) + QPoint(width() / 2, height() / 2);
   const QAction *action = menu->exec(pos);
   delete static_cast<QMenu *>(menu);
   if (that && action) {
      if (action == up) {
         stepBy(1);
      } else if (action == down) {
         stepBy(-1);
      } else if (action == selAll) {
         selectAll();
      }
   }
   event->accept();
#endif // QT_NO_CONTEXTMENU
}

/*!
    \reimp
*/

void QAbstractSpinBox::mouseMoveEvent(QMouseEvent *event)
{
   Q_D(QAbstractSpinBox);

   d->updateHoverControl(event->pos());

   // If we have a timer ID, update the state
   if (d->spinClickTimerId != -1 && d->buttonSymbols != NoButtons) {
      const StepEnabled se = stepEnabled();
      if ((se & StepUpEnabled) && d->hoverControl == QStyle::SC_SpinBoxUp) {
         d->updateState(true);
      } else if ((se & StepDownEnabled) && d->hoverControl == QStyle::SC_SpinBoxDown) {
         d->updateState(false);
      } else {
         d->reset();
      }
      event->accept();
   }
}

/*!
    \reimp
*/

void QAbstractSpinBox::mousePressEvent(QMouseEvent *event)
{
   Q_D(QAbstractSpinBox);

   if (event->button() != Qt::LeftButton || d->buttonState != None) {
      return;
   }

   d->updateHoverControl(event->pos());
   event->accept();

   const StepEnabled se = (d->buttonSymbols == NoButtons) ? StepEnabled(StepNone) : stepEnabled();
   if ((se & StepUpEnabled) && d->hoverControl == QStyle::SC_SpinBoxUp) {
      d->updateState(true);
   } else if ((se & StepDownEnabled) && d->hoverControl == QStyle::SC_SpinBoxDown) {
      d->updateState(false);
   } else {
      event->ignore();
   }
}

/*!
    \reimp
*/
void QAbstractSpinBox::mouseReleaseEvent(QMouseEvent *event)
{
   Q_D(QAbstractSpinBox);

   if ((d->buttonState & Mouse) != 0) {
      d->reset();
   }
   event->accept();
}

// --- QAbstractSpinBoxPrivate ---

/*!
    \internal
    Constructs a QAbstractSpinBoxPrivate object
*/

QAbstractSpinBoxPrivate::QAbstractSpinBoxPrivate()
   : edit(0), type(QVariant::Invalid), spinClickTimerId(-1),
     spinClickTimerInterval(100), spinClickThresholdTimerId(-1), spinClickThresholdTimerInterval(-1),
     effectiveSpinRepeatRate(1), buttonState(None), cachedText(QLatin1String("\x01")),
     cachedState(QValidator::Invalid), pendingEmit(false), readOnly(false), wrapping(false),
     ignoreCursorPositionChanged(false), frame(true), accelerate(false), keyboardTracking(true),
     cleared(false), ignoreUpdateEdit(false), correctionMode(QAbstractSpinBox::CorrectToPreviousValue),
     acceleration(0), hoverControl(QStyle::SC_None), buttonSymbols(QAbstractSpinBox::UpDownArrows), validator(0)
{
}

/*
   \internal
   Called when the QAbstractSpinBoxPrivate is destroyed
*/
QAbstractSpinBoxPrivate::~QAbstractSpinBoxPrivate()
{
}

/*!
    \internal
    Updates the old and new hover control. Does nothing if the hover
    control has not changed.
*/
bool QAbstractSpinBoxPrivate::updateHoverControl(const QPoint &pos)
{
   Q_Q(QAbstractSpinBox);
   QRect lastHoverRect = hoverRect;
   QStyle::SubControl lastHoverControl = hoverControl;
   bool doesHover = q->testAttribute(Qt::WA_Hover);
   if (lastHoverControl != newHoverControl(pos) && doesHover) {
      q->update(lastHoverRect);
      q->update(hoverRect);
      return true;
   }
   return !doesHover;
}

/*!
    \internal
    Returns the hover control at \a pos.
    This will update the hoverRect and hoverControl.
*/
QStyle::SubControl QAbstractSpinBoxPrivate::newHoverControl(const QPoint &pos)
{
   Q_Q(QAbstractSpinBox);

   QStyleOptionSpinBox opt;
   q->initStyleOption(&opt);
   opt.subControls = QStyle::SC_All;
   hoverControl = q->style()->hitTestComplexControl(QStyle::CC_SpinBox, &opt, pos, q);
   hoverRect = q->style()->subControlRect(QStyle::CC_SpinBox, &opt, hoverControl, q);
   return hoverControl;
}

/*!
    \internal
    Strips any prefix/suffix from \a text.
*/

QString QAbstractSpinBoxPrivate::stripped(const QString &t, int *pos) const
{
   QString text = t;
   if (specialValueText.size() == 0 || text != specialValueText) {
      int from = 0;
      int size = text.size();
      bool changed = false;
      if (prefix.size() && text.startsWith(prefix)) {
         from += prefix.size();
         size -= from;
         changed = true;
      }
      if (suffix.size() && text.endsWith(suffix)) {
         size -= suffix.size();
         changed = true;
      }
      if (changed) {
         text = text.mid(from, size);
      }
   }

   const int s = text.size();
   text = text.trimmed();
   if (pos) {
      (*pos) -= (s - text.size());
   }
   return text;

}

void QAbstractSpinBoxPrivate::updateEditFieldGeometry()
{
   Q_Q(QAbstractSpinBox);
   QStyleOptionSpinBox opt;
   q->initStyleOption(&opt);
   opt.subControls = QStyle::SC_SpinBoxEditField;
   edit->setGeometry(q->style()->subControlRect(QStyle::CC_SpinBox, &opt,
                     QStyle::SC_SpinBoxEditField, q));
}
/*!
    \internal
    Returns true if a specialValueText has been set and the current value is minimum.
*/

bool QAbstractSpinBoxPrivate::specialValue() const
{
   return (value == minimum && !specialValueText.isEmpty());
}

/*!
    \internal Virtual function that emits signals when the value
    changes. Reimplemented in the different subclasses.
*/

void QAbstractSpinBoxPrivate::emitSignals(EmitPolicy, const QVariant &)
{
}

/*!
    \internal

    Slot connected to the line edit's textChanged(const QString &)
    signal.
*/

void QAbstractSpinBoxPrivate::_q_editorTextChanged(const QString &t)
{
   Q_Q(QAbstractSpinBox);

   if (keyboardTracking) {
      QString tmp = t;
      int pos = edit->cursorPosition();
      QValidator::State state = q->validate(tmp, pos);
      if (state == QValidator::Acceptable) {
         const QVariant v = valueFromText(tmp);
         setValue(v, EmitIfChanged, tmp != t);
         pendingEmit = false;
      } else {
         pendingEmit = true;
      }
   } else {
      pendingEmit = true;
   }
}

/*!
    \internal

    Virtual slot connected to the line edit's
    cursorPositionChanged(int, int) signal. Will move the cursor to a
    valid position if the new one is invalid. E.g. inside the prefix.
    Reimplemented in Q[Date|Time|DateTime]EditPrivate to account for
    the different sections etc.
*/

void QAbstractSpinBoxPrivate::_q_editorCursorPositionChanged(int oldpos, int newpos)
{
   if (!edit->hasSelectedText() && !ignoreCursorPositionChanged && !specialValue()) {
      ignoreCursorPositionChanged = true;

      bool allowSelection = true;
      int pos = -1;
      if (newpos < prefix.size() && newpos != 0) {
         if (oldpos == 0) {
            allowSelection = false;
            pos = prefix.size();
         } else {
            pos = oldpos;
         }
      } else if (newpos > edit->text().size() - suffix.size()
                 && newpos != edit->text().size()) {
         if (oldpos == edit->text().size()) {
            pos = edit->text().size() - suffix.size();
            allowSelection = false;
         } else {
            pos = edit->text().size();
         }
      }
      if (pos != -1) {
         const int selSize = edit->selectionStart() >= 0 && allowSelection
                             ? (edit->selectedText().size()
                                * (newpos < pos ? -1 : 1)) - newpos + pos
                             : 0;

         const bool wasBlocked = edit->blockSignals(true);
         if (selSize != 0) {
            edit->setSelection(pos - selSize, selSize);
         } else {
            edit->setCursorPosition(pos);
         }
         edit->blockSignals(wasBlocked);
      }
      ignoreCursorPositionChanged = false;
   }
}

/*!
    \internal

    Initialises the QAbstractSpinBoxPrivate object.
*/

void QAbstractSpinBoxPrivate::init()
{
   Q_Q(QAbstractSpinBox);

   q->setLineEdit(new QLineEdit(q));
   edit->setObjectName(QLatin1String("qt_spinbox_lineedit"));
   validator = new QSpinBoxValidator(q, this);
   edit->setValidator(validator);

   QStyleOptionSpinBox opt;
   q->initStyleOption(&opt);
   spinClickTimerInterval = q->style()->styleHint(QStyle::SH_SpinBox_ClickAutoRepeatRate, &opt, q);
   spinClickThresholdTimerInterval = q->style()->styleHint(QStyle::SH_SpinBox_ClickAutoRepeatThreshold, &opt, q);
   q->setFocusPolicy(Qt::WheelFocus);
   q->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed, QSizePolicy::SpinBox));
   q->setAttribute(Qt::WA_InputMethodEnabled);

   q->setAttribute(Qt::WA_MacShowFocusRect);
}

/*!
    \internal

    Resets the state of the spinbox. E.g. the state is set to
    (Keyboard|Up) if Key up is currently pressed.
*/

void QAbstractSpinBoxPrivate::reset()
{
   Q_Q(QAbstractSpinBox);

   buttonState = None;
   if (q) {
      if (spinClickTimerId != -1) {
         q->killTimer(spinClickTimerId);
      }
      if (spinClickThresholdTimerId != -1) {
         q->killTimer(spinClickThresholdTimerId);
      }
      spinClickTimerId = spinClickThresholdTimerId = -1;
      acceleration = 0;
      q->update();
   }
}

/*!
    \internal

    Updates the state of the spinbox.
*/

void QAbstractSpinBoxPrivate::updateState(bool up, bool fromKeyboard /* = false */)
{
   Q_Q(QAbstractSpinBox);
   if ((up && (buttonState & Up)) || (!up && (buttonState & Down))) {
      return;
   }
   reset();
   if (q && (q->stepEnabled() & (up ? QAbstractSpinBox::StepUpEnabled
                                 : QAbstractSpinBox::StepDownEnabled))) {
      spinClickThresholdTimerId = q->startTimer(spinClickThresholdTimerInterval);
      buttonState = (up ? Up : Down) | (fromKeyboard ? Keyboard : Mouse);
      q->stepBy(up ? 1 : -1);
#ifndef QT_NO_ACCESSIBILITY
      QAccessible::updateAccessibility(q, 0, QAccessible::ValueChanged);
#endif
   }
}


/*!
    Initialize \a option with the values from this QSpinBox. This method
    is useful for subclasses when they need a QStyleOptionSpinBox, but don't want
    to fill in all the information themselves.

    \sa QStyleOption::initFrom()
*/
void QAbstractSpinBox::initStyleOption(QStyleOptionSpinBox *option) const
{
   if (!option) {
      return;
   }

   Q_D(const QAbstractSpinBox);
   option->initFrom(this);
   option->activeSubControls = QStyle::SC_None;
   option->buttonSymbols = d->buttonSymbols;
   option->subControls = QStyle::SC_SpinBoxFrame | QStyle::SC_SpinBoxEditField;
   if (d->buttonSymbols != QAbstractSpinBox::NoButtons) {
      option->subControls |= QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown;
      if (d->buttonState & Up) {
         option->activeSubControls = QStyle::SC_SpinBoxUp;
      } else if (d->buttonState & Down) {
         option->activeSubControls = QStyle::SC_SpinBoxDown;
      }
   }

   if (d->buttonState) {
      option->state |= QStyle::State_Sunken;
   } else {
      option->activeSubControls = d->hoverControl;
   }

   option->stepEnabled = style()->styleHint(QStyle::SH_SpinControls_DisableOnBounds)
                         ? stepEnabled()
                         : (QAbstractSpinBox::StepDownEnabled | QAbstractSpinBox::StepUpEnabled);

   option->frame = d->frame;
   if (d->readOnly) {
      option->state |= QStyle::State_ReadOnly;
   }
}

/*!
    \internal

    Bounds \a val to be within minimum and maximum. Also tries to be
    clever about setting it at min and max depending on what it was
    and what direction it was changed etc.
*/

QVariant QAbstractSpinBoxPrivate::bound(const QVariant &val, const QVariant &old, int steps) const
{
   QVariant v = val;
   if (!wrapping || steps == 0 || old.isNull()) {
      if (variantCompare(v, minimum) < 0) {
         v = wrapping ? maximum : minimum;
      }
      if (variantCompare(v, maximum) > 0) {
         v = wrapping ? minimum : maximum;
      }
   } else {
      const bool wasMin = old == minimum;
      const bool wasMax = old == maximum;
      const int oldcmp = variantCompare(v, old);
      const int maxcmp = variantCompare(v, maximum);
      const int mincmp = variantCompare(v, minimum);
      const bool wrapped = (oldcmp > 0 && steps < 0) || (oldcmp < 0 && steps > 0);
      if (maxcmp > 0) {
         v = ((wasMax && !wrapped && steps > 0) || (steps < 0 && !wasMin && wrapped))
             ? minimum : maximum;
      } else if (wrapped && (maxcmp > 0 || mincmp < 0)) {
         v = ((wasMax && steps > 0) || (!wasMin && steps < 0)) ? minimum : maximum;
      } else if (mincmp < 0) {
         v = (!wasMax && !wasMin ? minimum : maximum);
      }
   }

   return v;
}

/*!
    \internal

    Sets the value of the spin box to \a val. Depending on the value
    of \a ep it will also emit signals.
*/

void QAbstractSpinBoxPrivate::setValue(const QVariant &val, EmitPolicy ep, bool doUpdate)
{
   Q_Q(QAbstractSpinBox);

   const QVariant old = value;
   value = bound(val);
   pendingEmit = false;
   cleared = false;
   if (doUpdate) {
      updateEdit();
   }

   q->update();

   if (ep == AlwaysEmit || (ep == EmitIfChanged && old != value)) {
      emitSignals(ep, old);
   }
}

/*!
    \internal

    Updates the line edit to reflect the current value of the spin box.
*/

void QAbstractSpinBoxPrivate::updateEdit()
{
   Q_Q(QAbstractSpinBox);
   if (type == QVariant::Invalid) {
      return;
   }
   const QString newText = specialValue() ? specialValueText : prefix + textFromValue(value) + suffix;
   if (newText == edit->displayText() || cleared) {
      return;
   }

   const bool empty = edit->text().isEmpty();
   int cursor = edit->cursorPosition();
   int selsize = edit->selectedText().size();
   const bool sb = edit->blockSignals(true);
   edit->setText(newText);

   if (! specialValue()) {
      cursor = qBound(prefix.size(), cursor, edit->displayText().size() - suffix.size());

      if (selsize > 0) {
         edit->setSelection(cursor, selsize);
      } else {
         edit->setCursorPosition(empty ? prefix.size() : cursor);
      }
   }

   edit->blockSignals(sb);
   q->update();
}

/*!
    \internal

    Convenience function to set min/max values.
*/

void QAbstractSpinBoxPrivate::setRange(const QVariant &min, const QVariant &max)
{
   Q_Q(QAbstractSpinBox);

   clearCache();
   minimum = min;
   maximum = (variantCompare(min, max) < 0 ? max : min);
   cachedSizeHint = QSize(); // minimumSizeHint doesn't care about min/max

   reset();
   if (!(bound(value) == value)) {
      setValue(bound(value), EmitIfChanged);
   } else if (value == minimum && !specialValueText.isEmpty()) {
      updateEdit();
   }

   q->updateGeometry();
}

/*!
    \internal

    Convenience function to get a variant of the right type.
*/

QVariant QAbstractSpinBoxPrivate::getZeroVariant() const
{
   QVariant ret;
   switch (type) {
      case QVariant::Int:
         ret = QVariant((int)0);
         break;
      case QVariant::Double:
         ret = QVariant((double)0.0);
         break;
      default:
         break;
   }
   return ret;
}

/*!
    \internal

    Virtual method called that calls the public textFromValue()
    functions in the subclasses. Needed to change signature from
    QVariant to int/double/QDateTime etc. Used when needing to display
    a value textually.

    This method is reimeplemented in the various subclasses.
*/

QString QAbstractSpinBoxPrivate::textFromValue(const QVariant &) const
{
   return QString();
}

/*!
    \internal

    Virtual method called that calls the public valueFromText()
    functions in the subclasses. Needed to change signature from
    QVariant to int/double/QDateTime etc. Used when needing to
    interpret a string as another type.

    This method is reimeplemented in the various subclasses.
*/

QVariant QAbstractSpinBoxPrivate::valueFromText(const QString &) const
{
   return QVariant();
}
/*!
    \internal

    Interprets text and emits signals. Called when the spinbox needs
    to interpret the text on the lineedit.
*/

void QAbstractSpinBoxPrivate::interpret(EmitPolicy ep)
{
   Q_Q(QAbstractSpinBox);
   if (type == QVariant::Invalid || cleared) {
      return;
   }

   QVariant v = getZeroVariant();
   bool doInterpret = true;
   QString tmp = edit->displayText();
   int pos = edit->cursorPosition();
   const int oldpos = pos;

   if (q->validate(tmp, pos) != QValidator::Acceptable) {
      const QString copy = tmp;
      q->fixup(tmp);
      QASBDEBUG() << "QAbstractSpinBoxPrivate::interpret() text '"
                  << edit->displayText()
                  << "' >> '" << copy << '\''
                  << "' >> '" << tmp << '\'';

      doInterpret = tmp != copy && (q->validate(tmp, pos) == QValidator::Acceptable);
      if (!doInterpret) {
         v = (correctionMode == QAbstractSpinBox::CorrectToNearestValue
              ? variantBound(minimum, v, maximum) : value);
      }
   }
   if (doInterpret) {
      v = valueFromText(tmp);
   }
   clearCache();
   setValue(v, ep, true);
   if (oldpos != pos) {
      edit->setCursorPosition(pos);
   }
}

void QAbstractSpinBoxPrivate::clearCache() const
{
   cachedText.clear();
   cachedValue.clear();
   cachedState = QValidator::Acceptable;
}


// --- QSpinBoxValidator ---

/*!
    \internal
    Constructs a QSpinBoxValidator object
*/

QSpinBoxValidator::QSpinBoxValidator(QAbstractSpinBox *qp, QAbstractSpinBoxPrivate *dp)
   : QValidator(qp), qptr(qp), dptr(dp)
{
   setObjectName(QLatin1String("qt_spinboxvalidator"));
}

/*!
    \internal

    Checks for specialValueText, prefix, suffix and calls
    the virtual QAbstractSpinBox::validate function.
*/

QValidator::State QSpinBoxValidator::validate(QString &input, int &pos) const
{
   if (dptr->specialValueText.size() > 0 && input == dptr->specialValueText) {
      return QValidator::Acceptable;
   }

   if (!dptr->prefix.isEmpty() && !input.startsWith(dptr->prefix)) {
      input.prepend(dptr->prefix);
      pos += dptr->prefix.length();
   }

   if (!dptr->suffix.isEmpty() && !input.endsWith(dptr->suffix)) {
      input.append(dptr->suffix);
   }

   return qptr->validate(input, pos);
}
/*!
    \internal
    Calls the virtual QAbstractSpinBox::fixup function.
*/

void QSpinBoxValidator::fixup(QString &input) const
{
   qptr->fixup(input);
}

// --- global ---

/*!
    \internal
    Adds two variants together and returns the result.
*/

QVariant operator+(const QVariant &arg1, const QVariant &arg2)
{
   QVariant ret;

   if (arg1.type() != arg2.type())
      qWarning("QAbstractSpinBox: Internal error: Different types (%s vs %s) (%s:%d)",
               csPrintable(arg1.typeName()), csPrintable(arg2.typeName()),  __FILE__, __LINE__);

   switch (arg1.type()) {
      case QVariant::Int:
         ret = QVariant(arg1.toInt() + arg2.toInt());
         break;

      case QVariant::Double:
         ret = QVariant(arg1.toDouble() + arg2.toDouble());
         break;

      case QVariant::DateTime: {
         QDateTime a2 = arg2.toDateTime();
         QDateTime a1 = arg1.toDateTime().addDays(QDATETIMEEDIT_DATETIME_MIN.daysTo(a2));
         a1.setTime(a1.time().addMSecs(QTime().msecsTo(a2.time())));
         ret = QVariant(a1);
      }
      break;

      default:
         break;
   }

   return ret;
}


/*!
    \internal
    Subtracts two variants and returns the result.
*/

QVariant operator-(const QVariant &arg1, const QVariant &arg2)
{
   QVariant ret;

   if (arg1.type() != arg2.type())
      qWarning("QAbstractSpinBox: Internal error: Different types (%s vs %s) (%s:%d)",
               csPrintable(arg1.typeName()), csPrintable(arg2.typeName()),  __FILE__, __LINE__);

   switch (arg1.type()) {
      case QVariant::Int:
         ret = QVariant(arg1.toInt() - arg2.toInt());
         break;

      case QVariant::Double:
         ret = QVariant(arg1.toDouble() - arg2.toDouble());
         break;

      case QVariant::DateTime: {
         QDateTime a1 = arg1.toDateTime();
         QDateTime a2 = arg2.toDateTime();

         int days = a2.daysTo(a1);
         int secs = a2.secsTo(a1);
         int msecs = qMax(0, a1.time().msec() - a2.time().msec());

         if (days < 0 || secs < 0 || msecs < 0) {
            ret = arg1;

         } else {
            QDateTime dt = a2.addDays(days).addSecs(secs);
            if (msecs > 0) {
               dt.setTime(dt.time().addMSecs(msecs));
            }
            ret = QVariant(dt);
         }
      }
      break;

      default:
         break;
   }

   return ret;
}

/*!
    \internal
    Multiplies \a arg1 by \a multiplier and returns the result.
*/

QVariant operator*(const QVariant &arg1, double multiplier)
{
   QVariant ret;

   switch (arg1.type()) {
      case QVariant::Int:
         ret = QVariant((int)(arg1.toInt() * multiplier));
         break;
      case QVariant::Double:
         ret = QVariant(arg1.toDouble() * multiplier);
         break;
      case QVariant::DateTime: {
         double days = QDATETIMEEDIT_DATE_MIN.daysTo(arg1.toDateTime().date()) * multiplier;
         int daysInt = (int)days;
         days -= daysInt;
         long msecs = (long)((QDATETIMEEDIT_TIME_MIN.msecsTo(arg1.toDateTime().time()) * multiplier)
                             + (days * (24 * 3600 * 1000)));
         ret = QDateTime(QDate().addDays(int(days)), QTime().addMSecs(msecs));
         break;
      }
      default:
         ret = arg1;
         break;
   }

   return ret;
}



double operator/(const QVariant &arg1, const QVariant &arg2)
{
   double a1 = 0;
   double a2 = 0;

   switch (arg1.type()) {
      case QVariant::Int:
         a1 = (double)arg1.toInt();
         a2 = (double)arg2.toInt();
         break;
      case QVariant::Double:
         a1 = arg1.toDouble();
         a2 = arg2.toDouble();
         break;
      case QVariant::DateTime:
         a1 = QDATETIMEEDIT_DATE_MIN.daysTo(arg1.toDate());
         a2 = QDATETIMEEDIT_DATE_MIN.daysTo(arg2.toDate());
         a1 += (double)QDATETIMEEDIT_TIME_MIN.msecsTo(arg1.toDateTime().time()) / (long)(3600 * 24 * 1000);
         a2 += (double)QDATETIMEEDIT_TIME_MIN.msecsTo(arg2.toDateTime().time()) / (long)(3600 * 24 * 1000);
      default:
         break;
   }

   return (a1 != 0 && a2 != 0) ? (a1 / a2) : 0.0;
}

int QAbstractSpinBoxPrivate::variantCompare(const QVariant &arg1, const QVariant &arg2)
{
   switch (arg2.type()) {
      case QVariant::Date:
         Q_ASSERT_X(arg1.type() == QVariant::Date, "QAbstractSpinBoxPrivate::variantCompare",
                    csPrintable(QString("Internal error 1 (%1)").formatArg(arg1.typeName())));

         if (arg1.toDate() == arg2.toDate()) {
            return 0;
         } else if (arg1.toDate() < arg2.toDate()) {
            return -1;
         } else {
            return 1;
         }

      case QVariant::Time:
         Q_ASSERT_X(arg1.type() == QVariant::Time, "QAbstractSpinBoxPrivate::variantCompare",
                    csPrintable(QString("Internal error 2 (%1)").formatArg(arg1.typeName())));

         if (arg1.toTime() == arg2.toTime()) {
            return 0;
         } else if (arg1.toTime() < arg2.toTime()) {
            return -1;
         } else {
            return 1;
         }

      case QVariant::DateTime:
         if (arg1.toDateTime() == arg2.toDateTime()) {
            return 0;
         } else if (arg1.toDateTime() < arg2.toDateTime()) {
            return -1;
         } else {
            return 1;
         }
      case QVariant::Int:
         if (arg1.toInt() == arg2.toInt()) {
            return 0;
         } else if (arg1.toInt() < arg2.toInt()) {
            return -1;
         } else {
            return 1;
         }

      case QVariant::Double:
         if (arg1.toDouble() == arg2.toDouble()) {
            return 0;
         } else if (arg1.toDouble() < arg2.toDouble()) {
            return -1;
         } else {
            return 1;
         }

      case QVariant::Invalid:
         if (arg2.type() == QVariant::Invalid) {
            return 0;
         }

      default:
         Q_ASSERT_X(0, "QAbstractSpinBoxPrivate::variantCompare", csPrintable(QString("Internal error 3 (%1 %2)")
                  .formatArg(arg1.typeName()).formatArg(arg2.typeName())));
   }

   return -2;
}

QVariant QAbstractSpinBoxPrivate::variantBound(const QVariant &min, const QVariant &value, const QVariant &max)
{
   Q_ASSERT(variantCompare(min, max) <= 0);

   if (variantCompare(min, value) < 0) {
      const int compMax = variantCompare(value, max);
      return (compMax < 0 ? value : max);
   } else {
      return min;
   }
}

void QAbstractSpinBox::_q_editorTextChanged(const QString &un_named_arg1)
{
   Q_D(QAbstractSpinBox);
   d->_q_editorTextChanged(un_named_arg1);
}

void QAbstractSpinBox::_q_editorCursorPositionChanged(int un_named_arg1, int un_named_arg2)
{
   Q_D(QAbstractSpinBox);
   d->_q_editorCursorPositionChanged(un_named_arg1, un_named_arg2);
}

#endif // QT_NO_SPINBOX
