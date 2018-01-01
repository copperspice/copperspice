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

#ifndef QABSTRACTSTATE_H
#define QABSTRACTSTATE_H

#include <QtCore/qobject.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_STATEMACHINE

class QState;
class QStateMachine;
class QAbstractStatePrivate;

class Q_CORE_EXPORT QAbstractState : public QObject
{
   CORE_CS_OBJECT(QAbstractState)

 public:
   ~QAbstractState();

   QState *parentState() const;
   QStateMachine *machine() const;

   CORE_CS_SIGNAL_1(Public, void entered())
   CORE_CS_SIGNAL_2(entered)

   CORE_CS_SIGNAL_1(Public, void exited())
   CORE_CS_SIGNAL_2(exited)

 protected:
   QAbstractState(QState *parent = 0);

   virtual void onEntry(QEvent *event) = 0;
   virtual void onExit(QEvent *event) = 0;

   bool event(QEvent *e) override;
   QAbstractState(QAbstractStatePrivate &dd, QState *parent);

   QScopedPointer<QAbstractStatePrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QAbstractState)
   Q_DECLARE_PRIVATE(QAbstractState)
  
};

#endif //QT_NO_STATEMACHINE

QT_END_NAMESPACE

#endif
