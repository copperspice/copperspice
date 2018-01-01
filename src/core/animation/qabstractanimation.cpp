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

#include <qabstractanimation.h>
#include <qanimationgroup.h>
#include <QtCore/qdebug.h>
#include <qabstractanimation_p.h>

#include <QtCore/qmath.h>
#include <QtCore/qthreadstorage.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qpointer.h>

#ifndef QT_NO_ANIMATION

#define DEFAULT_TIMER_INTERVAL 16
#define STARTSTOP_TIMER_DELAY 0

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QThreadStorage<QUnifiedTimer *>, unifiedTimer)

QUnifiedTimer::QUnifiedTimer() :
   QObject(), defaultDriver(this), lastTick(0), timingInterval(DEFAULT_TIMER_INTERVAL),
   currentAnimationIdx(0), insideTick(false), consistentTiming(false), slowMode(false),
   slowdownFactor(5.0f), isPauseTimerActive(false), runningLeafAnimations(0)
{
   time.invalidate();
   driver = &defaultDriver;
}

QUnifiedTimer *QUnifiedTimer::instance(bool create)
{
   QUnifiedTimer *inst;

   if (create && !unifiedTimer()->hasLocalData()) {
      inst = new QUnifiedTimer;
      unifiedTimer()->setLocalData(inst);
   } else {
      inst = unifiedTimer()->localData();
   }

   return inst;
}

QUnifiedTimer *QUnifiedTimer::instance()
{
   return instance(true);
}

void QUnifiedTimer::ensureTimerUpdate()
{
   QUnifiedTimer *inst = QUnifiedTimer::instance(false);
   if (inst && inst->isPauseTimerActive) {
      inst->updateAnimationsTime();
   }
}

void QUnifiedTimer::updateAnimationsTime()
{
   //setCurrentTime can get this called again while we're the for loop. At least with pauseAnimations
   if (insideTick) {
      return;
   }

   qint64 totalElapsed = time.elapsed();
   // ignore consistentTiming in case the pause timer is active
   int delta = (consistentTiming && !isPauseTimerActive) ?
               timingInterval : totalElapsed - lastTick;
   if (slowMode) {
      if (slowdownFactor > 0) {
         delta = qRound(delta / slowdownFactor);
      } else {
         delta = 0;
      }
   }

   lastTick = totalElapsed;

   //we make sure we only call update time if the time has actually changed
   //it might happen in some cases that the time doesn't change because events are delayed
   //when the CPU load is high
   if (delta) {
      insideTick = true;
      for (currentAnimationIdx = 0; currentAnimationIdx < animations.count(); ++currentAnimationIdx) {
         QAbstractAnimation *animation = animations.at(currentAnimationIdx);
         int elapsed = QAbstractAnimationPrivate::get(animation)->totalCurrentTime
                       + (animation->direction() == QAbstractAnimation::Forward ? delta : -delta);
         animation->setCurrentTime(elapsed);
      }
      insideTick = false;
      currentAnimationIdx = 0;
   }
}

void QUnifiedTimer::updateAnimationTimer()
{
   QUnifiedTimer *inst = QUnifiedTimer::instance(false);
   if (inst) {
      inst->restartAnimationTimer();
   }
}

void QUnifiedTimer::restartAnimationTimer()
{
   if (runningLeafAnimations == 0 && !runningPauseAnimations.isEmpty()) {
      int closestTimeToFinish = closestPauseAnimationTimeToFinish();
      if (closestTimeToFinish < 0) {
         qDebug() << runningPauseAnimations;
         qDebug() << closestPauseAnimationTimeToFinish();
      }
      driver->stop();
      animationTimer.start(closestTimeToFinish, this);
      isPauseTimerActive = true;

   } else if (!driver->isRunning() || isPauseTimerActive) {
      driver->start();
      isPauseTimerActive = false;

   } else if (runningLeafAnimations == 0) {
      driver->stop();
   }
}

void QUnifiedTimer::setTimingInterval(int interval)
{
   timingInterval = interval;

   if (driver->isRunning() && !isPauseTimerActive) {
      //we changed the timing interval
      driver->stop();
      driver->start();
   }
}

