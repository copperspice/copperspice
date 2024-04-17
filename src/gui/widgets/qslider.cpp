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

#include <qslider.h>

#ifndef QT_NO_SLIDER

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

#include <qapplication.h>
#include <qevent.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qabstractslider_p.h>
#include <qdebug.h>

class QSliderPrivate : public QAbstractSliderPrivate
{
   Q_DECLARE_PUBLIC(QSlider)

 public:
   QStyle::SubControl pressedControl;
   int tickInterval;
   QSlider::TickPosition tickPosition;
   int clickOffset;
   void init();
   void resetLayoutItemMargins();
   int pixelPosToRangeValue(int pos) const;
   inline int pick(const QPoint &pt) const;

   QStyle::SubControl newHoverControl(const QPoint &pos);
   bool updateHoverControl(const QPoint &pos);
   QStyle::SubControl hoverControl;
   QRect hoverRect;
};

void QSliderPrivate::init()
{
   Q_Q(QSlider);
   pressedControl = QStyle::SC_None;
   tickInterval = 0;
   tickPosition = QSlider::NoTicks;
   hoverControl = QStyle::SC_None;
   q->setFocusPolicy(Qt::FocusPolicy(q->style()->styleHint(QStyle::SH_Button_FocusPolicy)));

   QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Fixed, QSizePolicy::Slider);
   if (orientation == Qt::Vertical) {
      sp.transpose();
   }

   q->setSizePolicy(sp);
   q->setAttribute(Qt::WA_WState_OwnSizePolicy, false);
   resetLayoutItemMargins();
}

void QSliderPrivate::resetLayoutItemMargins()
{
   Q_Q(QSlider);
   QStyleOptionSlider opt;
   q->initStyleOption(&opt);
   setLayoutItemMargins(QStyle::SE_SliderLayoutItem, &opt);
}

int QSliderPrivate::pixelPosToRangeValue(int pos) const
{
   Q_Q(const QSlider);
   QStyleOptionSlider opt;
   q->initStyleOption(&opt);
   QRect gr = q->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, q);
   QRect sr = q->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, q);
   int sliderMin, sliderMax, sliderLength;

   if (orientation == Qt::Horizontal) {
      sliderLength = sr.width();
      sliderMin = gr.x();
      sliderMax = gr.right() - sliderLength + 1;
   } else {
      sliderLength = sr.height();
      sliderMin = gr.y();
      sliderMax = gr.bottom() - sliderLength + 1;
   }
   return QStyle::sliderValueFromPosition(minimum, maximum, pos - sliderMin,
         sliderMax - sliderMin, opt.upsideDown);
}

inline int QSliderPrivate::pick(const QPoint &pt) const
{
   return orientation == Qt::Horizontal ? pt.x() : pt.y();
}

void QSlider::initStyleOption(QStyleOptionSlider *option) const
{
   if (!option) {
      return;
   }

   Q_D(const QSlider);
   option->initFrom(this);
   option->subControls = QStyle::SC_None;
   option->activeSubControls = QStyle::SC_None;
   option->orientation = d->orientation;
   option->maximum = d->maximum;
   option->minimum = d->minimum;
   option->tickPosition = (QSlider::TickPosition)d->tickPosition;
   option->tickInterval = d->tickInterval;

   option->upsideDown = (d->orientation == Qt::Horizontal) ?
      (d->invertedAppearance != (option->direction == Qt::RightToLeft))
      : (!d->invertedAppearance);

   option->direction = Qt::LeftToRight; // we use the upsideDown option instead
   option->sliderPosition = d->position;
   option->sliderValue = d->value;
   option->singleStep = d->singleStep;
   option->pageStep = d->pageStep;

   if (d->orientation == Qt::Horizontal) {
      option->state |= QStyle::State_Horizontal;
   }
}

