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

#ifndef QABSTRACTSTATE_P_H
#define QABSTRACTSTATE_P_H

QT_BEGIN_NAMESPACE

class QStateMachine;
class QAbstractState;

class QAbstractStatePrivate
{
   Q_DECLARE_PUBLIC(QAbstractState)

 public:
   virtual ~QAbstractStatePrivate() {}

   enum StateType {
      AbstractState,
      StandardState,
      FinalState,
      HistoryState
   };

   QAbstractStatePrivate(StateType type);

   static QAbstractStatePrivate *get(QAbstractState *q);
   static const QAbstractStatePrivate *get(const QAbstractState *q);

   QStateMachine *machine() const;

   void callOnEntry(QEvent *e);
   void callOnExit(QEvent *e);

   void emitEntered();
   void emitExited();

   uint stateType: 31;
   uint isMachine: 1;
   mutable QState *parentState;

 protected:
   QAbstractState *q_ptr;

};

QT_END_NAMESPACE

#endif
