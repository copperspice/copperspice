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

#include <qstate.h>

#ifndef QT_NO_STATEMACHINE

#include <qstate_p.h>
#include <qhistorystate.h>
#include <qhistorystate_p.h>
#include <qabstracttransition.h>
#include <qabstracttransition_p.h>
#include <qsignaltransition.h>
#include <qstatemachine.h>
#include <qstatemachine_p.h>

QT_BEGIN_NAMESPACE

QStatePrivate::QStatePrivate()
   : QAbstractStatePrivate(StandardState),
     errorState(0), initialState(0), childMode(QState::ExclusiveStates),
     childStatesListNeedsRefresh(true), transitionsListNeedsRefresh(true)
{
}

QStatePrivate::~QStatePrivate()
{
}

void QStatePrivate::emitFinished()
{
   Q_Q(QState);
   emit q->finished();
}

void QStatePrivate::emitPropertiesAssigned()
{
   Q_Q(QState);
   emit q->propertiesAssigned();
}

QState::QState(QState *parent)
   : QAbstractState(*new QStatePrivate, parent)
{
}

QState::QState(ChildMode childMode, QState *parent)
   : QAbstractState(*new QStatePrivate, parent)
{
   Q_D(QState);
   d->childMode = childMode;
}

// internal
QState::QState(QStatePrivate &dd, QState *parent)
   : QAbstractState(dd, parent)
{
}

QState::~QState()
{
}

QList<QAbstractState *> QStatePrivate::childStates() const
{
   Q_Q(const QState);

   if (childStatesListNeedsRefresh) {

      childStatesList.clear();
      QList<QObject *>::const_iterator it;

      QObjectList children = q->children();

      for (it = children.constBegin(); it != children.constEnd(); ++it) {
         QAbstractState *s = qobject_cast<QAbstractState *>(*it);

         if (! s || qobject_cast<QHistoryState *>(s)) {
            continue;
         }

         childStatesList.append(s);
      }
      childStatesListNeedsRefresh = false;
   }
   return childStatesList;
}

QList<QHistoryState *> QStatePrivate::historyStates() const
{
   Q_Q(const QState);

   QList<QHistoryState *> result;
   QList<QObject *>::const_iterator it;

   QObjectList children = q->children();

   for (it = children.constBegin(); it != children.constEnd(); ++it) {
      QHistoryState *h = qobject_cast<QHistoryState *>(*it);

      if (h) {
         result.append(h);
      }
   }
   return result;
}

QList<QAbstractTransition *> QStatePrivate::transitions() const
{
   Q_Q(const QState);

   if (transitionsListNeedsRefresh) {
      transitionsList.clear();
      QList<QObject *>::const_iterator it;

      QObjectList children = q->children();

      for (it = children.constBegin(); it != children.constEnd(); ++it) {
         QAbstractTransition *t = qobject_cast<QAbstractTransition *>(*it);
         if (t) {
            transitionsList.append(t);
         }
      }
      transitionsListNeedsRefresh = false;
   }
   return transitionsList;
}

#ifndef QT_NO_PROPERTIES

void QState::assignProperty(QObject *object, const char *name, const QVariant &value)
{
   Q_D(QState);

   if (! object) {
      qWarning("QState::assignProperty(): Can not assign property '%s' of null object", name);
      return;
   }

   for (int i = 0; i < d->propertyAssignments.size(); ++i) {
      QPropertyAssignment &assn = d->propertyAssignments[i];

      if ((assn.object == object) && (assn.propertyName == name)) {
         assn.value = value;
         return;
      }
   }

   d->propertyAssignments.append(QPropertyAssignment(object, name, value));
}

#endif // QT_NO_PROPERTIES

QAbstractState *QState::errorState() const
{
   Q_D(const QState);
   return d->errorState;
}

void QState::setErrorState(QAbstractState *state)
{
   Q_D(QState);

   if (state != 0 && qobject_cast<QStateMachine *>(state)) {
      qWarning("QStateMachine::setErrorState: root state cannot be error state");
      return;
   }
   if (state != 0 && (!state->machine() || ((state->machine() != machine()) && !qobject_cast<QStateMachine *>(this)))) {
      qWarning("QState::setErrorState(): Error state cannot belong to a different state machine");
      return;
   }

   d->errorState = state;
}

