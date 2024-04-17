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

#include <qapplication.h>
#include <qcursor.h>
#include <qevent.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qmenu.h>
#include <qelapsedtimer.h>

#ifndef QT_NO_SCROLLBAR

#include <qscrollbar_p.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

#include <limits.h>

bool QScrollBarPrivate::updateHoverControl(const QPoint &pos)
{
   Q_Q(QScrollBar);

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

QStyle::SubControl QScrollBarPrivate::newHoverControl(const QPoint &pos)
{
   Q_Q(QScrollBar);
   QStyleOptionSlider opt;
   q->initStyleOption(&opt);
   opt.subControls = QStyle::SC_All;
   hoverControl = q->style()->hitTestComplexControl(QStyle::CC_ScrollBar, &opt, pos, q);

   if (hoverControl == QStyle::SC_None) {
      hoverRect = QRect();
   } else {
      hoverRect = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt, hoverControl, q);
   }

   return hoverControl;
}

void QScrollBarPrivate::setTransient(bool value)
{
   Q_Q(QScrollBar);
   if (transient != value) {
      transient = value;
      if (q->isVisible()) {
         if (q->style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, q)) {
            q->update();
         }
      } else if (!transient) {
         q->show();
      }
   }
}

void QScrollBarPrivate::flash()
{
   Q_Q(QScrollBar);
   if (!flashed && q->style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, q)) {
      flashed = true;
      if (!q->isVisible()) {
         q->show();
      } else {
         q->update();
      }
   }

   if (!flashTimer) {
      flashTimer = q->startTimer(0);
   }
}

void QScrollBarPrivate::activateControl(uint control, int threshold)
{
   QAbstractSlider::SliderAction action = QAbstractSlider::SliderNoAction;
   switch (control) {
      case QStyle::SC_ScrollBarAddPage:
         action = QAbstractSlider::SliderPageStepAdd;
         break;

      case QStyle::SC_ScrollBarSubPage:
         action = QAbstractSlider::SliderPageStepSub;
         break;

      case QStyle::SC_ScrollBarAddLine:
         action = QAbstractSlider::SliderSingleStepAdd;
         break;

      case QStyle::SC_ScrollBarSubLine:
         action = QAbstractSlider::SliderSingleStepSub;
         break;

      case QStyle::SC_ScrollBarFirst:
         action = QAbstractSlider::SliderToMinimum;
         break;

      case QStyle::SC_ScrollBarLast:
         action = QAbstractSlider::SliderToMaximum;
         break;

      default:
         break;
   }

   if (action) {
      q_func()->setRepeatAction(action, threshold);
      q_func()->triggerAction(action);
   }
}

void QScrollBarPrivate::stopRepeatAction()
{
   Q_Q(QScrollBar);
   QStyle::SubControl tmp = pressedControl;
   q->setRepeatAction(QAbstractSlider::SliderNoAction);
   pressedControl = QStyle::SC_None;

   if (tmp == QStyle::SC_ScrollBarSlider) {
      q->setSliderDown(false);
   }

   QStyleOptionSlider opt;
   q->initStyleOption(&opt);
   q->repaint(q->style()->subControlRect(QStyle::CC_ScrollBar, &opt, tmp, q));
}

void QScrollBar::initStyleOption(QStyleOptionSlider *option) const
{
   if (!option) {
      return;
   }

   Q_D(const QScrollBar);
   option->initFrom(this);
   option->subControls = QStyle::SC_None;
   option->activeSubControls = QStyle::SC_None;
   option->orientation = d->orientation;
   option->minimum = d->minimum;
   option->maximum = d->maximum;
   option->sliderPosition = d->position;
   option->sliderValue = d->value;
   option->singleStep = d->singleStep;
   option->pageStep = d->pageStep;
   option->upsideDown = d->invertedAppearance;
   if (d->orientation == Qt::Horizontal) {
      option->state |= QStyle::State_Horizontal;
   }

   if ((d->flashed || !d->transient) && style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, this)) {
      option->state |= QStyle::State_On;
   }
}

#define HORIZONTAL (d_func()->orientation == Qt::Horizontal)
#define VERTICAL !HORIZONTAL

QScrollBar::QScrollBar(QWidget *parent)
   : QAbstractSlider(*new QScrollBarPrivate, parent)
{
   d_func()->orientation = Qt::Vertical;
   d_func()->init();
}

QScrollBar::QScrollBar(Qt::Orientation orientation, QWidget *parent)
   : QAbstractSlider(*new QScrollBarPrivate, parent)
{
   d_func()->orientation = orientation;
   d_func()->init();
}

QScrollBar::~QScrollBar()
{
}

