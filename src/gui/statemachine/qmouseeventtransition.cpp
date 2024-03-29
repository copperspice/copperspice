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

#include <qmouseeventtransition.h>

#ifndef QT_NO_STATEMACHINE

#include <qpainterpath.h>
#include <qstatemachine.h>

#include <qbasicmouseeventtransition_p.h>
#include <qeventtransition_p.h>

class QMouseEventTransitionPrivate : public QEventTransitionPrivate
{
   Q_DECLARE_PUBLIC(QMouseEventTransition)

 public:
   QMouseEventTransitionPrivate();

   QBasicMouseEventTransition *transition;
};

QMouseEventTransitionPrivate::QMouseEventTransitionPrivate()
{
}

QMouseEventTransition::QMouseEventTransition(QState *sourceState)
   : QEventTransition(*new QMouseEventTransitionPrivate, sourceState)
{
   Q_D(QMouseEventTransition);
   d->transition = new QBasicMouseEventTransition();
}

QMouseEventTransition::QMouseEventTransition(QObject *object, QEvent::Type type,
      Qt::MouseButton button,
      QState *sourceState)
   : QEventTransition(*new QMouseEventTransitionPrivate, object, type, sourceState)
{
   Q_D(QMouseEventTransition);
   d->transition = new QBasicMouseEventTransition(type, button);
}

QMouseEventTransition::~QMouseEventTransition()
{
   Q_D(QMouseEventTransition);
   delete d->transition;
}

Qt::MouseButton QMouseEventTransition::button() const
{
   Q_D(const QMouseEventTransition);
   return d->transition->button();
}

void QMouseEventTransition::setButton(Qt::MouseButton button)
{
   Q_D(QMouseEventTransition);
   d->transition->setButton(button);
}

Qt::KeyboardModifiers QMouseEventTransition::modifierMask() const
{
   Q_D(const QMouseEventTransition);
   return d->transition->modifierMask();
}

void QMouseEventTransition::setModifierMask(Qt::KeyboardModifiers modifierMask)
{
   Q_D(QMouseEventTransition);
   d->transition->setModifierMask(modifierMask);
}

QPainterPath QMouseEventTransition::hitTestPath() const
{
   Q_D(const QMouseEventTransition);
   return d->transition->hitTestPath();
}

void QMouseEventTransition::setHitTestPath(const QPainterPath &path)
{
   Q_D(QMouseEventTransition);
   d->transition->setHitTestPath(path);
}

bool QMouseEventTransition::eventTest(QEvent *event)
{
   Q_D(const QMouseEventTransition);
   if (!QEventTransition::eventTest(event)) {
      return false;
   }
   QStateMachine::WrappedEvent *we = static_cast<QStateMachine::WrappedEvent *>(event);
   d->transition->setEventType(we->event()->type());
   return QAbstractTransitionPrivate::get(d->transition)->callEventTest(we->event());
}

void QMouseEventTransition::onTransition(QEvent *event)
{
   QEventTransition::onTransition(event);
}

#endif //QT_NO_STATEMACHINE
