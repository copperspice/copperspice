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

#include <qprogressbar.h>

#ifndef QT_NO_PROGRESSBAR
#include <qevent.h>
#include <qpainter.h>
#include <qstylepainter.h>
#include <qstyleoption.h>
#include <qwidget_p.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

#include <limits.h>

class QProgressBarPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QProgressBar)

 public:
   QProgressBarPrivate();

   void init();
   void initDefaultFormat();
   inline void resetLayoutItemMargins();

   int minimum;
   int maximum;
   int value;
   Qt::Alignment alignment;
   uint textVisible : 1;
   uint defaultFormat: 1;
   int lastPaintedValue;
   Qt::Orientation orientation;
   bool invertedAppearance;
   QProgressBar::Direction textDirection;
   QString format;

   int bound(int val) const {
      return qMax(minimum - 1, qMin(maximum, val));
   }

   bool repaintRequired() const;
};

QProgressBarPrivate::QProgressBarPrivate()
   : minimum(0), maximum(100), value(-1), alignment(Qt::AlignLeft), textVisible(true),
     defaultFormat(true), lastPaintedValue(-1), orientation(Qt::Horizontal), invertedAppearance(false),
     textDirection(QProgressBar::TopToBottom)
{
   initDefaultFormat();
}

void QProgressBarPrivate::initDefaultFormat()
{
   if (defaultFormat) {
      format = QString("%p") + locale.percent();
   }
}

void QProgressBarPrivate::init()
{
   Q_Q(QProgressBar);
   QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Fixed);
   if (orientation == Qt::Vertical) {
      sp.transpose();
   }
   q->setSizePolicy(sp);
   q->setAttribute(Qt::WA_WState_OwnSizePolicy, false);
   resetLayoutItemMargins();
}

void QProgressBarPrivate::resetLayoutItemMargins()
{
   Q_Q(QProgressBar);
   QStyleOptionProgressBar option;
   q->initStyleOption(&option);
   setLayoutItemMargins(QStyle::SE_ProgressBarLayoutItem, &option);
}

void QProgressBar::initStyleOption(QStyleOptionProgressBar *option) const
{
   if (!option) {
      return;
   }
   Q_D(const QProgressBar);
   option->initFrom(this);

   if (d->orientation == Qt::Horizontal) {
      option->state |= QStyle::State_Horizontal;
   }
   option->minimum = d->minimum;
   option->maximum = d->maximum;
   option->progress = d->value;
   option->textAlignment = d->alignment;
   option->textVisible = d->textVisible;
   option->text = text();

   option->orientation = d->orientation;  // TODO: remove this member from QStyleOptionProgressBar
   option->invertedAppearance = d->invertedAppearance;
   option->bottomToTop = d->textDirection == QProgressBar::BottomToTop;
}

bool QProgressBarPrivate::repaintRequired() const
{
   Q_Q(const QProgressBar);
   if (value == lastPaintedValue) {
      return false;
   }

   const qint64 valueDifference = qAbs(qint64(value) - lastPaintedValue);

   // Check if the text needs to be repainted
   if (value == minimum || value == maximum) {
      return true;
   }

   const qint64 totalSteps = qint64(maximum) - minimum;

   if (textVisible) {
      if ((format.contains(QLatin1String("%v")))) {
         return true;
      }

      if ((format.contains(QLatin1String("%p"))
            && valueDifference >= qAbs(totalSteps / 100))) {
         return true;
      }
   }

   // Check if the bar needs to be repainted
   QStyleOptionProgressBar opt;
   q->initStyleOption(&opt);
   int cw = q->style()->pixelMetric(QStyle::PM_ProgressBarChunkWidth, &opt, q);

   QRect groove  = q->style()->subElementRect(QStyle::SE_ProgressBarGroove, &opt, q);

   // This expression is basically
   // (valueDifference / (maximum - minimum) > cw / groove.width())
   // transformed to avoid integer division.
   int grooveBlock = (q->orientation() == Qt::Horizontal) ? groove.width() : groove.height();

   return (valueDifference * grooveBlock > cw * totalSteps);
}

QProgressBar::QProgressBar(QWidget *parent)
   : QWidget(*(new QProgressBarPrivate), parent, Qt::EmptyFlag)
{
   d_func()->init();
}

QProgressBar::~QProgressBar()
{
}

void QProgressBar::reset()
{
   Q_D(QProgressBar);

   if (d->minimum == INT_MIN) {
      d->value = INT_MIN;
   } else {
      d->value = d->minimum - 1;
   }

   repaint();
}

void QProgressBar::setMinimum(int minimum)
{
   setRange(minimum, qMax(d_func()->maximum, minimum));
}

int QProgressBar::minimum() const
{
   return d_func()->minimum;
}

void QProgressBar::setMaximum(int maximum)
{
   setRange(qMin(d_func()->minimum, maximum), maximum);
}

int QProgressBar::maximum() const
{
   return d_func()->maximum;
}

void QProgressBar::setValue(int value)
{
   Q_D(QProgressBar);

   if (d->value == value || ((value > d->maximum || value < d->minimum)
         && (d->maximum != 0 || d->minimum != 0))) {
      return;
   }

   d->value = value;

   emit valueChanged(value);

#ifndef QT_NO_ACCESSIBILITY
   if (isVisible()) {
      QAccessibleValueChangeEvent event(this, value);
      QAccessible::updateAccessibility(&event);
   }
#endif

   if (d->repaintRequired()) {
      repaint();
   }
}