void QScrollBarPrivate::init()
{
   Q_Q(QScrollBar);
   invertedControls = true;
   pressedControl = hoverControl = QStyle::SC_None;
   pointerOutsidePressedControl = false;
   transient = q->style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, q);
   flashed = false;
   flashTimer = 0;
   q->setFocusPolicy(Qt::NoFocus);

   QSizePolicy sp(QSizePolicy::Minimum, QSizePolicy::Fixed, QSizePolicy::Slider);

   if (orientation == Qt::Vertical) {
      sp.transpose();
   }

   q->setSizePolicy(sp);
   q->setAttribute(Qt::WA_WState_OwnSizePolicy, false);
   q->setAttribute(Qt::WA_OpaquePaintEvent);
}

#ifndef QT_NO_CONTEXTMENU

void QScrollBar::contextMenuEvent(QContextMenuEvent *event)
{
   if (! style()->styleHint(QStyle::SH_ScrollBar_ContextMenu, nullptr, this)) {
      QAbstractSlider::contextMenuEvent(event);
      return;
   }

#ifndef QT_NO_MENU
   bool horiz = HORIZONTAL;
   QPointer<QMenu> menu = new QMenu(this);
   QAction *actScrollHere = menu->addAction(tr("Scroll here"));

   menu->addSeparator();
   QAction *actScrollTop    =  menu->addAction(horiz ? tr("Left edge") : tr("Top"));
   QAction *actScrollBottom = menu->addAction(horiz ? tr("Right edge") : tr("Bottom"));

   menu->addSeparator();
   QAction *actPageUp = menu->addAction(horiz ? tr("Page left") : tr("Page up"));
   QAction *actPageDn = menu->addAction(horiz ? tr("Page right") : tr("Page down"));

   menu->addSeparator();
   QAction *actScrollUp = menu->addAction(horiz ? tr("Scroll left") : tr("Scroll up"));
   QAction *actScrollDn = menu->addAction(horiz ? tr("Scroll right") : tr("Scroll down"));
   QAction *actionSelected = menu->exec(event->globalPos());
   delete menu;

   if (actionSelected == nullptr) {
      // do nothing

   } else if (actionSelected == actScrollHere) {
      setValue(d_func()->pixelPosToRangeValue(horiz ? event->pos().x() : event->pos().y()));

   } else if (actionSelected == actScrollTop) {
      triggerAction(QAbstractSlider::SliderToMinimum);

   } else if (actionSelected == actScrollBottom) {
      triggerAction(QAbstractSlider::SliderToMaximum);

   } else if (actionSelected == actPageUp) {
      triggerAction(QAbstractSlider::SliderPageStepSub);

   } else if (actionSelected == actPageDn) {
      triggerAction(QAbstractSlider::SliderPageStepAdd);

   } else if (actionSelected == actScrollUp) {
      triggerAction(QAbstractSlider::SliderSingleStepSub);

   } else if (actionSelected == actScrollDn) {
      triggerAction(QAbstractSlider::SliderSingleStepAdd);
   }
#endif // QT_NO_MENU
}

#endif // QT_NO_CONTEXTMENU

QSize QScrollBar::sizeHint() const
{
   ensurePolished();
   QStyleOptionSlider opt;
   initStyleOption(&opt);

   int scrollBarExtent = style()->pixelMetric(QStyle::PM_ScrollBarExtent, &opt, this);
   int scrollBarSliderMin = style()->pixelMetric(QStyle::PM_ScrollBarSliderMin, &opt, this);
   QSize size;

   if (opt.orientation == Qt::Horizontal) {
      size = QSize(scrollBarExtent * 2 + scrollBarSliderMin, scrollBarExtent);
   } else {
      size = QSize(scrollBarExtent, scrollBarExtent * 2 + scrollBarSliderMin);
   }

   return style()->sizeFromContents(QStyle::CT_ScrollBar, &opt, size, this)
         .expandedTo(QApplication::globalStrut());
}

void QScrollBar::sliderChange(SliderChange change)
{
   QAbstractSlider::sliderChange(change);
}

