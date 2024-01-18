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

#ifndef QABSTRACTTRANSITION_P_H
#define QABSTRACTTRANSITION_P_H

#include <qlist.h>
#include <qvector.h>
#include <qsharedpointer.h>

class QAbstractState;
class QState;
class QStateMachine;
class QAbstractTransition;

class Q_CORE_EXPORT QAbstractTransitionPrivate
{
   Q_DECLARE_PUBLIC(QAbstractTransition)

 public:
   QAbstractTransitionPrivate();

   virtual ~QAbstractTransitionPrivate()
   { }

   static QAbstractTransitionPrivate *get(QAbstractTransition *q);

   bool callEventTest(QEvent *e);
   virtual void callOnTransition(QEvent *e);

   QState *sourceState() const;
   QStateMachine *machine() const;
   void emitTriggered();

   QVector<QPointer<QAbstractState>> targetStates;
   QAbstractTransition::TransitionType transitionType;

#ifndef QT_NO_ANIMATION
   QList<QAbstractAnimation *> animations;
#endif

 protected:
   QAbstractTransition *q_ptr;
};

#endif
