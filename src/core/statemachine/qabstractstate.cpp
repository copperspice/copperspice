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

#include <qabstractstate.h>

#ifndef QT_NO_STATEMACHINE

#include <qabstractstate_p.h>
#include <qstate.h>
#include <qstate_p.h>
#include <qstatemachine.h>
#include <qstatemachine_p.h>

QT_BEGIN_NAMESPACE

QAbstractStatePrivate::QAbstractStatePrivate(StateType type)
   : stateType(type), isMachine(false), parentState(0)
{
}

QAbstractStatePrivate *QAbstractStatePrivate::get(QAbstractState *q)
{
   return q->d_func();
}

const QAbstractStatePrivate *QAbstractStatePrivate::get(const QAbstractState *q)
{
   return q->d_func();
}

QStateMachine *QAbstractStatePrivate::machine() const
{
   Q_Q(const QAbstractState);
   QObject *par = q->parent();

   while (par != 0) {
      if (QStateMachine *mach = qobject_cast<QStateMachine *>(par)) {
         return mach;
      }
      par = par->parent();
   }
   return 0;
}

void QAbstractStatePrivate::callOnEntry(QEvent *e)
{
   Q_Q(QAbstractState);
   q->onEntry(e);
}

void QAbstractStatePrivate::callOnExit(QEvent *e)
{
   Q_Q(QAbstractState);
   q->onExit(e);
}

void QAbstractStatePrivate::emitEntered()
{
   Q_Q(QAbstractState);
   emit q->entered();
}

void QAbstractStatePrivate::emitExited()
{
   Q_Q(QAbstractState);
   emit q->exited();
}

/*!
  Constructs a new state with the given \a parent state.
*/
QAbstractState::QAbstractState(QState *parent)
   : QObject(parent), d_ptr(new QAbstractStatePrivate(QAbstractStatePrivate::AbstractState))
{
   d_ptr->q_ptr = this;
}

/*!
  \internal
*/
QAbstractState::QAbstractState(QAbstractStatePrivate &dd, QState *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

/*!
  Destroys this state.
*/
QAbstractState::~QAbstractState()
{
}

/*!
  Returns this state's parent state, or 0 if the state has no parent state.
*/
QState *QAbstractState::parentState() const
{
   Q_D(const QAbstractState);
   if (d->parentState != parent()) {
      d->parentState = qobject_cast<QState *>(parent());
   }
   return d->parentState;
}

/*!
  Returns the state machine that this state is part of, or 0 if the state is
  not part of a state machine.
*/
QStateMachine *QAbstractState::machine() const
{
   Q_D(const QAbstractState);
   return d->machine();
}

/*!
  \fn QAbstractState::onExit(QEvent *event)

  This function is called when the state is exited. The given \a event is what
  caused the state to be exited. Reimplement this function to perform custom
  processing when the state is exited.
*/

/*!
  \fn QAbstractState::onEntry(QEvent *event)

  This function is called when the state is entered. The given \a event is
  what caused the state to be entered. Reimplement this function to perform
  custom processing when the state is entered.
*/

/*!
  \fn QAbstractState::entered()

  This signal is emitted when the state has been entered (after onEntry() has
  been called).
*/

/*!
  \fn QAbstractState::exited()

  This signal is emitted when the state has been exited (after onExit() has
  been called).
*/

/*!
  \reimp
*/
bool QAbstractState::event(QEvent *e)
{
   return QObject::event(e);
}

QT_END_NAMESPACE

#endif //QT_NO_STATEMACHINE