bool QSliderPrivate::updateHoverControl(const QPoint &pos)
{
   Q_Q(QSlider);
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

QStyle::SubControl QSliderPrivate::newHoverControl(const QPoint &pos)
{
   Q_Q(QSlider);

   QStyleOptionSlider opt;
   q->initStyleOption(&opt);
   opt.subControls = QStyle::SC_All;
   QRect handleRect = q->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, q);
   QRect grooveRect = q->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, q);
   QRect tickmarksRect = q->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderTickmarks, q);

   if (handleRect.contains(pos)) {
      hoverRect = handleRect;
      hoverControl = QStyle::SC_SliderHandle;
   } else if (grooveRect.contains(pos)) {
      hoverRect = grooveRect;
      hoverControl = QStyle::SC_SliderGroove;
   } else if (tickmarksRect.contains(pos)) {
      hoverRect = tickmarksRect;
      hoverControl = QStyle::SC_SliderTickmarks;
   } else {
      hoverRect = QRect();
      hoverControl = QStyle::SC_None;
   }

   return hoverControl;
}

QSlider::QSlider(QWidget *parent)
   : QAbstractSlider(*new QSliderPrivate, parent)
{
   d_func()->orientation = Qt::Vertical;
   d_func()->init();
}


QSlider::QSlider(Qt::Orientation orientation, QWidget *parent)
   : QAbstractSlider(*new QSliderPrivate, parent)
{
   d_func()->orientation = orientation;
   d_func()->init();
}


QSlider::~QSlider()
{
}


void QSlider::paintEvent(QPaintEvent *)
{
   Q_D(QSlider);
   QPainter p(this);
   QStyleOptionSlider opt;
   initStyleOption(&opt);

   opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
   if (d->tickPosition != NoTicks) {
      opt.subControls |= QStyle::SC_SliderTickmarks;
   }

   if (d->pressedControl) {
      opt.activeSubControls = d->pressedControl;
      opt.state |= QStyle::State_Sunken;
   } else {
      opt.activeSubControls = d->hoverControl;
   }

   style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
}

bool QSlider::event(QEvent *event)
{
   Q_D(QSlider);

   switch (event->type()) {
      case QEvent::HoverEnter:
      case QEvent::HoverLeave:
      case QEvent::HoverMove:
         if (const QHoverEvent *he = static_cast<const QHoverEvent *>(event)) {
            d->updateHoverControl(he->pos());
         }
         break;

      case QEvent::StyleChange:
      case QEvent::MacSizeChange:
         d->resetLayoutItemMargins();
         break;

      default:
         break;
   }

   return QAbstractSlider::event(event);
}

void QSlider::mousePressEvent(QMouseEvent *ev)
{
   Q_D(QSlider);

   if (d->maximum == d->minimum || (ev->buttons() ^ ev->button())) {
      ev->ignore();
      return;
   }

#ifdef QT_KEYPAD_NAVIGATION
   if (QApplication::keypadNavigationEnabled()) {
      setEditFocus(true);
   }
#endif
   ev->accept();
   if ((ev->button() & style()->styleHint(QStyle::SH_Slider_AbsoluteSetButtons)) == ev->button()) {
      QStyleOptionSlider opt;
      initStyleOption(&opt);
      const QRect sliderRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
      const QPoint center = sliderRect.center() - sliderRect.topLeft();
      // to take half of the slider off for the setSliderPosition call we use the center - topLeft

      setSliderPosition(d->pixelPosToRangeValue(d->pick(ev->pos() - center)));
      triggerAction(SliderMove);
      setRepeatAction(SliderNoAction);
      d->pressedControl = QStyle::SC_SliderHandle;
      update();
   } else if ((ev->button() & style()->styleHint(QStyle::SH_Slider_PageSetButtons)) == ev->button()) {
      QStyleOptionSlider opt;
      initStyleOption(&opt);
      d->pressedControl = style()->hitTestComplexControl(QStyle::CC_Slider,
            &opt, ev->pos(), this);
      SliderAction action = SliderNoAction;
      if (d->pressedControl == QStyle::SC_SliderGroove) {
         const QRect sliderRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
         int pressValue = d->pixelPosToRangeValue(d->pick(ev->pos() - sliderRect.center() + sliderRect.topLeft()));
         d->pressValue = pressValue;
         if (pressValue > d->value) {
            action = SliderPageStepAdd;
         } else if (pressValue < d->value) {
            action = SliderPageStepSub;
         }
         if (action) {
            triggerAction(action);
            setRepeatAction(action);
         }
      }
   } else {
      ev->ignore();
      return;
   }

   if (d->pressedControl == QStyle::SC_SliderHandle) {
      QStyleOptionSlider opt;
      initStyleOption(&opt);
      setRepeatAction(SliderNoAction);
      QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
      d->clickOffset = d->pick(ev->pos() - sr.topLeft());
      update(sr);
      setSliderDown(true);
   }
}

