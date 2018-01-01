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

#include <qpropertyanimation.h>
#include <qanimationgroup.h>
#include <qpropertyanimation_p.h>
#include <qmutexpool_p.h>
#include <qhash.h>

#ifndef QT_NO_ANIMATION

QT_BEGIN_NAMESPACE

void QPropertyAnimationPrivate::updateMetaProperty()
{
   if (! target || propertyName.isEmpty()) {
      propertyType = QVariant::Invalid;
      propertyIndex = -1;
      return;
   }

   // propertyType will be set to a valid type only if there is a Q_PROPERTY
   // otherwise it will be set to QVariant::Invalid at the end of this function

   propertyType  = targetValue->property(propertyName).userType();
   propertyIndex = targetValue->metaObject()->indexOfProperty(propertyName.constData());

   if (propertyType != QVariant::Invalid) {
      convertValues(propertyType);
   }

   if (propertyIndex == -1) {
      //there is no Q_PROPERTY on the object
      propertyType = QVariant::Invalid;
      if (!targetValue->dynamicPropertyNames().contains(propertyName)) {
         qWarning("QPropertyAnimation: Trying to animate a non-existant property %s",
                  propertyName.constData());
      }

   } else if (!targetValue->metaObject()->property(propertyIndex).isWritable()) {
      qWarning("QPropertyAnimation: Trying to animate a read only property %s",
               propertyName.constData());
   }
}

void QPropertyAnimationPrivate::updateProperty(const QVariant &newValue)
{
   if (state == QAbstractAnimation::Stopped) {
      return;
   }

   if (!target) {
      q_func()->stop(); //the target was destroyed we need to stop the animation
      return;
   }

   targetValue->setProperty(propertyName.constData(), newValue);
}

/*!
    Construct a QPropertyAnimation object. \a parent is passed to QObject's
    constructor.
*/
QPropertyAnimation::QPropertyAnimation(QObject *parent)
   : QVariantAnimation(*new QPropertyAnimationPrivate, parent)
{
}

/*!
    Construct a QPropertyAnimation object. \a parent is passed to QObject's
    constructor. The animation changes the property \a propertyName on \a
    target. The default duration is 250ms.

    \sa targetObject, propertyName
*/
QPropertyAnimation::QPropertyAnimation(QObject *target, const QByteArray &propertyName, QObject *parent)
   : QVariantAnimation(*new QPropertyAnimationPrivate, parent)
{
   setTargetObject(target);
   setPropertyName(propertyName);
}

/*!
    Destroys the QPropertyAnimation instance.
 */
QPropertyAnimation::~QPropertyAnimation()
{
   stop();
}

/*!
    \property QPropertyAnimation::targetObject
    \brief the target QObject for this animation.

    This property defines the target QObject for this animation.
 */
QObject *QPropertyAnimation::targetObject() const
{
   return d_func()->target.data();
}

void QPropertyAnimation::setTargetObject(QObject *target)
{
   Q_D(QPropertyAnimation);
   if (d->targetValue == target) {
      return;
   }

   if (d->state != QAbstractAnimation::Stopped) {
      qWarning("QPropertyAnimation::setTargetObject: you can't change the target of a running animation");
      return;
   }

   d->target = d->targetValue = target;
   d->updateMetaProperty();
}

/*!
    \property QPropertyAnimation::propertyName
    \brief the target property name for this animation

    This property defines the target property name for this animation. The
    property name is required for the animation to operate.
 */
QByteArray QPropertyAnimation::propertyName() const
{
   Q_D(const QPropertyAnimation);
   return d->propertyName;
}

void QPropertyAnimation::setPropertyName(const QByteArray &propertyName)
{
   Q_D(QPropertyAnimation);
   if (d->state != QAbstractAnimation::Stopped) {
      qWarning("QPropertyAnimation::setPropertyName: you can't change the property name of a running animation");
      return;
   }

   d->propertyName = propertyName;
   d->updateMetaProperty();
}


/*!
    \reimp
 */
bool QPropertyAnimation::event(QEvent *event)
{
   return QVariantAnimation::event(event);
}

/*!
    This virtual function is called by QVariantAnimation whenever the current value
    changes. \a value is the new, updated value. It updates the current value
    of the property on the target object.

    \sa currentValue, currentTime
 */
void QPropertyAnimation::updateCurrentValue(const QVariant &value)
{
   Q_D(QPropertyAnimation);
   d->updateProperty(value);
}

/*!
    \reimp

    If the startValue is not defined when the state of the animation changes from Stopped to Running,
    the current property value is used as the initial value for the animation.
*/
void QPropertyAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
   Q_D(QPropertyAnimation);

   if (!d->target && oldState == Stopped) {
      qWarning("QPropertyAnimation::updateState (%s): Changing state of an animation without target",
               d->propertyName.constData());
      return;
   }

   QVariantAnimation::updateState(newState, oldState);

   QPropertyAnimation *animToStop = 0;
   {
      QMutexLocker locker(QMutexPool::globalInstanceGet( &staticMetaObject() ));

      typedef QPair<QObject *, QByteArray> QPropertyAnimationPair;
      typedef QHash<QPropertyAnimationPair, QPropertyAnimation *> QPropertyAnimationHash;

      static QPropertyAnimationHash hash;

      //here we need to use value because we need to know to which pointer
      //the animation was referring in case stopped because the target was destroyed

      QPropertyAnimationPair key(d->targetValue, d->propertyName);
      if (newState == Running) {
         d->updateMetaProperty();
         animToStop = hash.value(key, 0);
         hash.insert(key, this);

         // update the default start value
         if (oldState == Stopped) {
            d->setDefaultStartEndValue(d->targetValue->property(d->propertyName.constData()));

            //let's check if we have a start value and an end value
            if (!startValue().isValid() && (d->direction == Backward || !d->defaultStartEndValue.isValid())) {
               qWarning("QPropertyAnimation::updateState (%s, %s, %s): starting an animation without start value",
                        d->propertyName.constData(), d->target.data()->metaObject()->className(),
                        qPrintable(d->target.data()->objectName()));
            }

            if (!endValue().isValid() && (d->direction == Forward || !d->defaultStartEndValue.isValid())) {
               qWarning("QPropertyAnimation::updateState (%s, %s, %s): starting an animation without end value",
                        d->propertyName.constData(), d->target.data()->metaObject()->className(),
                        qPrintable(d->target.data()->objectName()));
            }
         }
      } else if (hash.value(key) == this) {
         hash.remove(key);
      }
   }

   //we need to do that after the mutex was unlocked
   if (animToStop) {
      // try to stop the top level group
      QAbstractAnimation *current = animToStop;
      while (current->group() && current->state() != Stopped) {
         current = current->group();
      }
      current->stop();
   }
}

QT_END_NAMESPACE

#endif //QT_NO_ANIMATION