void QState::addTransition(QAbstractTransition *transition)
{
   Q_D(QState);

   if (!transition) {
      qWarning("QState::addTransition(): Can not add null transition");
      return ;
   }

   transition->setParent(this);
   const QList<QWeakPointer<QAbstractState> > &targets = QAbstractTransitionPrivate::get(transition)->targetStates;
   for (int i = 0; i < targets.size(); ++i) {
      QAbstractState *t = targets.at(i).data();
      if (!t) {
         qWarning("QState::addTransition(): Can not add transition to null state");
         return ;
      }
      if ((QAbstractStatePrivate::get(t)->machine() != d->machine())
            && QAbstractStatePrivate::get(t)->machine() && d->machine()) {
         qWarning("QState::addTransition(): Can not add transition to a state in a different state machine");
         return ;
      }
   }
   if (machine() != 0 && machine()->configuration().contains(this)) {
      QStateMachinePrivate::get(machine())->registerTransitions(this);
   }
}

namespace {

// ### Make public?
class UnconditionalTransition : public QAbstractTransition
{
 public:
   UnconditionalTransition(QAbstractState *target)
      : QAbstractTransition() {
      setTargetState(target);
   }
 protected:
   void onTransition(QEvent *) override {}
   bool eventTest(QEvent *) override {
      return true;
   }
};

} // namespace

/*!
  Adds an unconditional transition from this state to the given \a target
  state, and returns then new transition object.
*/
QAbstractTransition *QState::addTransition(QAbstractState *target)
{
   if (!target) {
      qWarning("QState::addTransition(): Can not add transition to null state");
      return 0;
   }
   UnconditionalTransition *trans = new UnconditionalTransition(target);
   addTransition(trans);
   return trans;
}

/*!
  Removes the given \a transition from this state.  The state releases
  ownership of the transition.

  \sa addTransition()
*/
void QState::removeTransition(QAbstractTransition *transition)
{
   Q_D(QState);
   if (!transition) {
      qWarning("QState::removeTransition(): Can not remove null transition");
      return;
   }

   if (transition->sourceState() != this) {
      qWarning("QState::removeTransition(): Transition %p's source state (%p)"
               " is different from this state (%p)", transition, transition->sourceState(), this);
      return;
   }

   QStateMachinePrivate *mach = QStateMachinePrivate::get(d->machine());
   if (mach) {
      mach->unregisterTransition(transition);
   }
   transition->setParent(0);
}

QList<QAbstractTransition *> QState::transitions() const
{
   Q_D(const QState);
   return d->transitions();
}

void QState::onEntry(QEvent *event)
{
   Q_UNUSED(event);
}

void QState::onExit(QEvent *event)
{
   Q_UNUSED(event);
}

QAbstractState *QState::initialState() const
{
   Q_D(const QState);
   return d->initialState;
}

void QState::setInitialState(QAbstractState *state)
{
   Q_D(QState);
   if (d->childMode == QState::ParallelStates) {
      qWarning("QState::setInitialState: ignoring attempt to set initial state "
               "of parallel state group %p", this);
      return;
   }
   if (state && (state->parentState() != this)) {
      qWarning("QState::setInitialState: state %p is not a child of this state (%p)",
               state, this);
      return;
   }
   d->initialState = state;
}

/*!
  Returns the child mode of this state.
*/
QState::ChildMode QState::childMode() const
{
   Q_D(const QState);
   return d->childMode;
}

/*!
  Sets the child \a mode of this state.
*/
void QState::setChildMode(ChildMode mode)
{
   Q_D(QState);
   d->childMode = mode;
}

/*!
  \reimp
*/
bool QState::event(QEvent *e)
{
   Q_D(QState);
   if ((e->type() == QEvent::ChildAdded) || (e->type() == QEvent::ChildRemoved)) {
      d->childStatesListNeedsRefresh = true;
      d->transitionsListNeedsRefresh = true;
   }
   return QAbstractState::event(e);
}

QT_END_NAMESPACE

#endif //QT_NO_STATEMACHINE
