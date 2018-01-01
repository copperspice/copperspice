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

#ifndef QEVENTTRANSITION_H
#define QEVENTTRANSITION_H

#include <QtCore/qabstracttransition.h>
#include <QtCore/qcoreevent.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_STATEMACHINE

class QEventTransitionPrivate;

class Q_CORE_EXPORT QEventTransition : public QAbstractTransition
{
   CORE_CS_OBJECT(QEventTransition)

   CORE_CS_PROPERTY_READ(eventSource, eventSource)
   CORE_CS_PROPERTY_WRITE(eventSource, setEventSource)
   CORE_CS_PROPERTY_READ(eventType, eventType)
   CORE_CS_PROPERTY_WRITE(eventType, setEventType)

 public:
   QEventTransition(QState *sourceState = 0);
   QEventTransition(QObject *object, QEvent::Type type, QState *sourceState = 0);
   ~QEventTransition();

   QObject *eventSource() const;
   void setEventSource(QObject *object);

   QEvent::Type eventType() const;
   void setEventType(QEvent::Type type);

 protected:
   bool eventTest(QEvent *event) override;
   void onTransition(QEvent *event) override;

   bool event(QEvent *e) override;

   QEventTransition(QEventTransitionPrivate &dd, QState *parent);
   QEventTransition(QEventTransitionPrivate &dd, QObject *object, QEvent::Type type, QState *parent);

 private:
   Q_DISABLE_COPY(QEventTransition)
   Q_DECLARE_PRIVATE(QEventTransition)
};

#endif //QT_NO_STATEMACHINE

QT_END_NAMESPACE

#endif
