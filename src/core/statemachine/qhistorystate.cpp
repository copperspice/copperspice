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

#include <qhistorystate.h>

#ifndef QT_NO_STATEMACHINE

#include <qhistorystate_p.h>

QT_BEGIN_NAMESPACE

QHistoryStatePrivate::QHistoryStatePrivate()
   : QAbstractStatePrivate(HistoryState),
     defaultState(0), historyType(QHistoryState::ShallowHistory)
{
}

QHistoryStatePrivate *QHistoryStatePrivate::get(QHistoryState *q)
{
   return q->d_func();
}

/*!
  Constructs a new shallow history state with the given \a parent state.
*/
QHistoryState::QHistoryState(QState *parent)
   : QAbstractState(*new QHistoryStatePrivate, parent)
{
}
/*!
  Constructs a new history state of the given \a type, with the given \a
  parent state.
*/
QHistoryState::QHistoryState(HistoryType type, QState *parent)
   : QAbstractState(*new QHistoryStatePrivate, parent)
{
   Q_D(QHistoryState);
   d->historyType = type;
}

/*!
  Destroys this history state.
*/
QHistoryState::~QHistoryState()
{
}

/*!
  Returns this history state's default state.  The default state indicates the
  state to transition to if the parent state has never been entered before.
*/
QAbstractState *QHistoryState::defaultState() const
{
   Q_D(const QHistoryState);
   return d->defaultState;
}

/*!
  Sets this history state's default state to be the given \a state.
  \a state must be a sibling of this history state.

  Note that this function does not set \a state as the initial state
  of its parent.
*/
void QHistoryState::setDefaultState(QAbstractState *state)
{
   Q_D(QHistoryState);
   if (state && state->parentState() != parentState()) {
      qWarning("QHistoryState::setDefaultState: state %p does not belong "
               "to this history state's group (%p)", state, parentState());
      return;
   }
   d->defaultState = state;
}

/*!
  Returns the type of history that this history state records.
*/
QHistoryState::HistoryType QHistoryState::historyType() const
{
   Q_D(const QHistoryState);
   return d->historyType;
}

/*!
  Sets the \a type of history that this history state records.
*/
void QHistoryState::setHistoryType(HistoryType type)
{
   Q_D(QHistoryState);
   d->historyType = type;
}

/*!
  \reimp
*/
void QHistoryState::onEntry(QEvent *event)
{
   Q_UNUSED(event);
}

/*!
  \reimp
*/
void QHistoryState::onExit(QEvent *event)
{
   Q_UNUSED(event);
}

/*!
  \reimp
*/
bool QHistoryState::event(QEvent *e)
{
   return QAbstractState::event(e);
}

QT_END_NAMESPACE

#endif //QT_NO_STATEMACHINE
