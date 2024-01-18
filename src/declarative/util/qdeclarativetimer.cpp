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

#include "private/qdeclarativetimer_p.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qpauseanimation.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

class QDeclarativeTimerPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeTimer)

 public:
   QDeclarativeTimerPrivate()
      : interval(1000), running(false), repeating(false), triggeredOnStart(false)
      , classBegun(false), componentComplete(false), firstTick(true) {}
   int interval;
   QPauseAnimation pause;
   bool running : 1;
   bool repeating : 1;
   bool triggeredOnStart : 1;
   bool classBegun : 1;
   bool componentComplete : 1;
   bool firstTick : 1;
};

/*!
    \qmlclass Timer QDeclarativeTimer
    \ingroup qml-utility-elements
    \since 4.7
    \brief The Timer item triggers a handler at a specified interval.

    A Timer can be used to trigger an action either once, or repeatedly
    at a given interval.

    Here is a Timer that shows the current date and time, and updates
    the text every 500 milliseconds. It uses the JavaScript \c Date
    object to access the current time.

    \qml
    import QtQuick 1.0

    Item {
        Timer {
            interval: 500; running: true; repeat: true
            onTriggered: time.text = Date().toString()
        }

        Text { id: time }
    }
    \endqml

    The Timer element is synchronized with the animation timer.  Since the animation
    timer is usually set to 60fps, the resolution of Timer will be
    at best 16ms.

    If the Timer is running and one of its properties is changed, the
    elapsed time will be reset.  For example, if a Timer with interval of
    1000ms has its \e repeat property changed 500ms after starting, the
    elapsed time will be reset to 0, and the Timer will be triggered
    1000ms later.

    \sa {declarative/toys/clocks}{Clocks example}
*/

QDeclarativeTimer::QDeclarativeTimer(QObject *parent)
   : QObject(*(new QDeclarativeTimerPrivate), parent)
{
   Q_D(QDeclarativeTimer);
   connect(&d->pause, SIGNAL(currentLoopChanged(int)), this, SLOT(ticked()));
   connect(&d->pause, SIGNAL(finished()), this, SLOT(finished()));
   d->pause.setLoopCount(1);
   d->pause.setDuration(d->interval);
}

/*!
    \qmlproperty int Timer::interval

    Sets the \a interval between triggers, in milliseconds.

    The default interval is 1000 milliseconds.
*/
void QDeclarativeTimer::setInterval(int interval)
{
   Q_D(QDeclarativeTimer);
   if (interval != d->interval) {
      d->interval = interval;
      update();
      emit intervalChanged();
   }
}

int QDeclarativeTimer::interval() const
{
   Q_D(const QDeclarativeTimer);
   return d->interval;
}

/*!
    \qmlproperty bool Timer::running

    If set to true, starts the timer; otherwise stops the timer.
    For a non-repeating timer, \a running is set to false after the
    timer has been triggered.

    \a running defaults to false.

    \sa repeat
*/
bool QDeclarativeTimer::isRunning() const
{
   Q_D(const QDeclarativeTimer);
   return d->running;
}

void QDeclarativeTimer::setRunning(bool running)
{
   Q_D(QDeclarativeTimer);
   if (d->running != running) {
      d->running = running;
      d->firstTick = true;
      emit runningChanged();
      update();
   }
}

/*!
    \qmlproperty bool Timer::repeat

    If \a repeat is true the timer is triggered repeatedly at the
    specified interval; otherwise, the timer will trigger once at the
    specified interval and then stop (i.e. running will be set to false).

    \a repeat defaults to false.

    \sa running
*/
bool QDeclarativeTimer::isRepeating() const
{
   Q_D(const QDeclarativeTimer);
   return d->repeating;
}

void QDeclarativeTimer::setRepeating(bool repeating)
{
   Q_D(QDeclarativeTimer);
   if (repeating != d->repeating) {
      d->repeating = repeating;
      update();
      emit repeatChanged();
   }
}

/*!
    \qmlproperty bool Timer::triggeredOnStart

    When a timer is started, the first trigger is usually after the specified
    interval has elapsed.  It is sometimes desirable to trigger immediately
    when the timer is started; for example, to establish an initial
    state.

    If \a triggeredOnStart is true, the timer is triggered immediately
    when started, and subsequently at the specified interval. Note that if
    \e repeat is set to false, the timer is triggered twice; once on start,
    and again at the interval.

    \a triggeredOnStart defaults to false.

    \sa running
*/
bool QDeclarativeTimer::triggeredOnStart() const
{
   Q_D(const QDeclarativeTimer);
   return d->triggeredOnStart;
}

void QDeclarativeTimer::setTriggeredOnStart(bool triggeredOnStart)
{
   Q_D(QDeclarativeTimer);
   if (d->triggeredOnStart != triggeredOnStart) {
      d->triggeredOnStart = triggeredOnStart;
      update();
      emit triggeredOnStartChanged();
   }
}

/*!
    \qmlmethod Timer::start()
    \brief Starts the timer.

    If the timer is already running, calling this method has no effect.  The
    \c running property will be true following a call to \c start().
*/
void QDeclarativeTimer::start()
{
   setRunning(true);
}

/*!
    \qmlmethod Timer::stop()
    \brief Stops the timer.

    If the timer is not running, calling this method has no effect.  The
    \c running property will be false following a call to \c stop().
*/
void QDeclarativeTimer::stop()
{
   setRunning(false);
}

/*!
    \qmlmethod Timer::restart()
    \brief Restarts the timer.

    If the Timer is not running it will be started, otherwise it will be
    stopped, reset to initial state and started.  The \c running property
    will be true following a call to \c restart().
*/
void QDeclarativeTimer::restart()
{
   setRunning(false);
   setRunning(true);
}

void QDeclarativeTimer::update()
{
   Q_D(QDeclarativeTimer);
   if (d->classBegun && !d->componentComplete) {
      return;
   }
   d->pause.stop();
   if (d->running) {
      d->pause.setCurrentTime(0);
      d->pause.setLoopCount(d->repeating ? -1 : 1);
      d->pause.setDuration(d->interval);
      d->pause.start();
      if (d->triggeredOnStart && d->firstTick) {
         QCoreApplication::removePostedEvents(this, QEvent::MetaCall);
         QMetaObject::invokeMethod(this, "ticked", Qt::QueuedConnection);
      }
   }
}

void QDeclarativeTimer::classBegin()
{
   Q_D(QDeclarativeTimer);
   d->classBegun = true;
}

void QDeclarativeTimer::componentComplete()
{
   Q_D(QDeclarativeTimer);
   d->componentComplete = true;
   update();
}

/*!
    \qmlsignal Timer::onTriggered()

    This handler is called when the Timer is triggered.
*/
void QDeclarativeTimer::ticked()
{
   Q_D(QDeclarativeTimer);
   if (d->running && (d->pause.currentTime() > 0 || (d->triggeredOnStart && d->firstTick))) {
      emit triggered();
   }
   d->firstTick = false;
}

void QDeclarativeTimer::finished()
{
   Q_D(QDeclarativeTimer);
   if (d->repeating || !d->running) {
      return;
   }
   emit triggered();
   d->running = false;
   d->firstTick = false;
   emit runningChanged();
}

QT_END_NAMESPACE
