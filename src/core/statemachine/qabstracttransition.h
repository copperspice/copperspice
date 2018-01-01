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

#ifndef QABSTRACTTRANSITION_H
#define QABSTRACTTRANSITION_H

#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_STATEMACHINE

class QEvent;
class QAbstractState;
class QState;
class QStateMachine;
class QAbstractTransitionPrivate;

#ifndef QT_NO_ANIMATION
class QAbstractAnimation;
#endif

class Q_CORE_EXPORT QAbstractTransition : public QObject
{
   CORE_CS_OBJECT(QAbstractTransition)

   CORE_CS_PROPERTY_READ(sourceState, sourceState)
   CORE_CS_PROPERTY_READ(targetState, targetState)
   CORE_CS_PROPERTY_WRITE(targetState, setTargetState)
   CORE_CS_PROPERTY_READ(targetStates, targetStates)
   CORE_CS_PROPERTY_WRITE(targetStates, setTargetStates)

 public:
   QAbstractTransition(QState *sourceState = 0);
   virtual ~QAbstractTransition();

   QState *sourceState() const;
   QAbstractState *targetState() const;
   void setTargetState(QAbstractState *target);
   QList<QAbstractState *> targetStates() const;
   void setTargetStates(const QList<QAbstractState *> &targets);

   QStateMachine *machine() const;

#ifndef QT_NO_ANIMATION
   void addAnimation(QAbstractAnimation *animation);
   void removeAnimation(QAbstractAnimation *animation);
   QList<QAbstractAnimation *> animations() const;
#endif

   CORE_CS_SIGNAL_1(Public, void triggered())
   CORE_CS_SIGNAL_2(triggered)

 protected:
   virtual bool eventTest(QEvent *event) = 0;

   virtual void onTransition(QEvent *event) = 0;

   bool event(QEvent *e) override;

   QAbstractTransition(QAbstractTransitionPrivate &dd, QState *parent);

   QScopedPointer<QAbstractTransitionPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QAbstractTransition)
   Q_DECLARE_PRIVATE(QAbstractTransition)

};

#endif //QT_NO_STATEMACHINE

QT_END_NAMESPACE

#endif
