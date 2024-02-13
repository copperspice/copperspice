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

#include <qeventtransition.h>

#ifndef QT_NO_STATEMACHINE

#include <qdebug.h>
#include <qstate.h>
#include <qstatemachine.h>

#include <qeventtransition_p.h>
#include <qstate_p.h>
#include <qstatemachine_p.h>

QEventTransitionPrivate::QEventTransitionPrivate()
{
   object     = nullptr;
   eventType  = QEvent::None;
   registered = false;
}

QEventTransitionPrivate *QEventTransitionPrivate::get(QEventTransition *q)
{
   return q->d_func();
}

void QEventTransitionPrivate::unregister()
{
   Q_Q(QEventTransition);

   if (! registered || !machine()) {
      return;
   }

   QStateMachinePrivate::get(machine())->unregisterEventTransition(q);
}

void QEventTransitionPrivate::maybeRegister()
{
   Q_Q(QEventTransition);

   if (! machine() || !machine()->configuration().contains(sourceState())) {
      return;
   }

   QStateMachinePrivate::get(machine())->registerEventTransition(q);
}

QEventTransition::QEventTransition(QState *sourceState)
   : QAbstractTransition(*new QEventTransitionPrivate, sourceState)
{
}

QEventTransition::QEventTransition(QObject *object, QEvent::Type type, QState *sourceState)
   : QAbstractTransition(*new QEventTransitionPrivate, sourceState)
{
   Q_D(QEventTransition);
   d->registered = false;
   d->object = object;
   d->eventType = type;
}

QEventTransition::QEventTransition(QEventTransitionPrivate &dd, QState *parent)
   : QAbstractTransition(dd, parent)
{
}

QEventTransition::QEventTransition(QEventTransitionPrivate &dd, QObject *object,
      QEvent::Type type, QState *parent)
   : QAbstractTransition(dd, parent)
{
   Q_D(QEventTransition);

   d->registered = false;
   d->object = object;
   d->eventType = type;
}

QEventTransition::~QEventTransition()
{
}

QEvent::Type QEventTransition::eventType() const
{
   Q_D(const QEventTransition);
   return d->eventType;
}

void QEventTransition::setEventType(QEvent::Type type)
{
   Q_D(QEventTransition);

   if (d->eventType == type) {
      return;
   }

   d->unregister();
   d->eventType = type;
   d->maybeRegister();
}

QObject *QEventTransition::eventSource() const
{
   Q_D(const QEventTransition);
   return d->object;
}

void QEventTransition::setEventSource(QObject *object)
{
   Q_D(QEventTransition);

   if (d->object == object) {
      return;
   }

   d->unregister();
   d->object = object;
   d->maybeRegister();
}

bool QEventTransition::eventTest(QEvent *event)
{
   Q_D(const QEventTransition);

   if (event->type() == QEvent::StateMachineWrapped) {
      QStateMachine::WrappedEvent *we = static_cast<QStateMachine::WrappedEvent *>(event);

      return (we->object() == d->object) && (we->event()->type() == d->eventType);
   }

   return false;
}

void QEventTransition::onTransition(QEvent *event)
{
   (void) event;
}

bool QEventTransition::event(QEvent *e)
{
   return QAbstractTransition::event(e);
}

#endif //QT_NO_STATEMACHINE
