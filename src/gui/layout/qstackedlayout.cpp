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

#include <qstackedlayout.h>

#include <qalgorithms.h>
#include <qlayout_p.h>
#include <qlist.h>
#include <qwidget.h>

#include <qwidget_p.h>
#include <qlayoutengine_p.h>

class QStackedLayoutPrivate : public QLayoutPrivate
{
   Q_DECLARE_PUBLIC(QStackedLayout)

 public:
   QStackedLayoutPrivate() : index(-1), stackingMode(QStackedLayout::StackOne) {}
   QLayoutItem *replaceAt(int index, QLayoutItem *newitem) override;
   QList<QLayoutItem *> list;

   int index;
   QStackedLayout::StackingMode stackingMode;
};

QLayoutItem *QStackedLayoutPrivate::replaceAt(int idx, QLayoutItem *newitem)
{
   Q_Q(QStackedLayout);

   if (idx < 0 || idx >= list.size() || !newitem) {
      return nullptr;
   }

   QWidget *wdg = newitem->widget();
   if (! wdg) {
      qWarning("QStackedLayout::replaceAt() Only widgets can be added to a stacked layout");
      return nullptr;
   }

   QLayoutItem *orgitem = list.at(idx);
   list.replace(idx, newitem);

   if (idx == index) {
      q->setCurrentIndex(index);
   }

   return orgitem;
}

QStackedLayout::QStackedLayout()
   : QLayout(*new QStackedLayoutPrivate, nullptr, nullptr)
{
}

QStackedLayout::QStackedLayout(QWidget *parent)
   : QLayout(*new QStackedLayoutPrivate, nullptr, parent)
{
}

QStackedLayout::QStackedLayout(QLayout *parentLayout)
   : QLayout(*new QStackedLayoutPrivate, parentLayout, nullptr)
{
}

QStackedLayout::~QStackedLayout()
{
   Q_D(QStackedLayout);
   qDeleteAll(d->list);
}

int QStackedLayout::addWidget(QWidget *widget)
{
   Q_D(QStackedLayout);
   return insertWidget(d->list.count(), widget);
}

int QStackedLayout::insertWidget(int index, QWidget *widget)
{
   Q_D(QStackedLayout);
   addChildWidget(widget);
   index = qMin(index, d->list.count());

   if (index < 0) {
      index = d->list.count();
   }

   QWidgetItem *wi = QLayoutPrivate::createWidgetItem(this, widget);
   d->list.insert(index, wi);
   invalidate();

   if (d->index < 0) {
      setCurrentIndex(index);

   } else {
      if (index <= d->index) {
         ++d->index;
      }
      if (d->stackingMode == StackOne) {
         widget->hide();
      }
      widget->lower();
   }

   return index;
}

QLayoutItem *QStackedLayout::itemAt(int index) const
{
   Q_D(const QStackedLayout);
   return d->list.value(index);
}

QLayoutItem *QStackedLayout::takeAt(int index)
{
   Q_D(QStackedLayout);
   if (index < 0 || index >= d->list.size()) {
      return nullptr;
   }

   QLayoutItem *item = d->list.takeAt(index);
   if (index == d->index) {
      d->index = -1;

      if ( d->list.count() > 0 ) {
         int newIndex = (index == d->list.count()) ? index - 1 : index;
         setCurrentIndex(newIndex);
      } else {
         emit currentChanged(-1);
      }

   } else if (index < d->index) {
      --d->index;
   }

   emit widgetRemoved(index);
   if (item->widget() && ! CSInternalRefCount::get_m_wasDeleted(item->widget())) {
      item->widget()->hide();
   }

   return item;
}

void QStackedLayout::setCurrentIndex(int index)
{
   Q_D(QStackedLayout);

   QWidget *prev = currentWidget();
   QWidget *next = widget(index);

   if (! next || next == prev) {
      return;
   }

   bool reenableUpdates = false;
   QWidget *parent = parentWidget();

   if (parent && parent->updatesEnabled()) {
      reenableUpdates = true;
      parent->setUpdatesEnabled(false);
   }

   QPointer<QWidget> fw = parent ? parent->window()->focusWidget() : nullptr;
   const bool focusWasOnOldPage = fw && (prev && prev->isAncestorOf(fw));

   if (prev) {
      prev->clearFocus();
      if (d->stackingMode == StackOne) {
         prev->hide();
      }
   }

   d->index = index;
   next->raise();
   next->show();

   // try to move focus onto the incoming widget if focus
   // was somewhere on the outgoing widget.

   if (parent) {
      if (focusWasOnOldPage) {
         // look for the best focus widget we can find
         if (QWidget *nfw = next->focusWidget()) {
            nfw->setFocus();
         } else {
            // second best: first child widget in the focus chain
            if (QWidget *i = fw) {
               while ((i = i->nextInFocusChain()) != fw) {

                  if (((i->focusPolicy() & Qt::TabFocus) == Qt::TabFocus)
                        && !i->focusProxy() && i->isVisibleTo(next) && i->isEnabled()
                        && next->isAncestorOf(i)) {

                     i->setFocus();
                     break;
                  }
               }

               // third best: incoming widget
               if (i == fw ) {
                  next->setFocus();
               }
            }
         }
      }
   }

   if (reenableUpdates) {
      parent->setUpdatesEnabled(true);
   }

   emit currentChanged(index);
}

