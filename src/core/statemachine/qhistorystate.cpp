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

#include <qhistorystate.h>

#ifndef QT_NO_STATEMACHINE

#include <qhistorystate_p.h>

QHistoryStatePrivate::QHistoryStatePrivate()
   : QAbstractStatePrivate(HistoryState), defaultTransition(nullptr), historyType(QHistoryState::ShallowHistory)
{
}

DefaultStateTransition::DefaultStateTransition(QHistoryState *source, QAbstractState *target)
   : QAbstractTransition()
{
   setParent(source);
   setTargetState(target);
}

QHistoryStatePrivate *QHistoryStatePrivate::get(QHistoryState *q)
{
   return q->d_func();
}

QHistoryState::QHistoryState(QState *parent)
   : QAbstractState(*new QHistoryStatePrivate, parent)
{
}

QHistoryState::QHistoryState(HistoryType type, QState *parent)
   : QAbstractState(*new QHistoryStatePrivate, parent)
{
   Q_D(QHistoryState);
   d->historyType = type;
}

QHistoryState::~QHistoryState()
{
}

QAbstractTransition *QHistoryState::defaultTransition() const
{
   Q_D(const QHistoryState);
   return d->defaultTransition;
}

void QHistoryState::setDefaultTransition(QAbstractTransition *transition)
{
   Q_D(QHistoryState);

   if (d->defaultTransition != transition) {
      d->defaultTransition = transition;
      transition->setParent(this);
      emit defaultTransitionChanged();
   }
}

QAbstractState *QHistoryState::defaultState() const
{
   Q_D(const QHistoryState);
   return d->defaultTransition ? d->defaultTransition->targetState() : nullptr;
}

void QHistoryState::setDefaultState(QAbstractState *state)
{
   Q_D(QHistoryState);

   if (state && state->parentState() != parentState()) {
      qWarning("QHistoryState::setDefaultState() State %p does not belong "
            "to this history state group (%p)", static_cast<void *>(state), static_cast<void *>(parentState()) );
      return;
   }

   if (! d->defaultTransition || d->defaultTransition->targetStates().size() != 1
         || d->defaultTransition->targetStates().first() != state) {

      if (! d->defaultTransition || ! dynamic_cast<DefaultStateTransition *>(d->defaultTransition)) {
         d->defaultTransition = new DefaultStateTransition(this, state);
         emit defaultTransitionChanged();

      } else {
         d->defaultTransition->setTargetState(state);
      }

      emit defaultStateChanged();
   }
}

QHistoryState::HistoryType QHistoryState::historyType() const
{
   Q_D(const QHistoryState);
   return d->historyType;
}

void QHistoryState::setHistoryType(HistoryType type)
{
   Q_D(QHistoryState);

   if (d->historyType != type) {
      d->historyType = type;
      emit historyTypeChanged();
   }
}

void QHistoryState::onEntry(QEvent *event)
{
   (void) event;
}

void QHistoryState::onExit(QEvent *event)
{
   (void) event;
}

bool QHistoryState::event(QEvent *e)
{
   return QAbstractState::event(e);
}

#endif //QT_NO_STATEMACHINE
