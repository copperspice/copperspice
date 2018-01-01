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

#ifndef QVariantAnimation_H
#define QVariantAnimation_H

#include <QtCore/qeasingcurve.h>
#include <QtCore/qabstractanimation.h>
#include <QtCore/qvector.h>
#include <QtCore/qvariant.h>
#include <QtCore/qpair.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ANIMATION

class QVariantAnimationPrivate;

class Q_CORE_EXPORT QVariantAnimation : public QAbstractAnimation
{
   CORE_CS_OBJECT(QVariantAnimation)

   CORE_CS_PROPERTY_READ(startValue, startValue)
   CORE_CS_PROPERTY_WRITE(startValue, setStartValue)
   CORE_CS_PROPERTY_READ(endValue, endValue)
   CORE_CS_PROPERTY_WRITE(endValue, setEndValue)
   CORE_CS_PROPERTY_READ(currentValue, currentValue)
   CORE_CS_PROPERTY_NOTIFY(currentValue, valueChanged)
   CORE_CS_PROPERTY_READ(duration, duration)
   CORE_CS_PROPERTY_WRITE(duration, setDuration)
   CORE_CS_PROPERTY_READ(easingCurve, easingCurve)
   CORE_CS_PROPERTY_WRITE(easingCurve, setEasingCurve)

 public:
   typedef QPair<qreal, QVariant> KeyValue;
   typedef QVector<KeyValue> KeyValues;

   QVariantAnimation(QObject *parent = nullptr);
   ~QVariantAnimation();

   QVariant startValue() const;
   void setStartValue(const QVariant &value);

   QVariant endValue() const;
   void setEndValue(const QVariant &value);

   QVariant keyValueAt(qreal step) const;
   void setKeyValueAt(qreal step, const QVariant &value);

   KeyValues keyValues() const;
   void setKeyValues(const KeyValues &values);

   QVariant currentValue() const;

   int duration() const override;
   void setDuration(int msecs);

   QEasingCurve easingCurve() const;
   void setEasingCurve(const QEasingCurve &easing);

   typedef QVariant (*Interpolator)(const void *from, const void *to, qreal progress);

   CORE_CS_SIGNAL_1(Public, void valueChanged(const QVariant &value))
   CORE_CS_SIGNAL_2(valueChanged, value)

 protected:
   QVariantAnimation(QVariantAnimationPrivate &dd, QObject *parent = nullptr);
   bool event(QEvent *event) override;

   void updateCurrentTime(int) override;
   void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState) override;

   virtual void updateCurrentValue(const QVariant &value) = 0;
   virtual QVariant interpolated(const QVariant &from, const QVariant &to, qreal progress) const;

 private:
   template <typename T> friend void qRegisterAnimationInterpolator(QVariant (*func)(const T &, const T &, qreal));
   static void registerInterpolator(Interpolator func, int interpolationType);

   Q_DISABLE_COPY(QVariantAnimation)
   Q_DECLARE_PRIVATE(QVariantAnimation)
};

template <typename T>
void qRegisterAnimationInterpolator(QVariant (*func)(const T &from, const T &to, qreal progress))
{
   QVariantAnimation::registerInterpolator(reinterpret_cast<QVariantAnimation::Interpolator>(func), qMetaTypeId<T>());
}

#endif //QT_NO_ANIMATION

QT_END_NAMESPACE

#endif //QANIMATION_H