void QSlider::mouseMoveEvent(QMouseEvent *ev)
{
   Q_D(QSlider);

   if (d->pressedControl != QStyle::SC_SliderHandle) {
      ev->ignore();
      return;
   }

   ev->accept();
   int newPosition = d->pixelPosToRangeValue(d->pick(ev->pos()) - d->clickOffset);
   QStyleOptionSlider opt;
   initStyleOption(&opt);
   setSliderPosition(newPosition);
}

void QSlider::mouseReleaseEvent(QMouseEvent *ev)
{
   Q_D(QSlider);

   if (d->pressedControl == QStyle::SC_None || ev->buttons()) {
      ev->ignore();
      return;
   }

   ev->accept();
   QStyle::SubControl oldPressed = QStyle::SubControl(d->pressedControl);
   d->pressedControl = QStyle::SC_None;
   setRepeatAction(SliderNoAction);

   if (oldPressed == QStyle::SC_SliderHandle) {
      setSliderDown(false);
   }

   QStyleOptionSlider opt;
   initStyleOption(&opt);
   opt.subControls = oldPressed;
   update(style()->subControlRect(QStyle::CC_Slider, &opt, oldPressed, this));
}

QSize QSlider::sizeHint() const
{
   Q_D(const QSlider);

   ensurePolished();
   const int SliderLength = 84, TickSpace = 5;

   QStyleOptionSlider opt;
   initStyleOption(&opt);

   int thick = style()->pixelMetric(QStyle::PM_SliderThickness, &opt, this);

   if (d->tickPosition & TicksAbove) {
      thick += TickSpace;
   }
   if (d->tickPosition & TicksBelow) {
      thick += TickSpace;
   }

   int w = thick, h = SliderLength;
   if (d->orientation == Qt::Horizontal) {
      w = SliderLength;
      h = thick;
   }
   return style()->sizeFromContents(QStyle::CT_Slider, &opt, QSize(w, h), this).expandedTo(QApplication::globalStrut());
}

QSize QSlider::minimumSizeHint() const
{
   Q_D(const QSlider);

   QSize s = sizeHint();
   QStyleOptionSlider opt;
   initStyleOption(&opt);
   int length = style()->pixelMetric(QStyle::PM_SliderLength, &opt, this);

   if (d->orientation == Qt::Horizontal) {
      s.setWidth(length);
   } else {
      s.setHeight(length);
   }

   return s;
}

void QSlider::setTickPosition(TickPosition position)
{
   Q_D(QSlider);
   d->tickPosition = position;
   d->resetLayoutItemMargins();
   update();
   updateGeometry();
}

QSlider::TickPosition QSlider::tickPosition() const
{
   return d_func()->tickPosition;
}

void QSlider::setTickInterval(int ts)
{
   d_func()->tickInterval = qMax(0, ts);
   update();
}

int QSlider::tickInterval() const
{
   return d_func()->tickInterval;
}

Q_GUI_EXPORT QStyleOptionSlider qt_qsliderStyleOption(QSlider *slider)
{
   QStyleOptionSlider sliderOption;
   slider->initStyleOption(&sliderOption);
   return sliderOption;
}

#endif


