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

// do not move include, if qthread.h is included directly forward declarations are not sufficient 12/30/2013
#include <qobject.h>

#ifndef QTHREAD_H
#define QTHREAD_H

#include <qscopedpointer.h>

#include <limits.h>

class QThreadData;
class QThreadPrivate;
class QAbstractEventDispatcher;

class Q_CORE_EXPORT QThread : public QObject
{
   CORE_CS_OBJECT(QThread)

 public:
   static Qt::HANDLE currentThreadId();
   static QThread *currentThread();
   static int idealThreadCount();
   static void yieldCurrentThread();

   explicit QThread(QObject *parent = nullptr);
   ~QThread();

   enum Priority {
      IdlePriority,

      LowestPriority,
      LowPriority,
      NormalPriority,
      HighPriority,
      HighestPriority,

      TimeCriticalPriority,

      InheritPriority
   };

   void setPriority(Priority priority);
   Priority priority() const;

   bool isFinished() const;
   bool isRunning() const;

   void requestInterruption();
   bool isInterruptionRequested() const;
   void setStackSize(uint stackSize);
   uint stackSize() const;

   void exit(int returnCode = 0);

   // default argument causes thread to block indefinately
   bool wait(unsigned long time = ULONG_MAX);

   static void sleep(unsigned long secs);
   static void msleep(unsigned long msecs);
   static void usleep(unsigned long usecs);

   QAbstractEventDispatcher *eventDispatcher() const;
   void setEventDispatcher(QAbstractEventDispatcher *eventDispatcher);

   bool event(QEvent *event) override;
   int loopLevel() const;

   CORE_CS_SIGNAL_1(Public, void started())
   CORE_CS_SIGNAL_2(started)

   CORE_CS_SIGNAL_1(Public, void finished())
   CORE_CS_SIGNAL_2(finished)

   CORE_CS_SLOT_1(Public, void start(Priority priority = InheritPriority))
   CORE_CS_SLOT_2(start)

   CORE_CS_SLOT_1(Public, void terminate())
   CORE_CS_SLOT_2(terminate)

   CORE_CS_SLOT_1(Public, void quit())
   CORE_CS_SLOT_2(quit)

 protected:
   virtual void run();
   int exec();

   static void setTerminationEnabled(bool enabled = true);

   QThread(QThreadPrivate &dd, QObject *parent = nullptr);
   QScopedPointer<QThreadPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QThread)

   friend class QCoreApplication;
   friend class QThreadData;
};

#endif