bool QScrollBar::event(QEvent *event)
{
   Q_D(QScrollBar);

   switch (event->type()) {
      case QEvent::HoverEnter:
      case QEvent::HoverLeave:
      case QEvent::HoverMove:
         if (const QHoverEvent *he = static_cast<const QHoverEvent *>(event)) {
            d_func()->updateHoverControl(he->pos());
         }
         break;

      case QEvent::StyleChange:
         d_func()->setTransient(style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, this));
         break;

      case QEvent::Timer:
         if (static_cast<QTimerEvent *>(event)->timerId() == d->flashTimer) {
            if (d->flashed && style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, this)) {
               d->flashed = false;
               update();
            }
            killTimer(d->flashTimer);
            d->flashTimer = 0;
         }
         break;

      default:
         break;
   }
   return QAbstractSlider::event(event);
}
#ifndef QT_NO_WHEELEVENT
void QScrollBar::wheelEvent(QWheelEvent *event)
{
   event->ignore();

   int delta = event->delta();

   // scrollbar is a special case - in vertical mode it reaches minimum
   // value in the upper position, however QSlider's minimum value is on
   // the bottom. So we need to invert a value, but since the scrollbar is
   // inverted by default, we need to inverse the delta value for the
   // horizontal orientation.

   if (event->orientation() == Qt::Horizontal) {
      delta = -delta;
   }

   Q_D(QScrollBar);
   if (d->scrollByDelta(event->orientation(), event->modifiers(), delta)) {
      event->accept();
   }

   if (event->phase() == Qt::ScrollBegin) {
      d->setTransient(false);
   } else if (event->phase() == Qt::ScrollEnd) {
      d->setTransient(true);
   }
}

#endif

void QScrollBar::paintEvent(QPaintEvent *)
{
   Q_D(QScrollBar);

   QPainter p(this);
   QStyleOptionSlider opt;
   initStyleOption(&opt);
   opt.subControls = QStyle::SC_All;

   if (d->pressedControl) {
      opt.activeSubControls = (QStyle::SubControl)d->pressedControl;
      if (!d->pointerOutsidePressedControl) {
         opt.state |= QStyle::State_Sunken;
      }

   } else {
      opt.activeSubControls = (QStyle::SubControl)d->hoverControl;
   }
   style()->drawComplexControl(QStyle::CC_ScrollBar, &opt, &p, this);
}

void QScrollBar::mousePressEvent(QMouseEvent *e)
{
   Q_D(QScrollBar);

   if (d->repeatActionTimer.isActive()) {
      d->stopRepeatAction();
   }

   bool midButtonAbsPos = style()->styleHint(QStyle::SH_ScrollBar_MiddleClickAbsolutePosition, nullptr, this);
   QStyleOptionSlider opt;
   initStyleOption(&opt);

   if (d->maximum == d->minimum // no range
      || (e->buttons() & (~e->button())) // another button was clicked before
      || !(e->button() == Qt::LeftButton || (midButtonAbsPos && e->button() == Qt::MiddleButton))) {
      return;
   }

   d->pressedControl = style()->hitTestComplexControl(QStyle::CC_ScrollBar, &opt, e->pos(), this);
   d->pointerOutsidePressedControl = false;

   QRect sr = style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSlider, this);
   QPoint click = e->pos();
   QPoint pressValue = click - sr.center() + sr.topLeft();

   d->pressValue = d->orientation == Qt::Horizontal ? d->pixelPosToRangeValue(pressValue.x()) :
      d->pixelPosToRangeValue(pressValue.y());

   if (d->pressedControl == QStyle::SC_ScrollBarSlider) {
      d->clickOffset = HORIZONTAL ? (click.x() - sr.x()) : (click.y() - sr.y());
      d->snapBackPosition = d->position;
   }

   if ((d->pressedControl == QStyle::SC_ScrollBarAddPage
         || d->pressedControl == QStyle::SC_ScrollBarSubPage)
      && ((midButtonAbsPos && e->button() == Qt::MiddleButton)
         || (style()->styleHint(QStyle::SH_ScrollBar_LeftClickAbsolutePosition, &opt, this)
            && e->button() == Qt::LeftButton))) {

      int sliderLength = HORIZONTAL ? sr.width() : sr.height();
      setSliderPosition(d->pixelPosToRangeValue((HORIZONTAL ? e->pos().x()
               : e->pos().y()) - sliderLength / 2));

      d->pressedControl = QStyle::SC_ScrollBarSlider;
      d->clickOffset = sliderLength / 2;
   }
   const int initialDelay = 500; // default threshold

   QElapsedTimer time;
   time.start();

   d->activateControl(d->pressedControl, initialDelay);
   repaint(style()->subControlRect(QStyle::CC_ScrollBar, &opt, d->pressedControl, this));

   if (time.elapsed() >= initialDelay && d->repeatActionTimer.isActive()) {
      // It took more than 500ms (the initial timer delay) to process the repaint(), we
      // therefore need to restart the timer in case we have a pending mouse release event;
      // otherwise we'll get a timer event right before the release event,
      // causing the repeat action to be invoked twice on a single mouse click.
      // 50ms is the default repeat time (see activateControl/setRepeatAction).
      d->repeatActionTimer.start(50, this);
   }

   if (d->pressedControl == QStyle::SC_ScrollBarSlider) {
      setSliderDown(true);
   }
}

void QScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
   Q_D(QScrollBar);

   if (!d->pressedControl) {
      return;
   }

   if (e->buttons() & (~e->button())) {
      // some other button is still pressed
      return;
   }

   d->stopRepeatAction();
}

void QScrollBar::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QScrollBar);

   if (! d->pressedControl) {
      return;
   }

   QStyleOptionSlider opt;
   initStyleOption(&opt);

   if (! (e->buttons() & Qt::LeftButton ||  ((e->buttons() & Qt::MiddleButton)
            && style()->styleHint(QStyle::SH_ScrollBar_MiddleClickAbsolutePosition, &opt, this)))) {
      return;
   }

   if (d->pressedControl == QStyle::SC_ScrollBarSlider) {
      QPoint click = e->pos();
      int newPosition = d->pixelPosToRangeValue((HORIZONTAL ? click.x() : click.y()) - d->clickOffset);
      int m = style()->pixelMetric(QStyle::PM_MaximumDragDistance, &opt, this);

      if (m >= 0) {
         QRect r = rect();
         r.adjust(-m, -m, m, m);
         if (! r.contains(e->pos())) {
            newPosition = d->snapBackPosition;
         }
      }

      setSliderPosition(newPosition);

   } else if (!style()->styleHint(QStyle::SH_ScrollBar_ScrollWhenPointerLeavesControl, &opt, this)) {

      if (style()->styleHint(QStyle::SH_ScrollBar_RollBetweenButtons, &opt, this)
         && d->pressedControl & (QStyle::SC_ScrollBarAddLine | QStyle::SC_ScrollBarSubLine)) {
         QStyle::SubControl newSc = style()->hitTestComplexControl(QStyle::CC_ScrollBar, &opt, e->pos(), this);

         if (newSc == d->pressedControl && !d->pointerOutsidePressedControl) {
            return;   // nothing to do
         }

         if (newSc & (QStyle::SC_ScrollBarAddLine | QStyle::SC_ScrollBarSubLine)) {
            d->pointerOutsidePressedControl = false;
            QRect scRect = style()->subControlRect(QStyle::CC_ScrollBar, &opt, newSc, this);
            scRect |= style()->subControlRect(QStyle::CC_ScrollBar, &opt, d->pressedControl, this);
            d->pressedControl = newSc;
            d->activateControl(d->pressedControl, 0);
            update(scRect);
            return;
         }
      }

      // stop scrolling when the mouse pointer leaves a control
      // similar to push buttons
      QRect pr = style()->subControlRect(QStyle::CC_ScrollBar, &opt, d->pressedControl, this);
      if (pr.contains(e->pos()) == d->pointerOutsidePressedControl) {
         if ((d->pointerOutsidePressedControl = !d->pointerOutsidePressedControl)) {
            d->pointerOutsidePressedControl = true;
            setRepeatAction(SliderNoAction);
            repaint(pr);
         } else  {
            d->activateControl(d->pressedControl);
         }
      }
   }
}

int QScrollBarPrivate::pixelPosToRangeValue(int pos) const
{
   Q_Q(const QScrollBar);

   QStyleOptionSlider opt;
   q->initStyleOption(&opt);

   QRect gr = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarGroove, q);
   QRect sr = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSlider, q);
   int sliderMin, sliderMax, sliderLength;

   if (orientation == Qt::Horizontal) {
      sliderLength = sr.width();
      sliderMin = gr.x();
      sliderMax = gr.right() - sliderLength + 1;

      if (q->layoutDirection() == Qt::RightToLeft) {
         opt.upsideDown = !opt.upsideDown;
      }

   } else {
      sliderLength = sr.height();
      sliderMin = gr.y();
      sliderMax = gr.bottom() - sliderLength + 1;
   }

   return  QStyle::sliderValueFromPosition(minimum, maximum, pos - sliderMin,
         sliderMax - sliderMin, opt.upsideDown);
}

void QScrollBar::hideEvent(QHideEvent *)
{
   Q_D(QScrollBar);

   if (d->pressedControl) {
      d->pressedControl = QStyle::SC_None;
      setRepeatAction(SliderNoAction);
   }
}

Q_GUI_EXPORT QStyleOptionSlider qt_qscrollbarStyleOption(QScrollBar *scrollbar)
{
   QStyleOptionSlider opt;
   scrollbar->initStyleOption(&opt);
   return opt;
}

#endif // QT_NO_SCROLLBAR
