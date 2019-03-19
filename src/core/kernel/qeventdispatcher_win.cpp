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

#include <qeventdispatcher_win_p.h>

#include <qabstracteventdispatcher_p.h>
#include <qcoreapplication.h>
#include <qcoreapplication_p.h>
#include <qhash.h>
#include <qpair.h>
#include <qset.h>
#include <qstringparser.h>
#include <qsocketnotifier.h>
#include <qsystemlibrary_p.h>
#include <qthread_p.h>
#include <qvarlengtharray.h>
#include <qwineventnotifier.h>

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

enum {
   WM_QT_SOCKETNOTIFIER   = WM_USER,
   WM_QT_SENDPOSTEDEVENTS = WM_USER + 1,
   SendPostedEventsWindowsTimerId = ~1u
};

class QEventDispatcherWin32Private;

struct QSockNot {
   QSocketNotifier *obj;
   int fd;
};
typedef QHash<int, QSockNot *> QSNDict;

struct WinTimerInfo {                           // internal timer info
   QObject *dispatcher;
   int timerId;
   int interval;
   QObject *obj;                               // - object to receive events
   bool inTimerEvent;
   int fastTimerId;
};

class QZeroTimerEvent : public QTimerEvent
{
 public:
   inline QZeroTimerEvent(int timerId)
      : QTimerEvent(timerId) {
      t = QEvent::ZeroTimerEvent;
   }
};

typedef QList<WinTimerInfo *>  WinTimerVec;     // vector of TimerInfo structs
typedef QHash<int, WinTimerInfo *> WinTimerDict; // fast dict of timers

#if !defined(DWORD_PTR) && !defined(Q_OS_WIN64)
#define DWORD_PTR DWORD
#endif

typedef MMRESULT(WINAPI *ptimeSetEvent)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);
typedef MMRESULT(WINAPI *ptimeKillEvent)(UINT);

static ptimeSetEvent qtimeSetEvent = 0;
static ptimeKillEvent qtimeKillEvent = 0;

LRESULT QT_WIN_CALLBACK qt_internal_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp);

static void resolveTimerAPI()
{
   static bool triedResolve = false;
   if (!triedResolve) {

      QSystemLibrary library(QLatin1String("winmm"));

      if (library.load()) {
         qtimeSetEvent = (ptimeSetEvent)library.resolve("timeSetEvent");
         qtimeKillEvent = (ptimeKillEvent)library.resolve("timeKillEvent");
      }

      triedResolve = true;
   }
}


class QEventDispatcherWin32Private : public QAbstractEventDispatcherPrivate
{
   Q_DECLARE_PUBLIC(QEventDispatcherWin32)

 public:
   QEventDispatcherWin32Private();
   ~QEventDispatcherWin32Private();

   DWORD threadId;

   bool interrupt;

   // internal window handle used for socketnotifiers/timers/etc
   HWND internalHwnd;
   HHOOK getMessageHook;

   // for controlling when to send posted events
   QAtomicInt serialNumber;
   int lastSerialNumber, sendPostedEventsWindowsTimerId;
   QAtomicInt wakeUps;

   // timers
   WinTimerVec timerVec;
   WinTimerDict timerDict;
   void registerTimer(WinTimerInfo *t);
   void unregisterTimer(WinTimerInfo *t, bool closingDown = false);
   void sendTimerEvent(int timerId);

   // socket notifiers
   QSNDict sn_read;
   QSNDict sn_write;
   QSNDict sn_except;
   void doWsaAsyncSelect(int socket);

   QList<QWinEventNotifier *> winEventNotifierList;
   void activateEventNotifier(QWinEventNotifier *wen);

   QList<MSG> queuedUserInputEvents;
   QList<MSG> queuedSocketEvents;

   QThreadData *get_m_ThreadData();
};

QEventDispatcherWin32Private::QEventDispatcherWin32Private()
   : threadId(GetCurrentThreadId()), interrupt(false), internalHwnd(0), getMessageHook(0),
     serialNumber(0), lastSerialNumber(0), sendPostedEventsWindowsTimerId(0), wakeUps(0)
{
   resolveTimerAPI();
}

QEventDispatcherWin32Private::~QEventDispatcherWin32Private()
{
   if (internalHwnd) {
      DestroyWindow(internalHwnd);
   }

   QString className = "QEventDispatcherWin32_Internal_Widget" + QString::number(quintptr(qt_internal_proc));
   UnregisterClass(&className.toStdWString()[0], qWinAppInst());
}

