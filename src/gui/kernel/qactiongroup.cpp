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

#include <qactiongroup.h>

#ifndef QT_NO_ACTION

#include <qaction_p.h>
#include <qapplication.h>
#include <qevent.h>
#include <qlist.h>

class QActionGroupPrivate
{
   Q_DECLARE_PUBLIC(QActionGroup)

 public:
   QActionGroupPrivate() : exclusive(1), enabled(1), visible(1)  { }
   virtual ~QActionGroupPrivate() {}

   QList<QAction *> actions;
   QPointer<QAction> current;
   uint exclusive : 1;
   uint enabled : 1;
   uint visible : 1;

 protected:
   QActionGroup *q_ptr;

 private:
   void _q_actionTriggered();  //private slot
   void _q_actionChanged();    //private slot
   void _q_actionHovered();    //private slot
};

void QActionGroupPrivate::_q_actionChanged()
{
   Q_Q(QActionGroup);
   QAction *action = qobject_cast<QAction *>(q->sender());
   Q_ASSERT_X(action != nullptr, "QWidgetGroup::_q_actionChanged", "internal error");

   if (exclusive) {
      if (action->isChecked()) {
         if (action != current) {
            if (current) {
               current->setChecked(false);
            }
            current = action;
         }

      } else if (action == current) {
         current = nullptr;
      }
   }
}

void QActionGroupPrivate::_q_actionTriggered()
{
   Q_Q(QActionGroup);

   QAction *action = qobject_cast<QAction *>(q->sender());
   Q_ASSERT_X(action != nullptr, "QWidgetGroup::_q_actionTriggered", "internal error");

   emit q->triggered(action);
}

void QActionGroupPrivate::_q_actionHovered()
{
   Q_Q(QActionGroup);

   QAction *action = qobject_cast<QAction *>(q->sender());
   Q_ASSERT_X(action != nullptr, "QWidgetGroup::_q_actionHovered", "internal error");

   emit q->hovered(action);
}

QActionGroup::QActionGroup(QObject *parent)
   : QObject(parent), d_ptr(new QActionGroupPrivate)
{
   d_ptr->q_ptr = this;
}

QActionGroup::~QActionGroup()
{
}

QAction *QActionGroup::addAction(QAction *a)
{
   Q_D(QActionGroup);

   if (! d->actions.contains(a)) {
      d->actions.append(a);

      QObject::connect(a, &QAction::triggered, this, &QActionGroup::_q_actionTriggered);
      QObject::connect(a, &QAction::changed,   this, &QActionGroup::_q_actionChanged);
      QObject::connect(a, &QAction::hovered,   this, &QActionGroup::_q_actionHovered);
   }

   if (! a->d_func()->forceDisabled) {
      a->setEnabled(d->enabled);
      a->d_func()->forceDisabled = false;
   }

   if (!a->d_func()->forceInvisible) {
      a->setVisible(d->visible);
      a->d_func()->forceInvisible = false;
   }

   if (a->isChecked()) {
      d->current = a;
   }

   QActionGroup *oldGroup = a->d_func()->group;
   if (oldGroup != this) {
      if (oldGroup) {
         oldGroup->removeAction(a);
      }
      a->d_func()->group = this;
   }
   return a;
}

QAction *QActionGroup::addAction(const QString &text)
{
   return new QAction(text, this);
}

QAction *QActionGroup::addAction(const QIcon &icon, const QString &text)
{
   return new QAction(icon, text, this);
}

void QActionGroup::removeAction(QAction *action)
{
   Q_D(QActionGroup);

   if (d->actions.removeAll(action)) {
      if (action == d->current) {
         d->current = nullptr;
      }

      QObject::disconnect(action, &QAction::triggered, this, &QActionGroup::_q_actionTriggered);
      QObject::disconnect(action, &QAction::changed,   this, &QActionGroup::_q_actionChanged);
      QObject::disconnect(action, &QAction::hovered,   this, &QActionGroup::_q_actionHovered);
      action->d_func()->group = nullptr;
   }
}

QList<QAction *> QActionGroup::actions() const
{
   Q_D(const QActionGroup);
   return d->actions;
}

void QActionGroup::setExclusive(bool b)
{
   Q_D(QActionGroup);
   d->exclusive = b;
}

bool QActionGroup::isExclusive() const
{
   Q_D(const QActionGroup);
   return d->exclusive;
}

void QActionGroup::setEnabled(bool b)
{
   Q_D(QActionGroup);
   d->enabled = b;
   for (QList<QAction *>::const_iterator it = d->actions.constBegin(); it != d->actions.constEnd(); ++it) {
      if (!(*it)->d_func()->forceDisabled) {
         (*it)->setEnabled(b);
         (*it)->d_func()->forceDisabled = false;
      }
   }
}

bool QActionGroup::isEnabled() const
{
   Q_D(const QActionGroup);
   return d->enabled;
}

/*!
  Returns the currently checked action in the group, or 0 if none
  are checked.
*/
QAction *QActionGroup::checkedAction() const
{
   Q_D(const QActionGroup);
   return d->current;
}

void QActionGroup::setVisible(bool b)
{
   Q_D(QActionGroup);
   d->visible = b;

   for (QList<QAction *>::iterator it = d->actions.begin(); it != d->actions.end(); ++it) {
      if (!(*it)->d_func()->forceInvisible) {
         (*it)->setVisible(b);
         (*it)->d_func()->forceInvisible = false;
      }
   }
}

bool QActionGroup::isVisible() const
{
   Q_D(const QActionGroup);
   return d->visible;
}

void QActionGroup::_q_actionTriggered()
{
   Q_D(QActionGroup);
   d->_q_actionTriggered();
}

void QActionGroup::_q_actionChanged()
{
   Q_D(QActionGroup);
   d->_q_actionChanged();
}

void QActionGroup::_q_actionHovered()
{
   Q_D(QActionGroup);
   d->_q_actionHovered();
}

#endif // QT_NO_ACTION
