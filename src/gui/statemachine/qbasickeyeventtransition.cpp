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

#include <qbasickeyeventtransition_p.h>

#ifndef QT_NO_STATEMACHINE

#include <qdebug.h>
#include <qevent.h>

#include <qabstracttransition_p.h>

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

QBasicKeyEventTransition::QBasicKeyEventTransition(QState *sourceState)
   : QAbstractTransition(*new QBasicKeyEventTransitionPrivate, sourceState)
{
}

QBasicKeyEventTransition::QBasicKeyEventTransition(QEvent::Type type, int key,
      QState *sourceState)
   : QAbstractTransition(*new QBasicKeyEventTransitionPrivate, sourceState)
{
   Q_D(QBasicKeyEventTransition);
   d->eventType = type;
   d->key = key;
}

QBasicKeyEventTransition::QBasicKeyEventTransition(QEvent::Type type, int key,
      Qt::KeyboardModifiers modifierMask, QState *sourceState)
   : QAbstractTransition(*new QBasicKeyEventTransitionPrivate, sourceState)
{
   Q_D(QBasicKeyEventTransition);
   d->eventType = type;
   d->key = key;
   d->modifierMask = modifierMask;
}

QBasicKeyEventTransition::~QBasicKeyEventTransition()
{
}

QEvent::Type QBasicKeyEventTransition::eventType() const
{
   Q_D(const QBasicKeyEventTransition);
   return d->eventType;
}

void QBasicKeyEventTransition::setEventType(QEvent::Type type)
{
   Q_D(QBasicKeyEventTransition);
   d->eventType = type;
}

int QBasicKeyEventTransition::key() const
{
   Q_D(const QBasicKeyEventTransition);
   return d->key;
}

void QBasicKeyEventTransition::setKey(int key)
{
   Q_D(QBasicKeyEventTransition);
   d->key = key;
}

Qt::KeyboardModifiers QBasicKeyEventTransition::modifierMask() const
{
   Q_D(const QBasicKeyEventTransition);
   return d->modifierMask;
}

void QBasicKeyEventTransition::setModifierMask(Qt::KeyboardModifiers modifierMask)
{
   Q_D(QBasicKeyEventTransition);
   d->modifierMask = modifierMask;
}

bool QBasicKeyEventTransition::eventTest(QEvent *event)
{
   Q_D(const QBasicKeyEventTransition);

   if (event->type() == d->eventType) {
      QKeyEvent *ke = static_cast<QKeyEvent *>(event);

      return (ke->key() == d->key) && ((ke->modifiers() & d->modifierMask) == d->modifierMask);
   }

   return false;
}

void QBasicKeyEventTransition::onTransition(QEvent *)
{
}

#endif //QT_NO_STATEMACHINE
