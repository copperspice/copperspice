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

#include <qfuturewatcher.h>
#include <qfuturewatcher_p.h>

#include <qalgorithms.h>
#include <qcoreevent.h>
#include <qcoreapplication.h>
#include <qthread.h>

QT_BEGIN_NAMESPACE

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

/*! \fn void QFutureWatcher::pause()

    Pauses the asynchronous computation represented by the future(). This is a
    convenience method that simply calls setPaused(true).

    \sa resume()
*/
void QFutureWatcherBase::pause()
{
   futureInterface().setPaused(true);
}

/*! \fn void QFutureWatcher::resume()

    Resumes the asynchronous computation represented by the future(). This is
    a convenience method that simply calls setPaused(false).

    \sa pause()
*/
void QFutureWatcherBase::resume()
{
   futureInterface().setPaused(false);
}

/*! \fn void QFutureWatcher::togglePaused()

    Toggles the paused state of the asynchronous computation. In other words,
    if the computation is currently paused, calling this function resumes it;
    if the computation is running, it becomes paused. This is a convenience
    method for calling setPaused(!isPaused()).

    \sa setPaused(), pause(), resume()
*/
void QFutureWatcherBase::togglePaused()
{
   futureInterface().togglePaused();
}

/*! \fn int QFutureWatcher::progressValue() const

    Returns the current progress value, which is between the progressMinimum()
    and progressMaximum().

    \sa progressMinimum(), progressMaximum()
*/
int QFutureWatcherBase::progressValue() const
{
   return futureInterface().progressValue();
}

/*! \fn int QFutureWatcher::progressMinimum() const

    Returns the minimum progressValue().

    \sa progressValue(), progressMaximum()
*/
int QFutureWatcherBase::progressMinimum() const
{
   return futureInterface().progressMinimum();
}

/*! \fn int QFutureWatcher::progressMaximum() const

    Returns the maximum progressValue().

    \sa progressValue(), progressMinimum()
*/
int QFutureWatcherBase::progressMaximum() const
{
   return futureInterface().progressMaximum();
}

/*! \fn QString QFutureWatcher::progressText() const

    Returns the (optional) textual representation of the progress as reported
    by the asynchronous computation.

    Be aware that not all computations provide a textual representation of the
    progress, and as such, this function may return an empty string.
*/
QString QFutureWatcherBase::progressText() const
{
   return futureInterface().progressText();
}

/*! \fn bool QFutureWatcher::isStarted() const

    Returns true if the asynchronous computation represented by the future()
    has been started; otherwise returns false.
*/
bool QFutureWatcherBase::isStarted() const
{
   return futureInterface().queryState(QFutureInterfaceBase::Started);
}

/*! \fn bool QFutureWatcher::isFinished() const

    Returns true if the asynchronous computation represented by the future()
    has finished; otherwise returns false.
*/
bool QFutureWatcherBase::isFinished() const
{
   Q_D(const QFutureWatcherBase);
   return d->finished;
}

/*! \fn bool QFutureWatcher::isRunning() const

    Returns true if the asynchronous computation represented by the future()
    is currently running; otherwise returns false.
*/
bool QFutureWatcherBase::isRunning() const
{
   return futureInterface().queryState(QFutureInterfaceBase::Running);
}

/*! \fn bool QFutureWatcher::isCanceled() const

    Returns true if the asynchronous computation has been canceled with the
    cancel() function; otherwise returns false.

    Be aware that the computation may still be running even though this
    function returns true. See cancel() for more details.
*/
bool QFutureWatcherBase::isCanceled() const
{
   return futureInterface().queryState(QFutureInterfaceBase::Canceled);
}

/*! \fn bool QFutureWatcher::isPaused() const

    Returns true if the asynchronous computation has been paused with the
    pause() function; otherwise returns false.

    Be aware that the computation may still be running even though this
    function returns true. See setPaused() for more details.

    \sa setPaused(), togglePaused()
*/
bool QFutureWatcherBase::isPaused() const
{
   return futureInterface().queryState(QFutureInterfaceBase::Paused);
}

/*! \fn void QFutureWatcher::waitForFinished()

    Waits for the asynchronous computation to finish (including cancel()ed
    computations).
*/
void QFutureWatcherBase::waitForFinished()
{
   futureInterface().waitForFinished();
}

/*! \fn void QFutureWatcher::setPendingResultsLimit(int limit)

    The setPendingResultsLimit() provides throttling control. When the number
    of pending resultReadyAt() or resultsReadyAt() signals exceeds the
    \a limit, the computation represented by the future will be throttled
    automatically. The computation will resume once the number of pending
    signals drops below the \a limit.
*/

