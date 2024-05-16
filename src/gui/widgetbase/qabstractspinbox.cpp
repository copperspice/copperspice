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

#include <qabstractspinbox.h>

#include <qplatformdefs.h>

#include <qabstractspinbox_p.h>
#include <qdatetimeparser_p.h>
#include <qdatetime_p.h>
#include <qlineedit_p.h>

#ifndef QT_NO_SPINBOX

#include <qapplication.h>
#include <qclipboard.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>
#include <qdebug.h>
#include <qevent.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qstylehints.h>
#include <qstylepainter.h>

#ifndef QT_NO_ACCESSIBILITY
# include <qaccessible.h>
#endif

QAbstractSpinBox::QAbstractSpinBox(QWidget *parent)
   : QWidget(*new QAbstractSpinBoxPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QAbstractSpinBox);
   d->init();
}

QAbstractSpinBox::QAbstractSpinBox(QAbstractSpinBoxPrivate &dd, QWidget *parent)
   : QWidget(dd, parent, Qt::EmptyFlag)
{
   Q_D(QAbstractSpinBox);
   d->init();
}

QAbstractSpinBox::~QAbstractSpinBox()
{
}

QAbstractSpinBox::ButtonSymbols QAbstractSpinBox::buttonSymbols() const
{
   Q_D(const QAbstractSpinBox);
   return d->buttonSymbols;
}

