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

#include <qabstractslider.h>

#include <qapplication.h>
#include <qdebug.h>
#include <qevent.h>

#include <qabstractslider_p.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

#include <limits.h>

QAbstractSliderPrivate::QAbstractSliderPrivate()
   : minimum(0), maximum(99), pageStep(10), value(0), position(0), pressValue(-1),
     singleStep(1), offset_accumulated(0), tracking(true),
     blocktracking(false), pressed(false),
     invertedAppearance(false), invertedControls(false),
     orientation(Qt::Horizontal), repeatAction(QAbstractSlider::SliderNoAction)

#ifdef QT_KEYPAD_NAVIGATION
   , isAutoRepeating(false), repeatMultiplier(1)
{
   firstRepeat.invalidate();

#else
{

#endif

}

QAbstractSliderPrivate::~QAbstractSliderPrivate()
{
}

void QAbstractSlider::setRange(int min, int max)
{
   Q_D(QAbstractSlider);
   int oldMin = d->minimum;
   int oldMax = d->maximum;
   d->minimum = min;
   d->maximum = qMax(min, max);

   if (oldMin != d->minimum || oldMax != d->maximum) {
      sliderChange(SliderRangeChange);
      emit rangeChanged(d->minimum, d->maximum);
      setValue(d->value); // re-bound
   }
}

void QAbstractSliderPrivate::setSteps(int single, int page)
{
   Q_Q(QAbstractSlider);
   singleStep = qAbs(single);
   pageStep = qAbs(page);
   q->sliderChange(QAbstractSlider::SliderStepsChange);
}

QAbstractSlider::QAbstractSlider(QWidget *parent)
   : QWidget(*new QAbstractSliderPrivate, parent, Qt::EmptyFlag)
{
}

QAbstractSlider::QAbstractSlider(QAbstractSliderPrivate &dd, QWidget *parent)
   : QWidget(dd, parent, Qt::EmptyFlag)
{
}

QAbstractSlider::~QAbstractSlider()
{
}

void QAbstractSlider::setOrientation(Qt::Orientation orientation)
{
   Q_D(QAbstractSlider);

   if (d->orientation == orientation) {
      return;
   }

   d->orientation = orientation;

   if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
      QSizePolicy sp = sizePolicy();
      sp.transpose();
      setSizePolicy(sp);
      setAttribute(Qt::WA_WState_OwnSizePolicy, false);
   }

   update();
   updateGeometry();
}

Qt::Orientation QAbstractSlider::orientation() const
{
   Q_D(const QAbstractSlider);
   return d->orientation;
}

void QAbstractSlider::setMinimum(int min)
{
   Q_D(QAbstractSlider);
   setRange(min, qMax(d->maximum, min));
}

int QAbstractSlider::minimum() const
{
   Q_D(const QAbstractSlider);
   return d->minimum;
}

void QAbstractSlider::setMaximum(int max)
{
   Q_D(QAbstractSlider);
   setRange(qMin(d->minimum, max), max);
}

int QAbstractSlider::maximum() const
{
   Q_D(const QAbstractSlider);
   return d->maximum;
}

void QAbstractSlider::setSingleStep(int step)
{
   Q_D(QAbstractSlider);

   if (step != d->singleStep) {
      d->setSteps(step, d->pageStep);
   }
}

int QAbstractSlider::singleStep() const
{
   Q_D(const QAbstractSlider);
   return d->singleStep;
}

void QAbstractSlider::setPageStep(int step)
{
   Q_D(QAbstractSlider);

   if (step != d->pageStep) {
      d->setSteps(d->singleStep, step);
   }
}

int QAbstractSlider::pageStep() const
{
   Q_D(const QAbstractSlider);
   return d->pageStep;
}

void QAbstractSlider::setTracking(bool enable)
{
   Q_D(QAbstractSlider);
   d->tracking = enable;
}

bool QAbstractSlider::hasTracking() const
{
   Q_D(const QAbstractSlider);
   return d->tracking;
}

void QAbstractSlider::setSliderDown(bool down)
{
   Q_D(QAbstractSlider);
   bool doEmit = d->pressed != down;

   d->pressed = down;

   if (doEmit) {
      if (down) {
         emit sliderPressed();
      } else {
         emit sliderReleased();
      }
   }

   if (!down && d->position != d->value) {
      triggerAction(SliderMove);
   }
}

