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

#include <qeventdispatcher_win_p.h>

#include <qcoreapplication.h>
#include <qhash.h>
#include <qpair.h>
#include <qset.h>
#include <qstringparser.h>
#include <qsocketnotifier.h>
#include <qvarlengtharray.h>
#include <qwineventnotifier.h>

#include <qcoreapplication_p.h>
#include <qmutexpool_p.h>
#include <qsystemlibrary_p.h>
#include <qthread_p.h>

HINSTANCE qWinAppInst();
extern uint qGlobalPostedEventsCount();

#ifndef TIME_KILL_SYNCHRONOUS
#  define TIME_KILL_SYNCHRONOUS 0x0100
#endif

#ifndef QS_RAWINPUT
#  define QS_RAWINPUT 0x0400
#endif

#ifndef WM_TOUCH
#  define WM_TOUCH 0x0240
#endif

#ifndef QT_NO_GESTURES

#ifndef WM_GESTURE
#  define WM_GESTURE 0x0119
#endif

#ifndef WM_GESTURENOTIFY
#  define WM_GESTURENOTIFY 0x011A
#endif

#endif // QT_NO_GESTURES

static constexpr UINT_PTR WM_CS_SOCKET_NOTIFIER    = WM_USER;
static constexpr UINT_PTR WM_CS_SENDPOSTED_EVENTS  = WM_USER + 1;
static constexpr UINT_PTR WM_CS_ACTIVATE_NOTIFIERS = WM_USER + 2;
static constexpr UINT_PTR WM_CS_SENDPOSTED_TIMER   = WM_USER + 3;

class QEventDispatcherWin32Private;

#if ! defined(DWORD_PTR) && ! defined(Q_OS_WIN64)
#define DWORD_PTR DWORD
#endif

using ptimeSetEvent  = MMRESULT(WINAPI *)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);
using ptimeKillEvent = MMRESULT(WINAPI *)(UINT);

static ptimeSetEvent qtimeSetEvent   = nullptr;
static ptimeKillEvent qtimeKillEvent = nullptr;

LRESULT QT_WIN_CALLBACK qt_internal_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp);

static void resolveTimerAPI()
{
   static bool triedResolve = false;

   if (! triedResolve) {
      QRecursiveMutexLocker locker(QMutexPool::globalInstanceGet(&triedResolve));

      if (triedResolve) {
         return;
      }

      triedResolve   = true;
      qtimeSetEvent  = (ptimeSetEvent)QSystemLibrary::resolve(QLatin1String("winmm"), "timeSetEvent");
      qtimeKillEvent = (ptimeKillEvent)QSystemLibrary::resolve(QLatin1String("winmm"), "timeKillEvent");
   }
}

QEventDispatcherWin32Private::QEventDispatcherWin32Private()
   : threadId(GetCurrentThreadId()), interrupt(false),
     closingDown(false), internalHwnd(nullptr),
     getMessageHook(nullptr), serialNumber(0), lastSerialNumber(0),
     sendPostedEventsWindowsTimerId(0),
     wakeUps(0), activateNotifiersPosted(false)
{
   resolveTimerAPI();
}

QEventDispatcherWin32Private::~QEventDispatcherWin32Private()
{
   if (internalHwnd) {
      DestroyWindow(internalHwnd);
   }
}

void QEventDispatcherWin32Private::activateEventNotifier(QWinEventNotifier *wen)
{
   QEvent event(QEvent::WinEventAct);
   QCoreApplication::sendEvent(wen, &event);
}

// called by a workerthread
void WINAPI QT_WIN_CALLBACK qt_fast_timer_proc(uint timerId, uint, DWORD_PTR user, DWORD_PTR, DWORD_PTR)
{
   if (! timerId) {
      // sanity check
      return;
   }

   WinTimerInfo *t = (WinTimerInfo *)user;

   Q_ASSERT(t);
   QCoreApplication::postEvent(t->dispatcher, new QTimerEvent(t->timerId));
}

LRESULT QT_WIN_CALLBACK qt_internal_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
   if (message == WM_NCCREATE) {
      return true;
   }

   MSG msg;
   msg.hwnd = hwnd;
   msg.message = message;
   msg.wParam = wp;
   msg.lParam = lp;

   QAbstractEventDispatcher *dispatcher = QAbstractEventDispatcher::instance();
   long result;

   if (! dispatcher) {
      if (message == WM_TIMER) {
         KillTimer(hwnd, wp);
      }

      return 0;

   } else if (dispatcher->filterNativeEvent("windows_dispatcher_MSG", &msg, &result)) {
      return result;
   }

