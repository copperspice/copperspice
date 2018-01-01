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

#ifndef QSIGNALTRANSITION_H
#define QSIGNALTRANSITION_H

#include <QtCore/qabstracttransition.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_STATEMACHINE

class Q_CORE_EXPORT QSignalTransition : public QAbstractTransition
{
   CORE_CS_OBJECT(QSignalTransition)

   CORE_CS_PROPERTY_READ(senderObject,  senderObject)
   CORE_CS_PROPERTY_WRITE(senderObject, setSenderObject)

 public:
   QSignalTransition(QState *sourceState = 0);

   template<class SignalClass, class ...SignalArgs>
   QSignalTransition(QObject *sender, void (SignalClass::*signal)(SignalArgs...), QState *sourceState = 0);

   ~QSignalTransition();

   QObject *senderObject() const;
   void setSenderObject(QObject *sender);

   CsSignal::Internal::BentoAbstract *get_signalBento() const;

   void unregister();
   void maybeRegister();

   virtual void callOnTransition(QEvent *e);

 protected:
   bool eventTest(QEvent *event) override;
   void onTransition(QEvent *event) override;
   bool event(QEvent *e) override;

 private:
   Q_DISABLE_COPY(QSignalTransition)

   QObject *m_sender;
   QScopedPointer<CsSignal::Internal::BentoAbstract> m_signalBento;
};


template<class SignalClass, class ...SignalArgs>
QSignalTransition::QSignalTransition(QObject *sender, void (SignalClass::*signal)(SignalArgs...), QState *sourceState)
   : QAbstractTransition(sourceState)
{
   m_sender = sender;

   // store the signal method pointer in a CSBento
   m_signalBento.reset(new CSBento<void (SignalClass::*)(SignalArgs...)> {signal});
}

#endif //QT_NO_STATEMACHINE

QT_END_NAMESPACE

#endif