bool QAbstractSlider::isSliderDown() const
{
   Q_D(const QAbstractSlider);
   return d->pressed;
}

void QAbstractSlider::setSliderPosition(int position)
{
   Q_D(QAbstractSlider);
   position = d->bound(position);

   if (position == d->position) {
      return;
   }

   d->position = position;

   if (!d->tracking) {
      update();
   }

   if (d->pressed) {
      emit sliderMoved(position);
   }

   if (d->tracking && !d->blocktracking) {
      triggerAction(SliderMove);
   }
}

int QAbstractSlider::sliderPosition() const
{
   Q_D(const QAbstractSlider);
   return d->position;
}

int QAbstractSlider::value() const
{
   Q_D(const QAbstractSlider);
   return d->value;
}

void QAbstractSlider::setValue(int value)
{
   Q_D(QAbstractSlider);
   value = d->bound(value);

   if (d->value == value && d->position == value) {
      return;
   }

   d->value = value;

   if (d->position != value) {
      d->position = value;

      if (d->pressed) {
         emit sliderMoved((d->position = value));
      }
   }

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleValueChangeEvent event(this, d->value);
   QAccessible::updateAccessibility(&event);
#endif

   sliderChange(SliderValueChange);
   emit valueChanged(value);
}

bool QAbstractSlider::invertedAppearance() const
{
   Q_D(const QAbstractSlider);
   return d->invertedAppearance;
}

void QAbstractSlider::setInvertedAppearance(bool invert)
{
   Q_D(QAbstractSlider);
   d->invertedAppearance = invert;
   update();
}

bool QAbstractSlider::invertedControls() const
{
   Q_D(const QAbstractSlider);
   return d->invertedControls;
}

void QAbstractSlider::setInvertedControls(bool invert)
{
   Q_D(QAbstractSlider);
   d->invertedControls = invert;
}

void QAbstractSlider::triggerAction(SliderAction action)
{
   Q_D(QAbstractSlider);
   d->blocktracking = true;

   switch (action) {
      case SliderSingleStepAdd:
         setSliderPosition(d->overflowSafeAdd(d->effectiveSingleStep()));
         break;

      case SliderSingleStepSub:
         setSliderPosition(d->overflowSafeAdd(-d->effectiveSingleStep()));
         break;

      case SliderPageStepAdd:
         setSliderPosition(d->overflowSafeAdd(d->pageStep));
         break;

      case SliderPageStepSub:
         setSliderPosition(d->overflowSafeAdd(-d->pageStep));
         break;

      case SliderToMinimum:
         setSliderPosition(d->minimum);
         break;

      case SliderToMaximum:
         setSliderPosition(d->maximum);
         break;

      case SliderMove:
      case SliderNoAction:
         break;
   };

   emit actionTriggered(action);

   d->blocktracking = false;

   setValue(d->position);
}

void QAbstractSlider::setRepeatAction(SliderAction action, int thresholdTime, int repeatTime)
{
   Q_D(QAbstractSlider);

   if ((d->repeatAction = action) == SliderNoAction) {
      d->repeatActionTimer.stop();
   } else {
      d->repeatActionTime = repeatTime;
      d->repeatActionTimer.start(thresholdTime, this);
   }
}

QAbstractSlider::SliderAction QAbstractSlider::repeatAction() const
{
   Q_D(const QAbstractSlider);
   return d->repeatAction;
}

void QAbstractSlider::timerEvent(QTimerEvent *e)
{
   Q_D(QAbstractSlider);

   if (e->timerId() == d->repeatActionTimer.timerId()) {
      if (d->repeatActionTime) { // was threshold time, use repeat time next time
         d->repeatActionTimer.start(d->repeatActionTime, this);
         d->repeatActionTime = 0;
      }

      if (d->repeatAction == SliderPageStepAdd) {
         d->setAdjustedSliderPosition(d->overflowSafeAdd(d->pageStep));
      } else if (d->repeatAction == SliderPageStepSub) {
         d->setAdjustedSliderPosition(d->overflowSafeAdd(-d->pageStep));
      } else {
         triggerAction(d->repeatAction);
      }
   }
}

void QAbstractSlider::sliderChange(SliderChange)
{
   update();
}

