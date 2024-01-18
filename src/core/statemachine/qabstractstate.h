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

#ifndef QABSTRACTSTATE_H
#define QABSTRACTSTATE_H

#include <qobject.h>
#include <qscopedpointer.h>

#ifndef QT_NO_STATEMACHINE

class QState;
class QStateMachine;
class QAbstractStatePrivate;

class Q_CORE_EXPORT QAbstractState : public QObject
{
   CORE_CS_OBJECT(QAbstractState)

   CORE_CS_PROPERTY_READ(active, active)
   CORE_CS_PROPERTY_NOTIFY(active, activeChanged)

 public:
   QAbstractState(const QAbstractState &) = delete;
   QAbstractState &operator=(const QAbstractState &) = delete;

   ~QAbstractState();

   QState *parentState() const;
   QStateMachine *machine() const;

   bool active() const;

   CORE_CS_SIGNAL_1(Public, void entered())
   CORE_CS_SIGNAL_2(entered)

   CORE_CS_SIGNAL_1(Public, void exited())
   CORE_CS_SIGNAL_2(exited)

   CORE_CS_SIGNAL_1(Public, void activeChanged(bool active))
   CORE_CS_SIGNAL_2(activeChanged, active)

 protected:
   QAbstractState(QState *parent = nullptr);

   virtual void onEntry(QEvent *event) = 0;
   virtual void onExit(QEvent *event) = 0;

   bool event(QEvent *event) override;
   QAbstractState(QAbstractStatePrivate &dd, QState *parent);

   QScopedPointer<QAbstractStatePrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QAbstractState)
};

#endif //QT_NO_STATEMACHINE

#endif
