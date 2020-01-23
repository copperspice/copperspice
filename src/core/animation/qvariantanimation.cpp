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

#include <qvariantanimation.h>

#include <qrect.h>
#include <qline.h>
#include <qmutex.h>

#include <qmutexpool_p.h>
#include <qvariantanimation_p.h>

#include <algorithm>

#ifndef QT_NO_ANIMATION

static bool animationValueLessThan(const QVariantAnimation::KeyValue &p1, const QVariantAnimation::KeyValue &p2)
{
   return p1.first < p2.first;
}

static QVariant defaultInterpolator(const void *, const void *, qreal)
{
   return QVariant();
}

template<>
inline QRect _q_interpolate(const QRect &f, const QRect &t, qreal progress)
{
   QRect ret;
   ret.setCoords(_q_interpolate(f.left(), t.left(), progress),
                 _q_interpolate(f.top(), t.top(), progress),
                 _q_interpolate(f.right(), t.right(), progress),
                 _q_interpolate(f.bottom(), t.bottom(), progress));
   return ret;
}

template<>
inline QRectF _q_interpolate(const QRectF &f, const QRectF &t, qreal progress)
{
   qreal x1, y1, w1, h1;
   f.getRect(&x1, &y1, &w1, &h1);
   qreal x2, y2, w2, h2;
   t.getRect(&x2, &y2, &w2, &h2);
   return QRectF(_q_interpolate(x1, x2, progress), _q_interpolate(y1, y2, progress),
                 _q_interpolate(w1, w2, progress), _q_interpolate(h1, h2, progress));
}

template<>
inline QLine _q_interpolate(const QLine &f, const QLine &t, qreal progress)
{
   return QLine( _q_interpolate(f.p1(), t.p1(), progress), _q_interpolate(f.p2(), t.p2(), progress));
}

template<>
inline QLineF _q_interpolate(const QLineF &f, const QLineF &t, qreal progress)
{
   return QLineF( _q_interpolate(f.p1(), t.p1(), progress), _q_interpolate(f.p2(), t.p2(), progress));
}

QVariantAnimationPrivate::QVariantAnimationPrivate() : duration(250), interpolator(&defaultInterpolator)
{ }

void QVariantAnimationPrivate::convertValues(int t)
{
   // ensures all the keyValues are of type t
   for (int i = 0; i < keyValues.count(); ++i) {
      QVariantAnimation::KeyValue &pair = keyValues[i];
      pair.second.convert(static_cast<QVariant::Type>(t));
   }

   // also need update to the current interval if needed
   currentInterval.start.second.convert(static_cast<QVariant::Type>(t));
   currentInterval.end.second.convert(static_cast<QVariant::Type>(t));

   updateInterpolator();
}

void QVariantAnimationPrivate::updateInterpolator()
{
   uint type = currentInterval.start.second.userType();

   if (type == currentInterval.end.second.userType()) {
      interpolator = getInterpolator(type);
   } else {
      interpolator = nullptr;
   }

   if (interpolator == nullptr) {
      interpolator = &defaultInterpolator;
   }
}

void QVariantAnimationPrivate::recalculateCurrentInterval(bool force/*=false*/)
{
   // can not interpolate if we don't have at least 2 values
   if ((keyValues.count() + (defaultStartEndValue.isValid() ? 1 : 0)) < 2) {
      return;
   }

   const qreal endProgress = (direction == QAbstractAnimation::Forward) ? qreal(1) : qreal(0);
   const qreal progress = easing.valueForProgress(((duration == 0) ? endProgress : qreal(currentTime) / qreal(duration)));

   //0 and 1 are still the boundaries
   if (force || (currentInterval.start.first > 0 && progress < currentInterval.start.first)
         || (currentInterval.end.first < 1 && progress > currentInterval.end.first)) {

      //let's update currentInterval
      QVariantAnimation::KeyValues::const_iterator it = std::lower_bound(keyValues.constBegin(),
            keyValues.constEnd(),
            qMakePair(progress, QVariant()),
            animationValueLessThan);

      if (it == keyValues.constBegin()) {
         //the item pointed to by it is the start element in the range
         if (it->first == 0 && keyValues.count() > 1) {
            currentInterval.start = *it;
            currentInterval.end = *(it + 1);
         } else {
            currentInterval.start = qMakePair(qreal(0), defaultStartEndValue);
            currentInterval.end = *it;
         }
      } else if (it == keyValues.constEnd()) {
         --it; //position the iterator on the last item
         if (it->first == 1 && keyValues.count() > 1) {
            //we have an end value (item with progress = 1)
            currentInterval.start = *(it - 1);
            currentInterval.end = *it;
         } else {
            //we use the default end value here
            currentInterval.start = *it;
            currentInterval.end = qMakePair(qreal(1), defaultStartEndValue);
         }
      } else {
         currentInterval.start = *(it - 1);
         currentInterval.end = *it;
      }

      // update all the values of the currentInterval
      updateInterpolator();
   }
   setCurrentValueForProgress(progress);
}