void QEventDispatcherWin32Private::activateEventNotifier(QWinEventNotifier *wen)
{
   QEvent event(QEvent::WinEventAct);
   QCoreApplication::sendEvent(wen, &event);
}

QThreadData *QEventDispatcherWin32Private::get_m_ThreadData()
{
   Q_Q(QEventDispatcherWin32);

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(q);
   return threadData;
}

// This function is called by a workerthread
void WINAPI QT_WIN_CALLBACK qt_fast_timer_proc(uint timerId, uint, DWORD_PTR user, DWORD_PTR, DWORD_PTR)
{
   if (!timerId) {
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

   QCoreApplication *app = QCoreApplication::instance();
   long result;

   if (! app) {
      if (message == WM_TIMER) {
         KillTimer(hwnd, wp);
      }
      return 0;

   } else if (app->filterEvent(&msg, &result)) {
      return result;
   }

#ifdef GWLP_USERDATA
   QEventDispatcherWin32 *q = (QEventDispatcherWin32 *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
#else
   QEventDispatcherWin32 *q = (QEventDispatcherWin32 *) GetWindowLong(hwnd, GWL_USERDATA);
#endif

   QEventDispatcherWin32Private *d = 0;
   if (q != 0) {
      d = q->d_func();
   }

   if (message == WM_QT_SOCKETNOTIFIER) {
      // socket notifier message
      int type = -1;

      switch (WSAGETSELECTEVENT(lp)) {
         case FD_READ:
         case FD_CLOSE:
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
      }

      if (type >= 0) {
         Q_ASSERT(d != 0);
         QSNDict *sn_vec[3] = { &d->sn_read, &d->sn_write, &d->sn_except };
         QSNDict *dict = sn_vec[type];

         QSockNot *sn = dict ? dict->value(wp) : 0;
         if (sn) {
            QEvent event(QEvent::SockAct);
            QCoreApplication::sendEvent(sn->obj, &event);
         }
      }
      return 0;

   } else if (message == WM_QT_SENDPOSTEDEVENTS
                  || (message == WM_TIMER && d->sendPostedEventsWindowsTimerId != 0 &&
                  wp == (uint)d->sendPostedEventsWindowsTimerId)) {

      // we also use a Windows timer to send posted events when the message queue is full
      int localSerialNumber = d->serialNumber.load();

      // calling a method in the private class 12/2013
      QThreadData *threadData = d->get_m_ThreadData();

      if (localSerialNumber != d->lastSerialNumber) {
         d->lastSerialNumber = localSerialNumber;
         QCoreApplicationPrivate::sendPostedEvents(0, 0, threadData);
      }

      return 0;

   } else if (message == WM_TIMER) {
      Q_ASSERT(d != 0);
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
   if (wp == PM_REMOVE) {
      QEventDispatcherWin32 *q = qobject_cast<QEventDispatcherWin32 *>(QAbstractEventDispatcher::instance());
      Q_ASSERT(q != 0);

      if (q) {
         MSG *msg = (MSG *) lp;
         QEventDispatcherWin32Private *d = q->d_func();
         int localSerialNumber = d->serialNumber.load();
         static const UINT mask = inputTimerMask();

         if (HIWORD(GetQueueStatus(mask)) == 0) {
            // no more input or timer events in the message queue, we can allow posted events to be sent normally now
            if (d->sendPostedEventsWindowsTimerId != 0) {
               // stop the timer to send posted events, since we now allow the WM_QT_SENDPOSTEDEVENTS message
               KillTimer(d->internalHwnd, d->sendPostedEventsWindowsTimerId);
               d->sendPostedEventsWindowsTimerId = 0;
            }

            (void) d->wakeUps.fetchAndStoreRelease(0);
            if (localSerialNumber != d->lastSerialNumber
                  // if this message IS the one that triggers sendPostedEvents(), no need to post it again
                  && (msg->hwnd != d->internalHwnd || msg->message != WM_QT_SENDPOSTEDEVENTS)) {

               PostMessage(d->internalHwnd, WM_QT_SENDPOSTEDEVENTS, 0, 0);
            }

         } else if (d->sendPostedEventsWindowsTimerId == 0 && localSerialNumber != d->lastSerialNumber) {

            // start a special timer to continue delivering posted events while
            // there are still input and timer messages in the message queue

            d->sendPostedEventsWindowsTimerId = SetTimer(d->internalHwnd, SendPostedEventsWindowsTimerId, 0, NULL);

            // we specify zero, but Windows uses USER_TIMER_MINIMUM
            // we don't check the return value of SetTimer()... if creating the timer failed, there's little
            // we can do. we just have to accept that posted events will be starved

         }
      }
   }

   return CallNextHookEx(0, code, wp, lp);

}

static HWND qt_create_internal_window(const QEventDispatcherWin32 *eventDispatcher)
{
   // make sure that multiple instances can coexist in the same process
   QString className = "QEventDispatcherWin32_Internal_Widget" + QString::number(quintptr(qt_internal_proc));

   WNDCLASS wc;
   wc.style         = 0;
   wc.lpfnWndProc   = qt_internal_proc;
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = 0;
   wc.hInstance     = qWinAppInst();
   wc.hIcon         = 0;
   wc.hCursor       = 0;
   wc.hbrBackground = 0;
   wc.lpszMenuName  = NULL;

   std::wstring tmp = className.toStdWString();
   wc.lpszClassName = &tmp[0];

   RegisterClass(&wc);
   HWND wnd = CreateWindow(wc.lpszClassName,  // classname
                           wc.lpszClassName,  // window name
                           0,                 // style
                           0, 0, 0, 0,        // geometry
                           0,                 // parent
                           0,                 // menu handle
                           qWinAppInst(),     // application
                           0);                // windows creation data.

   if (! wnd) {
      qWarning("QEventDispatcher: Failed to create QEventDispatcherWin32 internal window: %d\n", (int)GetLastError());
   }

#ifdef GWLP_USERDATA
   SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)eventDispatcher);
#else
   SetWindowLong(wnd, GWL_USERDATA, (LONG)eventDispatcher);
#endif

   return wnd;
}

void QEventDispatcherWin32Private::registerTimer(WinTimerInfo *t)
{
   Q_ASSERT(internalHwnd);

   Q_Q(QEventDispatcherWin32);

   int ok = 0;
   if (t->interval > 20 || ! t->interval || ! qtimeSetEvent) {
      ok = 1;
      if (!t->interval) {
         // optimization for single-shot-zero-timer
         QCoreApplication::postEvent(q, new QZeroTimerEvent(t->timerId));

      } else {
         ok = SetTimer(internalHwnd, t->timerId, (uint) t->interval, 0);
      }

   } else {
      ok = t->fastTimerId = qtimeSetEvent(t->interval, 1, qt_fast_timer_proc, (DWORD_PTR)t,
               TIME_CALLBACK_FUNCTION | TIME_PERIODIC | TIME_KILL_SYNCHRONOUS);

      if (ok == 0) {
         // fall back to normal timer if no more multimedia timers available
         ok = SetTimer(internalHwnd, t->timerId, (uint) t->interval, 0);
      }
   }

   if (ok == 0) {
      qErrnoWarning("QEventDispatcherWin32::registerTimer: Failed to create a timer");
   }
}

void QEventDispatcherWin32Private::unregisterTimer(WinTimerInfo *t, bool closingDown)
{
   // mark timer as unused
   std::atomic<bool> &inThreadChangeEvent = CSInternalEvents::get_m_inThreadChangeEvent(t->obj);

   if (! inThreadChangeEvent.load() && ! closingDown) {
      QAbstractEventDispatcherPrivate::releaseTimerId(t->timerId);
   }

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

      QTimerEvent e(t->timerId);
      QCoreApplication::sendEvent(t->obj, &e);

      // timer could have been removed
      t = timerDict.value(timerId);
      if (t) {
         t->inTimerEvent = false;
      }
   }
}

