/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qplatformdefs.h>
#include <qapplication.h>
#include <qeventdispatcher_qpa_p.h>
#include <qeventdispatcher_unix_p.h>
#include <qapplication_p.h>
#include <qplatformeventloopintegration_qpa.h>
#include <QWindowSystemInterface>
#include <QtCore/QElapsedTimer>
#include <QtCore/QAtomicInt>
#include <QtCore/QSemaphore>
#include <QtCore/QDebug>
#include <errno.h>
#include <limits.h>

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE

class Rendezvous
{
 public:
   void checkpoint() {
      if (state.testAndSetOrdered(0, 1)) {
         semaphore.acquire();
      } else if (state.testAndSetAcquire(1, 0)) {
         semaphore.release();
      } else {
         qWarning("Barrier internal error");
      }
   }
 private:
   QSemaphore semaphore;
   QAtomicInt state;
};

class SelectWorker : public QThread
{
 public:
   SelectWorker(QEventDispatcherQPAPrivate *eventDispatcherPrivate)
      : QThread(),
        m_edPrivate(eventDispatcherPrivate),
        m_retVal(0) {
   }

   void setSelectValues(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds) {
      m_nfds = nfds;
      m_readfds = readfds;
      m_writefds = writefds;
      m_exceptfds = exceptfds;


   }

   int retVal() const {
      return m_retVal;
   }

 protected:
   void run();

 private:
   QEventDispatcherQPAPrivate *m_edPrivate;
   int m_retVal;

   int m_nfds;
   fd_set *m_readfds, *m_writefds, *m_exceptfds;
};

class QEventDispatcherQPAPrivate : public QEventDispatcherUNIXPrivate
{
   Q_DECLARE_PUBLIC(QEventDispatcherQPA)
 public:
   QEventDispatcherQPAPrivate()
      :  eventLoopIntegration(0),
         barrierBeforeBlocking(0),
         barrierReturnValue(0),
         selectReturnMutex(0),
         selectWorkerNeedsSync(true),
         selectWorkerHasResult(false),
         selectWorker(0),
         m_integrationInitialised(false),
         m_hasIntegration(false),
         m_isEventLoopIntegrationRunning(false) {

   }

   ~QEventDispatcherQPAPrivate() {
      delete selectWorker;
      delete eventLoopIntegration;
      delete barrierBeforeBlocking;
      delete barrierReturnValue;
      delete selectReturnMutex;
   }

   bool hasIntegration() const {
      if (!m_integrationInitialised) {
         QEventDispatcherQPAPrivate *that = const_cast<QEventDispatcherQPAPrivate *>(this);
         if (qApp && (qApp->thread() == QThread::currentThread())) { // guiThread
            if (QApplicationPrivate::platformIntegration()) {
               that->eventLoopIntegration = QApplicationPrivate::platformIntegration()->createEventLoopIntegration();
               if (that->eventLoopIntegration) {
                  that->selectWorker = new SelectWorker(that);
                  that->barrierBeforeBlocking = new Rendezvous;
                  that->barrierReturnValue = new Rendezvous;
                  that->selectReturnMutex = new QMutex;
                  that->selectWorker->start();
                  that->m_hasIntegration = true;
                  if (!QElapsedTimer::isMonotonic()) {
                     qWarning("Having eventloop integration without monotonic timers can lead to undefined behaviour");
                  }
               }
            }
         }
         that->m_integrationInitialised = true;
      }
      return m_hasIntegration;
   }

   bool isEventLoopIntegrationRunning() const {
      return m_isEventLoopIntegrationRunning;
   }

   void runEventLoopIntegration() {
      if (qApp && (qApp->thread() == QThread::currentThread())) {
         m_isEventLoopIntegrationRunning = true;
         eventLoopIntegration->startEventLoop();
      }
   }

   QPlatformEventLoopIntegration *eventLoopIntegration;
   Rendezvous *barrierBeforeBlocking;
   Rendezvous *barrierReturnValue;

   QMutex *selectReturnMutex;
   bool selectWorkerNeedsSync;
   bool selectWorkerHasResult;

   SelectWorker *selectWorker;
 private:
   bool m_integrationInitialised;
   bool m_hasIntegration;
   bool m_isEventLoopIntegrationRunning;
};

QEventDispatcherQPA::QEventDispatcherQPA(QObject *parent)
   : QEventDispatcherUNIX(*new QEventDispatcherQPAPrivate, parent)
{ }

