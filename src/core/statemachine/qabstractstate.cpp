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

#include <qabstractstate.h>

#ifndef QT_NO_STATEMACHINE

#include <qstate.h>
#include <qstatemachine.h>

#include <qabstractstate_p.h>
#include <qstate_p.h>
#include <qstatemachine_p.h>

QAbstractStatePrivate::QAbstractStatePrivate(StateType type)
   : stateType(type), isMachine(false), active(false), parentState(nullptr)
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

   while (par != nullptr) {
      if (QStateMachine *mach = dynamic_cast<QStateMachine *>(par)) {
         return mach;
      }

      par = par->parent();
   }

   return nullptr;
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

   if (! active) {
      active = true;
      emit q->activeChanged(true);
   }
}

void QAbstractStatePrivate::emitExited()
{
   Q_Q(QAbstractState);

   if (active) {
      active = false;
      emit q->activeChanged(false);
   }

   emit q->exited();
}

QAbstractState::QAbstractState(QState *parent)
   : QObject(parent), d_ptr(new QAbstractStatePrivate(QAbstractStatePrivate::AbstractState))
{
   d_ptr->q_ptr = this;
}

QAbstractState::QAbstractState(QAbstractStatePrivate &dd, QState *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QAbstractState::~QAbstractState()
{
}

QState *QAbstractState::parentState() const
{
   Q_D(const QAbstractState);

   if (d->parentState != parent()) {
      d->parentState = qobject_cast<QState *>(parent());
   }

   return d->parentState;
}

QStateMachine *QAbstractState::machine() const
{
   Q_D(const QAbstractState);
   return d->machine();
}

bool QAbstractState::active() const
{
   Q_D(const QAbstractState);
   return d->active;
}

bool QAbstractState::event(QEvent *e)
{
   return QObject::event(e);
}

#endif