void QEventDispatcherWin32Private::doWsaAsyncSelect(int socket)
{
   Q_ASSERT(internalHwnd);
   int sn_event = 0;

   if (sn_read.contains(socket)) {
      sn_event |= FD_READ | FD_CLOSE | FD_ACCEPT;
   }

   if (sn_write.contains(socket)) {
      sn_event |= FD_WRITE | FD_CONNECT;
   }

   if (sn_except.contains(socket)) {
      sn_event |= FD_OOB;
   }

   // BoundsChecker may emit a warning for WSAAsyncSelect when sn_event == 0
   // This is a BoundsChecker bug and not a Qt bug
   WSAAsyncSelect(socket, internalHwnd, sn_event ? unsigned(WM_QT_SOCKETNOTIFIER) : unsigned(0), sn_event);
}

void QEventDispatcherWin32::createInternalHwnd()
{
   Q_D(QEventDispatcherWin32);

   Q_ASSERT(!d->internalHwnd);
   if (d->internalHwnd) {
      return;
   }
   d->internalHwnd = qt_create_internal_window(this);

   // setup GetMessage hook needed to drive our posted events
   d->getMessageHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC) qt_GetMessageHook, NULL, GetCurrentThreadId());

   if (!d->getMessageHook) {
      qFatal("Qt: INTERNALL ERROR: failed to install GetMessage hook");
   }

   // register all socket notifiers
   QList<int> sockets = (d->sn_read.keys().toSet() + d->sn_write.keys().toSet() + d->sn_except.keys().toSet()).toList();

   for (int i = 0; i < sockets.count(); ++i) {
      d->doWsaAsyncSelect(sockets.at(i));
   }

   // start all normal timers
   for (int i = 0; i < d->timerVec.count(); ++i) {
      d->registerTimer(d->timerVec.at(i));
   }

   // trigger a call to sendPostedEvents()
   wakeUp();
}

