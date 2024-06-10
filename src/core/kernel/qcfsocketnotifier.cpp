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

#include <qcfsocketnotifier_p.h>
#include <qcoreapplication.h>
#include <qsocketnotifier.h>
#include <qthread.h>

void qt_mac_socket_callback(CFSocketRef s, CFSocketCallBackType callbackType, CFDataRef,
      const void *, void *info)
{
   QCFSocketNotifier *cfSocketNotifier = static_cast<QCFSocketNotifier *>(info);
   int nativeSocket = CFSocketGetNative(s);
   MacSocketInfo *socketInfo = cfSocketNotifier->macSockets.value(nativeSocket);
   QEvent notifierEvent(QEvent::SockAct);

   // There is a race condition that happen where we disable the notifier and
   // the kernel still has a notification to pass on. We then get this
   // notification after we've successfully disabled the CFSocket, but our
   // notifier is now gone. The upshot is we have to check the notifier every time.

   if (callbackType == kCFSocketReadCallBack) {
      if (socketInfo->readNotifier && socketInfo->readEnabled) {
         socketInfo->readEnabled = false;
         QCoreApplication::sendEvent(socketInfo->readNotifier, &notifierEvent);
      }
   } else if (callbackType == kCFSocketWriteCallBack) {
      if (socketInfo->writeNotifier && socketInfo->writeEnabled) {
         socketInfo->writeEnabled = false;
         QCoreApplication::sendEvent(socketInfo->writeNotifier, &notifierEvent);
      }
   }

   if (cfSocketNotifier->maybeCancelWaitForMoreEvents) {
      cfSocketNotifier->maybeCancelWaitForMoreEvents(cfSocketNotifier->eventDispatcher);
   }
}

CFRunLoopSourceRef qt_mac_add_socket_to_runloop(const CFSocketRef socket)
{
   CFRunLoopSourceRef loopSource = CFSocketCreateRunLoopSource(kCFAllocatorDefault, socket, 0);

   if (! loopSource) {
      return nullptr;
   }

   CFRunLoopAddSource(CFRunLoopGetMain(), loopSource, kCFRunLoopCommonModes);
   return loopSource;
}

void qt_mac_remove_socket_from_runloop(const CFSocketRef socket, CFRunLoopSourceRef runloop)
{
   Q_ASSERT(runloop);

   CFRunLoopRemoveSource(CFRunLoopGetMain(), runloop, kCFRunLoopCommonModes);
   CFSocketDisableCallBacks(socket, kCFSocketReadCallBack);
   CFSocketDisableCallBacks(socket, kCFSocketWriteCallBack);
}

QCFSocketNotifier::QCFSocketNotifier()
   : eventDispatcher(nullptr), maybeCancelWaitForMoreEvents(nullptr), enableNotifiersObserver(nullptr)
{
}

QCFSocketNotifier::~QCFSocketNotifier()
{
}

void QCFSocketNotifier::setHostEventDispatcher(QAbstractEventDispatcher *hostEventDispacher)
{
   eventDispatcher = hostEventDispacher;
}

void QCFSocketNotifier::setMaybeCancelWaitForMoreEventsCallback(MaybeCancelWaitForMoreEventsFn callBack)
{
   maybeCancelWaitForMoreEvents = callBack;
}

void QCFSocketNotifier::registerSocketNotifier(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);
   int nativeSocket = notifier->socket();
   int type = notifier->type();

#if defined(CS_SHOW_DEBUG_CORE)

   if (nativeSocket < 0 || nativeSocket > FD_SETSIZE) {
      qDebug("QSocketNotifier::registerSocketNotifier() Internal error");
      return;

   } else if (notifier->thread() != eventDispatcher->thread()
         || eventDispatcher->thread() != QThread::currentThread()) {
      qDebug("QSocketNotifier::registerSocketNotifier() Socket notifiers can not be enabled from another thread");
      return;
   }

#endif

   if (type == QSocketNotifier::Exception) {
      qWarning("QSocketNotifier::registerSocketNotifier() Exception is not supported on this OS");
      return;
   }

   // Check if we have a CFSocket for the native socket, create one if not.
   MacSocketInfo *socketInfo = macSockets.value(nativeSocket);

   if (! socketInfo) {
      socketInfo = new MacSocketInfo();

      // Create CFSocket, specify that we want both read and write callbacks (the callbacks
      // are enabled/disabled later on).
      const int callbackTypes = kCFSocketReadCallBack | kCFSocketWriteCallBack;
      CFSocketContext context = {0, this, nullptr, nullptr, nullptr};
      socketInfo->socket = CFSocketCreateWithNative(kCFAllocatorDefault, nativeSocket, callbackTypes, qt_mac_socket_callback, &context);

      if (CFSocketIsValid(socketInfo->socket) == false) {
         qWarning("QEventDispatcher::registerSocketNotifier() Failed to create CFSocket");
         return;
      }

      CFOptionFlags flags = CFSocketGetSocketFlags(socketInfo->socket);
      // QSocketNotifier doesn't close the socket upon destruction/invalidation
      flags &= ~kCFSocketCloseOnInvalidate;

      // Expicitly disable automatic re-enable, as we do that manually on each runloop pass
      flags &= ~(kCFSocketAutomaticallyReenableWriteCallBack | kCFSocketAutomaticallyReenableReadCallBack);
      CFSocketSetSocketFlags(socketInfo->socket, flags);

      macSockets.insert(nativeSocket, socketInfo);
   }

   if (type == QSocketNotifier::Read) {
      Q_ASSERT(socketInfo->readNotifier == nullptr);
      socketInfo->readNotifier = notifier;
      socketInfo->readEnabled = false;
   } else if (type == QSocketNotifier::Write) {
      Q_ASSERT(socketInfo->writeNotifier == nullptr);
      socketInfo->writeNotifier = notifier;
      socketInfo->writeEnabled = false;
   }

   if (!enableNotifiersObserver) {
      // Create a run loop observer which enables the socket notifiers on each
      // pass of the run loop, before any sources are processed.
      CFRunLoopObserverContext context = {};
      context.info = this;

      enableNotifiersObserver = CFRunLoopObserverCreate(kCFAllocatorDefault, kCFRunLoopBeforeSources,
            true, 0, enableSocketNotifiers, &context);

      Q_ASSERT(enableNotifiersObserver);
      CFRunLoopAddObserver(CFRunLoopGetMain(), enableNotifiersObserver, kCFRunLoopCommonModes);
   }
}

