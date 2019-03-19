/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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
** Copyright (C) 2007-2008, Apple, Inc.
***********************************************************************/

#include <qplatformdefs.h>
#include <qt_mac_p.h>
#include <qeventdispatcher_mac_p.h>
#include <qapplication.h>
#include <qevent.h>
#include <qdialog.h>
#include <qhash.h>
#include <qsocketnotifier.h>
#include <qwidget_p.h>
#include <qthread_p.h>
#include <qapplication_p.h>
#include <qcocoaapplication_mac_p.h>
#include <qt_cocoa_helpers_mac_p.h>

#ifndef QT_NO_THREAD
#  include <qmutex.h>
#endif

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE

extern void qt_event_request_timer(MacTimerInfo *); //qapplication_mac.cpp
extern MacTimerInfo *qt_event_get_timer(EventRef); //qapplication_mac.cpp
extern void qt_event_request_select(QEventDispatcherMac *); //qapplication_mac.cpp
extern void qt_event_request_updates(); //qapplication_mac.cpp
extern OSWindowRef qt_mac_window_for(const QWidget *); //qwidget_mac.cpp
extern bool qt_is_gui_used; //qapplication.cpp
extern bool qt_sendSpontaneousEvent(QObject *, QEvent *); // qapplication.cpp
extern bool qt_mac_is_macsheet(const QWidget *); //qwidget_mac.cpp

static inline CFRunLoopRef mainRunLoop()
{
   return CFRunLoopGetMain();
}

/* timer call back */
void QEventDispatcherMacPrivate::activateTimer(CFRunLoopTimerRef, void *info)
{
   int timerID = qint64(info);

   MacTimerInfo *tmr;
   tmr = macTimerHash.value(timerID);

   if (tmr == 0 || tmr->pending == true) {
      return;   // Can't send another timer event if it's pending.
   }

   if (blockSendPostedEvents) {
      QCoreApplication::postEvent(tmr->obj, new QTimerEvent(tmr->id));

   } else {
      tmr->pending = true;
      QTimerEvent e(tmr->id);
      qt_sendSpontaneousEvent(tmr->obj, &e);

      // Get the value again in case the timer gets unregistered during the sendEvent.
      tmr = macTimerHash.value(timerID);
      if (tmr != 0) {
         tmr->pending = false;
      }
   }

}

void QEventDispatcherMac::registerTimer(int timerId, int interval, QObject *obj)
{
#ifndef QT_NO_DEBUG
   if (timerId < 1 || interval < 0 || !obj) {
      qWarning("QEventDispatcherMac::registerTimer: invalid arguments");
      return;
   } else if (obj->thread() != thread() || thread() != QThread::currentThread()) {
      qWarning("QObject::startTimer: timers cannot be started from another thread");
      return;
   }
#endif

   MacTimerInfo *t = new MacTimerInfo();
   t->id = timerId;
   t->interval = interval;
   t->obj = obj;
   t->runLoopTimer = 0;
   t->pending = false;

   CFAbsoluteTime fireDate = CFAbsoluteTimeGetCurrent();
   CFTimeInterval cfinterval = qMax(CFTimeInterval(interval) / 1000, 0.0000001);
   fireDate += cfinterval;
   QEventDispatcherMacPrivate::macTimerHash.insert(timerId, t);
   CFRunLoopTimerContext info = { 0, (void *)timerId, 0, 0, 0 };
   t->runLoopTimer = CFRunLoopTimerCreate(0, fireDate, cfinterval, 0, 0,
                                          QEventDispatcherMacPrivate::activateTimer, &info);
   if (t->runLoopTimer == 0) {
      qFatal("QEventDispatcherMac::registerTimer: Cannot create timer");
   }
   CFRunLoopAddTimer(mainRunLoop(), t->runLoopTimer, kCFRunLoopCommonModes);
}

bool QEventDispatcherMac::unregisterTimer(int identifier)
{
#ifndef QT_NO_DEBUG
   if (identifier < 1) {
      qWarning("QEventDispatcherMac::unregisterTimer: invalid argument");
      return false;
   } else if (thread() != QThread::currentThread()) {
      qWarning("QObject::killTimer: timers cannot be stopped from another thread");
      return false;
   }
#endif
   if (identifier <= 0) {
      return false;   // not init'd or invalid timer
   }

   MacTimerInfo *timerInfo = QEventDispatcherMacPrivate::macTimerHash.take(identifier);
   if (timerInfo == 0) {
      return false;
   }

   if (! CSInternalEvents::get_m_inThreadChangeEvent(timerInfo->obj).load())  {
      QAbstractEventDispatcherPrivate::releaseTimerId(identifier);
   }

   CFRunLoopTimerInvalidate(timerInfo->runLoopTimer);
   CFRelease(timerInfo->runLoopTimer);
   delete timerInfo;

   return true;
}

