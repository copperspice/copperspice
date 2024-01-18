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

   void cs_updateCustomType();

   // might be used in declarative, add Q_CORE_EXPORT
   static QVariantAnimation::CustomFormula cs_getCustomType(uint typeId);

   void convertValues(uint typeId);

   static QVariantAnimationPrivate *get(QVariantAnimation *q) {
      return q->d_func();
   }

   void setValueAt(double, const QVariant &);
   void setDefaultStartEndValue(const QVariant &value);
   void setCurrentValueForProgress(const double progress);
   void recalculateCurrentInterval(bool force = false);

   QVariant valueAt(double step) const;

   // keeps track of the current interval
   struct {
      QVariantAnimation::ValuePair start;
      QVariantAnimation::ValuePair end;

   } m_currentInterval;

   QVariant m_currentValue;
   QVariant m_defaultValue;

   int m_duration;
   QEasingCurve m_easing;

   QVector<QVariantAnimation::ValuePair> m_keyValues;
   QVariantAnimation::CustomFormula m_callBack;

 private:
   Q_DECLARE_PUBLIC(QVariantAnimation)
};

template <typename T>
T cs_genericFormula(const T &from, const T &to, double progress)
{
   return T(from + (to - from) * progress);
}

template <typename T>
QVariant cs_variantFormula(const QVariant &from, const QVariant &to, double progress)
{
   return cs_genericFormula(from.getData<T>(), to.getData<T>(), progress);
}

#endif // QT_NO_ANIMATION

#endif
