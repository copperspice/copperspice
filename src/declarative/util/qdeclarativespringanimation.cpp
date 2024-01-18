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

#include <qdeclarativespringanimation_p.h>
#include <qdeclarativeanimation_p_p.h>
#include <qdeclarativeproperty_p.h>
#include <QtCore/qdebug.h>
#include <limits.h>
#include <math.h>

QT_BEGIN_NAMESPACE

class QDeclarativeSpringAnimationPrivate : public QDeclarativePropertyAnimationPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeSpringAnimation)

 public:

   struct SpringAnimation {
      SpringAnimation()
         : currentValue(0), to(0), velocity(0), start(0), duration(0) {}
      qreal currentValue;
      qreal to;
      qreal velocity;
      int start;
      int duration;
   };
   QHash<QDeclarativeProperty, SpringAnimation> activeAnimations;

   qreal maxVelocity;
   qreal velocityms;
   int lastTime;
   qreal mass;
   qreal spring;
   qreal damping;
   qreal epsilon;
   qreal modulus;

   bool useMass : 1;
   bool haveModulus : 1;

   enum Mode {
      Track,
      Velocity,
      Spring
   };
   Mode mode;

   QDeclarativeSpringAnimationPrivate()
      : maxVelocity(0), velocityms(0), lastTime(0)
      , mass(1.0), spring(0.), damping(0.), epsilon(0.01)
      , modulus(0.0), useMass(false), haveModulus(false)
      , mode(Track), clock(0) {
   }

   void tick(int time);
   bool animate(const QDeclarativeProperty &property, SpringAnimation &animation, int elapsed);
   void updateMode();

   typedef QTickAnimationProxy<QDeclarativeSpringAnimationPrivate, &QDeclarativeSpringAnimationPrivate::tick> Clock;
   Clock *clock;
};

void QDeclarativeSpringAnimationPrivate::tick(int time)
{
   if (mode == Track) {
      clock->stop();
      return;
   }
   int elapsed = time - lastTime;
   if (!elapsed) {
      return;
   }

   if (mode == Spring) {
      if (elapsed < 16) { // capped at 62fps.
         return;
      }
      int count = elapsed / 16;
      lastTime = time - (elapsed - count * 16);
   } else {
      lastTime = time;
   }

   QMutableHashIterator<QDeclarativeProperty, SpringAnimation> it(activeAnimations);
   while (it.hasNext()) {
      it.next();
      if (animate(it.key(), it.value(), elapsed)) {
         it.remove();
      }
   }

   if (activeAnimations.isEmpty()) {
      clock->stop();
   }
}

bool QDeclarativeSpringAnimationPrivate::animate(const QDeclarativeProperty &property, SpringAnimation &animation,
      int elapsed)
{
   qreal srcVal = animation.to;

   bool stop = false;

   if (haveModulus) {
      animation.currentValue = fmod(animation.currentValue, modulus);
      srcVal = fmod(srcVal, modulus);
   }
   if (mode == Spring) {
      // Real men solve the spring DEs using RK4.
      // We'll do something much simpler which gives a result that looks fine.
      int count = elapsed / 16;
      for (int i = 0; i < count; ++i) {
         qreal diff = srcVal - animation.currentValue;
         if (haveModulus && qAbs(diff) > modulus / 2) {
            if (diff < 0) {
               diff += modulus;
            } else {
               diff -= modulus;
            }
         }
         if (useMass) {
            animation.velocity = animation.velocity + (spring * diff - damping * animation.velocity) / mass;
         } else {
            animation.velocity = animation.velocity + spring * diff - damping * animation.velocity;
         }
         if (maxVelocity > qreal(0.)) {
            // limit velocity
            if (animation.velocity > maxVelocity) {
               animation.velocity = maxVelocity;
            } else if (animation.velocity < -maxVelocity) {
               animation.velocity = -maxVelocity;
            }
         }
         animation.currentValue += animation.velocity * qreal(16.0) / qreal(1000.0);
         if (haveModulus) {
            animation.currentValue = fmod(animation.currentValue, modulus);
            if (animation.currentValue < qreal(0.0)) {
               animation.currentValue += modulus;
            }
         }
      }
      if (qAbs(animation.velocity) < epsilon && qAbs(srcVal - animation.currentValue) < epsilon) {
         animation.velocity = 0.0;
         animation.currentValue = srcVal;
         stop = true;
      }
   } else {
      qreal moveBy = elapsed * velocityms;
      qreal diff = srcVal - animation.currentValue;
      if (haveModulus && qAbs(diff) > modulus / 2) {
         if (diff < 0) {
            diff += modulus;
         } else {
            diff -= modulus;
         }
      }
      if (diff > 0) {
         animation.currentValue += moveBy;
         if (haveModulus) {
            animation.currentValue = fmod(animation.currentValue, modulus);
         }
      } else {
         animation.currentValue -= moveBy;
         if (haveModulus && animation.currentValue < qreal(0.0)) {
            animation.currentValue = fmod(animation.currentValue, modulus) + modulus;
         }
      }
      if (lastTime - animation.start >= animation.duration) {
         animation.currentValue = animation.to;
         stop = true;
      }
   }

   qreal old_to = animation.to;

   QDeclarativePropertyPrivate::write(property, animation.currentValue,
                                      QDeclarativePropertyPrivate::BypassInterceptor |
                                      QDeclarativePropertyPrivate::DontRemoveBinding);

   return (stop && old_to == animation.to); // do not stop if we got restarted
}