void QUnifiedTimer::timerEvent(QTimerEvent *event)
{
   //in the case of consistent timing we make sure the orders in which events come is always the same
   //for that purpose we do as if the startstoptimer would always fire before the animation timer

   if ((consistentTiming && startStopAnimationTimer.isActive()) ||
         event->timerId() == startStopAnimationTimer.timerId()) {
      startStopAnimationTimer.stop();

      //we transfer the waiting animations into the "really running" state
      animations += animationsToStart;
      animationsToStart.clear();

      if (animations.isEmpty()) {
         animationTimer.stop();
         isPauseTimerActive = false;
         // invalidate the start reference time
         time.invalidate();
      } else {
         restartAnimationTimer();
         if (!time.isValid()) {
            lastTick = 0;
            time.start();
         }
      }
   }

   if (event->timerId() == animationTimer.timerId()) {
      // update current time on all top level animations
      updateAnimationsTime();
      restartAnimationTimer();
   }
}

void QUnifiedTimer::registerAnimation(QAbstractAnimation *animation, bool isTopLevel)
{
   QUnifiedTimer *inst = instance(true); //we create the instance if needed
   inst->registerRunningAnimation(animation);

   if (isTopLevel) {
      Q_ASSERT(!QAbstractAnimationPrivate::get(animation)->hasRegisteredTimer);
      QAbstractAnimationPrivate::get(animation)->hasRegisteredTimer = true;
      inst->animationsToStart << animation;
      if (!inst->startStopAnimationTimer.isActive()) {
         inst->startStopAnimationTimer.start(STARTSTOP_TIMER_DELAY, inst);
      }
   }
}

void QUnifiedTimer::unregisterAnimation(QAbstractAnimation *animation)
{
   QUnifiedTimer *inst = QUnifiedTimer::instance(false);
   if (inst) {
      //at this point the unified timer should have been created
      //but it might also have been already destroyed in case the application is shutting down

      inst->unregisterRunningAnimation(animation);

      if (!QAbstractAnimationPrivate::get(animation)->hasRegisteredTimer) {
         return;
      }

      int idx = inst->animations.indexOf(animation);
      if (idx != -1) {
         inst->animations.removeAt(idx);
         // this is needed if we unregister an animation while its running
         if (idx <= inst->currentAnimationIdx) {
            --inst->currentAnimationIdx;
         }

         if (inst->animations.isEmpty() && !inst->startStopAnimationTimer.isActive()) {
            inst->startStopAnimationTimer.start(STARTSTOP_TIMER_DELAY, inst);
         }
      } else {
         inst->animationsToStart.removeOne(animation);
      }
   }
   QAbstractAnimationPrivate::get(animation)->hasRegisteredTimer = false;
}

void QUnifiedTimer::registerRunningAnimation(QAbstractAnimation *animation)
{
   if (QAbstractAnimationPrivate::get(animation)->isGroup) {
      return;
   }

   if (QAbstractAnimationPrivate::get(animation)->isPause) {
      runningPauseAnimations << animation;
   } else {
      runningLeafAnimations++;
   }
}

void QUnifiedTimer::unregisterRunningAnimation(QAbstractAnimation *animation)
{
   if (QAbstractAnimationPrivate::get(animation)->isGroup) {
      return;
   }

   if (QAbstractAnimationPrivate::get(animation)->isPause) {
      runningPauseAnimations.removeOne(animation);
   } else {
      runningLeafAnimations--;
   }
   Q_ASSERT(runningLeafAnimations >= 0);
}

int QUnifiedTimer::closestPauseAnimationTimeToFinish()
{
   int closestTimeToFinish = INT_MAX;
   for (int i = 0; i < runningPauseAnimations.size(); ++i) {
      QAbstractAnimation *animation = runningPauseAnimations.at(i);
      int timeToFinish;

      if (animation->direction() == QAbstractAnimation::Forward) {
         timeToFinish = animation->duration() - animation->currentLoopTime();
      } else {
         timeToFinish = animation->currentLoopTime();
      }

      if (timeToFinish < closestTimeToFinish) {
         closestTimeToFinish = timeToFinish;
      }
   }
   return closestTimeToFinish;
}

