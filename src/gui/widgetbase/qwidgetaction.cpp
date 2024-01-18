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

#include <qwidgetaction.h>
#include <qdebug.h>

#ifndef QT_NO_ACTION

#include <qwidgetaction_p.h>

QWidgetAction::QWidgetAction(QObject *parent)
   : QAction(*(new QWidgetActionPrivate), parent)
{
}

QWidgetAction::~QWidgetAction()
{
   Q_D(QWidgetAction);

   for (auto &item : d->createdWidgets) {
      disconnect(item, &QObject::destroyed, this, &QWidgetAction::_q_widgetDestroyed);
   }

   QList<QWidget *> widgetsToDelete = d->createdWidgets;
   d->createdWidgets.clear();

   qDeleteAll(widgetsToDelete);
   delete d->defaultWidget;
}

void QWidgetAction::setDefaultWidget(QWidget *widget)
{
   Q_D(QWidgetAction);

   if (widget == d->defaultWidget || d->defaultWidgetInUse) {
      return;
   }

   delete d->defaultWidget;
   d->defaultWidget = widget;

   if (!widget) {
      return;
   }

   setVisible(!(widget->isHidden() && widget->testAttribute(Qt::WA_WState_ExplicitShowHide)));
   d->defaultWidget->hide();
   d->defaultWidget->setParent(nullptr);
   d->defaultWidgetInUse = false;

   if (!isEnabled()) {
      d->defaultWidget->setEnabled(false);
   }
}

QWidget *QWidgetAction::defaultWidget() const
{
   Q_D(const QWidgetAction);
   return d->defaultWidget;
}

QWidget *QWidgetAction::requestWidget(QWidget *parent)
{
   Q_D(QWidgetAction);

   QWidget *w = createWidget(parent);

   if (! w) {
      if (d->defaultWidgetInUse || ! d->defaultWidget) {
         return nullptr;
      }

      d->defaultWidget->setParent(parent);
      d->defaultWidgetInUse = true;
      return d->defaultWidget;
   }

   connect(w, &QWidget::destroyed, this, &QWidgetAction::_q_widgetDestroyed);
   d->createdWidgets.append(w);

   return w;
}

void QWidgetAction::releaseWidget(QWidget *widget)
{
   Q_D(QWidgetAction);

   if (widget == d->defaultWidget) {
      d->defaultWidget->hide();
      d->defaultWidget->setParent(nullptr);
      d->defaultWidgetInUse = false;
      return;
   }

   if (! d->createdWidgets.contains(widget)) {
      return;
   }

   disconnect(widget, &QWidget::destroyed, this, &QWidgetAction::_q_widgetDestroyed);
   d->createdWidgets.removeAll(widget);
   deleteWidget(widget);
}

bool QWidgetAction::event(QEvent *event)
{
   Q_D(QWidgetAction);

   if (event->type() == QEvent::ActionChanged) {
      if (d->defaultWidget) {
         d->defaultWidget->setEnabled(isEnabled());
      }

      for (int i = 0; i < d->createdWidgets.count(); ++i) {
         d->createdWidgets.at(i)->setEnabled(isEnabled());
      }
   }

   return QAction::event(event);
}

bool QWidgetAction::eventFilter(QObject *obj, QEvent *event)
{
   return QAction::eventFilter(obj, event);
}

QWidget *QWidgetAction::createWidget(QWidget *parent)
{
   (void) parent;

   return nullptr;
}

void QWidgetAction::deleteWidget(QWidget *widget)
{
   widget->hide();
   widget->deleteLater();
}

QList<QWidget *> QWidgetAction::createdWidgets() const
{
   Q_D(const QWidgetAction);
   return d->createdWidgets;
}

void QWidgetAction::_q_widgetDestroyed(QObject *object)
{
   Q_D(QWidgetAction);
   d->_q_widgetDestroyed(object);
}

#endif // QT_NO_ACTION
