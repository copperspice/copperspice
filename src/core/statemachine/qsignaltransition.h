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

#ifndef QSIGNALTRANSITION_H
#define QSIGNALTRANSITION_H

#include <qabstracttransition.h>

#ifndef QT_NO_STATEMACHINE

class QSignalTransitionPrivate;

class Q_CORE_EXPORT QSignalTransition : public QAbstractTransition
{
   CORE_CS_OBJECT(QSignalTransition)

   CORE_CS_PROPERTY_READ(senderObject,   senderObject)
   CORE_CS_PROPERTY_WRITE(senderObject,  setSenderObject)
   CORE_CS_PROPERTY_NOTIFY(senderObject, senderObjectChanged)

 public:
   QSignalTransition(QState *sourceState = nullptr);

   template <class SignalClass, class ...SignalArgs>
   QSignalTransition(QObject *sender, void (SignalClass::*signal)(SignalArgs...), QState *sourceState = nullptr);

   QSignalTransition(const QSignalTransition &) = delete;
   QSignalTransition &operator=(const QSignalTransition &) = delete;

   ~QSignalTransition();

   const QObject *senderObject() const;
   void setSenderObject(const QObject *sender);

   CsSignal::Internal::BentoAbstract *get_signalBento() const;

   void unregister();
   void maybeRegister();

   CORE_CS_SIGNAL_1(Public, void senderObjectChanged())
   CORE_CS_SIGNAL_2(senderObjectChanged)

 protected:
   bool eventTest(QEvent *event) override;
   void onTransition(QEvent *event) override;
   bool event(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QSignalTransition)

   const QObject *m_sender;
   QScopedPointer<CsSignal::Internal::BentoAbstract> m_signalBento;
};

template <class SignalClass, class ...SignalArgs>
QSignalTransition::QSignalTransition(QObject *sender, void (SignalClass::*signal)(SignalArgs...), QState *sourceState)
   : QAbstractTransition(sourceState)
{
   m_sender = sender;

   // store the signal method pointer in a CSBento
   m_signalBento.reset(new CSBento<void (SignalClass::*)(SignalArgs...)> {signal});
}

template <class SignalClass, class ...SignalArgs>
QSignalTransition *QState::addTransition(QObject *sender, void (SignalClass::*signal)(SignalArgs...),
      QAbstractState *target)
{
   if (! sender) {
      qWarning("QState::addTransition() No sender was specified");
      return nullptr;
   }

   if (! signal) {
      qWarning("QState::addTransition() No signal was specified");
      return nullptr;
   }

   if (! target) {
      qWarning("QState::addTransition() No target was specified");
      return nullptr;
   }

   QSignalTransition *trans = new QSignalTransition(sender, signal);
   trans->setTargetState(target);
   addTransition(trans);

   return trans;
}

#endif // QT_NO_STATEMACHINE

#endif