bool QEventDispatcherMac::unregisterTimers(QObject *obj)
{
#ifndef QT_NO_DEBUG
   if (!obj) {
      qWarning("QEventDispatcherMac::unregisterTimers: invalid argument");
      return false;
   } else if (obj->thread() != thread() || thread() != QThread::currentThread()) {
      qWarning("QObject::killTimers: timers cannot be stopped from another thread");
      return false;
   }
#endif

   MacTimerHash::iterator it = QEventDispatcherMacPrivate::macTimerHash.begin();
   while (it != QEventDispatcherMacPrivate::macTimerHash.end()) {
      MacTimerInfo *timerInfo = it.value();

      if (timerInfo->obj != obj) {
         ++it;

      } else {

         if (! CSInternalEvents::get_m_inThreadChangeEvent(timerInfo->obj).load())  {
            QAbstractEventDispatcherPrivate::releaseTimerId(timerInfo->id);
         }

         CFRunLoopTimerInvalidate(timerInfo->runLoopTimer);
         CFRelease(timerInfo->runLoopTimer);
         delete timerInfo;
         it = QEventDispatcherMacPrivate::macTimerHash.erase(it);
      }
   }
   return true;
}

QList<QEventDispatcherMac::TimerInfo>
QEventDispatcherMac::registeredTimers(QObject *object) const
{
   if (!object) {
      qWarning("QEventDispatcherMac:registeredTimers: invalid argument");
      return QList<TimerInfo>();
   }

   QList<TimerInfo> list;

   MacTimerHash::const_iterator it = QEventDispatcherMacPrivate::macTimerHash.constBegin();
   while (it != QEventDispatcherMacPrivate::macTimerHash.constEnd()) {
      MacTimerInfo *t = it.value();
      if (t->obj == object) {
         list << TimerInfo(t->id, t->interval);
      }
      ++it;
   }
   return list;
}

/**************************************************************************
    Socket Notifiers
 *************************************************************************/
void qt_mac_socket_callback(CFSocketRef s, CFSocketCallBackType callbackType, CFDataRef,
                            const void *,  void *info)
{
   QEventDispatcherMacPrivate *const eventDispatcher
      = static_cast<QEventDispatcherMacPrivate *>(info);
   int nativeSocket = CFSocketGetNative(s);
   MacSocketInfo *socketInfo = eventDispatcher->macSockets.value(nativeSocket);
   QEvent notifierEvent(QEvent::SockAct);

   // There is a race condition that happen where we disable the notifier and
   // the kernel still has a notification to pass on. We then get this
   // notification after we've successfully disabled the CFSocket, but our Qt
   // notifier is now gone. The upshot is we have to check the notifier
   // everytime.
   if (callbackType == kCFSocketReadCallBack) {
      if (socketInfo->readNotifier) {
         QApplication::sendEvent(socketInfo->readNotifier, &notifierEvent);
      }
   } else if (callbackType == kCFSocketWriteCallBack) {
      if (socketInfo->writeNotifier) {
         QApplication::sendEvent(socketInfo->writeNotifier, &notifierEvent);
      }
   }
}

/*
    Adds a loop source for the given socket to the current run loop.
*/
CFRunLoopSourceRef qt_mac_add_socket_to_runloop(const CFSocketRef socket)
{
   CFRunLoopSourceRef loopSource = CFSocketCreateRunLoopSource(kCFAllocatorDefault, socket, 0);
   if (!loopSource) {
      return 0;
   }

   CFRunLoopAddSource(mainRunLoop(), loopSource, kCFRunLoopCommonModes);
   return loopSource;
}

/*
    Removes the loop source for the given socket from the current run loop.
*/
void qt_mac_remove_socket_from_runloop(const CFSocketRef socket, CFRunLoopSourceRef runloop)
{
   Q_ASSERT(runloop);
   CFRunLoopRemoveSource(mainRunLoop(), runloop, kCFRunLoopCommonModes);
   CFSocketDisableCallBacks(socket, kCFSocketReadCallBack);
   CFSocketDisableCallBacks(socket, kCFSocketWriteCallBack);
   CFRunLoopSourceInvalidate(runloop);
}

/*
    Register a QSocketNotifier with the mac event system by creating a CFSocket with
    with a read/write callback.

    Qt has separate socket notifiers for reading and writing, but on the mac there is
    a limitation of one CFSocket object for each native socket.
*/
void QEventDispatcherMac::registerSocketNotifier(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);
   int nativeSocket = notifier->socket();
   int type = notifier->type();
#ifndef QT_NO_DEBUG
   if (nativeSocket < 0 || nativeSocket > FD_SETSIZE) {
      qWarning("QSocketNotifier: Internal error");
      return;
   } else if (notifier->thread() != thread()
              || thread() != QThread::currentThread()) {
      qWarning("QSocketNotifier: socket notifiers cannot be enabled from another thread");
      return;
   }
