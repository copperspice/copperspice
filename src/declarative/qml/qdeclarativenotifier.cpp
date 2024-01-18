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

#include "private/qdeclarativenotifier_p.h"
#include "private/qdeclarativeproperty_p.h"

QT_BEGIN_NAMESPACE

void QDeclarativeNotifier::emitNotify(QDeclarativeNotifierEndpoint *endpoint)
{
   QDeclarativeNotifierEndpoint::Notifier *n = endpoint->asNotifier();

   QDeclarativeNotifierEndpoint **oldDisconnected = n->disconnected;
   n->disconnected = &endpoint;

   if (n->next) {
      emitNotify(n->next);
   }

   if (endpoint) {
      void *args[] = { 0 };

      QMetaObject::metacall(endpoint->target, QMetaObject::InvokeMetaMethod,
                            endpoint->targetMethod, args);

      if (endpoint) {
         endpoint->asNotifier()->disconnected = oldDisconnected;
      }
   }

   if (oldDisconnected) {
      *oldDisconnected = endpoint;
   }
}

void QDeclarativeNotifierEndpoint::connect(QObject *source, int sourceSignal)
{
   Signal *s = toSignal();

   if (s->source == source && s->sourceSignal == sourceSignal) {
      refCount++;
      return;
   }

   disconnect();

   QDeclarativePropertyPrivate::connect(source, sourceSignal, target, targetMethod);

   s->source = source;
   s->sourceSignal = sourceSignal;
   refCount++;
}

void QDeclarativeNotifierEndpoint::copyAndClear(QDeclarativeNotifierEndpoint &other)
{
   other.disconnect();

   other.target = target;
   other.targetMethod = targetMethod;

   if (!isConnected()) {
      return;
   }

   if (SignalType == type) {
      Signal *other_s = other.toSignal();
      Signal *s = asSignal();

      other_s->source = s->source;
      other_s->sourceSignal = s->sourceSignal;
      s->source = 0;
   } else if (NotifierType == type) {
      Notifier *other_n = other.toNotifier();
      Notifier *n = asNotifier();

      other_n->notifier = n->notifier;
      other_n->disconnected = n->disconnected;
      if (other_n->disconnected) {
         *other_n->disconnected = &other;
      }

      if (n->next) {
         other_n->next = n->next;
         n->next->asNotifier()->prev = &other_n->next;
      }
      other_n->prev = n->prev;
      *other_n->prev = &other;

      n->prev = 0;
      n->next = 0;
      n->disconnected = 0;
      n->notifier = 0;
   }
}


QT_END_NAMESPACE

