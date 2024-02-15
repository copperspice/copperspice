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

#ifndef QEASINGCURVE_H
#define QEASINGCURVE_H

#include <qglobal.h>
#include <qobject.h>

class QEasingCurvePrivate;

class Q_CORE_EXPORT QEasingCurve
{
   CORE_CS_GADGET(QEasingCurve)

 public:
   using EasingFunction = qreal (*)(qreal progress);

   enum Type {
      Linear,
      InQuad, OutQuad, InOutQuad, OutInQuad,
      InCubic, OutCubic, InOutCubic, OutInCubic,
      InQuart, OutQuart, InOutQuart, OutInQuart,
      InQuint, OutQuint, InOutQuint, OutInQuint,
      InSine, OutSine, InOutSine, OutInSine,
      InExpo, OutExpo, InOutExpo, OutInExpo,
      InCirc, OutCirc, InOutCirc, OutInCirc,
      InElastic, OutElastic, InOutElastic, OutInElastic,
      InBack, OutBack, InOutBack, OutInBack,
      InBounce, OutBounce, InOutBounce, OutInBounce,
      InCurve, OutCurve, SineCurve, CosineCurve,
      Custom, NCurveTypes
   };

   CORE_CS_ENUM(Type)

   QEasingCurve(Type type = Linear);
   QEasingCurve(const QEasingCurve &other);

   ~QEasingCurve();

   QEasingCurve &operator=(const QEasingCurve &other);

   bool operator==(const QEasingCurve &other) const;
   bool operator!=(const QEasingCurve &other) const {
      return !(this->operator==(other));
   }

   qreal amplitude() const;
   void setAmplitude(qreal amplitude);

   qreal period() const;
   void setPeriod(qreal period);

   qreal overshoot() const;
   void setOvershoot(qreal overshoot);

   Type type() const;
   void setType(Type type);

   void setCustomType(EasingFunction func);
   EasingFunction customType() const;

   qreal valueForProgress(qreal progress) const;

 private:
   QEasingCurvePrivate *d_ptr;

   friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QEasingCurve &easing);
   friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QEasingCurve &easing);
   friend Q_CORE_EXPORT QDebug operator<<(QDebug debug, const QEasingCurve &easing);
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QEasingCurve &easing);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QEasingCurve &easing);

Q_CORE_EXPORT QDebug operator<<(QDebug debug, const QEasingCurve &easing);

#endif
