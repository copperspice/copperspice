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

#include <qkeyeventtransition.h>

#ifndef QT_NO_STATEMACHINE

#include <qstatemachine.h>

#include <qbasickeyeventtransition_p.h>
#include <qeventtransition_p.h>

class QKeyEventTransitionPrivate : public QEventTransitionPrivate
{
   Q_DECLARE_PUBLIC(QKeyEventTransition)

 public:
   QKeyEventTransitionPrivate() {}

   QBasicKeyEventTransition *transition;
};

QKeyEventTransition::QKeyEventTransition(QState *sourceState)
   : QEventTransition(*new QKeyEventTransitionPrivate, sourceState)
{
   Q_D(QKeyEventTransition);
   d->transition = new QBasicKeyEventTransition();
}

QKeyEventTransition::QKeyEventTransition(QObject *object, QEvent::Type type,
      int key, QState *sourceState)
   : QEventTransition(*new QKeyEventTransitionPrivate, object, type, sourceState)
{
   Q_D(QKeyEventTransition);
   d->transition = new QBasicKeyEventTransition(type, key);
}

QKeyEventTransition::~QKeyEventTransition()
{
   Q_D(QKeyEventTransition);
   delete d->transition;
}

int QKeyEventTransition::key() const
{
   Q_D(const QKeyEventTransition);
   return d->transition->key();
}

void QKeyEventTransition::setKey(int key)
{
   Q_D(QKeyEventTransition);
   d->transition->setKey(key);
}

Qt::KeyboardModifiers QKeyEventTransition::modifierMask() const
{
   Q_D(const QKeyEventTransition);
   return d->transition->modifierMask();
}

void QKeyEventTransition::setModifierMask(Qt::KeyboardModifiers modifierMask)
{
   Q_D(QKeyEventTransition);
   d->transition->setModifierMask(modifierMask);
}

bool QKeyEventTransition::eventTest(QEvent *event)
{
   Q_D(const QKeyEventTransition);
   if (!QEventTransition::eventTest(event)) {
      return false;
   }
   QStateMachine::WrappedEvent *we = static_cast<QStateMachine::WrappedEvent *>(event);
   d->transition->setEventType(we->event()->type());
   return QAbstractTransitionPrivate::get(d->transition)->callEventTest(we->event());
}

void QKeyEventTransition::onTransition(QEvent *event)
{
   QEventTransition::onTransition(event);
}

#endif //QT_NO_STATEMACHINE
