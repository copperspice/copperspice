/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qwineventnotifier.h>
#include <qeventdispatcher_win_p.h>
#include <qcoreapplication.h>
#include <qthread_p.h>


/*
    \class QWinEventNotifier
    \brief The QWinEventNotifier class provides support for the Windows Wait functions.

    The QWinEventNotifier class makes it possible to use the wait
    functions on windows in a asynchronous manner. With this class
    you can register a HANDLE to an event and get notification when
    that event becomes signalled. The state of the event is not modified
    in the process so if it is a manual reset event you will need to
    reset it after the notification.
*/


QWinEventNotifier::QWinEventNotifier(QObject *parent)
   : QObject(parent), handleToEvent(0), enabled(false)
{}

QWinEventNotifier::QWinEventNotifier(HANDLE hEvent, QObject *parent)
   : QObject(parent), handleToEvent(hEvent), enabled(false)
{
   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   QEventDispatcherWin32 *eventDispatcher = qobject_cast<QEventDispatcherWin32 *>(threadData->eventDispatcher);

   Q_ASSERT_X(eventDispatcher, "QWinEventNotifier::QWinEventNotifier()",
              "Cannot create a win event notifier without a QEventDispatcherWin32");

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

   if (!eventDispatcher) { // perhaps application is shutting down
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
         QMetaObject::invokeMethod(this, "setEnabled", Qt::QueuedConnection,
                                   Q_ARG(bool, enabled));
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


