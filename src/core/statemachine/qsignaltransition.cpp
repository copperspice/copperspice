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

#include <qsignaltransition.h>

#ifndef QT_NO_STATEMACHINE

#include <qstate.h>
#include <qstate_p.h>
#include <qstatemachine.h>
#include <qdebug.h>

#include <qstatemachine_p.h>
#include <qsignaltransition_p.h>

void QSignalTransition::unregister()
{
   if (! m_signalBento || ! machine()) {
      return;
   }

   QStateMachinePrivate::get(machine())->unregisterSignalTransition(this);
}

void QSignalTransition::maybeRegister()
{
   if (! machine() || ! machine()->configuration().contains(sourceState())) {
      return;
   }

   QStateMachinePrivate::get(machine())->registerSignalTransition(this);
}

QSignalTransition::QSignalTransition(QState *sourceState)
   : QAbstractTransition(*new QSignalTransitionPrivate, sourceState)
{
   m_sender = nullptr;
}

QSignalTransition::~QSignalTransition()
{
}

const QObject *QSignalTransition::senderObject() const
{
   return m_sender;
}

void QSignalTransition::setSenderObject(const QObject *sender)
{
   if (sender == m_sender) {
      return;
   }

   unregister();
   m_sender = sender;

   maybeRegister();
}

CsSignal::Internal::BentoAbstract *QSignalTransition::get_signalBento() const
{
   return m_signalBento.data();
}

bool QSignalTransition::eventTest(QEvent *event)
{
   if (event->type() == QEvent::StateMachineSignal) {

      QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent *>(event);
      return (se->sender() == m_sender);
   }

   return false;
}

void QSignalTransition::onTransition(QEvent *event)
{
   (void) event;
}

bool QSignalTransition::event(QEvent *e)
{
   return QAbstractTransition::event(e);
}

void QSignalTransitionPrivate::callOnTransition(QEvent *e)
{
   Q_Q(QSignalTransition);
   q->onTransition(e);
}

#endif //QT_NO_STATEMACHINE
