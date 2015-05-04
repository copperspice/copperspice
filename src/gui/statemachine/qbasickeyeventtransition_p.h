/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QBASICKEYEVENTTRANSITION_P_H
#define QBASICKEYEVENTTRANSITION_P_H

#include <QtCore/qabstracttransition.h>

#ifndef QT_NO_STATEMACHINE

#include <QtGui/qevent.h>

QT_BEGIN_NAMESPACE

class QBasicKeyEventTransitionPrivate;

class QBasicKeyEventTransition : public QAbstractTransition
{
   CS_OBJECT(QBasicKeyEventTransition)

 public:
   QBasicKeyEventTransition(QState *sourceState = 0);
   QBasicKeyEventTransition(QEvent::Type type, int key, QState *sourceState = 0);
   QBasicKeyEventTransition(QEvent::Type type, int key, Qt::KeyboardModifiers modifierMask, QState *sourceState = 0);
   ~QBasicKeyEventTransition();

   QEvent::Type eventType() const;
   void setEventType(QEvent::Type type);

   int key() const;
   void setKey(int key);

   Qt::KeyboardModifiers modifierMask() const;
   void setModifierMask(Qt::KeyboardModifiers modifiers);

 protected:
   bool eventTest(QEvent *event);
   void onTransition(QEvent *);

 private:
   Q_DISABLE_COPY(QBasicKeyEventTransition)
   Q_DECLARE_PRIVATE(QBasicKeyEventTransition)
};

QT_END_NAMESPACE

#endif //QT_NO_STATEMACHINE

#endif