void QAbstractSpinBox::setButtonSymbols(ButtonSymbols buttonSymbols)
{
   Q_D(QAbstractSpinBox);

   if (d->buttonSymbols != buttonSymbols) {
      d->buttonSymbols = buttonSymbols;
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
   QEvent event(QEvent::ReadOnlyChange);
   QApplication::sendEvent(this, &event);
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

bool QAbstractSpinBox::isGroupSeparatorShown() const
{
   Q_D(const QAbstractSpinBox);
   return d->showGroupSeparator;
}

void QAbstractSpinBox::setGroupSeparatorShown(bool shown)
{
   Q_D(QAbstractSpinBox);

   if (d->showGroupSeparator == shown) {
      return;
   }

   d->showGroupSeparator = shown;
   d->setValue(d->value, EmitIfChanged);
   updateGeometry();
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

void QAbstractSpinBox::selectAll()
{
   Q_D(QAbstractSpinBox);

   if (! d->specialValue()) {
      const int tmp = d->edit->displayText().size() - d->suffix.size();
      d->edit->setSelection(tmp, -(tmp - d->prefix.size()));
   } else {
      d->edit->selectAll();
   }
}

void QAbstractSpinBox::clear()
{
   Q_D(QAbstractSpinBox);

   d->edit->setText(d->prefix + d->suffix);
   d->edit->setCursorPosition(d->prefix.size());
   d->cleared = true;
}

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

QValidator::State QAbstractSpinBox::validate(QString &, int &) const
{
   return QValidator::Acceptable;
}

void QAbstractSpinBox::fixup(QString &) const
{
}

void QAbstractSpinBox::stepUp()
{
   stepBy(1);
}

void QAbstractSpinBox::stepDown()
{
   stepBy(-1);
}

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

QLineEdit *QAbstractSpinBox::lineEdit() const
{
   Q_D(const QAbstractSpinBox);

   return d->edit;
}

void QAbstractSpinBox::setLineEdit(QLineEdit *lineEdit)
{
   Q_D(QAbstractSpinBox);

   if (! lineEdit) {
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
      connect(d->edit, &QLineEdit::textChanged,           this, &QAbstractSpinBox::_q_editorTextChanged);
      connect(d->edit, &QLineEdit::cursorPositionChanged, this, &QAbstractSpinBox::_q_editorCursorPositionChanged);
   }

   d->updateEditFieldGeometry();
   d->edit->setContextMenuPolicy(Qt::NoContextMenu);
   d->edit->d_func()->control->setAccessibleObject(this);

   if (isVisible()) {
      d->edit->show();
   }

   if (isVisible()) {
      d->updateEdit();
   }
}

void QAbstractSpinBox::interpretText()
{
   Q_D(QAbstractSpinBox);
   d->interpret(EmitIfChanged);
}

QVariant QAbstractSpinBox::inputMethodQuery(Qt::InputMethodQuery query) const
{
   Q_D(const QAbstractSpinBox);
   const QVariant lineEditValue = d->edit->inputMethodQuery(query);

   switch (query) {
      case Qt::ImHints:
         if (const int hints = inputMethodHints()) {
            return QVariant(hints | lineEditValue.toInt());
         }

         break;

      default:
         break;
   }

   return lineEditValue;
}

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
         d->updateHoverControl(static_cast<const QHoverEvent *>(event)->pos());
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

void QAbstractSpinBox::changeEvent(QEvent *event)
{
   Q_D(QAbstractSpinBox);

   switch (event->type()) {
      case QEvent::StyleChange:
         d->spinClickTimerInterval = style()->styleHint(QStyle::SH_SpinBox_ClickAutoRepeatRate, nullptr, this);
         d->spinClickThresholdTimerInterval = style()->styleHint(QStyle::SH_SpinBox_ClickAutoRepeatThreshold, nullptr, this);
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

            if (d->pendingEmit) {
               // pendingEmit can be true even if it has npt changed.
               d->interpret(EmitIfChanged);   // E.g. 10 to 10.0
            }
         }

         break;

      default:
         break;
   }

   QWidget::changeEvent(event);
}

void QAbstractSpinBox::resizeEvent(QResizeEvent *event)
{
   Q_D(QAbstractSpinBox);
   QWidget::resizeEvent(event);

   d->updateEditFieldGeometry();
   update();
}

QSize QAbstractSpinBox::sizeHint() const
{
   Q_D(const QAbstractSpinBox);

   if (d->cachedSizeHint.isEmpty()) {
      ensurePolished();

      const QFontMetrics fm(fontMetrics());
      int h = d->edit->sizeHint().height();
      int w = 0;

      QString s;
      QString fixedContent = d->prefix + d->textFromValue(d->minimum) + d->suffix + ' ';

      s = d->textFromValue(d->minimum);
      s.truncate(18);

      s += fixedContent;
      w = qMax(w, fm.width(s));
      s = d->textFromValue(d->maximum);
      s.truncate(18);
      s += fixedContent;
      w = qMax(w, fm.width(s));

      if (d->specialValueText.size()) {
         s = d->specialValueText;
         w = qMax(w, fm.width(s));
      }

      w += 2; // cursor blinking space

      QStyleOptionSpinBox opt;
      initStyleOption(&opt);
      QSize hint(w, h);

      d->cachedSizeHint = style()->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, this)
            .expandedTo(QApplication::globalStrut());
   }

   return d->cachedSizeHint;
}

QSize QAbstractSpinBox::minimumSizeHint() const
{
   Q_D(const QAbstractSpinBox);

   if (d->cachedMinimumSizeHint.isEmpty()) {
      ensurePolished();

      const QFontMetrics fm(fontMetrics());
      int h = d->edit->minimumSizeHint().height();
      int w = 0;

      QString s;
      QString fixedContent = d->prefix + ' ';
      s = d->textFromValue(d->minimum);
      s.truncate(18);
      s += fixedContent;
      w = qMax(w, fm.width(s));
      s = d->textFromValue(d->maximum);
      s.truncate(18);
      s += fixedContent;
      w = qMax(w, fm.width(s));

      if (d->specialValueText.size()) {
         s = d->specialValueText;
         w = qMax(w, fm.width(s));
      }

      w += 2; // cursor blinking space

      QStyleOptionSpinBox opt;
      initStyleOption(&opt);
      QSize hint(w, h);

      d->cachedMinimumSizeHint = style()->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, this)
            .expandedTo(QApplication::globalStrut());
   }

   return d->cachedMinimumSizeHint;
}