int QStackedLayout::currentIndex() const
{
   Q_D(const QStackedLayout);
   return d->index;
}

void QStackedLayout::setCurrentWidget(QWidget *widget)
{
   int index = indexOf(widget);

   if (index == -1) {
      qWarning("QStackedLayout::setCurrentWidget() Widget %p is not part of this stacked layout",
            static_cast<void *>(widget));
      return;
   }

   setCurrentIndex(index);
}

QWidget *QStackedLayout::currentWidget() const
{
   Q_D(const QStackedLayout);
   return d->index >= 0 ? d->list.at(d->index)->widget() : nullptr;
}

QWidget *QStackedLayout::widget(int index) const
{
   Q_D(const QStackedLayout);
   if (index < 0 || index >= d->list.size()) {
      return nullptr;
   }

   return d->list.at(index)->widget();
}

int QStackedLayout::count() const
{
   Q_D(const QStackedLayout);
   return d->list.size();
}

void QStackedLayout::addItem(QLayoutItem *item)
{
   QWidget *widget = item->widget();

   if (widget) {
      addWidget(widget);
      delete item;
   } else {
      qWarning("QStackedLayout::addItem() Only widgets can be added to a stacked layout");
   }
}

QSize QStackedLayout::sizeHint() const
{
   Q_D(const QStackedLayout);
   QSize s(0, 0);
   int n = d->list.count();

   for (int i = 0; i < n; ++i)
      if (QWidget *widget = d->list.at(i)->widget()) {
         QSize ws(widget->sizeHint());
         if (widget->sizePolicy().horizontalPolicy() == QSizePolicy::Ignored) {
            ws.setWidth(0);
         }
         if (widget->sizePolicy().verticalPolicy() == QSizePolicy::Ignored) {
            ws.setHeight(0);
         }
         s = s.expandedTo(ws);
      }
   return s;
}

QSize QStackedLayout::minimumSize() const
{
   Q_D(const QStackedLayout);
   QSize s(0, 0);
   int n = d->list.count();

   for (int i = 0; i < n; ++i)
      if (QWidget *widget = d->list.at(i)->widget()) {
         s = s.expandedTo(qSmartMinSize(widget));
      }
   return s;
}

void QStackedLayout::setGeometry(const QRect &rect)
{
   Q_D(QStackedLayout);

   switch (d->stackingMode) {
      case StackOne:
         if (QWidget *widget = currentWidget()) {
            widget->setGeometry(rect);
         }
         break;

      case StackAll:
         if (const int n = d->list.count()) {
            for (int i = 0; i < n; ++i) {
               if (QWidget *widget = d->list.at(i)->widget()) {
                  widget->setGeometry(rect);
               }
            }
         }
         break;
   }
}

bool QStackedLayout::hasHeightForWidth() const
{
   const int n = count();

   for (int i = 0; i < n; ++i) {
      if (QLayoutItem *item = itemAt(i)) {
         if (item->hasHeightForWidth()) {
            return true;
         }
      }
   }

   return false;
}

int QStackedLayout::heightForWidth(int width) const
{
   const int n = count();

   int hfw = 0;
   for (int i = 0; i < n; ++i) {
      if (QLayoutItem *item = itemAt(i)) {
         if (QWidget *w = item->widget())
            /*
            Note: Does not query the layout item, but bypasses it and asks the widget
            directly. This is consistent with how QStackedLayout::sizeHint() is
            implemented. This also avoids an issue where QWidgetItem::heightForWidth()
            returns -1 if the widget is hidden.
            */
         {
            hfw = qMax(hfw, w->heightForWidth(width));
         }
      }
   }

   hfw = qMax(hfw, minimumSize().height());
   return hfw;
}

QStackedLayout::StackingMode QStackedLayout::stackingMode() const
{
   Q_D(const QStackedLayout);
   return d->stackingMode;
}

void QStackedLayout::setStackingMode(StackingMode stackingMode)
{
   Q_D(QStackedLayout);
   if (d->stackingMode == stackingMode) {
      return;
   }
   d->stackingMode = stackingMode;

   const int n = d->list.count();
   if (n == 0) {
      return;
   }

   switch (d->stackingMode) {
      case StackOne:
         if (const int idx = currentIndex())
            for (int i = 0; i < n; ++i)
               if (QWidget *widget = d->list.at(i)->widget()) {
                  widget->setVisible(i == idx);
               }
         break;
      case StackAll: { // Turn overlay on: Make sure all widgets are the same size
         QRect geometry;
         if (const QWidget *widget = currentWidget()) {
            geometry = widget->geometry();
         }
         for (int i = 0; i < n; ++i)
            if (QWidget *widget = d->list.at(i)->widget()) {
               if (!geometry.isNull()) {
                  widget->setGeometry(geometry);
               }
               widget->setVisible(true);
            }
      }
      break;
   }
}


