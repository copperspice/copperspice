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

#ifndef QEVENTTRANSITION_H
#define QEVENTTRANSITION_H

#include <qabstracttransition.h>
#include <qcoreevent.h>

#ifndef QT_NO_STATEMACHINE

class QEventTransitionPrivate;

class Q_CORE_EXPORT QEventTransition : public QAbstractTransition
{
   CORE_CS_OBJECT(QEventTransition)

   CORE_CS_PROPERTY_READ(eventSource,  eventSource)
   CORE_CS_PROPERTY_WRITE(eventSource, setEventSource)

   CORE_CS_PROPERTY_READ(eventType,  eventType)
   CORE_CS_PROPERTY_WRITE(eventType, setEventType)

 public:
   QEventTransition(QState *sourceState = nullptr);
   QEventTransition(QObject *object, QEvent::Type type, QState *sourceState = nullptr);

   QEventTransition(const QEventTransition &) = delete;
   QEventTransition &operator=(const QEventTransition &) = delete;

   ~QEventTransition();

   QObject *eventSource() const;
   void setEventSource(QObject *object);

   QEvent::Type eventType() const;
   void setEventType(QEvent::Type type);

 protected:
   bool eventTest(QEvent *event) override;
   void onTransition(QEvent *event) override;

   bool event(QEvent *event) override;

   QEventTransition(QEventTransitionPrivate &dd, QState *parent);
   QEventTransition(QEventTransitionPrivate &dd, QObject *object, QEvent::Type type, QState *parent);

 private:
   Q_DECLARE_PRIVATE(QEventTransition)
};

#endif //QT_NO_STATEMACHINE

#endif
