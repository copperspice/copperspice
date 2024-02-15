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

#ifndef QCFSOCKETNOTIFIER_P_H
#define QCFSOCKETNOTIFIER_P_H

#include <qabstracteventdispatcher.h>
#include <qhash.h>

#include <CoreFoundation/CoreFoundation.h>

struct MacSocketInfo {
   MacSocketInfo() : socket(nullptr), runloop(nullptr), readNotifier(nullptr), writeNotifier(nullptr),
      readEnabled(false), writeEnabled(false) {}
   CFSocketRef socket;
   CFRunLoopSourceRef runloop;
   QObject *readNotifier;
   QObject *writeNotifier;
   bool readEnabled;
   bool writeEnabled;
};

using MacSocketHash                  = QHash<int, MacSocketInfo *>;
using MaybeCancelWaitForMoreEventsFn = void (*)(QAbstractEventDispatcher *hostEventDispacher);

// The CoreFoundationSocketNotifier class implements socket notifiers support using
// CFSocket for event dispatchers running on top of the Core Foundation run loop system.
// (currently Mac and iOS)
//
// The principal functions are registerSocketNotifier() and unregisterSocketNotifier().
//
// setHostEventDispatcher() should be called at startup.
// removeSocketNotifiers() should be called at shutdown.
//
class Q_CORE_EXPORT QCFSocketNotifier
{
 public:
   QCFSocketNotifier();
   ~QCFSocketNotifier();
   void setHostEventDispatcher(QAbstractEventDispatcher *hostEventDispacher);
   void setMaybeCancelWaitForMoreEventsCallback(MaybeCancelWaitForMoreEventsFn callBack);
   void registerSocketNotifier(QSocketNotifier *notifier);
   void unregisterSocketNotifier(QSocketNotifier *notifier);
   void removeSocketNotifiers();

 private:
   void destroyRunLoopObserver();

   static void unregisterSocketInfo(MacSocketInfo *socketInfo);
   static void enableSocketNotifiers(CFRunLoopObserverRef ref, CFRunLoopActivity activity, void *info);

   MacSocketHash macSockets;
   QAbstractEventDispatcher *eventDispatcher;
   MaybeCancelWaitForMoreEventsFn maybeCancelWaitForMoreEvents;
   CFRunLoopObserverRef enableNotifiersObserver;

   friend void qt_mac_socket_callback(CFSocketRef, CFSocketCallBackType, CFDataRef, const void *, void *);
};

#endif