#ifdef GWLP_USERDATA
   QEventDispatcherWin32 *q = (QEventDispatcherWin32 *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
#else
   QEventDispatcherWin32 *q = (QEventDispatcherWin32 *) GetWindowLong(hwnd, GWL_USERDATA);
#endif

   QEventDispatcherWin32Private *d = nullptr;

   if (q != nullptr) {
      d = q->d_func();
   }

   if (message == WM_CS_SOCKET_NOTIFIER) {
      // socket notifier message
      int type = -1;

      switch (WSAGETSELECTEVENT(lp)) {
         case FD_READ:
         case FD_ACCEPT:
            type = 0;
            break;

         case FD_WRITE:
         case FD_CONNECT:
            type = 1;
            break;

         case FD_OOB:
            type = 2;
            break;

         case FD_CLOSE:
            type = 3;
            break;
      }

      if (type >= 0) {
         Q_ASSERT(d != nullptr);
         QSNDict *sn_vec[4] = { &d->sn_read, &d->sn_write, &d->sn_except, &d->sn_read };
         QSNDict *dict = sn_vec[type];

         QSockNot *sn = dict ? dict->value(wp) : nullptr;

         if (sn) {
            d->doWsaAsyncSelect(sn->fd, 0);
            d->active_fd[sn->fd].selected = false;
            d->postActivateSocketNotifiers();

            if (type < 3) {
               QEvent event(QEvent::SockAct);
               QCoreApplication::sendEvent(sn->obj, &event);

            } else {
               QEvent event(QEvent::SockClose);
               QCoreApplication::sendEvent(sn->obj, &event);
            }
         }
      }

      return 0;

   } else if (message == WM_CS_ACTIVATE_NOTIFIERS) {
      Q_ASSERT(d != nullptr);

      // register all socket notifiers

      auto iter     = d->active_fd.begin();
      auto iter_end = d->active_fd.end();

      while (iter != iter_end) {
         QSockFd &sd = iter.value();

         if (! sd.selected) {
            d->doWsaAsyncSelect(iter.key(), sd.event);
            sd.selected = true;
         }

         ++iter;
      }

      d->activateNotifiersPosted = false;

      return 0;

   } else if (message == WM_CS_SENDPOSTED_EVENTS ||
         (message == WM_TIMER && d->sendPostedEventsWindowsTimerId != 0 &&
         wp == (uint)d->sendPostedEventsWindowsTimerId)) {

      // we also use a Windows timer to send posted events when the message queue is full
      const int localSerialNumber = d->serialNumber.load();

      // calling a method in the private class
      // QThreadData *threadData = d->get_m_ThreadData();

      if (localSerialNumber != d->lastSerialNumber) {
         d->lastSerialNumber = localSerialNumber;
         q->sendPostedEvents();
      }

      return 0;

   } else if (message == WM_TIMER) {
      Q_ASSERT(d != nullptr);
      d->sendTimerEvent(wp);
      return 0;
   }

   return DefWindowProc(hwnd, message, wp, lp);
}

static inline UINT inputTimerMask()
{
   UINT result = QS_TIMER | QS_INPUT | QS_RAWINPUT;

   // QS_POINTER became part of QS_INPUT in Windows 8
   // following code is for Windows 8

#if defined(QS_TOUCH) && defined(QS_POINTER)

   if (QSysInfo::WindowsVersion < QSysInfo::WV_WINDOWS8) {
      result &= ~(QS_TOUCH | QS_POINTER);
   }

#endif

   return result;
}

LRESULT QT_WIN_CALLBACK qt_GetMessageHook(int code, WPARAM wp, LPARAM lp)
{
   QEventDispatcherWin32 *q = qobject_cast<QEventDispatcherWin32 *>(QAbstractEventDispatcher::instance());
   Q_ASSERT(q != nullptr);

   if (wp == PM_REMOVE) {

      if (q) {
         MSG *msg = (MSG *) lp;
         QEventDispatcherWin32Private *d = q->d_func();
         const int localSerialNumber = d->serialNumber.load();
         static const UINT mask = inputTimerMask();

         if (HIWORD(GetQueueStatus(mask)) == 0) {
            // no more input or timer events in the message queue, allow posted events to be sent normally now

            if (d->sendPostedEventsWindowsTimerId != 0) {
               // stop the timer to send posted events, since we now allow the WM_CS_SENDPOSTED_EVENTS message
               KillTimer(d->internalHwnd, d->sendPostedEventsWindowsTimerId);
               d->sendPostedEventsWindowsTimerId = 0;
            }

            (void) d->wakeUps.fetchAndStoreRelease(0);

            if (localSerialNumber != d->lastSerialNumber &&
                  (msg->hwnd != d->internalHwnd || msg->message != WM_CS_SENDPOSTED_EVENTS)) {

               // if this message is the one which triggers sendPostedEvents() there is no need to post it again
               PostMessage(d->internalHwnd, WM_CS_SENDPOSTED_EVENTS, 0, 0);
            }

         } else if (d->sendPostedEventsWindowsTimerId == 0 && localSerialNumber != d->lastSerialNumber) {

            // start a special timer to continue delivering posted events while
            // there are still input and timer messages in the message queue

            d->sendPostedEventsWindowsTimerId = SetTimer(d->internalHwnd, WM_CS_SENDPOSTED_TIMER, 0, nullptr);

            // specify zero but Windows uses USER_TIMER_MINIMUM
            // return value of SetTimer() is not checked
         }
      }
   }

   return q->d_func()->getMessageHook ? CallNextHookEx(nullptr, code, wp, lp) : 0;
}

