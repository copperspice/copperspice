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

#include <qstatusbar.h>

#ifndef QT_NO_STATUSBAR

#include <qlist.h>
#include <qdebug.h>
#include <qevent.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qsizegrip.h>
#include <qmainwindow.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

#include <qlayoutengine_p.h>
#include <qwidget_p.h>

class QStatusBarPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QStatusBar)

 public:
   QStatusBarPrivate() {}

   struct SBItem {
      SBItem(QWidget *widget, int stretch, bool permanent)
         : s(stretch), w(widget), p(permanent) {}
      int s;
      QWidget *w;
      bool p;
   };

   QList<SBItem *> items;
   QString tempItem;

   QBoxLayout *box;
   QTimer *timer;

#ifndef QT_NO_SIZEGRIP
   QSizeGrip *resizer;
   bool showSizeGrip;
#endif

   int savedStrut;


   int indexToLastNonPermanentWidget() const {
      int i = items.size() - 1;
      for (; i >= 0; --i) {
         SBItem *item = items.at(i);
         if (!(item && item->p)) {
            break;
         }
      }
      return i;
   }

#ifndef QT_NO_SIZEGRIP
   void tryToShowSizeGrip() {
      if (!showSizeGrip) {
         return;
      }

      showSizeGrip = false;
      if (!resizer || resizer->isVisible()) {
         return;
      }

      resizer->setAttribute(Qt::WA_WState_ExplicitShowHide, false);
      QMetaObject::invokeMethod(resizer, "_q_showIfNotHidden", Qt::DirectConnection);
      resizer->setAttribute(Qt::WA_WState_ExplicitShowHide, false);
   }
#endif

   QRect messageRect() const;
};


QRect QStatusBarPrivate::messageRect() const
{
   Q_Q(const QStatusBar);
   bool rtl = q->layoutDirection() == Qt::RightToLeft;

   int left = 6;
   int right = q->width() - 12;

#ifndef QT_NO_SIZEGRIP
   if (resizer && resizer->isVisible()) {
      if (rtl) {
         left = resizer->x() + resizer->width();
      } else {
         right = resizer->x();
      }
   }
#endif

   for (int i = 0; i < items.size(); ++i) {
      QStatusBarPrivate::SBItem *item = items.at(i);
      if (!item) {
         break;
      }
      if (item->p && item->w->isVisible()) {
         if (item->p) {
            if (rtl) {
               left = qMax(left, item->w->x() + item->w->width() + 2);
            } else {
               right = qMin(right, item->w->x() - 2);
            }
         }
         break;
      }
   }
   return QRect(left, 0, right - left, q->height());
}


QStatusBar::QStatusBar(QWidget *parent)
   : QWidget(*new QStatusBarPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QStatusBar);
   d->box   = nullptr;
   d->timer = nullptr;

#ifndef QT_NO_SIZEGRIP
   d->resizer = nullptr;
   setSizeGripEnabled(true);    // causes reformat()
#else
   reformat();
#endif
}

QStatusBar::~QStatusBar()
{
   Q_D(QStatusBar);

   while (!d->items.isEmpty()) {
      delete d->items.takeFirst();
   }
}

void QStatusBar::addWidget(QWidget *widget, int stretch)
{
   if (!widget) {
      return;
   }
   insertWidget(d_func()->indexToLastNonPermanentWidget() + 1, widget, stretch);
}

int QStatusBar::insertWidget(int index, QWidget *widget, int stretch)
{
   if (!widget) {
      return -1;
   }

   Q_D(QStatusBar);
   QStatusBarPrivate::SBItem *item = new QStatusBarPrivate::SBItem(widget, stretch, false);

   int idx = d->indexToLastNonPermanentWidget();

   if (index < 0 || index > d->items.size() || (idx >= 0 && index > idx + 1)) {
      qWarning("QStatusBar::insertWidget() Index (%d) is out of range, appending widget instead", index);
      index = idx + 1;
   }
   d->items.insert(index, item);

   if (!d->tempItem.isEmpty()) {
      widget->hide();
   }

   reformat();
   if (! widget->isHidden() || !widget->testAttribute(Qt::WA_WState_ExplicitShowHide)) {
      widget->show();
   }

   return index;
}

void QStatusBar::addPermanentWidget(QWidget *widget, int stretch)
{
   if (!widget) {
      return;
   }
   insertPermanentWidget(d_func()->items.size(), widget, stretch);
}

