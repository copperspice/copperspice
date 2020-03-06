/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qabstracttransition.h>

#ifndef QT_NO_STATEMACHINE

#include <qabstracttransition_p.h>

#include <qabstractstate.h>
#include <qhistorystate.h>
#include <qstate.h>
#include <qstatemachine.h>

QAbstractTransitionPrivate::QAbstractTransitionPrivate()
{
}

QAbstractTransitionPrivate *QAbstractTransitionPrivate::get(QAbstractTransition *q)
{
   return q->d_func();
}

QStateMachine *QAbstractTransitionPrivate::machine() const
{
   QState *source = sourceState();
   if (!source) {
      return 0;
   }
   return source->machine();
}

bool QAbstractTransitionPrivate::callEventTest(QEvent *e)
{
   Q_Q(QAbstractTransition);
   return q->eventTest(e);
}

void QAbstractTransitionPrivate::callOnTransition(QEvent *e)
{
   Q_Q(QAbstractTransition);
   q->onTransition(e);
}

QState *QAbstractTransitionPrivate::sourceState() const
{
   Q_Q(const QAbstractTransition);
   return qobject_cast<QState *>(q->parent());
}

void QAbstractTransitionPrivate::emitTriggered()
{
   Q_Q(QAbstractTransition);
   emit q->triggered();
}

QAbstractTransition::QAbstractTransition(QState *sourceState)
   : QObject(sourceState), d_ptr(new QAbstractTransitionPrivate)
{
   d_ptr->q_ptr = this;
}

QAbstractTransition::QAbstractTransition(QAbstractTransitionPrivate &dd, QState *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QAbstractTransition::~QAbstractTransition()
{
}

QState *QAbstractTransition::sourceState() const
{
   Q_D(const QAbstractTransition);
   return d->sourceState();
}


QAbstractState *QAbstractTransition::targetState() const
{
   Q_D(const QAbstractTransition);

   if (d->targetStates.isEmpty()) {
      return nullptr;
   }

   return d->targetStates.first().data();
}

void QAbstractTransition::setTargetState(QAbstractState *target)
{
   Q_D(QAbstractTransition);
   if (!target) {
      d->targetStates.clear();
   } else {
      setTargetStates(QList<QAbstractState *>() << target);
   }
}

QList<QAbstractState *> QAbstractTransition::targetStates() const
{
   Q_D(const QAbstractTransition);

   QList<QAbstractState *> result;

   for (int i = 0; i < d->targetStates.size(); ++i) {
      QAbstractState *target = d->targetStates.at(i).data();

      if (target) {
         result.append(target);
      }
   }

   return result;
}

void QAbstractTransition::setTargetStates(const QList<QAbstractState *> &targets)
{
   Q_D(QAbstractTransition);

   for (int i = 0; i < targets.size(); ++i) {
      QAbstractState *target = targets.at(i);
      if (!target) {
         qWarning("QAbstractTransition::setTargetStates: target state(s) cannot be null");
         return;
      }
   }

   d->targetStates.clear();
   for (int i = 0; i < targets.size(); ++i) {
      d->targetStates.append(targets.at(i));
   }
}

/*!
  Returns the state machine that this transition is part of, or 0 if the
  transition is not part of a state machine.
*/
QStateMachine *QAbstractTransition::machine() const
{
   Q_D(const QAbstractTransition);
   return d->machine();
}

#ifndef QT_NO_ANIMATION

void QAbstractTransition::addAnimation(QAbstractAnimation *animation)
{
   Q_D(QAbstractTransition);
   if (!animation) {
      qWarning("QAbstractTransition::addAnimation: cannot add null animation");
      return;
   }
   d->animations.append(animation);
}


void QAbstractTransition::removeAnimation(QAbstractAnimation *animation)
{
   Q_D(QAbstractTransition);
   if (!animation) {
      qWarning("QAbstractTransition::removeAnimation: cannot remove null animation");
      return;
   }
   d->animations.removeOne(animation);
}


QList<QAbstractAnimation *> QAbstractTransition::animations() const
{
   Q_D(const QAbstractTransition);
   return d->animations;
}

#endif



bool QAbstractTransition::event(QEvent *e)
{
   return QObject::event(e);
}

#endif //QT_NO_STATEMACHINE
