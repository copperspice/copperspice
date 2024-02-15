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

#ifndef QABSTRACTANIMATION_P_H
#define QABSTRACTANIMATION_P_H

#include <qabstractanimation.h>
#include <qbasictimer.h>
#include <qdatetime.h>
#include <qelapsedtimer.h>
#include <qtimer.h>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

#ifndef QT_NO_ANIMATION

class QAnimationGroup;
class QAbstractAnimation;
class QUnifiedTimer;

class QAbstractAnimationPrivate
{
 public:
   QAbstractAnimationPrivate()
      : state(QAbstractAnimation::Stopped), direction(QAbstractAnimation::Forward),
        totalCurrentTime(0), currentTime(0),  loopCount(1), currentLoop(0), deleteWhenStopped(false),
        hasRegisteredTimer(false), isPause(false), isGroup(false), group(nullptr)
   {
   }

   virtual ~QAbstractAnimationPrivate()
   {
   }

   static QAbstractAnimationPrivate *get(QAbstractAnimation *q) {
      return q->d_func();
   }

   QAbstractAnimation::State state;
   QAbstractAnimation::Direction direction;
   void setState(QAbstractAnimation::State state);

   int totalCurrentTime;
   int currentTime;
   int loopCount;
   int currentLoop;

   bool deleteWhenStopped;
   bool hasRegisteredTimer;
   bool isPause;
   bool isGroup;

   QAnimationGroup *group;

 private:
   Q_DECLARE_PUBLIC(QAbstractAnimation)

 protected:
   QAbstractAnimation *q_ptr;
};

class QDefaultAnimationDriver : public QAnimationDriver
{
   CORE_CS_OBJECT(QDefaultAnimationDriver)

 public:
   QDefaultAnimationDriver(QUnifiedTimer *timer);
   void timerEvent(QTimerEvent *e) override;

   void started() override;
   void stopped() override;

 private:
   QBasicTimer m_timer;
   QUnifiedTimer *m_unified_timer;
};

class Q_CORE_EXPORT QAnimationDriverPrivate
{

 public:
   QAnimationDriverPrivate()
      : running(false)
   { }

   virtual ~QAnimationDriverPrivate() {
   }

   bool running;
};

using ElapsedTimer = QElapsedTimer;

class Q_CORE_EXPORT QUnifiedTimer : public QObject
{

 private:
   QUnifiedTimer();

 public:
   // may be required for declarative
   static QUnifiedTimer *instance();
   static QUnifiedTimer *instance(bool create);

   static void registerAnimation(QAbstractAnimation *animation, bool isTopLevel);
   static void unregisterAnimation(QAbstractAnimation *animation);

   //defines the timing interval. Default is DEFAULT_TIMER_INTERVAL
   void setTimingInterval(int interval);

   /*
      this allows to have a consistent timer interval at each tick from the timer
      not taking the real time that passed into account.
   */
   void setConsistentTiming(bool consistent) {
      consistentTiming = consistent;
   }

   // these facilitate fine-tuning of complex animations
   void setSlowModeEnabled(bool enabled) {
      slowMode = enabled;
   }

   void setSlowdownFactor(double factor) {
      slowdownFactor = factor;
   }

   /*
       this is used for updating the currentTime of all animations in case the pause
       timer is active or, otherwise, only of the animation passed as parameter.
   */
   static void ensureTimerUpdate();

   /*
       this will evaluate the need of restarting the pause timer in case there is still
       some pause animations running.
   */
   static void updateAnimationTimer();

   void installAnimationDriver(QAnimationDriver *driver);

   void restartAnimationTimer();
   void updateAnimationsTime();

   //useful for profiling/debugging
   int runningAnimationCount() {
      return animations.count();
   }

 protected:
   void timerEvent(QTimerEvent *) override;

 private:
   friend class QDefaultAnimationDriver;

   QAnimationDriver *driver;
   QDefaultAnimationDriver defaultDriver;

   QBasicTimer animationTimer;
   // timer used to delay the check if we should start/stop the animation timer
   QBasicTimer startStopAnimationTimer;

   ElapsedTimer time;

   qint64 lastTick;
   int timingInterval;
   int currentAnimationIdx;
   bool insideTick;
   bool consistentTiming;
   bool slowMode;

   // This factor will be used to divide the DEFAULT_TIMER_INTERVAL at each tick
   // when slowMode is enabled. Setting it to 0 or higher than DEFAULT_TIMER_INTERVAL (16)
   // stops all animations.
   double slowdownFactor;

   // bool to indicate that only pause animations are active
   bool isPauseTimerActive;

   QList<QAbstractAnimation *> animations, animationsToStart;

   // this is the count of running animations that are not a group neither a pause animation
   int runningLeafAnimations;
   QList<QAbstractAnimation *> runningPauseAnimations;

   void registerRunningAnimation(QAbstractAnimation *animation);
   void unregisterRunningAnimation(QAbstractAnimation *animation);

   int closestPauseAnimationTimeToFinish();
};

#endif //QT_NO_ANIMATION

#endif