void QAbstractSpinBox::paintEvent(QPaintEvent *)
{
   QStyleOptionSpinBox opt;
   initStyleOption(&opt);

   QStylePainter p(this);
   p.drawComplexControl(QStyle::CC_SpinBox, opt);
}

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
         [[fallthrough]];

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

         if (! (stepEnabled() & (up ? StepUpEnabled : StepDownEnabled))) {
            return;
         }

         if (!up) {
            steps *= -1;
         }

         if (style()->styleHint(QStyle::SH_SpinBox_AnimateButton, nullptr, this)) {
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
         QAccessibleValueChangeEvent event(this, d->value);
         QAccessible::updateAccessibility(&event);
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

      case Qt::Key_U:
         if (event->modifiers() & Qt::ControlModifier && QGuiApplication::platformName() == "xcb") {
            // only X11
            event->accept();

            if (!isReadOnly()) {
               clear();
            }

            return;
         }

         break;

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

   if (! d->edit->text().isEmpty()) {
      d->cleared = false;
   }

   if (! isVisible()) {
      d->ignoreUpdateEdit = true;
   }
}

void QAbstractSpinBox::keyReleaseEvent(QKeyEvent *event)
{
   Q_D(QAbstractSpinBox);

   if (d->buttonState & Keyboard && !event->isAutoRepeat())  {
      d->reset();
   } else {
      d->edit->event(event);
   }
}

#ifndef QT_NO_WHEELEVENT
void QAbstractSpinBox::wheelEvent(QWheelEvent *event)
{
   Q_D(QAbstractSpinBox);
   d->wheelDeltaRemainder += event->angleDelta().y();
   const int steps = d->wheelDeltaRemainder / 120;
   d->wheelDeltaRemainder -= steps * 120;

   if (stepEnabled() & (steps > 0 ? StepUpEnabled : StepDownEnabled)) {
      stepBy(event->modifiers() & Qt::ControlModifier ? steps * 10 : steps);
   }

   event->accept();
}
#endif

void QAbstractSpinBox::focusInEvent(QFocusEvent *event)
{
   Q_D(QAbstractSpinBox);

   d->edit->event(event);

   if (event->reason() == Qt::TabFocusReason || event->reason() == Qt::BacktabFocusReason) {
      selectAll();
   }

   QWidget::focusInEvent(event);
}

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
   if (! QApplication::keypadNavigationEnabled())
#endif

      emit editingFinished();
}

void QAbstractSpinBox::closeEvent(QCloseEvent *event)
{
   Q_D(QAbstractSpinBox);

   d->reset();

   if (d->pendingEmit) {
      d->interpret(EmitIfChanged);
   }

   QWidget::closeEvent(event);
}

void QAbstractSpinBox::hideEvent(QHideEvent *event)
{
   Q_D(QAbstractSpinBox);

   d->reset();

   if (d->pendingEmit) {
      d->interpret(EmitIfChanged);
   }

   QWidget::hideEvent(event);
}

