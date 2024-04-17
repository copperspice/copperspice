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

#include <qscrollarea.h>

#ifndef QT_NO_SCROLLAREA

#include <qapplication.h>
#include <qdebug.h>
#include <qlayout.h>
#include <qscrollbar.h>
#include <qstyle.h>
#include <qvariant.h>

#include <qlayoutengine_p.h>
#include <qscrollarea_p.h>

QScrollArea::QScrollArea(QWidget *parent)
   : QAbstractScrollArea(*new QScrollAreaPrivate, parent)
{
   Q_D(QScrollArea);
   d->viewport->setBackgroundRole(QPalette::NoRole);
   d->vbar->setSingleStep(20);
   d->hbar->setSingleStep(20);
   d->layoutChildren();
}

QScrollArea::QScrollArea(QScrollAreaPrivate &dd, QWidget *parent)
   : QAbstractScrollArea(dd, parent)
{
   Q_D(QScrollArea);
   d->viewport->setBackgroundRole(QPalette::NoRole);
   d->vbar->setSingleStep(20);
   d->hbar->setSingleStep(20);
   d->layoutChildren();
}

QScrollArea::~QScrollArea()
{
}

void QScrollAreaPrivate::updateWidgetPosition()
{
   Q_Q(QScrollArea);

   Qt::LayoutDirection dir = q->layoutDirection();
   QRect scrolled = QStyle::visualRect(dir, viewport->rect(), QRect(QPoint(-hbar->value(), -vbar->value()),
            widget->size()));

   QRect aligned = QStyle::alignedRect(dir, alignment, widget->size(), viewport->rect());

   widget->move(widget->width() < viewport->width() ? aligned.x() : scrolled.x(),
      widget->height() < viewport->height() ? aligned.y() : scrolled.y());
}

void QScrollAreaPrivate::updateScrollBars()
{
   Q_Q(QScrollArea);

   if (! widget) {
      return;
   }

   QSize p = viewport->size();
   QSize m = q->maximumViewportSize();

   QSize min = qSmartMinSize(widget);
   QSize max = qSmartMaxSize(widget);

   if (resizable) {
      if ((widget->layout() ? widget->layout()->hasHeightForWidth() : widget->sizePolicy().hasHeightForWidth())) {
         QSize p_hfw = p.expandedTo(min).boundedTo(max);
         int h = widget->heightForWidth( p_hfw.width() );
         min = QSize(p_hfw.width(), qMax(p_hfw.height(), h));
      }
   }

   if ((resizable && m.expandedTo(min) == m && m.boundedTo(max) == m)
      || (!resizable && m.expandedTo(widget->size()) == m)) {
      p = m;   // no scroll bars needed
   }

   if (resizable) {
      widget->resize(p.expandedTo(min).boundedTo(max));
   }
   QSize v = widget->size();

   hbar->setRange(0, v.width() - p.width());
   hbar->setPageStep(p.width());
   vbar->setRange(0, v.height() - p.height());
   vbar->setPageStep(p.height());
   updateWidgetPosition();

}

QWidget *QScrollArea::widget() const
{
   Q_D(const QScrollArea);
   return d->widget;
}

void QScrollArea::setWidget(QWidget *widget)
{
   Q_D(QScrollArea);

   if (widget == d->widget || !widget) {
      return;
   }

   delete d->widget;
   d->widget = nullptr;
   d->hbar->setValue(0);
   d->vbar->setValue(0);

   if (widget->parentWidget() != d->viewport) {
      widget->setParent(d->viewport);
   }

   if (!widget->testAttribute(Qt::WA_Resized)) {
      widget->resize(widget->sizeHint());
   }

   d->widget = widget;
   d->widget->setAutoFillBackground(true);
   widget->installEventFilter(this);
   d->widgetSize = QSize();
   d->updateScrollBars();
   d->widget->show();
}

QWidget *QScrollArea::takeWidget()
{
   Q_D(QScrollArea);

   QWidget *w = d->widget;
   d->widget = nullptr;

   if (w) {
      w->setParent(nullptr);
   }

   return w;
}

bool QScrollArea::event(QEvent *e)
{
   Q_D(QScrollArea);

   if (e->type() == QEvent::StyleChange || e->type() == QEvent::LayoutRequest) {
      d->updateScrollBars();
   }

#ifdef QT_KEYPAD_NAVIGATION
   else if (QApplication::keypadNavigationEnabled()) {
      if (e->type() == QEvent::Show) {
         QApplication::instance()->installEventFilter(this);
      } else if (e->type() == QEvent::Hide) {
         QApplication::instance()->removeEventFilter(this);
      }
   }
#endif

   return QAbstractScrollArea::event(e);
}

bool QScrollArea::eventFilter(QObject *o, QEvent *e)
{
   Q_D(QScrollArea);

#ifdef QT_KEYPAD_NAVIGATION
   if (d->widget && o != d->widget && e->type() == QEvent::FocusIn
         && QApplication::keypadNavigationEnabled()) {

      if (o->isWidgetType()) {
         ensureWidgetVisible(static_cast<QWidget *>(o));
      }
   }
#endif

   if (o == d->widget && e->type() == QEvent::Resize) {
      d->updateScrollBars();
   }

   return QAbstractScrollArea::eventFilter(o, e);
   return false;
}