QEventDispatcherQPA::~QEventDispatcherQPA()
{ }

bool QEventDispatcherQPA::processEvents(QEventLoop::ProcessEventsFlags flags)
{
   Q_D(QEventDispatcherQPA);

   if (d->hasIntegration()) {
      if (!d->isEventLoopIntegrationRunning()) {
         d->runEventLoopIntegration();
      }
      if (d->threadData->quitNow) {
         d->eventLoopIntegration->quitEventLoop();
         return false;
      }
   }

   int nevents = 0;

   // handle gui and posted events
   d->interrupt = false;
   QApplication::sendPostedEvents();

   while (!d->interrupt) {        // also flushes output buffer ###can be optimized
      QWindowSystemInterfacePrivate::WindowSystemEvent *event;
      if (!(flags & QEventLoop::ExcludeUserInputEvents)
            && QWindowSystemInterfacePrivate::windowSystemEventsQueued() > 0) {
         // process a pending user input event
         event = QWindowSystemInterfacePrivate::getWindowSystemEvent();
         if (!event) {
            break;
         }
      } else {
         break;
      }

      if (filterEvent(event)) {
         delete event;
         continue;
      }
      nevents++;

      QApplicationPrivate::processWindowSystemEvent(event);
      delete event;
   }

   if (!d->interrupt) {
      if (QEventDispatcherUNIX::processEvents(flags)) {
         return true;
      }
   }

   return (nevents > 0);
}

bool QEventDispatcherQPA::hasPendingEvents()
{
   extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
   return qGlobalPostedEventsCount() || QWindowSystemInterfacePrivate::windowSystemEventsQueued();
}

void QEventDispatcherQPA::registerSocketNotifier(QSocketNotifier *notifier)
{
   Q_D(QEventDispatcherQPA);
   QEventDispatcherUNIX::registerSocketNotifier(notifier);
   if (d->hasIntegration()) {
      wakeUp();
   }

}

void QEventDispatcherQPA::unregisterSocketNotifier(QSocketNotifier *notifier)
{
   Q_D(QEventDispatcherQPA);
   QEventDispatcherUNIX::unregisterSocketNotifier(notifier);
   if (d->hasIntegration()) {
      wakeUp();
   }
}

void QEventDispatcherQPA::flush()
{
   if (qApp) {
      qApp->sendPostedEvents();
   }
}

int QEventDispatcherQPA::select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                                timeval *timeout)
{
   Q_D(QEventDispatcherQPA);
   int retVal = 0;
   if (d->hasIntegration()) {
      qint64 timeoutmsec = LONG_MAX; // wait if we don't have timers
      if (timeout) {
         timeoutmsec = timeout->tv_sec * 1000 + (timeout->tv_usec / 1000);
      }
      d->selectReturnMutex->lock();
      if (d->selectWorkerNeedsSync) {
         if (d->selectWorkerHasResult) {
            retVal = d->selectWorker->retVal();
            d->selectWorkerHasResult = false;

            d->selectReturnMutex->unlock();
            d->barrierReturnValue->checkpoint();
            d->eventLoopIntegration->setNextTimerEvent(0);
            return retVal;
         } else {
            d->selectWorkerNeedsSync = false;
            d->selectWorker->setSelectValues(nfds, readfds, writefds, exceptfds);
            d->barrierBeforeBlocking->checkpoint();
         }
      }
      d->selectReturnMutex->unlock();
      d->eventLoopIntegration->setNextTimerEvent(timeoutmsec);
      retVal = 0; //is 0 if select has not returned
   } else {
      retVal = QEventDispatcherUNIX::select(nfds, readfds, writefds, exceptfds, timeout);
   }
   return retVal;
}


void SelectWorker::run()
{

   while (true) {
      m_retVal = 0;
      m_edPrivate->barrierBeforeBlocking->checkpoint(); // wait for mainthread
      int tmpRet = qt_safe_select(m_nfds, m_readfds, m_writefds, m_exceptfds, 0);
      m_edPrivate->selectReturnMutex->lock();
      m_edPrivate->eventLoopIntegration->qtNeedsToProcessEvents();

      m_edPrivate->selectWorkerNeedsSync = true;
      m_edPrivate->selectWorkerHasResult = true;
      m_retVal = tmpRet;

      m_edPrivate->selectReturnMutex->unlock();
      m_edPrivate->barrierReturnValue->checkpoint();
   }
}
QT_END_NAMESPACE
