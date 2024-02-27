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

#include <qfuturewatcher.h>

#include <qalgorithms.h>
#include <qcoreapplication.h>
#include <qcoreevent.h>
#include <qthread.h>

#include <qfuturewatcher_p.h>

QFutureWatcherBase::QFutureWatcherBase(QObject *parent)
   : QObject(parent), d_ptr(new QFutureWatcherBasePrivate)
{
   d_ptr->q_ptr = this;
}

QFutureWatcherBase::~QFutureWatcherBase()
{
}

void QFutureWatcherBase::cancel()
{
   futureInterface().cancel();
}

void QFutureWatcherBase::setPaused(bool paused)
{
   futureInterface().setPaused(paused);
}
void QFutureWatcherBase::pause()
{
   futureInterface().setPaused(true);
}

void QFutureWatcherBase::resume()
{
   futureInterface().setPaused(false);
}

void QFutureWatcherBase::togglePaused()
{
   futureInterface().togglePaused();
}

int QFutureWatcherBase::progressValue() const
{
   return futureInterface().progressValue();
}

int QFutureWatcherBase::progressMinimum() const
{
   return futureInterface().progressMinimum();
}

int QFutureWatcherBase::progressMaximum() const
{
   return futureInterface().progressMaximum();
}

QString QFutureWatcherBase::progressText() const
{
   return futureInterface().progressText();
}

bool QFutureWatcherBase::isStarted() const
{
   return futureInterface().queryState(QFutureInterfaceBase::Started);
}

bool QFutureWatcherBase::isFinished() const
{
   Q_D(const QFutureWatcherBase);
   return d->finished;
}

bool QFutureWatcherBase::isRunning() const
{
   return futureInterface().queryState(QFutureInterfaceBase::Running);
}

bool QFutureWatcherBase::isCanceled() const
{
   return futureInterface().queryState(QFutureInterfaceBase::Canceled);
}

bool QFutureWatcherBase::isPaused() const
{
   return futureInterface().queryState(QFutureInterfaceBase::Paused);
}

void QFutureWatcherBase::waitForFinished()
{
   futureInterface().waitForFinished();
}

bool QFutureWatcherBase::event(QEvent *event)
{
   Q_D(QFutureWatcherBase);

   if (event->type() == QEvent::FutureCallOut) {
      QFutureCallOutEvent *callOutEvent = static_cast<QFutureCallOutEvent *>(event);

      if (futureInterface().isPaused()) {
         d->pendingCallOutEvents.append(callOutEvent->clone());
         return true;
      }

      if (callOutEvent->m_callOutType == QFutureCallOutEvent::Resumed
            && !d->pendingCallOutEvents.isEmpty()) {
         // send the resume
         d->sendCallOutEvent(callOutEvent);

         // next send all pending call outs
         for (int i = 0; i < d->pendingCallOutEvents.count(); ++i) {
            d->sendCallOutEvent(d->pendingCallOutEvents.at(i));
         }

         qDeleteAll(d->pendingCallOutEvents);
         d->pendingCallOutEvents.clear();
      } else {
         d->sendCallOutEvent(callOutEvent);
      }

      return true;
   }

   return QObject::event(event);
}

void QFutureWatcherBase::setPendingResultsLimit(int limit)
{
   Q_D(QFutureWatcherBase);
   d->maximumPendingResultsReady = limit;
}

void QFutureWatcherBase::connectNotify(const QMetaMethod &signal) const
{
   Q_D(const QFutureWatcherBase);

   static QMetaMethod resultSignal   = QMetaMethod::fromSignal(&QFutureWatcherBase::resultReadyAt);
   static QMetaMethod finishedSignal = QMetaMethod::fromSignal(&QFutureWatcherBase::finished);

   if (signal == resultSignal) {
      d->resultAtConnected.ref();
   }

   if (signal == finishedSignal) {
      if (futureInterface().isRunning()) {
         qWarning("QFutureWatcher::connectNotify() Connecting after calling setFuture() is likely to produce a race condition");
      }
   }
}

void QFutureWatcherBase::disconnectNotify(const QMetaMethod &signal) const
{
   Q_D(const QFutureWatcherBase);

   static QMetaMethod resultSignal = QMetaMethod::fromSignal(&QFutureWatcherBase::resultReadyAt);

   if (signal == resultSignal) {
      d->resultAtConnected.deref();
   }
}

QFutureWatcherBasePrivate::QFutureWatcherBasePrivate()
   : maximumPendingResultsReady(QThread::idealThreadCount() * 2), resultAtConnected(0)
{
}

void QFutureWatcherBase::connectOutputInterface()
{
   futureInterface().d->connectOutputInterface(d_func());
}

void QFutureWatcherBase::disconnectOutputInterface(bool pendingAssignment)
{
   if (pendingAssignment) {
      Q_D(QFutureWatcherBase);

      d->pendingResultsReady = 0;
      qDeleteAll(d->pendingCallOutEvents);
      d->pendingCallOutEvents.clear();
      d->finished = false;
   }

   futureInterface().d->disconnectOutputInterface(d_func());
}

void QFutureWatcherBasePrivate::postCallOutEvent(const QFutureCallOutEvent &callOutEvent)
{
   Q_Q(QFutureWatcherBase);

   if (callOutEvent.m_callOutType == QFutureCallOutEvent::ResultsReady) {
      if (pendingResultsReady.fetchAndAddRelaxed(1) >= maximumPendingResultsReady) {
         q->futureInterface().d->internal_setThrottled(true);
      }
   }

   QCoreApplication::postEvent(q, callOutEvent.clone());
}

void QFutureWatcherBasePrivate::callOutInterfaceDisconnected()
{
   QCoreApplication::removePostedEvents(q_func(), QEvent::FutureCallOut);
}

void QFutureWatcherBasePrivate::sendCallOutEvent(QFutureCallOutEvent *event)
{
   Q_Q(QFutureWatcherBase);

   switch (event->m_callOutType) {
      case QFutureCallOutEvent::Started:
         emit q->started();
         break;

      case QFutureCallOutEvent::Finished:
         finished = true;
         emit q->finished();
         break;

      case QFutureCallOutEvent::Canceled:
         pendingResultsReady = 0;
         emit q->canceled();
         break;

      case QFutureCallOutEvent::Paused:
         if (q->futureInterface().isCanceled()) {
            break;
         }

         emit q->paused();
         break;

      case QFutureCallOutEvent::Resumed:
         if (q->futureInterface().isCanceled()) {
            break;
         }

         emit q->resumed();
         break;

      case QFutureCallOutEvent::ResultsReady: {
         if (q->futureInterface().isCanceled()) {
            break;
         }

         if (pendingResultsReady.fetchAndAddRelaxed(-1) <= maximumPendingResultsReady) {
            q->futureInterface().setThrottled(false);
         }

         const int beginIndex = event->m_index1;
         const int endIndex   = event->m_index2;

         emit q->resultsReadyAt(beginIndex, endIndex);

         if (resultAtConnected.load() <= 0) {
            break;
         }

         for (int i = beginIndex; i < endIndex; ++i) {
            emit q->resultReadyAt(i);
         }

      }
      break;

      case QFutureCallOutEvent::Progress:
         if (q->futureInterface().isCanceled()) {
            break;
         }

         emit q->progressValueChanged(event->m_index1);

         if (! event->m_text.isEmpty()) {
            q->progressTextChanged(event->m_text);
         }

         break;

      case QFutureCallOutEvent::ProgressRange:
         emit q->progressRangeChanged(event->m_index1, event->m_index2);
         break;

      default:
         break;
   }
}
