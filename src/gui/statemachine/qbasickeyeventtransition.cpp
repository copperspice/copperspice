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

#include <qbasickeyeventtransition_p.h>

#ifndef QT_NO_STATEMACHINE

#include <QtGui/qevent.h>
#include <qdebug.h>
#include <qabstracttransition_p.h>

QT_BEGIN_NAMESPACE

/*!
  \internal
  \class QBasicKeyEventTransition
  \since 4.6
  \ingroup statemachine

  \brief The QBasicKeyEventTransition class provides a transition for Qt key events.
*/

class QBasicKeyEventTransitionPrivate : public QAbstractTransitionPrivate
{
   Q_DECLARE_PUBLIC(QBasicKeyEventTransition)
 public:
   QBasicKeyEventTransitionPrivate();

   static QBasicKeyEventTransitionPrivate *get(QBasicKeyEventTransition *q);

   QEvent::Type eventType;
   int key;
   Qt::KeyboardModifiers modifierMask;
};

QBasicKeyEventTransitionPrivate::QBasicKeyEventTransitionPrivate()
{
   eventType = QEvent::None;
   key = 0;
   modifierMask = Qt::NoModifier;
}

QBasicKeyEventTransitionPrivate *QBasicKeyEventTransitionPrivate::get(QBasicKeyEventTransition *q)
{
   return q->d_func();
}

/*!
  Constructs a new key event transition with the given \a sourceState.
*/
QBasicKeyEventTransition::QBasicKeyEventTransition(QState *sourceState)
   : QAbstractTransition(*new QBasicKeyEventTransitionPrivate, sourceState)
{
}

/*!
  Constructs a new event transition for events of the given \a type for the
  given \a key, with the given \a sourceState.
*/
QBasicKeyEventTransition::QBasicKeyEventTransition(QEvent::Type type, int key,
      QState *sourceState)
   : QAbstractTransition(*new QBasicKeyEventTransitionPrivate, sourceState)
{
   Q_D(QBasicKeyEventTransition);
   d->eventType = type;
   d->key = key;
}

/*!
  Constructs a new event transition for events of the given \a type for the
  given \a key, with the given \a modifierMask and \a sourceState.
*/
QBasicKeyEventTransition::QBasicKeyEventTransition(QEvent::Type type, int key,
      Qt::KeyboardModifiers modifierMask,
      QState *sourceState)
   : QAbstractTransition(*new QBasicKeyEventTransitionPrivate, sourceState)
{
   Q_D(QBasicKeyEventTransition);
   d->eventType = type;
   d->key = key;
   d->modifierMask = modifierMask;
}

/*!
  Destroys this event transition.
*/
QBasicKeyEventTransition::~QBasicKeyEventTransition()
{
}

/*!
  Returns the event type that this key event transition is associated with.
*/
QEvent::Type QBasicKeyEventTransition::eventType() const
{
   Q_D(const QBasicKeyEventTransition);
   return d->eventType;
}

/*!
  Sets the event \a type that this key event transition is associated with.
*/
void QBasicKeyEventTransition::setEventType(QEvent::Type type)
{
   Q_D(QBasicKeyEventTransition);
   d->eventType = type;
}

/*!
  Returns the key that this key event transition checks for.
*/
int QBasicKeyEventTransition::key() const
{
   Q_D(const QBasicKeyEventTransition);
   return d->key;
}

/*!
  Sets the key that this key event transition will check for.
*/
void QBasicKeyEventTransition::setKey(int key)
{
   Q_D(QBasicKeyEventTransition);
   d->key = key;
}

/*!
  Returns the keyboard modifier mask that this key event transition checks
  for.
*/
Qt::KeyboardModifiers QBasicKeyEventTransition::modifierMask() const
{
   Q_D(const QBasicKeyEventTransition);
   return d->modifierMask;
}

/*!
  Sets the keyboard modifier mask that this key event transition will check
  for.
*/
void QBasicKeyEventTransition::setModifierMask(Qt::KeyboardModifiers modifierMask)
{
   Q_D(QBasicKeyEventTransition);
   d->modifierMask = modifierMask;
}

/*!
  \reimp
*/
bool QBasicKeyEventTransition::eventTest(QEvent *event)
{
   Q_D(const QBasicKeyEventTransition);
   if (event->type() == d->eventType) {
      QKeyEvent *ke = static_cast<QKeyEvent *>(event);
      return (ke->key() == d->key)
             && ((ke->modifiers() & d->modifierMask) == d->modifierMask);
   }
   return false;
}

/*!
  \reimp
*/
void QBasicKeyEventTransition::onTransition(QEvent *)
{
}

QT_END_NAMESPACE

#endif //QT_NO_STATEMACHINE