QEventDispatcherWin32::QEventDispatcherWin32(QObject *parent)
   : QAbstractEventDispatcher(*new QEventDispatcherWin32Private, parent)
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
            haveMessage = PeekMessage(&msg, 0, 0, 0, PM_REMOVE);

            if (haveMessage && (flags & QEventLoop::ExcludeUserInputEvents)
                  && ((msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
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
               haveMessage = false;
               d->queuedUserInputEvents.append(msg);
            }

            if (haveMessage && (flags & QEventLoop::ExcludeSocketNotifiers)
                  && (msg.message == WM_QT_SOCKETNOTIFIER && msg.hwnd == d->internalHwnd)) {
               // queue socket events for later processing
               haveMessage = false;
               d->queuedSocketEvents.append(msg);
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

            if (d->internalHwnd == msg.hwnd && msg.message == WM_QT_SENDPOSTEDEVENTS) {
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

            if (! filterEvent(&msg)) {
               TranslateMessage(&msg);
               DispatchMessage(&msg);
            }

         } else if (waitRet < WAIT_OBJECT_0 + nCount) {
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

         if (waitRet < WAIT_OBJECT_0 + nCount) {
            d->activateEventNotifier(d->winEventNotifierList.at(waitRet - WAIT_OBJECT_0));
            retVal = true;
         }
      }

   } while (canWait);

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   if (! seenWM_QT_SENDPOSTEDEVENTS && (flags & QEventLoop::EventLoopExec) == 0) {
      // when called "manually", always send posted events
      QCoreApplicationPrivate::sendPostedEvents(0, 0, threadData);
   }

   if (needWM_QT_SENDPOSTEDEVENTS) {
      PostMessage(d->internalHwnd, WM_QT_SENDPOSTEDEVENTS, 0, 0);
   }

   return retVal;
}

bool QEventDispatcherWin32::hasPendingEvents()
{
   MSG msg;
   return qGlobalPostedEventsCount() || PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
}

void QEventDispatcherWin32::registerSocketNotifier(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);
   int sockfd = notifier->socket();
   int type = notifier->type();

#ifndef QT_NO_DEBUG
   if (sockfd < 0) {
      qWarning("QSocketNotifier: Internal error");

      return;
   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qWarning("QSocketNotifier: socket notifiers cannot be enabled from another thread");
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
      qWarning("QSocketNotifier: Multiple socket notifiers for "
               "same socket %d and type %s", sockfd, t[type]);
   }

   QSockNot *sn = new QSockNot;
   sn->obj = notifier;
   sn->fd  = sockfd;
   dict->insert(sn->fd, sn);

   if (d->internalHwnd) {
      d->doWsaAsyncSelect(sockfd);
   }
}

void QEventDispatcherWin32::unregisterSocketNotifier(QSocketNotifier *notifier)
{
   Q_ASSERT(notifier);
   int sockfd = notifier->socket();
   int type = notifier->type();

#ifndef QT_NO_DEBUG
   if (sockfd < 0) {
      qWarning("QSocketNotifier: Internal error");
      return;
   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qWarning("QSocketNotifier: socket notifiers cannot be disabled from another thread");
      return;
   }
#endif

   Q_D(QEventDispatcherWin32);
   QSNDict *sn_vec[3] = { &d->sn_read, &d->sn_write, &d->sn_except };
   QSNDict *dict = sn_vec[type];
   QSockNot *sn  = dict->value(sockfd);

   if (!sn) {
      return;
   }

   dict->remove(sockfd);
   delete sn;

   if (d->internalHwnd) {
      d->doWsaAsyncSelect(sockfd);
   }
}