void QDeclarativeSpringAnimationPrivate::updateMode()
{
   if (spring == 0. && maxVelocity == 0.) {
      mode = Track;
   } else if (spring > 0.) {
      mode = Spring;
   } else {
      mode = Velocity;
      QHash<QDeclarativeProperty, SpringAnimation>::iterator it;
      for (it = activeAnimations.begin(); it != activeAnimations.end(); ++it) {
         SpringAnimation &animation = *it;
         animation.start = lastTime;
         qreal dist = qAbs(animation.currentValue - animation.to);
         if (haveModulus && dist > modulus / 2) {
            dist = modulus - fmod(dist, modulus);
         }
         animation.duration = dist / velocityms;
      }
   }
}

/*!
    \qmlclass SpringAnimation QDeclarativeSpringAnimation
    \ingroup qml-animation-transition
    \inherits NumberAnimation
    \since 4.7

    \brief The SpringAnimation element allows a property to track a value in a spring-like motion.

    SpringAnimation mimics the oscillatory behavior of a spring, with the appropriate \l spring constant to
    control the acceleration and the \l damping to control how quickly the effect dies away.

    You can also limit the maximum \l velocity of the animation.

    The following \l Rectangle moves to the position of the mouse using a
    SpringAnimation when the mouse is clicked. The use of the \l Behavior
    on the \c x and \c y values indicates that whenever these values are
    changed, a SpringAnimation should be applied.

    \snippet doc/src/snippets/declarative/springanimation.qml 0

    Like any other animation element, a SpringAnimation can be applied in a
    number of ways, including transitions, behaviors and property value
    sources. The \l {QML Animation and Transitions} documentation shows a
    variety of methods for creating animations.

    \sa SmoothedAnimation, {QML Animation and Transitions}, {declarative/animation/basics}{Animation basics example}, {declarative/toys/clocks}{Clocks example}
*/

QDeclarativeSpringAnimation::QDeclarativeSpringAnimation(QObject *parent)
   : QDeclarativeNumberAnimation(*(new QDeclarativeSpringAnimationPrivate), parent)
{
   Q_D(QDeclarativeSpringAnimation);
   d->clock = new QDeclarativeSpringAnimationPrivate::Clock(d, this);
}

QDeclarativeSpringAnimation::~QDeclarativeSpringAnimation()
{
}

/*!
    \qmlproperty real SpringAnimation::velocity

    This property holds the maximum velocity allowed when tracking the source.

    The default value is 0 (no maximum velocity).
*/

qreal QDeclarativeSpringAnimation::velocity() const
{
   Q_D(const QDeclarativeSpringAnimation);
   return d->maxVelocity;
}

void QDeclarativeSpringAnimation::setVelocity(qreal velocity)
{
   Q_D(QDeclarativeSpringAnimation);
   d->maxVelocity = velocity;
   d->velocityms = velocity / 1000.0;
   d->updateMode();
}

/*!
    \qmlproperty real SpringAnimation::spring

    This property describes how strongly the target is pulled towards the
    source. The default value is 0 (that is, the spring-like motion is disabled).

    The useful value range is 0 - 5.0.

    When this property is set and the \l velocity value is greater than 0,
    the \l velocity limits the maximum speed.
*/
qreal QDeclarativeSpringAnimation::spring() const
{
   Q_D(const QDeclarativeSpringAnimation);
   return d->spring;
}