void QCFSocketNotifier::unregisterSocketNotifier(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);
   int nativeSocket = notifier->socket();
   int type = notifier->type();

#if defined(CS_SHOW_DEBUG_CORE)
   if (nativeSocket < 0 || nativeSocket > FD_SETSIZE) {
      qDebug("QSocketNotifier::unregisterSocketNotifier() Internal error");
      return;

   } else if (notifier->thread() != eventDispatcher->thread() || eventDispatcher->thread() != QThread::currentThread()) {
      qDebug("QSocketNotifier::unregisterSocketNotifier() Socket notifiers can not be disabled from another thread");
      return;
   }
#endif

   if (type == QSocketNotifier::Exception) {
      qWarning("QSocketNotifier::unregisterSocketNotifier() Exception is not supported on this OS");
      return;
   }

   MacSocketInfo *socketInfo = macSockets.value(nativeSocket);

   if (! socketInfo) {
      qWarning("QEventDispatcherMac::unregisterSocketNotifier() Unable to unregister an unknown notifier");
      return;
   }

   // Decrement read/write counters and disable callbacks if necessary.
   if (type == QSocketNotifier::Read) {
      Q_ASSERT(notifier == socketInfo->readNotifier);
      socketInfo->readNotifier = nullptr;
      socketInfo->readEnabled = false;
      CFSocketDisableCallBacks(socketInfo->socket, kCFSocketReadCallBack);

   } else if (type == QSocketNotifier::Write) {
      Q_ASSERT(notifier == socketInfo->writeNotifier);
      socketInfo->writeNotifier = nullptr;
      socketInfo->writeEnabled = false;
      CFSocketDisableCallBacks(socketInfo->socket, kCFSocketWriteCallBack);
   }

   // Remove CFSocket from runloop if this was the last QSocketNotifier.
   if (socketInfo->readNotifier == nullptr && socketInfo->writeNotifier == nullptr) {
      unregisterSocketInfo(socketInfo);
      delete socketInfo;
      macSockets.remove(nativeSocket);
   }
}

void QCFSocketNotifier::removeSocketNotifiers()
{
   // Remove CFSockets from the runloop.
   for (MacSocketInfo *socketInfo : macSockets) {
      unregisterSocketInfo(socketInfo);
      delete socketInfo;
   }

   macSockets.clear();

   destroyRunLoopObserver();
}

void QCFSocketNotifier::destroyRunLoopObserver()
{
   if (! enableNotifiersObserver) {
      return;
   }

   CFRunLoopObserverInvalidate(enableNotifiersObserver);
   CFRelease(enableNotifiersObserver);
   enableNotifiersObserver = nullptr;
}

void QCFSocketNotifier::unregisterSocketInfo(MacSocketInfo *socketInfo)
{
   if (socketInfo->runloop) {
      if (CFSocketIsValid(socketInfo->socket)) {
         qt_mac_remove_socket_from_runloop(socketInfo->socket, socketInfo->runloop);
      }

      CFRunLoopSourceInvalidate(socketInfo->runloop);
      CFRelease(socketInfo->runloop);
   }

   CFSocketInvalidate(socketInfo->socket);
   CFRelease(socketInfo->socket);
}

void QCFSocketNotifier::enableSocketNotifiers(CFRunLoopObserverRef, CFRunLoopActivity, void *info)
{
   QCFSocketNotifier *that = static_cast<QCFSocketNotifier *>(info);

   for (MacSocketInfo *socketInfo : that->macSockets) {
      if (! CFSocketIsValid(socketInfo->socket)) {
         continue;
      }

      if (! socketInfo->runloop) {
         // Add CFSocket to runloop.
         if (! (socketInfo->runloop = qt_mac_add_socket_to_runloop(socketInfo->socket))) {
            qWarning("QEventDispatcher::registerSocketNotifier() Failed to add CFSocket to runloop");
            CFSocketInvalidate(socketInfo->socket);
            continue;
         }

         if (! socketInfo->readNotifier) {
            CFSocketDisableCallBacks(socketInfo->socket, kCFSocketReadCallBack);
         }

         if (! socketInfo->writeNotifier) {
            CFSocketDisableCallBacks(socketInfo->socket, kCFSocketWriteCallBack);
         }
      }

      if (socketInfo->readNotifier && !socketInfo->readEnabled) {
         socketInfo->readEnabled = true;
         CFSocketEnableCallBacks(socketInfo->socket, kCFSocketReadCallBack);
      }

      if (socketInfo->writeNotifier && !socketInfo->writeEnabled) {
         socketInfo->writeEnabled = true;
         CFSocketEnableCallBacks(socketInfo->socket, kCFSocketWriteCallBack);
      }
   }
}
