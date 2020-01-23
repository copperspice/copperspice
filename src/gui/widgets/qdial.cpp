/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qdial.h>

#ifndef QT_NO_DIAL

#include <qapplication.h>
#include <qbitmap.h>
#include <qcolor.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpolygon.h>
#include <qregion.h>
#include <qstyle.h>
#include <qstylepainter.h>
#include <qstyleoption.h>
#include <qslider.h>
#include <qabstractslider_p.h>
#include <qmath_p.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

#include <qmath.h>

class QDialPrivate : public QAbstractSliderPrivate
{
   Q_DECLARE_PUBLIC(QDial)
 public:
   QDialPrivate() {
      wrapping = false;
      tracking = true;
      doNotEmit = false;
      target = qreal(3.7);
   }

   qreal target;
   uint showNotches : 1;
   uint wrapping : 1;
   uint doNotEmit : 1;

   int valueFromPoint(const QPoint &) const;
   double angle(const QPoint &, const QPoint &) const;
   void init();
   int bound(int val) const override;
};

void QDialPrivate::init()
{
   Q_Q(QDial);
   showNotches = false;
   q->setFocusPolicy(Qt::WheelFocus);
}

int QDialPrivate::bound(int val) const
{
   if (wrapping) {
      if ((val >= minimum) && (val <= maximum)) {
         return val;
      }
      val = minimum + ((val - minimum) % (maximum - minimum));
      if (val < minimum) {
         val += maximum - minimum;
      }
      return val;
   } else {
      return QAbstractSliderPrivate::bound(val);
   }
}

/*!
    Initialize \a option with the values from this QDial. This method
    is useful for subclasses when they need a QStyleOptionSlider, but don't want
    to fill in all the information themselves.

    \sa QStyleOption::initFrom()
*/
void QDial::initStyleOption(QStyleOptionSlider *option) const
{
   if (!option) {
      return;
   }

   Q_D(const QDial);
   option->initFrom(this);
   option->minimum = d->minimum;
   option->maximum = d->maximum;
   option->sliderPosition = d->position;
   option->sliderValue = d->value;
   option->singleStep = d->singleStep;
   option->pageStep = d->pageStep;
   option->upsideDown = !d->invertedAppearance;
   option->notchTarget = d->target;
   option->dialWrapping = d->wrapping;
   option->subControls = QStyle::SC_All;
   option->activeSubControls = QStyle::SC_None;
   if (!d->showNotches) {
      option->subControls &= ~QStyle::SC_DialTickmarks;
      option->tickPosition = QSlider::TicksAbove;
   } else {
      option->tickPosition = QSlider::NoTicks;
   }
   option->tickInterval = notchSize();
}

int QDialPrivate::valueFromPoint(const QPoint &p) const
{
   Q_Q(const QDial);
   double yy = (double)q->height() / 2.0 - p.y();
   double xx = (double)p.x() - q->width() / 2.0;
   double a = (xx || yy) ? std::atan2(yy, xx) : 0;

   if (a < Q_PI / -2) {
      a = a + Q_PI * 2;
   }

   int dist = 0;
   int minv = minimum, maxv = maximum;

   if (minimum < 0) {
      dist = -minimum;
      minv = 0;
      maxv = maximum + dist;
   }

   int r = maxv - minv;
   int v;
   if (wrapping) {
      v =  (int)(0.5 + minv + r * (Q_PI * 3 / 2 - a) / (2 * Q_PI));
   } else {
      v =  (int)(0.5 + minv + r * (Q_PI * 4 / 3 - a) / (Q_PI * 10 / 6));
   }

   if (dist > 0) {
      v -= dist;
   }

   return !invertedAppearance ? bound(v) : maximum - bound(v);
}

QDial::QDial(QWidget *parent)
   : QAbstractSlider(*new QDialPrivate, parent)
{
   Q_D(QDial);
   d->init();
}


QDial::~QDial()
{
}

/*! \reimp */
void QDial::resizeEvent(QResizeEvent *e)
{
   QWidget::resizeEvent(e);
}

/*!
  \reimp
*/

void QDial::paintEvent(QPaintEvent *)
{
   QStylePainter p(this);
   QStyleOptionSlider option;
   initStyleOption(&option);
   p.drawComplexControl(QStyle::CC_Dial, option);
}

/*!
  \reimp
*/

