/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qbasictimer.h>
#include <qcoreapplication.h>
#include <qabstracteventdispatcher_p.h>

void QBasicTimer::start(int msec, QObject *obj)
{
   QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance();

   if (! eventDispatcher) {
     qWarning("QBasicTimer::start: QBasicTimer can only be used with threads started with QThread");
     return;
   }

   if (obj && obj->thread() != eventDispatcher->thread()) {
     qWarning("QBasicTimer::start: Timers cannot be started from another thread");
     return;
   }

   if (id) {
      if (eventDispatcher->unregisterTimer(id)) {
         QAbstractEventDispatcherPrivate::releaseTimerId(id);
      } else {
         qWarning("QBasicTimer::start: Stopping previous timer failed. Possibly trying to stop from a different thread");
      }
   }

   id = 0;

   if (obj) {
      id = eventDispatcher->registerTimer(msec, Qt::CoarseTimer, obj);
   }
}

void QBasicTimer::start(int msec, Qt::TimerType timerType, QObject *obj)
{
   QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance();

   if (msec < 0) {
      qWarning("QBasicTimer::start: Timers cannot have negative timeouts");
      return;
   }

   if (! eventDispatcher) {
      qWarning("QBasicTimer::start: QBasicTimer can only be used with threads started with QThread");
      return;
   }

   if (obj && obj->thread() != eventDispatcher->thread()) {
     qWarning("QBasicTimer::start: Timers cannot be started from another thread");
     return;
   }

   if (id) {
      if (eventDispatcher->unregisterTimer(id)) {
         QAbstractEventDispatcherPrivate::releaseTimerId(id);
      } else {
         qWarning("QBasicTimer::start: Stopping previous timer failed. Possibly trying to stop from a different thread");
      }
   }

   id = 0;

   if (obj) {
      id = eventDispatcher->registerTimer(msec, timerType, obj);
   }
}

void QBasicTimer::stop()
{
   if (id) {
      QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance();

      if (eventDispatcher) {
         if (! eventDispatcher->unregisterTimer(id)) {
            qWarning("QBasicTimer::stop: Failed. Possibly trying to stop from a different thread");
            return;
         }

         QAbstractEventDispatcherPrivate::releaseTimerId(id);
      }
   }

   id = 0;
}