void QScrollArea::resizeEvent(QResizeEvent *)
{
   Q_D(QScrollArea);
   d->updateScrollBars();

}

void QScrollArea::scrollContentsBy(int, int)
{
   Q_D(QScrollArea);

   if (!d->widget) {
      return;
   }
   d->updateWidgetPosition();
}

bool QScrollArea::widgetResizable() const
{
   Q_D(const QScrollArea);
   return d->resizable;
}

void QScrollArea::setWidgetResizable(bool resizable)
{
   Q_D(QScrollArea);

   d->resizable = resizable;
   updateGeometry();
   d->updateScrollBars();
}

QSize QScrollArea::sizeHint() const
{
   Q_D(const QScrollArea);
   int f = 2 * d->frameWidth;
   QSize sz(f, f);
   int h = fontMetrics().height();

   if (d->widget) {
      if (!d->widgetSize.isValid()) {
         d->widgetSize = d->resizable ? d->widget->sizeHint() : d->widget->size();
      }

      sz += d->widgetSize;

   } else {
      sz += QSize(12 * h, 8 * h);
   }

   if (d->vbarpolicy == Qt::ScrollBarAlwaysOn) {
      sz.setWidth(sz.width() + d->vbar->sizeHint().width());
   }

   if (d->hbarpolicy == Qt::ScrollBarAlwaysOn) {
      sz.setHeight(sz.height() + d->hbar->sizeHint().height());
   }

   return sz.boundedTo(QSize(36 * h, 24 * h));
}

QSize QScrollArea::viewportSizeHint() const
{
   Q_D(const QScrollArea);

   if (d->widget) {
      return d->resizable ? d->widget->sizeHint() : d->widget->size();
   }

   const int h = fontMetrics().height();
   return QSize(6 * h, 4 * h);
}

bool QScrollArea::focusNextPrevChild(bool next)
{
   if (QWidget::focusNextPrevChild(next)) {
      if (QWidget *fw = focusWidget()) {
         ensureWidgetVisible(fw);
      }

      return true;
   }

   return false;
}

void QScrollArea::ensureVisible(int x, int y, int xmargin, int ymargin)
{
   Q_D(QScrollArea);

   int logicalX = QStyle::visualPos(layoutDirection(), d->viewport->rect(), QPoint(x, y)).x();

   if (logicalX - xmargin < d->hbar->value()) {
      d->hbar->setValue(qMax(0, logicalX - xmargin));
   } else if (logicalX > d->hbar->value() + d->viewport->width() - xmargin) {
      d->hbar->setValue(qMin(logicalX - d->viewport->width() + xmargin, d->hbar->maximum()));
   }

   if (y - ymargin < d->vbar->value()) {
      d->vbar->setValue(qMax(0, y - ymargin));
   } else if (y > d->vbar->value() + d->viewport->height() - ymargin) {
      d->vbar->setValue(qMin(y - d->viewport->height() + ymargin, d->vbar->maximum()));
   }
}

void QScrollArea::ensureWidgetVisible(QWidget *childWidget, int xmargin, int ymargin)
{
   Q_D(QScrollArea);

   if (!d->widget->isAncestorOf(childWidget)) {
      return;
   }

   const QRect microFocus = childWidget->inputMethodQuery(Qt::ImCursorRectangle).toRect();
   const QRect defaultMicroFocus = childWidget->QWidget::inputMethodQuery(Qt::ImCursorRectangle).toRect();

   QRect focusRect = (microFocus != defaultMicroFocus)
      ? QRect(childWidget->mapTo(d->widget, microFocus.topLeft()), microFocus.size())
      : QRect(childWidget->mapTo(d->widget, QPoint(0, 0)), childWidget->size());
   const QRect visibleRect(-d->widget->pos(), d->viewport->size());

   if (visibleRect.contains(focusRect)) {
      return;
   }

   focusRect.adjust(-xmargin, -ymargin, xmargin, ymargin);

   if (focusRect.width() > visibleRect.width()) {
      d->hbar->setValue(focusRect.center().x() - d->viewport->width() / 2);
   } else if (focusRect.right() > visibleRect.right()) {
      d->hbar->setValue(focusRect.right() - d->viewport->width());
   } else if (focusRect.left() < visibleRect.left()) {
      d->hbar->setValue(focusRect.left());
   }

   if (focusRect.height() > visibleRect.height()) {
      d->vbar->setValue(focusRect.center().y() - d->viewport->height() / 2);
   } else if (focusRect.bottom() > visibleRect.bottom()) {
      d->vbar->setValue(focusRect.bottom() - d->viewport->height());
   } else if (focusRect.top() < visibleRect.top()) {
      d->vbar->setValue(focusRect.top());
   }
}

void QScrollArea::setAlignment(Qt::Alignment alignment)
{
   Q_D(QScrollArea);

   d->alignment = alignment;
   if (d->widget) {
      d->updateWidgetPosition();
   }
}

Qt::Alignment QScrollArea::alignment() const
{
   Q_D(const QScrollArea);
   return d->alignment;
}

#endif // QT_NO_SCROLLAREA
