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

#ifndef QEVENTDISPATCHER_WIN_P_H
#define QEVENTDISPATCHER_WIN_P_H

#include <qabstracteventdispatcher.h>

#include <qhash.h>
#include <qt_windows.h>

#include <qabstracteventdispatcher_p.h>

class QEventDispatcherWin32Private;
class QWinEventNotifier;

// forward declaration
LRESULT QT_WIN_CALLBACK qt_internal_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp);
quint64 qt_msectime();

class Q_CORE_EXPORT QEventDispatcherWin32 : public QAbstractEventDispatcher
{
   CORE_CS_OBJECT(QEventDispatcherWin32)
   Q_DECLARE_PRIVATE(QEventDispatcherWin32)

 public:
   explicit QEventDispatcherWin32(QObject *parent = nullptr);
   ~QEventDispatcherWin32();

   bool QT_ENSURE_STACK_ALIGNED_FOR_SSE processEvents(QEventLoop::ProcessEventsFlags flags) override;
   bool hasPendingEvents() override;

   void registerSocketNotifier(QSocketNotifier *notifier) override;
   void unregisterSocketNotifier(QSocketNotifier *notifier) override;

   void registerTimer(int timerId, int interval, Qt::TimerType timerType, QObject *object) override;
   bool unregisterTimer(int timerId) override;
   bool unregisterTimers(QObject *object) override;
   QList<QTimerInfo> registeredTimers(QObject *object) const override;

   bool registerEventNotifier(QWinEventNotifier *notifier) override;
   void unregisterEventNotifier(QWinEventNotifier *notifier) override;
   void activateEventNotifiers();

   int remainingTime(int timerId) override;
   void wakeUp() override;
   void interrupt() override;
   void flush() override;

   void startingUp() override;
   void closingDown() override;

   bool event(QEvent *e) override;

 protected:
   QEventDispatcherWin32(QEventDispatcherWin32Private &dd, QObject *parent = nullptr);
   virtual void sendPostedEvents();
   void doUnregisterSocketNotifier(QSocketNotifier *notifier);

   void createInternalHwnd();
   void installMessageHook();
   void uninstallMessageHook();

 private:
   friend LRESULT QT_WIN_CALLBACK qt_internal_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp);
   friend LRESULT QT_WIN_CALLBACK qt_GetMessageHook(int, WPARAM, LPARAM);
};

struct QSockNot {
   QSocketNotifier *obj;
   int fd;
};
using QSNDict = QHash<int, QSockNot *>;

struct QSockFd {
   long event;
   bool selected;

   explicit inline QSockFd(long ev = 0) : event(ev), selected(false) { }
};
using QSFDict = QHash<int, QSockFd>;

struct WinTimerInfo {                           // internal timer info
   QObject *dispatcher;
   int timerId;
   int interval;
   Qt::TimerType timerType;
   quint64 timeout;                            // when to actually fire
   QObject *obj;                               // object to receive events
   bool inTimerEvent;
   int fastTimerId;
};

class QZeroTimerEvent : public QTimerEvent
{
 public:
   explicit inline QZeroTimerEvent(int timerId)
      : QTimerEvent(timerId)
   {
      t = QEvent::ZeroTimerEvent;
   }
};

using WinTimerVec  = QList<WinTimerInfo *>  ;      // vector of TimerInfo structs
using WinTimerDict = QHash<int, WinTimerInfo *>;   // fast dict of timers

class Q_CORE_EXPORT QEventDispatcherWin32Private : public QAbstractEventDispatcherPrivate
{
   Q_DECLARE_PUBLIC(QEventDispatcherWin32)

 public:
   QEventDispatcherWin32Private();
   ~QEventDispatcherWin32Private();

   DWORD threadId;

   bool interrupt;
   bool closingDown;

   // internal window handle used for socketnotifiers/timers/etc
   HWND internalHwnd;
   HHOOK getMessageHook;

   // for controlling when to send posted events
   QAtomicInt serialNumber;
   int lastSerialNumber;
   int sendPostedEventsWindowsTimerId;
   QAtomicInt wakeUps;

   // timers
   WinTimerVec timerVec;
   WinTimerDict timerDict;
   void registerTimer(WinTimerInfo *t);
   void unregisterTimer(WinTimerInfo *t);
   void sendTimerEvent(int timerId);

   // socket notifiers
   QSNDict sn_read;
   QSNDict sn_write;
   QSNDict sn_except;
   QSFDict active_fd;

   bool activateNotifiersPosted;
   void postActivateSocketNotifiers();

   void doWsaAsyncSelect(int socket, long event);

   QList<QWinEventNotifier *> winEventNotifierList;
   void activateEventNotifier(QWinEventNotifier *wen);

   QList<MSG> queuedUserInputEvents;
   QList<MSG> queuedSocketEvents;
};

#endif // QEVENTDISPATCHER_WIN_P_H