#endif

   Q_D(QEventDispatcherMac);

   if (type == QSocketNotifier::Exception) {
      qWarning("QSocketNotifier::Exception is not supported on Mac OS X");
      return;
   }

   // Check if we have a CFSocket for the native socket, create one if not.
   MacSocketInfo *socketInfo = d->macSockets.value(nativeSocket);
   if (!socketInfo) {
      socketInfo = new MacSocketInfo();

      // Create CFSocket, specify that we want both read and write callbacks (the callbacks
      // are enabled/disabled later on).
      const int callbackTypes = kCFSocketReadCallBack | kCFSocketWriteCallBack;
      CFSocketContext context = {0, d, 0, 0, 0};
      socketInfo->socket = CFSocketCreateWithNative(kCFAllocatorDefault, nativeSocket, callbackTypes, qt_mac_socket_callback,
                           &context);
      if (CFSocketIsValid(socketInfo->socket) == false) {
         qWarning("QEventDispatcherMac::registerSocketNotifier: Failed to create CFSocket");
         return;
      }

      CFOptionFlags flags = CFSocketGetSocketFlags(socketInfo->socket);
      flags |= kCFSocketAutomaticallyReenableWriteCallBack; //QSocketNotifier stays enabled after a write
      flags &= ~kCFSocketCloseOnInvalidate; //QSocketNotifier doesn't close the socket upon destruction/invalidation
      CFSocketSetSocketFlags(socketInfo->socket, flags);

      // Add CFSocket to runloop.
      if (!(socketInfo->runloop = qt_mac_add_socket_to_runloop(socketInfo->socket))) {
         qWarning("QEventDispatcherMac::registerSocketNotifier: Failed to add CFSocket to runloop");
         CFSocketInvalidate(socketInfo->socket);
         CFRelease(socketInfo->socket);
         return;
      }

      // Disable both callback types by default. This must be done after
      // we add the CFSocket to the runloop, or else these calls will have
      // no effect.
      CFSocketDisableCallBacks(socketInfo->socket, kCFSocketReadCallBack);
      CFSocketDisableCallBacks(socketInfo->socket, kCFSocketWriteCallBack);

      d->macSockets.insert(nativeSocket, socketInfo);
   }

   // Increment read/write counters and select enable callbacks if necessary.
   if (type == QSocketNotifier::Read) {
      Q_ASSERT(socketInfo->readNotifier == 0);
      socketInfo->readNotifier = notifier;
      CFSocketEnableCallBacks(socketInfo->socket, kCFSocketReadCallBack);
   } else if (type == QSocketNotifier::Write) {
      Q_ASSERT(socketInfo->writeNotifier == 0);
      socketInfo->writeNotifier = notifier;
      CFSocketEnableCallBacks(socketInfo->socket, kCFSocketWriteCallBack);
   }
}

/*
    Unregister QSocketNotifer. The CFSocket correspoding to this notifier is
    removed from the runloop of this is the last notifier that users
    that CFSocket.
*/
void QEventDispatcherMac::unregisterSocketNotifier(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);
   int nativeSocket = notifier->socket();
   int type = notifier->type();
#ifndef QT_NO_DEBUG
   if (nativeSocket < 0 || nativeSocket > FD_SETSIZE) {
      qWarning("QSocketNotifier: Internal error");
      return;
   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qWarning("QSocketNotifier: socket notifiers cannot be disabled from another thread");
      return;
   }
#endif

   Q_D(QEventDispatcherMac);

   if (type == QSocketNotifier::Exception) {
      qWarning("QSocketNotifier::Exception is not supported on Mac OS X");
      return;
   }
   MacSocketInfo *socketInfo = d->macSockets.value(nativeSocket);
   if (!socketInfo) {
      qWarning("QEventDispatcherMac::unregisterSocketNotifier: Tried to unregister a not registered notifier");
      return;
   }

   // Decrement read/write counters and disable callbacks if necessary.
   if (type == QSocketNotifier::Read) {
      Q_ASSERT(notifier == socketInfo->readNotifier);
      socketInfo->readNotifier = 0;
      CFSocketDisableCallBacks(socketInfo->socket, kCFSocketReadCallBack);
   } else if (type == QSocketNotifier::Write) {
      Q_ASSERT(notifier == socketInfo->writeNotifier);
      socketInfo->writeNotifier = 0;
      CFSocketDisableCallBacks(socketInfo->socket, kCFSocketWriteCallBack);
   }

   // Remove CFSocket from runloop if this was the last QSocketNotifier.
   if (socketInfo->readNotifier == 0 && socketInfo->writeNotifier == 0) {
      if (CFSocketIsValid(socketInfo->socket)) {
         qt_mac_remove_socket_from_runloop(socketInfo->socket, socketInfo->runloop);
      }
      CFRunLoopSourceInvalidate(socketInfo->runloop);
      CFRelease(socketInfo->runloop);
      CFSocketInvalidate(socketInfo->socket);
      CFRelease(socketInfo->socket);
      delete socketInfo;
      d->macSockets.remove(nativeSocket);
   }
}

bool QEventDispatcherMac::hasPendingEvents()
{
   extern uint qGlobalPostedEventsCount();
   return qGlobalPostedEventsCount() || (qt_is_gui_used && CS_GetNumEventsInQueue(CS_GetMainEventQueue()));
}

static bool qt_mac_send_event(QEventLoop::ProcessEventsFlags, OSEventRef event, OSWindowRef pt)
{
   if (pt) {
      [pt sendEvent: event];
   } else {
      [[NSApplication sharedApplication] sendEvent: event];
   }
   return true;
}

static bool IsMouseOrKeyEvent( NSEvent *event )
{
   bool    result    = false;

   switch ( [event type] ) {
      case NSLeftMouseDown:
      case NSLeftMouseUp:
      case NSRightMouseDown:
      case NSRightMouseUp:
      case NSMouseMoved:                // ??
      case NSLeftMouseDragged:
      case NSRightMouseDragged:
      case NSMouseEntered:
      case NSMouseExited:
      case NSKeyDown:
      case NSKeyUp:
      case NSFlagsChanged:            // key modifiers changed?
      case NSCursorUpdate:            // ??
      case NSScrollWheel:
      case NSTabletPoint:
      case NSTabletProximity:
      case NSOtherMouseDown:
      case NSOtherMouseUp:
      case NSOtherMouseDragged:

#ifndef QT_NO_GESTURES
      case NSEventTypeGesture: // touch events
      case NSEventTypeMagnify:
      case NSEventTypeSwipe:
      case NSEventTypeRotate:
      case NSEventTypeBeginGesture:
      case NSEventTypeEndGesture:
#endif

         result    = true;
         break;

      default:
         break;
   }
   return result;
}