struct QWindowsMessageWindowClassContext {
   QWindowsMessageWindowClassContext();
   ~QWindowsMessageWindowClassContext();

   ATOM atom;
   std::wstring className;
};

QWindowsMessageWindowClassContext::QWindowsMessageWindowClassContext()
   : atom(0)
{
   // make sure that multiple instances can coexist in the same process
   const QString classStr = "QEventDispatcherWin32_Internal_Widget" + QString::number(quintptr(qt_internal_proc));

   WNDCLASS wc;
   wc.style         = 0;
   wc.lpfnWndProc   = qt_internal_proc;
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = 0;
   wc.hInstance     = qWinAppInst();
   wc.hIcon         = nullptr;
   wc.hCursor       = nullptr;
   wc.hbrBackground = nullptr;
   wc.lpszMenuName  = NULL;

   className = classStr.toStdWString();
   wc.lpszClassName = &className[0];

   atom = RegisterClass(&wc);

   if (! atom) {
      qErrnoWarning("%s RegisterClass() failed", csPrintable(classStr));
      className.clear();
   }
}

QWindowsMessageWindowClassContext::~QWindowsMessageWindowClassContext()
{
   if (! className.empty()) {
      UnregisterClass(&className[0], qWinAppInst());
   }
}

static QWindowsMessageWindowClassContext *qWindowsMessageWindowClassContext()
{
   static QWindowsMessageWindowClassContext retval;
   return &retval;
}

static HWND qt_create_internal_window(const QEventDispatcherWin32 *eventDispatcher)
{
   QWindowsMessageWindowClassContext *ctx = qWindowsMessageWindowClassContext();

   if (! ctx->atom) {
      return nullptr;
   }

   HWND parent = HWND_MESSAGE;

   HWND wnd = CreateWindow(&(ctx->className)[0], &(ctx->className)[0],        // classname, window name
         0,                // style
         0, 0, 0, 0,       // geometry
         parent,           // parent
         nullptr,          // menu handle
         qWinAppInst(),    // application
         nullptr);         // windows creation data.

   if (! wnd) {
      qWarning("QEventDispatcher() Failed to create QEventDispatcherWin32 internal window, %d\n", (int)GetLastError());
      return nullptr;
   }

#ifdef GWLP_USERDATA
   SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)eventDispatcher);
#else
   SetWindowLong(wnd, GWL_USERDATA, (LONG)eventDispatcher);
#endif

   return wnd;
}

static void calculateNextTimeout(WinTimerInfo *t, quint64 currentTime)
{
   uint interval = t->interval;

   if ((interval >= 20000u && t->timerType != Qt::PreciseTimer) || t->timerType == Qt::VeryCoarseTimer) {
      // round the interval, VeryCoarseTimers only have full second accuracy
      interval = ((interval + 500)) / 1000 * 1000;
   }

   t->interval = interval;
   t->timeout = currentTime + interval;
}
void QEventDispatcherWin32Private::registerTimer(WinTimerInfo *t)
{
   Q_ASSERT(internalHwnd);

   Q_Q(QEventDispatcherWin32);

   int ok = 0;
   calculateNextTimeout(t, qt_msectime());
   uint interval = t->interval;

   if (interval == 0u) {
      // optimization for single-shot-zero-timer
      QCoreApplication::postEvent(q, new QZeroTimerEvent(t->timerId));
      ok = 1;

   } else if ((interval < 20u || t->timerType == Qt::PreciseTimer) && qtimeSetEvent) {
      ok = t->fastTimerId = qtimeSetEvent(interval, 1, qt_fast_timer_proc, (DWORD_PTR)t,
            TIME_CALLBACK_FUNCTION | TIME_PERIODIC | TIME_KILL_SYNCHRONOUS);
   }

   if (ok == 0) {
      // fall back to normal timer if no more multimedia timers available
      ok = SetTimer(internalHwnd, t->timerId, interval, nullptr);
   }

   if (ok == 0) {
      qErrnoWarning("QEventDispatcherWin32::registerTimer: Failed to create a timer");
   }
}