int QStatusBar::insertPermanentWidget(int index, QWidget *widget, int stretch)
{
   if (!widget) {
      return -1;
   }

   Q_D(QStatusBar);
   QStatusBarPrivate::SBItem *item = new QStatusBarPrivate::SBItem(widget, stretch, true);

   int idx = d->indexToLastNonPermanentWidget();
   if (index < 0 || index > d->items.size() || (idx >= 0 && index <= idx)) {
      qWarning("QStatusBar::insertPermanentWidget() Index (%d) is out of range, appending widget instead", index);
      index = d->items.size();
   }
   d->items.insert(index, item);

   reformat();
   if (!widget->isHidden() || !widget->testAttribute(Qt::WA_WState_ExplicitShowHide)) {
      widget->show();
   }

   return index;
}

void QStatusBar::removeWidget(QWidget *widget)
{
   if (!widget) {
      return;
   }

   Q_D(QStatusBar);
   bool found = false;
   QStatusBarPrivate::SBItem *item;

   for (int i = 0; i < d->items.size(); ++i) {
      item = d->items.at(i);
      if (!item) {
         break;
      }
      if (item->w == widget) {
         d->items.removeAt(i);
         item->w->hide();
         delete item;
         found = true;
         break;
      }
   }

   if (found) {
      reformat();
   }

#if defined(CS_SHOW_DEBUG_GUI_WIDGETS)
   else {
      qDebug("QStatusBar::removeWidget(): Widget not found.");
   }
#endif
}

bool QStatusBar::isSizeGripEnabled() const
{
#ifdef QT_NO_SIZEGRIP
   return false;
#else
   Q_D(const QStatusBar);
   return !!d->resizer;
#endif
}

void QStatusBar::setSizeGripEnabled(bool enabled)
{
#ifdef QT_NO_SIZEGRIP
   // nothing
#else
   Q_D(QStatusBar);

   if (!enabled != !d->resizer) {
      if (enabled) {
         d->resizer = new QSizeGrip(this);
         d->resizer->hide();
         d->resizer->installEventFilter(this);
         d->showSizeGrip = true;

      } else {
         delete d->resizer;
         d->resizer = nullptr;
         d->showSizeGrip = false;
      }
      reformat();
      if (d->resizer && isVisible()) {
         d->tryToShowSizeGrip();
      }
   }
#endif
}

void QStatusBar::reformat()
{
   Q_D(QStatusBar);

   if (d->box) {
      delete d->box;
   }

   QBoxLayout *vbox;

#ifndef QT_NO_SIZEGRIP
   if (d->resizer) {
      d->box = new QHBoxLayout(this);
      d->box->setMargin(0);
      vbox = new QVBoxLayout;
      d->box->addLayout(vbox);
   } else
#endif
   {
      vbox = d->box = new QVBoxLayout(this);
      d->box->setMargin(0);
   }

   vbox->addSpacing(3);
   QBoxLayout *l = new QHBoxLayout;
   vbox->addLayout(l);
   l->addSpacing(2);
   l->setSpacing(6);

   int maxH = fontMetrics().height();

   int i;
   QStatusBarPrivate::SBItem *item;

   for (i = 0, item = nullptr; i < d->items.size(); ++i) {
      item = d->items.at(i);
      if (!item || item->p) {
         break;
      }
      l->addWidget(item->w, item->s);
      int itemH = qMin(qSmartMinSize(item->w).height(), item->w->maximumHeight());
      maxH = qMax(maxH, itemH);
   }

   l->addStretch(0);

   for (item = nullptr; i < d->items.size(); ++i) {
      item = d->items.at(i);
      if (!item) {
         break;
      }

      l->addWidget(item->w, item->s);
      int itemH = qMin(qSmartMinSize(item->w).height(), item->w->maximumHeight());
      maxH = qMax(maxH, itemH);
   }

#ifndef QT_NO_SIZEGRIP
   if (d->resizer) {
      maxH = qMax(maxH, d->resizer->sizeHint().height());
      d->box->addSpacing(1);
      d->box->addWidget(d->resizer, 0, Qt::AlignBottom);
   }
#endif

   l->addStrut(maxH);
   d->savedStrut = maxH;
   vbox->addSpacing(2);
   d->box->activate();
   update();
}

void QStatusBar::showMessage(const QString &message, int timeout)
{
   Q_D(QStatusBar);

   if (timeout > 0) {
      if (!d->timer) {
         d->timer = new QTimer(this);
         connect(d->timer, &QTimer::timeout, this, &QStatusBar::clearMessage);
      }
      d->timer->start(timeout);

   } else if (d->timer) {
      delete d->timer;
      d->timer = nullptr;
   }

   if (d->tempItem == message) {
      return;
   }
   d->tempItem = message;

   hideOrShow();
}

