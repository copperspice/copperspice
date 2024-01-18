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

#ifndef QSCROLLERPROPERTIES_H
#define QSCROLLERPROPERTIES_H

#include <qscopedpointer.h>
#include <qvariant.h>

class QScroller;
class QScrollerPrivate;
class QScrollerPropertiesPrivate;

class Q_GUI_EXPORT QScrollerProperties
{
 public:
   enum OvershootPolicy {
      OvershootWhenScrollable,
      OvershootAlwaysOff,
      OvershootAlwaysOn
   };

   enum FrameRates {
      Standard,
      Fps60,
      Fps30,
      Fps20
   };

   enum ScrollMetric {
      MousePressEventDelay,                    // qreal [s]
      DragStartDistance,                       // qreal [m]
      DragVelocitySmoothingFactor,             // qreal [0..1/s]  (complex calculation involving time) v = v_new* DASF + v_old * (1-DASF)
      AxisLockThreshold,                       // qreal [0..1] atan(|min(dx,dy)|/|max(dx,dy)|)

      ScrollingCurve,                          // QEasingCurve
      DecelerationFactor,                      // slope of the curve

      MinimumVelocity,                         // qreal [m/s]
      MaximumVelocity,                         // qreal [m/s]
      MaximumClickThroughVelocity,             // qreal [m/s]

      AcceleratingFlickMaximumTime,            // qreal [s]
      AcceleratingFlickSpeedupFactor,          // qreal [1..]

      SnapPositionRatio,                       // qreal [0..1]
      SnapTime,                                // qreal [s]

      OvershootDragResistanceFactor,           // qreal [0..1]
      OvershootDragDistanceFactor,             // qreal [0..1]
      OvershootScrollDistanceFactor,           // qreal [0..1]
      OvershootScrollTime,                     // qreal [s]

      HorizontalOvershootPolicy,               // enum OvershootPolicy
      VerticalOvershootPolicy,                 // enum OvershootPolicy
      FrameRate,                               // enum FrameRates

      ScrollMetricCount
   };

   QScrollerProperties();
   QScrollerProperties(const QScrollerProperties &sp);

   virtual ~QScrollerProperties();

   QScrollerProperties &operator=(const QScrollerProperties &other);

   bool operator==(const QScrollerProperties &other) const;
   bool operator!=(const QScrollerProperties &other) const;

   static void setDefaultScrollerProperties(const QScrollerProperties &sp);
   static void unsetDefaultScrollerProperties();

   QVariant scrollMetric(ScrollMetric metric) const;
   void setScrollMetric(ScrollMetric metric, const QVariant &value);

 protected:
   QScopedPointer<QScrollerPropertiesPrivate> d;

 private:
   QScrollerProperties(QScrollerPropertiesPrivate &dd);

   friend class QScrollerPropertiesPrivate;
   friend class QScroller;
   friend class QScrollerPrivate;
};

CS_DECLARE_METATYPE(QScrollerProperties)

#endif