void QEventDispatcherWin32Private::unregisterTimer(WinTimerInfo *t)
{
   if (t->interval == 0) {
      QCoreApplicationPrivate::removePostedTimerEvent(t->dispatcher, t->timerId);

   } else if (t->fastTimerId != 0) {
      qtimeKillEvent(t->fastTimerId);
      QCoreApplicationPrivate::removePostedTimerEvent(t->dispatcher, t->timerId);

   } else if (internalHwnd) {
      KillTimer(internalHwnd, t->timerId);
   }

   delete t;
}

void QEventDispatcherWin32Private::sendTimerEvent(int timerId)
{
   WinTimerInfo *t = timerDict.value(timerId);

   if (t && ! t->inTimerEvent) {
      // send event, but don't allow it to recurse
      t->inTimerEvent = true;

      // recalculate next emission
      calculateNextTimeout(t, qt_msectime());
      QTimerEvent e(t->timerId);
      QCoreApplication::sendEvent(t->obj, &e);

      // timer could have been removed
      t = timerDict.value(timerId);

      if (t) {
         t->inTimerEvent = false;
      }
   }
}

void QEventDispatcherWin32Private::doWsaAsyncSelect(int socket, long event)
{
   Q_ASSERT(internalHwnd);

   // BoundsChecker may emit a warning for WSAAsyncSelect when event == 0
   // This is a BoundsChecker bug and not a bug

   WSAAsyncSelect(socket, internalHwnd, event ? int(WM_CS_SOCKET_NOTIFIER) : 0, event);
}

void QEventDispatcherWin32Private::postActivateSocketNotifiers()
{
   if (! activateNotifiersPosted) {
      activateNotifiersPosted = PostMessage(internalHwnd, WM_CS_ACTIVATE_NOTIFIERS, 0, 0);
   }
}

void QEventDispatcherWin32::createInternalHwnd()
{
   Q_D(QEventDispatcherWin32);

   if (d->internalHwnd) {
      return;
   }

   d->internalHwnd = qt_create_internal_window(this);
   installMessageHook();

   // start all normal timers
   for (int i = 0; i < d->timerVec.count(); ++i) {
      d->registerTimer(d->timerVec.at(i));
   }
}

void QEventDispatcherWin32::installMessageHook()
{
   Q_D(QEventDispatcherWin32);

   if (d->getMessageHook) {
      return;
   }

   // setup GetMessage hook needed to drive our posted events
   d->getMessageHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC) qt_GetMessageHook, nullptr, GetCurrentThreadId());

   if (! d->getMessageHook) {
      int errorCode = GetLastError();

      qFatal("INTERNAL ERROR: failed to install GetMessage hook: %d, %s",
            errorCode, csPrintable(qt_error_string(errorCode)));
   }
}

void QEventDispatcherWin32::uninstallMessageHook()
{
   Q_D(QEventDispatcherWin32);

   if (d->getMessageHook) {
      UnhookWindowsHookEx(d->getMessageHook);
   }

   d->getMessageHook = nullptr;
}

QEventDispatcherWin32::QEventDispatcherWin32(QObject *parent)
   : QAbstractEventDispatcher(*new QEventDispatcherWin32Private, parent)
{
}

QEventDispatcherWin32::QEventDispatcherWin32(QEventDispatcherWin32Private &dd, QObject *parent)
   : QAbstractEventDispatcher(dd, parent)
{
}

QEventDispatcherWin32::~QEventDispatcherWin32()
{
}

