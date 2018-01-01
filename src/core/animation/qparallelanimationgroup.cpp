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

#include <qparallelanimationgroup.h>
#include <qparallelanimationgroup_p.h>

// #define QANIMATION_DEBUG

#ifndef QT_NO_ANIMATION

QT_BEGIN_NAMESPACE

QParallelAnimationGroup::QParallelAnimationGroup(QObject *parent)
   : QAnimationGroup(*new QParallelAnimationGroupPrivate, parent)
{
}

QParallelAnimationGroup::QParallelAnimationGroup(QParallelAnimationGroupPrivate &dd,
      QObject *parent)
   : QAnimationGroup(dd, parent)
{
}

QParallelAnimationGroup::~QParallelAnimationGroup()
{
}

/*!
    \reimp
*/
int QParallelAnimationGroup::duration() const
{
   Q_D(const QParallelAnimationGroup);
   int ret = 0;

   for (int i = 0; i < d->animations.size(); ++i) {
      QAbstractAnimation *animation = d->animations.at(i);
      const int currentDuration = animation->totalDuration();
      if (currentDuration == -1) {
         return -1;   // Undetermined length
      }

      ret = qMax(ret, currentDuration);
   }

   return ret;
}

/*!
    \reimp
*/
void QParallelAnimationGroup::updateCurrentTime(int currentTime)
{
   Q_D(QParallelAnimationGroup);
   if (d->animations.isEmpty()) {
      return;
   }

   if (d->currentLoop > d->lastLoop) {
      // simulate completion of the loop
      int dura = duration();
      if (dura > 0) {
         for (int i = 0; i < d->animations.size(); ++i) {
            QAbstractAnimation *animation = d->animations.at(i);
            if (animation->state() != QAbstractAnimation::Stopped) {
               d->animations.at(i)->setCurrentTime(dura);   // will stop
            }
         }
      }
   } else if (d->currentLoop < d->lastLoop) {
      // simulate completion of the loop seeking backwards
      for (int i = 0; i < d->animations.size(); ++i) {
         QAbstractAnimation *animation = d->animations.at(i);
         //we need to make sure the animation is in the right state
         //and then rewind it
         d->applyGroupState(animation);
         animation->setCurrentTime(0);
         animation->stop();
      }
   }

#ifdef QANIMATION_DEBUG
   qDebug("QParallellAnimationGroup %5d: setCurrentTime(%d), loop:%d, last:%d, timeFwd:%d, lastcurrent:%d, %d",
          __LINE__, d->currentTime, d->currentLoop, d->lastLoop, timeFwd, d->lastCurrentTime, state());
#endif
   // finally move into the actual time of the current loop
   for (int i = 0; i < d->animations.size(); ++i) {
      QAbstractAnimation *animation = d->animations.at(i);
      const int dura = animation->totalDuration();
      //if the loopcount is bigger we should always start all animations
      if (d->currentLoop > d->lastLoop
            //if we're at the end of the animation, we need to start it if it wasn't already started in this loop
            //this happens in Backward direction where not all animations are started at the same time
            || d->shouldAnimationStart(animation, d->lastCurrentTime > dura /*startIfAtEnd*/)) {
         d->applyGroupState(animation);
      }

      if (animation->state() == state()) {
         animation->setCurrentTime(currentTime);
         if (dura > 0 && currentTime > dura) {
            animation->stop();
         }
      }
   }
   d->lastLoop = d->currentLoop;
   d->lastCurrentTime = currentTime;
}

/*!
    \reimp
*/
void QParallelAnimationGroup::updateState(QAbstractAnimation::State newState,
      QAbstractAnimation::State oldState)
{
   Q_D(QParallelAnimationGroup);
   QAnimationGroup::updateState(newState, oldState);

   switch (newState) {
      case Stopped:
         for (int i = 0; i < d->animations.size(); ++i) {
            d->animations.at(i)->stop();
         }
         d->disconnectUncontrolledAnimations();
         break;
      case Paused:
         for (int i = 0; i < d->animations.size(); ++i)
            if (d->animations.at(i)->state() == Running) {
               d->animations.at(i)->pause();
            }
         break;
      case Running:
         d->connectUncontrolledAnimations();
         for (int i = 0; i < d->animations.size(); ++i) {
            QAbstractAnimation *animation = d->animations.at(i);
            if (oldState == Stopped) {
               animation->stop();
            }
            animation->setDirection(d->direction);
            if (d->shouldAnimationStart(animation, oldState == Stopped)) {
               animation->start();
            }
         }
         break;
   }
}

