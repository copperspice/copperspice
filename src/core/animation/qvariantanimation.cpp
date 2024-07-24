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

#include <qvariantanimation.h>
#include <qvariantanimation_p.h>

#include <qline.h>
#include <qmutex.h>
#include <qrect.h>

#include <qmutexpool_p.h>

#include <algorithm>

#ifndef QT_NO_ANIMATION

libguarded::shared_guarded<QHash<uint, QVariantAnimation::CustomFormula>> &QVariantAnimation::getFormulas()
{
   static libguarded::shared_guarded<QHash<uint, QVariantAnimation::CustomFormula>> retval;
   return retval;
}

static bool animationValueLessThan(const QVariantAnimation::ValuePair &p1, const QVariantAnimation::ValuePair &p2)
{
   return p1.first < p2.first;
}

static QVariant cs_defaultFormula(const QVariant &, const QVariant &, double)
{
   return QVariant();
}

template <>
inline QRect cs_genericFormula(const QRect &from, const QRect &to, double progress)
{
   QRect retval;

   retval.setCoords(cs_genericFormula(from.left(), to.left(), progress),
         cs_genericFormula(from.top(),    to.top(),    progress),
         cs_genericFormula(from.right(),  to.right(),  progress),
         cs_genericFormula(from.bottom(), to.bottom(), progress));

   return retval;
}

template <>
inline QRectF cs_genericFormula(const QRectF &from, const QRectF &to, double progress)
{
   double x1, y1, w1, h1;
   from.getRect(&x1, &y1, &w1, &h1);

   double x2, y2, w2, h2;
   to.getRect(&x2, &y2, &w2, &h2);

   return QRectF(cs_genericFormula(x1, x2, progress), cs_genericFormula(y1, y2, progress),
         cs_genericFormula(w1, w2, progress), cs_genericFormula(h1, h2, progress));
}

template <>
inline QLine cs_genericFormula(const QLine &from, const QLine &to, double progress)
{
   return QLine( cs_genericFormula(from.p1(), to.p1(), progress), cs_genericFormula(from.p2(), to.p2(), progress));
}

template <>
inline QLineF cs_genericFormula(const QLineF &from, const QLineF &to, double progress)
{
   return QLineF(cs_genericFormula(from.p1(), to.p1(), progress), cs_genericFormula(from.p2(), to.p2(), progress));
}

QVariantAnimationPrivate::QVariantAnimationPrivate()
   : m_duration(250), m_callBack(&cs_defaultFormula)
{ }

void QVariantAnimationPrivate::convertValues(uint typeId)
{
   // ensures all the values are of typeId
   for (auto &item : m_keyValues) {
      item.second.convert(static_cast<QVariant::Type>(typeId));
   }

   // also need update to the current interval if needed
   m_currentInterval.start.second.convert(static_cast<QVariant::Type>(typeId));
   m_currentInterval.end.second.convert(static_cast<QVariant::Type>(typeId));

   cs_updateCustomType();
}

void QVariantAnimationPrivate::cs_updateCustomType()
{
   uint type = m_currentInterval.start.second.userType();

   if (type == m_currentInterval.end.second.userType()) {
      m_callBack = cs_getCustomType(type);
   } else {
      m_callBack = nullptr;
   }

   if (m_callBack == nullptr) {
      m_callBack = &cs_defaultFormula;
   }
}

void QVariantAnimationPrivate::recalculateCurrentInterval(bool force)
{
   // must have at least 2 values
   int tmp = 0;

   if (m_defaultValue.isValid()) {
      tmp = 1;
   }

   if ((m_keyValues.count() + tmp) < 2) {
      return;
   }

   const double endProgress = (direction == QAbstractAnimation::Forward) ? double(1) : double(0);
   const double progress    = m_easing.valueForProgress(((m_duration == 0) ? endProgress : double(currentTime) / double(m_duration)));

   // 0 and 1 are still the boundaries
   if (force || (m_currentInterval.start.first > 0 && progress < m_currentInterval.start.first)
         || (m_currentInterval.end.first < 1 && progress > m_currentInterval.end.first)) {

      // update currentInterval
      auto iter = std::lower_bound(m_keyValues.constBegin(),
            m_keyValues.constEnd(), qMakePair(progress, QVariant()), animationValueLessThan);

      if (iter == m_keyValues.constBegin()) {
         // the item pointed to by it is the start element in the range

         if (iter->first == 0 && m_keyValues.count() > 1) {
            m_currentInterval.start = *iter;
            m_currentInterval.end   = *(iter + 1);

         } else {
            m_currentInterval.start = qMakePair(double(0), m_defaultValue);
            m_currentInterval.end = *iter;
         }

      } else if (iter == m_keyValues.constEnd()) {
         // position the iterator on the last item

         --iter;

         if (iter->first == 1 && m_keyValues.count() > 1) {
            // have an end value (item with progress = 1)
            m_currentInterval.start = *(iter - 1);
            m_currentInterval.end   = *iter;

         } else {
            // use the default end value here
            m_currentInterval.start = *iter;
            m_currentInterval.end   = qMakePair(double(1), m_defaultValue);
         }

      } else {
         m_currentInterval.start = *(iter - 1);
         m_currentInterval.end   = *iter;
      }

      // update all the values of the currentInterval
      cs_updateCustomType();
   }

   setCurrentValueForProgress(progress);
}

