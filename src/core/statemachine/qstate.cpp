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

#include <qstate.h>

#ifndef QT_NO_STATEMACHINE

#include <qhistorystate.h>
#include <qabstracttransition.h>
#include <qsignaltransition.h>
#include <qstatemachine.h>

#include <qstate_p.h>
#include <qabstracttransition_p.h>
#include <qhistorystate_p.h>
#include <qstatemachine_p.h>

QStatePrivate::QStatePrivate()
   : QAbstractStatePrivate(StandardState),
     errorState(nullptr), initialState(nullptr), childMode(QState::ExclusiveStates),
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

void QState::assignProperty(QObject *object, const QString &name, const QVariant &value)
{
   Q_D(QState);

   if (! object) {
      qWarning("QState::assignProperty() Unable to assign property %s to invalid object (nullptr)", csPrintable(name));
      return;
   }

   for (int i = 0; i < d->propertyAssignments.size(); ++i) {
      QPropertyAssignment &assn = d->propertyAssignments[i];

      if (assn.hasTarget(object, name)) {
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

   if (state != nullptr && qobject_cast<QStateMachine *>(state)) {
      qWarning("QStateMachine::setErrorState() Unable to set error state for the 'root state'");
      return;
   }

   if (state != nullptr && (!state->machine() || ((state->machine() != machine()) && !qobject_cast<QStateMachine *>(this)))) {
      qWarning("QState::setErrorState() Unable to set error state from a different state machine");
      return;
   }

   if (d->errorState != state) {
      d->errorState = state;
      emit errorStateChanged();
   }
}

void QState::addTransition(QAbstractTransition *transition)
{
   Q_D(QState);

   if (!transition) {
      qWarning("QState::addTransition() Unable to add invalid transition");
      return ;
   }

   transition->setParent(this);
   const QVector<QPointer<QAbstractState>> &targets = QAbstractTransitionPrivate::get(transition)->targetStates;

   for (int i = 0; i < targets.size(); ++i) {
      QAbstractState *t = targets.at(i).data();

      if (!t) {
         qWarning("QState::addTransition() Unable to add invalid transition (nullptr)");
         return ;
      }

      if ((QAbstractStatePrivate::get(t)->machine() != d->machine())
            && QAbstractStatePrivate::get(t)->machine() && d->machine()) {
         qWarning("QState::addTransition() Unable to add transition for a state from a different state machine");
         return ;
      }
   }

   if (QStateMachine *mach = machine()) {
      QStateMachinePrivate::get(mach)->maybeRegisterTransition(transition);
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

QAbstractTransition *QState::addTransition(QAbstractState *target)
{
   if (target == nullptr) {
      qWarning("QState::addTransition() Unable to add transition to an invalid state (nullptr)");
      return nullptr;
   }

   UnconditionalTransition *trans = new UnconditionalTransition(target);
   addTransition(trans);

   return trans;
}

void QState::removeTransition(QAbstractTransition *transition)
{
   Q_D(QState);

   if (transition == nullptr) {
      qWarning("QState::removeTransition() Unable to remove an invalid transition (nullptr)");
      return;
   }

   if (transition->sourceState() != this) {
      qWarning("QState::removeTransition() Transition source state does not match the current state");
      return;
   }

   QStateMachinePrivate *mach = QStateMachinePrivate::get(d->machine());

   if (mach) {
      mach->unregisterTransition(transition);
   }

   transition->setParent(nullptr);
}

QList<QAbstractTransition *> QState::transitions() const
{
   Q_D(const QState);
   return d->transitions();
}

void QState::onEntry(QEvent *event)
{
   (void) event;
}

void QState::onExit(QEvent *event)
{
   (void) event;
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
      qWarning("QState::setInitialState() Unable to set initial state of parallel state group");
      return;
   }

   if (state && (state->parentState() != this)) {
      qWarning("QState::setInitialState() New state is not a child of the current state");
      return;
   }

   if (d->initialState != state) {
      d->initialState = state;
      emit initialStateChanged();
   }
}

QState::ChildMode QState::childMode() const
{
   Q_D(const QState);
   return d->childMode;
}

void QState::setChildMode(ChildMode mode)
{
   Q_D(QState);

   if (mode == QState::ParallelStates && d->initialState) {
      qWarning("QState::setChildMode() Setting the child mode of the current state to 'parallel' removes the initial state");

      d->initialState = nullptr;
      emit initialStateChanged();
   }

   if (d->childMode != mode) {
      d->childMode = mode;
      emit childModeChanged();
   }
}

bool QState::event(QEvent *e)
{
   Q_D(QState);

   if ((e->type() == QEvent::ChildAdded) || (e->type() == QEvent::ChildRemoved)) {
      d->childStatesListNeedsRefresh = true;
      d->transitionsListNeedsRefresh = true;

      if ((e->type() == QEvent::ChildRemoved) && (static_cast<QChildEvent *>(e)->child() == d->initialState)) {
         d->initialState = nullptr;
      }
   }

   return QAbstractState::event(e);
}

#endif //QT_NO_STATEMACHINE