void QVariantAnimationPrivate::setCurrentValueForProgress(const qreal progress)
{
   Q_Q(QVariantAnimation);

   const qreal startProgress = currentInterval.start.first;
   const qreal endProgress   = currentInterval.end.first;
   const qreal localProgress = (progress - startProgress) / (endProgress - startProgress);

   QVariant ret = q->interpolated(currentInterval.start.second, currentInterval.end.second, localProgress);
   qSwap(currentValue, ret);
   q->updateCurrentValue(currentValue);

   emit q->valueChanged(currentValue);
}

QVariant QVariantAnimationPrivate::valueAt(qreal step) const
{
   QVariantAnimation::KeyValues::const_iterator result =
      std::lower_bound(keyValues.constBegin(), keyValues.constEnd(), qMakePair(step, QVariant()), animationValueLessThan);

   if (result != keyValues.constEnd() && ! animationValueLessThan(qMakePair(step, QVariant()), *result)) {
      return result->second;
   }

   return QVariant();
}

void QVariantAnimationPrivate::setValueAt(qreal step, const QVariant &value)
{
   if (step < qreal(0.0) || step > qreal(1.0)) {
      qWarning("QVariantAnimation::setValueAt: invalid step = %f", step);
      return;
   }

   QVariantAnimation::KeyValue pair(step, value);

   QVariantAnimation::KeyValues::iterator result = std::lower_bound(keyValues.begin(), keyValues.end(),
                  pair, animationValueLessThan);

   if (result == keyValues.end() || result->first != step) {
      keyValues.insert(result, pair);

   } else {
      if (value.isValid()) {
         result->second = value;    // replaces the previous value
      } else {
         keyValues.erase(result);   // removes the previous value
      }
   }

   recalculateCurrentInterval(/*force=*/true);
}

void QVariantAnimationPrivate::setDefaultStartEndValue(const QVariant &value)
{
   defaultStartEndValue = value;
   recalculateCurrentInterval(/*force=*/true);
}

QVariantAnimation::QVariantAnimation(QObject *parent) : QAbstractAnimation(*new QVariantAnimationPrivate, parent)
{
}

QVariantAnimation::QVariantAnimation(QVariantAnimationPrivate &dd, QObject *parent) : QAbstractAnimation(dd, parent)
{
}

QVariantAnimation::~QVariantAnimation()
{
}

QEasingCurve QVariantAnimation::easingCurve() const
{
   Q_D(const QVariantAnimation);
   return d->easing;
}

void QVariantAnimation::setEasingCurve(const QEasingCurve &easing)
{
   Q_D(QVariantAnimation);
   d->easing = easing;
   d->recalculateCurrentInterval();
}

typedef QVector<QVariantAnimation::Interpolator> QInterpolatorVector;
Q_GLOBAL_STATIC(QInterpolatorVector, registeredInterpolators)

void QVariantAnimation::registerInterpolator(QVariantAnimation::Interpolator func, int interpolationType)
{
   // will override any existing interpolators
   QInterpolatorVector *interpolators = registeredInterpolators();

   // When built on solaris with GCC, the destructors can be called
   // in such an order that we get here with interpolators == NULL,
   // to continue causes the app to crash on exit with a SEGV

   if (interpolators != nullptr) {

      QMutexLocker locker(QMutexPool::globalInstanceGet(interpolators));

      if (int(interpolationType) >= interpolators->count()) {
         interpolators->resize(int(interpolationType) + 1);
      }
      interpolators->replace(interpolationType, func);
   }
}