bool QFutureWatcherBase::event(QEvent *event)
{
   Q_D(QFutureWatcherBase);
   if (event->type() == QEvent::FutureCallOut) {
      QFutureCallOutEvent *callOutEvent = static_cast<QFutureCallOutEvent *>(event);

      if (futureInterface().isPaused()) {
         d->pendingCallOutEvents.append(callOutEvent->clone());
         return true;
      }

      if (callOutEvent->callOutType == QFutureCallOutEvent::Resumed
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

void QFutureWatcherBase::connectNotify(const char *signal)
{
   Q_D(QFutureWatcherBase);

   const char *resultSignal = "resultReadyAt(int)";

   if (qstrcmp(signal, resultSignal) == 0) {
      d->resultAtConnected.ref();
   }

#ifndef QT_NO_DEBUG
   const char *finishedSignal = "finished()";

   if (qstrcmp(signal, finishedSignal) == 0) {
      if (futureInterface().isRunning()) {
         //connections should be established before calling stFuture to avoid race.
         // (The future could finish before the connection is made.)
         qWarning("QFutureWatcher::connect: connecting after calling setFuture() is likely to produce race");
      }
   }

#endif

}

void QFutureWatcherBase::disconnectNotify(const char *signal)
{
   Q_D(QFutureWatcherBase);

   const char *resultSignal = "resultReadyAt(int)";

   if (qstrcmp(signal, resultSignal) == 0) {
      d->resultAtConnected.deref();
   }
}

/*!
    \internal
*/
QFutureWatcherBasePrivate::QFutureWatcherBasePrivate()
   : maximumPendingResultsReady(QThread::idealThreadCount() * 2),
     resultAtConnected(0)
{ }

/*!
    \internal
*/
void QFutureWatcherBase::connectOutputInterface()
{
   futureInterface().d->connectOutputInterface(d_func());
}

/*!
    \internal
*/
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

   if (callOutEvent.callOutType == QFutureCallOutEvent::ResultsReady) {
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

   switch (event->callOutType) {
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

         const int beginIndex = event->index1;
         const int endIndex = event->index2;

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

         emit q->progressValueChanged(event->index1);

         if (! event->text.isEmpty()) {
            q->progressTextChanged(event->text);
         }
         break;

      case QFutureCallOutEvent::ProgressRange:
         emit q->progressRangeChanged(event->index1, event->index2);
         break;

      default:
         break;
   }
}


/*! \fn const T &QFutureWatcher::result() const

    Returns the first result in the future(). If the result is not immediately
    available, this function will block and wait for the result to become
    available. This is a convenience method for calling resultAt(0).

    \sa resultAt()
*/

/*! \fn const T &QFutureWatcher::resultAt(int index) const

    Returns the result at \a index in the future(). If the result is not
    immediately available, this function will block and wait for the result to
    become available.

    \sa result()
*/

/*! \fn void QFutureWatcher::setFuture(const QFuture<T> &future)

    Starts watching the given \a future.

    One of the signals might be emitted for the current state of the
    \a future. For example, if the future is already stopped, the
    finished signal will be emitted.

    To avoid a race condition, it is important to call this function
    \e after doing the connections.
*/

/*! \fn QFuture<T> QFutureWatcher::future() const

    Returns the watched future.
*/

/*! \fn void QFutureWatcher::started()

    This signal is emitted when this QFutureWatcher starts watching the future
    set with setFuture().
*/

/*!
    \fn void QFutureWatcher::finished()
    This signal is emitted when the watched future finishes.
*/

/*!
    \fn void QFutureWatcher::canceled()
    This signal is emitted if the watched future is canceled.
*/

/*! \fn void QFutureWatcher::paused()
    This signal is emitted when the watched future is paused.
*/

/*! \fn void QFutureWatcher::resumed()
    This signal is emitted when the watched future is resumed.
*/

/*!
    \fn void QFutureWatcher::progressRangeChanged(int minimum, int maximum)

    The progress range for the watched future has changed to \a minimum and
    \a maximum
*/

/*!
    \fn void QFutureWatcher::progressValueChanged(int progressValue)

    This signal is emitted when the watched future reports progress,
    \a progressValue gives the current progress. In order to avoid overloading
    the GUI event loop, QFutureWatcher limits the progress signal emission
    rate. This means that listeners connected to this slot might not get all
    progress reports the future makes. The last progress update (where
    \a progressValue equals the maximum value) will always be delivered.
*/

/*! \fn void QFutureWatcher::progressTextChanged(const QString &progressText)

    This signal is emitted when the watched future reports textual progress
    information, \a progressText.
*/

/*!
    \fn void QFutureWatcher::resultReadyAt(int index)

    This signal is emitted when the watched future reports a ready result at
    \a index. If the future reports multiple results, the index will indicate
    which one it is. Results can be reported out-of-order. To get the result,
    call future().result(index);
*/

/*!
    \fn void QFutureWatcher::resultsReadyAt(int beginIndex, int endIndex);

    This signal is emitted when the watched future reports ready results.
    The results are indexed from \a beginIndex to \a endIndex.

*/

QT_END_NAMESPACE