static inline void qt_mac_waitForMoreEvents()
{
   // If no event exist in the cocoa event que, wait
   // (and free up cpu time) until at least one event occur.
   // This implementation is a bit on the edge, but seems to
   // work fine:
   NSEvent *event = [[NSApplication sharedApplication] nextEventMatchingMask: NSAnyEventMask
                     untilDate: [NSDate distantFuture]
                     inMode: NSDefaultRunLoopMode
                     dequeue: YES];
   if (event) {
      [[NSApplication sharedApplication] postEvent: event atStart: YES];
   }
}


static inline void qt_mac_waitForMoreModalSessionEvents()
{
   // If no event exist in the cocoa event que, wait
   // (and free up cpu time) until at least one event occur.
   // This implementation is a bit on the edge, but seems to
   // work fine:
   NSEvent *event = [[NSApplication sharedApplication] nextEventMatchingMask: NSAnyEventMask
                     untilDate: [NSDate distantFuture]
                     inMode: NSModalPanelRunLoopMode
                     dequeue: YES];
   if (event) {
      [[NSApplication sharedApplication] postEvent: event atStart: YES];
   }
}

bool QEventDispatcherMac::processEvents(QEventLoop::ProcessEventsFlags flags)
{
   Q_D(QEventDispatcherMac);
   d->interrupt = false;

   bool interruptLater = false;
   QtMacInterruptDispatcherHelp::cancelInterruptLater();

   // In case we end up recursing while we now process events, make sure
   // that we send remaining posted Qt events before this call returns:
   wakeUp();
   emit awake();

   bool excludeUserEvents = flags & QEventLoop::ExcludeUserInputEvents;
   bool retVal = false;
   forever {
      if (d->interrupt)
      {
         break;
      }

      QMacCocoaAutoReleasePool pool;
      NSEvent *event = 0;

      // First, send all previously excluded input events, if any:
      if (!excludeUserEvents)
      {
         while (!d->queuedUserInputEvents.isEmpty()) {
            event = static_cast<NSEvent *>(d->queuedUserInputEvents.takeFirst());
            if (!filterEvent(event)) {
               qt_mac_send_event(flags, event, 0);
               retVal = true;
            }
            [event release];
         }
      }

      // If Qt is used as a plugin, or as an extension in a native cocoa
      // application, we should not run or stop NSApplication; This will be
      // done from the application itself. And if processEvents is called
      // manually (rather than from a QEventLoop), we cannot enter a tight
      // loop and block this call, but instead we need to return after one flush.
      // Finally, if we are to exclude user input events, we cannot call [[NSApplication sharedApplication] run]
      // as we then loose control over which events gets dispatched:
      const bool canExec_3rdParty = d->nsAppRunCalledByQt || ![[NSApplication sharedApplication] isRunning];
      const bool canExec_Qt = !excludeUserEvents &&
                              (flags & QEventLoop::DialogExec || flags & QEventLoop::EventLoopExec) ;

      if (canExec_Qt && canExec_3rdParty)
      {
         // We can use exec-mode, meaning that we can stay in a tight loop until
         // interrupted. This is mostly an optimization, but it allow us to use
         // [[NSApplication sharedApplication] run], which is the normal code path for cocoa applications.

         if (NSModalSession session = d->currentModalSession()) {
            bool oldValue = d->currentExecIsNSAppRun;
            d->currentExecIsNSAppRun = false;

            while ([[NSApplication sharedApplication] runModalSession: session] == NSRunContinuesResponse && !d->interrupt) {
               qt_mac_waitForMoreModalSessionEvents();
            }

            if (!d->interrupt && session == d->currentModalSessionCached) {
               // Someone called [[NSApplication sharedApplication] stopModal:] from outside the event
               // dispatcher (e.g to stop a native dialog). But that call wrongly stopped
               // 'session' as well. As a result, we need to restart all internal sessions:
               d->temporarilyStopAllModalSessions();
            }

            d->currentExecIsNSAppRun = oldValue;

         } else {
            d->nsAppRunCalledByQt = true;

            bool oldValue = d->currentExecIsNSAppRun;
            d->currentExecIsNSAppRun = true;

            [[NSApplication sharedApplication] run];

            d->currentExecIsNSAppRun = oldValue;
         }

         retVal = true;

      } else {
         // We cannot block the thread (and run in a tight loop).
         // Instead we will process all current pending events and return.
         d->ensureNSAppInitialized();
         if (NSModalSession session = d->currentModalSession())
         {
            // INVARIANT: a modal window is executing.
            if (!excludeUserEvents) {
               // Since we can dispatch all kinds of events, we choose
               // to use cocoa's native way of running modal sessions:
               if (flags & QEventLoop::WaitForMoreEvents) {
                  qt_mac_waitForMoreModalSessionEvents();
               }
               NSInteger status = [[NSApplication sharedApplication] runModalSession: session];
               if (status != NSRunContinuesResponse && session == d->currentModalSessionCached) {
                  // INVARIANT: Someone called [[NSApplication sharedApplication] stopModal:] from outside the event
                  // dispatcher (e.g to stop a native dialog). But that call wrongly stopped
                  // 'session' as well. As a result, we need to restart all internal sessions:
                  d->temporarilyStopAllModalSessions();
               }
               retVal = true;
            } else do {
                  // Dispatch all non-user events (but que non-user events up for later). In
                  // this case, we need more control over which events gets dispatched, and
                  // cannot use [[NSApplication sharedApplication] runModalSession:session]:
                  event = [[NSApplication sharedApplication] nextEventMatchingMask: NSAnyEventMask
                           untilDate: nil
                           inMode: NSModalPanelRunLoopMode
                           dequeue: YES];

                  if (event) {
                     if (IsMouseOrKeyEvent(event)) {
                        [event retain];
                        d->queuedUserInputEvents.append(event);
                        continue;
                     }

                     if (!filterEvent(event) && qt_mac_send_event(flags, event, 0)) {
                        retVal = true;
                     }
                  }
               } while (!d->interrupt && event != nil);

         } else do {
               // INVARIANT: No modal window is executing.
               event = [[NSApplication sharedApplication] nextEventMatchingMask: NSAnyEventMask
               untilDate: nil
               inMode: NSDefaultRunLoopMode
               dequeue: YES];

               if (event)
               {
                  if (flags & QEventLoop::ExcludeUserInputEvents) {
                     if (IsMouseOrKeyEvent(event)) {
                        [event retain];
                        d->queuedUserInputEvents.append(event);
                        continue;
                     }
                  }
                  if (!filterEvent(event) && qt_mac_send_event(flags, event, 0)) {
                     retVal = true;
                  }
               }
            } while (!d->interrupt && event != nil);

         // Be sure to flush the Qt posted events when not using exec mode
         // (exec mode will always do this call from the event loop source):

         QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

         if (! d->interrupt)
         {
            QCoreApplicationPrivate::sendPostedEvents(0, 0, threadData);
         }

         // Since the window that holds modality might have changed while processing
         // events, we we need to interrupt when we return back the previous process
         // event recursion to ensure that we spin the correct modal session.
         // We do the interruptLater at the end of the function to ensure that we don't
         // disturb the 'wait for more events' below (as deleteLater will post an event):

         interruptLater = true;
      }


      QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

      bool canWait = (threadData->canWait && !retVal
                      && !d->interrupt && (flags & QEventLoop::WaitForMoreEvents));

      if (canWait)
      {
         // INVARIANT: We haven't processed any events yet. And we're told
         // to stay inside this function until at least one event is processed.
         qt_mac_waitForMoreEvents();
         flags &= ~QEventLoop::WaitForMoreEvents;
      } else {
         // Done with event processing for now.
         // Leave the function:
         break;
      }
   }

   // If we're interrupted, we need to interrupt the _current_
   // recursion as well to check if it is  still supposed to be
   // executing. This way we wind down the stack until we land
   // on a recursion that again calls processEvents (typically
   // from QEventLoop), and set interrupt to false:
   if (d->interrupt) {
      interrupt();
   }

   if (interruptLater) {
      QtMacInterruptDispatcherHelp::interruptLater();
   }

   return retVal;
}