void QDial::mousePressEvent(QMouseEvent *e)
{
   Q_D(QDial);
   if (d->maximum == d->minimum ||
      (e->button() != Qt::LeftButton)  ||
      (e->buttons() ^ e->button())) {
      e->ignore();
      return;
   }
   e->accept();
   setSliderPosition(d->valueFromPoint(e->pos()));
   // ### This isn't quite right,
   // we should be doing a hit test and only setting this if it's
   // the actual dial thingie (similar to what QSlider does), but we have no
   // subControls for QDial.
   setSliderDown(true);
}


/*!
  \reimp
*/

void QDial::mouseReleaseEvent(QMouseEvent *e)
{
   Q_D(QDial);
   if (e->buttons() & (~e->button()) ||
      (e->button() != Qt::LeftButton)) {
      e->ignore();
      return;
   }
   e->accept();
   setValue(d->valueFromPoint(e->pos()));
   setSliderDown(false);
}


/*!
  \reimp
*/

void QDial::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QDial);
   if (!(e->buttons() & Qt::LeftButton)) {
      e->ignore();
      return;
   }
   e->accept();
   d->doNotEmit = true;
   setSliderPosition(d->valueFromPoint(e->pos()));
   d->doNotEmit = false;
}


/*!
    \reimp
*/

void QDial::sliderChange(SliderChange change)
{
   QAbstractSlider::sliderChange(change);
}

void QDial::setWrapping(bool enable)
{
   Q_D(QDial);
   if (d->wrapping == enable) {
      return;
   }
   d->wrapping = enable;
   update();
}


/*!
    \property QDial::wrapping
    \brief whether wrapping is enabled

    If true, wrapping is enabled; otherwise some space is inserted at the bottom
    of the dial to separate the ends of the range of valid values.

    If enabled, the arrow can be oriented at any angle on the dial. If disabled,
    the arrow will be restricted to the upper part of the dial; if it is rotated
    into the space at the bottom of the dial, it will be clamped to the closest
    end of the valid range of values.

    By default this property is false.
*/

bool QDial::wrapping() const
{
   Q_D(const QDial);
   return d->wrapping;
}


/*!
    \property QDial::notchSize
    \brief the current notch size

    The notch size is in range control units, not pixels, and if
    possible it is a multiple of singleStep() that results in an
    on-screen notch size near notchTarget().

    By default, this property has a value of 1.

    \sa notchTarget(), singleStep()
*/

int QDial::notchSize() const
{
   Q_D(const QDial);
   // radius of the arc
   int r = qMin(width(), height()) / 2;
   // length of the whole arc
   int l = (int)(r * (d->wrapping ? 6 : 5) * Q_PI / 6);
   // length of the arc from minValue() to minValue()+pageStep()
   if (d->maximum > d->minimum + d->pageStep) {
      l = (int)(0.5 + l * d->pageStep / (d->maximum - d->minimum));
   }
   // length of a singleStep arc
   l = l * d->singleStep / (d->pageStep ? d->pageStep : 1);
   if (l < 1) {
      l = 1;
   }
   // how many times singleStep can be draw in d->target pixels
   l = (int)(0.5 + d->target / l);
   // we want notchSize() to be a non-zero multiple of lineStep()
   if (!l) {
      l = 1;
   }
   return d->singleStep * l;
}

void QDial::setNotchTarget(double target)
{
   Q_D(QDial);
   d->target = target;
   update();
}

/*!
    \property QDial::notchTarget
    \brief the target number of pixels between notches

    The notch target is the number of pixels QDial attempts to put
    between each notch.

    The actual size may differ from the target size.

    The default notch target is 3.7 pixels.
*/
qreal QDial::notchTarget() const
{
   Q_D(const QDial);
   return d->target;
}


void QDial::setNotchesVisible(bool visible)
{
   Q_D(QDial);
   d->showNotches = visible;
   update();
}

/*!
    \property QDial::notchesVisible
    \brief whether the notches are shown

    If the property is true, a series of notches are drawn around the dial
    to indicate the range of values available; otherwise no notches are
    shown.

    By default, this property is disabled.
*/
bool QDial::notchesVisible() const
{
   Q_D(const QDial);
   return d->showNotches;
}

/*!
  \reimp
*/

QSize QDial::minimumSizeHint() const
{
   return QSize(50, 50);
}

/*!
  \reimp
*/

QSize QDial::sizeHint() const
{
   return QSize(100, 100).expandedTo(QApplication::globalStrut());
}

/*!
  \reimp
*/
bool QDial::event(QEvent *e)
{
   return QAbstractSlider::event(e);
}

#endif // QT_NO_DIAL
