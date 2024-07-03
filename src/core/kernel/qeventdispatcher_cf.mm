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

#include <qeventdispatcher_cf_p.h>

#include <qdebug.h>
#include <qthread.h>

#include <qcoreapplication_p.h>
#include <qcore_unix_p.h>
#include <qcore_mac_p.h>
#include <qthread_p.h>

#include <limits>

#ifdef Q_OS_DARWIN
#  include <AppKit/NSApplication.h>
#else
#  include <UIKit/UIApplication.h>
#endif

@interface RunLoopModeTracker : NSObject
{
   QStack<CFStringRef> m_runLoopModes;
}
@end

@implementation RunLoopModeTracker

- (id) init
{
   if (self = [super init]) {
      m_runLoopModes.push(kCFRunLoopDefaultMode);

      [[NSNotificationCenter defaultCenter]
            addObserver:self
            selector:@selector(receivedNotification:)
            name:nil

#ifdef Q_OS_DARWIN
            object:[NSApplication sharedApplication]];
#else
            object:[UIApplication sharedApplication]];
#endif

   }

   return self;
}

- (void) dealloc
{
   [[NSNotificationCenter defaultCenter] removeObserver:self];

   [super dealloc];
}

static CFStringRef runLoopMode(NSDictionary *dictionary)
{
   for (NSString *key in dictionary) {
      if (CFStringHasSuffix((CFStringRef)key, CFSTR("RunLoopMode"))) {
         return (CFStringRef)[dictionary objectForKey: key];
      }
   }

   return nil;
}

- (void) receivedNotification:(NSNotification *) notification
{
   if (CFStringHasSuffix((CFStringRef)notification.name, CFSTR("RunLoopModePushNotification"))) {
      if (CFStringRef mode = runLoopMode(notification.userInfo)) {
         m_runLoopModes.push(mode);
      } else {
         qWarning("QEventDispatcher::receivedNotification() Encountered run loop push notification without run loop mode");
      }

   } else if (CFStringHasSuffix((CFStringRef)notification.name, CFSTR("RunLoopModePopNotification"))) {
      CFStringRef mode = runLoopMode(notification.userInfo);

      if (CFStringCompare(mode, [self currentMode], 0) == kCFCompareEqualTo) {
         m_runLoopModes.pop();
      } else {
         qWarning("QEventDispatcher::receivedNotification() Tried to pop run loop mode which was never pushed, %s",
               csPrintable(QCFString::toQString(mode)));
      }

      Q_ASSERT(m_runLoopModes.size() >= 1);
   }
}

- (CFStringRef) currentMode
{
   return m_runLoopModes.top();
}

@end

#define Q_MIRROR_ENUM(name) name = name

class RunLoopDebugger : public QObject
{
   CORE_CS_OBJECT(RunLoopDebugger)
   CORE_CS_ENUM(Activity)
   CORE_CS_ENUM(Result)

 public:
   enum Activity {
      Q_MIRROR_ENUM(kCFRunLoopEntry),
      Q_MIRROR_ENUM(kCFRunLoopBeforeTimers),
      Q_MIRROR_ENUM(kCFRunLoopBeforeSources),
      Q_MIRROR_ENUM(kCFRunLoopBeforeWaiting),
      Q_MIRROR_ENUM(kCFRunLoopAfterWaiting),
      Q_MIRROR_ENUM(kCFRunLoopExit)
   };

   enum Result {
      Q_MIRROR_ENUM(kCFRunLoopRunFinished),
      Q_MIRROR_ENUM(kCFRunLoopRunStopped),
      Q_MIRROR_ENUM(kCFRunLoopRunTimedOut),
      Q_MIRROR_ENUM(kCFRunLoopRunHandledSource)
   };
};

