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

#include <qbitmap.h>
#include <qevent.h>
#include <qstylepainter.h>
#include <qrubberband.h>
#include <qtimer.h>

#ifndef QT_NO_RUBBERBAND

#include <qstyle.h>
#include <qstyleoption.h>
#include <qdebug.h>

#include <qwidget_p.h>

//### a rubberband window type would be a more elegant solution
#define RUBBERBAND_WINDOW_TYPE Qt::ToolTip

class QRubberBandPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QRubberBand)

 public:
   QRect rect;
   QRubberBand::Shape shape;
   QRegion clipping;
   void updateMask();
};

void QRubberBand::initStyleOption(QStyleOptionRubberBand *option) const
{
   if (!option) {
      return;
   }
   option->initFrom(this);
   option->shape = d_func()->shape;

#ifndef Q_OS_DARWIN
   option->opaque = true;
#else
   option->opaque = windowFlags() & RUBBERBAND_WINDOW_TYPE;
#endif
}

QRubberBand::QRubberBand(Shape s, QWidget *p)
   : QWidget(*new QRubberBandPrivate, p, (p  && p->windowType() != Qt::Desktop) ? Qt::Widget : RUBBERBAND_WINDOW_TYPE)
{
   Q_D(QRubberBand);
   d->shape = s;
   setAttribute(Qt::WA_TransparentForMouseEvents);
   setAttribute(Qt::WA_NoSystemBackground);

   setAttribute(Qt::WA_WState_ExplicitShowHide);
   setVisible(false);
}


QRubberBand::~QRubberBand()
{
}


QRubberBand::Shape QRubberBand::shape() const
{
   Q_D(const QRubberBand);
   return d->shape;
}

void QRubberBandPrivate::updateMask()
{
   Q_Q(QRubberBand);

   QStyleHintReturnMask mask;
   QStyleOptionRubberBand opt;

   q->initStyleOption(&opt);

   if (q->style()->styleHint(QStyle::SH_RubberBand_Mask, &opt, q, &mask)) {
      q->setMask(mask.region);
   } else {
      q->clearMask();
   }
}

void QRubberBand::paintEvent(QPaintEvent *)
{
   QStylePainter painter(this);
   QStyleOptionRubberBand option;
   initStyleOption(&option);
   painter.drawControl(QStyle::CE_RubberBand, option);
}

void QRubberBand::changeEvent(QEvent *e)
{
   QWidget::changeEvent(e);
   switch (e->type()) {
      case QEvent::ParentChange:
         if (parent()) {
            setWindowFlags(windowFlags() & ~RUBBERBAND_WINDOW_TYPE);
         } else {
            setWindowFlags(windowFlags() | RUBBERBAND_WINDOW_TYPE);
         }
         break;

      default:
         break;
   }

   if (e->type() == QEvent::ZOrderChange) {
      raise();
   }
}

void QRubberBand::showEvent(QShowEvent *e)
{
   raise();
   e->ignore();
}

void QRubberBand::resizeEvent(QResizeEvent *)
{
   Q_D(QRubberBand);
   d->updateMask();
}

void QRubberBand::moveEvent(QMoveEvent *)
{
   Q_D(QRubberBand);
   d->updateMask();
}

void QRubberBand::setGeometry(const QRect &geom)
{
   QWidget::setGeometry(geom);
}

bool QRubberBand::event(QEvent *e)
{
   return QWidget::event(e);
}


#endif // QT_NO_RUBBERBAND
