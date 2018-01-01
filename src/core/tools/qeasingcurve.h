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

#ifndef QEASINGCURVE_H
#define QEASINGCURVE_H

#include <QtCore/qglobal.h>
#include <qobject.h>

QT_BEGIN_NAMESPACE

class QEasingCurvePrivate;

class Q_CORE_EXPORT QEasingCurve
{
   CORE_CS_GADGET(QEasingCurve)

 public:
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
   inline bool operator!=(const QEasingCurve &other) const {
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
   typedef qreal (*EasingFunction)(qreal progress);
   void setCustomType(EasingFunction func);
   EasingFunction customType() const;

   qreal valueForProgress(qreal progress) const;

 private:
   QEasingCurvePrivate *d_ptr;

   friend Q_CORE_EXPORT QDebug operator<<(QDebug debug, const QEasingCurve &item);

#ifndef QT_NO_DATASTREAM
   friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QEasingCurve &);
   friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QEasingCurve &);
#endif

};

Q_CORE_EXPORT QDebug operator<<(QDebug debug, const QEasingCurve &item);

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QEasingCurve &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QEasingCurve &);
#endif

QT_END_NAMESPACE

#endif
