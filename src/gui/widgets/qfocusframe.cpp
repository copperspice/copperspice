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

#include <qfocusframe.h>
#include <qstyle.h>
#include <qbitmap.h>
#include <qstylepainter.h>
#include <qstyleoption.h>
#include <qdebug.h>

#include <qwidget_p.h>

class QFocusFramePrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QFocusFrame)

   QWidget *widget;
   QWidget *frameParent;
   bool showFrameAboveWidget;

 public:
   QFocusFramePrivate(QObject *q) {
      widget      = nullptr;
      frameParent = nullptr;
      CSInternalEvents::set_m_sendChildEvents(q, false);
      showFrameAboveWidget = false;
   }

   void updateSize();
   void update();
};

void QFocusFramePrivate::update()
{
   Q_Q(QFocusFrame);

   q->setParent(frameParent);
   updateSize();

   if (q->parentWidget()->rect().intersects(q->geometry())) {
      if (showFrameAboveWidget) {
         q->raise();
      } else {
         q->stackUnder(widget);
      }
      q->show();
   } else {
      q->hide();
   }
}

void QFocusFramePrivate::updateSize()
{
   Q_Q(QFocusFrame);

   if (!widget) {
      return;
   }

   int vmargin = q->style()->pixelMetric(QStyle::PM_FocusFrameVMargin),
         hmargin = q->style()->pixelMetric(QStyle::PM_FocusFrameHMargin);

   QPoint pos(widget->x(), widget->y());

   if (q->parentWidget() != widget->parentWidget()) {
      pos = widget->parentWidget()->mapTo(q->parentWidget(), pos);
   }

   QRect geom(pos.x() - hmargin, pos.y() - vmargin,
         widget->width() + (hmargin * 2), widget->height() + (vmargin * 2));

   if (q->geometry() == geom) {
      return;
   }

   q->setGeometry(geom);
   QStyleHintReturnMask mask;
   QStyleOption opt;
   q->initStyleOption(&opt);

   if (q->style()->styleHint(QStyle::SH_FocusFrame_Mask, &opt, q, &mask)) {
      q->setMask(mask.region);
   }
}

void QFocusFrame::initStyleOption(QStyleOption *option) const
{
   if (!option) {
      return;
   }

   option->initFrom(this);
}

QFocusFrame::QFocusFrame(QWidget *parent)
   : QWidget(*new QFocusFramePrivate(this), parent, Qt::EmptyFlag)
{
   setAttribute(Qt::WA_TransparentForMouseEvents);
   setFocusPolicy(Qt::NoFocus);
   setAttribute(Qt::WA_NoChildEventsForParent, true);
   setAttribute(Qt::WA_AcceptDrops, style()->styleHint(QStyle::SH_FocusFrame_AboveWidget, nullptr, this));
}

QFocusFrame::~QFocusFrame()
{
}

void QFocusFrame::setWidget(QWidget *widget)
{
   Q_D(QFocusFrame);

   if (style()->styleHint(QStyle::SH_FocusFrame_AboveWidget, nullptr, this)) {
      d->showFrameAboveWidget = true;
   } else {
      d->showFrameAboveWidget = false;
   }

   if (widget == d->widget) {
      return;
   }
   if (d->widget) {
      // Remove event filters from the widget hierarchy.
      QWidget *p = d->widget;
      do {
         p->removeEventFilter(this);
         if (!d->showFrameAboveWidget || p == d->frameParent) {
            break;
         }
         p = p->parentWidget();
      } while (p);
   }

   if (widget && !widget->isWindow() && widget->parentWidget()->windowType() != Qt::SubWindow) {
      d->widget = widget;
      d->widget->installEventFilter(this);

      QWidget *p = widget->parentWidget();
      QWidget *prev = nullptr;

      if (d->showFrameAboveWidget) {
         // Find the right parent for the focus frame.
         while (p) {
            // Traverse the hirerarchy of the 'widget' for setting event filter.
            // During this if come across toolbar or a top level, use that
            // as the parent for the focus frame. If we find a scroll area
            // use its viewport as the parent.
            bool isScrollArea = false;
            if (p->isWindow() || p->inherits("QToolBar") || (isScrollArea = p->inherits("QAbstractScrollArea"))) {
               d->frameParent = p;
               // The previous one in the hierarchy will be the viewport.
               if (prev && isScrollArea) {
                  d->frameParent = prev;
               }
               break;

            } else {
               p->installEventFilter(this);
               prev = p;
               p = p->parentWidget();
            }
         }
      } else {
         d->frameParent = p;
      }
      d->update();

   } else {
      d->widget = nullptr;
      hide();
   }
}

QWidget *QFocusFrame::widget() const
{
   Q_D(const QFocusFrame);
   return d->widget;
}

void QFocusFrame::paintEvent(QPaintEvent *)
{
   Q_D(QFocusFrame);

   if (!d->widget) {
      return;
   }

   QStylePainter p(this);
   QStyleOption option;

   initStyleOption(&option);
   int vmargin = style()->pixelMetric(QStyle::PM_FocusFrameVMargin);
   int hmargin = style()->pixelMetric(QStyle::PM_FocusFrameHMargin);

   QWidgetPrivate *wd = qt_widget_private(d->widget);
   QRect rect = wd->clipRect().adjusted(0, 0, hmargin * 2, vmargin * 2);

   p.setClipRect(rect);
   p.drawControl(QStyle::CE_FocusFrame, option);
}

bool QFocusFrame::eventFilter(QObject *o, QEvent *e)
{
   Q_D(QFocusFrame);
   if (o == d->widget) {
      switch (e->type()) {
         case QEvent::Move:
         case QEvent::Resize:
            d->updateSize();
            break;

         case QEvent::Hide:
         case QEvent::StyleChange:
            hide();
            break;

         case QEvent::ParentChange:
            if (d->showFrameAboveWidget) {
               QWidget *w = d->widget;
               setWidget(nullptr);
               setWidget(w);
            } else {
               d->update();
            }
            break;

         case QEvent::Show:
            d->update();
            show();
            break;

         case QEvent::PaletteChange:
            setPalette(d->widget->palette());
            break;

         case QEvent::ZOrderChange:
            if (style()->styleHint(QStyle::SH_FocusFrame_AboveWidget, nullptr, this)) {
               raise();
            } else {
               stackUnder(d->widget);
            }
            break;

         case QEvent::Destroy:
            setWidget(nullptr);
            break;

         default:
            break;
      }

   } else if (d->showFrameAboveWidget) {
      // Handle changes in the parent widgets we are monitoring.
      switch (e->type()) {
         case QEvent::Move:
         case QEvent::Resize:
            d->updateSize();
            break;

         case QEvent::ZOrderChange:
            raise();
            break;

         default:
            break;
      }
   }

   return false;
}

bool QFocusFrame::event(QEvent *e)
{
   return QWidget::event(e);
}