void QUnifiedTimer::installAnimationDriver(QAnimationDriver *d)
{
   if (driver) {

      if (driver->isRunning()) {
         qWarning("QUnifiedTimer: Cannot change animation driver while animations are running");
         return;
      }

      if (driver != &defaultDriver) {
         delete driver;
      }
   }

   driver = d;
}

QAnimationDriver::QAnimationDriver(QObject *parent)
   : QObject(parent), d_ptr(new QAnimationDriverPrivate)
{
}

QAnimationDriver::QAnimationDriver(QAnimationDriverPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
}

QAnimationDriver::~QAnimationDriver()
{
}

void QAnimationDriver::advance()
{
   QUnifiedTimer *instance = QUnifiedTimer::instance();

   // update current time on all top level animations
   instance->updateAnimationsTime();
   instance->restartAnimationTimer();
}

void QAnimationDriver::install()
{
   QUnifiedTimer *timer = QUnifiedTimer::instance(true);
   timer->installAnimationDriver(this);
}

bool QAnimationDriver::isRunning() const
{
   return d_func()->running;
}

void QAnimationDriver::start()
{
   Q_D(QAnimationDriver);
   if (! d->running) {
      started();
      d->running = true;
   }
}

void QAnimationDriver::stop()
{
   Q_D(QAnimationDriver);
   if (d->running) {
      stopped();
      d->running = false;
   }
}

QDefaultAnimationDriver::QDefaultAnimationDriver(QUnifiedTimer *timer)
   : QAnimationDriver(0), m_unified_timer(timer)
{
}

void QDefaultAnimationDriver::timerEvent(QTimerEvent *e)
{
   Q_ASSERT(e->timerId() == m_timer.timerId());
   Q_UNUSED(e); // if the assertions are disabled
   advance();
}

void QDefaultAnimationDriver::started()
{
   m_timer.start(m_unified_timer->timingInterval, this);
}

void QDefaultAnimationDriver::stopped()
{
   m_timer.stop();
}

void QAbstractAnimationPrivate::setState(QAbstractAnimation::State newState)
{
   Q_Q(QAbstractAnimation);

   if (state == newState) {
      return;
   }

   if (loopCount == 0)  {
      return;
   }

   QAbstractAnimation::State oldState = state;
   int oldCurrentTime = currentTime;
   int oldCurrentLoop = currentLoop;

   QAbstractAnimation::Direction oldDirection = direction;

   // check if we should Rewind
   if ((newState == QAbstractAnimation::Paused || newState == QAbstractAnimation::Running)
         && oldState == QAbstractAnimation::Stopped) {

      // reset the time if needed
      // do not call setCurrentTime because this might change the way the animation behaves.
      // changing the state or changing the current value
      totalCurrentTime = currentTime = (direction == QAbstractAnimation::Forward) ?
                                       0 : (loopCount == -1 ? q->duration() : q->totalDuration());
   }

   state = newState;
   QWeakPointer<QAbstractAnimation> guard(q);

   //(un)registration of the animation must always happen before calls to
   //virtual function (updateState) to ensure a correct state of the timer

   bool isTopLevel = ! group || group->state() == QAbstractAnimation::Stopped;

   if (oldState == QAbstractAnimation::Running) {
      if (newState == QAbstractAnimation::Paused && hasRegisteredTimer)  {
         QUnifiedTimer::ensureTimerUpdate();
      }

      // animation is not running any more
      QUnifiedTimer::unregisterAnimation(q);

   } else if (newState == QAbstractAnimation::Running) {
      QUnifiedTimer::registerAnimation(q, isTopLevel);

   }

   q->updateState(newState, oldState);
   if (! guard || newState != state)  {
      // this is to be safe if updateState changes the state
      return;
   }

   // notify state change
   emit q->stateChanged(newState, oldState);

   if (! guard || newState != state)  {
      //this is to be safe if updateState changes the state
      return;
   }

   switch (state) {
      case QAbstractAnimation::Paused:
         break;

      case QAbstractAnimation::Running: {
         // this ensures that the value is updated now that the animation is running
         if (oldState == QAbstractAnimation::Stopped) {
            if (isTopLevel) {
               // currentTime needs to be updated if pauseTimer is active
               QUnifiedTimer::ensureTimerUpdate();
               q->setCurrentTime(totalCurrentTime);
            }
         }
      }
      break;

      case QAbstractAnimation::Stopped:
         // Leave running state
         int dura = q->duration();

         if (deleteWhenStopped) {
            q->deleteLater();
         }

         if (dura == -1 || loopCount < 0
               || (oldDirection == QAbstractAnimation::Forward && (oldCurrentTime * (oldCurrentLoop + 1)) == (dura * loopCount))
               || (oldDirection == QAbstractAnimation::Backward && oldCurrentTime == 0)) {
            emit q->finished();
         }

         break;
   }
}

