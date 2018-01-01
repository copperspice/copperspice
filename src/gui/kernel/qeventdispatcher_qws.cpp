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

#include <qplatformdefs.h>
#include <qapplication.h>
#include <qwscommand_qws_p.h>
#include <qwsdisplay_qws.h>
#include <qwsevent_qws.h>
#include <qwindowsystem_qws.h>
#include <qeventdispatcher_qws_p.h>
#include <qeventdispatcher_unix_p.h>
#include <qmutex.h>
#include <errno.h>

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE

class QEventDispatcherQWSPrivate : public QEventDispatcherUNIXPrivate
{
   Q_DECLARE_PUBLIC(QEventDispatcherQWS)

 public:
   inline QEventDispatcherQWSPrivate() {
   }
   QList<QWSEvent *> queuedUserInputEvents;
};


QEventDispatcherQWS::QEventDispatcherQWS(QObject *parent)
   : QEventDispatcherUNIX(*new QEventDispatcherQWSPrivate, parent)
{ }

QEventDispatcherQWS::~QEventDispatcherQWS()
{ }



// from qapplication_qws.cpp
extern QWSDisplay *qt_fbdpy; // QWS `display'

//#define ZERO_FOR_THE_MOMENT

bool QEventDispatcherQWS::processEvents(QEventLoop::ProcessEventsFlags flags)
{
   Q_D(QEventDispatcherQWS);
   // process events from the QWS server
   int           nevents = 0;

   // handle gui and posted events
   d->interrupt = false;
   QApplication::sendPostedEvents();

   while (!d->interrupt) {        // also flushes output buffer ###can be optimized
      QWSEvent *event;
      if (!(flags & QEventLoop::ExcludeUserInputEvents)
            && !d->queuedUserInputEvents.isEmpty()) {
         // process a pending user input event
         event = d->queuedUserInputEvents.takeFirst();
      } else if  (qt_fbdpy->eventPending()) {
         event = qt_fbdpy->getEvent();        // get next event
         if (flags & QEventLoop::ExcludeUserInputEvents) {
            // queue user input events
            if (event->type == QWSEvent::Mouse || event->type == QWSEvent::Key) {
               d->queuedUserInputEvents.append(event);
               continue;
            }
         }
      } else {
         break;
      }

      if (filterEvent(event)) {
         delete event;
         continue;
      }
      nevents++;

      bool ret = qApp->qwsProcessEvent(event) == 1;
      delete event;
      if (ret) {
         return true;
      }
   }

   if (!d->interrupt) {
      extern QList<QWSCommand *> *qt_get_server_queue();
      if (!qt_get_server_queue()->isEmpty()) {
         QWSServer::processEventQueue();
      }

      if (QEventDispatcherUNIX::processEvents(flags)) {
         return true;
      }
   }
   return (nevents > 0);
}

bool QEventDispatcherQWS::hasPendingEvents()
{
   extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
   return qGlobalPostedEventsCount() || qt_fbdpy->eventPending();
}

void QEventDispatcherQWS::startingUp()
{

}

void QEventDispatcherQWS::closingDown()
{

}

void QEventDispatcherQWS::flush()
{
   if (qApp) {
      qApp->sendPostedEvents();
   }
   (void)qt_fbdpy->eventPending(); // flush
}


int QEventDispatcherQWS::select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                                timeval *timeout)
{
   return QEventDispatcherUNIX::select(nfds, readfds, writefds, exceptfds, timeout);
}

QT_END_NAMESPACE
