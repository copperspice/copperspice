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

#include <qpropertyanimation.h>
#include <qpropertyanimation_p.h>

#include <qanimationgroup.h>
#include <qhash.h>

#include <qmutexpool_p.h>

#ifndef QT_NO_ANIMATION

void QPropertyAnimationPrivate::updateMetaProperty()
{
   if (! target || propertyName.isEmpty()) {
      propertyType  = QVariant::Invalid;
      propertyIndex = -1;
      return;
   }

   // propertyType will be set to a valid type only if there is a Q_PROPERTY
   // otherwise it will be set to QVariant::Invalid at the end of this function

   propertyType  = targetValue->property(propertyName).userType();
   propertyIndex = targetValue->metaObject()->indexOfProperty(propertyName);

   if (propertyType != QVariant::Invalid) {
      convertValues(propertyType);
   }

   if (propertyIndex == -1) {
      // there is no PROPERTY on the object
      propertyType = QVariant::Invalid;

      if (! targetValue->dynamicPropertyNames().contains(propertyName)) {
         qWarning("QPropertyAnimationPrivate::updateMetaProperty() Trying to animate a non existent property %s", csPrintable(propertyName));
      }

   } else if (! targetValue->metaObject()->property(propertyIndex).isWritable()) {
      qWarning("QPropertyAnimationPrivate::updateMetaProperty() Trying to animate a read only property %s", csPrintable(propertyName));
   }
}

void QPropertyAnimationPrivate::updateProperty(const QVariant &newValue)
{
   if (state == QAbstractAnimation::Stopped) {
      return;
   }

   if (! target) {
      // target was destroyed, need to stop the animation
      q_func()->stop();
      return;
   }

   targetValue->setProperty(propertyName, newValue);
}

QPropertyAnimation::QPropertyAnimation(QObject *parent)
   : QVariantAnimation(*new QPropertyAnimationPrivate, parent)
{
}

QPropertyAnimation::QPropertyAnimation(QObject *target, const QString &propertyName, QObject *parent)
   : QVariantAnimation(*new QPropertyAnimationPrivate, parent)
{
   setTargetObject(target);
   setPropertyName(propertyName);
}

QPropertyAnimation::~QPropertyAnimation()
{
   stop();
}

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
      qWarning("QPropertyAnimation::setTargetObject() Unable to change the target of a running animation");
      return;
   }

   d->target      = target;
   d->targetValue = target;

   d->updateMetaProperty();
}

QString QPropertyAnimation::propertyName() const
{
   Q_D(const QPropertyAnimation);
   return d->propertyName;
}

void QPropertyAnimation::setPropertyName(const QString &propertyName)
{
   Q_D(QPropertyAnimation);

   if (d->state != QAbstractAnimation::Stopped) {
      qWarning("QPropertyAnimation::setPropertyName() Unable to change the property name of a running animation");
      return;
   }

   d->propertyName = propertyName;
   d->updateMetaProperty();
}

bool QPropertyAnimation::event(QEvent *event)
{
   return QVariantAnimation::event(event);
}

void QPropertyAnimation::updateCurrentValue(const QVariant &value)
{
   Q_D(QPropertyAnimation);
   d->updateProperty(value);
}

void QPropertyAnimation::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
   Q_D(QPropertyAnimation);

   if (! d->target && oldState == Stopped) {
      qWarning("QPropertyAnimation::updateState() Changing state of an animation without a target, %s",
            d->propertyName.constData());
      return;
   }

   QVariantAnimation::updateState(newState, oldState);
   QPropertyAnimation *animToStop = nullptr;

   {
      QRecursiveMutexLocker locker(QMutexPool::globalInstanceGet( &staticMetaObject() ));

      using QPropertyAnimationPair = QPair<QObject *, QString>;
      using QPropertyAnimationHash = QHash<QPropertyAnimationPair, QPropertyAnimation *>;

      static QPropertyAnimationHash hash;
      QPropertyAnimationPair key(d->targetValue, d->propertyName);

      if (newState == Running) {
         d->updateMetaProperty();

         animToStop = hash.value(key, nullptr);
         hash.insert(key, this);

         // update the default start value
         if (oldState == Stopped) {

            // if startValue is not defined, current property value is used as the initial value for the animation
            d->setDefaultStartEndValue(d->targetValue->property(d->propertyName));

            // check if start and end values exist
            if (! startValue().isValid() && (d->direction == Backward || ! d->m_defaultValue.isValid())) {

               qWarning("QPropertyAnimation::updateState (%s, %s, %s): Trying to start an animation, no start value available",
                        csPrintable(d->propertyName), csPrintable(d->target.data()->metaObject()->className()),
                        csPrintable(d->target.data()->objectName()));
            }

            if (! endValue().isValid() && (d->direction == Forward || ! d->m_defaultValue.isValid())) {

               qWarning("QPropertyAnimation::updateState (%s, %s, %s): Trying to start an animation, no end value available",
                        csPrintable(d->propertyName), csPrintable(d->target.data()->metaObject()->className()),
                        csPrintable(d->target.data()->objectName()));
            }
         }

      } else if (hash.value(key) == this) {
         hash.remove(key);
      }
   }

   // need to do after the mutex is unlocked
   if (animToStop) {
      // try to stop the top level group
      QAbstractAnimation *current = animToStop;

      while (current->group() && current->state() != Stopped) {
         current = current->group();
      }

      current->stop();
   }
}

#endif //QT_NO_ANIMATION