bool QEventDispatcherWin32::processEvents(QEventLoop::ProcessEventsFlags flags)
{
   Q_D(QEventDispatcherWin32);

   if (! d->internalHwnd) {
      createInternalHwnd();
      wakeUp(); // trigger a call to sendPostedEvents()
   }

   d->interrupt = false;
   emit awake();

   bool canWait;
   bool retVal = false;
   bool seenWM_QT_SENDPOSTEDEVENTS = false;
   bool needWM_QT_SENDPOSTEDEVENTS = false;

   do {
      DWORD waitRet = 0;
      HANDLE pHandles[MAXIMUM_WAIT_OBJECTS - 1];
      QVarLengthArray<MSG> processedTimers;

      while (! d->interrupt) {
         DWORD nCount = d->winEventNotifierList.count();
         Q_ASSERT(nCount < MAXIMUM_WAIT_OBJECTS - 1);

         MSG msg;
         bool haveMessage;

         if (! (flags & QEventLoop::ExcludeUserInputEvents) && ! d->queuedUserInputEvents.isEmpty()) {
            // process queued user input events
            haveMessage = true;
            msg = d->queuedUserInputEvents.takeFirst();

         } else if (! (flags & QEventLoop::ExcludeSocketNotifiers) && ! d->queuedSocketEvents.isEmpty()) {
            // process queued socket events
            haveMessage = true;
            msg = d->queuedSocketEvents.takeFirst();

         } else {
            haveMessage = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);

            if (haveMessage) {
               if ((flags & QEventLoop::ExcludeUserInputEvents)
                     && ((msg.message >= WM_KEYFIRST  && msg.message <= WM_KEYLAST)
                     || (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)
                     || msg.message == WM_MOUSEWHEEL
                     || msg.message == WM_MOUSEHWHEEL
                     || msg.message == WM_TOUCH

#ifndef QT_NO_GESTURES
                     || msg.message == WM_GESTURE
                     || msg.message == WM_GESTURENOTIFY
#endif

                     || msg.message == WM_CLOSE)) {

                  // queue user input events for later processing
                  d->queuedUserInputEvents.append(msg);
                  continue;
               }

               if ((flags & QEventLoop::ExcludeSocketNotifiers)
                     && (msg.message == WM_CS_SOCKET_NOTIFIER && msg.hwnd == d->internalHwnd)) {

                  // queue socket events for later processing
                  d->queuedSocketEvents.append(msg);
                  continue;
               }
            }
         }

         if (! haveMessage) {
            // no message - check for signalled objects
            for (int i = 0; i < (int)nCount; i++) {
               pHandles[i] = d->winEventNotifierList.at(i)->handle();
            }

            waitRet = MsgWaitForMultipleObjectsEx(nCount, pHandles, 0, QS_ALLINPUT, MWMO_ALERTABLE);

            if ((haveMessage = (waitRet == WAIT_OBJECT_0 + nCount))) {
               // a new message has arrived, process it
               continue;
            }
         }

         if (haveMessage) {
            if (! d->getMessageHook) {
               (void) qt_GetMessageHook(0, PM_REMOVE, (LPARAM) &msg);
            }

            if (d->internalHwnd == msg.hwnd && msg.message == WM_CS_SENDPOSTED_EVENTS) {
               if (seenWM_QT_SENDPOSTEDEVENTS) {
                  // when calling processEvents() "manually", we only want to send posted events once
                  needWM_QT_SENDPOSTEDEVENTS = true;
                  continue;
               }

               seenWM_QT_SENDPOSTEDEVENTS = true;

            } else if (msg.message == WM_TIMER) {
               // avoid live-lock by keeping track of the timers we have already sent
               bool found = false;

               for (int i = 0; ! found && i < processedTimers.count(); ++i) {
                  const MSG processed = processedTimers.constData()[i];
                  found = (processed.wParam == msg.wParam && processed.hwnd == msg.hwnd && processed.lParam == msg.lParam);
               }

               if (found) {
                  continue;
               }

               processedTimers.append(msg);

            } else if (msg.message == WM_QUIT) {
               if (QCoreApplication::instance()) {
                  QCoreApplication::instance()->quit();
               }

               return false;
            }

            if (! filterNativeEvent(QByteArray("windows_generic_MSG"), &msg, nullptr)) {
               TranslateMessage(&msg);
               DispatchMessage(&msg);
            }

         } else if (waitRet - WAIT_OBJECT_0 < nCount) {
            d->activateEventNotifier(d->winEventNotifierList.at(waitRet - WAIT_OBJECT_0));

         } else {
            // nothing todo so break
            break;
         }

         retVal = true;
      }

      // still nothing - wait for message or signalled objects
      canWait = (! retVal && ! d->interrupt && (flags & QEventLoop::WaitForMoreEvents));

      if (canWait) {
         DWORD nCount = d->winEventNotifierList.count();
         Q_ASSERT(nCount < MAXIMUM_WAIT_OBJECTS - 1);

         for (int i = 0; i < (int)nCount; i++) {
            pHandles[i] = d->winEventNotifierList.at(i)->handle();
         }

         emit aboutToBlock();
         waitRet = MsgWaitForMultipleObjectsEx(nCount, pHandles, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
         emit awake();

         if (waitRet - WAIT_OBJECT_0 < nCount) {
            d->activateEventNotifier(d->winEventNotifierList.at(waitRet - WAIT_OBJECT_0));
            retVal = true;
         }
      }

   } while (canWait);

   if (! seenWM_QT_SENDPOSTEDEVENTS && (flags & QEventLoop::EventLoopExec) == 0) {
      // when called "manually", always send posted events
      sendPostedEvents();
   }

   if (needWM_QT_SENDPOSTEDEVENTS) {
      PostMessage(d->internalHwnd, WM_CS_SENDPOSTED_EVENTS, 0, 0);
   }

   return retVal;
}

