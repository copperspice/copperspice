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

#ifndef QHISTORYSTATE_P_H
#define QHISTORYSTATE_P_H

#include <qabstractstate_p.h>

#include <qabstracttransition.h>
#include <qhistorystate.h>
#include <qlist.h>

class QHistoryStatePrivate : public QAbstractStatePrivate
{
   Q_DECLARE_PUBLIC(QHistoryState)

 public:
   QHistoryStatePrivate();

   static QHistoryStatePrivate *get(QHistoryState *q);

   QAbstractTransition *defaultTransition;
   QHistoryState::HistoryType historyType;
   QList<QAbstractState *> configuration;
};

class DefaultStateTransition : public QAbstractTransition
{
   CORE_CS_OBJECT(DefaultStateTransition)

 public:
   DefaultStateTransition(QHistoryState *source, QAbstractState *target);

 protected:
   // It does not matter whether this transition matches any event or not. It is always associated
   // with a QHistoryState, and as soon as the state-machine detects that it enters a history
   // state, it will handle this transition as a special case. The history state itself is never
   // entered either: either the stored configuration will be used, or the target(s) of this
   // transition are used.

   bool eventTest(QEvent *event) override {
      (void) event;
      return false;
   }

   void onTransition(QEvent *event) override {
      (void) event;
   }
};

#endif