void QEventDispatcherMac::wakeUp()
{
   Q_D(QEventDispatcherMac);
   d->serialNumber.ref();
   CFRunLoopSourceSignal(d->postedEventsSource);
   CFRunLoopWakeUp(mainRunLoop());
}

void QEventDispatcherMac::flush()
{
   if (qApp) {
      QWidgetList tlws = QApplication::topLevelWidgets();
      for (int i = 0; i < tlws.size(); i++) {
         QWidget *tlw = tlws.at(i);
         if (tlw->isVisible()) {
            macWindowFlush(qt_mac_window_for(tlw));
         }
      }
   }
}

/*****************************************************************************
  QEventDispatcherMac Implementation
 *****************************************************************************/
MacTimerHash QEventDispatcherMacPrivate::macTimerHash;
bool QEventDispatcherMacPrivate::blockSendPostedEvents = false;
bool QEventDispatcherMacPrivate::interrupt = false;

QStack<QCocoaModalSessionInfo> QEventDispatcherMacPrivate::cocoaModalSessionStack;
bool QEventDispatcherMacPrivate::currentExecIsNSAppRun = false;
bool QEventDispatcherMacPrivate::nsAppRunCalledByQt = false;
bool QEventDispatcherMacPrivate::cleanupModalSessionsNeeded = false;
NSModalSession QEventDispatcherMacPrivate::currentModalSessionCached = 0;

void QEventDispatcherMacPrivate::ensureNSAppInitialized()
{
   // Some elements in Cocoa require NSApplication to be running before
   // they get fully initialized, in particular the menu bar. This
   // function is intended for cases where a dialog is told to execute before
   // QApplication::exec is called, or the application spins the events loop
   // manually rather than calling QApplication:exec.
   // The function makes sure that NSApplication starts running, but stops
   // it again as soon as the send posted events callback is called. That way
   // we let Cocoa finish the initialization it seems to need. We'll only
   // apply this trick at most once for any application, and we avoid doing it
   // for the common case where main just starts QApplication::exec.

   if (nsAppRunCalledByQt || [[NSApplication sharedApplication] isRunning]) {
      return;
   }

   nsAppRunCalledByQt = true;

   bool oldValue1 = interrupt;
   interrupt = true;

   bool oldValue2 = currentExecIsNSAppRun;
   currentExecIsNSAppRun = true;

   [[NSApplication sharedApplication] run];

   interrupt = oldValue1;
   currentExecIsNSAppRun = oldValue2;

}

