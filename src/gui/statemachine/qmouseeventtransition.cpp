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

#include <qmouseeventtransition.h>

#ifndef QT_NO_STATEMACHINE

#include <qbasicmouseeventtransition_p.h>
#include <QtCore/qstatemachine.h>
#include <QtGui/qpainterpath.h>
#include <qeventtransition_p.h>

QT_BEGIN_NAMESPACE

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

/*!
  Constructs a new mouse event transition with the given \a sourceState.
*/
QMouseEventTransition::QMouseEventTransition(QState *sourceState)
   : QEventTransition(*new QMouseEventTransitionPrivate, sourceState)
{
   Q_D(QMouseEventTransition);
   d->transition = new QBasicMouseEventTransition();
}

/*!
  Constructs a new mouse event transition for events of the given \a type for
  the given \a object, with the given \a button and \a sourceState.
*/
QMouseEventTransition::QMouseEventTransition(QObject *object, QEvent::Type type,
      Qt::MouseButton button,
      QState *sourceState)
   : QEventTransition(*new QMouseEventTransitionPrivate, object, type, sourceState)
{
   Q_D(QMouseEventTransition);
   d->transition = new QBasicMouseEventTransition(type, button);
}

/*!
  Destroys this mouse event transition.
*/
QMouseEventTransition::~QMouseEventTransition()
{
   Q_D(QMouseEventTransition);
   delete d->transition;
}

/*!
  Returns the button that this mouse event transition checks for.
*/
Qt::MouseButton QMouseEventTransition::button() const
{
   Q_D(const QMouseEventTransition);
   return d->transition->button();
}

/*!
  Sets the \a button that this mouse event transition will check for.
*/
void QMouseEventTransition::setButton(Qt::MouseButton button)
{
   Q_D(QMouseEventTransition);
   d->transition->setButton(button);
}

/*!
  Returns the keyboard modifier mask that this mouse event transition checks
  for.
*/
Qt::KeyboardModifiers QMouseEventTransition::modifierMask() const
{
   Q_D(const QMouseEventTransition);
   return d->transition->modifierMask();
}

/*!
  Sets the keyboard modifier mask that this mouse event transition will
  check for to \a modifierMask.
*/
void QMouseEventTransition::setModifierMask(Qt::KeyboardModifiers modifierMask)
{
   Q_D(QMouseEventTransition);
   d->transition->setModifierMask(modifierMask);
}

/*!
  Returns the hit test path for this mouse event transition.
*/
QPainterPath QMouseEventTransition::hitTestPath() const
{
   Q_D(const QMouseEventTransition);
   return d->transition->hitTestPath();
}

/*!
  Sets the hit test path for this mouse event transition to \a path.
  If a valid path has been set, the transition will only trigger if the mouse
  event position (QMouseEvent::pos()) is inside the path.

  \sa QPainterPath::contains()
*/
void QMouseEventTransition::setHitTestPath(const QPainterPath &path)
{
   Q_D(QMouseEventTransition);
   d->transition->setHitTestPath(path);
}

/*!
  \reimp
*/
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

/*!
  \reimp
*/
void QMouseEventTransition::onTransition(QEvent *event)
{
   QEventTransition::onTransition(event);
}

QT_END_NAMESPACE

#endif //QT_NO_STATEMACHINE