bool QEventDispatcherWin32::hasPendingEvents()
{
   MSG msg;
   return qGlobalPostedEventsCount() || PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE);
}

void QEventDispatcherWin32::registerSocketNotifier(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);
   int sockfd = notifier->socket();
   int type   = notifier->type();

#if defined(CS_SHOW_DEBUG_CORE)

   if (sockfd < 0) {
      qDebug("QEventDispatcherWin32::registerSocketNotifier() Internal error");
      return;

   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qDebug("QEventDispatcherWin32::registerSocketNotifier() Socket notifiers can not be enabled from another thread");
      return;
   }

#endif

   Q_D(QEventDispatcherWin32);

   QSNDict *sn_vec[3] = { &d->sn_read, &d->sn_write, &d->sn_except };
   QSNDict *dict = sn_vec[type];

   if (QCoreApplication::closingDown()) {
      // ### d->exitloop?
      return;   // after sn_cleanup, don't reinitialize.
   }

   if (dict->contains(sockfd)) {
      const char *t[] = { "Read", "Write", "Exception" };

      /* Variable "socket" below is a function pointer. */
      qWarning("QEventDispatcherWin32::registerSocketNotifier() Multiple socket notifiers for "
            "the same socket %d and type %s", sockfd, t[type]);
   }

   createInternalHwnd();
   QSockNot *sn = new QSockNot;
   sn->obj = notifier;
   sn->fd  = sockfd;
   dict->insert(sn->fd, sn);

   long event = 0;

   if (d->sn_read.contains(sockfd)) {
      event |= FD_READ | FD_CLOSE | FD_ACCEPT;
   }

   if (d->sn_write.contains(sockfd)) {
      event |= FD_WRITE | FD_CONNECT;
   }

   if (d->sn_except.contains(sockfd)) {
      event |= FD_OOB;
   }

   QSFDict::iterator it = d->active_fd.find(sockfd);

   if (it != d->active_fd.end()) {
      QSockFd &sd = it.value();

      if (sd.selected) {
         d->doWsaAsyncSelect(sockfd, 0);
         sd.selected = false;
      }

      sd.event |= event;

   } else {
      d->active_fd.insert(sockfd, QSockFd(event));
   }

   d->postActivateSocketNotifiers();
}

void QEventDispatcherWin32::unregisterSocketNotifier(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);

#if defined(CS_SHOW_DEBUG_CORE)
   int sockfd = notifier->socket();

   if (sockfd < 0) {
      qDebug("QEventDispatcherWin32::unregisterSocketNotifier() Internal error");
      return;

   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qDebug("QEventDispatcherWin32::unregisterSocketNotifier() Socket notifiers can not be disabled from another thread");
      return;
   }

#endif

   doUnregisterSocketNotifier(notifier);
}

void QEventDispatcherWin32::doUnregisterSocketNotifier(QSocketNotifier *notifier)
{
   Q_D(QEventDispatcherWin32);

   int type = notifier->type();
   int sockfd = notifier->socket();
   Q_ASSERT(sockfd >= 0);

   QSFDict::iterator it = d->active_fd.find(sockfd);

   if (it != d->active_fd.end()) {
      QSockFd &sd = it.value();

      if (sd.selected) {
         d->doWsaAsyncSelect(sockfd, 0);
      }

      const long event[3] = { FD_READ | FD_CLOSE | FD_ACCEPT, FD_WRITE | FD_CONNECT, FD_OOB };
      sd.event ^= event[type];

      if (sd.event == 0) {
         d->active_fd.erase(it);
      } else if (sd.selected) {
         sd.selected = false;
         d->postActivateSocketNotifiers();
      }

   }

   QSNDict *sn_vec[3] = { &d->sn_read, &d->sn_write, &d->sn_except };
   QSNDict *dict = sn_vec[type];
   QSockNot *sn  = dict->value(sockfd);

   if (! sn) {
      return;
   }

   dict->remove(sockfd);
   delete sn;
}