void QEventDispatcherMacPrivate::temporarilyStopAllModalSessions()
{
   // Flush, and Stop, all created modal session, and as
   // such, make them pending again. The next call to
   // currentModalSession will recreate them again. The
   // reason to stop all session like this is that otherwise
   // a call [[NSApplication sharedApplication] stop] would not stop NSApp, but rather
   // the current modal session. So if we need to stop NSApp
   // we need to stop all the modal session first. To avoid changing
   // the stacking order of the windows while doing so, we put
   // up a block that is used in QCocoaWindow and QCocoaPanel:
   int stackSize = cocoaModalSessionStack.size();
   for (int i = 0; i < stackSize; ++i) {
      QCocoaModalSessionInfo &info = cocoaModalSessionStack[i];
      if (info.session) {
         [[NSApplication sharedApplication] endModalSession: info.session];
         info.session = 0;
      }
   }
   currentModalSessionCached = 0;
}

NSModalSession QEventDispatcherMacPrivate::currentModalSession()
{
   // If we have one or more modal windows, this function will create
   // a session for each of those, and return the one for the top.
   if (currentModalSessionCached) {
      return currentModalSessionCached;
   }

   if (cocoaModalSessionStack.isEmpty()) {
      return 0;
   }

   int sessionCount = cocoaModalSessionStack.size();
   for (int i = 0; i < sessionCount; ++i) {
      QCocoaModalSessionInfo &info = cocoaModalSessionStack[i];
      if (!info.widget) {
         continue;
      }
      if (info.widget->testAttribute(Qt::WA_DontShowOnScreen)) {
         continue;
      }
      if (!info.session) {
         QMacCocoaAutoReleasePool pool;
         NSWindow *window = qt_mac_window_for(info.widget);
         if (!window) {
            continue;
         }

         ensureNSAppInitialized();

         bool oldValue = blockSendPostedEvents;
         blockSendPostedEvents = true;

         info.nswindow = window;
         [(NSWindow *) info.nswindow retain];
         int levelBeforeEnterModal = [window level];
         info.session = [[NSApplication sharedApplication] beginModalSessionForWindow: window];

         // Make sure we don't stack the window lower that it was before
         // entering modal, in case it e.g. had the stays-on-top flag set:

         if (levelBeforeEnterModal > [window level]) {
            [window setLevel: levelBeforeEnterModal];
         }

         blockSendPostedEvents = oldValue;
      }

      currentModalSessionCached = info.session;
      cleanupModalSessionsNeeded = false;
   }
   return currentModalSessionCached;
}

static void setChildrenWorksWhenModal(QWidget *widget, bool worksWhenModal)
{
   // For NSPanels (but not NSWindows, sadly), we can set the flag
   // worksWhenModal, so that they are active even when they are not modal.
   QList<QDialog *> dialogs = widget->findChildren<QDialog *>();
   for (int i = 0; i < dialogs.size(); ++i) {
      NSWindow *window = qt_mac_window_for(dialogs[i]);
      if (window && [window isKindOfClass: [NSPanel class]]) {
         [static_cast<NSPanel *>(window) setWorksWhenModal: worksWhenModal];
         if (worksWhenModal && [window isVisible]) {
            [window orderFront: window];
         }
      }
   }
}

void QEventDispatcherMacPrivate::updateChildrenWorksWhenModal()
{
   // Make the dialog children of the widget
   // active. And make the dialog children of
   // the previous modal dialog unactive again:
   QMacCocoaAutoReleasePool pool;
   int size = cocoaModalSessionStack.size();
   if (size > 0) {
      if (QWidget *prevModal = cocoaModalSessionStack[size - 1].widget) {
         setChildrenWorksWhenModal(prevModal, true);
      }
      if (size > 1) {
         if (QWidget *prevModal = cocoaModalSessionStack[size - 2].widget) {
            setChildrenWorksWhenModal(prevModal, false);
         }
      }
   }
}

void QEventDispatcherMacPrivate::cleanupModalSessions()
{
   // Go through the list of modal sessions, and end those
   // that no longer has a widget assosiated; no widget means
   // the the session has logically ended. The reason we wait like
   // this to actually end the sessions for real (rather than at the
   // point they were marked as stopped), is that ending a session
   // when no other session runs below it on the stack will make cocoa
   // drop some events on the floor.
   QMacCocoaAutoReleasePool pool;
   int stackSize = cocoaModalSessionStack.size();

   for (int i = stackSize - 1; i >= 0; --i) {
      QCocoaModalSessionInfo &info = cocoaModalSessionStack[i];
      if (info.widget) {
         // This session has a widget, and is therefore not marked
         // as stopped. So just make it current. There might still be other
         // stopped sessions on the stack, but those will be stopped on
         // a later "cleanup" call.
         currentModalSessionCached = info.session;
         break;
      }
      cocoaModalSessionStack.remove(i);
      currentModalSessionCached = 0;
      if (info.session) {
         [[NSApplication sharedApplication] endModalSession: info.session];
         [(NSWindow *)info.nswindow release];
      }
   }

   updateChildrenWorksWhenModal();
   cleanupModalSessionsNeeded = false;
}