void QAbstractSpinBox::timerEvent(QTimerEvent *event)
{
   Q_D(QAbstractSpinBox);

   bool doStep = false;

   if (event->timerId() == d->spinClickThresholdTimerId) {
      killTimer(d->spinClickThresholdTimerId);
      d->spinClickThresholdTimerId = -1;

      d->effectiveSpinRepeatRate = d->buttonState & Keyboard
            ? QGuiApplication::styleHints()->keyboardAutoRepeatRate()
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
         if (! (st & StepUpEnabled)) {
            d->reset();
         } else {
            stepBy(1);
         }
      } else if (d->buttonState & Down) {
         if (! (st & StepDownEnabled)) {
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

void QAbstractSpinBox::contextMenuEvent(QContextMenuEvent *event)
{
#ifdef QT_NO_CONTEXTMENU
   (void) event;
#else
   Q_D(QAbstractSpinBox);

   QPointer<QMenu> menu = d->edit->createStandardContextMenu();

   if (! menu) {
      return;
   }

   d->reset();

   QAction *selAll = new QAction(tr("&Select All"), menu);
   menu->insertAction(d->edit->d_func()->selectAllAction, selAll);
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
#endif
}

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

void QAbstractSpinBox::mouseReleaseEvent(QMouseEvent *event)
{
   Q_D(QAbstractSpinBox);

   if ((d->buttonState & Mouse) != 0) {
      d->reset();
   }

   event->accept();
}

QAbstractSpinBoxPrivate::QAbstractSpinBoxPrivate()
   : edit(nullptr), type(QVariant::Invalid), spinClickTimerId(-1), spinClickTimerInterval(100),
     spinClickThresholdTimerId(-1), spinClickThresholdTimerInterval(-1), effectiveSpinRepeatRate(1),
     buttonState(None), cachedText("\x01"), cachedState(QValidator::Invalid), pendingEmit(false),
     readOnly(false), wrapping(false), ignoreCursorPositionChanged(false), frame(true), accelerate(false),
     keyboardTracking(true), cleared(false), ignoreUpdateEdit(false),
     correctionMode(QAbstractSpinBox::CorrectToPreviousValue), acceleration(0),
     hoverControl(QStyle::SC_None), buttonSymbols(QAbstractSpinBox::UpDownArrows), validator(nullptr),
     showGroupSeparator(0), wheelDeltaRemainder(0)
{
}

QAbstractSpinBoxPrivate::~QAbstractSpinBoxPrivate()
{
}

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

bool QAbstractSpinBoxPrivate::specialValue() const
{
   return (value == minimum && !specialValueText.isEmpty());
}

void QAbstractSpinBoxPrivate::emitSignals(EmitPolicy, const QVariant &)
{
}

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

void QAbstractSpinBoxPrivate::_q_editorCursorPositionChanged(int oldpos, int newpos)
{
   if (! edit->hasSelectedText() && ! ignoreCursorPositionChanged && !specialValue()) {
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
               ? (edit->selectedText().size() * (newpos < pos ? -1 : 1)) - newpos + pos : 0;

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

void QAbstractSpinBoxPrivate::init()
{
   Q_Q(QAbstractSpinBox);

   q->setLineEdit(new QLineEdit(q));
   edit->setObjectName("qt_spinbox_lineedit");
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

void QAbstractSpinBoxPrivate::updateState(bool up, bool fromKeyboard)
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
      QAccessibleValueChangeEvent event(q, value);
      QAccessible::updateAccessibility(&event);
#endif
   }
}

void QAbstractSpinBox::initStyleOption(QStyleOptionSpinBox *option) const
{
   if (! option) {
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
         ? stepEnabled() : (QAbstractSpinBox::StepDownEnabled | QAbstractSpinBox::StepUpEnabled);

   option->frame = d->frame;
}

QVariant QAbstractSpinBoxPrivate::bound(const QVariant &val, const QVariant &old, int steps) const
{
   QVariant v = val;

   if (! wrapping || steps == 0 || ! old.isValid()) {
      if (variantCompare(v, minimum) < 0) {
         v = wrapping ? maximum : minimum;
      }

      if (variantCompare(v, maximum) > 0) {
         v = wrapping ? minimum : maximum;
      }

   } else {
      const bool wasMin  = old == minimum;
      const bool wasMax  = old == maximum;

      const int oldcmp   = variantCompare(v, old);
      const int maxcmp   = variantCompare(v, maximum);
      const int mincmp   = variantCompare(v, minimum);
      const bool wrapped = (oldcmp > 0 && steps < 0) || (oldcmp < 0 && steps > 0);

      if (maxcmp > 0) {
         v = ((wasMax && !wrapped && steps > 0) || (steps < 0 && !wasMin && wrapped)) ? minimum : maximum;
      } else if (wrapped && (maxcmp > 0 || mincmp < 0)) {
         v = ((wasMax && steps > 0) || (! wasMin && steps < 0)) ? minimum : maximum;
      } else if (mincmp < 0) {
         v = (!wasMax && !wasMin ? minimum : maximum);
      }
   }

   return v;
}

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

   q->update();
   edit->blockSignals(sb);
}

void QAbstractSpinBoxPrivate::setRange(const QVariant &min, const QVariant &max)
{
   Q_Q(QAbstractSpinBox);

   clearCache();
   minimum = min;
   maximum = (variantCompare(min, max) < 0 ? max : min);
   cachedSizeHint = QSize();           // minimumSizeHint doesn't care about min/max
   cachedMinimumSizeHint = QSize();    // minimumSizeHint cares about min/max

   reset();

   if (! (bound(value) == value)) {
      setValue(bound(value), EmitIfChanged);
   } else if (value == minimum && !specialValueText.isEmpty()) {
      updateEdit();
   }

   q->updateGeometry();
}

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

QString QAbstractSpinBoxPrivate::textFromValue(const QVariant &) const
{
   return QString();
}

QVariant QAbstractSpinBoxPrivate::valueFromText(const QString &) const
{
   return QVariant();
}

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

#if defined(CS_SHOW_DEBUG_GUI_WIDGETS)
      qDebug() << "QAbstractSpinBoxPrivate::interpret() text '"
            << edit->displayText()
            << "' >> '" << copy << '\''
            << "' >> '" << tmp << '\'';
#endif

      doInterpret = tmp != copy && (q->validate(tmp, pos) == QValidator::Acceptable);

      if (! doInterpret) {
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

QSpinBoxValidator::QSpinBoxValidator(QAbstractSpinBox *qp, QAbstractSpinBoxPrivate *dp)
   : QValidator(qp), qptr(qp), dptr(dp)
{
   setObjectName("qt_spinboxvalidator");
}

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

void QSpinBoxValidator::fixup(QString &input) const
{
   qptr->fixup(input);
}

QVariant operator+(const QVariant &arg1, const QVariant &arg2)
{
   QVariant ret;

   if (arg1.type() != arg2.type()) {
      qWarning("QAbstractSpinBox: Variant data types do not match (%s vs %s)",
            csPrintable(arg1.typeName()), csPrintable(arg2.typeName()));
   }

   switch (arg1.type()) {
      case QVariant::Int:  {
         const int int1 = arg1.toInt();
         const int int2 = arg2.toInt();

         if (int1 > 0 && (int2 >= INT_MAX - int1)) {
            // The increment overflows
            ret = QVariant(INT_MAX);

         } else if (int1 < 0 && (int2 <= INT_MIN - int1)) {
            // The increment underflows
            ret = QVariant(INT_MIN);

         } else {
            ret = QVariant(int1 + int2);
         }

         break;
      }

      case QVariant::Double:
         ret = QVariant(arg1.toDouble() + arg2.toDouble());
         break;

      case QVariant::DateTime: {
         QDateTime a2 = arg2.toDateTime();
         QDateTime a1 = arg1.toDateTime().addDays(QDATETIME_DATETIME_MIN.daysTo(a2));
         a1.setTime(a1.time().addMSecs(QTime().msecsTo(a2.time())));
         ret = QVariant(a1);
      }
      break;

      default:
         break;
   }

   return ret;
}

QVariant operator-(const QVariant &arg1, const QVariant &arg2)
{
   QVariant ret;

   if (arg1.type() != arg2.type()) {
      qWarning("QAbstractSpinBox: Variant data types do not match (%s vs %s)",
            csPrintable(arg1.typeName()), csPrintable(arg2.typeName()));
   }

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

QVariant operator*(const QVariant &arg1, double multiplier)
{
   QVariant ret;

   switch (arg1.type()) {
      case QVariant::Int:
         ret = static_cast<int>(qBound<double>(INT_MIN, arg1.toInt() * multiplier, INT_MAX));
         break;

      case QVariant::Double:
         ret = QVariant(arg1.toDouble() * multiplier);
         break;

      case QVariant::DateTime: {
         double days = QDATETIME_DATE_MIN.daysTo(arg1.toDateTime().date()) * multiplier;
         int daysInt = (int)days;
         days -= daysInt;

         long msecs = (long)((QDATETIME_TIME_MIN.msecsTo(arg1.toDateTime().time()) * multiplier)
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
         a1 = QDATETIME_DATE_MIN.daysTo(arg1.toDate());
         a2 = QDATETIME_DATE_MIN.daysTo(arg2.toDate());
         a1 += (double)QDATETIME_TIME_MIN.msecsTo(arg1.toDateTime().time()) / (long)(3600 * 24 * 1000);
         a2 += (double)QDATETIME_TIME_MIN.msecsTo(arg2.toDateTime().time()) / (long)(3600 * 24 * 1000);

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

         [[fallthrough]];

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

void QAbstractSpinBox::_q_editorTextChanged(const QString &text)
{
   Q_D(QAbstractSpinBox);
   d->_q_editorTextChanged(text);
}

void QAbstractSpinBox::_q_editorCursorPositionChanged(int oldpos, int newpos)
{
   Q_D(QAbstractSpinBox);
   d->_q_editorCursorPositionChanged(oldpos, newpos);
}

#endif // QT_NO_SPINBOX
