/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qeventloop.h>
#include <qabstracteventdispatcher.h>
#include <qcoreapplication.h>
#include <qelapsedtimer.h>
#include <qthread_p.h>



class QEventLoopPrivate
{
   Q_DECLARE_PUBLIC(QEventLoop)

 public:
   inline QEventLoopPrivate() : exit(true), inExec(false), returnCode(-1)   {  }
   virtual ~QEventLoopPrivate() {}

   bool exit, inExec;
   int returnCode;

 protected:
   QEventLoop *q_ptr;

};

/*!
    Constructs an event loop object with the given \a parent.
*/
QEventLoop::QEventLoop(QObject *parent)
   : QObject(parent), d_ptr(new QEventLoopPrivate)
{
   d_ptr->q_ptr = this;
   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   if (! QCoreApplication::instance()) {
      qWarning("QEventLoop: Can not be used without QApplication");

   } else if (! threadData->eventDispatcher) {
      QThreadPrivate::createEventDispatcher(threadData);
   }
}

QEventLoop::~QEventLoop()
{ }

bool QEventLoop::processEvents(ProcessEventsFlags flags)
{
   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   if (! threadData->eventDispatcher) {
      return false;
   }

   if (flags & DeferredDeletion) {
      QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
   }

   return threadData->eventDispatcher.load()->processEvents(flags);
}

int QEventLoop::exec(ProcessEventsFlags flags)
{
   Q_D(QEventLoop);
   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   // we need to protect from race condition with QThread::exit
   QMutexLocker locker(&threadData->get_QThreadPrivate()->mutex);

   if (threadData->quitNow) {
      return -1;
   }

   if (d->inExec) {
      qWarning("QEventLoop::exec: instance %p has already called exec()", this);
      return -1;
   }

   d->inExec = true;
   d->exit   = false;

   ++threadData->loopLevel;
   threadData->eventLoops.push(this);
   locker.unlock();

   // remove posted quit events when entering a new event loop
   QCoreApplication *app = QCoreApplication::instance();

   if (app && app->thread() == thread()) {
      QCoreApplication::removePostedEvents(app, QEvent::Quit);
   }

   try {
      while (! d->exit) {
         processEvents(flags | WaitForMoreEvents | EventLoopExec);
      }

   } catch (...) {
      qWarning("CopperSpice has caught an exception thrown from an event handler.\n"
               "Reimplement QApplication::notify() and catch all exceptions.\n");

      // copied from below
      locker.relock();

      QEventLoop *eventLoop = threadData->eventLoops.pop();
      Q_ASSERT_X(eventLoop == this, "QEventLoop::exec()", "internal error");
      Q_UNUSED(eventLoop); // --release warning

      d->inExec = false;
      --threadData->loopLevel;

      throw;
   }

   // copied above
   locker.relock();

   QEventLoop *eventLoop = threadData->eventLoops.pop();
   Q_ASSERT_X(eventLoop == this, "QEventLoop::exec()", "internal error");
   Q_UNUSED(eventLoop); // --release warning

   d->inExec = false;
   --threadData->loopLevel;

   return d->returnCode;
}

void QEventLoop::processEvents(ProcessEventsFlags flags, int maxTime)
{
   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   if (! threadData->eventDispatcher) {
      return;
   }

   QElapsedTimer start;
   start.start();

   if (flags & DeferredDeletion) {
      QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
   }

   while (processEvents(flags & ~WaitForMoreEvents)) {
      if (start.elapsed() > maxTime) {
         break;
      }

      if (flags & DeferredDeletion) {
         QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
      }
   }
}

void QEventLoop::exit(int returnCode)
{
   Q_D(QEventLoop);
   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   if (! threadData->eventDispatcher) {
      return;
   }

   d->returnCode = returnCode;
   d->exit = true;
   threadData->eventDispatcher.load()->interrupt();
}

/*!
    Returns true if the event loop is running; otherwise returns
    false. The event loop is considered running from the time when
    exec() is called until exit() is called.

    \sa exec() exit()
 */
bool QEventLoop::isRunning() const
{
   Q_D(const QEventLoop);
   return ! d->exit;
}

/*!
    Wakes up the event loop.

    \sa QAbstractEventDispatcher::wakeUp()
*/
void QEventLoop::wakeUp()
{
   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   if (! threadData->eventDispatcher) {
      return;
   }

   threadData->eventDispatcher.load()->wakeUp();
}

void QEventLoop::quit()
{
   exit(0);
}