void QDeclarativeSpringAnimation::setSpring(qreal spring)
{
   Q_D(QDeclarativeSpringAnimation);
   d->spring = spring;
   d->updateMode();
}

/*!
    \qmlproperty real SpringAnimation::damping
    This property holds the spring damping value.

    This value describes how quickly the spring-like motion comes to rest.
    The default value is 0.

    The useful value range is 0 - 1.0. The lower the value, the faster it
    comes to rest.
*/
qreal QDeclarativeSpringAnimation::damping() const
{
   Q_D(const QDeclarativeSpringAnimation);
   return d->damping;
}

void QDeclarativeSpringAnimation::setDamping(qreal damping)
{
   Q_D(QDeclarativeSpringAnimation);
   if (damping > 1.) {
      damping = 1.;
   }

   d->damping = damping;
}


/*!
    \qmlproperty real SpringAnimation::epsilon
    This property holds the spring epsilon.

    The epsilon is the rate and amount of change in the value which is close enough
    to 0 to be considered equal to zero. This will depend on the usage of the value.
    For pixel positions, 0.25 would suffice. For scale, 0.005 will suffice.

    The default is 0.01. Tuning this value can provide small performance improvements.
*/
qreal QDeclarativeSpringAnimation::epsilon() const
{
   Q_D(const QDeclarativeSpringAnimation);
   return d->epsilon;
}

void QDeclarativeSpringAnimation::setEpsilon(qreal epsilon)
{
   Q_D(QDeclarativeSpringAnimation);
   d->epsilon = epsilon;
}

/*!
    \qmlproperty real SpringAnimation::modulus
    This property holds the modulus value. The default value is 0.

    Setting a \a modulus forces the target value to "wrap around" at the modulus.
    For example, setting the modulus to 360 will cause a value of 370 to wrap around to 10.
*/
qreal QDeclarativeSpringAnimation::modulus() const
{
   Q_D(const QDeclarativeSpringAnimation);
   return d->modulus;
}

void QDeclarativeSpringAnimation::setModulus(qreal modulus)
{
   Q_D(QDeclarativeSpringAnimation);
   if (d->modulus != modulus) {
      d->haveModulus = modulus != 0.0;
      d->modulus = modulus;
      d->updateMode();
      emit modulusChanged();
   }
}

/*!
    \qmlproperty real SpringAnimation::mass
    This property holds the "mass" of the property being moved.

    The value is 1.0 by default.

    A greater mass causes slower movement and a greater spring-like
    motion when an item comes to rest.
*/
qreal QDeclarativeSpringAnimation::mass() const
{
   Q_D(const QDeclarativeSpringAnimation);
   return d->mass;
}

void QDeclarativeSpringAnimation::setMass(qreal mass)
{
   Q_D(QDeclarativeSpringAnimation);
   if (d->mass != mass && mass > 0.0) {
      d->useMass = mass != 1.0;
      d->mass = mass;
      emit massChanged();
   }
}

void QDeclarativeSpringAnimation::transition(QDeclarativeStateActions &actions,
      QDeclarativeProperties &modified,
      TransitionDirection direction)
{
   Q_D(QDeclarativeSpringAnimation);
   Q_UNUSED(direction);

   if (d->clock->state() != QAbstractAnimation::Running) {
      d->lastTime = 0;
   }

   QDeclarativeNumberAnimation::transition(actions, modified, direction);

   if (!d->actions) {
      return;
   }

   if (!d->actions->isEmpty()) {
      for (int i = 0; i < d->actions->size(); ++i) {
         const QDeclarativeProperty &property = d->actions->at(i).property;
         QDeclarativeSpringAnimationPrivate::SpringAnimation &animation
            = d->activeAnimations[property];
         animation.to = d->actions->at(i).toValue.toReal();
         animation.start = d->lastTime;
         if (d->fromIsDefined) {
            animation.currentValue = d->actions->at(i).fromValue.toReal();
         } else {
            animation.currentValue = property.read().toReal();
         }
         if (d->mode == QDeclarativeSpringAnimationPrivate::Velocity) {
            qreal dist = qAbs(animation.currentValue - animation.to);
            if (d->haveModulus && dist > d->modulus / 2) {
               dist = d->modulus - fmod(dist, d->modulus);
            }
            animation.duration = dist / d->velocityms;
         }
      }
   }
}


QAbstractAnimation *QDeclarativeSpringAnimation::qtAnimation()
{
   Q_D(QDeclarativeSpringAnimation);
   return d->clock;
}

QT_END_NAMESPACE