QAbstractAnimation::QAbstractAnimation(QObject *parent)
   : QObject(0), d_ptr(new QAbstractAnimationPrivate)
{
   d_ptr->q_ptr = this;

   // Allow auto-add on reparent
   setParent(parent);
}

// internal
QAbstractAnimation::QAbstractAnimation(QAbstractAnimationPrivate &dd, QObject *parent)
   : QObject(0), d_ptr(&dd)
{
   d_ptr->q_ptr = this;

   // Allow auto-add on reparent
   setParent(parent);
}

/*!
    Stops the animation if it's running, then destroys the
    QAbstractAnimation. If the animation is part of a QAnimationGroup, it is
    automatically removed before it's destroyed.
*/
QAbstractAnimation::~QAbstractAnimation()
{
   Q_D(QAbstractAnimation);

   // ca not call stop here. otherwise we get pure virtual calls
   if (d->state != Stopped) {
      QAbstractAnimation::State oldState = d->state;
      d->state = Stopped;
      emit stateChanged(oldState, d->state);

      if (oldState == QAbstractAnimation::Running) {
         QUnifiedTimer::unregisterAnimation(this);
      }
   }
}

QAbstractAnimation::State QAbstractAnimation::state() const
{
   Q_D(const QAbstractAnimation);
   return d->state;
}

QAnimationGroup *QAbstractAnimation::group() const
{
   Q_D(const QAbstractAnimation);
   return d->group;
}

QAbstractAnimation::Direction QAbstractAnimation::direction() const
{
   Q_D(const QAbstractAnimation);
   return d->direction;
}
void QAbstractAnimation::setDirection(Direction direction)
{
   Q_D(QAbstractAnimation);
   if (d->direction == direction) {
      return;
   }

   if (state() == Stopped) {
      if (direction == Backward) {
         d->currentTime = duration();
         d->currentLoop = d->loopCount - 1;
      } else {
         d->currentTime = 0;
         d->currentLoop = 0;
      }
   }

   // the commands order below is important: first we need to setCurrentTime with the old direction,
   // then update the direction on this and all children and finally restart the pauseTimer if needed
   if (d->hasRegisteredTimer) {
      QUnifiedTimer::ensureTimerUpdate();
   }

   d->direction = direction;
   updateDirection(direction);

   if (d->hasRegisteredTimer)
      // needed to update the timer interval in case of a pause animation
   {
      QUnifiedTimer::updateAnimationTimer();
   }

   emit directionChanged(direction);
}

int QAbstractAnimation::loopCount() const
{
   Q_D(const QAbstractAnimation);
   return d->loopCount;
}

void QAbstractAnimation::setLoopCount(int loopCount)
{
   Q_D(QAbstractAnimation);
   d->loopCount = loopCount;
}

int QAbstractAnimation::currentLoop() const
{
   Q_D(const QAbstractAnimation);
   return d->currentLoop;
}

int QAbstractAnimation::totalDuration() const
{
   int dura = duration();
   if (dura <= 0) {
      return dura;
   }

   int loopcount = loopCount();
   if (loopcount < 0) {
      return -1;
   }

   return dura * loopcount;
}

int QAbstractAnimation::currentLoopTime() const
{
   Q_D(const QAbstractAnimation);
   return d->currentTime;
}