void QVariantAnimationPrivate::setCurrentValueForProgress(const double progress)
{
   Q_Q(QVariantAnimation);

   const auto & [startProgress, startValue] = m_currentInterval.start;
   const auto & [endProgress, endValue]     = m_currentInterval.end;

   const double localProgress = (progress - startProgress) / (endProgress - startProgress);
   QVariant retval = q->interpolated(startValue, endValue, localProgress);

   qSwap(m_currentValue, retval);
   q->updateCurrentValue(m_currentValue);

   emit q->valueChanged(m_currentValue);
}

QVariant QVariantAnimationPrivate::valueAt(double step) const
{
   auto iter = std::lower_bound(m_keyValues.constBegin(), m_keyValues.constEnd(),
         qMakePair(step, QVariant()), animationValueLessThan);

   if (iter != m_keyValues.constEnd() && ! animationValueLessThan(qMakePair(step, QVariant()), *iter)) {
      return iter->second;
   }

   return QVariant();
}

void QVariantAnimationPrivate::setValueAt(double step, const QVariant &value)
{
   if (step < double(0.0) || step > double(1.0)) {
      qWarning("QVariantAnimation::setValueAt() Invalid step = %f", step);
      return;
   }

   QVariantAnimation::ValuePair pair(step, value);

   auto iter = std::lower_bound(m_keyValues.begin(), m_keyValues.end(),
         pair, animationValueLessThan);

   if (iter == m_keyValues.end() || iter->first != step) {
      m_keyValues.insert(iter, pair);

   } else {
      if (value.isValid()) {
         iter->second = value;      // replaces the previous value
      } else {
         m_keyValues.erase(iter);   // removes the previous value
      }
   }

   recalculateCurrentInterval(true);
}

void QVariantAnimationPrivate::setDefaultStartEndValue(const QVariant &value)
{
   m_defaultValue = value;
   recalculateCurrentInterval(true);
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
   return d->m_easing;
}

void QVariantAnimation::setEasingCurve(const QEasingCurve &easing)
{
   Q_D(QVariantAnimation);
   d->m_easing = easing;
   d->recalculateCurrentInterval();
}

QVariantAnimation::CustomFormula QVariantAnimationPrivate::cs_getCustomType(uint typeId)
{
   switch (typeId) {
      case QVariant::Int:
         return cs_variantFormula<int>;

      case QVariant::UInt:
         return cs_variantFormula<uint>;

      case QVariant::Double:
         return cs_variantFormula<double>;

      case QVariant::Float:
         return cs_variantFormula<float>;

      case QVariant::Line:
         return cs_variantFormula<QLine>;

      case QVariant::LineF:
         return cs_variantFormula<QLineF>;

      case QVariant::Point:
         return cs_variantFormula<QPoint>;

      case QVariant::PointF:
         return cs_variantFormula<QPointF>;

      case QVariant::Size:
         return cs_variantFormula<QSize>;

      case QVariant::SizeF:
         return cs_variantFormula<QSizeF>;

      case QVariant::Rect:
         return cs_variantFormula<QRect>;

      case QVariant::RectF:
         return cs_variantFormula<QRectF>;

      default:
         break;
   }

   libguarded::shared_guarded<QHash<uint, QVariantAnimation::CustomFormula>>::shared_handle hash =
         QVariantAnimation::getFormulas().lock_shared();

   return hash->value(typeId, nullptr);
}

int QVariantAnimation::duration() const
{
   Q_D(const QVariantAnimation);
   return d->m_duration;
}

void QVariantAnimation::setDuration(int msecs)
{
   Q_D(QVariantAnimation);

   if (msecs < 0) {
      qWarning("QVariantAnimation::setDuration() Duration can not be negative");
      return;

   } else if (d->m_duration == msecs) {
      return;
   }

   d->m_duration = msecs;
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

QVariant QVariantAnimation::keyValueAt(double step) const
{
   return d_func()->valueAt(step);
}

void QVariantAnimation::setKeyValueAt(double step, const QVariant &value)
{
   d_func()->setValueAt(step, value);
}

QVector<QVariantAnimation::ValuePair> QVariantAnimation::keyValues() const
{
   return d_func()->m_keyValues;
}

void QVariantAnimation::setKeyValues(const QVector<QVariantAnimation::ValuePair> &values)
{
   Q_D(QVariantAnimation);

   d->m_keyValues = values;
   std::sort(d->m_keyValues.begin(), d->m_keyValues.end(), animationValueLessThan);
   d->recalculateCurrentInterval(true);
}

QVariant QVariantAnimation::currentValue() const
{
   Q_D(const QVariantAnimation);

   if (! d->m_currentValue.isValid()) {
      const_cast<QVariantAnimationPrivate *>(d)->recalculateCurrentInterval();
   }

   return d->m_currentValue;
}

bool QVariantAnimation::event(QEvent *event)
{
   return QAbstractAnimation::event(event);
}

void QVariantAnimation::updateState(QAbstractAnimation::State, QAbstractAnimation::State)
{
}

QVariant QVariantAnimation::interpolated(const QVariant &from, const QVariant &to, double progress) const
{
   return d_func()->m_callBack(from, to, progress);
}

void QVariantAnimation::updateCurrentTime(int)
{
   d_func()->recalculateCurrentInterval();
}

#endif