template<typename T> static inline QVariantAnimation::Interpolator castToInterpolator(QVariant (*func)(const T &from,
      const T &to, qreal progress))
{
   return reinterpret_cast<QVariantAnimation::Interpolator>(func);
}

QVariantAnimation::Interpolator QVariantAnimationPrivate::getInterpolator(int interpolationType)
{
   QInterpolatorVector *interpolators = registeredInterpolators();

   QMutexLocker locker(QMutexPool::globalInstanceGet(interpolators));

   QVariantAnimation::Interpolator ret = 0;
   if (interpolationType < interpolators->count()) {
      ret = interpolators->at(interpolationType);
      if (ret) {
         return ret;
      }
   }

   switch (interpolationType) {
      case QMetaType::Int:
         return castToInterpolator(_q_interpolateVariant<int>);
      case QMetaType::UInt:
         return castToInterpolator(_q_interpolateVariant<uint>);
      case QMetaType::Double:
         return castToInterpolator(_q_interpolateVariant<double>);
      case QMetaType::Float:
         return castToInterpolator(_q_interpolateVariant<float>);
      case QMetaType::QLine:
         return castToInterpolator(_q_interpolateVariant<QLine>);
      case QMetaType::QLineF:
         return castToInterpolator(_q_interpolateVariant<QLineF>);
      case QMetaType::QPoint:
         return castToInterpolator(_q_interpolateVariant<QPoint>);
      case QMetaType::QPointF:
         return castToInterpolator(_q_interpolateVariant<QPointF>);
      case QMetaType::QSize:
         return castToInterpolator(_q_interpolateVariant<QSize>);
      case QMetaType::QSizeF:
         return castToInterpolator(_q_interpolateVariant<QSizeF>);
      case QMetaType::QRect:
         return castToInterpolator(_q_interpolateVariant<QRect>);
      case QMetaType::QRectF:
         return castToInterpolator(_q_interpolateVariant<QRectF>);
      default:
         return nullptr;
   }
}

int QVariantAnimation::duration() const
{
   Q_D(const QVariantAnimation);
   return d->duration;
}

void QVariantAnimation::setDuration(int msecs)
{
   Q_D(QVariantAnimation);

   if (msecs < 0) {
      qWarning("QVariantAnimation::setDuration: Duration can not be negative");
      return;
   }

   if (d->duration == msecs) {
      return;
   }

   d->duration = msecs;
   d->recalculateCurrentInterval();
}

QVariant QVariantAnimation::startValue() const
{
   return keyValueAt(0);
}

void QVariantAnimation::setStartValue(const QVariant &value)
{
   setKeyValueAt(0, value);
}

QVariant QVariantAnimation::endValue() const
{
   return keyValueAt(1);
}

void QVariantAnimation::setEndValue(const QVariant &value)
{
   setKeyValueAt(1, value);
}

QVariant QVariantAnimation::keyValueAt(qreal step) const
{
   return d_func()->valueAt(step);
}

void QVariantAnimation::setKeyValueAt(qreal step, const QVariant &value)
{
   d_func()->setValueAt(step, value);
}

QVariantAnimation::KeyValues QVariantAnimation::keyValues() const
{
   return d_func()->keyValues;
}

void QVariantAnimation::setKeyValues(const KeyValues &keyValues)
{
   Q_D(QVariantAnimation);

   d->keyValues = keyValues;
   std::sort(d->keyValues.begin(), d->keyValues.end(), animationValueLessThan);
   d->recalculateCurrentInterval(true);
}

QVariant QVariantAnimation::currentValue() const
{
   Q_D(const QVariantAnimation);

   if (! d->currentValue.isValid()) {
      const_cast<QVariantAnimationPrivate *>(d)->recalculateCurrentInterval();
   }

   return d->currentValue;
}

bool QVariantAnimation::event(QEvent *event)
{
   return QAbstractAnimation::event(event);
}

void QVariantAnimation::updateState(QAbstractAnimation::State, QAbstractAnimation::State)
{
}

QVariant QVariantAnimation::interpolated(const QVariant &from, const QVariant &to, qreal progress) const
{
   return d_func()->interpolator(from.constData(), to.constData(), progress);
}

void QVariantAnimation::updateCurrentTime(int)
{
   d_func()->recalculateCurrentInterval();
}

#endif