#define Q_ENUM_PRINTER(enumName) \
   static QString csPrintable##enumName(int value) \
   { \
      return RunLoopDebugger::staticMetaObject().enumerator(RunLoopDebugger::staticMetaObject().indexOfEnumerator(#enumName)).valueToKey(value); \
   }

Q_ENUM_PRINTER(Activity);
Q_ENUM_PRINTER(Result);

static QDebug operator<<(QDebug s, timespec tv)
{
   s << tv.tv_sec << "." << qSetFieldWidth(9) << qSetPadChar(QChar(48)) << tv.tv_nsec << reset;
   return s;
}

static const CFTimeInterval kCFTimeIntervalMinimum = 0;
static const CFTimeInterval kCFTimeIntervalDistantFuture = std::numeric_limits<CFTimeInterval>::max();

#pragma mark - Class definition

QEventDispatcherCoreFoundation::QEventDispatcherCoreFoundation(QObject *parent)
   : QAbstractEventDispatcher(parent), m_processEvents(QEventLoop::EventLoopExec),
     m_postedEventsRunLoopSource(this, &QEventDispatcherCoreFoundation::processPostedEvents),
     m_runLoopActivityObserver(this, &QEventDispatcherCoreFoundation::handleRunLoopActivity,

#if defined(CS_SHOW_DEBUG_CORE_OSX)
     kCFRunLoopAllActivities),
#else
     kCFRunLoopBeforeWaiting | kCFRunLoopAfterWaiting),
#endif

     m_runLoopModeTracker([[RunLoopModeTracker alloc] init]),  m_runLoopTimer(nullptr),
     m_blockedRunLoopTimer(nullptr), m_overdueTimerScheduled(false)
{
   m_cfSocketNotifier.setHostEventDispatcher(this);

   m_postedEventsRunLoopSource.addToMode(kCFRunLoopCommonModes);
   m_runLoopActivityObserver.addToMode(kCFRunLoopCommonModes);
}

QEventDispatcherCoreFoundation::~QEventDispatcherCoreFoundation()
{
   invalidateTimer();

   for (auto item : m_timerInfoList) {
      delete item;
   }

   m_cfSocketNotifier.removeSocketNotifiers();
}

bool QEventDispatcherCoreFoundation::processEvents(QEventLoop::ProcessEventsFlags flags)
{
   bool eventsProcessed = false;

   if (flags & (QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers)) {
      qWarning() << "QEventDispatcherCoreFoundation::processEvents() Flags not supported on iOS " << flags;
   }

#if defined(CS_SHOW_DEBUG_CORE_OSX)
   qDebug() << "QEventDispatcherCoreFoundation::processEvents() Entering with " << flags;
#endif

   if (m_blockedRunLoopTimer) {
      Q_ASSERT(m_blockedRunLoopTimer == m_runLoopTimer);

#if defined(CS_SHOW_DEBUG_CORE_OSX)
      qDebug() << "QEventDispatcherCoreFoundation::processEvents() Recursing from blocked timer "
            << m_blockedRunLoopTimer;
#endif

      m_runLoopTimer = nullptr; // Unset current timer to force creation of new timer
      updateTimers();
   }

   if (m_processEvents.deferredWakeUp) {
      // We may be processing events recursivly as a result of processing a posted event,
      // in which case we need to signal the run-loop source so that this iteration of
      // processEvents will take care of the newly posted events.
      m_postedEventsRunLoopSource.signal();
      m_processEvents.deferredWakeUp = false;

#if defined(CS_SHOW_DEBUG_CORE_OSX)
      qDebug() << "QEventDispatcherCoreFoundation::processEvents() Processed deferred wake-up";
#endif
   }

   // The documentation states that this signal is emitted after the event
   // loop returns from a function that could block, which is not the case
   // here, but all the other event dispatchers emit awake at the start of
   // processEvents, and the QEventLoop auto-test has an explicit check for
   // this behavior, so we assume it's for a good reason and do it as well.
   emit awake();

   ProcessEventsState previousState = m_processEvents;
   m_processEvents = ProcessEventsState(flags);

   bool returnAfterSingleSourceHandled = !(m_processEvents.flags & QEventLoop::EventLoopExec);

   while (true) {
      CFStringRef mode = [m_runLoopModeTracker currentMode];

      CFTimeInterval duration = (m_processEvents.flags & QEventLoop::WaitForMoreEvents) ?
            kCFTimeIntervalDistantFuture : kCFTimeIntervalMinimum;

#if defined(CS_SHOW_DEBUG_CORE_OSX)
      qDebug() << "QEventDispatcherCoreFoundation::processEvents() Calling CFRunLoopRunInMode = "
            << csPrintable(QCFString::toQString(mode))
            << " for " << duration << " ms, processing single source = " << returnAfterSingleSourceHandled;
#endif

      SInt32 result = CFRunLoopRunInMode(mode, duration, returnAfterSingleSourceHandled);

#if defined(CS_SHOW_DEBUG_CORE_OSX)
      qDebug() << "QEventDispatcherCoreFoundation::processEvents() result = " << csPrintableResult(result);
#endif

      eventsProcessed |= (result == kCFRunLoopRunHandledSource
                  || m_processEvents.processedPostedEvents
                  || m_processEvents.processedTimers);

      if (result == kCFRunLoopRunFinished) {
         // should only happen at application shutdown, as the main runloop
         // will presumably always have sources registered
         break;

      } else if (m_processEvents.wasInterrupted) {

         if (m_processEvents.flags & QEventLoop::EventLoopExec) {
            Q_ASSERT(result == kCFRunLoopRunStopped);

            // The runloop was potentially stopped (interrupted) as a response to
            // an event loop being asked to exit. We check the topmost eventloop
            // is still supposed to keep going and return if not. Note that the runloop
            // might get stopped as a result of a non-top eventloop being asked to exit,
            // in which case we continue running the top event loop until that is asked
            // to exit, and then unwind back to the previous event loop which will break
            // immediately, since it has already been exited.

            QEventLoop *currentEventLoop = QThreadData::current()->eventLoops.top();
            Q_ASSERT(currentEventLoop);

            if (! currentEventLoop->isRunning()) {

#if defined(CS_SHOW_DEBUG_CORE_OSX)
               qDebug() << "QEventDispatcherCoreFoundation::processEvents() Top level event loop was exited";
#endif
               break;

            } else {

#if defined(CS_SHOW_DEBUG_CORE_OSX)
               qDebug() << "QEventDispatcherCoreFoundation::processEvents() "
                     << "Top level event loop still running, making another pass";
#endif
            }

         } else {
            // We were called manually, through processEvents(), and should stop processing
            // events, even if we did not finish processing all the queued events.

#if defined(CS_SHOW_DEBUG_CORE_OSX)
            qDebug() << "QEventDispatcherCoreFoundation::processEvents() Top level processEvents was interrupted";
#endif

            break;
         }
      }

      if (m_processEvents.flags & QEventLoop::EventLoopExec) {
         // We were called from QEventLoop's exec(), which blocks until the event
         // loop is asked to exit by calling processEvents repeatedly. Instead of
         // re-entering this method again and again from QEventLoop, we can block
         // here,  one lever closer to CFRunLoopRunInMode, by running the native
         // event loop again and again until we're interrupted by QEventLoop.
         continue;

      } else {
         // We were called 'manually', through processEvents()

         if (result == kCFRunLoopRunHandledSource) {
            // We processed one or more sources, but there might still be other
            // sources that did not get a chance to process events, so we need
            // to do another pass.

            // But we should only wait for more events the first time
            m_processEvents.flags &= ~QEventLoop::WaitForMoreEvents;
            continue;

         } else if (m_overdueTimerScheduled && !m_processEvents.processedTimers) {
            // CFRunLoopRunInMode does not guarantee a scheduled timer with a fire
            // date in the past (overdue) will fire on the next run loop pass.
            // The CS API documents zero-interval timers to always be
            // handled after processing all available window system events.

#if defined(CS_SHOW_DEBUG_CORE_OSX)
            qDebug() << "QEventDispatcherCoreFoundation::processEvents() Manually processing timers due to overdue timer";
#endif

            processTimers(nullptr);
            eventsProcessed = true;
         }
      }

      break;
   }

   if (m_blockedRunLoopTimer) {
      invalidateTimer();
      m_runLoopTimer = m_blockedRunLoopTimer;
   }

   if (m_processEvents.deferredUpdateTimers) {
      updateTimers();
   }

   if (m_processEvents.deferredWakeUp) {
      m_postedEventsRunLoopSource.signal();

#if defined(CS_SHOW_DEBUG_CORE_OSX)
      qDebug() << "QEventDispatcherCoreFoundation::processEvents() Processed deferred wake up";
#endif
   }

   bool wasInterrupted = m_processEvents.wasInterrupted;

   // Restore state of previous processEvents() call
   m_processEvents = previousState;

   if (wasInterrupted) {
      // The current processEvents run has been interrupted, but there may still be
      // others below it (eg, in the case of nested event loops). We need to trigger
      // another interrupt so that the parent processEvents call has a chance to check
      // if it should continue.

#if defined(CS_SHOW_DEBUG_CORE_OSX)
      qDebug() << "QEventDispatcherCoreFoundation::processEvents() Forwarding interrupt in case of nested processEvents";
#endif

      interrupt();
   }

#if defined(CS_SHOW_DEBUG_CORE_OSX)
   qDebug() << "QEventDispatcherCoreFoundation::processEvents() Returning with eventsProcessed = " << eventsProcessed;
#endif

   return eventsProcessed;
}

bool QEventDispatcherCoreFoundation::processPostedEvents()
{
   if (m_processEvents.processedPostedEvents && !(m_processEvents.flags & QEventLoop::EventLoopExec)) {

#if defined(CS_SHOW_DEBUG_CORE_OSX)
      qDebug() << "QEventDispatcherCoreFoundation::processPostedEvents() Already processed events this pass";
#endif

      return false;
   }

   m_processEvents.processedPostedEvents = true;

#if defined(CS_SHOW_DEBUG_CORE_OSX)
   qDebug() << "QEventDispatcherCoreFoundation::processPostedEvents() Sending posted events for " << m_processEvents.flags;
#endif

   QCoreApplication::sendPostedEvents();

   return true;
}

void QEventDispatcherCoreFoundation::processTimers(CFRunLoopTimerRef timer)
{
   if (m_processEvents.processedTimers && !(m_processEvents.flags & QEventLoop::EventLoopExec)) {

#if defined(CS_SHOW_DEBUG_CORE_OSX)
      qDebug() << "QEventDispatcherCoreFoundation::processTimers() Already processed timers this pass";
#endif

      m_processEvents.deferredUpdateTimers = true;
      return;
   }

#if defined(CS_SHOW_DEBUG_CORE_OSX)
   qDebug() << "QEventDispatcherCoreFoundation::processTimers() CFRunLoopTimer "
         << timer << " fired, activating timers";
#endif

   // Activating timers might recurse into processEvents() if a timer-callback
   // brings up a new event-loop or tries to processes events manually. Although
   // a CFRunLoop can recurse inside its callbacks, a single CFRunLoopTimer can
   // not. So, for each recursion into processEvents() from a timer-callback we
   // need to set up a new timer-source. Instead of doing it preemtivly each
   // time we activate timers, we set a flag and let processEvents()
   // decide whether or not it needs to bring up a new timer source.

   // We may have multiple recused timers, so keep track of the previous blocked timer
   CFRunLoopTimerRef previouslyBlockedRunLoopTimer = m_blockedRunLoopTimer;

   m_blockedRunLoopTimer = timer;
   m_timerInfoList.activateTimers();
   m_blockedRunLoopTimer = previouslyBlockedRunLoopTimer;
   m_processEvents.processedTimers = true;

   // Now that the timer source is unblocked we may need to schedule it again
   updateTimers();
}

void QEventDispatcherCoreFoundation::handleRunLoopActivity(CFRunLoopActivity activity)
{
#if defined(CS_SHOW_DEBUG_CORE_OSX)
   qDebug() << "QEventDispatcherCoreFoundation::handleRunLoopActivity() " << csPrintableActivity(activity);
#endif

   switch (activity) {
      case kCFRunLoopBeforeWaiting:
         if (m_processEvents.processedTimers && !(m_processEvents.flags & QEventLoop::EventLoopExec)
               && m_processEvents.flags & QEventLoop::WaitForMoreEvents) {

            // CoreFoundation does not treat a timer as a reason to exit CFRunLoopRunInMode
            // when asked to only process a single source, so we risk waiting a long time for
            // a 'proper' source to fire (typically a system source that we don't control).
            // To fix this we do an explicit interrupt after processing our timer, so that
            // processEvents() gets a chance to re-evaluate the state of things.

            interrupt();
         }

         emit aboutToBlock();
         break;

      case kCFRunLoopAfterWaiting:
         emit awake();
         break;

#if defined(CS_SHOW_DEBUG_CORE_OSX)
      case kCFRunLoopEntry:
      case kCFRunLoopBeforeTimers:
      case kCFRunLoopBeforeSources:
      case kCFRunLoopExit:
         break;
#endif

      default:
         // may want to throw
         break;
   }
}

bool QEventDispatcherCoreFoundation::hasPendingEvents()
{
   // There doesn't seem to be any API on iOS to peek into the other sources
   // to figure out if there are pending non-CS events. As a workaround
   // assume that if the run-loop is currently blocking and waiting for a
   // source to signal then there are no system-events pending. If this
   // function is called from the main thread then the second clause
   // of the condition will always be true, as the run loop is
   // never waiting in that case. The function would be more aptly named
   // 'maybeHasPendingEvents' in our case.

   extern uint qGlobalPostedEventsCount();
   return qGlobalPostedEventsCount() || !CFRunLoopIsWaiting(CFRunLoopGetMain());
}

void QEventDispatcherCoreFoundation::wakeUp()
{
   if (m_processEvents.processedPostedEvents && !(m_processEvents.flags & QEventLoop::EventLoopExec)) {
      // A manual processEvents call should only result in processing the events posted
      // up until then. Any newly posted events as result of processing existing posted
      // events should be handled in the next call to processEvents(). Since we're using
      // a run-loop source to process our posted events we need to prevent it from being
      // signaled as a result of posting new events, otherwise we end up in an infinite
      // loop. We do however need to signal the source at some point, so that the newly
      // posted event gets processed on the next processEvents() call, so we flag the
      // need to do a deferred wake-up.

      m_processEvents.deferredWakeUp = true;

#if defined(CS_SHOW_DEBUG_CORE_OSX)
      qDebug() << "QEventDispatcherCoreFoundation::wakeUp() Already processed posted events, deferring wakeUp";
#endif

      return;
   }

   m_postedEventsRunLoopSource.signal();
   CFRunLoopWakeUp(CFRunLoopGetMain());

#if defined(CS_SHOW_DEBUG_CORE_OSX)
   qDebug() << "QEventDispatcherCoreFoundation::wakeUp() Signaled posted event run-loop source";
#endif
}

void QEventDispatcherCoreFoundation::interrupt()
{
#if defined(CS_SHOW_DEBUG_CORE_OSX)
   qDebug() << "QEventDispatcherCoreFoundation::interrupt() Marking current processEvent as interrupted";
#endif

   m_processEvents.wasInterrupted = true;
   CFRunLoopStop(CFRunLoopGetMain());
}

void QEventDispatcherCoreFoundation::flush()
{
   // X11 only
}

#pragma mark - Socket notifiers

void QEventDispatcherCoreFoundation::registerSocketNotifier(QSocketNotifier *notifier)
{
   m_cfSocketNotifier.registerSocketNotifier(notifier);
}

void QEventDispatcherCoreFoundation::unregisterSocketNotifier(QSocketNotifier *notifier)
{
   m_cfSocketNotifier.unregisterSocketNotifier(notifier);
}

#pragma mark - Timers

void QEventDispatcherCoreFoundation::registerTimer(int timerId, int interval, Qt::TimerType timerType, QObject *object)
{
#if defined(CS_SHOW_DEBUG_CORE_OSX)
   qDebug() << "QEventDispatcherCoreFoundation::registerTimer() id = " << timerId << ", interval = " << interval
         << ", type = " << timerType << ", object = " << object;
#endif

   Q_ASSERT(timerId > 0 && interval >= 0 && object);
   Q_ASSERT(object->thread() == thread() && thread() == QThread::currentThread());

   m_timerInfoList.registerTimer(timerId, interval, timerType, object);
   updateTimers();
}

bool QEventDispatcherCoreFoundation::unregisterTimer(int timerId)
{
   Q_ASSERT(timerId > 0);
   Q_ASSERT(thread() == QThread::currentThread());

   bool returnValue = m_timerInfoList.unregisterTimer(timerId);

#if defined(CS_SHOW_DEBUG_CORE_OSX)
   qDebug() << "QEventDispatcherCoreFoundation::unregisterTimer() id = " << timerId << ", timers left: "
         << m_timerInfoList.size();
#endif

   updateTimers();

   return returnValue;
}

bool QEventDispatcherCoreFoundation::unregisterTimers(QObject *object)
{
   Q_ASSERT(object && object->thread() == thread() && thread() == QThread::currentThread());

   bool returnValue = m_timerInfoList.unregisterTimers(object);

#if defined(CS_SHOW_DEBUG_CORE_OSX)
   qDebug() << "QEventDispatcherCoreFoundation::unregisterTimers() object = " << object << ", timers left: "
         << m_timerInfoList.size();
#endif

   updateTimers();

   return returnValue;
}

QList<QTimerInfo> QEventDispatcherCoreFoundation::registeredTimers(QObject *object) const
{
   Q_ASSERT(object);
   return m_timerInfoList.registeredTimers(object);
}

int QEventDispatcherCoreFoundation::remainingTime(int timerId)
{
   Q_ASSERT(timerId > 0);
   return m_timerInfoList.timerRemainingTime(timerId);
}

static double timespecToSeconds(const timespec &spec)
{
   static double nanosecondsPerSecond = 1.0 * 1000 * 1000 * 1000;
   return spec.tv_sec + (spec.tv_nsec / nanosecondsPerSecond);
}

void QEventDispatcherCoreFoundation::updateTimers()
{
   if (m_timerInfoList.size() > 0) {
      // timers are registered, so create or reschedule CF timer to match

      timespec tv = { -1, -1 };

      CFAbsoluteTime timeToFire = m_timerInfoList.timerWait(tv)
            ? CFAbsoluteTimeGetCurrent() + timespecToSeconds(tv)  // timer ready to fire right now, or some time in the future
            : kCFTimeIntervalDistantFuture;                       // have timers, but they are all currently blocked by callbacks

      if (! m_runLoopTimer) {
         m_runLoopTimer = CFRunLoopTimerCreateWithHandler(kCFAllocatorDefault,
               timeToFire, kCFTimeIntervalDistantFuture, 0, 0,
               ^(CFRunLoopTimerRef timer) { processTimers(timer); } );

         CFRunLoopAddTimer(CFRunLoopGetMain(), m_runLoopTimer, kCFRunLoopCommonModes);

#if defined(CS_SHOW_DEBUG_CORE_OSX)
         qDebug() << "QEventDispatcherCoreFoundation::updateTimers() Created new CFRunLoopTimer " << m_runLoopTimer;
#endif

      } else {
         CFRunLoopTimerSetNextFireDate(m_runLoopTimer, timeToFire);

#if defined(CS_SHOW_DEBUG_CORE_OSX)
         qDebug() << "QEventDispatcherCoreFoundation::updateTimers() Re-scheduled CFRunLoopTimer " << m_runLoopTimer;
#endif
      }

      m_overdueTimerScheduled = ! timespecToSeconds(tv);

#if defined(CS_SHOW_DEBUG_CORE_OSX)
      qDebug() << "QEventDispatcherCoreFoundation::updateTimers() Next timeout in " << tv << " seconds";
#endif

   } else {
      // No timers are registered, so make sure we are not running any CF timers
      invalidateTimer();

      m_overdueTimerScheduled = false;
   }
}

void QEventDispatcherCoreFoundation::invalidateTimer()
{
   if (! m_runLoopTimer || (m_runLoopTimer == m_blockedRunLoopTimer)) {
      return;
   }

   CFRunLoopTimerInvalidate(m_runLoopTimer);

#if defined(CS_SHOW_DEBUG_CORE_OSX)
   qDebug() << "QEventDispatcherCoreFoundation::invalidateTimer() Invalidated CFRunLoopTimer " << m_runLoopTimer;
#endif

   CFRelease(m_runLoopTimer);
   m_runLoopTimer = nullptr;
}