int QAbstractAnimation::currentTime() const
{
   Q_D(const QAbstractAnimation);
   return d->totalCurrentTime;
}
void QAbstractAnimation::setCurrentTime(int msecs)
{
   Q_D(QAbstractAnimation);
   msecs = qMax(msecs, 0);

   // Calculate new time and loop.
   int dura = duration();
   int totalDura = dura <= 0 ? dura : ((d->loopCount < 0) ? -1 : dura * d->loopCount);

   if (totalDura != -1) {
      msecs = qMin(totalDura, msecs);
   }

   d->totalCurrentTime = msecs;

   // Update new values.
   int oldLoop = d->currentLoop;
   d->currentLoop = ((dura <= 0) ? 0 : (msecs / dura));

   if (d->currentLoop == d->loopCount) {
      //we're at the end
      d->currentTime = qMax(0, dura);
      d->currentLoop = qMax(0, d->loopCount - 1);

   } else {
      if (d->direction == Forward) {
         d->currentTime = (dura <= 0) ? msecs : (msecs % dura);
      } else {
         d->currentTime = (dura <= 0) ? msecs : ((msecs - 1) % dura) + 1;
         if (d->currentTime == dura) {
            --d->currentLoop;
         }
      }
   }

   updateCurrentTime(d->currentTime);
   if (d->currentLoop != oldLoop) {
      emit currentLoopChanged(d->currentLoop);
   }

   // All animations are responsible for stopping the animation when their
   // own end state is reached; in this case the animation is time driven,
   // and has reached the end.
   if ((d->direction == Forward && d->totalCurrentTime == totalDura)
         || (d->direction == Backward && d->totalCurrentTime == 0)) {
      stop();
   }
}

void QAbstractAnimation::start(DeletionPolicy policy)
{
   Q_D(QAbstractAnimation);

   if (d->state == Running) {
      return;
   }

   d->deleteWhenStopped = policy;
   d->setState(Running);
}

void QAbstractAnimation::stop()
{
   Q_D(QAbstractAnimation);

   if (d->state == Stopped) {
      return;
   }

   d->setState(Stopped);
}

void QAbstractAnimation::pause()
{
   Q_D(QAbstractAnimation);
   if (d->state == Stopped) {
      qWarning("QAbstractAnimation::pause: Cannot pause a stopped animation");
      return;
   }

   d->setState(Paused);
}

/*!
    Resumes the animation after it was paused. When the animation is resumed,
    it emits the resumed() and stateChanged() signals. The currenttime is not
    changed.

    \sa start(), pause(), state()
 */
void QAbstractAnimation::resume()
{
   Q_D(QAbstractAnimation);
   if (d->state != Paused) {
      qWarning("QAbstractAnimation::resume: "
               "Cannot resume an animation that is not paused");
      return;
   }

   d->setState(Running);
}

/*!
    If \a paused is true, the animation is paused.
    If \a paused is false, the animation is resumed.

    \sa state(), pause(), resume()
*/
void QAbstractAnimation::setPaused(bool paused)
{
   if (paused) {
      pause();
   } else {
      resume();
   }
}


/*!
    \reimp
*/
bool QAbstractAnimation::event(QEvent *event)
{
   return QObject::event(event);
}

/*!
    \fn virtual void QAbstractAnimation::updateCurrentTime(int currentTime) = 0;

    This pure virtual function is called every time the animation's
    \a currentTime changes.

    \sa updateState()
*/

/*!
    This virtual function is called by QAbstractAnimation when the state
    of the animation is changed from \a oldState to \a newState.

    \sa start(), stop(), pause(), resume()
*/
void QAbstractAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
   Q_UNUSED(oldState);
   Q_UNUSED(newState);
}

/*!
    This virtual function is called by QAbstractAnimation when the direction
    of the animation is changed. The \a direction argument is the new direction.

    \sa setDirection(), direction()
*/
void QAbstractAnimation::updateDirection(QAbstractAnimation::Direction direction)
{
   Q_UNUSED(direction);
}


QT_END_NAMESPACE

#endif //QT_NO_ANIMATION
