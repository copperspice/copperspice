/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QEVENTDISPATCHER_WIN_P_H
#define QEVENTDISPATCHER_WIN_P_H

#include <QtCore/qabstracteventdispatcher.h>
#include <QtCore/qt_windows.h>

QT_BEGIN_NAMESPACE

class QWinEventNotifier;
class QEventDispatcherWin32Private;

// forward declaration
LRESULT QT_WIN_CALLBACK qt_internal_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp);

class Q_CORE_EXPORT QEventDispatcherWin32 : public QAbstractEventDispatcher
{
   CS_OBJECT(QEventDispatcherWin32)
   Q_DECLARE_PRIVATE(QEventDispatcherWin32)

   void createInternalHwnd();
   friend class QGuiEventDispatcherWin32;

 public:
   explicit QEventDispatcherWin32(QObject *parent = 0);
   ~QEventDispatcherWin32();

   bool QT_ENSURE_STACK_ALIGNED_FOR_SSE processEvents(QEventLoop::ProcessEventsFlags flags);
   bool hasPendingEvents();

   void registerSocketNotifier(QSocketNotifier *notifier);
   void unregisterSocketNotifier(QSocketNotifier *notifier);

   void registerTimer(int timerId, int interval, QObject *object);
   bool unregisterTimer(int timerId);
   bool unregisterTimers(QObject *object);
   QList<TimerInfo> registeredTimers(QObject *object) const;

   bool registerEventNotifier(QWinEventNotifier *notifier);
   void unregisterEventNotifier(QWinEventNotifier *notifier);
   void activateEventNotifiers();

   void wakeUp();
   void interrupt();
   void flush();

   void startingUp();
   void closingDown();

   bool event(QEvent *e);

 private:
   friend LRESULT QT_WIN_CALLBACK qt_internal_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp);
   friend LRESULT QT_WIN_CALLBACK qt_GetMessageHook(int, WPARAM, LPARAM);
};

QT_END_NAMESPACE

#endif // QEVENTDISPATCHER_WIN_P_H