int QProgressBar::value() const
{
   return d_func()->value;
}

void QProgressBar::setRange(int minimum, int maximum)
{
   Q_D(QProgressBar);

   if (minimum != d->minimum || maximum != d->maximum) {
      d->minimum = minimum;
      d->maximum = qMax(minimum, maximum);

      if (d->value < (qint64(d->minimum) - 1) || d->value > d->maximum) {
         reset();
      } else {
         update();
      }
   }
}

void QProgressBar::setTextVisible(bool visible)
{
   Q_D(QProgressBar);
   if (d->textVisible != visible) {
      d->textVisible = visible;
      repaint();
   }
}

bool QProgressBar::isTextVisible() const
{
   return d_func()->textVisible;
}

void QProgressBar::setAlignment(Qt::Alignment alignment)
{
   if (d_func()->alignment != alignment) {
      d_func()->alignment = alignment;
      repaint();
   }
}

Qt::Alignment QProgressBar::alignment() const
{
   return d_func()->alignment;
}

void QProgressBar::paintEvent(QPaintEvent *)
{
   QStylePainter paint(this);
   QStyleOptionProgressBar opt;
   initStyleOption(&opt);
   paint.drawControl(QStyle::CE_ProgressBar, opt);
   d_func()->lastPaintedValue = d_func()->value;
}

QSize QProgressBar::sizeHint() const
{
   ensurePolished();
   QFontMetrics fm = fontMetrics();
   QStyleOptionProgressBar opt;
   initStyleOption(&opt);
   int cw = style()->pixelMetric(QStyle::PM_ProgressBarChunkWidth, &opt, this);
   QSize size = QSize(qMax(9, cw) * 7 + fm.width(QLatin1Char('0')) * 4, fm.height() + 8);

   if (opt.orientation == Qt::Vertical) {
      size = size.transposed();
   }
   return style()->sizeFromContents(QStyle::CT_ProgressBar, &opt, size, this);
}

QSize QProgressBar::minimumSizeHint() const
{
   QSize size;
   if (orientation() == Qt::Horizontal) {
      size = QSize(sizeHint().width(), fontMetrics().height() + 2);
   } else {
      size = QSize(fontMetrics().height() + 2, sizeHint().height());
   }

   return size;
}

QString QProgressBar::text() const
{
   Q_D(const QProgressBar);
   if ((d->maximum == 0 && d->minimum == 0) || d->value < d->minimum
      || (d->value == INT_MIN && d->minimum == INT_MIN)) {
      return QString();
   }

   qint64 totalSteps = qint64(d->maximum) - d->minimum;

   QString result = d->format;
   QLocale locale = d->locale; // Omit group separators for compatibility with previous versions that were non-localized.
   locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);

   result.replace(QLatin1String("%m"), locale.toString(totalSteps));
   result.replace(QLatin1String("%v"), locale.toString(d->value));

   // If max and min are equal and we get this far, it means that the
   // progress bar has one step and that we are on that step. Return
   // 100% here in order to avoid division by zero further down.
   if (totalSteps == 0) {
      result.replace(QLatin1String("%p"), locale.toString(int(100)));
      return result;
   }

   int progress = (qreal(d->value) - d->minimum) * 100.0 / totalSteps;
   result.replace(QLatin1String("%p"), locale.toString(progress));
   return result;
}

void QProgressBar::setOrientation(Qt::Orientation orientation)
{
   Q_D(QProgressBar);

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

   d->resetLayoutItemMargins();
   update();
   updateGeometry();
}

Qt::Orientation QProgressBar::orientation() const
{
   Q_D(const QProgressBar);
   return d->orientation;
}

void QProgressBar::setInvertedAppearance(bool invert)
{
   Q_D(QProgressBar);
   d->invertedAppearance = invert;
   update();
}

bool QProgressBar::invertedAppearance() const
{
   Q_D(const QProgressBar);
   return d->invertedAppearance;
}

void QProgressBar::setTextDirection(QProgressBar::Direction textDirection)
{
   Q_D(QProgressBar);
   d->textDirection = textDirection;
   update();
}

QProgressBar::Direction QProgressBar::textDirection() const
{
   Q_D(const QProgressBar);
   return d->textDirection;
}

bool QProgressBar::event(QEvent *e)
{
   Q_D(QProgressBar);

   switch (e->type()) {
      case QEvent::StyleChange:
#ifdef Q_OS_DARWIN
      case QEvent::MacSizeChange:
#endif
         d->resetLayoutItemMargins();
         break;

      case QEvent::LocaleChange:
         d->initDefaultFormat();
         break;

      default:
         break;
   }

   return QWidget::event(e);
}

void QProgressBar::setFormat(const QString &format)
{
   Q_D(QProgressBar);
   if (d->format == format) {
      return;
   }

   d->format = format;
   d->defaultFormat = false;
   update();
}

void QProgressBar::resetFormat()
{
   Q_D(QProgressBar);
   d->defaultFormat = true;
   d->initDefaultFormat();
   update();
}

QString QProgressBar::format() const
{
   Q_D(const QProgressBar);
   return d->format;
}

#endif // QT_NO_PROGRESSBAR
