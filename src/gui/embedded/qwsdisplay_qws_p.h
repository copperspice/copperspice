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

#ifndef QWSDISPLAY_QWS_P_H
#define QWSDISPLAY_QWS_P_H

#include <qwsdisplay_qws.h>
#include <qwssocket_qws.h>
#include <qwsevent_qws.h>
#include <qwssharedmemory_p.h>
#include <qwscommand_qws_p.h>
#include <qwslock_p.h>

QT_BEGIN_NAMESPACE

class QWSDisplay::Data
{
 public:
   Data(QObject *parent, bool singleProcess = false);
   ~Data();

   void flush();

   bool queueNotEmpty();
   QWSEvent *dequeue();
   QWSEvent *peek();

   bool directServerConnection();
   void fillQueue();

#ifndef QT_NO_QWS_MULTIPROCESS
   void connectToPipe();
   void waitForConnection();
   void waitForPropertyReply();
   void waitForRegionAck(int winId);
   void waitForRegionEvents(int winId, bool ungrabDisplay);
   bool hasPendingRegionEvents() const;
#endif

   void waitForCreation();

#ifndef QT_NO_COP
   void waitForQCopResponse();
#endif

   void init();
   void reinit( const QString &newAppName );
   void create(int n = 1);

   void flushCommands();
   void sendCommand(QWSCommand &cmd);
   void sendSynchronousCommand(QWSCommand &cmd);

   QWSEvent *readMore();

   int takeId();

   void setMouseFilter(void (*filter)(QWSMouseEvent *));

   //####public data members

   //    QWSRegionManager *rgnMan;
   uchar *sharedRam;

#ifndef QT_NO_QWS_MULTIPROCESS
   QWSSharedMemory shm;
#endif

   int sharedRamSize;

#ifndef QT_NO_QWS_MULTIPROCESS
   static QWSLock *clientLock;

   static bool lockClient(QWSLock::LockType, int timeout = -1);
   static void unlockClient(QWSLock::LockType);
   static bool waitClient(QWSLock::LockType, int timeout = -1);
   static QWSLock *getClientLock();
#endif

 private:

#ifndef QT_NO_QWS_MULTIPROCESS
   QWSSocket *csocket;
#endif
   QList<QWSEvent *> queue;

   QWSConnectedEvent *connected_event;
   QWSMouseEvent *mouse_event;
   int region_events_count;
   int mouse_state;
   int mouse_winid;
   QPoint region_offset;
   int region_offset_window;

#ifndef QT_NO_COP
   QWSQCopMessageEvent *qcop_response;
#endif

   QWSEvent *current_event;
   QList<int> unused_identifiers;

#ifdef QAPPLICATION_EXTRA_DEBUG
   int mouse_event_count;
#endif

   void (*mouseFilter)(QWSMouseEvent *);

   enum { VariableEvent = -1 };

};

QT_END_NAMESPACE

#endif // QWSDISPLAY_QWS_P_H
