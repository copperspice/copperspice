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

#include <qabstracttransition.h>

#ifndef QT_NO_STATEMACHINE

#include <qabstractstate.h>
#include <qhistorystate.h>
#include <qstate.h>
#include <qstatemachine.h>

#include <qabstracttransition_p.h>

QAbstractTransitionPrivate::QAbstractTransitionPrivate()
   : transitionType(QAbstractTransition::ExternalTransition)
{
}

QAbstractTransitionPrivate *QAbstractTransitionPrivate::get(QAbstractTransition *q)
{
   return q->d_func();
}

QStateMachine *QAbstractTransitionPrivate::machine() const
{
   if (QState *source = sourceState()) {
      return source->machine();
   }

   Q_Q(const QAbstractTransition);

   if (QHistoryState *parent = dynamic_cast<QHistoryState *>(q->parent())) {
      return parent->machine();
   }

   return nullptr;
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

   if ((d->targetStates.size() == 1 && target == d->targetStates.at(0).data()) ||
         (d->targetStates.isEmpty() && target == nullptr)) {
      return;
   }

   if (! target) {
      d->targetStates.clear();
   } else {
      setTargetStates(QList<QAbstractState *>() << target);
   }

   emit targetStateChanged();
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

      if (targets.at(i) == nullptr) {
         qWarning("QAbstractTransition::setTargetStates() Target state(s) invalid (nullptr)");
         return;
      }
   }

   // clean out any target states that got destroyed, but which there is still a QPointer active
   for (int i = 0; i < d->targetStates.size(); ) {
      if (d->targetStates.at(i).isNull()) {
         d->targetStates.remove(i);
      } else {
         ++i;
      }
   }

   // Easy check: if both lists are empty, we're done.
   if (targets.isEmpty() && d->targetStates.isEmpty()) {
      return;
   }

   bool sameList = true;

   if (targets.size() != d->targetStates.size()) {
      // If the sizes of the lists are different, we don't need to be smart: they're different.
      // So we can just set the new list as the targetStates.
      sameList = false;

   } else {
      QVector<QPointer<QAbstractState>> copy(d->targetStates);

      for (int i = 0; i < targets.size(); ++i) {
         sameList = sameList && copy.removeOne(targets.at(i));

         if (! sameList) {
            break;   // we now know the lists are not the same, so stop the loop
         }
      }

      sameList = sameList && copy.isEmpty();
   }

   if (sameList) {
      return;
   }

   d->targetStates.resize(targets.size());

   for (int i = 0; i < targets.size(); ++i) {
      d->targetStates[i] = targets.at(i);
   }

   emit targetStatesChanged();
}

QAbstractTransition::TransitionType QAbstractTransition::transitionType() const
{
   Q_D(const QAbstractTransition);
   return d->transitionType;
}

void QAbstractTransition::setTransitionType(TransitionType type)
{
   Q_D(QAbstractTransition);
   d->transitionType = type;
}

QStateMachine *QAbstractTransition::machine() const
{
   Q_D(const QAbstractTransition);
   return d->machine();
}

#ifndef QT_NO_ANIMATION

void QAbstractTransition::addAnimation(QAbstractAnimation *animation)
{
   Q_D(QAbstractTransition);

   if (! animation) {
      qWarning("QAbstractTransition::addAnimation() Unable to add animation (nullptr)");
      return;
   }

   d->animations.append(animation);
}

void QAbstractTransition::removeAnimation(QAbstractAnimation *animation)
{
   Q_D(QAbstractTransition);

   if (! animation) {
      qWarning("QAbstractTransition::removeAnimation() Unable to remove animation (nullptr)");
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
