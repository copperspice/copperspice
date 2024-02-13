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

#ifndef QEVENTDISPATCHER_GLIB_P_H
#define QEVENTDISPATCHER_GLIB_P_H

#include <qabstracteventdispatcher.h>

#include <qhash.h>

#include <qabstracteventdispatcher_p.h>

class QEventDispatcherGlibPrivate;

using GMainContext = _GMainContext

struct GPostEventSource;
struct GSocketNotifierSource;
struct GTimerSource;
struct GIdleTimerSource;

class Q_CORE_EXPORT QEventDispatcherGlib : public QAbstractEventDispatcher
{
   CORE_CS_OBJECT(QEventDispatcherGlib)
   Q_DECLARE_PRIVATE(QEventDispatcherGlib)

 public:
   explicit QEventDispatcherGlib(QObject *parent = nullptr);
   explicit QEventDispatcherGlib(GMainContext *context, QObject *parent = nullptr);
   ~QEventDispatcherGlib();

   bool processEvents(QEventLoop::ProcessEventsFlags flags) override;
   bool hasPendingEvents() override;

   void registerSocketNotifier(QSocketNotifier *socketNotifier) override;
   void unregisterSocketNotifier(QSocketNotifier *socketNotifier) override;

   void registerTimer(int timerId, int interval, Qt::TimerType timerType, QObject *object) override;
   bool unregisterTimer(int timerId) override;
   bool unregisterTimers(QObject *object) override;
   QList<QTimerInfo> registeredTimers(QObject *object) const override;

   int remainingTime(int timerId) override;
   void wakeUp() override;
   void interrupt() override;
   void flush() override;

   static bool versionSupported();

 protected:
   QEventDispatcherGlib(QEventDispatcherGlibPrivate &dd, QObject *parent);
};

class Q_CORE_EXPORT QEventDispatcherGlibPrivate : public QAbstractEventDispatcherPrivate
{
 public:
   QEventDispatcherGlibPrivate(GMainContext *context = nullptr);
   GMainContext *mainContext;
   GPostEventSource *postEventSource;
   GSocketNotifierSource *socketNotifierSource;
   GTimerSource *timerSource;
   GIdleTimerSource *idleTimerSource;

   void runTimersOnceWithNormalPriority();
};

#endif
