/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qwidgetaction.h>
#include <qdebug.h>

#ifndef QT_NO_ACTION
#include <qwidgetaction_p.h>

QT_BEGIN_NAMESPACE

QWidgetAction::QWidgetAction(QObject *parent)
   : QAction(*(new QWidgetActionPrivate), parent)
{
}

QWidgetAction::~QWidgetAction()
{
   Q_D(QWidgetAction);
   for (int i = 0; i < d->createdWidgets.count(); ++i)
      disconnect(d->createdWidgets.at(i), SIGNAL(destroyed(QObject *)),
                 this, SLOT(_q_widgetDestroyed(QObject *)));
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
   d->defaultWidget->setParent(0);
   d->defaultWidgetInUse = false;
   if (!isEnabled()) {
      d->defaultWidget->setEnabled(false);
   }
}

/*!
    Returns the default widget.
*/
QWidget *QWidgetAction::defaultWidget() const
{
   Q_D(const QWidgetAction);
   return d->defaultWidget;
}

/*!
    Returns a widget that represents the action, with the given \a
    parent.

    Container widgets that support actions can call this function to
    request a widget as visual representation of the action.

    \sa releaseWidget(), createWidget(), defaultWidget()
*/
QWidget *QWidgetAction::requestWidget(QWidget *parent)
{
   Q_D(QWidgetAction);

   QWidget *w = createWidget(parent);
   if (!w) {
      if (d->defaultWidgetInUse || !d->defaultWidget) {
         return 0;
      }
      d->defaultWidget->setParent(parent);
      d->defaultWidgetInUse = true;
      return d->defaultWidget;
   }

   connect(w, SIGNAL(destroyed(QObject *)),
           this, SLOT(_q_widgetDestroyed(QObject *)));
   d->createdWidgets.append(w);
   return w;
}

/*!
    Releases the specified \a widget.

    Container widgets that support actions call this function when a widget
    action is removed.

    \sa requestWidget(), deleteWidget(), defaultWidget()
*/
void QWidgetAction::releaseWidget(QWidget *widget)
{
   Q_D(QWidgetAction);

   if (widget == d->defaultWidget) {
      d->defaultWidget->hide();
      d->defaultWidget->setParent(0);
      d->defaultWidgetInUse = false;
      return;
   }

   if (!d->createdWidgets.contains(widget)) {
      return;
   }

   disconnect(widget, SIGNAL(destroyed(QObject *)),
              this, SLOT(_q_widgetDestroyed(QObject *)));
   d->createdWidgets.removeAll(widget);
   deleteWidget(widget);
}

/*!
    \reimp
*/
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

/*!
    \reimp
 */
bool QWidgetAction::eventFilter(QObject *obj, QEvent *event)
{
   return QAction::eventFilter(obj, event);
}

/*!
    This function is called whenever the action is added to a container widget
    that supports custom widgets. If you don't want a custom widget to be
    used as representation of the action in the specified \a parent widget then
    0 should be returned.

    \sa deleteWidget()
*/
QWidget *QWidgetAction::createWidget(QWidget *parent)
{
   Q_UNUSED(parent)
   return 0;
}

/*!
    This function is called whenever the action is removed from a
    container widget that displays the action using a custom \a
    widget previously created using createWidget(). The default
    implementation hides the \a widget and schedules it for deletion
    using QObject::deleteLater().

    \sa createWidget()
*/
void QWidgetAction::deleteWidget(QWidget *widget)
{
   widget->hide();
   widget->deleteLater();
}

/*!
    Returns the list of widgets that have been using createWidget() and
    are currently in use by widgets the action has been added to.
*/
QList<QWidget *> QWidgetAction::createdWidgets() const
{
   Q_D(const QWidgetAction);
   return d->createdWidgets;
}

void QWidgetAction::_q_widgetDestroyed(QObject *un_named_arg1)
{
   Q_D(QWidgetAction);
   d->_q_widgetDestroyed(un_named_arg1);
}

QT_END_NAMESPACE

#endif // QT_NO_ACTION