bool QAbstractSliderPrivate::scrollByDelta(Qt::Orientation orientation, Qt::KeyboardModifiers modifiers, int delta)
{
   Q_Q(QAbstractSlider);
   int stepsToScroll = 0;

   // in Qt scrolling to the right gives negative values.
   if (orientation == Qt::Horizontal) {
      delta = -delta;
   }

   qreal offset = qreal(delta) / 120;

   if ((modifiers & Qt::ControlModifier) || (modifiers & Qt::ShiftModifier)) {
      // Scroll one page regardless of delta:
      stepsToScroll = qBound(-pageStep, int(offset * pageStep), pageStep);
      offset_accumulated = 0;

   } else {
      // Calculate how many lines to scroll. Depending on what delta is (and
      // offset), we might end up with a fraction (e.g. scroll 1.3 lines). We can
      // only scroll whole lines, so we keep the reminder until next event.

      qreal stepsToScrollF = offset * effectiveSingleStep();

#ifndef QT_NO_WHEELEVENT
      stepsToScrollF *= QApplication::wheelScrollLines();
#endif

      // Check if wheel changed direction since last event:
      if (offset_accumulated != 0 && (offset / offset_accumulated) < 0) {
         offset_accumulated = 0;
      }

      offset_accumulated += stepsToScrollF;

      // Don't scroll more than one page in any case:
      stepsToScroll = qBound(-pageStep, int(offset_accumulated), pageStep);

      offset_accumulated -= int(offset_accumulated);

      if (stepsToScroll == 0) {
         // We moved less than a line, but might still have accumulated partial scroll,
         // unless we already are at one of the ends.
         const float effective_offset = invertedControls ? -offset_accumulated : offset_accumulated;

         if (effective_offset > 0.f && value < maximum) {
            return true;
         }

         if (effective_offset < 0.f && value > minimum) {
            return true;
         }

         offset_accumulated = 0;
         return false;
      }
   }

   if (invertedControls) {
      stepsToScroll = -stepsToScroll;
   }

   int prevValue = value;
   position = bound(overflowSafeAdd(stepsToScroll)); // value will be updated by triggerAction()
   q->triggerAction(QAbstractSlider::SliderMove);

   if (prevValue == value) {
      offset_accumulated = 0;
      return false;
   }

   return true;
}

#ifndef QT_NO_WHEELEVENT
void QAbstractSlider::wheelEvent(QWheelEvent *e)
{
   Q_D(QAbstractSlider);
   e->ignore();
   int delta = e->delta();

   if (d->scrollByDelta(e->orientation(), e->modifiers(), delta)) {
      e->accept();
   }
}
#endif

