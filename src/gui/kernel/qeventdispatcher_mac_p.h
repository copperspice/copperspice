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

/***********************************************************************
** Copyright (C) 2007-2008, Apple, Inc.
***********************************************************************/

#ifndef QEVENTDISPATCHER_MAC_P_H
#define QEVENTDISPATCHER_MAC_P_H

#include <QtGui/qwindowdefs.h>
#include <QtCore/qhash.h>
#include <QtCore/qstack.h>
#include <qabstracteventdispatcher_p.h>
#include <qt_mac_p.h>

QT_BEGIN_NAMESPACE

typedef struct _NSModalSession *NSModalSession;
typedef struct _QCocoaModalSessionInfo {
   QPointer<QWidget> widget;
   NSModalSession session;
   void *nswindow;
} QCocoaModalSessionInfo;


class QEventDispatcherMacPrivate;

class QEventDispatcherMac : public QAbstractEventDispatcher
{
   GUI_CS_OBJECT(QEventDispatcherMac)
   Q_DECLARE_PRIVATE(QEventDispatcherMac)

 public:
   explicit QEventDispatcherMac(QObject *parent = nullptr);
   ~QEventDispatcherMac();

   bool processEvents(QEventLoop::ProcessEventsFlags flags) override;
   bool hasPendingEvents() override;

   void registerSocketNotifier(QSocketNotifier *notifier) override;
   void unregisterSocketNotifier(QSocketNotifier *notifier) override;

   void registerTimer(int timerId, int interval, QObject *object) override;
   bool unregisterTimer(int timerId) override;
   bool unregisterTimers(QObject *object) override;
   QList<TimerInfo> registeredTimers(QObject *object) const override;

   void wakeUp() override;
   void flush() override;
   void interrupt() override;

 private:
   friend class QApplicationPrivate;
};

struct MacTimerInfo {
   int id;
   int interval;
   QObject *obj;
   bool pending;
   CFRunLoopTimerRef runLoopTimer;
   bool operator==(const MacTimerInfo &other) {
      return (id == other.id);
   }
};
typedef QHash<int, MacTimerInfo *> MacTimerHash;

struct MacSocketInfo {
   MacSocketInfo() : socket(0), runloop(0), readNotifier(0), writeNotifier(0) {}
   CFSocketRef socket;
   CFRunLoopSourceRef runloop;
   QObject *readNotifier;
   QObject *writeNotifier;
};
typedef QHash<int, MacSocketInfo *> MacSocketHash;

class QEventDispatcherMacPrivate : public QAbstractEventDispatcherPrivate
{
   Q_DECLARE_PUBLIC(QEventDispatcherMac)

 public:
   QEventDispatcherMacPrivate();

   static MacTimerHash macTimerHash;
   // Set 'blockSendPostedEvents' to true if you _really_ need
   // to make sure that qt events are not posted while calling
   // low-level cocoa functions (like beginModalForWindow). And
   // use a QBoolBlocker to be safe:
   static bool blockSendPostedEvents;

   QThreadData *internal_get_ThreadData();

   // The following variables help organizing modal sessions:
   static QStack<QCocoaModalSessionInfo> cocoaModalSessionStack;
   static bool currentExecIsNSAppRun;
   static bool nsAppRunCalledByQt;
   static bool cleanupModalSessionsNeeded;
   static NSModalSession currentModalSessionCached;
   static NSModalSession currentModalSession();
   static void updateChildrenWorksWhenModal();
   static void temporarilyStopAllModalSessions();
   static void beginModalSession(QWidget *widget);
   static void endModalSession(QWidget *widget);
   static void cancelWaitForMoreEvents();
   static void cleanupModalSessions();
   static void ensureNSAppInitialized();

   MacSocketHash macSockets;
   QList<void *> queuedUserInputEvents; // List of EventRef in Carbon, and NSEvent * in Cocoa
   CFRunLoopSourceRef postedEventsSource;
   CFRunLoopObserverRef waitingObserver;
   CFRunLoopObserverRef firstTimeObserver;
   QAtomicInt serialNumber;
   int lastSerial;
   static bool interrupt;

 private:
   static Boolean postedEventSourceEqualCallback(const void *info1, const void *info2);
   static void postedEventsSourcePerformCallback(void *info);
   static void activateTimer(CFRunLoopTimerRef, void *info);
   static void waitingObserverCallback(CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *info);
   static void firstLoopEntry(CFRunLoopObserverRef ref, CFRunLoopActivity activity, void *info);
};


class QtMacInterruptDispatcherHelp : public QObject
{
   static QtMacInterruptDispatcherHelp *instance;
   bool cancelled;

   QtMacInterruptDispatcherHelp();
   ~QtMacInterruptDispatcherHelp();

 public:
   static void interruptLater();
   static void cancelInterruptLater();
};

QT_END_NAMESPACE

#endif // QEVENTDISPATCHER_MAC_P_H