void QEventDispatcherMacPrivate::beginModalSession(QWidget *widget)
{
   // Add a new, empty (null), NSModalSession to the stack.
   // It will become active the next time QEventDispatcher::processEvents is called.
   // A QCocoaModalSessionInfo is considered pending to become active if the widget pointer
   // is non-zero, and the session pointer is zero (it will become active upon a call to
   // currentModalSession). A QCocoaModalSessionInfo is considered pending to be stopped if
   // the widget pointer is zero, and the session pointer is non-zero (it will be fully
   // stopped in cleanupModalSessions()).
   QCocoaModalSessionInfo info = {widget, 0, 0};
   cocoaModalSessionStack.push(info);
   updateChildrenWorksWhenModal();
   currentModalSessionCached = 0;
}

void QEventDispatcherMacPrivate::endModalSession(QWidget *widget)
{
   // Mark all sessions attached to widget as pending to be stopped. We do this
   // by setting the widget pointer to zero, but leave the session pointer.
   // We don't tell cocoa to stop any sessions just yet, because cocoa only understands
   // when we stop the _current_ modal session (which is the session on top of
   // the stack, and might not belong to 'widget').
   int stackSize = cocoaModalSessionStack.size();
   for (int i = stackSize - 1; i >= 0; --i) {
      QCocoaModalSessionInfo &info = cocoaModalSessionStack[i];
      if (info.widget == widget) {
         info.widget = 0;
         if (i == stackSize - 1) {
            // The top sessions ended. Interrupt the event dispatcher
            // to start spinning the correct session immidiatly:
            currentModalSessionCached = 0;
            cleanupModalSessionsNeeded = true;
            QEventDispatcherMac::instance()->interrupt();
         }
      }
   }
}

QEventDispatcherMacPrivate::QEventDispatcherMacPrivate()
{
}

QEventDispatcherMac::QEventDispatcherMac(QObject *parent)
   : QAbstractEventDispatcher(*new QEventDispatcherMacPrivate, parent)
{
   Q_D(QEventDispatcherMac);
   CFRunLoopSourceContext context;
   bzero(&context, sizeof(CFRunLoopSourceContext));
   context.info = d;
   context.equal = QEventDispatcherMacPrivate::postedEventSourceEqualCallback;
   context.perform = QEventDispatcherMacPrivate::postedEventsSourcePerformCallback;
   d->postedEventsSource = CFRunLoopSourceCreate(0, 0, &context);
   Q_ASSERT(d->postedEventsSource);
   CFRunLoopAddSource(mainRunLoop(), d->postedEventsSource, kCFRunLoopCommonModes);

   CFRunLoopObserverContext observerContext;
   bzero(&observerContext, sizeof(CFRunLoopObserverContext));
   observerContext.info = this;
   d->waitingObserver = CFRunLoopObserverCreate(kCFAllocatorDefault,
                        kCFRunLoopBeforeWaiting | kCFRunLoopAfterWaiting,
                        true, 0,
                        QEventDispatcherMacPrivate::waitingObserverCallback,
                        &observerContext);
   CFRunLoopAddObserver(mainRunLoop(), d->waitingObserver, kCFRunLoopCommonModes);

   /* The first cycle in the loop adds the source and the events of the source
      are not processed.
      We use an observer to process the posted events for the first
      execution of the loop. */
   CFRunLoopObserverContext firstTimeObserverContext;
   bzero(&firstTimeObserverContext, sizeof(CFRunLoopObserverContext));
   firstTimeObserverContext.info = d;
   d->firstTimeObserver = CFRunLoopObserverCreate(kCFAllocatorDefault,
                          kCFRunLoopEntry,
                          /* repeats = */ false,
                          0,
                          QEventDispatcherMacPrivate::firstLoopEntry,
                          &firstTimeObserverContext);
   CFRunLoopAddObserver(mainRunLoop(), d->firstTimeObserver, kCFRunLoopCommonModes);
}

void QEventDispatcherMacPrivate::waitingObserverCallback(CFRunLoopObserverRef,
      CFRunLoopActivity activity, void *info)
{
   if (activity == kCFRunLoopBeforeWaiting) {
      emit static_cast<QEventDispatcherMac *>(info)->aboutToBlock();
   } else {
      emit static_cast<QEventDispatcherMac *>(info)->awake();
   }
}

Boolean QEventDispatcherMacPrivate::postedEventSourceEqualCallback(const void *info1, const void *info2)
{
   return info1 == info2;
}

inline static void processPostedEvents(QEventDispatcherMacPrivate *const d, const bool blockSendPostedEvents)
{
   if (blockSendPostedEvents) {
      // We're told to not send posted events (because the event dispatcher
      // is currently working on setting up the correct session to run). But
      // we still need to make sure that we don't fall asleep until pending events
      // are sendt, so we just signal this need, and return:
      CFRunLoopSourceSignal(d->postedEventsSource);
      return;
   }


   if (d->cleanupModalSessionsNeeded) {
      d->cleanupModalSessions();
   }


   if (d->interrupt) {

      if (d->currentExecIsNSAppRun) {
         // The event dispatcher has been interrupted. But since
         // [NSApplication run] is running the event loop, we
         // delayed stopping it until now (to let cocoa process
         // pending cocoa events first).
         if (d->currentModalSessionCached) {
            d->temporarilyStopAllModalSessions();
         }
         [[NSApplication sharedApplication] stop: NSApp];
         d->cancelWaitForMoreEvents();
      }

      return;
   }

   auto tmp = d->serialNumber.load();

   QThreadData *threadData = d->internal_get_ThreadData();

   if (! threadData->canWait || (tmp != d->lastSerial)) {
      d->lastSerial = tmp;
      QApplicationPrivate::sendPostedEvents(0, 0, threadData);
   }
}

