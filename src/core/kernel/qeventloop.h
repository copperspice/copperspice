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

// do not move include, if qeventloop.h is included directly forward declarations are not sufficient 12/30/2013
#include <qobject.h>

#ifndef QEVENTLOOP_H
#define QEVENTLOOP_H

#include <qscopedpointer.h>

class QEventLoopPrivate;

class Q_CORE_EXPORT QEventLoop : public QObject
{
   CORE_CS_OBJECT(QEventLoop)
   Q_DECLARE_PRIVATE(QEventLoop)

 public:
   explicit QEventLoop(QObject *parent = nullptr);
   ~QEventLoop();

   enum ProcessEventsFlag {
      AllEvents              = 0x00,
      ExcludeUserInputEvents = 0x01,
      ExcludeSocketNotifiers = 0x02,
      WaitForMoreEvents      = 0x04,
      X11ExcludeTimers       = 0x08,
      DeferredDeletion       = 0x10,
      EventLoopExec          = 0x20,
      DialogExec             = 0x40
   };
   using ProcessEventsFlags = QFlags<ProcessEventsFlag>;

   bool processEvents(ProcessEventsFlags flags = AllEvents);
   void processEvents(ProcessEventsFlags flags, int maxTime);

   int exec(ProcessEventsFlags flags = AllEvents);
   void exit(int returnCode = 0);
   bool isRunning() const;

   void wakeUp();

   CORE_CS_SLOT_1(Public, void quit())
   CORE_CS_SLOT_2(quit)

 protected:
   QScopedPointer<QEventLoopPrivate> d_ptr;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(QEventLoop::ProcessEventsFlags)

#endif // QEVENTLOOP_H
