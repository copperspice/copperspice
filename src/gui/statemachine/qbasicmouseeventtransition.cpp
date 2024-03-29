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

#include <qbasicmouseeventtransition_p.h>

#ifndef QT_NO_STATEMACHINE

#include <qdebug.h>
#include <qevent.h>
#include <qpainterpath.h>

#include <qabstracttransition_p.h>

class QBasicMouseEventTransitionPrivate : public QAbstractTransitionPrivate
{
   Q_DECLARE_PUBLIC(QBasicMouseEventTransition)

 public:
   QBasicMouseEventTransitionPrivate();

   static QBasicMouseEventTransitionPrivate *get(QBasicMouseEventTransition *q);

   QEvent::Type eventType;
   Qt::MouseButton button;
   Qt::KeyboardModifiers modifierMask;
   QPainterPath path;
};

QBasicMouseEventTransitionPrivate::QBasicMouseEventTransitionPrivate()
{
   eventType = QEvent::None;
   button = Qt::NoButton;
}

QBasicMouseEventTransitionPrivate *QBasicMouseEventTransitionPrivate::get(QBasicMouseEventTransition *q)
{
   return q->d_func();
}

QBasicMouseEventTransition::QBasicMouseEventTransition(QState *sourceState)
   : QAbstractTransition(*new QBasicMouseEventTransitionPrivate, sourceState)
{
}

QBasicMouseEventTransition::QBasicMouseEventTransition(QEvent::Type type,
      Qt::MouseButton button,
      QState *sourceState)
   : QAbstractTransition(*new QBasicMouseEventTransitionPrivate, sourceState)
{
   Q_D(QBasicMouseEventTransition);
   d->eventType = type;
   d->button = button;
}

QBasicMouseEventTransition::~QBasicMouseEventTransition()
{
}

QEvent::Type QBasicMouseEventTransition::eventType() const
{
   Q_D(const QBasicMouseEventTransition);
   return d->eventType;
}

void QBasicMouseEventTransition::setEventType(QEvent::Type type)
{
   Q_D(QBasicMouseEventTransition);
   d->eventType = type;
}

Qt::MouseButton QBasicMouseEventTransition::button() const
{
   Q_D(const QBasicMouseEventTransition);
   return d->button;
}

void QBasicMouseEventTransition::setButton(Qt::MouseButton button)
{
   Q_D(QBasicMouseEventTransition);
   d->button = button;
}

Qt::KeyboardModifiers QBasicMouseEventTransition::modifierMask() const
{
   Q_D(const QBasicMouseEventTransition);
   return d->modifierMask;
}

void QBasicMouseEventTransition::setModifierMask(Qt::KeyboardModifiers modifierMask)
{
   Q_D(QBasicMouseEventTransition);
   d->modifierMask = modifierMask;
}

QPainterPath QBasicMouseEventTransition::hitTestPath() const
{
   Q_D(const QBasicMouseEventTransition);
   return d->path;
}

void QBasicMouseEventTransition::setHitTestPath(const QPainterPath &path)
{
   Q_D(QBasicMouseEventTransition);
   d->path = path;
}

bool QBasicMouseEventTransition::eventTest(QEvent *event)
{
   Q_D(const QBasicMouseEventTransition);
   if (event->type() == d->eventType) {
      QMouseEvent *me = static_cast<QMouseEvent *>(event);
      return (me->button() == d->button)
             && ((me->modifiers() & d->modifierMask) == d->modifierMask)
             && (d->path.isEmpty() || d->path.contains(me->pos()));
   }
   return false;
}

void QBasicMouseEventTransition::onTransition(QEvent *)
{
}

#endif //QT_NO_STATEMACHINE