// internal
QThreadData *QEventDispatcherMacPrivate::internal_get_ThreadData()
{
   Q_Q(QEventDispatcherMac);

   return CSInternalThreadData::get_m_ThreadData(q);
}

void QEventDispatcherMacPrivate::firstLoopEntry(CFRunLoopObserverRef ref, CFRunLoopActivity activity, void *info)
{
   Q_UNUSED(ref);
   Q_UNUSED(activity);

   QApplicationPrivate::qt_initAfterNSAppStarted();
   processPostedEvents(static_cast<QEventDispatcherMacPrivate *>(info), blockSendPostedEvents);
}

void QEventDispatcherMacPrivate::postedEventsSourcePerformCallback(void *info)
{
   processPostedEvents(static_cast<QEventDispatcherMacPrivate *>(info), blockSendPostedEvents);
}


void QEventDispatcherMacPrivate::cancelWaitForMoreEvents()
{
   // In case the event dispatcher is waiting for more
   // events somewhere, we post a dummy event to wake it up:
   QMacCocoaAutoReleasePool pool;
   [[NSApplication sharedApplication] postEvent: [NSEvent otherEventWithType: NSApplicationDefined location: NSZeroPoint
                      modifierFlags: 0 timestamp: 0. windowNumber: 0 context: 0
                      subtype: QtCocoaEventSubTypeWakeup data1: 0 data2: 0] atStart: NO];
}

void QEventDispatcherMac::interrupt()
{
   Q_D(QEventDispatcherMac);
   d->interrupt = true;
   wakeUp();

   // We do nothing more here than setting d->interrupt = true, and
   // poke the event loop if it is sleeping. Actually stopping
   // NSApp, or the current modal session, is done inside the send
   // posted events callback. We do this to ensure that all current pending
   // cocoa events gets delivered before we stop. Otherwise, if we now stop
   // the last event loop recursion, cocoa will just drop pending posted
   // events on the floor before we get a chance to reestablish a new session.
   d->cancelWaitForMoreEvents();

}

QEventDispatcherMac::~QEventDispatcherMac()
{
   Q_D(QEventDispatcherMac);
   //timer cleanup
   MacTimerHash::iterator it = QEventDispatcherMacPrivate::macTimerHash.begin();
   while (it != QEventDispatcherMacPrivate::macTimerHash.end()) {
      MacTimerInfo *t = it.value();
      if (t->runLoopTimer) {
         CFRunLoopTimerInvalidate(t->runLoopTimer);
         CFRelease(t->runLoopTimer);
      }
      delete t;
      ++it;
   }
   QEventDispatcherMacPrivate::macTimerHash.clear();

   // Remove CFSockets from the runloop.
   for (MacSocketHash::const_iterator it = d->macSockets.constBegin(); it != d->macSockets.constEnd(); ++it) {
      MacSocketInfo *socketInfo = (*it);

      if (CFSocketIsValid(socketInfo->socket)) {
         qt_mac_remove_socket_from_runloop(socketInfo->socket, socketInfo->runloop);
         CFRunLoopSourceInvalidate(socketInfo->runloop);
         CFRelease(socketInfo->runloop);
         CFSocketInvalidate(socketInfo->socket);
         CFRelease(socketInfo->socket);
      }
   }
   CFRunLoopRemoveSource(mainRunLoop(), d->postedEventsSource, kCFRunLoopCommonModes);
   CFRelease(d->postedEventsSource);

   CFRunLoopObserverInvalidate(d->waitingObserver);
   CFRelease(d->waitingObserver);

   CFRunLoopObserverInvalidate(d->firstTimeObserver);
   CFRelease(d->firstTimeObserver);
}

QtMacInterruptDispatcherHelp *QtMacInterruptDispatcherHelp::instance = 0;

QtMacInterruptDispatcherHelp::QtMacInterruptDispatcherHelp() : cancelled(false)
{
   // The whole point of this class is that we enable a way to interrupt
   // the event dispatcher when returning back to a lower recursion level
   // than where interruptLater was called. This is needed to detect if
   // [[NSApplication sharedApplication] run] should still be running at the recursion level it is at.
   // Since the interrupt is canceled if processEvents is called before
   // this object gets deleted, we also avoid interrupting unnecessary.
   deleteLater();
}

QtMacInterruptDispatcherHelp::~QtMacInterruptDispatcherHelp()
{
   if (cancelled) {
      return;
   }
   instance = 0;
   QEventDispatcherMac::instance()->interrupt();
}

void QtMacInterruptDispatcherHelp::cancelInterruptLater()
{
   if (!instance) {
      return;
   }
   instance->cancelled = true;
   delete instance;
   instance = 0;
}

void QtMacInterruptDispatcherHelp::interruptLater()
{
   cancelInterruptLater();
   instance = new QtMacInterruptDispatcherHelp;
}

QT_END_NAMESPACE