void QEventDispatcherWin32::registerTimer(int timerId, int interval, QObject *object)
{
   if (timerId < 1 || interval < 0 || !object) {
      qWarning("QEventDispatcherWin32::registerTimer: invalid arguments");
      return;

   } else if (object->thread() != thread() || thread() != QThread::currentThread()) {
      qWarning("QObject::startTimer: timers cannot be started from another thread");
      return;
   }

   Q_D(QEventDispatcherWin32);

   WinTimerInfo *t = new WinTimerInfo;
   t->dispatcher   = this;
   t->timerId      = timerId;
   t->interval     = interval;
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
   if (timerId < 1) {
      qWarning("QEventDispatcherWin32::unregisterTimer: invalid argument");
      return false;
   }

   QThread *currentThread = QThread::currentThread();

   if (thread() != currentThread) {
      qWarning("QObject::killTimer: timers cannot be stopped from another thread");
      return false;
   }

   Q_D(QEventDispatcherWin32);
   if (d->timerVec.isEmpty() || timerId <= 0) {
      return false;
   }

   WinTimerInfo *t = d->timerDict.value(timerId);
   if (!t) {
      return false;
   }

   d->timerDict.remove(t->timerId);
   d->timerVec.removeAll(t);
   d->unregisterTimer(t);

   return true;
}

bool QEventDispatcherWin32::unregisterTimers(QObject *object)
{
   if (! object) {
      qWarning("QEventDispatcherWin32::unregisterTimers: invalid argument");
      return false;
   }

   QThread *currentThread = QThread::currentThread();

   if (object->thread() != thread() || thread() != currentThread) {
      qWarning("QObject::killTimers: timers can not be stopped from another thread");
      return false;
   }

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

QList<QEventDispatcherWin32::TimerInfo>
QEventDispatcherWin32::registeredTimers(QObject *object) const
{
   if (!object) {
      qWarning("QEventDispatcherWin32:registeredTimers: invalid argument");
      return QList<TimerInfo>();
   }

   Q_D(const QEventDispatcherWin32);
   QList<TimerInfo> list;

   for (int i = 0; i < d->timerVec.size(); ++i) {
      const WinTimerInfo *t = d->timerVec.at(i);
      if (t && t->obj == object) {
         list << TimerInfo(t->timerId, t->interval);
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
      qWarning("QWinEventNotifier: event notifiers cannot be enabled from another thread");
      return false;
   }

   Q_D(QEventDispatcherWin32);

   if (d->winEventNotifierList.contains(notifier)) {
      return true;
   }

   if (d->winEventNotifierList.count() >= MAXIMUM_WAIT_OBJECTS - 2) {
      qWarning("QWinEventNotifier: Cannot have more than %d enabled at one time", MAXIMUM_WAIT_OBJECTS - 2);
      return false;
   }
   d->winEventNotifierList.append(notifier);
   return true;
}

void QEventDispatcherWin32::unregisterEventNotifier(QWinEventNotifier *notifier)
{
   if (!notifier) {
      qWarning("QWinEventNotifier: Internal error");
      return;

   } else if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
      qWarning("QWinEventNotifier: Event notifiers can not be disabled from another thread");
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

   //### this could break if events are removed/added in the activation
   for (int i = 0; i < d->winEventNotifierList.count(); i++) {
      if (WaitForSingleObjectEx(d->winEventNotifierList.at(i)->handle(), 0, TRUE) == WAIT_OBJECT_0) {
         d->activateEventNotifier(d->winEventNotifierList.at(i));
      }
   }
}

void QEventDispatcherWin32::wakeUp()
{
   Q_D(QEventDispatcherWin32);
   d->serialNumber.ref();

   if (d->internalHwnd && d->wakeUps.testAndSetAcquire(0, 1)) {
      // post a WM_QT_SENDPOSTEDEVENTS to this thread if there isn't one already pending
      PostMessage(d->internalHwnd, WM_QT_SENDPOSTEDEVENTS, 0, 0);
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
   while (!d->sn_read.isEmpty()) {
      unregisterSocketNotifier((*(d->sn_read.begin()))->obj);
   }
   while (!d->sn_write.isEmpty()) {
      unregisterSocketNotifier((*(d->sn_write.begin()))->obj);
   }
   while (!d->sn_except.isEmpty()) {
      unregisterSocketNotifier((*(d->sn_except.begin()))->obj);
   }

   // clean up any timers
   for (int i = 0; i < d->timerVec.count(); ++i) {
      d->unregisterTimer(d->timerVec.at(i), true);
   }
   d->timerVec.clear();
   d->timerDict.clear();

   if (d->getMessageHook) {
      UnhookWindowsHookEx(d->getMessageHook);
   }
   d->getMessageHook = 0;

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