void QAbstractSlider::keyPressEvent(QKeyEvent *ev)
{
   Q_D(QAbstractSlider);

   SliderAction action = SliderNoAction;

#ifdef QT_KEYPAD_NAVIGATION

   if (ev->isAutoRepeat()) {
      if (! d->firstRepeat.isValid()) {
         d->firstRepeat.start();
      } else if (1 == d->repeatMultiplier) {
         // This is the interval in milli seconds which one key repetition
         // takes.
         const int repeatMSecs = d->firstRepeat.elapsed();

         /**
          * The time it takes to currently navigate the whole slider.
          */
         const qreal currentTimeElapse = (qreal(maximum()) / singleStep()) * repeatMSecs;

         /**
          * This is an arbitrarily determined constant in msecs that
          * specifies how long time it should take to navigate from the
          * start to the end(excluding starting key auto repeat).
          */
         const int SliderRepeatElapse = 2500;

         d->repeatMultiplier = currentTimeElapse / SliderRepeatElapse;
      }

   } else if (d->firstRepeat.isValid()) {
      d->firstRepeat.invalidate();
      d->repeatMultiplier = 1;
   }

#endif

   switch (ev->key()) {
#ifdef QT_KEYPAD_NAVIGATION

      case Qt::Key_Select:
         if (QApplication::keypadNavigationEnabled()) {
            setEditFocus(!hasEditFocus());
         } else {
            ev->ignore();
         }

         break;

      case Qt::Key_Back:
         if (QApplication::keypadNavigationEnabled() && hasEditFocus()) {
            setValue(d->origValue);
            setEditFocus(false);
         } else {
            ev->ignore();
         }

         break;
#endif

      // It seems we need to use invertedAppearance for Left and right, otherwise, things look weird.
      case Qt::Key_Left:

#ifdef QT_KEYPAD_NAVIGATION

         // In QApplication::KeypadNavigationDirectional, we want to change the slider
         // value if there is no left/right navigation possible and if this slider is not
         // inside a tab widget.

         if (QApplication::keypadNavigationEnabled()
               && (! hasEditFocus() && QApplication::navigationMode() == Qt::NavigationModeKeypadTabOrder
                     || d->orientation == Qt::Vertical || ! hasEditFocus()
                     && (QWidgetPrivate::canKeypadNavigate(Qt::Horizontal) || QWidgetPrivate::inTabWidget(this)))) {

            ev->ignore();
            return;
         }

         if (QApplication::keypadNavigationEnabled() && d->orientation == Qt::Vertical) {
            action = d->invertedControls ? SliderSingleStepSub : SliderSingleStepAdd;
         } else
#endif
            if (isRightToLeft()) {
               action = d->invertedAppearance ? SliderSingleStepSub : SliderSingleStepAdd;
            } else {
               action = !d->invertedAppearance ? SliderSingleStepSub : SliderSingleStepAdd;
            }

         break;

      case Qt::Key_Right:
#ifdef QT_KEYPAD_NAVIGATION

         // Same logic as in Qt::Key_Left
         if (QApplication::keypadNavigationEnabled()
               && (! hasEditFocus() && QApplication::navigationMode() == Qt::NavigationModeKeypadTabOrder
               || d->orientation == Qt::Vertical || !hasEditFocus()
               && (QWidgetPrivate::canKeypadNavigate(Qt::Horizontal) || QWidgetPrivate::inTabWidget(this)))) {

            ev->ignore();
            return;
         }

         if (QApplication::keypadNavigationEnabled() && d->orientation == Qt::Vertical) {
            action = d->invertedControls ? SliderSingleStepAdd : SliderSingleStepSub;
         } else
#endif
            if (isRightToLeft()) {
               action = d->invertedAppearance ? SliderSingleStepAdd : SliderSingleStepSub;
            } else {
               action = !d->invertedAppearance ? SliderSingleStepAdd : SliderSingleStepSub;
            }

         break;

      case Qt::Key_Up:
#ifdef QT_KEYPAD_NAVIGATION

         // In QApplication::KeypadNavigationDirectional, we want to change the slider
         // value if there is no up/down navigation possible.
         if (QApplication::keypadNavigationEnabled()
               && (QApplication::navigationMode() == Qt::NavigationModeKeypadTabOrder
               || d->orientation == Qt::Horizontal
               || ! hasEditFocus() && QWidgetPrivate::canKeypadNavigate(Qt::Vertical))) {
            ev->ignore();
            break;
         }

#endif
         action = d->invertedControls ? SliderSingleStepSub : SliderSingleStepAdd;
         break;

      case Qt::Key_Down:
#ifdef QT_KEYPAD_NAVIGATION

         // Same logic as in Qt::Key_Up
         if (QApplication::keypadNavigationEnabled()
               && (QApplication::navigationMode() == Qt::NavigationModeKeypadTabOrder
               || d->orientation == Qt::Horizontal
               || ! hasEditFocus() && QWidgetPrivate::canKeypadNavigate(Qt::Vertical))) {
            ev->ignore();
            break;
         }

#endif
         action = d->invertedControls ? SliderSingleStepAdd : SliderSingleStepSub;
         break;

      case Qt::Key_PageUp:
         action = d->invertedControls ? SliderPageStepSub : SliderPageStepAdd;
         break;

      case Qt::Key_PageDown:
         action = d->invertedControls ? SliderPageStepAdd : SliderPageStepSub;
         break;

      case Qt::Key_Home:
         action = SliderToMinimum;
         break;

      case Qt::Key_End:
         action = SliderToMaximum;
         break;

      default:
         ev->ignore();
         break;
   }

   if (action) {
      triggerAction(action);
   }
}

void QAbstractSlider::changeEvent(QEvent *ev)
{
   Q_D(QAbstractSlider);

   switch (ev->type()) {
      case QEvent::EnabledChange:
         if (! isEnabled()) {
            d->repeatActionTimer.stop();
            setSliderDown(false);
         }

         [[fallthrough]];

      default:
         QWidget::changeEvent(ev);
   }
}

bool QAbstractSlider::event(QEvent *e)
{
#ifdef QT_KEYPAD_NAVIGATION
   Q_D(QAbstractSlider);

   switch (e->type()) {
      case QEvent::FocusIn:
         d->origValue = d->value;
         break;

      default:
         break;
   }

#endif

   return QWidget::event(e);
}