void QStatusBar::clearMessage()
{
   Q_D(QStatusBar);

   if (d->tempItem.isEmpty()) {
      return;
   }

   if (d->timer) {
      delete d->timer;
      d->timer = nullptr;
   }

   d->tempItem.clear();
   hideOrShow();
}


QString QStatusBar::currentMessage() const
{
   Q_D(const QStatusBar);
   return d->tempItem;
}

void QStatusBar::hideOrShow()
{
   Q_D(QStatusBar);
   bool haveMessage = !d->tempItem.isEmpty();

   QStatusBarPrivate::SBItem *item = nullptr;
   for (int i = 0; i < d->items.size(); ++i) {
      item = d->items.at(i);
      if (!item || item->p) {
         break;
      }
      if (haveMessage && item->w->isVisible()) {
         item->w->hide();
         item->w->setAttribute(Qt::WA_WState_ExplicitShowHide, false);
      } else if (!haveMessage && !item->w->testAttribute(Qt::WA_WState_ExplicitShowHide)) {
         item->w->show();
      }
   }

   emit messageChanged(d->tempItem);

#ifndef QT_NO_ACCESSIBILITY
   if (QAccessible::isActive()) {
      QAccessibleEvent event(this, QAccessible::NameChanged);
      QAccessible::updateAccessibility(&event);
   }
#endif

   repaint(d->messageRect());
}

void QStatusBar::showEvent(QShowEvent *)
{
#ifndef QT_NO_SIZEGRIP
   Q_D(QStatusBar);
   if (d->resizer && d->showSizeGrip) {
      d->tryToShowSizeGrip();
   }
#endif
}

void QStatusBar::paintEvent(QPaintEvent *event)
{
   Q_D(QStatusBar);
   bool haveMessage = !d->tempItem.isEmpty();

   QPainter p(this);
   QStyleOption opt;
   opt.initFrom(this);
   style()->drawPrimitive(QStyle::PE_PanelStatusBar, &opt, &p, this);

   for (int i = 0; i < d->items.size(); ++i) {
      QStatusBarPrivate::SBItem *item = d->items.at(i);
      if (item && item->w->isVisible() && (!haveMessage || item->p)) {
         QRect ir = item->w->geometry().adjusted(-2, -1, 2, 1);
         if (event->rect().intersects(ir)) {
            QStyleOption opt(0);
            opt.rect = ir;
            opt.palette = palette();
            opt.state = QStyle::State_None;
            style()->drawPrimitive(QStyle::PE_FrameStatusBarItem, &opt, &p, item->w);
         }
      }
   }

   if (haveMessage) {
      p.setPen(palette().foreground().color());
      p.drawText(d->messageRect(), Qt::AlignLeading | Qt::AlignVCenter | Qt::TextSingleLine, d->tempItem);
   }
}

void QStatusBar::resizeEvent(QResizeEvent *e)
{
   QWidget::resizeEvent(e);
}

bool QStatusBar::event(QEvent *e)
{
   Q_D(QStatusBar);

   if (e->type() == QEvent::LayoutRequest) {
      // Calculate new strut height and call reformat() if it has changed
      int maxH = fontMetrics().height();

      QStatusBarPrivate::SBItem *item = nullptr;
      for (int i = 0; i < d->items.size(); ++i) {
         item = d->items.at(i);
         if (!item) {
            break;
         }
         int itemH = qMin(qSmartMinSize(item->w).height(), item->w->maximumHeight());
         maxH = qMax(maxH, itemH);
      }

#ifndef QT_NO_SIZEGRIP
      if (d->resizer) {
         maxH = qMax(maxH, d->resizer->sizeHint().height());
      }
#endif

      if (maxH != d->savedStrut) {
         reformat();
      } else {
         update();
      }
   }

   if (e->type() == QEvent::ChildRemoved) {
      QStatusBarPrivate::SBItem *item = nullptr;

      for (int i = 0; i < d->items.size(); ++i) {
         item = d->items.at(i);

         if (! item) {
            break;
         }

         if (item->w == ((QChildEvent *)e)->child()) {
            d->items.removeAt(i);
            delete item;
         }
      }
   }

   // On Mac OS X.5 it is possible to drag the window by clicking
   // on the tool bar on most applications.

   return QWidget::event(e);
}
#endif
