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

#include <qkeyeventtransition.h>

#ifndef QT_NO_STATEMACHINE

#include <qbasickeyeventtransition_p.h>
#include <QtCore/qstatemachine.h>
#include <qeventtransition_p.h>

QT_BEGIN_NAMESPACE

class QKeyEventTransitionPrivate : public QEventTransitionPrivate
{
   Q_DECLARE_PUBLIC(QKeyEventTransition)
 public:
   QKeyEventTransitionPrivate() {}

   QBasicKeyEventTransition *transition;
};

/*!
  Constructs a new key event transition with the given \a sourceState.
*/
QKeyEventTransition::QKeyEventTransition(QState *sourceState)
   : QEventTransition(*new QKeyEventTransitionPrivate, sourceState)
{
   Q_D(QKeyEventTransition);
   d->transition = new QBasicKeyEventTransition();
}

/*!
  Constructs a new key event transition for events of the given \a type for
  the given \a object, with the given \a key and \a sourceState.
*/
QKeyEventTransition::QKeyEventTransition(QObject *object, QEvent::Type type,
      int key, QState *sourceState)
   : QEventTransition(*new QKeyEventTransitionPrivate, object, type, sourceState)
{
   Q_D(QKeyEventTransition);
   d->transition = new QBasicKeyEventTransition(type, key);
}

/*!
  Destroys this key event transition.
*/
QKeyEventTransition::~QKeyEventTransition()
{
   Q_D(QKeyEventTransition);
   delete d->transition;
}

/*!
  Returns the key that this key event transition checks for.
*/
int QKeyEventTransition::key() const
{
   Q_D(const QKeyEventTransition);
   return d->transition->key();
}

/*!
  Sets the key that this key event transition will check for.
*/
void QKeyEventTransition::setKey(int key)
{
   Q_D(QKeyEventTransition);
   d->transition->setKey(key);
}

/*!
  Returns the keyboard modifier mask that this key event transition checks
  for.
*/
Qt::KeyboardModifiers QKeyEventTransition::modifierMask() const
{
   Q_D(const QKeyEventTransition);
   return d->transition->modifierMask();
}

/*!
  Sets the keyboard modifier mask that this key event transition will
  check for to \a modifierMask.
*/
void QKeyEventTransition::setModifierMask(Qt::KeyboardModifiers modifierMask)
{
   Q_D(QKeyEventTransition);
   d->transition->setModifierMask(modifierMask);
}

/*!
  \reimp
*/
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

/*!
  \reimp
*/
void QKeyEventTransition::onTransition(QEvent *event)
{
   QEventTransition::onTransition(event);
}

QT_END_NAMESPACE

#endif //QT_NO_STATEMACHINE
