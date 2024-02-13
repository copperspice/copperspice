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

#include <qwineventnotifier.h>

#include <qcoreapplication.h>

#include <qeventdispatcher_win_p.h>
#include <qthread_p.h>

QWinEventNotifier::QWinEventNotifier(QObject *parent)
   : QObject(parent), handleToEvent(nullptr), enabled(false)
{
}

QWinEventNotifier::QWinEventNotifier(HANDLE hEvent, QObject *parent)
   : QObject(parent), handleToEvent(hEvent), enabled(false)
{
   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   QEventDispatcherWin32 *eventDispatcher = qobject_cast<QEventDispatcherWin32 *>(threadData->eventDispatcher);

   Q_ASSERT_X(eventDispatcher, "QWinEventNotifier::QWinEventNotifier()",
         "Unable to create a win event notifier without a QEventDispatcherWin32");

   eventDispatcher->registerEventNotifier(this);
   enabled = true;
}

QWinEventNotifier::~QWinEventNotifier()
{
   setEnabled(false);
}

void QWinEventNotifier::setHandle(HANDLE hEvent)
{
   setEnabled(false);
   handleToEvent = hEvent;
}

HANDLE  QWinEventNotifier::handle() const
{
   return handleToEvent;
}

bool QWinEventNotifier::isEnabled() const
{
   return enabled;
}

void QWinEventNotifier::setEnabled(bool enable)
{
   if (enabled == enable) {                      // no change
      return;
   }

   enabled = enable;

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);
   QEventDispatcherWin32 *eventDispatcher = qobject_cast<QEventDispatcherWin32 *>(threadData->eventDispatcher);

   if (! eventDispatcher) {
      // perhaps application is shutting down
      return;
   }

   if (enabled) {
      eventDispatcher->registerEventNotifier(this);
   } else {
      eventDispatcher->unregisterEventNotifier(this);
   }
}

bool QWinEventNotifier::event(QEvent *e)
{
   if (e->type() == QEvent::ThreadChange) {
      if (enabled) {
         QMetaObject::invokeMethod(this, "setEnabled", Qt::QueuedConnection, Q_ARG(bool, enabled));
         setEnabled(false);
      }
   }

   QObject::event(e);                        // will activate filters

   if (e->type() == QEvent::WinEventAct) {
      emit activated(handleToEvent);
      return true;
   }

   return false;
}