void QEventDispatcherWin32::registerTimer(int timerId, int interval, Qt::TimerType timerType, QObject *object)
{
#if defined(CS_SHOW_DEBUG_CORE)

   if (timerId < 1 || interval < 0 || !object) {
      qDebug("QEventDispatcherWin32::registerTimer() Invalid arguments");
      return;

   } else if (object->thread() != thread() || thread() != QThread::currentThread()) {
      qDebug("QEventDispatcherWin32::registerTimer() Timers can not be started from another thread");
      return;
   }

#endif

   Q_D(QEventDispatcherWin32);

   // exiting ... do not register new timers
   // (QCoreApplication::closingDown() is set too late to be used here)
   if (d->closingDown) {
      return;
   }

   WinTimerInfo *t = new WinTimerInfo;
   t->dispatcher   = this;
   t->timerId      = timerId;
   t->interval     = interval;
   t->timerType    = timerType;
   t->obj          = object;
   t->inTimerEvent = false;
   t->fastTimerId  = 0;

   if (d->internalHwnd) {
      d->registerTimer(t);
   }

   d->timerVec.append(t);                       // store in timer vector
   d->timerDict.insert(t->timerId, t);          // store timers in dict
}

bool QEventDispatcherWin32::unregisterTimer(int timerId)
{
#if defined(CS_SHOW_DEBUG_CORE)
   if (timerId < 1) {
      qDebug("QEventDispatcherWin32::unregisterTimer() Invalid argument");
      return false;
   }

   QThread *currentThread = QThread::currentThread();

   if (thread() != currentThread) {
      qDebug("QEventDispatcherWin32::unregisterTimer() Timers can not be stopped from another thread");
      return false;
   }
#endif

   Q_D(QEventDispatcherWin32);

   if (d->timerVec.isEmpty() || timerId <= 0) {
      return false;
   }

   WinTimerInfo *t = d->timerDict.value(timerId);

   if (! t) {
      return false;
   }

   d->timerDict.remove(t->timerId);
   d->timerVec.removeAll(t);
   d->unregisterTimer(t);

   return true;
}

bool QEventDispatcherWin32::unregisterTimers(QObject *object)
{
#if defined(CS_SHOW_DEBUG_CORE)
   if (! object) {
      qDebug("QEventDispatcherWin32::unregisterTimers() Invalid argument");
      return false;
   }

   QThread *currentThread = QThread::currentThread();

   if (object->thread() != thread() || thread() != currentThread) {
      qDebug("QEventDispatcherWin32::unregisterTimers() Timers can not be stopped from another thread");
      return false;
   }
#endif

   Q_D(QEventDispatcherWin32);

   if (d->timerVec.isEmpty()) {
      return false;
   }

   WinTimerInfo *t;

   for (int i = 0; i < d->timerVec.size(); i++) {
      t = d->timerVec.at(i);

      if (t && t->obj == object) {                // object found
         d->timerDict.remove(t->timerId);
         d->timerVec.removeAt(i);
         d->unregisterTimer(t);
         --i;
      }
   }

   return true;
}

QList<QTimerInfo> QEventDispatcherWin32::registeredTimers(QObject *object) const
{
   if (! object) {
      qWarning("QEventDispatcherWin32:registeredTimers() Invalid argument");
      return QList<QTimerInfo>();
   }

   Q_D(const QEventDispatcherWin32);
   QList<QTimerInfo> list;

   for (int i = 0; i < d->timerVec.size(); ++i) {
      const WinTimerInfo *t = d->timerVec.at(i);

      if (t && t->obj == object) {
         list << QTimerInfo(t->timerId, t->interval, t->timerType);
      }
   }

   return list;
}

bool QEventDispatcherWin32::registerEventNotifier(QWinEventNotifier *notifier)
{
   if (!notifier) {
      qWarning("QWinEventNotifier: Internal error");
      return false;

   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qWarning("QWinEventNotifier::registerEventNotifier() Event notifiers can not be enabled from another thread");
      return false;
   }

   Q_D(QEventDispatcherWin32);

   if (d->winEventNotifierList.contains(notifier)) {
      return true;
   }

   if (d->winEventNotifierList.count() >= MAXIMUM_WAIT_OBJECTS - 2) {
      qWarning("QEventDispatcherWin32::registerEventNotifier() Unable to have more than %d enabled at one time",
            MAXIMUM_WAIT_OBJECTS - 2);
      return false;
   }

   d->winEventNotifierList.append(notifier);

   return true;
}

