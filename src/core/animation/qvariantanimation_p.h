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

#ifndef QVariantAnimation_P_H
#define QVariantAnimation_P_H

#include <qeasingcurve.h>
#include <qvariantanimation.h>
#include <qvector.h>

#include <qabstractanimation_p.h>

#ifndef QT_NO_ANIMATION

class QVariantAnimationPrivate : public QAbstractAnimationPrivate
{
 public:
   QVariantAnimationPrivate();

   static QVariantAnimationPrivate *get(QVariantAnimation *q) {
      return q->d_func();
   }

   void setDefaultStartEndValue(const QVariant &value);

   QVariant currentValue;
   QVariant defaultStartEndValue;

   // keeps track of the current interval
   struct {
      QVariantAnimation::KeyValue start, end;
   } currentInterval;

   QEasingCurve easing;
   int duration;
   QVariantAnimation::KeyValues keyValues;
   QVariantAnimation::Interpolator interpolator;

   void setCurrentValueForProgress(const qreal progress);
   void recalculateCurrentInterval(bool force = false);
   void setValueAt(qreal, const QVariant &);
   QVariant valueAt(qreal step) const;
   void convertValues(int t);

   void updateInterpolator();

 private:
   Q_DECLARE_PUBLIC(QVariantAnimation)
   static Q_CORE_EXPORT QVariantAnimation::Interpolator getInterpolator(int interpolationType);
};

template<typename T>
inline T _q_interpolate(const T &f, const T &t, qreal progress)
{
   return T(f + (t - f) * progress);
}

template<typename T>
inline QVariant _q_interpolateVariant(const T &from, const T &to, qreal progress)
{
   return _q_interpolate(from, to, progress);
}

#endif // QT_NO_ANIMATION

#endif