void QParallelAnimationGroup::_q_uncontrolledAnimationFinished()
{
   Q_D(QParallelAnimationGroup);
   d->_q_uncontrolledAnimationFinished();
}

void QParallelAnimationGroupPrivate::_q_uncontrolledAnimationFinished()
{
   Q_Q(QParallelAnimationGroup);

   QAbstractAnimation *animation = qobject_cast<QAbstractAnimation *>(q->sender());
   Q_ASSERT(animation);

   int uncontrolledRunningCount = 0;

   if (animation->duration() == -1 || animation->loopCount() < 0) {
      QHash<QAbstractAnimation *, int>::iterator it = uncontrolledFinishTime.begin();

      while (it != uncontrolledFinishTime.end()) {
         if (it.key() == animation) {
            *it = animation->currentTime();
         }
         if (it.value() == -1) {
            ++uncontrolledRunningCount;
         }
         ++it;
      }
   }

   if (uncontrolledRunningCount > 0) {
      return;
   }

   int maxDuration = 0;
   for (int i = 0; i < animations.size(); ++i) {
      maxDuration = qMax(maxDuration, animations.at(i)->totalDuration());
   }

   if (currentTime >= maxDuration) {
      q->stop();
   }
}

void QParallelAnimationGroupPrivate::disconnectUncontrolledAnimations()
{
   QHash<QAbstractAnimation *, int>::iterator it = uncontrolledFinishTime.begin();
   while (it != uncontrolledFinishTime.end()) {
      disconnectUncontrolledAnimation(it.key());
      ++it;
   }

   uncontrolledFinishTime.clear();
}

void QParallelAnimationGroupPrivate::connectUncontrolledAnimations()
{
   for (int i = 0; i < animations.size(); ++i) {
      QAbstractAnimation *animation = animations.at(i);
      if (animation->duration() == -1 || animation->loopCount() < 0) {
         uncontrolledFinishTime[animation] = -1;
         connectUncontrolledAnimation(animation);
      }
   }
}

bool QParallelAnimationGroupPrivate::shouldAnimationStart(QAbstractAnimation *animation, bool startIfAtEnd) const
{
   const int dura = animation->totalDuration();
   if (dura == -1) {
      return !isUncontrolledAnimationFinished(animation);
   }
   if (startIfAtEnd) {
      return currentTime <= dura;
   }
   if (direction == QAbstractAnimation::Forward) {
      return currentTime < dura;
   } else { //direction == QAbstractAnimation::Backward
      return currentTime && currentTime <= dura;
   }
}

void QParallelAnimationGroupPrivate::applyGroupState(QAbstractAnimation *animation)
{
   switch (state) {
      case QAbstractAnimation::Running:
         animation->start();
         break;
      case QAbstractAnimation::Paused:
         animation->pause();
         break;
      case QAbstractAnimation::Stopped:
      default:
         break;
   }
}


bool QParallelAnimationGroupPrivate::isUncontrolledAnimationFinished(QAbstractAnimation *anim) const
{
   return uncontrolledFinishTime.value(anim, -1) >= 0;
}

void QParallelAnimationGroupPrivate::animationRemoved(int index, QAbstractAnimation *anim)
{
   QAnimationGroupPrivate::animationRemoved(index, anim);
   disconnectUncontrolledAnimation(anim);
   uncontrolledFinishTime.remove(anim);
}

/*!
    \reimp
*/
void QParallelAnimationGroup::updateDirection(QAbstractAnimation::Direction direction)
{
   Q_D(QParallelAnimationGroup);
   //we need to update the direction of the current animation
   if (state() != Stopped) {
      for (int i = 0; i < d->animations.size(); ++i) {
         QAbstractAnimation *animation = d->animations.at(i);
         animation->setDirection(direction);
      }
   } else {
      if (direction == Forward) {
         d->lastLoop = 0;
         d->lastCurrentTime = 0;
      } else {
         // Looping backwards with loopCount == -1 does not really work well...
         d->lastLoop = (d->loopCount == -1 ? 0 : d->loopCount - 1);
         d->lastCurrentTime = duration();
      }
   }
}

/*!
    \reimp
*/
bool QParallelAnimationGroup::event(QEvent *event)
{
   return QAnimationGroup::event(event);
}

QT_END_NAMESPACE

#endif //QT_NO_ANIMATION