void QEventDispatcherWin32::unregisterEventNotifier(QWinEventNotifier *notifier)
{
   if (! notifier) {
      qWarning("QEventDispatcherWin32::registerEventNotifier() Internal error");
      return;

   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qWarning("QEventDispatcherWin32::registerEventNotifier() Event notifiers can not be disabled from another thread");
      return;
   }

   Q_D(QEventDispatcherWin32);

   int i = d->winEventNotifierList.indexOf(notifier);

   if (i != -1) {
      d->winEventNotifierList.takeAt(i);
   }
}

void QEventDispatcherWin32::activateEventNotifiers()
{
   Q_D(QEventDispatcherWin32);

   // could break if events are removed/added in the activation
   for (int i = 0; i < d->winEventNotifierList.count(); i++) {
      if (WaitForSingleObjectEx(d->winEventNotifierList.at(i)->handle(), 0, TRUE) == WAIT_OBJECT_0) {
         d->activateEventNotifier(d->winEventNotifierList.at(i));
      }
   }
}

int QEventDispatcherWin32::remainingTime(int timerId)
{
#if defined(CS_SHOW_DEBUG_CORE)
   if (timerId < 1) {
      qDebug("QEventDispatcherWin32::remainingTime() Invalid argument");
      return -1;
   }
#endif

   Q_D(QEventDispatcherWin32);

   if (d->timerVec.isEmpty()) {
      return -1;
   }

   quint64 currentTime = qt_msectime();

   WinTimerInfo *t;

   for (int i = 0; i < d->timerVec.size(); i++) {
      t = d->timerVec.at(i);

      if (t && t->timerId == timerId) {                // timer found
         if (currentTime < t->timeout) {
            // time to wait
            return t->timeout - currentTime;
         } else {
            return 0;
         }
      }
   }

#if defined(CS_SHOW_DEBUG_CORE)
   qDebug("QEventDispatcherWin32::remainingTime() Timer id %d was not found", timerId);
#endif

   return -1;
}

void QEventDispatcherWin32::wakeUp()
{
   Q_D(QEventDispatcherWin32);
   d->serialNumber.ref();

   int expected = 0;

   if (d->internalHwnd && d->wakeUps.compareExchange(expected, 1, std::memory_order_acquire)) {
      // post a WM_CS_SENDPOSTED_EVENTS to this thread if there is not one already pending
      PostMessage(d->internalHwnd, WM_CS_SENDPOSTED_EVENTS, 0, 0);
   }
}

void QEventDispatcherWin32::interrupt()
{
   Q_D(QEventDispatcherWin32);
   d->interrupt = true;
   wakeUp();
}

void QEventDispatcherWin32::flush()
{ }

void QEventDispatcherWin32::startingUp()
{ }

void QEventDispatcherWin32::closingDown()
{
   Q_D(QEventDispatcherWin32);

   // clean up any socketnotifiers
   while (! d->sn_read.isEmpty()) {
      doUnregisterSocketNotifier((*(d->sn_read.begin()))->obj);
   }

   while (! d->sn_write.isEmpty()) {
      doUnregisterSocketNotifier((*(d->sn_write.begin()))->obj);
   }

   while (! d->sn_except.isEmpty()) {
      doUnregisterSocketNotifier((*(d->sn_except.begin()))->obj);
   }

   // clean up any timers
   for (int i = 0; i < d->timerVec.count(); ++i) {
      d->unregisterTimer(d->timerVec.at(i));
   }

   d->timerVec.clear();
   d->timerDict.clear();

   d->closingDown = true;

   uninstallMessageHook();
}

bool QEventDispatcherWin32::event(QEvent *e)
{
   Q_D(QEventDispatcherWin32);

   if (e->type() == QEvent::ZeroTimerEvent) {
      QZeroTimerEvent *zte = static_cast<QZeroTimerEvent *>(e);
      WinTimerInfo *t = d->timerDict.value(zte->timerId());

      if (t) {
         t->inTimerEvent = true;

         QTimerEvent te(zte->timerId());
         QCoreApplication::sendEvent(t->obj, &te);

         t = d->timerDict.value(zte->timerId());

         if (t) {
            if (t->interval == 0 && t->inTimerEvent) {
               // post the next zero timer event as long as the timer was not restarted
               QCoreApplication::postEvent(this, new QZeroTimerEvent(zte->timerId()));
            }

            t->inTimerEvent = false;
         }
      }

      return true;

   } else if (e->type() == QEvent::Timer) {
      QTimerEvent *te = static_cast<QTimerEvent *>(e);
      d->sendTimerEvent(te->timerId());
   }

   return QAbstractEventDispatcher::event(e);
}
void QEventDispatcherWin32::sendPostedEvents()
{
   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);
   QCoreApplicationPrivate::sendPostedEvents(nullptr, 0, threadData);
}
