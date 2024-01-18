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

#ifndef QDECLARATIVENOTIFIER_P_H
#define QDECLARATIVENOTIFIER_P_H

#include <qdeclarativeguard_p.h>
#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

class QDeclarativeNotifierEndpoint;
class QDeclarativeNotifier
{
 public:
   inline QDeclarativeNotifier();
   inline ~QDeclarativeNotifier();
   inline void notify();

 private:
   friend class QDeclarativeNotifierEndpoint;

   static void emitNotify(QDeclarativeNotifierEndpoint *);
   QDeclarativeNotifierEndpoint *endpoints;
};

class QDeclarativeNotifierEndpoint
{
 public:
   inline QDeclarativeNotifierEndpoint();
   inline QDeclarativeNotifierEndpoint(QObject *t, int m);
   inline ~QDeclarativeNotifierEndpoint();

   QObject *target;
   int targetMethod;

   inline bool isConnected();
   inline bool isConnected(QObject *source, int sourceSignal);
   inline bool isConnected(QDeclarativeNotifier *);

   void connect(QObject *source, int sourceSignal);
   inline void connect(QDeclarativeNotifier *);

   // Disconnects unconditionally, regardless of the refcount
   inline void disconnect();

   // Decreases the refcount and disconnects when refcount reaches 0
   inline void deref();

   void copyAndClear(QDeclarativeNotifierEndpoint &other);

 private:
   friend class QDeclarativeNotifier;

   struct Signal {
      QDeclarativeGuard<QObject> source;
      int sourceSignal;
   };

   struct Notifier {
      QDeclarativeNotifier *notifier;
      QDeclarativeNotifierEndpoint **disconnected;

      QDeclarativeNotifierEndpoint  *next;
      QDeclarativeNotifierEndpoint **prev;
   };

   enum { InvalidType, SignalType, NotifierType } type;
   union {
      struct {
         Signal *signal;
         union {
            char signalData[sizeof(Signal)];
            qint64 q_for_alignment_1;
            double q_for_alignment_2;
         };
      } signal;
      Notifier notifier;
   };

   quint16 refCount;

   inline Notifier *toNotifier();
   inline Notifier *asNotifier();
   inline Signal *toSignal();
   inline Signal *asSignal();
};

QDeclarativeNotifier::QDeclarativeNotifier()
   : endpoints(0)
{
}

QDeclarativeNotifier::~QDeclarativeNotifier()
{
   QDeclarativeNotifierEndpoint *endpoint = endpoints;
   while (endpoint) {
      QDeclarativeNotifierEndpoint::Notifier *n = endpoint->asNotifier();
      endpoint = n->next;

      n->next = 0;
      n->prev = 0;
      n->notifier = 0;
      if (n->disconnected) {
         *n->disconnected = 0;
      }
      n->disconnected = 0;
   }
   endpoints = 0;
}

void QDeclarativeNotifier::notify()
{
   if (endpoints) {
      emitNotify(endpoints);
   }
}

QDeclarativeNotifierEndpoint::QDeclarativeNotifierEndpoint()
   : target(0), targetMethod(0), type(InvalidType), refCount(0)
{
}

QDeclarativeNotifierEndpoint::QDeclarativeNotifierEndpoint(QObject *t, int m)
   : target(t), targetMethod(m), type(InvalidType), refCount(0)
{
}

QDeclarativeNotifierEndpoint::~QDeclarativeNotifierEndpoint()
{
   disconnect();
   if (SignalType == type) {
      Signal *s = asSignal();
      s->~Signal();
   }
}

bool QDeclarativeNotifierEndpoint::isConnected()
{
   if (SignalType == type) {
      return asSignal()->source;
   } else if (NotifierType == type) {
      return asNotifier()->notifier;
   } else {
      return false;
   }
}

bool QDeclarativeNotifierEndpoint::isConnected(QObject *source, int sourceSignal)
{
   return SignalType == type && asSignal()->source == source && asSignal()->sourceSignal == sourceSignal;
}

bool QDeclarativeNotifierEndpoint::isConnected(QDeclarativeNotifier *notifier)
{
   return NotifierType == type && asNotifier()->notifier == notifier;
}

void QDeclarativeNotifierEndpoint::connect(QDeclarativeNotifier *notifier)
{
   Notifier *n = toNotifier();

   if (n->notifier == notifier) {
      refCount++;
      return;
   }

   disconnect();

   n->next = notifier->endpoints;
   if (n->next) {
      n->next->asNotifier()->prev = &n->next;
   }
   notifier->endpoints = this;
   n->prev = &notifier->endpoints;
   n->notifier = notifier;
   refCount++;
}

void QDeclarativeNotifierEndpoint::disconnect()
{
   if (type == SignalType) {
      Signal *s = asSignal();
      if (s->source) {
         QMetaObject::disconnectOne(s->source, s->sourceSignal, target, targetMethod);
         QObjectPrivate *const priv = QObjectPrivate::get(s->source);
         const QMetaMethod signal = s->source->metaObject()->method(s->sourceSignal);
         QVarLengthArray<char> signalSignature;
         QObjectPrivate::signalSignature(signal, &signalSignature);
         priv->disconnectNotify(signalSignature.constData());
         s->source = 0;
      }
   } else if (type == NotifierType) {
      Notifier *n = asNotifier();

      if (n->next) {
         n->next->asNotifier()->prev = n->prev;
      }
      if (n->prev) {
         *n->prev = n->next;
      }
      if (n->disconnected) {
         *n->disconnected = 0;
      }
      n->next = 0;
      n->prev = 0;
      n->disconnected = 0;
      n->notifier = 0;
   }
   refCount = 0;
}

void QDeclarativeNotifierEndpoint::deref()
{
   refCount--;
   if (refCount <= 0) {
      disconnect();
   }
}

QDeclarativeNotifierEndpoint::Notifier *QDeclarativeNotifierEndpoint::toNotifier()
{
   if (NotifierType == type) {
      return asNotifier();
   }

   if (SignalType == type) {
      disconnect();
      Signal *s = asSignal();
      s->~Signal();
   }

   type = NotifierType;
   Notifier *n = asNotifier();
   n->next = 0;
   n->prev = 0;
   n->disconnected = 0;
   n->notifier = 0;
   return n;
}

QDeclarativeNotifierEndpoint::Notifier *QDeclarativeNotifierEndpoint::asNotifier()
{
   Q_ASSERT(type == NotifierType);
   return &notifier;
}

QDeclarativeNotifierEndpoint::Signal *QDeclarativeNotifierEndpoint::toSignal()
{
   if (SignalType == type) {
      return asSignal();
   }

   disconnect();
   signal.signal = new (&signal.signalData) Signal;
   type = SignalType;
   return signal.signal;
}

QDeclarativeNotifierEndpoint::Signal *QDeclarativeNotifierEndpoint::asSignal()
{
   Q_ASSERT(type == SignalType);
   return signal.signal;
}

QT_END_NAMESPACE

#endif // QDECLARATIVENOTIFIER_P_H

