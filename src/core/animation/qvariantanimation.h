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

#ifndef QVariantAnimation_H
#define QVariantAnimation_H

#include <qabstractanimation.h>
#include <qeasingcurve.h>
#include <qhash.h>
#include <qpair.h>
#include <qvariant.h>
#include <qvector.h>

#include <cs_shared_guarded.h>

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
   using ValuePair = QPair<double, QVariant>;

   QVariantAnimation(QObject *parent = nullptr);
   QVariantAnimation (const QVariantAnimation & ) = delete;

   ~QVariantAnimation();

   QVariant startValue() const;
   void setStartValue(const QVariant &value);

   QVariant endValue() const;
   void setEndValue(const QVariant &value);

   QVariant keyValueAt(double step) const;
   void setKeyValueAt(double step, const QVariant &value);

   QVector<QVariantAnimation::ValuePair> keyValues() const;
   void setKeyValues(const QVector<QVariantAnimation::ValuePair> &keyValues);

   QVariant currentValue() const;

   int duration() const override;
   void setDuration(int msecs);

   QEasingCurve easingCurve() const;
   void setEasingCurve(const QEasingCurve &easing);

   using CustomFormula = std::function<QVariant (const QVariant &, const QVariant &, double)>;

   template <typename T>
   static void cs_addCustomType(CustomFormula callback);

   CORE_CS_SIGNAL_1(Public, void valueChanged(const QVariant &value))
   CORE_CS_SIGNAL_2(valueChanged, value)

 protected:
   QVariantAnimation(QVariantAnimationPrivate &dd, QObject *parent = nullptr);
   bool event(QEvent *event) override;

   void updateCurrentTime(int) override;
   void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState) override;

   virtual void updateCurrentValue(const QVariant &value) = 0;
   virtual QVariant interpolated(const QVariant &from, const QVariant &to, double progress) const;

 private:
   static libguarded::shared_guarded<QHash<uint, QVariantAnimation::CustomFormula>> &getFormulas();

   Q_DECLARE_PRIVATE(QVariantAnimation)
};

template <typename T>
void QVariantAnimation::cs_addCustomType(CustomFormula callback)
{
   // add a custom formula for a given T, this must occur before any annimation which uses this T is constructed
   // to remove and use the default formula, pass nulptr for func, overrides any existing formula

   uint typeId = QVariant::typeToTypeId<T>();

   libguarded::shared_guarded<QHash<uint, QVariantAnimation::CustomFormula>>::handle hash = getFormulas().lock();

   if (callback) {
      hash->insert(typeId, callback);

   } else {
      // std::function is empty
      hash->remove(typeId);
   }
}

#endif // QT_NO_ANIMATION

#endif
