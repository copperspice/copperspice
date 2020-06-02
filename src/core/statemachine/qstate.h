/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QSTATE_H
#define QSTATE_H

#include <qabstractstate.h>
#include <qlist.h>

#ifndef QT_NO_STATEMACHINE

class QAbstractTransition;
class QSignalTransition;
class QStatePrivate;

class Q_CORE_EXPORT QState : public QAbstractState
{
   CORE_CS_OBJECT(QState)

   CORE_CS_PROPERTY_READ(initialState, initialState)
   CORE_CS_PROPERTY_WRITE(initialState, setInitialState)

   CORE_CS_PROPERTY_READ(errorState, errorState)
   CORE_CS_PROPERTY_WRITE(errorState, setErrorState)

   CORE_CS_PROPERTY_READ(childMode, childMode)
   CORE_CS_PROPERTY_WRITE(childMode, setChildMode)

 public:
   enum ChildMode {
      ExclusiveStates,
      ParallelStates
   };

   CORE_CS_ENUM(ChildMode)

   QState(QState *parent = nullptr);
   QState(ChildMode childMode, QState *parent = nullptr);
   ~QState();

   void addTransition(QAbstractTransition *transition);

   template<class SignalClass, class ...SignalArgs>
   QSignalTransition *addTransition(QObject *sender, void (SignalClass::*signal)(SignalArgs...), QAbstractState *target);

   QAbstractTransition *addTransition(QAbstractState *target);
   void removeTransition(QAbstractTransition *transition);
   QList<QAbstractTransition *> transitions() const;

   QAbstractState *initialState() const;
   void setInitialState(QAbstractState *state);

   ChildMode childMode() const;
   void setChildMode(ChildMode mode);

   QAbstractState *errorState() const;
   void setErrorState(QAbstractState *state);

#ifndef QT_NO_PROPERTIES
   void assignProperty(QObject *object, const char *name, const QVariant &value);
#endif

 public:
   CORE_CS_SIGNAL_1(Public, void finished())
   CORE_CS_SIGNAL_2(finished)

   CORE_CS_SIGNAL_1(Public, void propertiesAssigned())
   CORE_CS_SIGNAL_2(propertiesAssigned)

 protected:
   void onEntry(QEvent *event) override;
   void onExit(QEvent *event) override;

   bool event(QEvent *e) override;

   QState(QStatePrivate &dd, QState *parent);

 private:
   Q_DISABLE_COPY(QState)
   Q_DECLARE_PRIVATE(QState)
};

#endif // QT_NO_STATEMACHINE

#endif
