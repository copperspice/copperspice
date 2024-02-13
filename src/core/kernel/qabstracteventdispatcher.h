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

#ifndef QABSTRACTEVENTDISPATCHER_H
#define QABSTRACTEVENTDISPATCHER_H

#include <qeventloop.h>
#include <qobject.h>
#include <qscopedpointer.h>

class QAbstractNativeEventFilter;
class QAbstractEventDispatcherPrivate;
class QSocketNotifier;

#ifdef Q_OS_WIN
class QWinEventNotifier;
#endif

class Q_CORE_EXPORT QAbstractEventDispatcher : public QObject
{
   CORE_CS_OBJECT(QAbstractEventDispatcher)
   Q_DECLARE_PRIVATE(QAbstractEventDispatcher)

 public:
   explicit QAbstractEventDispatcher(QObject *parent = nullptr);
   ~QAbstractEventDispatcher();

   static QAbstractEventDispatcher *instance(QThread *thread = nullptr);

   virtual bool processEvents(QEventLoop::ProcessEventsFlags flags) = 0;
   virtual bool hasPendingEvents() = 0;

   virtual void registerSocketNotifier(QSocketNotifier *notifier) = 0;
   virtual void unregisterSocketNotifier(QSocketNotifier *notifier) = 0;

   int registerTimer(int interval, Qt::TimerType timerType, QObject *object);

   virtual void registerTimer(int timerId, int interval, Qt::TimerType timerType, QObject *object) = 0;
   virtual bool unregisterTimer(int timerId) = 0;
   virtual bool unregisterTimers(QObject *object) = 0;
   virtual QList<QTimerInfo> registeredTimers(QObject *object) const = 0;

   virtual int remainingTime(int timerId) = 0;

#ifdef Q_OS_WIN
   virtual bool registerEventNotifier(QWinEventNotifier *notifier)   = 0;
   virtual void unregisterEventNotifier(QWinEventNotifier *notifier) = 0;
#endif

   virtual void wakeUp() = 0;
   virtual void interrupt() = 0;
   virtual void flush() = 0;

   virtual void startingUp();
   virtual void closingDown();

   void installNativeEventFilter(QAbstractNativeEventFilter *filterObj);
   void removeNativeEventFilter(QAbstractNativeEventFilter *filterObj);
   bool filterNativeEvent(const QByteArray &eventType, void *message, long *result);

   CORE_CS_SIGNAL_1(Public, void aboutToBlock())
   CORE_CS_SIGNAL_2(aboutToBlock)

   CORE_CS_SIGNAL_1(Public, void awake())
   CORE_CS_SIGNAL_2(awake)

 protected:
   QAbstractEventDispatcher(QAbstractEventDispatcherPrivate &, QObject *parent);

   QScopedPointer<QAbstractEventDispatcherPrivate> d_ptr;
};

#endif
