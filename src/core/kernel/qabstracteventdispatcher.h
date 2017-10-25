/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QABSTRACTEVENTDISPATCHER_H
#define QABSTRACTEVENTDISPATCHER_H

#include <QtCore/qobject.h>
#include <QtCore/qeventloop.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QAbstractEventDispatcherPrivate;
class QSocketNotifier;

class Q_CORE_EXPORT QAbstractEventDispatcher : public QObject
{
   CORE_CS_OBJECT(QAbstractEventDispatcher)
   Q_DECLARE_PRIVATE(QAbstractEventDispatcher)

 public:
   using TimerInfo = std::pair<int, int>;

   explicit QAbstractEventDispatcher(QObject *parent = nullptr);
   ~QAbstractEventDispatcher();

   static QAbstractEventDispatcher *instance(QThread *thread = 0);

   virtual bool processEvents(QEventLoop::ProcessEventsFlags flags) = 0;
   virtual bool hasPendingEvents() = 0;

   virtual void registerSocketNotifier(QSocketNotifier *notifier) = 0;
   virtual void unregisterSocketNotifier(QSocketNotifier *notifier) = 0;

   int registerTimer(int interval, QObject *object);
   virtual void registerTimer(int timerId, int interval, QObject *object) = 0;
   virtual bool unregisterTimer(int timerId) = 0;
   virtual bool unregisterTimers(QObject *object) = 0;
   virtual QList<TimerInfo> registeredTimers(QObject *object) const = 0;

   virtual void wakeUp() = 0;
   virtual void interrupt() = 0;
   virtual void flush() = 0;

   virtual void startingUp();
   virtual void closingDown();

   typedef bool(*EventFilter)(void *message);
   EventFilter setEventFilter(EventFilter filter);
   bool filterEvent(void *message);

   CORE_CS_SIGNAL_1(Public, void aboutToBlock())
   CORE_CS_SIGNAL_2(aboutToBlock)
   CORE_CS_SIGNAL_1(Public, void awake())
   CORE_CS_SIGNAL_2(awake)

   QAbstractEventDispatcher(QAbstractEventDispatcherPrivate &, QObject *parent);

 protected:
   QScopedPointer<QAbstractEventDispatcherPrivate> d_ptr;

};

QT_END_NAMESPACE

#endif // QABSTRACTEVENTDISPATCHER_H
