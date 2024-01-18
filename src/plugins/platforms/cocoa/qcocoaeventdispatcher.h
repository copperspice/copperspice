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

/***********************************************************************
* Copyright (c) 2007-2008, Apple, Inc.
* All rights reserved.
*
* Refer to APPLE_LICENSE.TXT (in this directory) for license terms
***********************************************************************/

#ifndef QCOCOA_EVENTDISPATCHER_H
#define QCOCOA_EVENTDISPATCHER_H

#include <qabstracteventdispatcher.h>
#include <qstack.h>
#include <qwindowdefs.h>
#include <qabstracteventdispatcher_p.h>
#include <qcfsocketnotifier_p.h>
#include <qtimerinfo_unix_p.h>

#include <CoreFoundation/CoreFoundation.h>

typedef struct _NSModalSession *NSModalSession;

typedef struct _QCocoaModalSessionInfo {
   QPointer<QWindow> window;
   NSModalSession session;
   void *nswindow;
} QCocoaModalSessionInfo;

class QCocoaEventDispatcherPrivate;

class QCocoaEventDispatcher : public QAbstractEventDispatcher
{
   CS_OBJECT(QCocoaEventDispatcher)
   Q_DECLARE_PRIVATE(QCocoaEventDispatcher)

 public:
   QCocoaEventDispatcher(QAbstractEventDispatcherPrivate &priv, QObject *parent = nullptr);
   explicit QCocoaEventDispatcher(QObject *parent = nullptr);
   ~QCocoaEventDispatcher();

   bool processEvents(QEventLoop::ProcessEventsFlags flags) override;
   bool hasPendingEvents() override;

   void registerSocketNotifier(QSocketNotifier *notifier) override;
   void unregisterSocketNotifier(QSocketNotifier *notifier) override;

   void registerTimer(int timerId, int interval, Qt::TimerType timerType, QObject *object) override;
   bool unregisterTimer(int timerId) override;
   bool unregisterTimers(QObject *object) override;

   QList<QTimerInfo> registeredTimers(QObject *object) const override;

   int remainingTime(int timerId) override;

   void wakeUp() override;
   void interrupt() override;
   void flush() override;

   static void clearCurrentThreadCocoaEventDispatcherInterruptFlag();

   friend void qt_mac_maybeCancelWaitForMoreEventsForwarder(QAbstractEventDispatcher *eventDispatcher);
};

class QCocoaEventDispatcherPrivate : public QAbstractEventDispatcherPrivate
{
   Q_DECLARE_PUBLIC(QCocoaEventDispatcher)

 public:
   QCocoaEventDispatcherPrivate();

   uint processEventsFlags;

   // timer handling
   QTimerInfoList timerInfoList;
   CFRunLoopTimerRef runLoopTimerRef;
   CFRunLoopSourceRef activateTimersSourceRef;
   void maybeStartCFRunLoopTimer();
   void maybeStopCFRunLoopTimer();
   static void runLoopTimerCallback(CFRunLoopTimerRef, void *info);
   static void activateTimersSourceCallback(void *info);

   // Set 'blockSendPostedEvents' to true if you _really_ need to make sure events
   // are not posted while calling low-level cocoa functions (like beginModalForWindow).
   // use a QBoolBlocker to be safe
   bool blockSendPostedEvents;

   // The following variables help organizing modal sessions:
   QStack<QCocoaModalSessionInfo> cocoaModalSessionStack;
   bool currentExecIsNSAppRun;
   bool nsAppRunCalledByQt;
   bool cleanupModalSessionsNeeded;
   uint processEventsCalled;

   NSModalSession currentModalSessionCached;
   NSModalSession currentModalSession();

   void updateChildrenWorksWhenModal();
   void temporarilyStopAllModalSessions();
   void beginModalSession(QWindow *widget);
   void endModalSession(QWindow *widget);
   void cleanupModalSessions();

   void cancelWaitForMoreEvents();
   void maybeCancelWaitForMoreEvents();
   void ensureNSAppInitialized();

   void removeQueuedUserInputEvents(int nsWinNumber);

   QCFSocketNotifier cfSocketNotifier;
   QList<void *> queuedUserInputEvents;          // NSEvent *
   CFRunLoopSourceRef postedEventsSource;
   CFRunLoopObserverRef waitingObserver;
   CFRunLoopObserverRef firstTimeObserver;
   QAtomicInt serialNumber;
   int lastSerial;
   bool interrupt;

   static void postedEventsSourceCallback(void *info);
   static void waitingObserverCallback(CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *info);
   static void firstLoopEntry(CFRunLoopObserverRef ref, CFRunLoopActivity activity, void *info);
   void processPostedEvents();

   static QCocoaEventDispatcherPrivate *get(QCocoaEventDispatcher *event) {
      return static_cast<QCocoaEventDispatcherPrivate *>(event->d_ptr.data());
   }
};

class QtCocoaInterruptDispatcher : public QObject
{
   static QtCocoaInterruptDispatcher *instance;
   bool cancelled;

   QtCocoaInterruptDispatcher();
   ~QtCocoaInterruptDispatcher();

 public:
   static void interruptLater();
   static void cancelInterruptLater();
};

#endif
